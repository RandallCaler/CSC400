#ifndef BEZIER_H
#define BEZIER_H

#include "glm/glm.hpp"
/*From D. McGirr (into the CG-verse previz 2019)*/

class Bezier
{
    public:

    static glm::vec3 quadErp(glm::vec3 a, glm::vec3 b, float t);
    static glm::vec3 lErp(glm::vec3 a, glm::vec3 b, float t);

    // used when you want a quadratic curve (one control point)
    static glm::vec3 quadBez(glm::vec3 (*interpolation)(glm::vec3, glm::vec3, float),
                        glm::vec3 a, 
                        glm::vec3 b, 
                        glm::vec3 control, 
                        float t);


    // two control points, for S-shaped curves or whatever you want
    static glm::vec3 cubeBez(glm::vec3 (*interpolation)(glm::vec3, glm::vec3, float), 
                        glm::vec3 a, 
                        glm::vec3 b, 
                        glm::vec3 control1,
                        glm::vec3 control2, 
                        float t);

};

#endif