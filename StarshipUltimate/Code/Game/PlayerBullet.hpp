#pragma once
#include "Game/Projectile.hpp"

class PlayerBullet : public Projectile {
public:
	PlayerBullet( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~PlayerBullet();

	virtual void BeginPlay();
	virtual void Update( float deltaTime );
	virtual void Render() const;
	virtual void Die( bool dieByCollision = true );
	virtual void SpawnCollisionEffect();
};