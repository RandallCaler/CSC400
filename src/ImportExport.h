#pragma once

#ifndef IMPORT_EXPORT_H_INCLUDED
#define IMPORT_EXPORT_H_INCLUDED

#include <map>
#include <set>
#include <fstream>
#include <iostream>
#include "Shape.h"
#include "Entity.h"
#include "Texture.h"
#include "ShaderManager.h"

#define DELIMITER ' '
#define SHADER_FLAG '1'
#define SHAPE_FLAG '2'
#define ENTITY_FLAG '3'
#define TEXTURE_FLAG '4'

// Where the resources are loaded from
extern string resourceDir;

typedef struct shapewmat {
    std::shared_ptr<Shape> shape;
    materials mats;
} shapewmat;

class ImporterExporter {
    public:
        ImporterExporter(map<string, shared_ptr<Shader>>* shaders, map<string, shared_ptr<Entity>>* worldentities);
        ~ImporterExporter();
        
        string readString();
        int readInt();
        float readFloat();

        void loadShader();
        void loadTexture();
        void loadSingleShape();
        void loadEntity();
        void loadFromFile(string path);

        string shadersToText();
        string shapesToText();
        string entitiesToText();
        void saveToFile(string path);

    private:
        map<string, pair<shared_ptr<Shape>, material>> shapeLibrary;
        map<string, shared_ptr<Texture>> textureLibrary;
        map<string, shared_ptr<Shader>>* shaders; // reference to main shader list
        map<string, shared_ptr<Entity>>* worldentities; // reference to main entity list
        map<string, pair<shared_ptr<Shape>, material>> shapeLibrary;
	    map<string, shared_ptr<Texture>> textureLibrary;
	    string buffer; // input data stream
        size_t delimit; // reusable tracker for delimiter locations
};

#endif