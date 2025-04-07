#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

class PlayerShip;
// boss first explorer
// similar to player ship: bounce off the edge and shoot bullets
// has three stages: attack and defense and spawn
// attack: shoot randomly: bullets or cone attack
// spawn: spawn wasps and asteroids
// defense: kill allies or asteroids to heal self
// defeat rewards: cone attack

class BossFirstExplorer : public Entity {
public:
	BossFirstExplorer( Vec2 startPos, Game* game, PlayerShip* playerShip );
	virtual ~BossFirstExplorer() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;
	//virtual void RenderUI() const override;
private:
	float m_shootCooldown = 11.f;
	float m_shootTimeCount = m_shootCooldown;
	float m_shootDuration = 0.f;
	float m_timeAfterLastshoot = 0.f;
	float m_coneCooldown = 7.f;
	float m_coneTimeCount = m_coneCooldown;
	float m_spawnCooldown = 6.f;
	float m_spawnTimeCount = m_spawnCooldown;
	float m_healCooldown = 1.f;
	float m_healTimeCount = m_healCooldown;
	PlayerShip* m_playerShip = nullptr;
	void DealCollidingEdge();
};