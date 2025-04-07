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

	void CreateCube( char const* materialFileName );
	void CreateSphere( char const* materialFileName );
	void CreateWorldGrid();
	virtual void DebugRender() const override;

protected:
	void CreateBuffers();
	void CreateDebugTangentBasisVectors();

public:
	std::vector<Vertex_PCU> m_unlitVertexes;
	Texture* m_unlitTexture = nullptr;

	std::vector<Vertex_PCUTBN> m_vertexes;
	std::vector<unsigned int> m_indexes;
	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
	Material* m_material = nullptr;

	std::vector<Vertex_PCU> m_debugVertexes;
	VertexBuffer* m_debugVertexBuffer = nullptr;
};