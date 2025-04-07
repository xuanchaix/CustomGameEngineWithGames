#pragma once
#include "Game/Entity.hpp"
class SpriteAnimDefinition;

class Bullet : public Entity {
public:
	Bullet( Vec2 const& startPos, Map* map, float orientationDegrees, EntityFaction faction, EntityType type = EntityType::_BULLET,
		float damage = 1.f, int bounceTimes = 0, float inheritedSpeed = 0.f, Entity* target = nullptr );
	virtual ~Bullet();

	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
	//virtual void DebugRender() const override;
	virtual void RenderUI() const override;
	void BounceOff( Vec2 const& normal );

	virtual void BeAttacked( float hit ) override;
public:
	float m_damage = 1;
	Vec2 m_lastFramePosition = Vec2( 0, 0 );
private:
	SpriteAnimDefinition* m_anim = nullptr;
	Texture* m_texture = nullptr;
	int m_bounceTimesLeft = 0;
	float m_bounceHalfVarianceDegrees;
	float m_lifeSpanSeconds = 0.f;
};