#include "ImportExport.h"

ImporterExporter::ImporterExporter(map<string, shared_ptr<Shader>>* shaders, map<string, shared_ptr<Entity>>* worldentities) {
	// mutable references to main's shaders and entities
	this->shaders = shaders;
	this->worldentities = worldentities;
}

ImporterExporter::~ImporterExporter() {
    ;
}

string ImporterExporter::readString() {
	// get the next substring from the savefile
	delimit = buffer.find(DELIMITER);
	string out = buffer.substr(0, delimit);
	buffer = buffer.substr(delimit + 1);
	return out;
}

int ImporterExporter::readInt() {
	// interpret the next substring of the savefile as an int
	delimit = buffer.find(DELIMITER);
	int out = strtol(buffer.substr(0, delimit).c_str(), NULL, 10);
	buffer = buffer.substr(delimit + 1);
	return out;
}

float ImporterExporter::readFloat() {
	//interpret the next substring of the savefile as a float
	delimit = buffer.find(DELIMITER);
	float out = stof(buffer.substr(0, delimit));
	buffer = buffer.substr(delimit + 1);
	return out;
}

void ImporterExporter::loadShader() {
	// construct a shader from the savefile entry in the buffer
	// add the shader to main's shader list

	string id = readString();
	string vertexSFile = readString();
	string fragSFile = readString();

	// shader initialization
	shared_ptr<Shader> shader = make_shared<Shader>(resourceDir + vertexSFile, resourceDir + fragSFile, false);
	
	// add uniforms to shader definition
	int numUniforms = readInt();
	printf("%s %s %s %i", 
		id.c_str(), vertexSFile.c_str(), fragSFile.c_str(), numUniforms);

	for (int i = 0; i < numUniforms; i++) {
		string uniform = readString();
		shader->setUniform(uniform);
		printf("%s ", uniform.c_str());
	}

	// add attributes to shader definition
	int numAttributes = readInt();
	printf("%i ", numAttributes);

	for (int i = 0; i < numAttributes; i++) {
		string attribute = readString();
		shader->setAttribute(attribute);
		printf("%s ", attribute.c_str());
	}
	printf("\n");

	// commit shader to main
	(*shaders)[id] = shader;
}

void ImporterExporter::loadTexture() {
	string id = readString();
	string textFile = readString();
    shared_ptr<Texture> texture = make_shared<Texture>();
    texture->setFilename(resourceDir + textFile);
    texture->setName(id);
    texture->setName(id);
    texture->init();
    texture->setUnit(0);
    texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    textureLibrary[id] = texture;
    textureLibrary[id] = texture;
}

void ImporterExporter::loadSingleShape() {
	// extract a mesh and lighting properties from the savefile entry in the buffer
	
	// tinyObj overhead
	vector<tinyobj::shape_t> TOshapes;
	vector<tinyobj::material_t> objMaterials;
	string errStr;

	string id = readString();
	string meshFile = readString();
	string shapeName = readString();

	// TinyObj handles file read
	bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDir + meshFile).c_str());
	if (!rc) {
		cerr << errStr << endl;
	}
	else {
		for(tinyobj::shape_t shape: TOshapes) {
			if (shape.name == shapeName) {
				shared_ptr<Shape> newShape = make_shared<Shape>();
				newShape->createShape(shape, resourceDir + meshFile, id);
				newShape->measure();
				newShape->init();
				BRDFmaterial newMat = material();
				shapeLibrary[id] = make_pair(newShape, newMat);
				break;
			}
		}
		errStr = "";
	}

	// import lighting properties
	shapeLibrary[id].second.lightColor.r = readFloat();
	shapeLibrary[id].second.lightColor.g = readFloat();
	shapeLibrary[id].second.lightColor.b = readFloat();

	shapeLibrary[id].second.albedo.r = readFloat();
	shapeLibrary[id].second.albedo.g = readFloat();
	shapeLibrary[id].second.albedo.b = readFloat();
	
	shapeLibrary[id].second.emissivity.r = readFloat();
	shapeLibrary[id].second.emissivity.g = readFloat();
	shapeLibrary[id].second.emissivity.b = readFloat();

	shapeLibrary[id].second.reflectance.r = readFloat();
	shapeLibrary[id].second.reflectance.g = readFloat();
	shapeLibrary[id].second.reflectance.b = readFloat();
	
	shapeLibrary[id].second.roughness = readFloat();

	printf("%s %s %s %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n", 
		id.c_str(), meshFile.c_str(), shapeName.c_str(),
		shapeLibrary[id].second.lightColor.r, shapeLibrary[id].second.lightColor.g, shapeLibrary[id].second.lightColor.b,
		shapeLibrary[id].second.albedo.r, shapeLibrary[id].second.albedo.g, shapeLibrary[id].second.albedo.b,
		shapeLibrary[id].second.emissivity.r, shapeLibrary[id].second.emissivity.g, shapeLibrary[id].second.emissivity.b,
		shapeLibrary[id].second.reflectance.r, shapeLibrary[id].second.reflectance.g, shapeLibrary[id].second.reflectance.b,
		shapeLibrary[id].second.roughness);
}

