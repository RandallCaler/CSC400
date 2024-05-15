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

#include <chrono>
#include <array>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>
#define PI 3.1415927
#define EPSILON 0.0001

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

enum ACTIVE_COLOR {
	ERR = -1,
	R,
	G,
	B
};

// Where the resources are loaded from
std::string resourceDir = "../resources";
std::string WORLD_FILE_NAME = "/world.txt";

map<string, shared_ptr<Shader>> shaders;
map<string, shared_ptr<Entity>> worldentities;

float deltaTime;
// 	view pitch dist angle playerpos playerrot animate g_eye
Camera cam = Camera(vec3(0, 0, 1), 17, 4, 0, vec3(0, -1.12, 0), 0, vec3(0, 0.5, 5));
Camera freeCam = Camera(vec3(0, 0, 1), 17, 4, 0, vec3(0, -1.12, 0), 0, vec3(0, 0.5, 5), true);
Camera* activeCam = &cam;

class Application : public EventCallbacks
{

public:
	WindowManager * windowManager = nullptr;
	// Our shader program - use this one for Blinn-Phong has diffuse
	shared_ptr<Shader> reg;
	shared_ptr<Shader> proghmap;
	//Our shader program for textures
	// shared_ptr<Shader> tex;

	bool editMode = false;

	ImporterExporter *levelEditor = new ImporterExporter(&shaders, &worldentities);

	std::vector<shared_ptr<Shape>> butterfly;

	InputHandler ih;

	Entity bf1 = Entity();
	Entity bf2 = Entity();
	Entity bf3 = Entity();
	// Entity *catEnt = new Manchot();
	
  	std::vector<Entity> bf;

	std::vector<shared_ptr<Shape>> flower;

	std::vector<shared_ptr<Shape>> tree1;
	
	std::vector<shared_ptr<Shape>> cat;
	
	std::vector<Entity> trees;

	std::vector<Entity> flowers;

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

	// temp variables, should be incorporated into controller
	map<string, shared_ptr<Entity>>::iterator activeEntity;
	float editSpeed = 2.0;
	int editSRT = 0; // 0 - translation, 1 - rotation, 2 - scale
	vec3 mobileVel = vec3(0);

