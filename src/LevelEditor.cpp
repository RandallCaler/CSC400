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
    ModelList();
    EntityList();
    CameraSpeed();
    diableInput = ImGui::IsAnyItemActive() && (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow));
}

void LevelEditor::FindMesh() {
    namespace fs = filesystem;
    vector<string> meshFiles;

    try {
        fs::path resourcePath(resourceDir);

        if (fs::exists(resourcePath) && fs::is_directory(resourcePath)) {
            for (const auto& entry : fs::directory_iterator(resourcePath)) {
                if (entry.is_regular_file() && entry.path().extension() == ".obj") {
                    meshFiles.push_back(entry.path().filename().string());
                }
            }
        }
        else {
            cerr << "Failed to open directory: " << resourceDir << endl;
        }
    }
    catch (const fs::filesystem_error& e) {
        cerr << "Filesystem error: " << e.what() << endl;
    }

    // Assuming you want to store the meshFiles in the class member
    this->meshFiles = meshFiles;
}

void LevelEditor::ModelList() {
    ImGui::Begin("Mesh List"); // Begin ImGui window

    if (meshFiles.empty()) {
        ImGui::Text("No mesh files found.");
    }
    else {   
        static string selectedModel = " ";
        for (const auto& file : meshFiles) {
            bool is_selected = (file == selectedModel);
            if (ImGui::Selectable(file.c_str(), is_selected)) {
                selectedModel = file;
                cout << "Model selected: " << selectedModel << endl;                  
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
                if (ImGui::Button("ADD##")) {
                    shared_ptr<Entity> newEntity = make_shared<Entity>();
                    newEntity->model = make_shared<Model>(resourceDir + '/' + selectedModel);
                    newEntity->defaultShaderName = "reg";
                    newEntity->tag = "";
                    vec3 view = -vec3(sin(-activeCam->angle) * cos(activeCam->pitch), sin(activeCam->pitch), cos(activeCam->angle) * cos(activeCam->pitch));                    
                    float distance = 2.0f;
                    newEntity->position = activeCam->cameraPos + view * distance;
                    newEntity->collidable = false;

                    string name = selectedModel.substr(0, selectedModel.find('.'));
                    int i = 1;
                    string tempName = name;
                    while (worldentities.find(tempName) != worldentities.end()) {            
                        tempName = name + "(" + to_string(i) + ")";
                        i++;
                    }
                    worldentities[tempName] = newEntity;
                    cur_name = tempName;
                }
            }
        }
    }

    ImGui::End(); // End ImGui window
}

void LevelEditor::EntityList()
{
    ImGui::Begin("Entity List"); // Begin ImGui window

    static string previous_name = "";
    static string selected_mesh_name = "";
    static Mesh* selected_mesh = nullptr;

    if (worldentities.empty()) {
        ImGui::Text("No entities found.");
    }
    else {
        for (const auto& pair : worldentities) {          
            const string& name = pair.first;
            bool is_selected = (cur_name == name);
            const bool was_selected = (previous_name == name);

            // Automatically collapse the previously selected node when a new one is selected
            if (cur_name != name && was_selected) {
                ImGui::SetNextItemOpen(false);
            }

            if (ImGui::TreeNodeEx(name.c_str(), (is_selected ? ImGuiTreeNodeFlags_Selected : 0))) {
                if (cur_name != name) {
                    previous_name = cur_name;  // Update the previous name
                    cur_name = name;  // Update the current name
                    selected_mesh_name = ""; // Deselect mesh
                    selected_mesh = nullptr;
                    cout << "Entity selected: " << name << endl;
                }

                ImGui::Indent(20.0f);

                // Show meshes as subitems
                auto selectedEntity = worldentities.find(cur_name);
                if (selectedEntity != worldentities.end()) {
                    for (auto& meshPair : selectedEntity->second->model->meshes) {
                        const string& meshName = meshPair.first;
                        if (ImGui::Selectable(meshName.c_str(), selected_mesh_name == meshName)) {
                            // Handle mesh selection
                            selected_mesh_name = meshName;
                            selected_mesh = &(meshPair.second);
                            cout << "Mesh selected: " << meshName << endl;
                        }
                    }
                }

                ImGui::Unindent(20.0f);

                ImGui::TreePop();
            }

            // Handle entity deletion
            if (is_selected && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete))) {
                cout << "Entity deleted: " << name << endl;
                worldentities.erase(cur_name);
                cur_name = "";
                previous_name = "";
                selected_mesh_name = "";
                selected_mesh = nullptr;
                break; // Break the loop as the iterator is invalidated after erasure
            }
        }
    }

    // Handle inspection of the selected entity or mesh
    if (selected_mesh) {
        MeshInspector(selected_mesh);
    }
    auto selectedEntity = worldentities.find(cur_name);
    if (selectedEntity != worldentities.end()) {
        cur_entity = Inspector(selectedEntity->second);
    }

    ImGui::End(); // End ImGui window
}


