#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
/*
Carrier: a ship spawning fighters and wander and dodge player attack

state 1: go to destination
state 2: dodge

spawn cool down: spawn 3 fighters
*/


class PlayerShip;

class Carrier : public Entity {
public:
	Carrier( Vec2 startPos, Game* game, PlayerShip* playership );
	virtual ~Carrier() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	PlayerShip* m_targetPlayerShip;
	float m_fighterSpawnCooldown = 4.f;
	float m_fighterSpawnTimer = 4.f;

	Vec2 m_targetWonderingDestination;

	int m_state = 0;
};