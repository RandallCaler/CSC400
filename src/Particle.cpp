//
// sueda - geometry edits Z. Wood
// 3/16
//

#include <iostream>
#include "Particle.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Texture.h"


float randFloat(float l, float h)
{
	float r = rand() / (float)RAND_MAX;
	return (1.0f - r) * l + r * h;
}

Particle::Particle(vec3 start) :
	charge(1.0f),
	m(1.0f),
	d(0.0f),
	x(start),
	v(0.0f, 0.0f, 0.0f),
	lifespan(1.0f),
	tEnd(0.0f),
	scale(1.0f),
	color(1.0f, 1.0f, 1.0f, 1.0f)
{
}

Particle::~Particle()
{
}

void Particle::load(vec3 start)
{
	// Random initialization
	rebirth(0.0f, start);
}

/* all particles born at the origin */
void Particle::rebirth(float t, vec3 start)
{
	charge = randFloat(0.0f, 1.0f) < 0.5 ? -1.0f : 1.0f;
	m = 1.0f;
	d = randFloat(0.0f, 0.02f);
	x = vec3(0, 3.5, -3);//start;
	v.x = randFloat(-0.27f, 0.08f); //0.3
	v.y = randFloat(-0.1f, 0.02f); //0.9
	v.z = randFloat(-0.3f, 2.0f); //0.27
	lifespan = randFloat(3.0f, 5.0f);
	tEnd = t + lifespan;
	scale = randFloat(0.2, 1.0f);
	color.r = randFloat(0.25f, 0.5f);
	color.g = randFloat(0.1f, 0.25f);
	color.b = randFloat(0.0f, 0.1f);
	color.a = 1.0f;
}

void Particle::update(float t, float h, const vec3& g, const vec3 start)
{
	if (t > tEnd) {
		rebirth(t, start);
	}

	//very simple update
	x += h * v;
	//To do - how do you want to update the forces?
	color.a = (tEnd - t) / lifespan;
}
