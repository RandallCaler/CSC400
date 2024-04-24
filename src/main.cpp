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
#include "Collider.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"
#include "stb_image.h"
#include "InputHandler.h"
// #include "Entity.h"
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

// static/global vars
int Entity::NEXT_ID = 0;

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

	std::vector<shared_ptr<Shape>> butterfly;

	InputHandler ih;


	Entity bf1 = Entity();
	Entity bf2 = Entity();
	Entity bf3 = Entity();
	Entity catEnt = Entity();
	
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

	//animation data
	float lightTrans = 0;
	float sTheta = 0;
	float cTheta = 0;
	float gTrans = 0;

	//camera
	double g_theta;
	vec3 strafe = vec3(1, 0, 0);

	// 	view pitch dist angle playerpos playerrot animate g_eye
	Camera cam = Camera(vec3(0, 0, 1), 17, 4, 0, vec3(0, -1.12, 0), 0, vec3(0, 0.5, 5));

	//player animation
	bool animate = false;
	float oscillate = 0;

	//rules for cat walking around
	bool back_up = false;
	
	//keyframes for cat walking animation
	double f[5][12] = {
			{0.5, -0.6, 0.1, -0.5, 1.0, 0.1, 0.5, -0.5, 0, 0, -0.58, 0.58},
			{0.5, 0, -0.4, -0.5, 0.45, 0.72, 0.45, -0.95, 0.72, 0, 0.1, 0.08},
			{0.65, -0.5, 0.7, -0.5, 0, 0.7, 0.2, -0.85, 0.75, 0.1, 0.1, 0.082},
			{0.4, -1.2, 1.3, 0.30, -0.3, 0, -0.2, -0.2, 0.1, 0.6, -0.6, 0},
			{-0.1, -0.45, 0.55, 0.30, 0.1, -0.3, 0.1, -0.2, 0.2, 0, -0.6, 0.6}
		};

	//bounding "cylinders" for flower & tree
	double flower_radial;
	double tree_radial;

	//bounds for world
	double bounds;
	
	//interpolation of keyframes for animation
	int cur_idx = 0, next_idx = 1;
	float frame = 0.0;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		animate = false;


		// KEY PRESSED

		if (key == GLFW_KEY_W && (action == GLFW_PRESS) && !catEnt.collider->IsColliding() && bounds < 19){
			ih.inputStates[0] = 1;

			cam.player_pos += vec3(sin(cam.player_rot) * 0.1, 0, cos(cam.player_rot) * 0.1);
			animate = true;
		}

		if (key == GLFW_KEY_A && (action == GLFW_PRESS) && !catEnt.collider->IsColliding()){
			ih.inputStates[1] = 1;

			cam.player_rot += 10 * 0.01745329;
			animate = true;
		}

		if (key == GLFW_KEY_S && (action == GLFW_PRESS) && bounds < 19){	
			ih.inputStates[2] = 1;

			cam.player_pos -= vec3(sin(cam.player_rot) * 0.1, 0, cos(cam.player_rot) * 0.1);
			animate = true;
		}

		if (key == GLFW_KEY_D && (action == GLFW_PRESS)&& !catEnt.collider->IsColliding()){
			ih.inputStates[3] = 1;

			cam.player_rot -= 10 * 0.01745329;
			animate = true;
		}

		if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS)){
			ih.inputStates[4] = 1;
		}

		// KEY RELEASED

		if (key == GLFW_KEY_W && (action == GLFW_RELEASE) && !catEnt.collider->IsColliding() && bounds < 19){
			ih.inputStates[0] = 0;

			cam.player_pos += vec3(sin(cam.player_rot) * 0.1, 0, cos(cam.player_rot) * 0.1);
			animate = true;
		}

		if (key == GLFW_KEY_A && (action == GLFW_RELEASE) && !catEnt.collider->IsColliding()){
			ih.inputStates[1] = 0;

			cam.player_rot += 10 * 0.01745329;
			animate = true;
		}

		if (key == GLFW_KEY_S && (action == GLFW_RELEASE) && bounds < 19){	
			ih.inputStates[2] = 0;

			cam.player_pos -= vec3(sin(cam.player_rot) * 0.1, 0, cos(cam.player_rot) * 0.1);
			animate = true;
		}

		if (key == GLFW_KEY_D && (action == GLFW_RELEASE)&& !catEnt.collider->IsColliding()){
			ih.inputStates[3] = 0;

			cam.player_rot -= 10 * 0.01745329;
			animate = true;
		}

		if (key == GLFW_KEY_SPACE && (action == GLFW_RELEASE)){
			ih.inputStates[4] = 0;
		}
	
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}

		ih.handleInput();
	}


	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
		//cout << "xDel + yDel " << deltaX << " " << deltaY << endl;
		cam.angle -= 10 * (deltaX / 57.296);
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


	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}


	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.72f, .84f, 1.06f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		g_theta = -PI/2.0;

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

		//bounding cylinder of flower
		flower_radial = std::sqrt(
			(flower[2]->max.x - flower[2]->min.x) * (flower[2]->max.x - flower[2]->min.x)
			+ (flower[2]->max.z - flower[2]->min.z) * (flower[2]->max.z - flower[2]->min.z)
		) * 2.5;


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

		//bounding cylinder for trunk
		tree_radial = std::sqrt(
			(tree1[0]->max.x - tree1[0]->min.x) * (tree1[0]->max.x - tree1[0]->min.x)
			+ (tree1[0]->max.z - tree1[0]->min.z) * (tree1[0]->max.z - tree1[0]->min.z)
		);

		// init butterfly 1
		bf1.initEntity(butterfly);
		bf1.position = vec3(2, -0.3, -1);
		bf1.m.forward = vec3(1, 0, 0);
		bf1.m.velocity = vec3(2, 0, 2);
		bf1.collider = new Collider(butterfly, Collider::BUTTERFLY);
		bf1.collider->SetEntityID(bf1.id);
		//cout << "butterfly 1 " << bf1.id << endl;
		bf1.collider->entityName = 'b';
		bf1.scale = 0.01;
    
    	// init butterfly 2
		bf2.initEntity(butterfly);
		bf2.position = vec3(-2, -0.3, 0.5);
		bf2.m.forward = vec3(1, 0, 0);
		bf2.m.velocity = vec3(.40, 0, .40);
		bf2.collider = new Collider(butterfly, Collider::BUTTERFLY);
		bf2.collider->SetEntityID(bf2.id);
		//cout << "butterfly 2 " << bf2.id << endl;
		bf2.collider->entityName = 'b';
		
		bf2.scale = 0.01;

    
   		 // init butterfly 3
		bf3.initEntity(butterfly);
		bf3.position = vec3(4, -0.3, 0.5);
		bf3.m.forward = vec3(1, 0, 0);
		bf3.m.velocity = vec3(.20, 0, .20);
		bf3.collider = new Collider(butterfly, Collider::BUTTERFLY);
		bf3.collider->SetEntityID(bf3.id);
		//cout << "butterfly 3 " << bf3.id << endl;
		bf3.collider->entityName = 'b';
	
		bf3.scale = 0.01;



		// init cat entity
		catEnt.initEntity(cat);
		catEnt.position = cam.player_pos;
		//cout << catEnt.position.x << ", " << catEnt.position.y << ", " << catEnt.position.z << endl;
		// set forward
		// set velocity
		catEnt.collider = new Collider(cat, Collider::CAT);
		catEnt.collider->SetEntityID(catEnt.id);
		gameObjects.push_back(catEnt);
		
		//cout << "cat " << catEnt.id << endl;
		catEnt.collider->entityName = 'c';

		// vec3 tree_loc[7];
		// tree_loc[0] = vec3(4, -5.5, 7);
		// tree_loc[1] = vec3(-2.9,-5.5, -7);
		// tree_loc[2] = vec3(8, -5.5, -3);
		// tree_loc[3] = vec3(6, -5.5, -3.7);
		// tree_loc[4] = vec3(-1, -5.5, 4.9);
		// tree_loc[5] = vec3(-5, -5.5, 9);
		// tree_loc[6] = vec3(-6, -5.5, 2);

		// // init tree entities
		// for (int i = 0; i < 7; i++) {
		// 	Entity e = Entity();
      	// 	e.initEntity(tree1);
		// 	e.position = tree_loc[i] + vec3(0, 4.1, 0);
		// 	e.setMaterials(0, 0, 0, 0, 0.897093, 0.588047, 0.331905, 0.5, 0.5, 0.5, 200);
		// 	for (int j = 1; j < 12; j++) {
		// 		e.setMaterials(j, 0.1, 0.2, 0.1, 0.285989, 0.567238, 0.019148, 0.5, 0.5, 0.5, 200);
		// 	}
		// 	e.collider = new Collider(tree1, Collider::TREE);
		// 	e.collider->SetEntityID(e.id);
		// 	trees.push_back(e);
		// 	gameObjects.push_back(e);
		// }

		// //where each flower will go
		// vec3 flower_loc[7];
		// flower_loc[0] = vec3(4, -1, 4);
		// flower_loc[1] = vec3(-2.3, -1, 3);
		// flower_loc[2] = vec3(-2, -1.2, -3);
		// flower_loc[3] = vec3(4, -1, -3.2);
		// flower_loc[4] = vec3(-5, -1, 2);
		// flower_loc[5] = vec3(1, -1, -1.7);
		// flower_loc[6] = vec3(3, -1, -2);

		// // init flower entities
		// for (int i = 0; i < 7; i++) {
		// 	Entity e = Entity();
      	// 	e.initEntity(flower);
		// 	e.position = flower_loc[i];
		// 	e.setMaterials(0, 0.2, 0.1, 0.1, 0.94, 0.42, 0.64, 0.7, 0.23, 0.60, 100);
		// 	e.setMaterials(1, 0.1, 0.1, 0.1, 0.94, 0.72, 0.22, 0.23, 0.23, 0.20, 100);
		// 	e.setMaterials(2, 0.05, 0.15, 0.05, 0.24, 0.92, 0.41, 1, 1, 1, 0);
		// 	e.collider = new Collider(flower, Collider::FLOWER);
		// 	e.collider->SetEntityID(e.id);
		// 	flowers.push_back(e);
		// 	gameObjects.push_back(e);
		// }
    
    bf.push_back(bf1);
    bf.push_back(bf2);
    bf.push_back(bf3);
	gameObjects.push_back(bf1);
	gameObjects.push_back(bf2);
	gameObjects.push_back(bf3);

	//cout << "butterfly entities size = " << bf.size() << endl;
	for(int i = 0; i < bf.size(); i++){
		//cout << "bf[" << i << "] id is " << bf[i].id << endl;
	}
    //cout << "gameObjects size = " << gameObjects.size() << endl;

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
     void drawGround(Shader s) {
     	s.prog->bind();
     	glBindVertexArray(GroundVertexArrayID);


		materials c;
		c.matAmb.r = 0.05;
        c.matAmb.g = 0.22;
        c.matAmb.b = 0.05;
        c.matDif.r = 0;
        c.matDif.g = 0;
        c.matDif.b = 0;
        c.matSpec.r = 3;
        c.matSpec.g = 3;
        c.matSpec.b = 3;
        c.matShine = 1.0;
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
			reg.setMaterial(bf[0].material[i]);
			bf[0].objs[i]->draw(reg.prog);
		}

		bf[1].setMaterials(0, 0.1, 0.1, 0.1, 0.02, 0.02, 0.02, 0.25, 0.23, 0.30, 9);
		bf[1].setMaterials(1, 0.2, 0.3, 0.3, 0.20, 0.73, 0.80, 0.9, 0.23, 0.20, 0.6);
		bf[1].setMaterials(2, 0.2, 0.3, 0.3, 0.20, 0.73, 0.80, 0.9, 0.23, 0.20, 0.6);

		reg.setModel(bf[1].position, -1.1, 4.1, 0, bf[1].scale); //body
		for (int i = 0; i < 3; i++) {
			reg.setMaterial(bf[1].material[i]);
			bf[1].objs[i]->draw(reg.prog);
		}
    
    	bf[2].setMaterials(0, 0.1, 0.1, 0.1, 0.02, 0.02, 0.02, 0.25, 0.23, 0.30, 9);
		bf[2].setMaterials(1, 0.3, 0.3, 0.2, 0.90, 0.73, 0.20, 0.9, 0.23, 0.20, 0.6);
		bf[2].setMaterials(2, 0.3, 0.3, 0.2, 0.90, 0.73, 0.20, 0.9, 0.23, 0.20, 0.6);

		reg.setModel(bf[2].position, -1.1, 4.1, 0, bf[2].scale); //body
		for (int i = 0; i < 3; i++) {
			reg.setMaterial(bf[2].material[i]);
			bf[2].objs[i]->draw(reg.prog);
		}


		//reg.setModel(vec3(4, -1, 4), cTheta*cTheta, 0, 0, 2.5);
		// for (int i = 0; i < 7; i++) {
					
		// 	reg.setModel(flowers[i].position, cTheta*cTheta+0.03, 0, 0, 2.5); 
		// 	for (int j = 0; j < 3; j++) {
		// 		reg.setMaterial(flowers[i].material[j]);
		// 		flowers[i].objs[j]->draw(reg.prog);
		// 	}
		// }

		// for (int i = 0; i < 7; i++) {
		// 	reg.setModel(trees[i].position, 0, 0, 0, 0.15); 
		// 	for (int j = 0; j < 12; j++) {
		// 		reg.setMaterial(trees[i].material[j]);
		// 		trees[i].objs[j]->draw(reg.prog);
		// 	}
		// }


		reg.prog->unbind();



		//using texture shader now
		tex.prog->bind();
		glUniformMatrix4fv(tex.prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));

		cam.SetView(tex.prog);


		materials c;
		c.matAmb.r = 0.17;
        c.matAmb.g = 0.05;
        c.matAmb.b = 0.05;
        c.matDif.r = 0;
        c.matDif.g = 0;
        c.matDif.b = 0;
        c.matSpec.r = 0.2;
        c.matSpec.g = 0.1;
        c.matSpec.b = 0.1;
        c.matShine = 5;
		tex.flip(1);
		tex.setMaterial(c);
		tex.setTexture(3);


		//hierarchical modeling with cat!!
		Model->pushMatrix();
			Model->translate(cam.player_pos + vec3(0, cos(oscillate) * 0.009, 0));
			Model->rotate(cam.player_rot, vec3(0, 1, 0));
			Model->scale(vec3(0.7, 0.7, 0.7));

			
			Model->pushMatrix();  // upper left leg
				Model->translate(vec3(0.16, 0.48, 0.35));
				Model->rotate(f[cur_idx][0] * (1 - frame) + f[next_idx][0] * frame, vec3(1, 0, 0));
				Model->translate(vec3(0, -0.09, 0));

				Model->pushMatrix();  // upper left calf
					Model->translate(vec3(0, -0.17, 0));
					Model->rotate(f[cur_idx][1] * (1 - frame) + f[next_idx][1] * frame, vec3(1, 0, 0));
					Model->translate(vec3(0, -0.17, 0));

					Model->pushMatrix();  // upper left foot
						Model->translate(vec3(0, -0.15, -0.018));
						Model->rotate(f[cur_idx][2] * (1 - frame) + f[next_idx][2] * frame, vec3(1, 0, 0));
						Model->translate(vec3(0, 0, 0.09));

						Model->scale(vec3(0.08, 0.045, 0.1));
						tex.setModel(Model);
						sphere->draw(tex.prog);
					Model->popMatrix();

					Model->scale(vec3(0.08, 0.18, 0.084));
					tex.setModel(Model);
					sphere->draw(tex.prog);
				Model->popMatrix();

				Model->scale(vec3(0.11, 0.29, 0.12));
				tex.setModel(Model);
				sphere->draw(tex.prog);
			Model->popMatrix();




			Model->pushMatrix(); // upper right leg
				Model->translate(vec3(-0.16, 0.48, 0.35));
				Model->rotate(f[cur_idx][3] * (1 - frame) + f[next_idx][3] * frame, vec3(1, 0, 0));
				Model->translate(vec3(0, -0.09, 0));

				Model->pushMatrix();  // upper right calf
					Model->translate(vec3(0, -0.17, 0));
					Model->rotate(f[cur_idx][4] * (1 - frame) + f[next_idx][4] * frame, vec3(1, 0, 0));
					Model->translate(vec3(0, -0.17, 0));

					Model->pushMatrix();  // upper right foot
						Model->translate(vec3(0, -0.15, -0.018));
						Model->rotate(f[cur_idx][5] * (1 - frame) + f[next_idx][5] * frame, vec3(1, 0, 0));
						Model->translate(vec3(0, 0, 0.09));

						Model->scale(vec3(0.08, 0.045, 0.1));
						tex.setModel(Model);
						sphere->draw(tex.prog);
					Model->popMatrix();

					Model->scale(vec3(0.08, 0.18, 0.084));
					tex.setModel(Model);
					sphere->draw(tex.prog);
				Model->popMatrix();

				Model->scale(vec3(0.11, 0.29, 0.12));
				tex.setModel(Model);
				sphere->draw(tex.prog);
			Model->popMatrix();




			Model->pushMatrix(); // lower left leg
				Model->translate(vec3(0.2, 0.58, -0.29));
				Model->rotate(f[cur_idx][6] * (1 - frame) + f[next_idx][6] * frame, vec3(1, 0, 0));
				Model->translate(vec3(0, -0.27, 0));

				Model->pushMatrix();  // lower left calf
					Model->translate(vec3(0, -0.18, 0));
					Model->rotate(f[cur_idx][7] * (1 - frame) + f[next_idx][7] * frame, vec3(1, 0, 0));
					Model->translate(vec3(0, -0.15, 0));

					Model->pushMatrix();  // lower left foot
						Model->translate(vec3(0, -0.13, 0));
						Model->rotate(f[cur_idx][8] * (1 - frame) + f[next_idx][8] * frame, vec3(1, 0, 0));
						Model->translate(vec3(0, 0, 0.045));

						Model->scale(vec3(0.08, 0.05, 0.11));
						tex.setModel(Model);
						sphere->draw(tex.prog);
					Model->popMatrix();

					Model->scale(vec3(0.082, 0.17, 0.082));
					tex.setModel(Model);
					sphere->draw(tex.prog);
				Model->popMatrix();

				Model->scale(vec3(0.12, 0.3, 0.12));
				tex.setModel(Model);
				sphere->draw(tex.prog);
			Model->popMatrix();





			Model->pushMatrix(); // lower right leg
				Model->translate(vec3(-0.2, 0.58, -0.29));
				Model->rotate(f[cur_idx][9] * (1 - frame) + f[next_idx][9] * frame, vec3(1, 0, 0));
				Model->translate(vec3(0, -0.27, 0));

				Model->pushMatrix();  // lower right calf
					Model->translate(vec3(0, -0.18, 0));
					Model->rotate(f[cur_idx][10] * (1 - frame) + f[next_idx][10] * frame, vec3(1, 0, 0));
					Model->translate(vec3(0, -0.15, 0));

					Model->pushMatrix();  // lower right foot
						Model->translate(vec3(0, -0.13, 0));
						Model->rotate(f[cur_idx][11] * (1 - frame) + f[next_idx][11] * frame, vec3(1, 0, 0));
						Model->translate(vec3(0, 0, 0.045));

						Model->scale(vec3(0.08, 0.05, 0.11));
						tex.setModel(Model);
						sphere->draw(tex.prog);
					Model->popMatrix();

					Model->scale(vec3(0.082, 0.17, 0.082));
					tex.setModel(Model);
					sphere->draw(tex.prog);
				Model->popMatrix();



				Model->scale(vec3(0.12, 0.3, 0.12));
				tex.setModel(Model);
				sphere->draw(tex.prog);
			Model->popMatrix();


			Model->pushMatrix(); // tail base
				Model->translate(vec3(0, 0.62, -0.27));
				Model->rotate(2.1, vec3(1, 0, 0));
				Model->translate(vec3(0, -0.27, 0));

				Model->pushMatrix();  // main stiff part
					Model->translate(vec3(0, -0.12, 0));
					Model->rotate(0.8, vec3(1, 0, 0));
					Model->translate(vec3(0, -0.15, 0));

					Model->pushMatrix();  // wiggling part
						Model->translate(vec3(0, -0.13, 0));
						Model->rotate(cos(glfwGetTime()*3) / 2, vec3(0, 0, 1));
						Model->translate(vec3(0, -0.13, 0));

						Model->pushMatrix();  // wiggling tip
							Model->translate(vec3(0, -0.06, 0));
							Model->rotate(cos(glfwGetTime()*5) / 3, vec3(0, 0, 1));
							Model->translate(vec3(0, -0.13, 0));

							Model->scale(vec3(0.09, 0.16, 0.09));
							tex.setModel(Model);
							sphere->draw(tex.prog);
						Model->popMatrix();


						Model->scale(vec3(0.1, 0.16, 0.1));
						tex.setModel(Model);
						sphere->draw(tex.prog);
					Model->popMatrix();

					Model->scale(vec3(0.11, 0.22, 0.11));
					tex.setModel(Model);
					sphere->draw(tex.prog);
				Model->popMatrix();

				Model->scale(vec3(0.12, 0.2, 0.12));
				tex.setModel(Model);
				sphere->draw(tex.prog);
			Model->popMatrix();

			tex.unbindTexture(3);
			tex.setTexture(2);

			Model->scale(vec3(0.4, 0.4, 0.4));
			tex.setModel(Model);
			cat[0]->draw(tex.prog); // body !


		Model->popMatrix();

		tex.unbindTexture(2);


		//sky box!


		materials sky_box;
		sky_box.matAmb.r = 0.2;
        sky_box.matAmb.g = 0.3;
        sky_box.matAmb.b = 0.65;
        sky_box.matDif.r = 0;
        sky_box.matDif.g = 0;
        sky_box.matDif.b = 0;
        sky_box.matSpec.r = 0;
        sky_box.matSpec.g = 0;
        sky_box.matSpec.b = 0;
        sky_box.matShine = 100.0;
		tex.flip(0);
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


		catEnt.position = cam.player_pos;
		//halt animations if cat collides with flower or tree
		//cout << catEnt.position.x << ", " << catEnt.position.y << ", " << catEnt.position.z << endl;
		//cout << "before calling check collision, catID = " << catEnt.id << endl;
		int collided = catEnt.collider->CatCollision(bf, &catEnt);

		if (collided != -1) {
			bf_flags[collided] = 1;
		}




		//cout << "cat collision = " << catEnt.collider->IsColliding() <<  endl;
	//	check_collision(flower_loc, 7, tree_loc, 7, player_pos);
		
		//update animation variables
		sTheta = -1*abs(sin(glfwGetTime() * 2));
		cTheta = cos(glfwGetTime()) / 4;
		if (animate) {
			frame += 0.1;
			if (frame >= 0.99 && frame <= 1.01) {
				frame = 0.0;
				cur_idx = (cur_idx + 1) % 5;
				next_idx = (next_idx + 1) % 5;
			}
			oscillate += 0.2;  //oscillate while walking
		}		
		oscillate += 0.02; //to make sure cat is not entirely stagnant

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

		deltaTime = glm::min(deltaTime, dt);

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