#pragma once
#include "Game/Entity.hpp"

class Prop : public Entity {
public:
	Prop( Game* game );
	virtual ~Prop();

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;
	//virtual void RenderUI() const override;

	//virtual void BeAttacked( int hit ) override;

public:
	std::vector<Vertex_PCU> m_vertexes;
	std::vector<unsigned int> m_indexes;
	Texture* m_texture = nullptr;
};