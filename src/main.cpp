/*
 * Program 4 example with diffuse and spline camera PRESS 'g'
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn (spline D. McGirr)
 */

#include <iostream>
#include <glad/glad.h>
#include <algorithm>
#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"
#include "stb_image.h"
#include "InputHandler.h"
#include "Entity.h"
#include "PhysicalObject.h"
#include "ShaderManager.h"
#include "ImportExport.h"
#include "Camera.h"
#include "LevelEditor.h"
#include "EventManager.h"
#include "GameManager.h"

#include <chrono>
#include <array>

// checks if apple device
#ifdef __APPLE__
    #define MA_NO_RUNTIME_LINKING
#endif
#define MINIAUDIO_IMPLEMENTATION
#include "../ext/miniaudio/miniaudio.h"

#define TINYOBJLOADER_IMPLEMENTATION
#define PI 3.1415927

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;


// Where the resources are loaded from
std::string resourceDir = "../resources";
std::string WORLD_FILE_NAME = "/world.json";
bool editMode = false;
float editSpeed = 7.0f;

map<string, shared_ptr<Shader>> shaders;
map<string, shared_ptr<Texture>> textureLibrary = { {"", nullptr} };
map<string, shared_ptr<Entity>> worldentities;
vector<string> tagList = { "" };
vector<shared_ptr<Entity>> collidables;

shared_ptr<Entity> cur_entity = nullptr;

float deltaTime;

// 	view pitch dist angle playerpos playerrot animate g_eye
Camera cam = Camera(vec3(0, 0, 1), 17, 4, 0, vec3(0, -1.12, 0), 0, vec3(0, 0.5, 5));
Camera freeCam = Camera(vec3(0, 0, 1), 17, 4, 0, vec3(0, -1.12, 0), 0, vec3(0, 0.5, 5), true);
Camera* activeCam = &cam;

// for background music, called once in main loop
ma_engine engine;

// Event e = Event("../resources/cute-world.mp3");

class Application : public EventCallbacks
{

public:
	WindowManager * windowManager = nullptr;
	// Our shader program - use this one for Blinn-Phong has diffuse
	//shared_ptr<Shader> reg;
	//shared_ptr<Shader> depth;
	//shared_ptr<Shader> proghmap;
	//Our shader program for textures
	// shared_ptr<Shader> tex;

	shared_ptr<Entity> player;

	ImporterExporter *levelEditor = new ImporterExporter(&shaders, &textureLibrary, &worldentities, &tagList, &collidables);
	GameManager *gameManager = new GameManager();

	shared_ptr<Program> DepthProg;
	GLuint depthMapFBO;
	const GLuint S_WIDTH = 1024, S_HEIGHT = 1024;
	GLuint depthMap;

	vec3 light_vec = vec3(1.0, 2.5, 1.0);
  
	LevelEditor* leGUI = new LevelEditor();

	InputHandler ih;
	int collisionSounds[1];

	Entity bf1 = Entity();
	Entity bf2 = Entity();
	Entity bf3 = Entity();
	// Entity *catEnt = new Manchot();
	
  	std::vector<Entity> bf;
	
	std::vector<Entity> trees;

	std::vector<Entity> flowers;

	EventManager *eManager = new EventManager();
	// Event *walking = new Event("../resources/music.mp3", &walkingEngine);


	int bf_flags[3] = {0, 0, 0};

	int nextID = 0;

	// added region buffer to hold regional data (1, 2, 3 = r, g, b)
	//global data for ground plane - direct load constant defined CPU data to GPU (not obj)
	GLuint GrndBuffObj, GrndRegionBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;
	//ground VAO
	GLuint GroundVertexArrayID;
  
	vec3 strafe = vec3(1, 0, 0);

	double cursor_x = 0;
	double cursor_y = 0;

	//bounds for world
	double bounds;

	int editSRT = 0; // 0 - translation, 1 - rotation, 2 - scale

	// hmap for terrain
	shared_ptr<Texture> hmap;
	vec3 groundPos = vec3(0, 0, 0);

