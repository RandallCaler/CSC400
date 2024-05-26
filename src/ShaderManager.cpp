
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Entity.h"

#include "ShaderManager.h"


using namespace std;
using namespace glm;



void Shader::initShader(const std::string &v, const std::string &f) {
    prog = make_shared<Program>();
	prog->setVerbose(true);
	prog->setShaderNames(v, f);
	prog->init();
}

void Shader::initTexVars() {
	prog->addUniform("flip");
	prog->addUniform("Texture0");
}

void Shader::setModel(vec3 trans, float rotZ, float rotY, float rotX, float sc) {
    mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
  	mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
  	mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
	mat4 RotZ = glm::rotate( glm::mat4(1.0f), rotZ, vec3(0, 0, 1));
  	mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(sc));
  	mat4 ctm = Trans*RotX*RotY*RotZ*ScaleS;
  	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
}

void Shader::setModel(Entity entity) {
    mat4 Trans = glm::translate(glm::mat4(1.0f), entity.position);
    mat4 RotX = glm::rotate(glm::mat4(1.0f), entity.rotX, vec3(1, 0, 0));
    mat4 RotY = glm::rotate(glm::mat4(1.0f), entity.rotY, vec3(0, 1, 0));
    mat4 RotZ = glm::rotate(glm::mat4(1.0f), entity.rotZ, vec3(0, 0, 1));
    mat4 ScaleS = glm::scale(glm::mat4(1.0f), entity.scaleVec);
    mat4 ctm = Trans * RotX * RotY * RotZ * ScaleS;
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
}

void Shader::setModel(std::shared_ptr<MatrixStack> M) {
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
}

void Shader::setModel(glm::mat4& M) {
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M));
}

void Shader::addTexture(const std::string &f) {
    shared_ptr<Texture> texture0 = make_shared<Texture>();
    texture0->setFilename(f);
    texture0->init(false);
    texture0->setUnit(0);
    texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    textures.push_back(texture0);
}

void Shader::setMaterial(BRDFmaterial material) {
    if (prog->getUniform("lightColor") > 0) {
        glUniform3f(prog->getUniform("lightColor"), material.lightColor.r, material.lightColor.g, material.lightColor.b);
    }
    if (prog->getUniform("albedo") > 0) {
        glUniform3f(prog->getUniform("albedo"), material.albedo.r, material.albedo.g, material.albedo.b);
    }
    if (prog->getUniform("reflectance") > 0) {
        glUniform3f(prog->getUniform("reflectance"), material.reflectance.r, material.reflectance.g, material.reflectance.b);
    }
    if (prog->getUniform("emissivity") > 0) {
        glUniform3f(prog->getUniform("emissivity"), material.emissivity.r, material.emissivity.g, material.emissivity.b);
    }
    if (prog->getUniform("roughness") > 0) {
        glUniform1f(prog->getUniform("roughness"), material.roughness);
    }
}

void Shader::flip(int f) {
	glUniform1i(prog->getUniform("flip"), f);
}


void Shader::setUniform(std::string uniformName) {
	prog->addUniform(uniformName);
}

void Shader::setAttribute(std::string attributeName) {
	prog->addAttribute(attributeName);
}

Shader::Shader(const std::string &v, const std::string &f, bool has_tex) {
    initShader(v, f);
    has_texture = has_tex;
}

Shader::Shader() {
    ;
}

Shader::~Shader() {
    ;
}


unsigned int ShaderManager::ID = 0;

ShaderManager::ShaderManager() : shaderList{}, shaderTable{} {}
void ShaderManager::AddShader(shared_ptr<Shader> shader) {
    shaderList.push_back(shader);
    shaderTable[shader->name] = ID++;
}
void ShaderManager::Draw(Entity& entity) {
    auto shaderptr = shaderList[shaderTable[entity.defaultShaderName]];
}