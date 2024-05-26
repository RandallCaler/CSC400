#pragma once

#ifndef IMPORT_EXPORT_H_INCLUDED
#define IMPORT_EXPORT_H_INCLUDED

#include <map>
#include <set>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "Model.h"
#include "Entity.h"
#include "Texture.h"
#include "ShaderManager.h"

using json = nlohmann::json;

// Where the resources are loaded from
extern string resourceDir;

class ImporterExporter {
    public:
        ImporterExporter(map<string, shared_ptr<Shader>>* shaders, map<string, shared_ptr<Entity>>* worldentities, vector<string>* tagList, vector<shared_ptr<Entity>>* collidables);
        ~ImporterExporter();
        
        void loadShader(const json& shaderData);
        void loadTexture(const json& texData);
        void loadEntity(const json& entData);
        void loadFromFile(string path);

        json shadersToJson();
        json entitiesToJson();
        json texturesToJson();
        string findFilename(string path);
        int saveToFile(string path);

    private:
        map<string, shared_ptr<Shader>>* shaders; // reference to main shader list
        map<string, shared_ptr<Entity>>* worldentities; // reference to main entity list
        map<string, shared_ptr<Model>>* modelList;
        vector<string>* tagList; // reference to main tag list
        vector<shared_ptr<Entity>>* collidables; // reference to main collidables
	    map<string, shared_ptr<Texture>> textureLibrary;    
	    string buffer; // input data stream
        size_t delimit; // reusable tracker for delimiter locations
};

#endif