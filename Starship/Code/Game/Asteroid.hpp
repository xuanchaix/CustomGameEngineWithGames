#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

constexpr int NUM_OF_ASTEROID_VERTS = 48;

class Asteroid : public Entity {
public:
	Asteroid( Vec2 startPos, Game* game );
	virtual ~Asteroid() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;
private:
	Vertex_PCU asteroidVerts[NUM_OF_ASTEROID_VERTS];
};