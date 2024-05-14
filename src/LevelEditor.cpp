#include "LevelEditor.h"


LevelEditor::LevelEditor(){}

void LevelEditor::Init(GLFWwindow* window) {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();

    io.FontGlobalScale = 2.0f;

    FindMesh();

}

void LevelEditor::NewFrame()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
}

void LevelEditor::Update() {
    MeshList();
    EntityList();

    //if (show_demo_window)
    //    ImGui::ShowDemoWindow(&show_demo_window);

    //// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    //{
    //    static float f = 0.0f;
    //    static int counter = 0;

    //    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

    //    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
    //    ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
    //    ImGui::Checkbox("Another Window", &show_another_window);

    //    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
    //    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

    //    if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
    //        counter++;
    //    ImGui::SameLine();
    //    ImGui::Text("counter = %d", counter);

    //    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    //    ImGui::End();


    //}

    //// 3. Show another simple window.
    //if (show_another_window)
    //{
    //    ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    //    ImGui::Text("Hello from another window!");
    //    if (ImGui::Button("Close Me"))
    //        show_another_window = false;
    //    ImGui::End();
    //}
}

void LevelEditor::FindMesh() {
    struct dirent* d;
    struct stat dst;
    DIR* dr;
    
    dr = opendir(resourceDir.c_str());

    if (dr != NULL) {
        for (d = readdir(dr); d != NULL; d = readdir(dr)) {
            string fileName = d->d_name;
            if (fileName == "." || fileName == "..") continue;
            string fullPath = resourceDir + "/" + fileName;
            if (stat(fullPath.c_str(), &dst) == 0) {  // Check file status; ensure stat call is successful
                if (S_ISREG(dst.st_mode)) {  // Check if it's a regular file
                    if (fullPath.substr(fullPath.length() - 4) == ".obj") {  // Check for .obj extension
                        meshFiles.push_back(fileName);  // Add to list if it's an .obj file
                    }
                }
            }
            else {
                std::cerr << "Failed to get stats for " << fullPath << std::endl;
            }
        }
        closedir(dr);
    }
    else {
        std::cerr << "Failed to open directory: " << resourceDir << std::endl;
    }

}

void LevelEditor::MeshList() {
    ImGui::Begin("Mesh List"); // Begin ImGui window
    static string cur_file = "";

    if (meshFiles.empty()) {
        ImGui::Text("No mesh files found.");
    }
    else {   
        for (const auto& file : meshFiles) {
            const bool is_selected = (cur_file == file);
            if (ImGui::Selectable(file.c_str(), is_selected)) {
                cur_file = file;
                cout << "Mesh selected: " << file << endl;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
        }
    }

    ImGui::End(); // End ImGui window
}

void LevelEditor::EntityList()
{
    ImGui::Begin("Entity List"); // Begin ImGui window
    static string cur_name = "";

    if (worldentities.empty()) {

    }
    else {
        for (const auto& pair : worldentities) {
            const string& name = pair.first;
            const bool is_selected = (cur_name == name);
            if (ImGui::Selectable(name.c_str(), is_selected)) {
                cur_name = name;
                cout << "Entity selected: " << name << endl;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
        }
    }

    auto selectedEntity = worldentities.find(cur_name);
    if (selectedEntity != worldentities.end()) {
        Inspector(selectedEntity->second);
        cur_entity = selectedEntity->second;
    }

    ImGui::End(); // End ImGui window
}

void LevelEditor::Inspector(shared_ptr<Entity> entity) {
    Begin("Inspector");

    //transform
    Text("Transform");
    ImGui::Separator();
    ImGui::DragFloat3("Position", &entity->position[0], 0.01f, 0.0f, 0.0f, "%.2f");
    glm::vec3 rotationDegrees = glm::vec3(
        glm::degrees(entity->rotX),
        glm::degrees(entity->rotY),
        glm::degrees(entity->rotZ)
    );
    if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotationDegrees), 0.5f, 0.0f, 0.0f, "%.2f")) {
        entity->rotX = glm::radians(rotationDegrees.x);
        entity->rotY = glm::radians(rotationDegrees.y);
        entity->rotZ = glm::radians(rotationDegrees.z);
    }
    ImGui::DragFloat3("Scale", &entity->scaleVec[0], 0.01f, 0.0f, 0.0f, "%.2f");
    ImGui::End();
}

void LevelEditor::Render() {
	// Render dear imgui into screen
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void LevelEditor::Shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