	// hmap for terrain
	shared_ptr<Texture> hmap;
	
	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_CAPS_LOCK && action == GLFW_PRESS) {
			editMode = !editMode;
			editSRT = 0;
			editSpeed = 2.0;
			if (editMode) {
				activeCam = &freeCam;
			}
			else {
				activeCam = &cam;
			}
		}
		// KEY PRESSED

		if (editMode) {
			if (action == GLFW_PRESS) {
				switch (key) {
					case GLFW_KEY_TAB:
						activeEntity++;
						if (activeEntity == worldentities.end()) {
							activeEntity = worldentities.begin();
						}
						break;
					case GLFW_KEY_UP:
						mobileVel.z = editSpeed;
						break;
					case GLFW_KEY_DOWN:
						mobileVel.z = -editSpeed;
						break;
					case GLFW_KEY_LEFT:
						mobileVel.x = editSpeed;
						break;
					case GLFW_KEY_RIGHT:
						mobileVel.x = -editSpeed;
						break;
					case GLFW_KEY_E:
						mobileVel.y = editSpeed;
						break;
					case GLFW_KEY_Q:
						mobileVel.y = -editSpeed;
						break;
					case GLFW_KEY_Z:
						editSRT = 2;
						break;
					case GLFW_KEY_X:
						editSRT = 1;
						break;
					case GLFW_KEY_C:
						editSRT = 0;
						break;
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
					case GLFW_KEY_LEFT_SHIFT:
						freeCam.vel.y = editSpeed;
						break;
					case GLFW_KEY_V:
						levelEditor->saveToFile(WORLD_FILE_NAME);
				}
			}
			if (action == GLFW_RELEASE) {
				switch (key) {
					case GLFW_KEY_UP:
					case GLFW_KEY_DOWN:
						mobileVel.z = 0.0;
						break;
					case GLFW_KEY_LEFT:
					case GLFW_KEY_RIGHT:
						mobileVel.x = 0.0;
						break;
					case GLFW_KEY_E:
					case GLFW_KEY_Q:
						mobileVel.y = 0.0;
						break;
					case GLFW_KEY_W:
					case GLFW_KEY_S:
						freeCam.vel.z = 0.0;
						break;
					case GLFW_KEY_A:
					case GLFW_KEY_D:
						freeCam.vel.x = 0.0;
						break;
					case GLFW_KEY_SPACE:
					case GLFW_KEY_LEFT_SHIFT:
						freeCam.vel.y = 0.0;
						break;
				}
			}
		}
		else {
			if (key == GLFW_KEY_W && (action == GLFW_PRESS) && !worldentities["bunny"]->collider->IsColliding() && bounds < 1000){
				ih.inputStates[0] = 1;
			}

			if (key == GLFW_KEY_A && (action == GLFW_PRESS) && !worldentities["bunny"]->collider->IsColliding()){
				ih.inputStates[1] = 1;
			}

			if (key == GLFW_KEY_S && (action == GLFW_PRESS) && bounds < 1000){	
				ih.inputStates[2] = 1;
			}

			if (key == GLFW_KEY_D && (action == GLFW_PRESS) && !worldentities["bunny"]->collider->IsColliding()){
				ih.inputStates[3] = 1;
			}

			if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS)){
				ih.inputStates[4] = 1;
			}
	
			if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}

			// KEY RELEASED

			if (key == GLFW_KEY_W && (action == GLFW_RELEASE) && !worldentities["bunny"]->collider->IsColliding() && bounds < 1000){
				ih.inputStates[0] = 0;
			}

			if (key == GLFW_KEY_A && (action == GLFW_RELEASE) && !worldentities["bunny"]->collider->IsColliding()){
				ih.inputStates[1] = 0;
			}

			if (key == GLFW_KEY_S && (action == GLFW_RELEASE) && bounds < 1000){
				ih.inputStates[2] = 0;
			}

			if (key == GLFW_KEY_D && (action == GLFW_RELEASE) && !worldentities["bunny"]->collider->IsColliding()){
				ih.inputStates[3] = 0;
			}

			if (key == GLFW_KEY_SPACE && (action == GLFW_RELEASE)){
				ih.inputStates[4] = 0;
			}
			
			if (key == GLFW_KEY_F1 && action == GLFW_RELEASE) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			// Entity *catptr = &catEnt;
			ih.handleInput(worldentities["bunny"].get(), &cam, deltaTime);
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
		if (editMode) {

			// cout << "INSIDE SCROLL CALLBACK BUNNY MOVEMENT" << endl;
			//cout << "xDel + yDel " << deltaX << " " << deltaY << endl;
			cam.angle -= 10 * (deltaX / 57.296);
			// ih.setRotation(worldentities["bunny"].get(), -10 * (deltaX / 57.296));

		}
		else {
			cam.angle -= 10 * (deltaX / 57.296);
			// ih.setRotation(worldentities["bunny"].get(), -10 * (deltaX / 57.296));

		}

	}


	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS) {
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				glfwGetCursorPos(window, &posX, &posY);
				cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
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
  }