void LevelEditor::MeshInspector(Mesh* mesh) {
    ImGui::Begin("Mesh Inspector");
    ImGui::Text("Mesh Name: %s", mesh->name.c_str());

    // Display UI for material attributes
    if (mesh) {
        material& mat = mesh->mat; // Assuming mesh has a member mat of type material

        ImGui::Separator();
        ImGui::Text("Material");

        ImGui::ColorEdit3("Light Color", reinterpret_cast<float*>(&mat.lightColor));
        ImGui::ColorEdit3("Albedo", reinterpret_cast<float*>(&mat.albedo));
        ImGui::ColorEdit3("Emissivity", reinterpret_cast<float*>(&mat.emissivity));
        ImGui::ColorEdit3("Reflectance", reinterpret_cast<float*>(&mat.reflectance));
        ImGui::DragFloat("Roughness", &mat.roughness, 0.01f, 0.0f, 1.0f, "%.2f");
    }

    ImGui::End();
}

shared_ptr<Entity> LevelEditor::Inspector(shared_ptr<Entity> entity) {
    ImGui::Begin("Inspector");

    static char new_name[256] = ""; // Buffer for new cur_name input
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

    ImGui::Separator();

    // Show the Add Tag window
    if (show_edit_tag_window) {
        EditTag(&show_edit_tag_window);
    }

    // Transform
    ImGui::Text("Transform");
    ImGui::Separator();
    ImGui::DragFloat3("Position", value_ptr(entity->position), 0.01f, 0.0f, 0.0f, "%.2f");
    vec3 rotationDegrees = degrees(vec3(entity->rotX, entity->rotY, entity->rotZ));
    if (ImGui::DragFloat3("Rotation", value_ptr(rotationDegrees), 0.5f, 0.0f, 0.0f, "%.2f")) {
        entity->rotX = radians(rotationDegrees.x);
        entity->rotY = radians(rotationDegrees.y);
        entity->rotZ = radians(rotationDegrees.z);
    }
    ImGui::DragFloat3("Scale", value_ptr(entity->scaleVec), 0.01f, 0.0f, 0.0f, "%.2f");

    ImGui::Separator();
    ImGui::Text("Collider");
    
    if (ImGui::Checkbox("##collider", &entity->collidable)) {
        if (entity->collidable == false) {
            collidables.erase(find(collidables.begin(), collidables.end(), entity));
        }
        else {
            collidables.push_back(entity);
        }
    }

    const char* current_shader = entity->defaultShaderName.c_str();
    ImGui::Separator();
    ImGui::Text("Shader");
    if (ImGui::BeginCombo("##shader", current_shader)) {
        for (const auto& s : shaders) {
            if (ImGui::Selectable(s.second->name.c_str())) {
                entity->defaultShaderName = s.second->name;
            }
        }
        ImGui::EndCombo();
    }

    const char* current_texture = entity->model->extTexture ? entity->model->extTexture->name.c_str() : " ";
    ImGui::Separator();
    ImGui::Text("Texture");
    if (ImGui::BeginCombo("##texture", current_texture)) {
        for (const auto& tex : textureLibrary) {
            if (tex.first == "") {
                if (ImGui::Selectable(" ")) {
                    entity->model->unbindExternalTexture();
                }
            }
            else {
                if (ImGui::Selectable(tex.first.c_str())) {
                    entity->model->unbindExternalTexture();
                    entity->model->loadExtTexture(tex.second);
                }
            }
            
        }
        ImGui::EndCombo();
    }


    ImGui::End();
    return entity;
}

void LevelEditor::EditTag(bool* flag) {
    static string selectedTag = " ";
    static char new_tag[256] = ""; // Buffer for new tag input
    ImGui::Begin("Edit Tag", flag);
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
        *flag = false;
        new_tag[0] = '\0';
    }
    ImGui::SameLine();
    if (ImGui::Button("Close")) {
        selectedTag = " ";
        *flag = false;
        new_tag[0] = '\0';
    }

    ImGui::End();
}


void LevelEditor::CameraSpeed() {
    ImGui::Begin("Camera Speed");
    ImGui::SliderFloat("##Speed", &editSpeed, 2.0f, 30.0f, "%.2f");
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

void LevelEditor::setCurName(string name)
{
    cur_name = name;
}

