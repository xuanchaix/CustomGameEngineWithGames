#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

// abandoned idea: deprecated to use, substituted by rocket
class LaserBeam : public Entity
{
public:
	float m_range;

public:
	LaserBeam( Vec2 startPos, Game* game, float forwardDegrees, Entity const* ship, bool isEnemyAttack, float range );

	virtual ~LaserBeam() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	Entity const* m_ship;
	float m_lifespan = LASER_LIFETIME_SECONDS;
	bool m_isEnemyAttack = false;
};