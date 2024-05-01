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
private:
	int width;
	int height;
	GLuint tid;
	GLint unit;
	unsigned char* data;
	
};

#endif