void ImporterExporter::loadEntity() {
	// compose an entity from the savefile entry in the buffer
	
	string id = readString();

	string shader = readString();

	int numShapes = readInt();
	printf("%s %s %i ", id.c_str(), shader.c_str(), numShapes);

	// lists of meshes and BRDFmaterial properties
	vector<shared_ptr<Shape>> entityShapes;
	vector<shared_ptr<Texture>> entityTextures;
	vector<BRDFmaterial>  entityMats;
	
	// extract meshes and materials from the shape library
	for (int i = 0; i < numShapes; i++) {
		string shapeID = readString();
		printf("%s ", shapeID.c_str());
		entityShapes.push_back(shapeLibrary[shapeID].first);
		entityMats.push_back(shapeLibrary[shapeID].second);
	}

	int numTextures = readInt();
	printf("%i ", numTextures);
	// extract textures from the texture library
	for (int i = 0; i < numTextures; i++) {
		string textID = readString();
		printf("%s ", textID.c_str());
		entityTextures.push_back(textureLibrary[textID]);
	}

	// initialize new entity with lists of meshes and materials
	shared_ptr<Entity> newEntity = make_shared<Entity>();
	newEntity->initEntity(entityShapes, entityTextures);
	newEntity->materials = entityMats;
	newEntity->defaultShaderName = shader;

	// import entity spatial properties
	newEntity->position.x = readFloat();
	newEntity->position.y = readFloat();
	newEntity->position.z = readFloat();

	newEntity->rotX = readFloat();
	newEntity->rotY = readFloat();
	newEntity->rotZ = readFloat();

	float scaleX = readFloat();
	newEntity->scaleVec.x = scaleX;
	newEntity->scale = scaleX;

	newEntity->scaleVec.y = readFloat();
	newEntity->scaleVec.z = readFloat();
	
	printf("%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n",
		newEntity->position.x, newEntity->position.y, newEntity->position.z,
		newEntity->rotX, newEntity->rotY, newEntity->rotZ,
		newEntity->scaleVec.x, newEntity->scaleVec.y, newEntity->scaleVec.z);
	
	// commit new entity to main
	(*worldentities)[id] = newEntity;
}

void ImporterExporter::loadFromFile(string path) {
	// ID-indexed library of mesh-BRDFmaterial pairs, for building entities from shape data

	printf("begin load from save at %s\n", (resourceDir+path).c_str());
	ifstream saveFile(resourceDir + path);

	// parse each line in the savefile
	while (getline(saveFile, buffer)) {
		cout << "this happened" << endl;
		// interpret line according to syntax designator at the start of the line
		char type = buffer[0];
		buffer = buffer.substr(2);
		switch (type) {
			case SHADER_FLAG:
				loadShader();
				break;
			case SHAPE_FLAG:
				loadSingleShape();
				break;
			case ENTITY_FLAG:
				loadEntity();
				break;
			case TEXTURE_FLAG:
				loadTexture();
		}
	}
	printf("end load from save\n");
}

// Shaders: 1 shaderID vertexSFile fragSFile numUniforms [uniform1...] numAttributes [attribute1...]
string ImporterExporter::shadersToText(){
	string result = "";
	int shaderCount = 1;

	// iterate through every shader and convert its properties to a string
	for (auto shaderIter = shaders->begin(); shaderIter != shaders->end(); shaderIter++) {
        string tag = "1 " + shaderIter->first + ' ';
        string files = "/" + findFilename(shaderIter->second->prog->getVShaderName()) + " /" 
			+ findFilename(shaderIter->second->prog->getFShaderName()) + " ";

		// process the uniforms
        map<string, GLint> uniRef = shaderIter->second->prog->getUniforms();
        string uniforms = to_string(uniRef.size()) + " ";
        for (auto iter = uniRef.begin(); iter != uniRef.end(); iter++) {
            uniforms += iter->first + " ";
        }

		//process the attributes
		map<string, GLint> attRef = shaderIter->second->prog->getAttributes();
        string attributes = to_string(attRef.size()) + " ";
        for (auto iter = attRef.begin(); iter != attRef.end(); iter++) {
            attributes += iter->first + " ";
        }

		result = result + tag + files + uniforms + attributes + '\n';
		shaderCount++;
    }

    return result;
}

