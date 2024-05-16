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
extern vector<string> tagList;;
extern shared_ptr<Entity> cur_entity;

class LevelEditor {
	public:
		LevelEditor();

		void Init(GLFWwindow* window);
		void NewFrame();
		void Update();
		void FindMesh();
		void MeshList();
		void EntityList();
		shared_ptr<Entity> Inspector(shared_ptr<Entity> entity);
		void Render();
		void Shutdown();
		bool diableInput = false;

	private:
		string cur_name;
		string cur_file;
		bool show_demo_window = true;
		bool show_another_window = false;
		vector<std::string> meshFiles;
};

#endif