#pragma once
#ifndef __Texture__
#define __Texture__

#include <glad/glad.h>
#include <string>
#include <utility>

using namespace std;

class Texture
{
public:
	Texture();
	virtual ~Texture();
	void setFilename(const std::string &f) { filename = f; }
	string getFilename() { return filename; }
	void setName(const std::string &n) { name = n; }
	void setBuffer(unsigned char* b, int size) { buffer = b; bufferSize = size; }
	void init(bool embedded);
	void initHmap();
	void setUnit(GLint u) { unit = u; }
	GLint getUnit() const { return unit; }
	void setType(string t) { type = t; }
	string getType() const { return type; }
	void bind(GLint handle);
	void unbind();
	void setWrapModes(GLint wrapS, GLint wrapT); // Must be called after init()
	GLint getID() const { return tid;}
	void setID(GLint id) { tid = id; }
	std::string filename;
	unsigned char* getData() const { return data; }
	//void freeData() const;
	std::pair<int, int> getDim() const { return { width, height }; }
	std::string name;
private:
	int width;
	int height;
	GLuint tid;
	GLint unit;
	string type;
	unsigned char* buffer;
	int bufferSize;
	unsigned char* data;
	
};

#endif
