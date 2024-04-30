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
    texture->init();
    texture->setUnit(0);
    texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
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
				newShape->createShape(shape);
				newShape->measure();
				newShape->init();
				material newMat = material();
				shapeLibrary[id] = make_pair(newShape, newMat);
				break;
			}
		}
		errStr = "";
	}

	// import lighting properties
	shapeLibrary[id].second.amb.r = readFloat();
	shapeLibrary[id].second.amb.g = readFloat();
	shapeLibrary[id].second.amb.b = readFloat();
	
	shapeLibrary[id].second.dif.r = readFloat();
	shapeLibrary[id].second.dif.g = readFloat();
	shapeLibrary[id].second.dif.b = readFloat();

	shapeLibrary[id].second.spec.r = readFloat();
	shapeLibrary[id].second.spec.g = readFloat();
	shapeLibrary[id].second.spec.b = readFloat();
	
	shapeLibrary[id].second.shine = readFloat();

	printf("%s %s %s %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n", 
		id.c_str(), meshFile.c_str(), shapeName.c_str(),
		shapeLibrary[id].second.amb.r, shapeLibrary[id].second.amb.g, shapeLibrary[id].second.amb.b,
		shapeLibrary[id].second.dif.r, shapeLibrary[id].second.dif.g, shapeLibrary[id].second.dif.b,
		shapeLibrary[id].second.spec.r, shapeLibrary[id].second.spec.g, shapeLibrary[id].second.spec.b,
		shapeLibrary[id].second.shine);
}

void ImporterExporter::loadEntity() {
	// compose an entity from the savefile entry in the buffer
	
	string id = readString();

	string shader = readString();

	int numShapes = readInt();
	printf("%s %s %i ", id.c_str(), shader.c_str(), numShapes);

	// lists of meshes and material properties
	vector<shared_ptr<Shape>> entityShapes;
	vector<shared_ptr<Texture>> entityTextures;
	vector<material> entityMats;
	
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
	// ID-indexed library of mesh-material pairs, for building entities from shape data

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
        string tag = "1 s" + to_string(shaderCount) + ' ';
        string files = "/" + shaderIter->second->prog->getVShaderName() + " /" + shaderIter->second->prog->getFShaderName() + " ";

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
	set<shapewmat> shapes;
	for(int i = 0; i < worldentities->size(); i++){
		
	}

	return result;
}

// Entities: 3 entityID numShapes [shapeID1...] transX transY transZ rotX rotY rotZ scaleX scaleY scaleZ
string ImporterExporter::entitiesToText(){
	string result = "";

	//for(auto entity = )

	return result;
}

void ImporterExporter::saveToFile(string outFileName){
	std::ofstream outFile(outFileName);

	if(outFile.is_open()){
		outFile << shadersToText();
		outFile.close();
		cout << "outFile has been written and closed." << endl;
	}
	else {
		std::cerr << "Error opening file from exporter." << endl;
	}
}