#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"

constexpr int NUM_OF_DEBRIS_VERTS = 48;

class Shield : public Entity {
public:
	Shield( Vec2 startPos, Game* game, Entity* owner, float size, Rgba8 const& color=Rgba8(0, 0, 255) );
	virtual ~Shield() override;

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	Entity* m_owner = nullptr;
	float m_size = 0.f;
};