#pragma once
#ifndef __Texture__
#define __Texture__

#include <glad/glad.h>
#include <string>
#include <utility>

class Texture
{
public:
	Texture();
	virtual ~Texture();
	void setFilename(const std::string &f) { filename = f; }
	void setName(const std::string &n) { name = n; }
	void init();
	void initHmap();
	void setUnit(GLint u) { unit = u; }
	GLint getUnit() const { return unit; }
	void bind(GLint handle);
	void unbind();
	void setWrapModes(GLint wrapS, GLint wrapT); // Must be called after init()
	GLint getID() const { return tid;}
	std::string filename;
	unsigned char* getData() const { return data; }
	void freeData() const;
	std::pair<int, int> getDim() const { return { width, height }; }
	std::string name;
private:
	int width;
	int height;
	GLuint tid;
	GLint unit;
	unsigned char* data;
	
};

#endif
