#pragma once
#include "Game/Entity.hpp"
#include "Game/Ray.hpp"
//Enemy logic
// state 1: searching: rotate counter-clockwise to searching -when see player go to 2
// state 2: shooting: turn to player and shoot -when cannot see player goto 1

//class Scorpio: stationary shooting turret
class Scorpio : public Entity {
public:
	Scorpio( Vec2 const& startPos, Map* map, EntityFaction faction );
	virtual ~Scorpio();

	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void DebugRender() const override;

	virtual void BeAttacked( float hit ) override;
public:
	float m_turretAngularVelocity;
	float m_turretOrientationDegrees;
private:
	float m_turretGoalOrientationDegrees;
	float m_shootCoolDown;
	float m_shootHalfAngleDegrees;
	Texture* m_baseTexture;
	Texture* m_turretTexture;
	RayCastResult2D m_thisFrameRayResult;
};