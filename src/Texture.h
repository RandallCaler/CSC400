#pragma once
#ifndef __Texture__
#define __Texture__

#include <glad/glad.h>
#include <string>

class Texture
{
public:
	Texture();
	virtual ~Texture();
	void setFilename(const std::string &f) { filename = f; }
	void init();
	void setUnit(GLint u) { unit = u; }
	GLint getUnit() const { return unit; }
	void bind(GLint handle);
	void unbind();
	void setWrapModes(GLint wrapS, GLint wrapT); // Must be called after init()
	GLint getID() const { return tid;}
private:
	std::string filename;
	int width;
	int height;
	GLuint tid;
	GLint unit;
	
};

#endif
