#include "ImportExport.h"

ImporterExporter::ImporterExporter(map<string, shared_ptr<Shader>>* shaders, map<string, shared_ptr<Entity>>* worldentities, vector<string>* tagList, vector<shared_ptr<Entity>>* collidables) {
	// mutable references to main's shaders and entities
	this->shaders = shaders;
	this->worldentities = worldentities;
	this->tagList = tagList;
	this->collidables = collidables;
}

ImporterExporter::~ImporterExporter() {
    ;
}

void ImporterExporter::loadShader(const json& shaderData) {
	// Extract shader information from the JSON object
	string id = shaderData["shaderName"];
	string vertexSFile = shaderData["vertShader"];
	string fragSFile = shaderData["fragShader"];

	// Construct the shader path with the directory
	shared_ptr<Shader> shader = make_shared<Shader>(resourceDir + vertexSFile, resourceDir + fragSFile, false);

	cout << "Loading Shader: " << id << " (" << vertexSFile << ", " << fragSFile << ")" << endl;

	// Add uniforms to shader definition
	for (const string& uniform : shaderData["uniforms"]) {
		shader->setUniform(uniform);
		cout << uniform << " ";
	}

	cout << shaderData["attributes"].size() << " ";

	// Add attributes to shader definition
	for (const string& attribute : shaderData["attributes"]) {
		shader->setAttribute(attribute);
		cout << attribute << " ";
	}

	// Add or update the shader in the main shader list
	(*shaders)[id] = shader;
}


void ImporterExporter::loadTexture(const json& texData) {
	// Extract texture information from the JSON object
	string id = texData["name"];
	string textFile = texData["filePath"];

	cout << "Loading Texture: " << id << " (" << textFile << ")" << endl;

	// Initialize the texture
	shared_ptr<Texture> texture = make_shared<Texture>();
	texture->setFilename(resourceDir + textFile);
	texture->setName(id);
	texture->init(false);
	texture->setUnit(0);
	texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	// Add or update the texture in the texture library
	textureLibrary[id] = texture;
}

void ImporterExporter::loadEntity(const json& entData) {
	string id = entData["name"];
	string file = entData["file"];
	string tag = entData["tag"];
	int collision = entData["collider"];
	string shader = entData["shader"];

	cout << "Loading Entity: " << id << endl;
	cout << "  Model: " << file << endl;
	cout << "  Shader: " << shader << endl;
	cout << "  Tag: " << tag << endl;
	cout << "  Collider: " << (collision ? "true" : "false") << endl;


	// Initialize new entity with lists of shapes and materials
	shared_ptr<Entity> newEntity = make_shared<Entity>();
	newEntity->model = make_shared<Model>(resourceDir + file);
	newEntity->defaultShaderName = shader;
	newEntity->tag = tag;

	// Extract textures from the texture library
	string textureID = entData["texture"];
	if (!textureID.empty()) {
		newEntity->model->loadTextureFromFile(textureLibrary[textureID]);
	}

	if (find(tagList->begin(), tagList->end(), tag) == tagList->end()) {
		tagList->push_back(tag);
	}

	if (collision == 1) {
		newEntity->collidable = true;
		collidables->push_back(newEntity);
	}
	else {
		newEntity->collidable = false;
	}

	// Import entity spatial properties
	newEntity->position.x = entData["position"][0];
	newEntity->position.y = entData["position"][1];
	newEntity->position.z = entData["position"][2];

	newEntity->rotX = entData["rotation"][0];
	newEntity->rotY = entData["rotation"][1];
	newEntity->rotZ = entData["rotation"][2];

	newEntity->scaleVec.x = entData["scale"][0];
	newEntity->scaleVec.y = entData["scale"][1];
	newEntity->scaleVec.z = entData["scale"][2];

	cout << "  Position: (" << newEntity->position.x << ", " << newEntity->position.y << ", " << newEntity->position.z << ")" << endl;
	cout << "  Rotation: (" << newEntity->rotX << ", " << newEntity->rotY << ", " << newEntity->rotZ << ")" << endl;
	cout << "  Scale: (" << newEntity->scaleVec.x << ", " << newEntity->scaleVec.y << ", " << newEntity->scaleVec.z << ")" << endl;

	for (const auto& meshData : entData["meshes"]) {
		string meshID = meshData["mesh"];
		newEntity->model->meshes[meshID].mat.lightColor = {
			meshData["material"]["lightColor"][0],
			meshData["material"]["lightColor"][1],
			meshData["material"]["lightColor"][2]
		};
		newEntity->model->meshes[meshID].mat.albedo = {
			meshData["material"]["albedo"][0],
			meshData["material"]["albedo"][1],
			meshData["material"]["albedo"][2]
		};
		newEntity->model->meshes[meshID].mat.emissivity = {
			meshData["material"]["emissivity"][0],
			meshData["material"]["emissivity"][1],
			meshData["material"]["emissivity"][2]
		};
		newEntity->model->meshes[meshID].mat.reflectance = {
			meshData["material"]["reflectance"][0],
			meshData["material"]["reflectance"][1],
			meshData["material"]["reflectance"][2]
		};
		newEntity->model->meshes[meshID].mat.roughness = meshData["material"]["roughness"];
		cout << "  Mesh: " << meshID << " with material properties loaded." << endl;
	}
	// Commit new entity to main
	(*worldentities)[id] = newEntity;
}


