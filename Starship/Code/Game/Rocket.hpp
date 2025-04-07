#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

class Rocket : public Entity {
public:
	Rocket( Vec2 startPos, Game* game, Vec2 velocity, float orientationDegree, float damage, bool isEnemyBullet );

	virtual ~Rocket() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;

	void RefreshTarget();

private:
	float m_lifespan = ROCKET_LIFETIME_SECONDS;
	float m_range;
	Vec2 m_startPos;
	Vec2 m_targetPos;
	Vec2 m_rndMiddlePos;
	float m_timer = 0.f;
	float m_timeToTarget = 0.f;
	Entity* m_target;
};