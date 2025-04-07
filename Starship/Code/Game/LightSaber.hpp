#pragma once

#include "Game/Entity.hpp"

class LightSaber : public Entity {
public:
	float m_length;

public:
	LightSaber( Vec2 startPos, Game* game, float forwardDegrees, Entity const* ship, float length );

	virtual ~LightSaber() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	Entity const* m_ship;
	float m_forwardDegrees = 0.f;
	float m_range = 0.1f;
};