#ifndef TRIANGLELIST_H
#define TRIANGLELIST_H


#include <vector>
#include <glad/glad.h>
#include <algorithm>

#include "GLSL.h"

class Terrain;

class TriangleList {

    public:
        TriangleList();
        void createTriangleList(int w, int d, const Terrain *terrain);
        void renderTerrain();

    private:

        typedef struct vertex {
            glm::vec3 pos;
            void initVertex(const Terrain *terrain, int x, int z);
        } vertex;

        void createGLState();
        void populateBuffers(const Terrain *terrain);
        void initVertices(const Terrain *terrain, std::vector<vertex>& vertices);
        int width = 0;
        int depth = 0;
        GLuint vao;
        GLuint vb;
        GLuint ib;

}




#endif