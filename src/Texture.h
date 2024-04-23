#pragma once
#ifndef __Texture__
#define __Texture__

#include <glad/glad.h>
#include <string>

using namespace std;
class Texture
{
public:
	Texture();
	virtual ~Texture();
	void setFilename(const string &f) { filename = f; }
	string getFilename() { return filename; }
	void setBuffer(unsigned char* b, int size) { buffer = b; bufferSize = size;}
	void init(bool embedded);
	void setType(string t) { type = t; }
	string getType() const { return type; }
	//void setWrapModes(GLint wrapS, GLint wrapT); // Must be called after init()
	GLint getID() const { return tid;}
	void setID(GLint id) { tid = id;}
private:
	string filename;
	unsigned char* buffer;
	int bufferSize;
	string type;
	GLuint tid;	
};

#endif
