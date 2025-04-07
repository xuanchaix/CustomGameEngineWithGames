#pragma once
#include "Game/Entity.hpp"

class PlayerShip;

class PlayerAsteroid : public Entity {
public:
	PlayerAsteroid( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~PlayerAsteroid();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false );
public:
	bool m_isActivated = false;
	PlayerShip* m_owner = nullptr;
	std::vector<Vertex_PCU> m_verts;
	float m_oritationToOwner = 0.f;
};

class BasicFollower : public Entity {
public:
	BasicFollower( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~BasicFollower();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false );

public:
	bool m_isActivated = false;
	PlayerShip* m_owner = nullptr;

	float m_acclerateToPlayerDist = 0.f;
	float m_idleDist = 0.f;
	//float m_keepOutToPlayerDist = 0.f;
};

class DiagonalRetinue : public BasicFollower {
public:
	DiagonalRetinue( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~DiagonalRetinue();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false );

protected:
	Timer* m_attackTimer = nullptr;
};

class LaserWingPlane : public BasicFollower {
public:
	LaserWingPlane( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~LaserWingPlane();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false );

protected:
	Timer* m_attackTimer = nullptr;
};

class WingPlane : public BasicFollower {
public:
	WingPlane( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~WingPlane();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();
	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false );
	virtual float GetMainWeaponDamage() const;

protected:
	Timer* m_attackTimer = nullptr; // timer for whole attack
	Timer* m_shootTimer = nullptr; // timer for whole shoot
	Timer* m_bulletTimer = nullptr; // timer for one bullet
};