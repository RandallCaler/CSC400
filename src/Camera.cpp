#include <memory>
#include "Camera.h"

using namespace std;
using namespace glm;

Camera::Camera(vec3 v, float p, float d, float a, vec3 pp, float pr, vec3 g)
{   
    cameraPos = vec3(0.0f, 0.0f, 4.0f);
    lookAtPt = vec3(0.0f, 0.0f, 0.0f);
    upV = vec3(0, 1, 0);
    player_pos = pp;
    player_rot = pr;
    view = v;
    pitch = p;
    dist = d;
    angle = a;
    horiz;
    vert;
    offX;
    offZ;
    g_eye = g;
}

Camera::~Camera()
{
}

void Camera::SetView(std::shared_ptr<Program> shader) {
    horiz = dist * cos(pitch * 0.01745329);   // for third person camera - calculate horizontal and
    vert = dist * sin(pitch * 0.01745329);    // vertical offset based on maintained distance
    offX = horiz * sin(angle);				// rotation around cat
    offZ = horiz * cos(angle);

    g_eye = vec3(player_pos[0] - offX, player_pos[1] + vert, player_pos[2] - offZ);
    glm::mat4 Cam = glm::lookAt(g_eye, player_pos, vec3(0, 1, 0));
    glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, value_ptr(Cam));
}
