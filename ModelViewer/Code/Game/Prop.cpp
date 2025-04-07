#include "Game/Prop.hpp"
#include "Game/Game.hpp"

Prop::Prop( Game* game )
	:Entity( game )
{
	//AddVertsForAABB3D( m_vertexes, AABB3( Vec3( -0.5f, -0.5f, -0.5f ), Vec3( 0.5f, 0.5f, 0.5f ) ) );
	m_vertexes.reserve( 1000 );
}

Prop::~Prop()
{
	delete m_vertexBuffer;
	delete m_indexBuffer;
	delete m_material;
	delete m_debugVertexBuffer;
}

void Prop::Update()
{
	float deltaSeconds = g_theGame->m_gameClock->GetDeltaSeconds();
	if (m_game->m_debugRotation) {
		m_orientation.m_yawDegrees += 45.f * deltaSeconds;
	}
	/*m_orientation.m_yawDegrees += m_angularVelocity.m_yawDegrees * deltaSeconds;
	m_orientation.m_pitchDegrees += m_angularVelocity.m_pitchDegrees * deltaSeconds;
	m_orientation.m_rollDegrees += m_angularVelocity.m_rollDegrees * deltaSeconds;*/
}

void Prop::Render() const
{
	if (m_material) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( m_material->m_shader );
		g_theRenderer->BindTexture( m_material->m_diffuseTxeture );
		g_theRenderer->BindTexture( m_material->m_normalTexture, 1 );
		g_theRenderer->BindTexture( m_material->m_specGlossEmitTexture, 2 );
		g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
		g_theRenderer->DrawVertexIndexed( m_vertexBuffer, m_indexBuffer, m_indexBuffer->GetIndexCount() );
	}
	else {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( m_unlitTexture );
		g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
		g_theRenderer->DrawVertexArray( m_unlitVertexes );
	}
}

void Prop::Die()
{
	m_isDead = true;
}

void Prop::CreateCube( char const* materialFileName )
{
	delete m_material;
	m_material = new Material( materialFileName, g_theRenderer );
	AddVertsForAABB3D( m_vertexes, m_indexes, AABB3( Vec3( -1.f, -1.f, -1.f ), Vec3( 1.f, 1.f, 1.f ) ), Rgba8::WHITE );

	CalculateTangentSpaceBasisVectors( m_vertexes, m_indexes, false, true );
	CreateBuffers();
	CreateDebugTangentBasisVectors();
	
}

void Prop::CreateSphere( char const* materialFileName )
{
	delete m_material;
	m_material = new Material( materialFileName, g_theRenderer );
	AddVertsForSphere3D( m_vertexes, m_indexes, Vec3(), 1.f, Rgba8::WHITE, AABB2::IDENTITY, 32, 64 );

	CalculateTangentSpaceBasisVectors( m_vertexes, m_indexes, false, true );
	CreateBuffers();
	CreateDebugTangentBasisVectors();
}

void Prop::DebugRender() const
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

void Prop::CreateBuffers()
{
	if (m_vertexBuffer) {
		delete m_vertexBuffer;
	}
	if (m_indexBuffer) {
		delete m_indexBuffer;
	}
	size_t sizeInByte = m_vertexes.size() * sizeof( Vertex_PCUTBN );
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer( sizeInByte, sizeof( Vertex_PCUTBN ) );
	g_theRenderer->CopyCPUToGPU( m_vertexes.data(), sizeInByte, m_vertexBuffer );

	sizeInByte = m_indexes.size() * sizeof( int );
	m_indexBuffer = g_theRenderer->CreateIndexBuffer( sizeInByte );
	g_theRenderer->CopyCPUToGPU( m_indexes.data(), sizeInByte, m_indexBuffer );
}

void Prop::CreateDebugTangentBasisVectors()
{
	delete m_debugVertexBuffer;
	std::vector<Vertex_PCU> debugVerts;
	for (auto& vert : m_vertexes) {
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

