#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"


class SummonLight : public Entity {
public:
	SummonLight( Vec2 startPos, Game* game, Vec2 const& targetPos, float travelTime, Entity* summonEntity, Rgba8 const& color );
	virtual ~SummonLight() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	float m_timer = 0.f;
	float m_travelTime;
	Vec2 m_startPosition;
	Vec2 m_targetPosition;
	Entity* m_summonEntity = nullptr;
};