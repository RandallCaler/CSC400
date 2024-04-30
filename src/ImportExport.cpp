#include "ImportExport.h"

ImporterExporter::ImporterExporter(map<string, shared_ptr<Shader>>* shaders, vector<shared_ptr<Entity>>* worldentities) {
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

void ImporterExporter::loadTexture(map<string, shared_ptr<Texture>>& textures) {
	string id = readString();
	string textFile = readString();
    shared_ptr<Texture> texture = make_shared<Texture>();
    texture->setFilename(resourceDir + textFile);
    texture->init();
    texture->setUnit(0);
    texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    textures[id] = texture;
}

void ImporterExporter::loadSingleShape(map<string, pair<shared_ptr<Shape>, materials>>& shapes) {
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
				newShape->createShape(shape);
				newShape->measure();
				newShape->init();
				materials newMat = materials();
				shapes[id] = make_pair(newShape, newMat);
				break;
			}
		}
		errStr = "";
	}

	// import lighting properties
	shapes[id].second.matAmb.r = readFloat();
	shapes[id].second.matAmb.g = readFloat();
	shapes[id].second.matAmb.b = readFloat();
	
	shapes[id].second.matDif.r = readFloat();
	shapes[id].second.matDif.g = readFloat();
	shapes[id].second.matDif.b = readFloat();

	shapes[id].second.matSpec.r = readFloat();
	shapes[id].second.matSpec.g = readFloat();
	shapes[id].second.matSpec.b = readFloat();
	
	shapes[id].second.matShine = readFloat();

	printf("%s %s %s %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n", 
		id.c_str(), meshFile.c_str(), shapeName.c_str(),
		shapes[id].second.matAmb.r, shapes[id].second.matAmb.g, shapes[id].second.matAmb.b,
		shapes[id].second.matDif.r, shapes[id].second.matDif.g, shapes[id].second.matDif.b,
		shapes[id].second.matSpec.r, shapes[id].second.matSpec.g, shapes[id].second.matSpec.b,
		shapes[id].second.matShine);
}

void ImporterExporter::loadEntity(map<string, pair<shared_ptr<Shape>, materials>>& shapes, map<string, shared_ptr<Texture>>& textures) {
	// compose an entity from the savefile entry in the buffer
	
	string id = readString();

	string shader = readString();

	int numShapes = readInt();
	printf("%s %s %i ", id.c_str(), shader.c_str(), numShapes);

	// lists of meshes and material properties
	vector<shared_ptr<Shape>> entityShapes;
	vector<shared_ptr<Texture>> entityTextures;
	vector<materials> entityMats;
	
	// extract meshes and materials from the shape library
	for (int i = 0; i < numShapes; i++) {
		string shapeID = readString();
		printf("%s ", shapeID.c_str());
		entityShapes.push_back(shapes[shapeID].first);
		entityMats.push_back(shapes[shapeID].second);
	}

	int numTextures = readInt();
	printf("%i ", numTextures);
	// extract textures from the texture library
	for (int i = 0; i < numTextures; i++) {
		string textID = readString();
		printf("%s ", textID.c_str());
		entityTextures.push_back(textures[textID]);
	}

	// initialize new entity with lists of meshes and materials
	shared_ptr<Entity> newEntity = make_shared<Entity>();
	newEntity->initEntity(entityShapes, entityTextures);
	newEntity->material = entityMats;
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
	(*worldentities).push_back(newEntity);
}

void ImporterExporter::loadFromFile(string path) {
	// ID-indexed library of mesh-material pairs, for building entities from shape data
	map<string, pair<shared_ptr<Shape>, materials>> shapeLibrary;
	map<string, shared_ptr<Texture>> textureLibrary;

	printf("begin load from save at %s\n", (resourceDir+path).c_str());
	ifstream saveFile(resourceDir + path);

	// parse each line in the savefile
	while (getline(saveFile, buffer)) {
		// interpret line according to syntax designator at the start of the line
		char type = buffer[0];
		buffer = buffer.substr(2);
		switch (type) {
			case SHADER_FLAG:
				loadShader();
				break;
			case SHAPE_FLAG:
				loadSingleShape(shapeLibrary);
				break;
			case ENTITY_FLAG:
				loadEntity(shapeLibrary, textureLibrary);
				break;
			case TEXTURE_FLAG:
				loadTexture(textureLibrary);
		}
	}
	printf("end load from save\n");
}