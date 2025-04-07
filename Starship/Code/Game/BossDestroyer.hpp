#pragma once
#include "Game/Entity.hpp"

class BossDestroyerTurret;

constexpr int DESTROYER_ENEMY_AMOUNT = 20;
constexpr int DESTROYER_TURRET_AMOUNT = 8;

//------------make a boss for fun------------
// boss is a huge star destroyer ship which has 4 stages
// player must hit some particular part of the boss to deal damage
// 
// stage1: enemy ships -- boss spawn more enemy ships to fight with player pass:kill all enemies
// stage2: gun pass:destroy all turrets
// stage3: laser gun -- boss shoots 2 lasers through the screen in 3s, cool down time is 2s pass:destroy the shield, cut down boss health to half
// stage4: laser gun + repair guns pass:kill the boss
// 
// always put boss in the last of the entity array so the boss can watch which part become garbage.
// 
// machine gun -- boss shoots a lot of bullets in 3s, cool down time is 2s
// state4: protect mode -- make shield and protect self from player's attack
class BossDestroyer : public Entity {
public:
	float m_nextLaserY1 = 0.f;
	float m_nextLaserY2 = 0.f;
public:
	BossDestroyer( Vec2 startPos, Game* game );
	virtual ~BossDestroyer() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void RenderUI() const override;
	virtual void BeAttacked( int hit ) override;

private:
	int m_stage = 1;
	int m_stage1Count = 0;
	Entity** m_spawnEnemies;
	BossDestroyerTurret** m_turret;
	float m_maxShieldHealth = 30.f;
	float m_shieldHealth = m_maxShieldHealth;
	float m_shieldHealthGrowthPerSecond = 2.f;

	float m_laserCoolDown = 6.f;
	float m_laserTime = m_laserCoolDown;
	float m_spawnBeetleCoolDown = 8.f;
	float m_spawnBeetleTimer = m_spawnBeetleCoolDown;

	int m_repairingTurret = 0;
	float m_repairChange = 6.f;
	float m_repairTime = m_repairChange;
	float m_healthAccumulation = 0.f;

	float m_stage4SpawnCooldown = 25.f;
	float m_stage4SpawnSpawnTime = m_stage4SpawnCooldown;
};