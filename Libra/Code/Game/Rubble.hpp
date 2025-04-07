#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include <vector>

class Rubble : public Entity {
public:
	Rubble( Vec2 const& startPos, Map* map );
	virtual ~Rubble();

	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	Rgba8 m_color;
	std::vector<Vertex_PCU> m_verts;
};