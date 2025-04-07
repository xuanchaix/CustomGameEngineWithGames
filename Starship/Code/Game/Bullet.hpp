#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

class Bullet : public Entity {
public:
	Bullet( Vec2 startPos, Game* game, Vec2 velocity, float orientationDegree, bool isEnemyBullet );

	virtual ~Bullet() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	float m_lifespan = BULLET_LIFETIME_SECONDS;
};