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
#include "Camera.h"

#include <chrono>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>
#define PI 3.1415927

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;


class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program - use this one for Blinn-Phong has diffuse


	Shader reg;           // 

	//Our shader program for textures
	Shader tex;

	//our geometry
	shared_ptr<Shape> sphere;

	std::vector<shared_ptr<Shape>> bunny;

	std::vector<shared_ptr<Shape>> butterfly;

	InputHandler ih;


	Entity bf1 = Entity();
	Entity bf2 = Entity();
	Entity bf3 = Entity();
	Entity *catEnt = new PhysicalObject();
	
  	std::vector<Entity> bf;

	std::vector<shared_ptr<Shape>> flower;

	std::vector<shared_ptr<Shape>> tree1;
	
	std::vector<shared_ptr<Shape>> cat;

	std::vector<Entity> gameObjects;
	
	std::vector<Entity> trees;

	std::vector<Entity> flowers;

	int bf_flags[3] = {0, 0, 0};


	int nextID = 0;

	//global data for ground plane - direct load constant defined CPU data to GPU (not obj)
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;
	//ground VAO
	GLuint GroundVertexArrayID;

	//the image to use as a texture (ground)
	shared_ptr<Texture> texture0;
	shared_ptr<Texture> texture1;	
	shared_ptr<Texture> texture2;
	shared_ptr<Texture> texture3;

	vec3 strafe = vec3(1, 0, 0);

	// 	view pitch dist angle playerpos playerrot animate g_eye
	Camera cam = Camera(vec3(0, 0, 1), 17, 4, 0, vec3(0, -1.12, 0), 0, vec3(0, 0.5, 5));

	//bounds for world
	double bounds;
	
	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		// KEY PRESSED

		if (key == GLFW_KEY_W && (action == GLFW_PRESS) && !catEnt.collider->IsColliding() && bounds < 19){
			ih.inputStates[0] = 1;
		}

		if (key == GLFW_KEY_A && (action == GLFW_PRESS) && !catEnt.collider->IsColliding()){
			ih.inputStates[1] = 1;
		}

		if (key == GLFW_KEY_S && (action == GLFW_PRESS) && bounds < 19){	
			ih.inputStates[2] = 1;
		}

		if (key == GLFW_KEY_D && (action == GLFW_PRESS)&& !catEnt.collider->IsColliding()){
			ih.inputStates[3] = 1;
		}

		if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS)){
			ih.inputStates[4] = 1;
		}

		// KEY RELEASED

		if (key == GLFW_KEY_W && (action == GLFW_RELEASE) && !catEnt.collider->IsColliding() && bounds < 19){
			ih.inputStates[0] = 0;
		}

		if (key == GLFW_KEY_A && (action == GLFW_RELEASE) && !catEnt.collider->IsColliding()){
			ih.inputStates[1] = 0;
		}

		if (key == GLFW_KEY_S && (action == GLFW_RELEASE) && bounds < 19){	
			ih.inputStates[2] = 0;
		}

		if (key == GLFW_KEY_D && (action == GLFW_RELEASE)&& !catEnt.collider->IsColliding()){
			ih.inputStates[3] = 0;
		}

		if (key == GLFW_KEY_SPACE && (action == GLFW_RELEASE)){
			ih.inputStates[4] = 0;
		}
	
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		// Entity *catptr = &catEnt;
		ih.handleInput(&catEnt, &cam);
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
		//cout << "xDel + yDel " << deltaX << " " << deltaY << endl;
		cam.angle -= 10 * (deltaX / 57.296);

		// cat entity updated with camera
		catEnt.m.forward = vec4(glm::normalize(cam.player_pos - cam.g_eye), 1);
		catEnt.m.forward.y = 0;
		catEnt.rotate -= 10 * (deltaX / 57.296);
		
	}


	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			//cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}

	void resizeCallback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}
