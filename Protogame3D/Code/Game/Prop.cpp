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

}

void Prop::Update()
{
	float deltaSeconds = g_theGame->m_gameClock->GetDeltaSeconds();
	m_orientation.m_yawDegrees += m_angularVelocity.m_yawDegrees * deltaSeconds;
	m_orientation.m_pitchDegrees += m_angularVelocity.m_pitchDegrees * deltaSeconds;
	m_orientation.m_rollDegrees += m_angularVelocity.m_rollDegrees * deltaSeconds;
}

void Prop::Render() const
{
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( m_texture );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	if ((int)m_indexes.size() > 0) {
		g_theRenderer->DrawVertexArray( m_vertexes, m_indexes );
	}
	else {
		g_theRenderer->DrawVertexArray( m_vertexes );
	}
}

void Prop::Die()
{
	m_isDead = true;
}

