#include <stdio.h>
#include <vector>

#include "TriangleList.h"
#include "Terrain.h"

void TriangleList::createTriangleList(int w, int d, Terrain *terrain) {
    width = w;
    depth = d;

    createGLState();
    populateBuffers(terrain);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void TriangleList::createGLState() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vb);
    glBindBuffer(GL_ARRAY_BUFFER, vb); 
    int POS = 0;
    glEnableVertexAttribArray(POS);
    size_t num = 0;
    glVertexAttribPointer(POS, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (const void *)(num * sizeof(float)));
    num += 3;
} 

void TriangleList::populateBuffers(const Terrain *terrain) {
    std::vector<vertex> vertices;
    vertices.resize(width * depth);
    initVertices(terrain, vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
}

void TriangleList::initVertex(const Terrain *terrain, int x, int z) {
    pos = glm::vec3(x, 0.0f, z);
}

void TriangleList::InitVertices(const Terrain *terrain, std::vector<vertex>& vertices) {
    int idx = 0;

    for (int z = 0; z < depth; z++) {
        for (int x = 0; x < width; x++) {
            vertices[idx].initVertex(terrain, x, z);
            idx++;
        }
    }
}

void TriangleList::renderTerrain() {
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, depth + width);
    glBindVertexArray(0);
}