void ImporterExporter::loadFromFile(string path) {
	// ID-indexed library of mesh-BRDFmaterial pairs, for building entities from shape data

	string fullPath = resourceDir + path;
	ifstream saveFile(fullPath);

	if (!saveFile.is_open()) {
		cerr << "Failed to open save file at " << fullPath << endl;
		return;
	}

	json j;
	saveFile >> j;  // Load the entire JSON structure from the file

	printf("begin load from save at %s\n", (fullPath).c_str());
	// parse each line in the savefile
	if (j.contains("shaders")) {
		for (const auto& shaderData : j["shaders"]) {
			loadShader(shaderData);
		}
	}

	if (j.contains("textures")) {
		for (const auto& texData : j["textures"]) {
			loadTexture(texData);
		}
	}

	if (j.contains("entities")) {
		for (const auto& entData : j["entities"]) {
			loadEntity(entData);
		}
	}


	
	printf("end load from save\n");
}

json ImporterExporter::shadersToJson() {
	json shadersJson = json::array();

	for (const auto& shaderPair : *shaders) {
		const auto& shader = shaderPair.second;
		json shaderJson;
		shaderJson["shaderName"] = shaderPair.first;
		shaderJson["vertexShader"] = findFilename(shader->prog->getVShaderName());
		shaderJson["fragmentShader"] = findFilename(shader->prog->getFShaderName());

		shaderJson["uniforms"] = json::array();
		for (const auto& uniform : shader->prog->getUniforms()) {
			shaderJson["uniforms"].push_back(uniform);
		}

		shaderJson["attributes"] = json::array();
		for (const auto& attribute : shader->prog->getAttributes()) {
			shaderJson["attributes"].push_back(attribute);
		}

		shadersJson.push_back(shaderJson);
	}

	return shadersJson;
}

json ImporterExporter::texturesToJson() {
	json texturesJson = json::array();

	for (const auto& texturePair : textureLibrary) {
		const auto& texture = texturePair.second;
		json textureJson;
		textureJson["name"] = texturePair.first;
		textureJson["filePath"] = findFilename(texture->getFilename());

		texturesJson.push_back(textureJson);
	}

	return texturesJson;
}

json ImporterExporter::entitiesToJson() {
	json entitiesJson = json::array();

	for (const auto& entityPair : *worldentities) {
		const auto& entity = entityPair.second;
		json entityJson;
		entityJson["name"] = entityPair.first;
		entityJson["file"] = findFilename(entity->model->filePath);
		entityJson["tag"] = entity->tag;
		entityJson["collider"] = entity->collidable ? 1 : 0;
		entityJson["shader"] = entity->defaultShaderName;

		if (entity->model->extTexture) {
			entityJson["texture"] = findFilename(entity->model->extTexture->getFilename());
		}
		else {
			entityJson["texture"] = "";
		}

		entityJson["position"] = { entity->position.x, entity->position.y, entity->position.z };
		entityJson["rotation"] = { entity->rotX, entity->rotY, entity->rotZ };
		entityJson["scale"] = { entity->scaleVec.x, entity->scaleVec.y, entity->scaleVec.z };

		entityJson["meshes"] = json::array();
		for (const auto& meshPair : entity->model->meshes) {
			json meshJson;
			const auto& mesh = meshPair.second;
			meshJson["mesh"] = meshPair.first;

			json materialJson;
			materialJson["lightColor"] = { mesh.mat.lightColor.r, mesh.mat.lightColor.g, mesh.mat.lightColor.b };
			materialJson["albedo"] = { mesh.mat.albedo.r, mesh.mat.albedo.g, mesh.mat.albedo.b };
			materialJson["emissivity"] = { mesh.mat.emissivity.r, mesh.mat.emissivity.g, mesh.mat.emissivity.b };
			materialJson["reflectance"] = { mesh.mat.reflectance.r, mesh.mat.reflectance.g, mesh.mat.reflectance.b };
			materialJson["roughness"] = mesh.mat.roughness;

			meshJson["material"] = materialJson;
			entityJson["meshes"].push_back(meshJson);
		}

		entitiesJson.push_back(entityJson);
	}

	return entitiesJson;
}

string ImporterExporter::findFilename(string path) {
	size_t pos = path.find(resourceDir);
	string result = "";
	if (pos != string::npos) {
		pos += resourceDir.length(); 

		// Extract the substring from the position to the end of the string
		result = path.substr(pos);

	}

	return result;
}

int ImporterExporter::saveToFile(string outFileName) {
	json saveJson;

	saveJson["shaders"] = shadersToJson();
	saveJson["textures"] = texturesToJson();
	saveJson["entities"] = entitiesToJson();

	ofstream saveFile(resourceDir + outFileName);
	if (!saveFile.is_open()) {
		cerr << "Failed to open save file at " << outFileName << endl;
		return -1;
	}

	saveFile << setw(4) << saveJson << endl;  // Pretty print with 4-space indentation
	saveFile.close();
	cout << "World saved to file " << outFileName << endl;
	return 1;
}
