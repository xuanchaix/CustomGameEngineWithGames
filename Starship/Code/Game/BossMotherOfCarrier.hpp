#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"


class PlayerShip;

class BossMotherOfCarrier : public Entity {
public:
	BossMotherOfCarrier( Vec2 startPos, Game* game, PlayerShip* playerShip );
	virtual ~BossMotherOfCarrier() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;

	virtual void BeAttacked( int hit ) override;

private:
	float const m_state1Time = 8.f;
	float const m_state2Time = 2.f;
	float const m_state3Time = 4.f;
	float const m_state4Time = 3.f;
	float const m_shieldCoolDown = 9.f;
	float const m_rushCoolDown = 20.f;

	PlayerShip* m_playerShip = nullptr;

	int m_state = 0;
	Vec2 m_targetWonderingDestination;
	Vec2 m_rushStartPos;
	Vec2 m_rushTargetPos;

	float m_internalTimer = 0.f;
	float m_rushTimer = 0.f;
	float m_spawnFighterTimer = 0.f;
	float m_shieldCoolDownTimer = 0.f;
	float m_shieldLastTimer = 0.f;
	float m_shieldAddHealthTimer = 0.f;
	bool m_shouldGoToState3 = false;
	bool m_hasShield = false;
	Entity* m_shieldEntity = nullptr;
};