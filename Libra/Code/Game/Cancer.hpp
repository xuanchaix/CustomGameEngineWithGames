#pragma once
#include "Game/Entity.hpp"

class Cancer : public Entity {
public:
	Cancer( Vec2 const& startPos, Map* map, EntityFaction faction );
	virtual ~Cancer();

	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void DebugRender() const override;

	virtual void BeAttacked( float hit ) override;
private:
	Texture* m_tankTexture;
	float m_wonderingCoolDown;
	float m_wonderingTime;
	float m_shootCoolDown;
};