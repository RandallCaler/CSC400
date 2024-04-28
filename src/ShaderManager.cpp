
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
	prog->addUniform("P");
	prog->addUniform("V");
	prog->addUniform("M");
	prog->addUniform("MatAmb");
	prog->addUniform("MatDif");
	prog->addUniform("MatSpec");
	prog->addUniform("MatShine");
	prog->addUniform("lightPos");
	prog->addAttribute("vertPos");
	prog->addAttribute("vertNor");
	prog->addAttribute("vertTex");
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

void Shader::setModel(std::shared_ptr<MatrixStack> M) {
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
}

void Shader::setModel(glm::mat4& M) {
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M));
}

void Shader::addTexture(const std::string &f) {
    if (has_texture) {
        shared_ptr<Texture> texture0 = make_shared<Texture>();
        texture0->setFilename(f);
        texture0->init();
        texture0->setUnit(0);
        texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        textures.push_back(texture0);
    }
}

void Shader::setMaterial(material material) {
    glUniform3f(prog->getUniform("MatAmb"), material.amb.r, material.amb.g, material.amb.b);
	glUniform3f(prog->getUniform("MatDif"), material.dif.r, material.dif.g, material.dif.b);
	glUniform3f(prog->getUniform("MatSpec"), material.spec.r, material.spec.g, material.spec.b);
	glUniform1f(prog->getUniform("MatShine"), material.shine);
}

void Shader::flip(int f) {
	glUniform1i(prog->getUniform("flip"), f);
}

void Shader::setTexture(int i) {
    textures[i]->bind(prog->getUniform("Texture0"));
}

void Shader::unbindTexture(int i) {
    textures[i]->unbind();
}


Shader::Shader(const std::string &v, const std::string &f, bool has_tex) {
    initShader(v, f);
    if (has_tex) {
        has_texture = true;
        initTexVars();
    }
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
    auto shaderptr = shaderList[shaderTable[entity.shaderName]];
}