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
    diableInput = ImGui::IsAnyItemActive() && (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow));
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
                cerr << "Failed to get stats for " << fullPath << endl;
            }
        }
        closedir(dr);
    }
    else {
        cerr << "Failed to open directory: " << resourceDir << endl;
    }

}

void LevelEditor::MeshList() {
    ImGui::Begin("Mesh List"); // Begin ImGui window

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
        cur_entity = Inspector(selectedEntity->second);
    }

    ImGui::End(); // End ImGui window
}


shared_ptr<Entity> LevelEditor::Inspector(shared_ptr<Entity> entity) {
    ImGui::Begin("Inspector");

    static char new_name[256] = ""; // Buffer for new cur_name input
    static char new_tag[256] = ""; // Buffer for new tag input
    static bool show_edit_tag_window = false; // Flag to show/hide the add tag window

    strncpy(new_name, cur_name.c_str(), sizeof(new_name) - 1);
    new_name[sizeof(new_name) - 1] = '\0';

    ImGui::Text("Name");
    if (ImGui::InputText("##entity_name", new_name, IM_ARRAYSIZE(new_name), ImGuiInputTextFlags_EnterReturnsTrue)) {
        // Handle cur_name change logic here
        string newNameStr(new_name);
        if (newNameStr != cur_name && !newNameStr.empty()) {
            // Check if the new cur_name already exists
            if (worldentities.find(newNameStr) == worldentities.end()) {
                // Update the entity cur_name in the map
                auto selectedEntity = worldentities.find(cur_name);
                if (selectedEntity != worldentities.end()) {
                    worldentities.erase(selectedEntity);  // Remove the old entry
                    worldentities[newNameStr] = entity;   // Insert the entity with the new cur_name
                    cur_name = newNameStr;
                    cout << "Entity cur_name changed to: " << cur_name << endl;
                }
            }
            else {
                cout << "cur_name already exists. Choose a different cur_name." << endl;
            }
        }
    }

    const char* current_tag = entity->tag.c_str();
    ImGui::Text("Tag");
    if (ImGui::BeginCombo("##entity_tag", current_tag)) {
        for (const auto& tag : tagList) {
            if (tag == "") {
                if (ImGui::Selectable(" ")) {
                    entity->tag = tag; // Update the entity's tag
                }
            }
            else {
                if (ImGui::Selectable(tag.c_str())) {
                    entity->tag = tag; // Update the entity's tag
                }
            }
            
        }
        ImGui::EndCombo();
    }

    if (ImGui::Button("Edit Tag")) {
        show_edit_tag_window = true;
    }

    // Show the Add Tag window
    if (show_edit_tag_window) {
        static string selectedTag = " ";
        ImGui::Begin("Edit Tag", &show_edit_tag_window);
        for (const auto& tag : tagList) {
            bool is_selected = (tag == selectedTag);
            if (tag != "") {
                if (ImGui::Selectable(tag.c_str(), is_selected)) {
                    selectedTag = tag;
                }
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
                if (ImGui::Button("Delete##")) {
                    for (auto ent : worldentities) {
                        if (ent.second->tag == tag) {
                            ent.second->tag = "";
                        }
                   }
                    tagList.erase(find(tagList.begin(), tagList.end(), tag));
                    selectedTag = " ";
                }
            }      
        }
        ImGui::Text("Add Tag");
        ImGui::InputText("##add_tag", new_tag, IM_ARRAYSIZE(new_tag));
        if (ImGui::Button("Add")) {
            string newTagStr(new_tag);
            if (newTagStr.find(' ') == string::npos && find(tagList.begin(), tagList.end(), newTagStr) == tagList.end()) {
                tagList.push_back(newTagStr);
                cout << "New tag added: " << newTagStr << endl;
            }
            selectedTag = " ";
            show_edit_tag_window = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Close")) {
            selectedTag = " ";
            show_edit_tag_window = false;
        }

        ImGui::End();
    }
    else {
        new_tag[0] = '\0';
    }

    // Transform
    ImGui::Text("Transform");
    ImGui::Separator();
    ImGui::DragFloat3("Position", glm::value_ptr(entity->position), 0.01f, 0.0f, 0.0f, "%.2f");
    glm::vec3 rotationDegrees = glm::degrees(glm::vec3(entity->rotX, entity->rotY, entity->rotZ));
    if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotationDegrees), 0.5f, 0.0f, 0.0f, "%.2f")) {
        entity->rotX = glm::radians(rotationDegrees.x);
        entity->rotY = glm::radians(rotationDegrees.y);
        entity->rotZ = glm::radians(rotationDegrees.z);
    }
    ImGui::DragFloat3("Scale", glm::value_ptr(entity->scaleVec), 0.01f, 0.0f, 0.0f, "%.2f");

    ImGui::End();
    return entity;
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
