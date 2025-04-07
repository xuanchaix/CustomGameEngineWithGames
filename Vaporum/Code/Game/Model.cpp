#include "Game/Model.hpp"

#include "Game/Model.hpp"
#include <filesystem>
#include <stdlib.h>
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

Model::Model( Game* game )
	:Entity( game )
{

}

Model::Model( Game* game, std::string const& fileName )
	:Entity( game )
{
	Load( fileName );
}

Model::~Model()
{
	delete m_gpuMesh;
	delete m_cpuMesh;
	delete m_material;
	delete m_debugVertexBuffer;
}

void Model::Update()
{
	//float deltaSeconds = g_theGame->m_gameClock->GetDeltaSeconds();
}

void Model::Render() const
{
	if (!m_gpuMesh) {
		return;
	}
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	if (m_material) {
		g_theRenderer->BindShader( m_material->m_shader );
		g_theRenderer->BindTexture( m_material->m_diffuseTxeture, 0 );
		g_theRenderer->BindTexture( m_material->m_normalTexture, 1 );
		g_theRenderer->BindTexture( m_material->m_specGlossEmitTexture, 2 );
		g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	}
	else {
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	}
	g_theRenderer->DrawVertexIndexed( m_gpuMesh->m_vertexBuffer, m_gpuMesh->m_indexBuffer, m_gpuMesh->m_indexBuffer->GetIndexCount() );
}

void Model::Die()
{

}

bool Model::Load( std::string const& modelName )
{
	delete m_gpuMesh;
	delete m_cpuMesh;
	delete m_material;
	delete m_debugVertexBuffer;
	m_debugVertexBuffer = nullptr;
	m_material = nullptr;
	m_gpuMesh = nullptr;
	m_cpuMesh = nullptr;
	char drive[300];
	char dir[300];
	char fname[300];
	char ext[300];
	_splitpath_s( modelName.c_str(), drive, 300, dir, 300, fname, 300, ext, 300 );
	if (!strcmp( ext, ".xml" )) {
		std::string fileName = modelName;
		XmlDocument xmlDocument;
		XmlError errorCode = xmlDocument.LoadFile( fileName.c_str() );
		GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, Stringf( "Error! Load Xml Document %s error", fileName.c_str() ) );
		XmlElement* root = xmlDocument.FirstChildElement();
		GUARANTEE_OR_DIE( !strcmp( root->Name(), "Model" ), "Error! Root name of model XML should be Model!" );
		m_name = ParseXmlAttribute( *root, "name", "Default" );
		std::string objFilePath = ParseXmlAttribute( *root, "path", "None" );
		std::string materialFilePath = ParseXmlAttribute( *root, "material", "None" );
		std::string shaderFilePath = ParseXmlAttribute( *root, "shader", "None" );

		XmlElement* transformElem = root->FirstChildElement( "Transform" );
		Vec3 xVec = ParseXmlAttribute( *transformElem, "x", Vec3( 1.f, 0.f, 0.f ) );
		Vec3 yVec = ParseXmlAttribute( *transformElem, "y", Vec3( 0.f, 1.f, 0.f ) );
		Vec3 zVec = ParseXmlAttribute( *transformElem, "z", Vec3( 0.f, 0.f, 1.f ) );
		Vec3 tVec = ParseXmlAttribute( *transformElem, "t", Vec3( 0.f, 0.f, 0.f ) );
		float scale = ParseXmlAttribute( *transformElem, "scale", 1.f );
		Mat44 transform = Mat44( xVec, yVec, zVec, tVec );
		transform.AppendScaleUniform3D( scale );

		m_cpuMesh = new CPUMesh( objFilePath, transform );
		if (materialFilePath != "None") {
			m_material = new Material( materialFilePath, g_theRenderer );
		}
		else {
			m_material = new Material();
			if (shaderFilePath != "None") {
				m_material->m_shader = g_theRenderer->CreateShader( shaderFilePath.c_str(), VertexType::PCUTBN );
			}
		}
		m_gpuMesh = new GPUMesh( m_cpuMesh, g_theRenderer );
	}
	else if (!strcmp( ext, ".obj" )) {
		m_cpuMesh = new CPUMesh( modelName, Mat44() );
		m_gpuMesh = new GPUMesh( m_cpuMesh, g_theRenderer );
	}
	//m_material = new Material()

#ifdef DEBUG_MODE
	double startTime = GetCurrentTimeSeconds();
	CreateDebugTangentBasisVectors();
	double endTime = GetCurrentTimeSeconds();
	DebuggerPrintf( "Created debug normals       time: %fs\n", endTime - startTime );
#endif

	return true;
}


void Model::CreateDebugTangentBasisVectors()
{
	if (!m_cpuMesh) {
		return;
	}
	delete m_debugVertexBuffer;
	std::vector<Vertex_PCU> debugVerts;
	for (auto& vert : m_cpuMesh->m_vertexes) {
		debugVerts.emplace_back( vert.m_position, Rgba8( 0, 0, 255 ) );
		debugVerts.emplace_back( vert.m_position + vert.m_normal * 0.1f, Rgba8( 0, 0, 255 ) );
		debugVerts.emplace_back( vert.m_position, Rgba8( 255, 0, 0 ) );
		debugVerts.emplace_back( vert.m_position + vert.m_tangent * 0.1f, Rgba8( 255, 0, 0 ) );
		debugVerts.emplace_back( vert.m_position, Rgba8( 0, 255, 0 ) );
		debugVerts.emplace_back( vert.m_position + vert.m_bitangent * 0.1f, Rgba8( 0, 255, 0 ) );
	}
	m_debugVertexBuffer = g_theRenderer->CreateVertexBuffer( debugVerts.size() * sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( debugVerts.data(), debugVerts.size() * sizeof( Vertex_PCU ), m_debugVertexBuffer );
	m_debugVertexBuffer->SetAsLinePrimitive( true );
}

void Model::DebugRender() const
{
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants( GetModelMatrix() );
	g_theRenderer->DrawVertexBuffer( m_debugVertexBuffer, m_debugVertexBuffer->GetVertexCount() );
}
