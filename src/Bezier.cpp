#include "Bezier.h"

// Interpolator Functions
// These return intermediate points 
// along a parametric curve between
// points a,b at t

glm::vec3 Bezier::lErp(glm::vec3 a, glm::vec3 b, float t)
{
    // simple linear interpolation
    return (1.0f - t) * a + t * b;
}

glm::vec3 Bezier::quadErp(glm::vec3 a, glm::vec3 b, float t)
{
    // slower at the beginning, faster at the end
    return (1.0f - t * t) * a + t * t * b;
}

/////////////////////////////////////////////////

// Bezier Functions
// These take an interpolator function pointer (or NULL for default)
// a glm::vec3 (a) for the starting point, a vecc3 (b) for the end,
// glm::vec3's for the control points (depending on the order of the curve),
// and a float t (between 0,1) as the parameter of the curve.


glm::vec3 Bezier::quadBez(glm::vec3 (*interpolation)(glm::vec3, glm::vec3, float),
                    glm::vec3 a,
                    glm::vec3 b, 
                    glm::vec3 control, 
                    float t)
{
    // determine which interpolation function to use.
    // NULL sets the default to linear interpolation
    interpolation = interpolation ? interpolation : lErp;
    
    // set up endpoints of line to interpolate
    // along to form the curve
    glm::vec3 q0 = (*interpolation)(a, control, t);
    glm::vec3 q1 = (*interpolation)(control, b, t);

    // return the interpolated point along the control line.
    // these points form the curve.
    return (*interpolation)(q0, q1, t);
}

glm::vec3 Bezier::cubeBez(glm::vec3 (*interpolation)(glm::vec3, glm::vec3, float), 
                    glm::vec3 a, 
                    glm::vec3 b, 
                    glm::vec3 control1, 
                    glm::vec3 control2, 
                    float t)
{
    // determine which interpolation function to use.
    // NULL sets the default to linear interpolation
    interpolation = interpolation ? interpolation : lErp;
    
    // set up endpoints of lines to interpolate
    // low order control lines between
    glm::vec3 q0 = (*interpolation)(a, control1, t);
    glm::vec3 q1 = (*interpolation)(control1, control2, t);
    glm::vec3 q2 = (*interpolation)(control2, b, t);

    // endpoints of lower order control line
    glm::vec3 r0 = (*interpolation)(q0, q1, t);
    glm::vec3 r1 = (*interpolation)(q1, q2, t);

    // return the interpolated point along the control line.
    // these points form the curve.
    return (*interpolation)(r0, r1, t);
}