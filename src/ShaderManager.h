#pragma once

#ifndef SHADER_MANAGER_H_INCLUDED
#define SHADER_MANAGER_H_INCLUDED

#include "Components.h"

struct materials;
#include "Entity.h"
#include <unordered_map>
#include <vector>
#include <string>

class Shader {
    public:
        void setModel(vec3 trans, float rotZ, float rotY, float rotX, float sc);
        void setModel(std::shared_ptr<MatrixStack> M);
        void setModel(Entity entity);
        void setModel(glm::mat4& M);

        void initShader(const std::string &v, const std::string &f);
        void initTexVars();

        void addTexture(const std::string &f);

        void setMaterial(BRDFmaterial material);

        void flip(int f);

        void setTexture(int i);
        void unbindTexture(int i);

        void setUniform(std::string uniformName);
        void setAttribute(std::string attributeName);

        
        Shader(const std::string &v, const std::string &f, bool has_tex);
        Shader();
        ~Shader();


        std::shared_ptr<Program> prog;
        bool has_texture;
	    std::vector<shared_ptr<Texture>> textures;
        std::string name;

};

class ShaderManager {
public:
    ShaderManager();
    void AddShader(shared_ptr<Shader> shader);
    void Draw(Entity& entity);
private:
    std::vector<shared_ptr<Shader>> shaderList;
    std::unordered_map<std::string, unsigned int> shaderTable;
    static unsigned int ID;
};

#endif