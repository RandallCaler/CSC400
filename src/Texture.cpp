#include "Texture.h"
#include "GLSL.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

Texture::Texture() :
	filename(""),
	tid(0)
{
	
}

Texture::~Texture()
{
	
}

void Texture::init(bool embedded)
{
	// Load texture
	int width, height, ncomps;
	stbi_set_flip_vertically_on_load(false);
	unsigned char* data;
	if (!embedded) {
		data = stbi_load(filename.c_str(), &width, &height, &ncomps, 0);
		if (!data) {
			cerr << filename << " not found" << endl;
		}
	}
	else {
		data = stbi_load_from_memory(buffer, static_cast<int>(bufferSize), &width, &height, &ncomps, 0);
		if (!data) {
			cerr << "texture info not found" << endl;
		}
	}
	
	
	GLenum format;
    if (ncomps == 1)
        format = GL_RED;
    else if (ncomps == 3)
        format = GL_RGB;
    else if (ncomps == 4)
        format = GL_RGBA;

	if((width & (width - 1)) != 0 || (height & (height - 1)) != 0) {
		cerr << filename << " must be a power of 2" << endl;
	}

	// Generate a texture buffer object
	glGenTextures(1, &tid);
	// Bind the current texture to be the newly generated texture object
	glBindTexture(GL_TEXTURE_2D, tid);
	// Load the actual texture data
	// Base level is 0, number of channels is 3, and border is 0.
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	// Generate image pyramid
	glGenerateMipmap(GL_TEXTURE_2D);
	// Set texture wrap modes for the S and T directions
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Set filtering mode for magnification and minimification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// Unbind
	glBindTexture(GL_TEXTURE_2D, 0);
	// Free image, since the data is now on the GPU
	stbi_image_free(data);
}

//void Texture::setWrapModes(GLint wrapS, GLint wrapT)
//{
//	// Must be called after init()
//	glBindTexture(GL_TEXTURE_2D, tid);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
//}