	ma_engine walkingEngine;
	ma_engine collectionEngine;
		
	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_L && action == GLFW_PRESS && !leGUI->diableInput) {
			editMode = !editMode;
			editSRT = 0;
			if (editMode) {
				activeCam = &freeCam;
			}
			else {
				activeCam = &cam;
				for (auto ent : worldentities) {
					if (ent.second->tag == "player") {
						player = ent.second;
					}
				}
			}
		}

		// KEY PRESSED

		if (editMode) {
			if (!leGUI->diableInput) {
				if (action == GLFW_PRESS) {
				switch (key) {
					case GLFW_KEY_W:
						freeCam.vel.z = editSpeed;
						break;
					case GLFW_KEY_S:
						freeCam.vel.z = -editSpeed;
						break;
					case GLFW_KEY_A:
						freeCam.vel.x = -editSpeed;
						break;
					case GLFW_KEY_D:
						freeCam.vel.x = editSpeed;
						break;
					case GLFW_KEY_SPACE:
						freeCam.vel.y = -editSpeed;
						break;
					case GLFW_KEY_LEFT_CONTROL:
						freeCam.vel.y = editSpeed;
						break;
					case GLFW_KEY_V:
						levelEditor->saveToFile(WORLD_FILE_NAME);
						break;
					case GLFW_KEY_F:
						if (cur_entity) {
							freeCam.cameraPos = cur_entity->position + vec3(0,2,2);
							freeCam.pitch = atan((freeCam.cameraPos.z - cur_entity->position.z) /
								(freeCam.cameraPos.y - cur_entity->position.y));
							freeCam.angle = 0;
						}
						break;
					}
				}
			}			
			if (action == GLFW_RELEASE) {
				switch (key) {
					case GLFW_KEY_W:
					case GLFW_KEY_S:
						freeCam.vel.z = 0.0;
						break;
					case GLFW_KEY_A:
					case GLFW_KEY_D:
						freeCam.vel.x = 0.0;
						break;
					case GLFW_KEY_SPACE:
					case GLFW_KEY_LEFT_CONTROL:
						freeCam.vel.y = 0.0;
						break;
				}
			}
		}
		else {
			if (key == GLFW_KEY_W && (action == GLFW_PRESS)) {
				ih.inputStates[0] = 1;
			}

			if (key == GLFW_KEY_A && (action == GLFW_PRESS)) {
				ih.inputStates[1] = 1;
			}

			if (key == GLFW_KEY_S && (action == GLFW_PRESS)) {	
				ih.inputStates[2] = 1;
			}

			if (key == GLFW_KEY_D && (action == GLFW_PRESS)) {
				ih.inputStates[3] = 1;
			}

			if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS)){
				ih.inputStates[4] = 1;
			}

			if (key == GLFW_KEY_LEFT_SHIFT && (action == GLFW_PRESS)){
				ih.inputStates[5] = 1;
			}

			if (key == GLFW_KEY_C && (action == GLFW_PRESS)) {
				player->sliding = true;
			}
	
			if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}

			// KEY RELEASED

			if (key == GLFW_KEY_W && (action == GLFW_RELEASE)){
				ih.inputStates[0] = 0;
			}

			if (key == GLFW_KEY_A && (action == GLFW_RELEASE)){
				ih.inputStates[1] = 0;
			}

			if (key == GLFW_KEY_S && (action == GLFW_RELEASE)){
				ih.inputStates[2] = 0;
			}

			if (key == GLFW_KEY_D && (action == GLFW_RELEASE)){
				ih.inputStates[3] = 0;
			}

			if (key == GLFW_KEY_SPACE && (action == GLFW_RELEASE)){
				ih.inputStates[4] = 0;
			}

			if (key == GLFW_KEY_LEFT_SHIFT && (action == GLFW_RELEASE)){
				ih.inputStates[5] = 0;
			}

			if (key == GLFW_KEY_C && (action == GLFW_RELEASE)) {
				player->sliding = false;
			}
			
			if (key == GLFW_KEY_F1 && action == GLFW_RELEASE) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			// Entity *catptr = &catEnt;
			ih.handleInput(player.get(), &cam, deltaTime);
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
		cam.angle -= 10 * (deltaX / 57.296);
	}


	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS) {
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				glfwGetCursorPos(window, &posX, &posY);
				cout << "Pos X " << posX <<  " Pos Y " << posY << endl;

				//editor mode selection
				if(editMode && !leGUI->diableInput){
					// Get the window height
					int windowHeight;
					glfwGetWindowSize(window, NULL, &windowHeight);
					selectEntity(posX, posY, windowHeight);
				}
			}
			else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				int cursor_mode = glfwGetInputMode(window, GLFW_CURSOR);
				if (cursor_mode == GLFW_CURSOR_DISABLED)
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				else {
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
					glfwGetCursorPos(window, &cursor_x, &cursor_y);
				}
			}
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height) {
		glViewport(0, 0, width, height);
	}

	void cursorPosCallback(GLFWwindow* window, double x, double y) {
		if (editMode) {
			if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
				float sensitivity = 0.001f;

				freeCam.angle += (x-cursor_x) * sensitivity;
				freeCam.pitch += (y-cursor_y) * sensitivity;

				freeCam.pitch = std::min(PI/2 - EPSILON, std::max(-PI/2 + EPSILON, (double)freeCam.pitch));

				cursor_x = x;
				cursor_y = y;
			}
		}
		else {
			cam.angle -= 0.001*(x-cursor_x);
			cursor_x = x;
			cursor_y = y;
		}
  }

