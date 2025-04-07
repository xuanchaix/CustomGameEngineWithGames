#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

class PlayerShip;

class BossDestroyerTurret : public Entity {
public:
	bool m_doNotShoot = true;
	bool m_isDeatroyed = false;
	bool m_isRepairing = false;
public:
	BossDestroyerTurret( Vec2 startPos, Game* game, PlayerShip* playership );
	virtual ~BossDestroyerTurret() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void BeAttacked( int hit ) override;

private:
	PlayerShip* m_targetPlayerShip;
	float m_shootCooldown = 7.f;
	float m_shootTimeCount = m_shootCooldown;
	bool m_hasShield = true;
	float m_shieldCoolDown = 20.f;
	float m_shieldTimeCount = m_shieldCoolDown;
	Entity* m_shieldEntity;
};