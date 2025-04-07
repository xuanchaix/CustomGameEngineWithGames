#pragma once
#include "Game/Entity.hpp"
#include "Game/Projectile.hpp"

/*
	state 0: preparation
	state 1: shoot bullets
	state 2: dash to player
	state 3: teleport
*/
class LittleDemon : public Entity {
public:
	LittleDemon( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~LittleDemon();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die();

	void BounceOffEdges( AABB2 const& edges );

protected:
	Entity* m_target = nullptr;
	Timer* m_stateTimer = nullptr;
	int m_numOfBullets = 0;
	Timer* m_state1ShootTimer = nullptr;
	Timer* m_state3CoolDownTimer = nullptr;
	Timer* m_state3TeleportTimer = nullptr;
	int m_state = 0;
	Vec2 m_dashDir;
};