// Shapes: 2 shapeID objFile objShapeName matAmbX matAmbY matAmbZ matDifX matDifY matDifZ matSpecX matSpecY matSpecZ matShine
string ImporterExporter::shapesToText(){
	string result = "";

	//create set of shapes w/ materials from entities
	for(auto iter = shapeLibrary.begin(); iter != shapeLibrary.end(); iter++){ // map<string, pair<shared_ptr<Shape>, BRDFmaterial >
		string tag = "2 " + iter->first + ' ' + findFilename(iter->second.first.get()->getFilePath()) + ' ' 
			+ iter->second.first.get()->getShapeName() + ' ';
		
		BRDFmaterial shapeMat = iter->second.second;
		string mats = to_string(shapeMat.lightColor.r) + ' ' + to_string(shapeMat.lightColor.g) + ' ' + to_string(shapeMat.lightColor.b) + ' ' +
					  to_string(shapeMat.albedo.r) + ' ' + to_string(shapeMat.albedo.g) + ' ' + to_string(shapeMat.albedo.b) + ' ' +
					  to_string(shapeMat.reflectance.r) + ' ' + to_string(shapeMat.reflectance.g) + ' ' + to_string(shapeMat.reflectance.b) + ' ' + 
					  to_string(shapeMat.emissivity.r) + ' ' + to_string(shapeMat.emissivity.g) + ' ' + to_string(shapeMat.emissivity.b) + ' ' + 
					  to_string(shapeMat.roughness) + '\n';
		
		result = result + tag + mats;
	}

	return result;
}

// Entities: 3 entityID numShapes [shapeID1...] transX transY transZ rotX rotY rotZ scaleX scaleY scaleZ
string ImporterExporter::entitiesToText(){
	string result = "";
	for (auto entityIter = worldentities->begin(); entityIter != worldentities->end(); entityIter++) {
		string entityInfo = "3 " + entityIter->first + " " + entityIter->second->defaultShaderName + " " + to_string(entityIter->second->objs.size()) + " ";
		for (auto shape : entityIter->second->objs) {
			entityInfo += shape->getName() + " ";
		}

		entityInfo += to_string(entityIter->second->textures.size()) + " ";
		if (entityIter->second->textures.size() != 0) {
			for (auto texture : entityIter->second->textures) {
				entityInfo += texture->name + " ";
			}
		}

		entityInfo += to_string(entityIter->second->position.x) + " " + to_string(entityIter->second->position.y) + " " + to_string(entityIter->second->position.z) + " "
			+ to_string(entityIter->second->rotX) + " " + to_string(entityIter->second->rotY) + " " + to_string(entityIter->second->rotZ) + " " 
			+ to_string(entityIter->second->scaleVec.x) + " " + to_string(entityIter->second->scaleVec.y) + " " + to_string(entityIter->second->scaleVec.z) + "\n";
		result += entityInfo;
	}

	return result;
}

// Entities: 3 entityID numShapes [shapeID1...] transX transY transZ rotX rotY rotZ scaleX scaleY scaleZ
string ImporterExporter::texturesToText() {
	string result = "";
	for (auto textIter = textureLibrary.begin(); textIter != textureLibrary.end(); textIter++) {
		string textInfo = "4 " + textIter->first + " " + findFilename(textIter->second->filename) + '\n';
		result += textInfo;
	}

	return result;
}

string ImporterExporter::findFilename(string path) {
	size_t pos = path.find(resourceDir);
	string result = "";
	if (pos != std::string::npos) {
		pos += resourceDir.length(); 

		// Extract the substring from the position to the end of the string
		result = path.substr(pos);

	}

	return result;
}

int ImporterExporter::saveToFile(string outFileName){
	std::ofstream outFile(resourceDir + outFileName);

	if(outFile.is_open()){
		outFile << shadersToText() << texturesToText() << shapesToText() << entitiesToText();
		outFile.close();
		cout << "World saved to file " << outFileName << endl;
	}
	else {
		std::cerr << "Error opening file from exporter." << endl;
	}
	return 1;
}