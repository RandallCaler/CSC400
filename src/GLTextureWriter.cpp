#include "GLTextureWriter.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <iostream>
#include <glad/glad.h>


/**
 * Retrieve the width of the texture
 * @return the texture width, in pixels.
 */
int getTextureWidth()
{
	GLint texWidth; 
	glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&texWidth);
	return texWidth;
}
/**
 * Retrieve the height of the texture
 * @return the texture height, in pixels.
 */
int getTextureHeight()
{
	GLint texHeight; 
	glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&texHeight);
	return texHeight;
}

/**
 * Retrieve data from a bound buffer
 */
void getData(void * dataBuffer, GLenum format, GLenum type)
{

	glGetTexImage(GL_TEXTURE_2D,
		0,
		format,
		type,
		dataBuffer);
	//Flip data buffer
	
}
/**
 * Write a texture to a file
 * @param  texture  a shared pointer to a texture
 * @param  fileName the name of the file
 * @return          true if successfull
 */
bool GLTextureWriter::WriteImage(std::shared_ptr<Texture> texture, std::string fileName)
{
	return WriteImage(texture->getID(), fileName);
}
/**
 * Write a texture to a file
 * @param  texture a texture object
 * @param  imgName the filename to write to
 * @return         true if successful.
 */
bool GLTextureWriter::WriteImage(const Texture &  texture, std::string imgName)
{
	return WriteImage(texture.getID(), imgName);
}

/**
 * Main logic of this code taken from stb_image.h
 * @param imgData Image data to flip
 * @param width   width of the image
 * @param height  height of the image
 */
void flip_buffer(char * imgData, int width, int height )
{

	int w =width, h = height;
	int depth = 3;
	int row,col,z;
	char temp;


	for (row = 0; row < (h>>1); row++) {
		for (col = 0; col < w; col++) {
			for (z = 0; z < depth; z++) {
				temp = imgData[(row * w + col) * depth + z];
				imgData[(row * w + col) * depth + z] = imgData[((h - row - 1) * w + col) * depth + z];
				imgData[((h - row - 1) * w + col) * depth + z] = temp;
			}
		}
	}
}

bool GLTextureWriter::WriteImage(GLint tid, std::string imgName)
{
	//Backup old openGL state.
	GLint backupBoundTexture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &backupBoundTexture);
	//Bind texture to buffer
	glBindTexture(GL_TEXTURE_2D,tid);
	//Retrieve width and height
	int txWidth = getTextureWidth();
	int txHeight = getTextureHeight();
	//Allocate buffer
	char * dataBuffer = new char[txWidth*txHeight*3];
	//Get data from Opengl
	getData(dataBuffer, GL_RGB, GL_UNSIGNED_BYTE);
	//Flip data for output
	flip_buffer(dataBuffer,txWidth,txHeight);
	//Write image to PNG
	int res =  stbi_write_png(imgName.c_str(), txWidth, txHeight, 3, dataBuffer, sizeof(char)*3*txWidth);
	if(!res)
	{
		std::cerr << "Could not write to  " << imgName << std::endl;
	}
	//Cleanup
	delete [] dataBuffer;
	//Bind old texture
	glBindTexture(GL_TEXTURE_2D,backupBoundTexture);
	return res;


}
