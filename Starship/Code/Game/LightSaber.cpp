#include "Game/LightSaber.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"
#include "Engine/Core/EngineCommon.hpp"

LightSaber::LightSaber( Vec2 startPos, Game* game, float forwardDegrees, Entity const* ship, float length )
	:Entity(startPos, game)
{
	m_ship = ship;
	m_length = length;
	m_forwardDegrees = forwardDegrees;
	m_orientationDegrees = ship->m_orientationDegrees + forwardDegrees;
	m_type = EntityType::debris;
}

LightSaber::~LightSaber()
{

}

void LightSaber::Update()
{
	m_orientationDegrees = m_ship->m_orientationDegrees + m_forwardDegrees;
	m_position = m_ship->m_position;
	m_game->DealSaberDamage( this );
}

void LightSaber::Render() const
{
	Rgba8 colorInner( 51, 255, 51, 255 );
	Rgba8 colorOuter( 51, 255, 51, 0 );
	float rngNum = m_game->m_randNumGen->RollRandomFloatInRange( 0.1f, 0.4f );
	Vertex_PCU laserVerts[18] = {
		Vertex_PCU( Vec3( 0.f, m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 0.f, -m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( m_length, m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 0.f, -m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( m_length, -m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( m_length, m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 0.f, -m_range - rngNum, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 0.f, -m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( m_length, -m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 0.f, -m_range - rngNum, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( m_length, -m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( m_length, -m_range - rngNum, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 0.f, m_range + rngNum, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 0.f, m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( m_length, m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 0.f, m_range + rngNum, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( m_length, m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( m_length, m_range + rngNum, 0.f ), colorInner, Vec2( 0, 0 ) ),
	};

	TransformVertexArrayXY3D( 18, laserVerts, 1.f, m_orientationDegrees, ((PlayerShip*)m_ship)->GetNosePosition() );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 18, laserVerts );
}

void LightSaber::Die()
{

}
