#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

#include "Program.h"
#include "Texture.h"

#include <string>
#include <vector>
#include <iostream>
using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
	//bone indexes which will influence this vertex
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float m_Weights[MAX_BONE_INFLUENCE];
};

typedef struct color {
    float r;
    float g;
    float b;
} color;


typedef struct BRDFmaterial {
    color lightColor;
    color albedo;
    color reflectance;
    color emissivity;
    float roughness;
} material;


class Mesh {
public:
    // mesh Data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<shared_ptr<Texture>>  textures;
    string name;
    unsigned int VAO;
    glm::vec3 min = glm::vec3(0);
    glm::vec3 max = glm::vec3(0);
    material mat;

    Mesh(){}

    // constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<shared_ptr<Texture>>  textures, string name)
        : mat({
            {1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f},
            {0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 0.0f},
            0.0f
            })
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        this->name = name;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
        measure();
    }

    void measure()
    {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;

        minX = minY = minZ = (std::numeric_limits<float>::max)();
        maxX = maxY = maxZ = -(std::numeric_limits<float>::max)();

        //Go through all vertices to determine min and max of each dimension
        for (size_t v = 0; v < vertices.size(); v++)
        {
            if (vertices[v].Position.x < minX) minX = vertices[v].Position.x;
            if (vertices[v].Position.x > maxX) maxX = vertices[v].Position.x;

            if (vertices[v].Position.y < minY) minY = vertices[v].Position.y;
            if (vertices[v].Position.y > maxY) maxY = vertices[v].Position.y;

            if (vertices[v].Position.z < minZ) minZ = vertices[v].Position.z;
            if (vertices[v].Position.z > maxZ) maxZ = vertices[v].Position.z;
        }

        min.x = minX;
	    min.y = minY;
	    min.z = minZ;
	    max.x = maxX;
	    max.y = maxY;
	    max.z = maxZ;
    }

    // render the mesh
    void Draw(const shared_ptr<Program> prog)
    {
        // bind appropriate textures
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        unsigned int num = 0;
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before 
            glUniform1i(prog->getUniform("Texture0"), i);
            glBindTexture(GL_TEXTURE_2D, textures[i]->getID());
            //string number;
            //string name = textures[i]->getType();
            //if (name == "texture_diffuse")
            //    number = std::to_string(diffuseNr++);
            //else if (name == "texture_specular")
            //    number = std::to_string(specularNr++); // transfer unsigned int to string
            //else if (name == "texture_normal")
            //    number = std::to_string(normalNr++); // transfer unsigned int to string
            //else if (name == "texture_height")
            //    number = std::to_string(heightNr++); // transfer unsigned int to string
            //else {
            //    glUniform1i(prog->getUniform("Texture0"), i);
            //    glBindTexture(GL_TEXTURE_2D, textures[i]->getID());
            //    continue;
            //}

            //// retrieve texture number (the N in diffuse_textureN)
            //// now set the sampler to the correct texture unit
            //glUniform1i(prog->getUniform(name + number), i);
            //// and finally bind the texture
            //glBindTexture(GL_TEXTURE_2D, textures[i]->getID());
        }

        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        for (unsigned int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
        }

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }

private:
    // render data 
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
       
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

		// weights
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
        glBindVertexArray(0);
    }
};
#endif