#pragma endregion
	void selectEntity(int x, int y, int windowHeight){
		glFlush();
		glFinish(); 

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// Read the pixel at location x,y
		int glY = windowHeight - y;
		unsigned char data[3];
		glReadPixels(x, glY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, data);
		cout << "r: " << +data[0] << "   g: " << +data[1] << "   b: " << +data[2] << endl;

		// convert color to entity id
		int pickedID = -1;

		// Iterate through all possible IDs to find the one that matches the color
		for (int testID = 0; testID < Entity::NEXT_ID; ++testID) {
			int r = (testID * 137) % 256;
			int g = (testID * 149) % 256;
			int b = (testID * 163) % 256;

			if (r == data[0] && g == data[1] && b == data[2]) {
				pickedID = testID;
				break;
			}
		}

		//cout << "pickedId = " << pickedID << endl;
		
		// find the entity with that id (if an entity was clicked) and set as active
		map<string, shared_ptr<Entity>>::iterator i;
		for (i = worldentities.begin(); i != worldentities.end(); i++) {
			shared_ptr<Entity> entity = i->second;
			if(entity->id == pickedID){
				leGUI->setCurName(i->first);
				cout << "active entity is now " << pickedID << endl;
				break;
			}
		}
	}


	void initShadow() {

		//generate the FBO for the shadow depth
		glGenFramebuffers(1, &depthMapFBO);

		//generate the texture
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, S_WIDTH, S_HEIGHT, 
			0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//bind with framebuffer's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.72f, .84f, 1.06f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		// glEnable(GL_CLIP_DISTANCE0);

		DepthProg = make_shared<Program>();
		DepthProg->setVerbose(true);
		DepthProg->setShaderNames(resourceDirectory + "/depth_vert.glsl", resourceDirectory + "/depth_frag.glsl");
		DepthProg->init();
		DepthProg->addUniform("LP");
		DepthProg->addUniform("LV");
		DepthProg->addUniform("P");
		DepthProg->addUniform("V");
		DepthProg->addUniform("M");
		DepthProg->addUniform("Texture0");
		DepthProg->addAttribute("vertPos");
		DepthProg->addAttribute("vertNor");
		DepthProg->addAttribute("vertTex");

		hmap = make_shared<Texture>();
		hmap->setFilename(resourceDirectory + "/hmap.png");
		hmap->initHmap();
		initShadow();

		for (auto ent : worldentities) {
			if (ent.second->tag == "player") {
				player = ent.second;
			}
		}

		//eManager->events.insert_or_assign("walking", walking);

	}

	void initSoundEngines(){
		
		ma_result result = ma_engine_init(NULL, &engine);
		if (result != MA_SUCCESS) {
			printf("Failed to initialize BACKGROUND MUSIC audio engine.");
			//return -1;
		}

		result = ma_engine_init(NULL, &walkingEngine);
		if (result != MA_SUCCESS) {
			printf("Failed to initialize WALKING audio engine.");
			//return -1;
		}

		result = ma_engine_init(NULL, &collectionEngine);
		if (result != MA_SUCCESS) {
			printf("Failed to initialize WALKING audio engine.");
			//return -1;
		}

		Event *walkingEv = new Event("../resources/walking-grass.mp3", &walkingEngine, true, "walking");
		eManager->events.insert_or_assign("walking", walkingEv);

		Event *collectionEv = new Event("../resources/marimba-y.mp3", &collectionEngine, false, "collection");
		eManager->events.insert_or_assign("collection", collectionEv);

		//return 0;
	}

	void uninitSoundEngines(){
		ma_engine_uninit(&engine);
		ma_engine_uninit(&walkingEngine);
		ma_engine_uninit(&collectionEngine);
	}

	void initGeom(const std::string& resourceDirectory)
	{   
		if (player) {
			player->m.forward = vec4(0, 0, 0.1, 1);
			player->m.velocity = vec3(0.1) * vec3(player->m.forward);
		}
		
		applyCollider();

		//code to load in the ground plane (CPU defined data passed to GPU)
		initHMapGround();
	}

	void applyCollider() {
		for (auto ent : collidables) {
			ent->collider = new Collider(ent.get());
			ent->collider->SetEntityID(ent->id);
			if (ent == player) {
				ent->collider->entityName = 'p';
			}
			else {
				ent->collider->entityName = 'c';
			}
			if (ent->tag == "food") {
				ent->collider->collectible = true;
			}			
		}
		cam.collider = new Collider(&cam);
	}

	//directly pass quad for the ground to the GPU
	void initHMapGround() {
		const float Y_MAX = 75;
		const float Y_MIN = -Y_MAX;

		vector<float> vertices;
		vector<float> regions;
		auto hmap_dim = hmap->getDim();
		auto hmap_data = hmap->getData();
		for (unsigned int i = 0; i < hmap_dim.second; i++) {
			for (unsigned int j = 0; j < hmap_dim.first; j++) {
				bool pit;
				float hvalr = (float)*(hmap_data + 3 * (i * hmap_dim.first + j));
				float hvalg = (float)*(hmap_data + 3 * (i * hmap_dim.first + j) + 1);
				float hvalb = (float)*(hmap_data + 3 * (i * hmap_dim.first + j) + 2);
				float hval = (hvalr + hvalg + hvalb) / (3 * 255.0f);
				//float hval = (std::max)(hvalr, (std::max)(hvalg, hvalb)) / 255.0f;
				
				pit = hval < .01;

				vertices.push_back(j - hmap_dim.first / 2.0f);
				vertices.push_back(hval * (Y_MAX - Y_MIN) + Y_MIN);
				vertices.push_back(i - hmap_dim.second / 2.0f);

				regions.push_back((pit ? 72 : hvalr) / 255.0f);
				regions.push_back(hvalg / 255.0f);
				regions.push_back((pit ? 100 : hvalb) / 255.0f);
			}
		}
		// hmap->freeData();

		vector<unsigned int> indices;
		for (unsigned int i = 0; i < hmap_dim.second - 1; i++) {
			for (unsigned int j = 0; j < hmap_dim.first - 1; j++) {
				int v0 = i * hmap_dim.first + j;
				int v1 = (i + 1) * hmap_dim.first + j;
				int v2 = (i + 1) * hmap_dim.first + j + 1;
				int v3 = i * hmap_dim.first + j + 1;

				indices.push_back(v0);
				indices.push_back(v2);
				indices.push_back(v3);

				indices.push_back(v0);
				indices.push_back(v1);
				indices.push_back(v2);
			}
		}

		Shape terrain;
		terrain.createShape(vertices, indices);
		terrain.generateNormals();
		std::vector<float> normals = terrain.getNormals();

		std::cout << "vert size : " << vertices.size() << " ind size: " << indices.size() << std::endl;

		//generate the ground VAO
      	glGenVertexArrays(1, &GroundVertexArrayID);
      	glBindVertexArray(GroundVertexArrayID);

      	glGenBuffers(1, &GrndBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
		
		glGenBuffers(1, &GrndRegionBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndRegionBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, regions.size() * sizeof(float), regions.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &GrndNorBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);

      	glGenBuffers(1, &GIndxBuffObj);
     	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
      	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		g_GiboLen = indices.size();

		if (player) {
			player->collider->SetGround(groundPos, vec3(1, Y_MAX - Y_MIN, 1));
		}
		
		cam.collider->SetGround(groundPos, vec3(1,Y_MAX-Y_MIN,1));

      }
	
      //code to draw the ground plane
     void drawGround(shared_ptr<Shader> curS) {
     	glBindVertexArray(GroundVertexArrayID);

		//draw the ground plane 
  		curS->setModel(groundPos, 0, 0, 0, 1);
  		glEnableVertexAttribArray(0);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
  		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		
		glEnableVertexAttribArray(1);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
  		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		
		glEnableVertexAttribArray(2);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndRegionBuffObj);
  		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		

   		// draw!
  		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
  		glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_INT, 0);

  		glDisableVertexAttribArray(0);
  		glDisableVertexAttribArray(1);
  		glDisableVertexAttribArray(2);
  		curS->prog->unbind();
     }



	mat4 SetOrthoMatrix(shared_ptr<Program> curShade) {
		mat4 ortho = glm::ortho(-15.0, 15.0, -15.0, 15.0, 0.1, 20.0);

		glUniformMatrix4fv(curShade->getUniform("LP"), 1, GL_FALSE, value_ptr(ortho));
		return ortho;
	}


	mat4 SetLightView(shared_ptr<Program> curShade, vec3 pos, vec3 LA, vec3 up) {
		mat4 Cam = glm::lookAt(pos, LA, up);

		glUniformMatrix4fv(curShade->getUniform("LV"), 1, GL_FALSE, value_ptr(Cam));
		return Cam;
	}


	void drawShadowMap() {
		float butterfly_height[3] = {1.1, 1.7, 1.5};

		vec3 butterfly_loc[3];
		butterfly_loc[0] = vec3(-2.3, -1, 3);
		butterfly_loc[1] = vec3(-2, -1.2, -3);
		butterfly_loc[2] = vec3(4, -1, 4);

		// BRDFmaterial imported from save file
		shaders["skybox"]->prog->setVerbose(false);
		map<string, shared_ptr<Entity>>::iterator i;

		for (i = worldentities.begin(); i != worldentities.end(); i++) {
			shared_ptr<Entity> entity = i->second;
			entity->generateModel();
		}

		for (i = worldentities.begin(); i != worldentities.end(); i++) {
			shared_ptr<Entity> entity = i->second;
			entity->model->Draw(DepthProg);

			
			//for (int i = 0; i < entity->objs.size(); i++) {	
			//	entity->objs[i]->draw(DepthProg);
			//}
		}
		
		DepthProg->unbind();

		bounds = std::sqrt(   //update cat's distance from skybox
			cam.player_pos[0] * cam.player_pos[0]
			+ cam.player_pos[2] * cam.player_pos[2]
		);

		// Pop matrix stacks.
	}


	void drawObjects(float aspect, mat4 LSpace, float deltaTime) {
	
		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();


		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 1000.0f);
		
		//material shader first
		shared_ptr<Shader> curS = shaders["reg"];
		curS->prog->bind();
		glUniformMatrix4fv(curS->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		activeCam->SetView(curS->prog, hmap);

		// directional light
		glUniform3f(curS->prog->getUniform("lightDir"), light_vec.x, light_vec.y, light_vec.z);
		glUniform1i(curS->prog->getUniform("shadowDepth"), 1);
      	glUniformMatrix4fv(curS->prog->getUniform("LS"), 1, GL_FALSE, value_ptr(LSpace));


		float butterfly_height[3] = {1.1, 1.7, 1.5};

		vec3 butterfly_loc[3];
		butterfly_loc[0] = vec3(-2.3, -1, 3);
		butterfly_loc[1] = vec3(-2, -1.2, -3);
		butterfly_loc[2] = vec3(4, -1, 4);

		// BRDFmaterial imported from save file
		shaders["skybox"]->prog->setVerbose(false);
		map<string, shared_ptr<Entity>>::iterator i;

		for (i = worldentities.begin(); i != worldentities.end(); i++) {
			shared_ptr<Entity> entity = i->second;
			entity->generateModel();
		}
    
		for (i = worldentities.begin(); i != worldentities.end(); i++) {
			shared_ptr<Entity> entity = i->second;
			if (shaders[entity->defaultShaderName] != curS) {
				curS->prog->unbind();
				curS = shaders[entity->defaultShaderName];
				curS->prog->bind();
				glUniformMatrix4fv(curS->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
				activeCam->SetView(curS->prog, hmap);
			}	
			if (shaders["skybox"] == curS) {
				entity->position = activeCam->cameraPos;
				// skybox is always the furthest surface away
				glDepthFunc(GL_LEQUAL);
			}

			if (entity->collider) {
				if (entity->id == player->id) {
					entity->updateMotion(deltaTime, hmap, collidables, collisionSounds);
				}
			}
	
			glUniform3f(curS->prog->getUniform("lightDir"), light_vec.x, light_vec.y, light_vec.z);
			glUniform1i(curS->prog->getUniform("shadowDepth"), 1);
			glUniformMatrix4fv(curS->prog->getUniform("LS"), 1, GL_FALSE, value_ptr(LSpace));
			glUniformMatrix4fv(curS->prog->getUniform("M"), 1, GL_FALSE, value_ptr(entity->modelMatrix));
      
			for (auto& meshPair : entity->model->meshes) {
				curS->setMaterial(meshPair.second.mat);
				meshPair.second.Draw(curS->prog);
			}
			
			if (shaders["skybox"] == curS) {
				// deactivate skybox backfill
				glDepthFunc(GL_LESS);
			}
		}
		

		curS->prog->unbind();

		curS = shaders["hmap"];

		curS->prog->bind();
		glUniformMatrix4fv(curS->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		activeCam->SetView(curS->prog, hmap);
		glUniform3f(curS->prog->getUniform("lightDir"), light_vec.x, light_vec.y, light_vec.z);
		glUniform1i(curS->prog->getUniform("shadowDepth"), 1);
      	glUniformMatrix4fv(curS->prog->getUniform("LS"), 1, GL_FALSE, value_ptr(LSpace));
		drawGround(curS);  //draw ground here

		bounds = std::sqrt(   //update cat's distance from skybox
			cam.player_pos[0] * cam.player_pos[0]
			+ cam.player_pos[2] * cam.player_pos[2]
		);

		// Pop matrix stacks.
		Projection->popMatrix();
	}

	void drawEditorObjects(float aspect, mat4 LSpace, float deltaTime) {
	
		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();


		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 1000.0f);

		//material shader first
		shared_ptr<Shader> curS = shaders["edit"];
		curS->prog->bind();
		glUniformMatrix4fv(curS->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		activeCam->SetView(curS->prog, hmap);

		float butterfly_height[3] = {1.1, 1.7, 1.5};

		vec3 butterfly_loc[3];
		butterfly_loc[0] = vec3(-2.3, -1, 3);
		butterfly_loc[1] = vec3(-2, -1.2, -3);
		butterfly_loc[2] = vec3(4, -1, 4);

	
		//vector<shared_ptr<Entity>> tempCollisionList = {worldentities["cube1"], player};

		// BRDFmaterial imported from save file
		map<string, shared_ptr<Entity>>::iterator i;

		for (i = worldentities.begin(); i != worldentities.end(); i++) {
			shared_ptr<Entity> entity = i->second;
			entity->generateModel();
		}
		for (i = worldentities.begin(); i != worldentities.end(); i++) {
			shared_ptr<Entity> entity = i->second;
			// glUniformMatrix4fv(curS->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
			// activeCam->SetView(curS->prog);


			//if (entity->collider) {
			//	vec4 colNorm =entity->collider->CheckCollision(deltaTime, tempCollisionList);
			//	if (entity->id == player->id) {
			//		entity->updateMotion(deltaTime, hmap, colNorm);
			//	}
			//}
	
			glUniformMatrix4fv(curS->prog->getUniform("M"), 1, GL_FALSE, value_ptr(entity->modelMatrix));
			glUniform3fv(curS->prog->getUniform("PickingColor"), 1, value_ptr(glm::vec3(entity->editorColor.r, entity->editorColor.g, entity->editorColor.b)));
			entity->model->Draw(curS->prog);
	
			//for (int i = 0; i < entity->objs.size(); i++) {
			//	glUniform3fv(curS->prog->getUniform("PickingColor"), 1, value_ptr(glm::vec3(entity->editorColor.r, entity->editorColor.g, entity->editorColor.b)));
			//	entity->objs[i]->draw(curS->prog);

			//}
		}
	

		curS->prog->unbind();
		curS = shaders["hmap"];

		curS->prog->bind();
		glUniformMatrix4fv(curS->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		activeCam->SetView(curS->prog, hmap);
		glUniform3f(curS->prog->getUniform("lightDir"), light_vec.x, light_vec.y, light_vec.z);
		glUniform1i(curS->prog->getUniform("shadowDepth"), 1);
      	glUniformMatrix4fv(curS->prog->getUniform("LS"), 1, GL_FALSE, value_ptr(LSpace));
		drawGround(curS);  //draw ground here

		bounds = std::sqrt(   //update cat's distance from skybox
			cam.player_pos[0] * cam.player_pos[0]
			+ cam.player_pos[2] * cam.player_pos[2]
		);

		// Pop matrix stacks.
		Projection->popMatrix();

		// editor mode 
		leGUI->NewFrame();
		leGUI->Update();
		leGUI->Render();
	}

	
	bool walkingEvent(){
		return ((player->m.curSpeed > 0 || player->m.curSpeed < 0) && (player->grounded));
	}
	
	bool collectionEvent(){
		return (collisionSounds[0] == 1 && eManager->eventHistory->at("collection") == false);
	}
	
	
	void checkSounds(){
		if (walkingEvent()) {eManager->triggerSound("walking");}
		else {eManager->stoppingSound("walking");}

		if (collectionEvent()) {
			cout << "collection event triggered, starting sound" << endl;
			eManager->triggerSound("collection");
			// collisionSounds[0] == 0;
		}
		else {eManager->stoppingSound("collection");}


	}

	void render(float frametime) {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

		vec3 lightLA = vec3(0.0);
    	vec3 lightUp = vec3(0, 1, 0);
		mat4 LO, LV, LSpace;
		// cout << "before" << endl;
		glViewport(0, 0, S_WIDTH, S_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);

		DepthProg->bind();
		  //TODO you will need to fix these
		LO = SetOrthoMatrix(DepthProg);
		LV = SetLightView(DepthProg, light_vec, lightLA, lightUp);
		drawShadowMap();
		DepthProg->unbind();
		glCullFace(GL_BACK);
		// cout << "1 pass" << endl;

      //this sets the output back to the screen
  	 	glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, width, height);
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE1);
  		glBindTexture(GL_TEXTURE_2D, depthMap);
		
		//player->updateMotion(frametime, hmap);
		if (player) {
			cam.player_pos = player->position;
		}
	
      	LSpace = LO*LV;
		float aspect = width/(float)height;
		if(editMode){
			drawEditorObjects(aspect, LSpace, frametime);
		}
		else{
			drawObjects(aspect, LSpace, frametime);
		}
		
		checkSounds();
	}

};

