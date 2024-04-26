#pragma once

#ifndef IMPORT_EXPORT_H_INCLUDED
#define IMPORT_EXPORT_H_INCLUDED

#include <string>
#include <map>
#include <fstream>
#include "Shape.h"
#include "Entity.h"
#include "ShaderManager.h"

#define DELIMITER ' '
#define SHADER_FLAG '1'
#define SHAPE_FLAG '2'
#define ENTITY_FLAG '3'

// Where the resources are loaded from
extern string resourceDir;

class ImporterExporter {
    public:
        ImporterExporter(map<string, shared_ptr<Shader>>* shaders, vector<shared_ptr<Entity>>* worldentities);
        ~ImporterExporter();
        
        string readString();
        int readInt();
        float readFloat();

        void loadShader();
        void loadSingleShape(map<string, pair<shared_ptr<Shape>, materials>>& shapes);
        void loadEntity(map<string, pair<shared_ptr<Shape>, materials>>& shapes);
        void loadFromFile(string path);

    private:
        map<string, shared_ptr<Shader>>* shaders; // reference to main shader list
        vector<shared_ptr<Entity>>* worldentities; // reference to main entity list
	    string buffer; // input data stream
        size_t delimit; // reusable tracker for delimiter locations
};

#endif