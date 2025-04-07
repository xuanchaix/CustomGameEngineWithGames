#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"

constexpr int NUM_OF_DEBRIS_VERTS = 48;

class Debris : public Entity {
public:
	Debris( Vec2 startPos, Game* game, Vec2 baseVelocity, float baseRadius, Rgba8 color );
	virtual ~Debris() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	Vertex_PCU debrisVerts[NUM_OF_DEBRIS_VERTS];
	float m_lifeSpan = DEBRIS_LIFETIME_SECONDS;

};