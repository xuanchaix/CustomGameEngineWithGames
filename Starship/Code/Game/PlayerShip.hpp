#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
struct Vertex_PCU;

enum class WeaponType { Bullet, ThreeBullet, LightSaber, Laser };

class PlayerShip : public Entity {
public:
	float m_enhanceTimeLeft = 0.f;
	float m_invincibleTimeLeft = 0.f;
	float m_ammoAmount = 0.f;
	float m_maxAmmo = 200.f;
	bool m_allowLighrSaber = false;
	bool m_superBulletWeapon = false;
	bool m_isLightSaberStart = false;
	bool m_isControllerPlugIn = false;
	float m_superWeaponCoolDownTime = 12.f;
	float m_superWeaponTimer = m_superWeaponCoolDownTime;
public:
	PlayerShip( Vec2 startPos, Game* game);
	virtual  ~PlayerShip() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void BeAttacked( int hit ) override;

	Vec2 GetNosePosition() const;
	static void GetShipVerts( Vertex_PCU* shipVertsOut, int numOfVerts = NUM_OF_PLAYER_SHIP_VERTS );
private:
	void DealCollidingEdge();

private:
	float m_ammoGrowSpeed = 15.f;
	float m_laserCoolDown = 0.f;
	float m_ammoCostModifier = 1.f;
	float m_maxSpeed = 25.f;

	bool m_isShieldOn = false;
	bool m_hasShield = false;
	float m_shieldCoolDownTime = 20.f;
	float m_shieldTimer = m_shieldCoolDownTime;
	float m_bulletTimer = 0.f;

	float m_coneAttackRange = CONE_RADIUS;

	float m_lightSaberAmmoCost = 40.f;
	float m_coneAttackAmmoCost = 12.f;
	float m_rocketAmmoCost = 20.f;

	float m_rocketRange = 3.f;

	float m_lightSaberRotateCooldown = 3.f;
	float m_lightSaberRotateTime = 0.f;

	Entity* m_shieldEntity = nullptr;
};