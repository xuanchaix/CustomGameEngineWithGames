#pragma once
#include "Game/Entity.hpp"
class SpriteAnimDefinition;

class Explosion : public Entity {

public:
	Explosion( Vec2 const& startPos, Map* map, float sizeFactor, float lifeSpanSeconds );
	virtual ~Explosion();

	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	SpriteAnimDefinition* m_anim = nullptr;
	float m_lifeSpanSeconds;
	float m_ageSeconds = 0.f;
};