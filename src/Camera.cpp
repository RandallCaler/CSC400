#include <memory>
#include "Camera.h"

using namespace std;
using namespace glm;

Camera::Camera(vec3 v, float p, float d, float a, vec3 pp, float pr, vec3 g, bool free)
{   
    cameraPos = vec3(0.0f, 0.0f, 4.0f);
    lookAtPt = pp;
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
    freeCam = free;
}

Camera::~Camera()
{
}

void Camera::updateCamera(float deltaTime) {
    vec3 v_dir = -vec3(sin(-angle) * cos(pitch), sin(pitch), cos(angle) * cos(pitch));
    cameraPos += (vec3(vel.z) * normalize(v_dir) + vec3(0,-vel.y,0) + vec3(vel.x) * normalize(cross(v_dir,vec3(0,1,0)))) * vec3(deltaTime);
}

void Camera::SetView(std::shared_ptr<Program> shader) {
    mat4 v_mat;
    if (freeCam) {
        vec3 v_dir = -vec3(sin(-angle) * cos(pitch), sin(pitch), cos(angle) * cos(pitch)) + cameraPos;
        v_mat = lookAt(cameraPos, v_dir, vec3(0,1,0));
    }
    else {
        horiz = dist * cos(pitch * 0.01745329);   // for third person camera - calculate horizontal and
        vert = dist * sin(pitch * 0.01745329);    // vertical offset based on maintained distance
        offX = horiz * sin(angle);				// rotation around cat
        offZ = horiz * cos(angle);

        g_eye = vec3(player_pos[0] - offX, player_pos[1] + vert, player_pos[2] - offZ);
        v_mat = lookAt(g_eye, player_pos, vec3(0, 1, 0));
    }

    glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, value_ptr(v_mat));
}