#pragma endregion

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.72f, .84f, 1.06f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		// glEnable(GL_CLIP_DISTANCE0);

		shaders["skybox"]->has_texture = true;
		shaders["tex"]->has_texture = true;
		shaders["hmap"]->has_texture = false;

		shaders["skybox"]->addTexture(resourceDirectory + "/sky.jpg");
		shaders["tex"]->addTexture(resourceDirectory + "/grass_tex.jpg");
		shaders["tex"]->addTexture(resourceDirectory + "/sky.jpg");
		shaders["tex"]->addTexture(resourceDirectory + "/cat_tex.jpg");
		shaders["tex"]->addTexture(resourceDirectory + "/cat_tex_legs.jpg");

		hmap = make_shared<Texture>();
		hmap->setFilename(resourceDirectory + "/hmap.png");
		hmap->initHmap();
	}

	void initGeom(const std::string& resourceDirectory)
	{
		string errStr;
		// Initialize cat mesh.
		vector<tinyobj::shape_t> TOshapesB;
 		vector<tinyobj::material_t> objMaterialsB;
		//load in the mesh and make the shape(s)
 		bool rc = tinyobj::LoadObj(TOshapesB, objMaterialsB, errStr, (resourceDirectory + "/cat.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {	
			cat.push_back(make_shared<Shape>());
			cat[0]->createShape(TOshapesB[0]);
			cat[0]->measure();
			cat[0]->init();
		}

		vector<tinyobj::shape_t> TOshapes3;
		rc = tinyobj::LoadObj(TOshapes3, objMaterialsB, errStr, (resourceDirectory + "/butterfly.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			//scan in all parts of butterfly
			for (int i = 0; i < 3; i++) {
				butterfly.push_back(make_shared<Shape>());
				butterfly[i]->createShape(TOshapes3[i]);
				butterfly[i]->measure();
				butterfly[i]->init();
			}
		}

		vector<tinyobj::shape_t> TOshapes4;
		rc = tinyobj::LoadObj(TOshapes4, objMaterialsB, errStr, (resourceDirectory + "/flower.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			//for now all our shapes will not have textures - change in later labs
			for (int i = 0; i < 3; i++) {
				//scan in current obj part of flower
				flower.push_back(make_shared<Shape>());
				flower[i]->createShape(TOshapes4[i]);
				flower[i]->measure();
				flower[i]->init();
			}
		}

		vector<tinyobj::shape_t> TOshapes5;
		rc = tinyobj::LoadObj(TOshapes5, objMaterialsB, errStr, (resourceDirectory + "/trees.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			//for now all our shapes will not have textures - change in later labs
			for (int i = 0; i < 12; i++) {
				//scan in current obj part of flower
				tree1.push_back(make_shared<Shape>());

				tree1[i]->createShape(TOshapes5[i]);
				tree1[i]->measure();
				tree1[i]->init();
			}
		}
    
		// IMPORT BUNNY
		worldentities["bunny"]->m.forward = vec4(0, 0, 0.1, 1);
		worldentities["bunny"]->m.velocity = vec3(0.1) * vec3(worldentities["bunny"]->m.forward);

		worldentities["bunny"]->collider = new Collider(worldentities["bunny"].get());
		worldentities["bunny"]->collider->SetEntityID(worldentities["bunny"]->id);
		//cout << "cat " << worldentities["bunny"]->id << endl;
		worldentities["bunny"]->collider->entityName = 'c';

		//code to load in the ground plane (CPU defined data passed to GPU)
		initHMapGround();
	}

	//directly pass quad for the ground to the GPU
	void initHMapGround() {
		const float Y_MAX = 5;
		const float Y_MIN = -Y_MAX;

		vector<float> vertices;
		vector<float> regions;

		auto hmap_dim = hmap->getDim();
		auto hmap_data = hmap->getData();
		for (unsigned int i = 0; i < hmap_dim.second; i++) {
			for (unsigned int j = 0; j < hmap_dim.first; j++) {
				unsigned char hvalr = *(hmap_data + 3 * (i * hmap_dim.first + j));
				unsigned char hvalg = *(hmap_data + 3 * (i * hmap_dim.first + j) + 1);
				unsigned char hvalb = *(hmap_data + 3 * (i * hmap_dim.first + j) + 2);
				float hval = (hvalr + hvalg + hvalb) / (3 * 255.0f);

				vertices.push_back(j - hmap_dim.first / 2.0f);
				vertices.push_back((hval / 255.0f) * (Y_MAX - Y_MIN) + Y_MIN);
				vertices.push_back(i - hmap_dim.second / 2.0f);

				regions.push_back(hvalr / 255.0f);
				regions.push_back(hvalg / 255.0f);
				regions.push_back(hvalb / 255.0f);
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
				indices.push_back(v3);
				indices.push_back(v1);
				indices.push_back(v3);
				indices.push_back(v2);
				indices.push_back(v1);
			}
		}

		std::cout << "vert size : " << vertices.size() << " ind size: " << indices.size() << std::endl;

		//generate the ground VAO
      	glGenVertexArrays(1, &GroundVertexArrayID);
      	glBindVertexArray(GroundVertexArrayID);

      	glGenBuffers(1, &GrndBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

		// VBO for regional data
		glGenBuffers(1, &GrndRegionBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, GrndRegionBuffObj);
		glBufferData(GL_ARRAY_BUFFER, regions.size() * sizeof(float), regions.data(), GL_STATIC_DRAW);
		
		glGenBuffers(1, &GIndxBuffObj);
     	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
      	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		g_GiboLen = indices.size();

		worldentities["bunny"]->collider->SetGround(vec3(0,-2.5,0), vec3(1,10,1));
      }
	
      //code to draw the ground plane
     void drawGround(shared_ptr<Shader> curS) {
     	glBindVertexArray(GroundVertexArrayID);

		//draw the ground plane 
  		curS->setModel(vec3(0, -2.5f, 0), 0, 0, 0, 1);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, GrndRegionBuffObj);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

   		// draw!
  		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
  		glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_INT, 0);

  		glDisableVertexAttribArray(0);
  		curS->prog->unbind();
     }

	void render(float frametime) {
		// Get current frame buffer size.
		int width, height;
		shared_ptr<Shader> curS = shaders["reg"];
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();


		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 1000.0f);

		// editor mode updates
		if (editMode) {
			switch (editSRT) {
				case 0:
					activeEntity->second->position += mobileVel * frametime;
					break;
				case 1:
					activeEntity->second->rotX += mobileVel.x * frametime;
					activeEntity->second->rotY += mobileVel.y * frametime;
					activeEntity->second->rotZ += mobileVel.z * frametime;
					break;
				case 2:
					activeEntity->second->scale += mobileVel.x * frametime;
					activeEntity->second->scaleVec += mobileVel * frametime;
					break;
			}
		}

		// updates player motion
		worldentities["bunny"]->updateMotion(frametime, hmap);
		cam.player_pos = worldentities["bunny"]->position;
		
		//material shader first
		curS->prog->bind();
		glUniformMatrix4fv(curS->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		activeCam->SetView(curS->prog);

		// directional light
		glUniform3f(curS->prog->getUniform("lightDir"), -1.0f, 1.0f, -1.0f);

		float butterfly_height[3] = {1.1, 1.7, 1.5};

		vec3 butterfly_loc[3];
		butterfly_loc[0] = vec3(-2.3, -1, 3);
		butterfly_loc[1] = vec3(-2, -1.2, -3);
		butterfly_loc[2] = vec3(4, -1, 4);
 

		vector<shared_ptr<Entity>> tempCollisionList = {worldentities["butterfly1"], worldentities["bunny"]};

		// material imported from save file
		shaders["skybox"]->prog->setVerbose(false);
		map<string, shared_ptr<Entity>>::iterator i;

		for (i = worldentities.begin(); i != worldentities.end(); i++) {
			shared_ptr<Entity> entity = i->second;
			entity->generateModel();
		}
		// for (i = worldentities.begin(); i != worldentities.end(); i++) {
		// 	shared_ptr<Entity> entity = i->second;
		// 	if (entity->collider) {
		// 		entity->collider->CalculateBoundingBox(entity->modelMatrix);
		// 	}
		// }

		for (i = worldentities.begin(); i != worldentities.end(); i++) {
			shared_ptr<Entity> entity = i->second;
			if (shaders[entity->defaultShaderName] != curS) {
				curS->prog->unbind();
				curS = shaders[entity->defaultShaderName];
				curS->prog->bind();
				glUniformMatrix4fv(curS->prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
				activeCam->SetView(curS->prog);
			}
			if (shaders["skybox"] == curS) {
				entity->position = activeCam->cameraPos;
				// skybox is always the furthest surface away
				glDepthFunc(GL_LEQUAL);
			}

			// if (entity->collider) {
			// 	int col =entity->collider->CheckCollision(tempCollisionList);
			// 	if (col == -1) {
			// 		entity->updateMotion(frametime);
			// 	}
			// 	else {
			// 		printf("entity %u colliding with %u\n", entity->id, col);
			// 	}
			// }
			
			glUniformMatrix4fv(curS->prog->getUniform("M"), 1, GL_FALSE, value_ptr(entity->modelMatrix));
			// curS->setModel(*entity);
			for (int i = 0; i < entity->objs.size(); i++) {
				if (curS->has_texture) {
					curS->flip(1);
        			entity->textures[i]->bind(curS->prog->getUniform("Texture0"));
				}
				curS->setMaterial(entity->materials[i]);
				entity->objs[i]->draw(curS->prog);
				if (curS->has_texture) {
    				entity->textures[i]->unbind();
					curS->unbindTexture(0);
				}
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
		activeCam->SetView(curS->prog);
		drawGround(curS);  //draw ground here


		// int collided = worldentities["bunny"]->collider->CheckCollision(tempCollisionList);


		bounds = std::sqrt(   //update cat's distance from skybox
			cam.player_pos[0] * cam.player_pos[0]
			+ cam.player_pos[2] * cam.player_pos[2]
		);

		// Pop matrix stacks.
		Projection->popMatrix();
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

	application->init(resourceDir);
	application->initGeom(resourceDir);

	float dt = 1 / 60.0;
	auto lastTime = chrono::high_resolution_clock::now();
	application->activeEntity = worldentities.begin();

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

	windowManager->shutdown();

	return 0;
}