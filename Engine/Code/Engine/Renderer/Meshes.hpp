#pragma once
#include <vector>
#include <string>
#include "Engine/Core/Vertex_PCU.hpp"
class IndexBuffer;
class VertexBuffer;
class Renderer;

class CPUMesh {
public:
	CPUMesh();
	CPUMesh( std::string const& objFileName, Mat44 const& transform );
	virtual ~CPUMesh();


	void Load( std::string const& objFileName, Mat44 const& transform );

	std::vector<unsigned int> m_indexes;
	std::vector<Vertex_PCUTBN> m_vertexes;

};

class GPUMesh {
public:
	GPUMesh();
	GPUMesh( CPUMesh* cpuMesh, Renderer* renderer );
	~GPUMesh();

	void Create( CPUMesh* cpuMesh, Renderer* renderer );
	void Render( Renderer* renderer ) const;

public:
	IndexBuffer* m_indexBuffer;
	VertexBuffer* m_vertexBuffer;
};