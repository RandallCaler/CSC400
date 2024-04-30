
#ifndef TERRAIN_H
#define TERRAIN_H

#include <iostream>
#include <vector>
#

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


class Terrain {
    public:
        Terrain() {}
        void LoadFromFile (const char *f);
        void PrintFloat();

    protected:
        int terrainSize = 0;
        float *heightMap;
};

#endif