#pragma endregion

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.72f, .84f, 1.06f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		reg = Shader(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl", false);
		tex = Shader(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag0.glsl", true);

		tex.addTexture(resourceDirectory + "/grass_tex.jpg");
		tex.addTexture(resourceDirectory + "/sky.jpg");
		tex.addTexture(resourceDirectory + "/cat_tex.jpg");
		tex.addTexture(resourceDirectory + "/cat_tex_legs.jpg");	
    
	}

	void initGeom(const std::string& resourceDirectory)
	{
		//EXAMPLE set up to read one shape from one obj file - convert to read several
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;
		//load in the mesh and make the shape(s)
 		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/sphereWTex.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			sphere = make_shared<Shape>();
			sphere->createShape(TOshapes[0]);
			sphere->measure();
			sphere->init();
		}

		// Initialize cat mesh.
		vector<tinyobj::shape_t> TOshapesB;
 		vector<tinyobj::material_t> objMaterialsB;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapesB, objMaterialsB, errStr, (resourceDirectory + "/cat.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {	
			cat.push_back(make_shared<Shape>());
			cat[0]->createShape(TOshapesB[0]);
			cat[0]->measure();
			cat[0]->init();
		}


		vector<tinyobj::shape_t> TOshapesC;
 		vector<tinyobj::material_t> objMaterialsC;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapesC, objMaterialsC, errStr, (resourceDirectory + "/bunny.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {	
			bunny.push_back(make_shared<Shape>());
			bunny[0]->createShape(TOshapesC[0]);
			bunny[0]->measure();
			bunny[0]->init();
		}

		vector<tinyobj::shape_t> TOshapes3;
		rc = tinyobj::LoadObj(TOshapes3, objMaterials, errStr, (resourceDirectory + "/butterfly.obj").c_str());
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
		rc = tinyobj::LoadObj(TOshapes4, objMaterials, errStr, (resourceDirectory + "/flower.obj").c_str());
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
		rc = tinyobj::LoadObj(TOshapes5, objMaterials, errStr, (resourceDirectory + "/trees.obj").c_str());
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

		// init butterfly 1
		bf1.initEntity(butterfly);
		bf1.position = vec3(2, -0.3, -1);
		bf1.m.forward = vec3(1, 0, 0);
		bf1.m.velocity = vec3(2, 0, 2);
		bf1.collider = new Collider(&bf1);
		bf1.collider->SetEntityID(bf1.id);
		//cout << "butterfly 1 " << bf1.id << endl;
		bf1.collider->entityName = 'b';
		bf1.scale = 0.01;
    
    	// init butterfly 2
		bf2.initEntity(butterfly);
		bf2.position = vec3(-2, -0.3, 0.5);
		bf2.m.forward = vec3(1, 0, 0);
		bf2.m.velocity = vec3(.40, 0, .40);
		bf2.collider = new Collider(&bf2);
		bf2.collider->SetEntityID(bf2.id);
		//cout << "butterfly 2 " << bf2.id << endl;
		bf2.collider->entityName = 'b';
		
		bf2.scale = 0.01;

    
   		 // init butterfly 3
		bf3.initEntity(butterfly);
		bf3.position = vec3(4, -0.3, 0.5);
		bf3.m.forward = vec3(1, 0, 0);
		bf3.m.velocity = vec3(.20, 0, .20);
		bf3.collider = new Collider(&bf3);
		bf3.collider->SetEntityID(bf3.id);
		//cout << "butterfly 3 " << bf3.id << endl;
		bf3.collider->entityName = 'b';
	
		bf3.scale = 0.01;



		// init cat entity
		catEnt.initEntity(bunny);
		catEnt.position = vec3(0, -1, 0);
		catEnt.m.forward = vec4(0, 0, 0.1, 1);
		catEnt.m.velocity = 0.1;
		catEnt.scale = 5.0;
		catEnt.rotate = 0.0;
		//catEnt.position = cam.player_pos;
		//cout << catEnt.position.x << ", " << catEnt.position.y << ", " << catEnt.position.z << endl;
		// set forward
		// set velocity
		catEnt->collider = new Collider(catEnt);
		catEnt->collider->SetEntityID(catEnt->id);
		gameObjects.push_back(*dynamic_cast<Entity *>(catEnt));
		
		//cout << "cat " << catEnt.id << endl;
		catEnt.collider->entityName = 'c';

    	bf.push_back(bf1);
		bf.push_back(bf2);
		bf.push_back(bf3);
		gameObjects.push_back(bf1);
		gameObjects.push_back(bf2);
		gameObjects.push_back(bf3);

		//code to load in the ground plane (CPU defined data passed to GPU)
		initGround();
	}

	//directly pass quad for the ground to the GPU
	void initGround() {

		float g_groundSize = 60;
		float g_groundY = -0.25;

  		// A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
		float GrndPos[] = {
			-g_groundSize, g_groundY, -g_groundSize,
			-g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY, -g_groundSize
		};

		float GrndNorm[] = {
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0
		};

		static GLfloat GrndTex[] = {
      		0, 0, // back
      		0, 1,
      		1, 1,
      		1, 0 };

      	unsigned short idx[] = {0, 1, 2, 0, 2, 3};

		//generate the ground VAO
      	glGenVertexArrays(1, &GroundVertexArrayID);
      	glBindVertexArray(GroundVertexArrayID);

      	g_GiboLen = 6;
      	glGenBuffers(1, &GrndBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

      	glGenBuffers(1, &GrndNorBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

      	glGenBuffers(1, &GrndTexBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

      	glGenBuffers(1, &GIndxBuffObj);
     	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
      	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
      }
      //code to draw the ground plane
     void drawGround(Shader& s) {
     	s.prog->bind();
     	glBindVertexArray(GroundVertexArrayID);


		material c;
		c.amb.r = 0.05;
        c.amb.g = 0.22;
        c.amb.b = 0.05;
        c.dif.r = 0;
        c.dif.g = 0;
        c.dif.b = 0;
        c.spec.r = 3;
        c.spec.g = 3;
        c.spec.b = 3;
        c.shine = 1.0;
		s.flip(1);
		s.setMaterial(c);
		s.setTexture(0);

		//draw the ground plane 
  		s.setModel(vec3(0, -1, 0), 0, 0, 0, 1);
  		glEnableVertexAttribArray(0);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
  		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  		glEnableVertexAttribArray(1);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
  		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  		glEnableVertexAttribArray(2);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
  		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

   		// draw!
  		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
  		glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

  		glDisableVertexAttribArray(0);
  		glDisableVertexAttribArray(1);
  		glDisableVertexAttribArray(2);
  		s.prog->unbind();
     }

	void render(float frametime) {
		// Get current frame buffer size.
		int width, height;
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
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

		
		//material shader first
		reg.prog->bind();
		glUniformMatrix4fv(reg.prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		cam.SetView(reg.prog);

		float butterfly_height[3] = {1.1, 1.7, 1.5};

		vec3 butterfly_loc[3];
		butterfly_loc[0] = vec3(-2.3, -1, 3);
		butterfly_loc[1] = vec3(-2, -1.2, -3);
		butterfly_loc[2] = vec3(4, -1, 4);
 
		bf[0].setMaterials(0, 0.1, 0.1, 0.1, 0.02, 0.02, 0.02, 0.25, 0.23, 0.30, 9);
		bf[0].setMaterials(1, 0.4, 0.2, 0.2, 0.94, 0.23, 0.20, 0.9, 0.23, 0.20, 0.6);
		bf[0].setMaterials(2, 0.4, 0.2, 0.2, 0.94, 0.23, 0.20, 0.9, 0.23, 0.20, 0.6);

		reg.setModel(bf[0].position, -1.1, 4.1, 0, bf[0].scale); //body

		for (int i = 0; i < 3; i++) {
			reg.setMaterial(bf[0].materials[i]);
			bf[0].objs[i]->draw(reg.prog);
		}

		bf[1].setMaterials(0, 0.1, 0.1, 0.1, 0.02, 0.02, 0.02, 0.25, 0.23, 0.30, 9);
		bf[1].setMaterials(1, 0.2, 0.3, 0.3, 0.20, 0.73, 0.80, 0.9, 0.23, 0.20, 0.6);
		bf[1].setMaterials(2, 0.2, 0.3, 0.3, 0.20, 0.73, 0.80, 0.9, 0.23, 0.20, 0.6);

		reg.setModel(bf[1].position, -1.1, 4.1, 0, bf[1].scale); //body
		for (int i = 0; i < 3; i++) {
			reg.setMaterial(bf[1].materials[i]);
			bf[1].objs[i]->draw(reg.prog);
		}
    
    	bf[2].setMaterials(0, 0.1, 0.1, 0.1, 0.02, 0.02, 0.02, 0.25, 0.23, 0.30, 9);
		bf[2].setMaterials(1, 0.3, 0.3, 0.2, 0.90, 0.73, 0.20, 0.9, 0.23, 0.20, 0.6);
		bf[2].setMaterials(2, 0.3, 0.3, 0.2, 0.90, 0.73, 0.20, 0.9, 0.23, 0.20, 0.6);

		reg.setModel(bf[2].position, -1.1, 4.1, 0, bf[2].scale); //body
		for (int i = 0; i < 3; i++) {
			reg.setMaterial(bf[2].materials[i]);
			bf[2].objs[i]->draw(reg.prog);
		}


		catEnt.setMaterials(0, 0.2, 0.3, 0.3, 0.20, 0.73, 0.80, 0.9, 0.23, 0.20, 0.6);
		reg.setModel(catEnt.position, 0, catEnt.rotate, 0, catEnt.scale);
		reg.setMaterial(catEnt.material[0]);
		catEnt.objs[0]->draw(reg.prog);

		std::cout << "entity position:" << catEnt.position.x << ", " << catEnt.position.y << ", " << catEnt.position.z << std::endl;

		reg.prog->unbind();



		//using texture shader now
		tex.prog->bind();
		glUniformMatrix4fv(tex.prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));

		cam.SetView(tex.prog);

		//sky box!

		material sky_box;
		sky_box.amb.r = 0.2;
        sky_box.amb.g = 0.3;
        sky_box.amb.b = 0.65;
        sky_box.dif.r = 0;
        sky_box.dif.g = 0;
        sky_box.dif.b = 0;
        sky_box.spec.r = 0;
        sky_box.spec.g = 0;
        sky_box.spec.b = 0;
        sky_box.shine = 100.0;
		tex.flip(0);
		tex.setMaterial(sky_box);
		tex.setMaterial(sky_box);
		tex.setTexture(1);

		Model->pushMatrix();
			Model->loadIdentity();
			Model->scale(vec3(20.0));
			tex.setModel(Model);

			sphere->draw(tex.prog);

		Model->popMatrix();

		tex.unbindTexture(1);


		drawGround(tex);  //draw ground here


		catEnt->position = cam.player_pos;

		//halt animations if cat collides with flower or tree
//		cout << catEnt.position.x << ", " << catEnt.position.y << ", " << catEnt.position.z << endl;
//		cout << "before calling check collision, catID = " << catEnt.id << endl;
<<<<<<< Updated upstream
		int collided = catEnt->collider->CatCollision(bf, catEnt);
=======
		//int collided = catEnt.collider->CatCollision(bf, &catEnt);
>>>>>>> Stashed changes

		/*if (collided != -1) {
			bf_flags[collided] = 1;
		}*/


		bounds = std::sqrt(   //update cat's distance from skybox
			cam.player_pos[0] * cam.player_pos[0]
			+ cam.player_pos[2] * cam.player_pos[2]
		);

		// Pop matrix stacks.
		Projection->popMatrix();

		for (int i = 0; i < 3; i ++){
			if (bf_flags[i] == 1) {
				bf[i].scale *= 0.95f;
				if (bf[i].scale < 0.00001) {
					bf[i].scale = 0.01;
					bf_flags[i] = 0;
					bf[i].position = vec3(0, -0.3, 0);
				}
			}
			bf[i].updateMotion(frametime);
		}


	}
};


int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	
	application->initGeom(resourceDir);

	float dt = 1 / 60.0;

	auto lastTime = chrono::high_resolution_clock::now();

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// save current time for next frame
		auto nextLastTime = chrono::high_resolution_clock::now();

		// get time since last frame
		float deltaTime =
			chrono::duration_cast<std::chrono::microseconds>(
				chrono::high_resolution_clock::now() - lastTime)
				.count();
		// convert microseconds (weird) to seconds (less weird)
		deltaTime *= 0.000001;

		//deltaTime = glm::min(deltaTime, dt);

		// reset lastTime so that we can calculate the deltaTime
		// on the next frame
		lastTime = nextLastTime;

		// Render scene.
		application->render(deltaTime);

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}