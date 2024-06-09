#pragma once
#ifndef __particleS__
#define __particleS__

#include <glm/glm.hpp>
#include <vector>
#include "Particle.h"
#include "Program.h"

using namespace glm;
using namespace std;

class ParticleSorter {
public:
	bool operator()(const shared_ptr<Particle> p0, const shared_ptr<Particle> p1) const
	{
		// Particle positions in world space
		const vec3& x0 = p0->getPosition();
		const vec3& x1 = p1->getPosition();
		// Particle positions in camera space
		vec4 x0w = C * vec4(x0.x, x0.y, x0.z, 1.0f);
		vec4 x1w = C * vec4(x1.x, x1.y, x1.z, 1.0f);
		return x0w.z < x1w.z;
	}

	mat4 C; // current camera matrix
};

class particleSys {
private:
	vector<shared_ptr<Particle>> particles;
	float t, h; //?
	vec3 g; //gravity
	int numP;
	vec3 start;
	ParticleSorter sorter;
	//this is not great that this is hard coded - you can make it better
	GLfloat points[900];
	GLfloat pointColors[1200];
	mat4 theCamera;
	unsigned vertArrObj;
	unsigned vertBuffObj;
	unsigned colorbuffer;

public:
	particleSys(vec3 source);
	void drawMe(std::shared_ptr<Program> prog);
	void gpuSetup();
	void update();
	void reSet();
	void setCamera(mat4 inC) { theCamera = inC; }
};


#endif
