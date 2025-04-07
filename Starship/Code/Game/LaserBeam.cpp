#include "Game/LaserBeam.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"

LaserBeam::LaserBeam( Vec2 startPos, Game* game, float forwardDegrees, Entity const* ship, bool isEnemyAttack, float range )
	:Entity(startPos, game)
{
	m_isEnemyAttack = isEnemyAttack;
	m_ship = ship;
	m_orientationDegrees = forwardDegrees;
	m_type = EntityType::debris;
	m_range = range / 2.f;
}

LaserBeam::~LaserBeam()
{

}

void LaserBeam::Update()
{
	float deltaTime = g_theGame->m_gameClock->GetDeltaSeconds();
	if (m_lifespan <= 0.f) {
		Die();
		return;
	}
	if (!m_isEnemyAttack) {
		m_orientationDegrees = m_ship->m_orientationDegrees;
		m_position = m_ship->m_position;
	}
	m_game->DealLaserDamage( this );
	m_lifespan -= deltaTime;
	if (m_lifespan < 0.f) {
		m_lifespan = 0.f;
	}
}

void LaserBeam::Render() const
{
	Rgba8 colorInner( 204, 102, 0, 255 );
	Rgba8 colorOuter( 204, 102, 0, 0 );
	float rngNum = m_game->m_randNumGen->RollRandomFloatInRange( 0.2f + m_range, 1.f + m_range );
	Vertex_PCU laserVerts[18] = {
		Vertex_PCU( Vec3( 0.f, m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 0.f, -m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 300.f, m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 0.f, -m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 300.f, -m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 300.f, m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 0.f, -m_range - rngNum, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 0.f, -m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 300.f, -m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 0.f, -m_range - rngNum, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 300.f, -m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 300.f, -m_range - rngNum, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 0.f, m_range + rngNum, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 0.f, m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 300.f, m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 0.f, m_range + rngNum, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 300.f, m_range, 0.f ), colorInner, Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 300.f, m_range + rngNum, 0.f ), colorInner, Vec2( 0, 0 ) ),
	};

	if (!m_isEnemyAttack) {
		TransformVertexArrayXY3D( 18, laserVerts, 1.f, m_ship->m_orientationDegrees, ((PlayerShip*)m_ship)->GetNosePosition() );
	}
	else {
		TransformVertexArrayXY3D( 18, laserVerts, 1.f, m_orientationDegrees, m_position );
	}	
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 18, laserVerts );
}

void LaserBeam::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}
