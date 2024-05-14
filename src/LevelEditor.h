#ifndef LEVELEDITOR_H
#define LEVELEDITOR_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stdio.h>
#include <iostream>
#include <sys/stat.h>
#include "dirent.h"
#include <conio.h>
#include <string>
#include <map>
#include "Entity.h"

using namespace std;
using namespace ImGui;


extern string resourceDir;
extern map<string, shared_ptr<Entity>> worldentities;

class LevelEditor {
	public:
		LevelEditor();

		void Init(GLFWwindow* window);
		void NewFrame();
		void Update();
		void FindMesh();
		void MeshList();
		void EntityList();
		void Inspector(shared_ptr<Entity> entity);
		void Render();
		void Shutdown();

	private:
		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		ImGuiIO io;
		vector<std::string> meshFiles;
};

#endif