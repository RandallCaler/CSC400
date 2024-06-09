//
// sueda
// November, 2014/ wood 16
//

#pragma once

#ifndef LAB471_PARTICLE_H_INCLUDED
#define LAB471_PARTICLE_H_INCLUDED

#include <vector>
#include <memory>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class MatrixStack;
class Program;
class Texture;

class Particle
{
public:
	Particle(vec3 pos);
	virtual ~Particle();
	void load(vec3 start);
	void rebirth(float t, vec3 start);
	void update(float t, float h, const glm::vec3& g, const vec3 start);
	const vec3& getPosition() const { return x; };
	void setPosition(const vec3& newPos) { x = newPos; };
	const vec3& getVelocity() const { return v; };
	void setVelocity(const vec3& newVelo) { v = newVelo; };
	const vec4& getColor() const { return color; };

private:
	float charge; // +1 or -1
	float m; // mass
	float d; // viscous damping
	vec3 x; // position
	vec3 v; // velocity
	float lifespan; // how long this particle lives
	float tEnd;     // time this particle dies
	float scale;
	vec4 color;
};

#endif // LAB471_PARTICLE_H_INCLUDED
