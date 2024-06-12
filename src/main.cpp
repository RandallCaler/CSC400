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
#include "Animation.h"
#include "Animator.h"

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
float worldSize = 0.5f;

map<string, shared_ptr<Shader>> shaders;
map<string, shared_ptr<Texture>> textureLibrary = { {"", nullptr} };
map<string, shared_ptr<Entity>> worldentities;
vector<string> tagList = { "" };
vector<shared_ptr<Entity>> collidables;
vector<shared_ptr<Entity>> boids;

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
	shared_ptr<Entity> glider;

	ImporterExporter *levelEditor = new ImporterExporter(&shaders, &textureLibrary, &worldentities, &tagList, &collidables, &boids);
	GameManager *gameManager = new GameManager();

	shared_ptr<Program> DepthProg;
	shared_ptr<Program> DepthProgDebug;
	shared_ptr<Program> DebugProg;

	GLuint depthMapFBO;
	const GLuint S_WIDTH = 8192, S_HEIGHT = 8192;
	GLuint depthMap;
	GLuint quad_vertexbuffer;
	 GLuint quad_VertexArrayID;

	vec3 light_vec = normalize(vec3(3, 25, 5));
  
	LevelEditor* leGUI = new LevelEditor();

	InputHandler ih;
	int collisionSounds[1];

	EventManager *eManager = new EventManager();
	// Event *walking = new Event("../resources/music.mp3", &walkingEngine);

	// added region buffer to hold regional data (1, 2, 3 = r, g, b)
	//global data for ground plane - direct load constant defined CPU data to GPU (not obj)
	GLuint GrndBuffObj, GrndRegionBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;
	//ground VAO
	GLuint GroundVertexArrayID;

	double cursor_x = 0;
	double cursor_y = 0;

	const float Y_MAX = 75;
	const float Y_MIN = -Y_MAX;

	int editSRT = 0; // 0 - translation, 1 - rotation, 2 - scale

	vec4 VFCplanes[6];

	// hmap for terrain
	shared_ptr<Texture> hmap;
	vec3 groundPos = vec3(0, 0, 0);

	ma_engine walkingEngine;
	ma_engine collectionEngine;

	shared_ptr<Animation> idle;
	shared_ptr<Animation> walking;
	shared_ptr<Animation> jumping;
	shared_ptr<Animation> gliding;
	shared_ptr<Animation> waving;
	shared_ptr<Animation> floating;
	Animator animator = Animator();
	Animator animator1 = Animator();
		
	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
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
				applyCollider();
			}
		}

		// KEY PRESSED

		if (editMode) {
			if (action == GLFW_PRESS && !leGUI->diableInput) {
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
		// in-game mode
		else {
			if (action == GLFW_PRESS) {
				switch (key) {
					case GLFW_KEY_W:
						ih.inputStates[0] = 1;
						break;
					case GLFW_KEY_A:
						ih.inputStates[1] = 1;
						break;
					case GLFW_KEY_S:
						ih.inputStates[2] = 1;
						break;
					case GLFW_KEY_D:
						ih.inputStates[3] = 1;
						break;
					case GLFW_KEY_SPACE:
						ih.inputStates[4] = 1;
						if (player->grounded){
							if (animator.getCurrentAnimation() == jumping && animator.m_AnimationCompletedOnce) {
								animator.PlayAnimation(jumping);
							}
							else {
								animator.PlayAnimation(jumping);
							}
						}
						break;
					case GLFW_KEY_LEFT_SHIFT:
						ih.inputStates[5] = 1;
						break;
					case GLFW_KEY_C:
						player->sliding = true;
						break;
					case GLFW_KEY_P:
						glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						break;
					case GLFW_KEY_1:
						if (animator.getCurrentAnimation() != waving) {
							animator.PlayAnimation(waving);
						}
						break;
				}
			}
			// KEY RELEASED
			if (action == GLFW_RELEASE) {
				switch (key) {
					case GLFW_KEY_W:
						ih.inputStates[0] = 0;
						break;
					case GLFW_KEY_A:
						ih.inputStates[1] = 0;
						break;
					case GLFW_KEY_S:
						ih.inputStates[2] = 0;
						break;
					case GLFW_KEY_D:
						ih.inputStates[3] = 0;
						break;
					case GLFW_KEY_SPACE:
						ih.inputStates[4] = 0;
						break;
					case GLFW_KEY_LEFT_SHIFT:
						ih.inputStates[5] = 0;
						break;
					case GLFW_KEY_C:
						player->sliding = false;
						break;
					case GLFW_KEY_P:
						glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
						break;
				}
			}

			// Entity *catptr = &catEnt;
			ih.handleInput(player.get(), &cam, deltaTime);
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
		cam.angle -= 10 * (deltaX / 57.296);
		player->rotY -= 10 * (deltaX / 57.296);
	}


	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS) {
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				glfwGetCursorPos(window, &posX, &posY);
				//cout << "Pos X " << posX <<  " Pos Y " << posY << endl;

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
			player->rotY -= 0.001*(x-cursor_x);
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
		//cout << "r: " << +data[0] << "   g: " << +data[1] << "   b: " << +data[2] << endl;

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
				//cout << "active entity is now " << pickedID << endl;
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
		DepthProg->addUniform("M");
		DepthProg->addUniform("Texture0");
		DepthProg->addAttribute("vertPos");
		DepthProg->addAttribute("vertNor");
		DepthProg->addAttribute("vertTex");

		DepthProgDebug = make_shared<Program>();
		DepthProgDebug->setVerbose(true);
		DepthProgDebug->setShaderNames(resourceDirectory + "/depth_vertDebug.glsl", resourceDirectory + "/depth_fragDebug.glsl");
		DepthProgDebug->init();

		DepthProgDebug->addUniform("LP");
		DepthProgDebug->addUniform("LV");
		DepthProgDebug->addUniform("M");
		DepthProgDebug->addAttribute("vertPos");
		//un-needed, better solution to modifying shape
		DepthProgDebug->addAttribute("vertNor");
		DepthProgDebug->addAttribute("vertTex");


		DebugProg = make_shared<Program>();
		DebugProg->setVerbose(true);
		DebugProg->setShaderNames(resourceDirectory + "/pass_vert.glsl", resourceDirectory + "/pass_texfrag.glsl");
		DebugProg->init();

		DebugProg->addUniform("texBuf");
  		DebugProg->addAttribute("vertPos");


		hmap = make_shared<Texture>();
		hmap->setFilename(resourceDirectory + "/hmap.png");
		hmap->initHmap();
		initShadow();

		for (auto ent : worldentities) {
			if (ent.second->tag == "player") {
				player = ent.second;
			}
			if (ent.second->tag == "glider") {
				glider = ent.second;
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
		//initQuad();
	}

	void initAnimation() {
		idle = make_shared<Animation>(player->model, 1);
		jumping = make_shared<Animation>(player->model, 2);
		walking = make_shared<Animation>(player->model, 3);
		waving = make_shared<Animation>(player->model, 4);
		gliding = make_shared<Animation>(player->model, 0);
		floating = make_shared<Animation>(glider->model, 0);
		animator.PlayAnimation(idle);
		animator1.PlayAnimation(floating);
	}

	void applyCollider() {
		for (auto ent : collidables) {
			if (!ent->collider) {
				ent->collider = new Collider(ent.get());
				ent->collider->SetEntityID(ent->id);
				if (ent == player) {
					ent->collider->entityName = 'p';
				}
				else {
					ent->collider->entityName = 'c';
				}
				if (ent->tag == "food") {
					//cout << "SET COLLECTIBLE TAG TO TRUE" << endl;
					ent->collider->collectible = true;
				}
			}	
		}
		if (!cam.collider) {
			cam.collider = new Collider(&cam);
		}
		
	}

	//directly pass quad for the ground to the GPU
	void initHMapGround() {
		vector<float> vertices;
		vector<float> regions;
		auto hmap_dim = hmap->getDim();
		auto hmap_data = hmap->getData();
		for (unsigned int i = 0; i < hmap_dim.second; i++) {
			for (unsigned int j = 0; j < hmap_dim.first; j++) {
				bool pit;
				int hvalr = *(hmap_data + 3 * (i * hmap_dim.first + j));
				int hvalg = *(hmap_data + 3 * (i * hmap_dim.first + j) + 1);
				int hvalb = *(hmap_data + 3 * (i * hmap_dim.first + j) + 2);
				int hvalmax = std::max(hvalr, std::max(hvalg, hvalb));
				float hval = (hvalr + hvalg + hvalb) / (3 * 255.0f);
				//float hval = (std::max)(hvalr, (std::max)(hvalg, hvalb)) / 255.0f;
				
				pit = hval < .01;

				vertices.push_back(worldSize * (j - hmap_dim.first / 2.0f));
				vertices.push_back(worldSize * (hval * (Y_MAX - Y_MIN) + Y_MIN));
				vertices.push_back(worldSize * (i - hmap_dim.second / 2.0f));

				if (pit) {
					regions.push_back(0);
				}
				else if (hvalmax == hvalr) {
					regions.push_back(1);
				}
				else if (hvalmax == hvalg) {
					regions.push_back(2);
				}
				else if (hvalmax == hvalb) {
					regions.push_back(3);
				}
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

		//std::cout << "vert size : " << vertices.size() << " ind size: " << indices.size() << std::endl;

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
			player->collider->SetGround(groundPos, worldSize * vec3(1, Y_MAX - Y_MIN, 1));
		}
		
		cam.collider->SetGround(groundPos, worldSize * vec3(1,Y_MAX-Y_MIN,1));

      }
	
      //code to draw the ground plane
     void drawGround(shared_ptr<Shader> curS) {
     	glBindVertexArray(GroundVertexArrayID);

		//draw the ground plane 
  		curS->setModel(groundPos, 0, 0, 0, 1);

		glUniform1f(curS->prog->getUniform("h_min"), worldSize * Y_MIN);
		glUniform1f(curS->prog->getUniform("h_max"), worldSize * Y_MAX);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, textureLibrary["rock"]->getID());
		glUniform1i(curS->prog->getUniform("terrain0"), 2);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, textureLibrary["sand"]->getID());
		glUniform1i(curS->prog->getUniform("terrain1"), 3);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, textureLibrary["rock"]->getID());
		glUniform1i(curS->prog->getUniform("terrain2"), 4);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, textureLibrary["grass"]->getID());
		glUniform1i(curS->prog->getUniform("terrain3"), 5);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, textureLibrary["rock"]->getID());
		glUniform1i(curS->prog->getUniform("terrain4"), 6);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, textureLibrary["snow"]->getID());
		glUniform1i(curS->prog->getUniform("terrain5"), 7);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, textureLibrary["water"]->getID());
		glUniform1i(curS->prog->getUniform("terrain6"), 8);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, textureLibrary["dudvwater"]->getID());
		glUniform1i(curS->prog->getUniform("DuDvMap"), 9);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  		glEnableVertexAttribArray(0);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
  		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		
		glEnableVertexAttribArray(1);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
  		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		
		glEnableVertexAttribArray(2);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndRegionBuffObj);
  		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
		

   		// draw!
  		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
  		glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_INT, 0);

  		glDisableVertexAttribArray(0);
  		glDisableVertexAttribArray(1);
  		glDisableVertexAttribArray(2);
  		curS->prog->unbind();
     }



	mat4 SetOrthoMatrix(shared_ptr<Program> curShade) {
		mat4 ortho = glm::ortho(-150.0, 150.0, -150.0, 150.0, 10.0, 100.0);

		glUniformMatrix4fv(curShade->getUniform("LP"), 1, GL_FALSE, value_ptr(ortho));
		return ortho;
	}


	mat4 SetLightView(shared_ptr<Program> curShade, vec3 pos, vec3 LA, vec3 up) {
		mat4 Cam = glm::lookAt(pos, LA, up);

		glUniformMatrix4fv(curShade->getUniform("LV"), 1, GL_FALSE, value_ptr(Cam));
		return Cam;
	}

	void ExtractVFPlanes(mat4 P, mat4 V) {
		vec4 Left, Right, Bottom, Top, Near, Far;
		/* composite matrix */
		mat4 comp = P*V;
		vec3 n; //use to pull out normal
		float l; //length of normal for plane normalization

		Left.x = comp[0][3] + comp[0][0]; 
		Left.y = comp[1][3] + comp[1][0]; 
		Left.z = comp[2][3] + comp[2][0];
		Left.w = comp[3][3] + comp[3][0];
		n = vec3(Left.x, Left.y, Left.z);
		l = length(n);
		Left = Left/l;
		VFCplanes[0] = Left;

		Right.x = comp[0][3] - comp[0][0]; // see handout to fill in with values from comp
		Right.y = comp[1][3] - comp[1][0]; // see handout to fill in with values from comp
		Right.z = comp[2][3] - comp[2][0]; // see handout to fill in with values from comp
		Right.w = comp[3][3] - comp[3][0]; // see handout to fill in with values from comp
		n = vec3(Right.x, Right.y, Right.z);
		l = length(n);
		Right = Right/l;
		VFCplanes[1] = Right;

		Bottom.x = comp[0][3] + comp[0][1]; // see handout to fill in with values from comp
		Bottom.y = comp[1][3] + comp[1][1]; // see handout to fill in with values from comp
		Bottom.z = comp[2][3] + comp[2][1]; // see handout to fill in with values from comp
		Bottom.w = comp[3][3] + comp[3][1]; // see handout to fill in with values from comp
		n = vec3(Bottom.x, Bottom.y, Bottom.z);
		l = length(n);
		Bottom = Bottom/l;
		VFCplanes[2] = Bottom;

		Top.x = comp[0][3] - comp[0][1]; // see handout to fill in with values from comp
		Top.y = comp[1][3] - comp[1][1]; // see handout to fill in with values from comp
		Top.z = comp[2][3] - comp[2][1]; // see handout to fill in with values from comp
		Top.w = comp[3][3] - comp[3][1]; // see handout to fill in with values from comp
		n = vec3(Top.x, Top.y, Top.z);
		l = length(n);
		Top = Top/l;
		VFCplanes[3] = Top;

		Near.x = comp[0][3] + comp[0][2]; // see handout to fill in with values from comp
		Near.y = comp[1][3] + comp[1][2]; // see handout to fill in with values from comp
		Near.z = comp[2][3] + comp[2][2]; // see handout to fill in with values from comp
		Near.w = comp[3][3] + comp[3][2]; // see handout to fill in with values from comp
		n = vec3(Near.x, Near.y, Near.z);
		l = length(n);
		Near = Near/l;
		VFCplanes[4] = Near;

		Far.x = comp[0][3] - comp[0][2]; // see handout to fill in with values from comp
		Far.y = comp[1][3] - comp[1][2]; // see handout to fill in with values from comp
		Far.z = comp[2][3] - comp[2][2]; // see handout to fill in with values from comp
		Far.w = comp[3][3] - comp[3][2]; // see handout to fill in with values from comp
		n = vec3(Far.x, Far.y, Far.z);
		l = length(n);
		Far = Far/l;
		VFCplanes[5] = Far;
	}
	
	/* returns whether to cull an entity */
	int ViewFrustCull(shared_ptr<Entity> entity) {
		float dist;

		for (int i = 0; i < 6; i++) {
			dist = dot(VFCplanes[i], vec4(entity->position, 1.0));
			//test against each plane
			float radius = std::max(entity->scaleVec.x, std::max(entity->scaleVec.y, entity->scaleVec.z));
			if (dist < radius * -1.0f) {
				return 1;
			}
		}
		return 0;
	}

	void drawShadowMap(mat4 LSpace) {
		auto Model = make_shared<MatrixStack>();

		map<string, shared_ptr<Entity>>::iterator i;

		for (i = worldentities.begin(); i != worldentities.end(); i++) {
			shared_ptr<Entity> entity = i->second;
			entity->generateModel();
		}
    
		for (i = worldentities.begin(); i != worldentities.end(); i++) {
			shared_ptr<Entity> entity = i->second;
			if (entity != worldentities["skybox"]) {
				mat4 M = entity->modelMatrix;
				if (entity == player) {
					// penguin animation does not center penguin on its model matrix
					// when drawing shadows, fake penguin position so that the penguin is in front
					// of surfaces it walks on
					M = translate(M, vec3(2) * light_vec);
				}
				glUniformMatrix4fv(DepthProg->getUniform("M"), 1, GL_FALSE, value_ptr(M));
				entity->model->Draw(DepthProg);
			}
		}
		
     	glBindVertexArray(GroundVertexArrayID);

		//draw the ground plane 
  		mat4 ctm = glm::mat4(1.0f);
  		glUniformMatrix4fv(DepthProg->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
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
	}


	void drawObjects(float aspect, mat4 LSpace, float deltaTime) {
		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();


		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 350.0f);
		mat4 P = Projection->topMatrix();
		
		//material shader first
		shared_ptr<Shader> curS = shaders["reg"];
		curS->prog->bind();

		glUniformMatrix4fv(curS->prog->getUniform("P"), 1, GL_FALSE, value_ptr(P));
		mat4 V = activeCam->SetView(curS->prog, hmap);
    	//only extract the planes for the game camaera
    	ExtractVFPlanes(P, V);

		// directional light
  		glActiveTexture(GL_TEXTURE1);
  		glBindTexture(GL_TEXTURE_2D, depthMap);
		glUniform3f(curS->prog->getUniform("lightDir"), light_vec.x, light_vec.y, light_vec.z);
		glUniform1i(curS->prog->getUniform("shadowDepth"), 1);
      	glUniformMatrix4fv(curS->prog->getUniform("LS"), 1, GL_FALSE, value_ptr(LSpace));


		// BRDFmaterial imported from save file
		shaders["skybox"]->prog->setVerbose(false);
		map<string, shared_ptr<Entity>>::iterator i;

		for (i = worldentities.begin(); i != worldentities.end(); i++) {
			shared_ptr<Entity> entity = i->second;
			entity->generateModel();
		}
    
		for (i = worldentities.begin(); i != worldentities.end(); i++) {
			shared_ptr<Entity> entity = i->second;

			if (entity->collider) {
				for (int i = 0; i < collidables.size(); i++) {
					if (collidables[i]->collider->boided) {
						collidables.erase(collidables.begin() + i);
					}
				}
				if (entity->collider->collectible){
					entity->updateBoids(deltaTime, hmap, boids, player);
					// cout << "BOIDED: " << entity->collider->boided << endl;
				}
			}
	
			if (entity == player) {
				player->updateMotion(deltaTime, hmap, collidables, collisionSounds);
			}

			if (shaders[entity->defaultShaderName] != curS) {
				curS->prog->unbind();
				curS = shaders[entity->defaultShaderName];
				curS->prog->bind();
				glUniformMatrix4fv(curS->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
				activeCam->SetView(curS->prog, hmap);
			}	

			if (shaders["animate"] == curS) {
				animator.UpdateAnimation(deltaTime);
				auto transforms = animator.GetFinalBoneMatrices();
				GLuint baseLocation = curS->prog->getUniform("finalBonesMatrices");
				for (int i = 0; i < transforms.size(); ++i) {
					glUniformMatrix4fv(baseLocation + i, 1, GL_FALSE, value_ptr(transforms[i]));
				}
			}
			else if (shaders["animate1"] == curS) {
				if (player->gliding) {
					entity->position = player->position;
					entity->position.y += 0.85;
					entity->rotY = player->rotY;
				}
				else {
					glider->position = vec3(0, 100, 0);
				}			
				animator1.UpdateAnimation(deltaTime);
				auto transforms = animator1.GetFinalBoneMatrices();
				GLuint baseLocation = curS->prog->getUniform("finalBonesMatrices");
				for (int i = 0; i < transforms.size(); ++i) {
					glUniformMatrix4fv(baseLocation + i, 1, GL_FALSE, value_ptr(transforms[i]));
				}
			}

			// do not render objects not in frame
			if (entity->defaultShaderName != "skybox" && ViewFrustCull(entity)) {
				continue;
			}

			if (shaders["skybox"] == curS) {
				glDisable(GL_CULL_FACE);
				entity->position = player->position;
				// skybox is always the furthest surface away
				glDepthFunc(GL_LEQUAL);
			}
			else {	
				glUniform3f(curS->prog->getUniform("lightDir"), light_vec.x, light_vec.y, light_vec.z);
				glUniform1i(curS->prog->getUniform("shadowDepth"), 1);
				glUniformMatrix4fv(curS->prog->getUniform("LS"), 1, GL_FALSE, value_ptr(LSpace));
			}

			glUniformMatrix4fv(curS->prog->getUniform("M"), 1, GL_FALSE, value_ptr(entity->modelMatrix));

			for (auto& meshPair : entity->model->meshes) {
				if (curS == shaders["reg"] || curS == shaders["platform"]) {
					curS->setMaterial(meshPair.second.mat);
				}
				meshPair.second.Draw(curS->prog);
			}
			
			if (shaders["skybox"] == curS) {
				// deactivate skybox backfill
				glEnable(GL_CULL_FACE);
				glDepthFunc(GL_LESS);
			}
		}
		
		curS->prog->unbind();

		curS = shaders["hmap"];

		curS->prog->bind();
		glUniformMatrix4fv(curS->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		activeCam->SetView(curS->prog, hmap);
  		glActiveTexture(GL_TEXTURE1);
  		glBindTexture(GL_TEXTURE_2D, depthMap);
		glUniform3f(curS->prog->getUniform("lightDir"), light_vec.x, light_vec.y, light_vec.z);
		glUniform1i(curS->prog->getUniform("shadowDepth"), 1);
		glUniform1f(curS->prog->getUniform("fTime"), glfwGetTime());
      	glUniformMatrix4fv(curS->prog->getUniform("LS"), 1, GL_FALSE, value_ptr(LSpace));
		drawGround(curS);  //draw ground here

		// Pop matrix stacks.
		Projection->popMatrix();
	}

	void drawEditorObjects(float aspect, mat4 LSpace, float deltaTime) {
	
		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();


		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 350.0f);

		//material shader first
		shared_ptr<Shader> curS = shaders["edit"];
		curS->prog->bind();
		glUniformMatrix4fv(curS->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		activeCam->SetView(curS->prog, hmap);

		// BRDFmaterial imported from save file
		map<string, shared_ptr<Entity>>::iterator i;

		for (i = worldentities.begin(); i != worldentities.end(); i++) {
			shared_ptr<Entity> entity = i->second;
			entity->generateModel();
		}
		for (i = worldentities.begin(); i != worldentities.end(); i++) {
			shared_ptr<Entity> entity = i->second;
	
			glUniformMatrix4fv(curS->prog->getUniform("M"), 1, GL_FALSE, value_ptr(entity->modelMatrix));
			glUniform3fv(curS->prog->getUniform("PickingColor"), 1, value_ptr(glm::vec3(entity->editorColor.r, entity->editorColor.g, entity->editorColor.b)));
			entity->model->Draw(curS->prog);
		}
	
		curS->prog->unbind();
		curS = shaders["hmap"];

		curS->prog->bind();
		glUniformMatrix4fv(curS->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		activeCam->SetView(curS->prog, hmap);
  		glActiveTexture(GL_TEXTURE1);
  		glBindTexture(GL_TEXTURE_2D, depthMap);
		glUniform3f(curS->prog->getUniform("lightDir"), light_vec.x, light_vec.y, light_vec.z);
		glUniform1i(curS->prog->getUniform("shadowDepth"), 1);
      	glUniformMatrix4fv(curS->prog->getUniform("LS"), 1, GL_FALSE, value_ptr(LSpace));
		drawGround(curS);  //draw ground here

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
			//cout <<  "collection event triggered, starting sound: " << collisionSounds[0] <<eManager->eventHistory->at("collection") << endl;
			eManager->triggerSound("collection");
		
			// eManager->eventHistory->at("collection") == true;
			
		}
		else {
			eManager->stoppingSound("collection");
			collisionSounds[0] = 0;
		}


	}

	void updateAnimation() {
		if (player->grounded) {
			if (player->m.curSpeed != 0) {
				if (animator.getCurrentAnimation() != walking) {
					animator.PlayAnimation(walking);			
				}
			}
			else {
				if (animator.getCurrentAnimation() != idle && animator.getCurrentAnimation() != waving) {
					animator.PlayAnimation(idle);
				}			
			}
		}
		else {
			if (player->gliding) {
				if (animator.getCurrentAnimation() != gliding) {
					animator.PlayAnimation(gliding);
				}
			}
		}
	}

	void render(float frametime) {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

		vec3 lightLA = vec3(0.0);
    	vec3 lightUp = vec3(0, 1, 0);
		mat4 LO, LV, LSpace;

		glViewport(0, 0, S_WIDTH, S_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);

		DepthProg->bind();
		//TODO you will need to fix these
		LO = SetOrthoMatrix(DepthProg);
		LV = SetLightView(DepthProg, player->position + vec3(50) * light_vec, player->position, lightUp);
		LSpace = LO*LV;
		drawShadowMap(LSpace);
		DepthProg->unbind();
		glCullFace(GL_BACK);
		// cout << "1 pass" << endl;

		//this sets the output back to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// cout << "before" << endl;
		

		glViewport(0, 0, width, height);
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		if (player) {
			cam.player_pos = player->position;
		}
	
		float aspect = width/(float)height;
		if(editMode){
			drawEditorObjects(aspect, LSpace, frametime);
		}
		else{
			updateAnimation();
			drawObjects(aspect, LSpace, frametime);
		}

		
		checkSounds();
	}

};

int main(int argc, char *argv[]) {
	// Where the resources are loaded from
	std::string resourceDir = "../resources";
	int frameCount = 0;
	float fps = 0.0f;

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
	application->init(resourceDir);
	application->initGeom(resourceDir);
	application->initSoundEngines();
	application->initAnimation();

	application->gameManager->init(application->player, worldentities);

	application->gameManager->init(application->player, worldentities);

	for (int i = 0; i < boids.size(); i++){
		//cout << "boids in population" << endl;
		//cout << (boids[i]->id) << " BOID " << (boids[i]->collider->collectible) << endl;
	}

	Event *ev = new Event("../resources/french-mood.mp3", &engine, true, "background");
	ev->startSound();

	float dt = 1 / 60.0;
	auto lastTime = chrono::high_resolution_clock::now();
	auto start = lastTime;

	application->leGUI->Init(windowManager->getHandle());
	float totalTime = 0.0;

	glEnable(GL_CULL_FACE);
	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// save current time for next frame
		auto nextLastTime = chrono::high_resolution_clock::now();

		// get time since last frame
		frameCount++;
		deltaTime = 
			chrono::duration_cast<std::chrono::microseconds>(
				nextLastTime - lastTime)
				.count();
		totalTime += deltaTime;
		// convert microseconds (weird) to seconds (less weird)
		if (totalTime > 1000000) {
			fps = frameCount / (totalTime / 1000000);
			frameCount = 0;
			totalTime = 0.0;
			//std::cout << "FPS: " << static_cast<int>(fps) << std::endl;
		}
		deltaTime *= 0.000001;

		// reset lastTime so that we can calculate the deltaTime
		// on the next frame
		lastTime = nextLastTime;

		activeCam->updateCamera(deltaTime);
		application->gameManager->update();
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