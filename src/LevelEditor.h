#ifndef LEVELEDITOR_H
#define LEVELEDITOR_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stdio.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <map>
#include "Entity.h"
#include "Camera.h"
#include "ShaderManager.h"

using namespace std;
using namespace ImGui;

class Camera;

extern string resourceDir;
extern map<string, shared_ptr<Entity>> worldentities;
extern map<string, shared_ptr<Shader>> shaders;
extern map<string, shared_ptr<Texture>> textureLibrary;
extern vector<string> tagList;
extern vector<shared_ptr<Entity>> collidables;
extern shared_ptr<Entity> cur_entity;
extern Camera* activeCam;
extern float editSpeed;

class LevelEditor {
	public:
		LevelEditor();

		void Init(GLFWwindow* window);
		void NewFrame();
		void Update();
		void FindMesh();
		void ModelList();
		void EntityList();
		shared_ptr<Entity> Inspector(shared_ptr<Entity> entity, bool* flag);
		void MeshInspector(Mesh* mesh, bool* flag);
		void EditTag(bool* flag);
		void CameraSpeed();
		void Render();
		void Shutdown();
		void setCurName(string name);
		vec3 calcDirection(float pitch, float yaw) {
			vec3 direction;
			direction.x = cos(pitch) * cos(yaw);
			direction.y = sin(pitch);
			direction.z = cos(pitch) * sin(yaw);
			return normalize(direction);
		}
		bool diableInput = false;

	private:
		string cur_name;
		bool show_demo_window = true;
		bool show_another_window = false;
		vector<std::string> meshFiles;
};

#endif