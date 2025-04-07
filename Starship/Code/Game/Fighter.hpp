#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
/*
Fighter:
*/

class PlayerShip;

class Fighter : public Entity {
public:
	Fighter( Vec2 startPos, Game* game, PlayerShip* playership );
	virtual ~Fighter() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	PlayerShip* m_targetPlayerShip;
	float m_shootCooldown = 6.f;
	float m_shootTimer = 0.f;

	int m_state = 0;
};