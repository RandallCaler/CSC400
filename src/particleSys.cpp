#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include <algorithm>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include "particleSys.h"
#include "GLSL.h"

using namespace std;

particleSys::particleSys(vec3 source) {

	numP = 300;
	t = 0.0f;
	h = 0.1f;
	g = vec3(0.0f, -0.098, 0.0f);
	start = source;
	theCamera = glm::mat4(1.0);
}

void particleSys::gpuSetup() {

	cout << "start: " << start.x << " " << start.y << " " << start.z << endl;
	for (int i = 0; i < numP; i++) {
		points[i * 3 + 0] = start.x;
		points[i * 3 + 1] = start.y;
		points[i * 3 + 2] = start.z;

		auto particle = make_shared<Particle>(start);
		particles.push_back(particle);
		particle->load(start);
	}

	//generate the VAO
	glGenVertexArrays(1, &vertArrObj);
	glBindVertexArray(vertArrObj);

	//generate vertex buffer to hand off to OGL - using instancing
	glGenBuffers(1, &vertBuffObj);

	//set the current state to focus on our vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffObj);

	//actually memcopy the data - only do this once
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points[0], GL_STREAM_DRAW);

	//Same but for color
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), &pointColors[0], GL_STREAM_DRAW);

	assert(glGetError() == GL_NO_ERROR);


}

void particleSys::reSet() {
	for (int i = 0; i < numP; i++) {
		particles[i]->load(start);
	}
}

void particleSys::drawMe(std::shared_ptr<Program> prog) {

	glBindVertexArray(vertArrObj);
	int color = prog->getAttribute("color");
	GLSL::enableVertexAttribArray(color);
	//std::cout << "Any Gl errors1: " << glGetError() << std::endl;
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	//std::cout << "Any Gl errors2: " << glGetError() << std::endl;
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	assert(glGetError() == GL_NO_ERROR);
	// GLSL::disableVertexAttribArray(color);

	int h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	//std::cout << "Any Gl errors1: " << glGetError() << std::endl;
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffObj);
	//std::cout << "Any Gl errors2: " << glGetError() << std::endl;
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(0, 1);
	glVertexAttribDivisor(1, 1);

	glDrawArraysInstanced(GL_POINTS, 0, 1, numP);
	glVertexAttribDivisor(0, 1);
	glVertexAttribDivisor(1, 1);
	glDisableVertexAttribArray(0);

}

void particleSys::update() {

	vec3 pos;
	vec4 col;
	vec3 acc = vec3(0, -0.098, 0);

	//update the particles
	for (auto particle : particles) {
		particle->update(t, h, g, start);
		vec3 velo = particle->getVelocity();
		velo += acc * h;
		particle->setVelocity(velo);
		vec3 pos = particle->getPosition();
		pos += velo * h;
		particle->setPosition(pos);
	}
	t += h;

	// Sort the particles by Z
	//temp->rotate(camRot, vec3(0, 1, 0));
	//be sure that camera matrix is updated prior to this update
	vec3 s, t, sk;
	vec4 p;
	quat r;
	glm::decompose(theCamera, s, r, t, sk, p);
	sorter.C = glm::toMat4(r);
	sort(particles.begin(), particles.end(), sorter);


	//go through all the particles and update the CPU buffer
	for (int i = 0; i < numP; i++) {
		pos = particles[i]->getPosition();
		col = particles[i]->getColor();
		points[i * 3 + 0] = pos.x;
		points[i * 3 + 1] = pos.y;
		points[i * 3 + 2] = pos.z;
		/*  To do - how can you integrate unique colors per particle? */
		pointColors[i * 4 + 0] = col.r + col.a / 10;
		pointColors[i * 4 + 1] = col.g + col.g / 10;
		pointColors[i * 4 + 2] = col.b + col.b / 10;
		pointColors[i * 4 + 3] = col.a;
	}

	//update the GPU data
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffObj);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numP * 3, points);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numP * 4, pointColors);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}
