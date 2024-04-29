#include <memory>
#include "Camera.h"

using namespace std;
using namespace glm;

Camera::Camera(vec3 v, float p, float d, float a, vec3 pp, float pr, vec3 g)
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
}

Camera::~Camera()
{
}

void Camera::SetView(std::shared_ptr<Program> shader) {
    vec3 v_eye = player_pos + vec3(0,2,-3);
    vec3 v_dir = -vec3(sin(-angle) * cos(pitch), sin(pitch), cos(angle) * cos(pitch)) + v_eye;
    // mat4 scale = glm::scale(glm::mat4(1.0f), view->scale);
	mat4 v_mat = lookAt(v_eye, v_dir, vec3(0,1,0));

	glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, value_ptr(v_mat));
}
