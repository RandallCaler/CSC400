/*
 * Program 4 example with diffuse and spline camera PRESS 'g'
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn (spline D. McGirr)
 */

#include <iostream>
#include <glad/glad.h>
#include <irrKlang.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Bezier.h"
#include "Spline.h"
#include "Model.h"
#include "Animation.h"
#include "Animator.h"

#define TINYOBJLOADER_IMPLEMENTATION
#define PI 3.1415927

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

using namespace std;
using namespace glm;
using namespace irrklang;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;


	//Our shader program for textures
	shared_ptr<Program> modProg;
	shared_ptr<Program> texProg;

	//global data for ground plane - direct load constant defined CPU data to GPU (not obj)
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;
	//ground VAO
	GLuint GroundVertexArrayID;

	string resourceDir;

	shared_ptr<Model> skybox;
	shared_ptr<Model> aatrox1;
	shared_ptr<Model> aatrox2;
	shared_ptr<Model> aatroxDance;
	shared_ptr<Model> curModel;

	//animation
	shared_ptr<Animation> aatrox1Anim;
	shared_ptr<Animation> aatrox2Anim;
	shared_ptr<Animation> dancelAnim;
	shared_ptr<Animation> curAnim;
	ISoundEngine* SoundEngine = createIrrKlangDevice();

	//animation data
	float lightTrans = 0;

	//camera
	double g_theta, g_phi;
	vec3 view = vec3(0, 0, 1);
	vec3 strafe = vec3(1, 0, 0);
	vec3 g_eye = vec3(0, 1, 4);
	vec3 g_lookAt = vec3(0, 1, -4);

	Spline splinepath[2];
	bool goCamera = false;
	bool isUlt = false;
	bool rotate = false;
	float rotAngle = 0.0f;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS){
			lightTrans += 0.5;
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS){
			lightTrans -= 0.5;
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_G && action == GLFW_RELEASE) {
			goCamera = !goCamera;
		}
		if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
			rotate = !rotate;
		}
		if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
			curModel = aatroxDance;
			curAnim = dancelAnim;
		}
		if (key == GLFW_KEY_B && action == GLFW_PRESS) {
			if (!isUlt) {
				curModel = aatrox1;
				curAnim = aatrox1Anim;
				isUlt = true;
			}
			else {
				curModel = aatrox2;
				curAnim = aatrox2Anim;
				isUlt = false;
			}		
		}	
		const float cameraSpeed = 0.1f;
		vec3 strafeVec = cameraSpeed * normalize(cross(view, vec3(0, 1, 0)));
		vec3 dollyVec = cameraSpeed * view;
		    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            g_eye -= dollyVec;
            g_lookAt -= dollyVec;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            g_eye += dollyVec;
            g_lookAt += dollyVec;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            g_eye += strafeVec;
            g_lookAt += strafeVec;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            g_eye -= strafeVec;
            g_lookAt -= strafeVec;
        }
    }
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			 glfwGetCursorPos(window, &posX, &posY);
			 cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
		g_theta += PI * deltaX / 200;

		g_phi += PI * deltaY/ 100;

		if (fabs(g_phi) > PI * 80 / 180) {
			g_phi = g_phi > 0 ? PI * 80 / 180 : -PI * 80 / 180;
		}

		view = -normalize(vec3(cos(g_phi) * cos(g_theta), sin(g_phi), cos(g_phi) * cos((PI / 2.0) - g_theta)));
		g_lookAt = g_eye + vec3(cos(g_phi) * cos(g_theta), sin(g_phi), cos(g_phi) * cos((PI / 2.0) - g_theta));
	}


	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();
		resourceDir = resourceDirectory;
		// Set background color.
		glClearColor(.72f, .84f, 1.06f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		g_theta = -PI / 2;

		// Initialize the GLSL program that we will use for texture mapping
		modProg = make_shared<Program>();
		modProg->setVerbose(true);
		modProg->setShaderNames(resourceDirectory + "/mod_vert.glsl", resourceDirectory + "/mod_frag.glsl");
		modProg->init();
		modProg->addUniform("P");
		modProg->addUniform("V");
		modProg->addUniform("M");
		modProg->addUniform("texture_diffuse1");
		modProg->addUniform("texture_specular1");
		modProg->addUniform("texture_normal1");
		modProg->addUniform("texture_height1");
		modProg->addUniform("lightPos");
		modProg->addUniform("flip");
		modProg->addUniform("finalBonesMatrices");
		modProg->addAttribute("vertPos");
		modProg->addAttribute("vertNor");
		modProg->addAttribute("vertTex");
		modProg->addAttribute("boneIds");
		modProg->addAttribute("weights");

		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag.glsl");
		texProg->init();
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
		texProg->addUniform("flip");
		texProg->addUniform("texture_diffuse1");
		texProg->addUniform("lightPos");
		texProg->addAttribute("vertPos");
		texProg->addAttribute("vertNor");
		texProg->addAttribute("vertTex");

  		// init splines up and down
       splinepath[0] = Spline(glm::vec3(-6,0,5), glm::vec3(-1,-5,5), glm::vec3(1, 5, 5), glm::vec3(2,0,5), 5);
       splinepath[1] = Spline(glm::vec3(2,0,5), glm::vec3(3,-2,5), glm::vec3(-0.25, 0.25, 5), glm::vec3(0,0,5), 5);
    
	}

	void initGeom(const std::string& resourceDirectory)
	{
		skybox = make_shared<Model>(resourceDirectory + "/skybox/nebula.glb");
		aatrox1 = make_shared<Model>(resourceDirectory + "/Aatrox.glb");
		aatrox1Anim = make_shared<Animation>((resourceDirectory + "/Aatrox.glb"), aatrox1);
		aatrox2 = make_shared<Model>(resourceDirectory + "/Aatrox1.glb");
		aatrox2Anim = make_shared<Animation>((resourceDirectory + "/Aatrox1.glb"), aatrox2);
		aatroxDance = make_shared<Model>(resourceDirectory + "/dance.glb");
		dancelAnim = make_shared<Animation>((resourceDirectory + "/dance.glb"), aatroxDance);
		//aatroxDance = make_shared<Model>(resourceDirectory + "/TrashContainer.dae");
		//dancelAnim = make_shared<Animation>((resourceDirectory + "/TrashContainer.dae"), aatroxDance);
		curModel = aatroxDance;
		curAnim = dancelAnim;
		SoundEngine->play2D((resourceDirectory + "/audio/aatrox.mp3").c_str(), true);
	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   	}

   	/* camera controls - do not change */
	void SetView(shared_ptr<Program>  shader) {
  		mat4 Cam = lookAt(g_eye, g_lookAt, vec3(0, 1, 0));
  		glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, value_ptr(Cam));
	}


   	void updateUsingCameraPath(float frametime)  {

   	  if (goCamera) {
       if(!splinepath[0].isDone()){
       		splinepath[0].update(frametime);
            g_eye = splinepath[0].getPosition();
        } else {
            splinepath[1].update(frametime);
            g_eye = splinepath[1].getPosition();
        }
      }
   	}

	void render(float frametime) {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		//update the camera position
		updateUsingCameraPath(frametime);

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

		const float rotSpeed = PI / 60;

		texProg->bind();
		glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		SetView(texProg);
		glUniform3f(texProg->getUniform("lightPos"), 2.0 + lightTrans, 2.0, 2.9);

		//draw big background skybox
		glUniform1i(texProg->getUniform("flip"), 1);
		Model->pushMatrix();
			Model->loadIdentity();
			Model->rotate(rotSpeed * frametime, vec3(0, 0, 1));
			Model->rotate(rotSpeed * frametime, vec3(0, 1, 0));
			Model->rotate(rotSpeed * frametime, vec3(1, 0, 0));
			Model->scale(vec3(20.0));
			setModel(texProg, Model);
			skybox->Draw(texProg);
		Model->popMatrix();


		texProg->unbind();

		modProg->bind();
		glUniformMatrix4fv(modProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		SetView(modProg);
		glUniform3f(modProg->getUniform("texture_normal1"), 0.08, 0.08, 0.08);
		glUniform3f(modProg->getUniform("lightPos"), 2.0 + lightTrans, 2.0, 2.9);
		
		Animator animator(curAnim);
		animator.UpdateAnimation(frametime);
		auto transforms = animator.GetFinalBoneMatrices();
		GLuint baseLocation = modProg->getUniform("finalBonesMatrices");
		for (int i = 0; i < transforms.size(); ++i) {
			glUniformMatrix4fv(baseLocation + i, 1, GL_FALSE, value_ptr(transforms[i]));
		}

		if (rotate) {
			rotAngle -= 0.3 * rotSpeed;
		}

		glUniform1i(modProg->getUniform("flip"), 1);
		float dScale = 1.0 / (curModel->max.x - curModel->min.x);
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(-0.2, -0.5, 0.0));
			Model->rotate(rotAngle, vec3(0, 1, 0));
			Model->scale(vec3(3 * dScale));
			setModel(modProg, Model);
			curModel->Draw(modProg);
		Model->popMatrix();



		modProg->unbind();

		// Pop matrix stacks.
		Projection->popMatrix();

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

	auto lastTime = chrono::high_resolution_clock::now();
	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// save current time for next frame
		//auto nextLastTime = chrono::high_resolution_clock::now();

		//get time since last frame
		float deltaTime =
			chrono::duration_cast<std::chrono::microseconds>(
				chrono::high_resolution_clock::now() - lastTime)
				.count();

		// convert microseconds (weird) to seconds (less weird)
		deltaTime *= 0.000001;

		// reset lastTime so that we can calculate the deltaTime
		// on the next frame
		//lastTime = nextLastTime;

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
