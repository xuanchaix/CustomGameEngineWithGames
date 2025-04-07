#pragma once
#include "Game/Entity.hpp"

class Model : public Entity {
public:
	Model( Game* game );
	Model( Game* game, std::string const& fileName );
	virtual ~Model();

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void DebugRender() const override;

	bool Load( std::string const& modelName );
	void CreateDebugTangentBasisVectors();

	std::string m_name;
	Material* m_material = nullptr;
	CPUMesh* m_cpuMesh = nullptr;
	GPUMesh* m_gpuMesh = nullptr;
	VertexBuffer* m_debugVertexBuffer = nullptr;
};