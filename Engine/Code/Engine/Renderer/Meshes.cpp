#include "Engine/Renderer/Meshes.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/ObjLoader.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

CPUMesh::CPUMesh()
{

}

CPUMesh::CPUMesh( std::string const& objFileName, Mat44 const& transform )
{
	Load( objFileName, transform );
}

CPUMesh::~CPUMesh()
{

}

void CPUMesh::Load( std::string const& objFileName, Mat44 const& transform )
{
	bool hasNormals;
	bool hasTextureCoords;
	ObjLoader::Load( objFileName, m_vertexes, m_indexes, hasNormals, hasTextureCoords, transform );

	double startTime = GetCurrentTimeSeconds();
	CalculateTangentSpaceBasisVectors( m_vertexes, m_indexes, !hasNormals, hasTextureCoords );
	double endTime = GetCurrentTimeSeconds();

	DebuggerPrintf( "Calculated tangent basis    time: %fs\n", endTime - startTime );

	UNUSED( hasTextureCoords );
}

GPUMesh::GPUMesh()
{

}

GPUMesh::GPUMesh( CPUMesh* cpuMesh, Renderer* renderer )
{
	Create( cpuMesh, renderer );
}

GPUMesh::~GPUMesh()
{
	delete m_vertexBuffer;
	delete m_indexBuffer;
}

void GPUMesh::Create( CPUMesh* cpuMesh, Renderer* renderer )
{
	double startTime = GetCurrentTimeSeconds();
	if (m_vertexBuffer) {
		delete m_vertexBuffer;
	}
	if (m_indexBuffer) {
		delete m_indexBuffer;
	}
	size_t sizeInByte = cpuMesh->m_vertexes.size() * sizeof( Vertex_PCUTBN );
	m_vertexBuffer = renderer->CreateVertexBuffer( sizeInByte, sizeof( Vertex_PCUTBN ) );
	renderer->CopyCPUToGPU( cpuMesh->m_vertexes.data(), sizeInByte, m_vertexBuffer );

	sizeInByte = cpuMesh->m_indexes.size() * sizeof( int );
	m_indexBuffer = renderer->CreateIndexBuffer( sizeInByte );
	renderer->CopyCPUToGPU( cpuMesh->m_indexes.data(), sizeInByte, m_indexBuffer );

	double endTime = GetCurrentTimeSeconds();
	DebuggerPrintf( "Created GPU mesh            time: %fs\n", endTime - startTime );
}

void GPUMesh::Render( Renderer* renderer ) const
{
	renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	renderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	renderer->SetDepthMode( DepthMode::ENABLED );
	renderer->SetBlendMode( BlendMode::OPAQUE );
	renderer->BindShader( nullptr );
	renderer->BindTexture( nullptr );
	renderer->SetModelConstants();
	renderer->DrawVertexIndexed( m_vertexBuffer, m_indexBuffer, m_indexBuffer->GetIndexCount() );
}
