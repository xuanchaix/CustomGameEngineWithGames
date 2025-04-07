#pragma once
#include "Game/Entity.hpp"

class Building : public Entity {
public:
	Building( Vec2 const& startPos, Map* map );
	virtual ~Building();

	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void RenderUI() const override;

	virtual void BeAttacked( float hit ) override;

private:

};