int main(int argc, char *argv[]) {
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2) {
		resourceDir = argv[1];
	}

	// printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
	// printf("Renderer: %s\n", glGetString(GL_RENDERER));

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();

	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;
	glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->levelEditor->loadFromFile(WORLD_FILE_NAME);
	//application->gameManager->init(application->player, worldentities);

	application->init(resourceDir);
	application->initGeom(resourceDir);
	application->initSoundEngines();


	Event *ev = new Event("../resources/french-mood.mp3", &engine, true, "background");
	ev->startSound();

	float dt = 1 / 60.0;
	auto lastTime = chrono::high_resolution_clock::now();

	application->leGUI->Init(windowManager->getHandle());

	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// save current time for next frame
		auto nextLastTime = chrono::high_resolution_clock::now();

		// get time since last frame
		deltaTime = 
			chrono::duration_cast<std::chrono::microseconds>(
				chrono::high_resolution_clock::now() - lastTime)
				.count();
		// convert microseconds (weird) to seconds (less weird)
		deltaTime *= 0.000001;

		//deltaTime = glm::min(deltaTime, dt);

		// reset lastTime so that we can calculate the deltaTime
		// on the next frame
		lastTime = nextLastTime;

		activeCam->updateCamera(deltaTime);
		// Render scene.
		application->render(deltaTime);

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	application->uninitSoundEngines();
	application->leGUI->Shutdown();
	windowManager->shutdown();
	

	return 0;
}