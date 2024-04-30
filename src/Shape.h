
#pragma once

#ifndef LAB471_SHAPE_H_INCLUDED
#define LAB471_SHAPE_H_INCLUDED

#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader/tiny_obj_loader.h>

class Program;


class Shape
{

public:

	void createShape(tinyobj::shape_t & shape);
	void init();
	void generateNormals();
	void measure();
	void drawInit(const std::shared_ptr<Program> prog) const;
	void drawClean() const;
	void draw(const std::shared_ptr<Program> prog) const;
	void drawInstanced(const std::shared_ptr<Program> prog, int count) const;

	glm::vec3 min = glm::vec3(0);
	glm::vec3 max = glm::vec3(0);

private:

	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	unsigned int eleBufID = 0;
	unsigned int posBufID = 0;
	unsigned int norBufID = 0;
	unsigned int texBufID = 0;
	unsigned int vaoID = 0;

};

#endif // LAB471_SHAPE_H_INCLUDED
