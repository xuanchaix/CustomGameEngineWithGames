#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

class ConeAttack : public Entity {
public:
	ConeAttack( Vec2 startPos, Game* game, float sectorForwardDegrees, Entity const* ship, bool isEnemyAttack, float radius=CONE_RADIUS, float apertureDegrees=CONE_APERTURE_DEGREES );

	virtual ~ConeAttack() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	Entity const* m_ship;
	float m_sectorApertureDegrees;
	float m_sectorForwardDegrees;
	float m_sectorRadius;
	float m_lifespan = CONE_LIFETIME_SECONDS;
	bool m_isEnemyAttack = false;
};