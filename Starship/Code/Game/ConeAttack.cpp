#include "Game/ConeAttack.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Game.hpp"

ConeAttack::ConeAttack( Vec2 startPos, Game* game, float sectorForwardDegrees, Entity const* ship, bool isEnemyAttack, float radius, float apertureDegrees )
	:Entity(startPos, game)
{
	m_isEnemyAttack = isEnemyAttack;
	m_ship = ship;
	m_sectorApertureDegrees = apertureDegrees;
	m_sectorForwardDegrees = sectorForwardDegrees;
	m_sectorRadius = radius;
	m_type = EntityType::debris;
}

ConeAttack::~ConeAttack()
{

}

void ConeAttack::Update()
{
	float deltaTime = g_theGame->m_gameClock->GetDeltaSeconds();
	if (m_lifespan <= 0.f || !m_ship->IsAlive()) {
		Die();
		return;
	}
	m_lifespan -= deltaTime;
	if (m_lifespan < 0.f) {
		m_lifespan = 0.f;
	}
}

void ConeAttack::Render() const
{
	if (!m_ship->IsAlive()) {
		return;
	}
	constexpr int NUM_OF_TRIANGLES = 16;
	constexpr int NUM_OF_VERTS = NUM_OF_TRIANGLES * 3;
	Vertex_PCU coneVerts[NUM_OF_VERTS];
	float DEGREE_PER_TRI = m_sectorApertureDegrees / NUM_OF_TRIANGLES;
	for (int i = 0; i < NUM_OF_TRIANGLES; i++) {
		if (!m_isEnemyAttack) {
			coneVerts[3 * i] = Vertex_PCU( Vec3( 0.f, 0.f, 0.f ), Rgba8( 102, 255, 102, (unsigned char)(200 /** m_lifespan*/) ), Vec2( 0.f, 0.f ) );
			coneVerts[3 * i + 1] = Vertex_PCU( Vec3( m_sectorRadius * (CONE_LIFETIME_SECONDS - m_lifespan) / CONE_LIFETIME_SECONDS * CosDegrees( i * DEGREE_PER_TRI ), m_sectorRadius * (CONE_LIFETIME_SECONDS - m_lifespan) / CONE_LIFETIME_SECONDS * SinDegrees( i * DEGREE_PER_TRI ), 0.f ), Rgba8( 76, 153, 0, (unsigned char)(50 /** m_lifespan*/) ), Vec2( 0.f, 0.f ) );
			coneVerts[3 * i + 2] = Vertex_PCU( Vec3( m_sectorRadius * (CONE_LIFETIME_SECONDS - m_lifespan) / CONE_LIFETIME_SECONDS * CosDegrees( (i + 1) * DEGREE_PER_TRI ), m_sectorRadius * (CONE_LIFETIME_SECONDS - m_lifespan) / CONE_LIFETIME_SECONDS * SinDegrees( (i + 1) * DEGREE_PER_TRI ), 0.f ), Rgba8( 76, 153, 0, (unsigned char)(50 /** m_lifespan*/) ), Vec2( 0.f, 0.f ) );
		}
		else {
			coneVerts[3 * i] = Vertex_PCU( Vec3( 0.f, 0.f, 0.f ), Rgba8( 255, 255, 102, (unsigned char)(200 /** m_lifespan*/) ), Vec2( 0.f, 0.f ) );
			coneVerts[3 * i + 1] = Vertex_PCU( Vec3( m_sectorRadius * (CONE_LIFETIME_SECONDS - m_lifespan) / CONE_LIFETIME_SECONDS * CosDegrees( i * DEGREE_PER_TRI ), m_sectorRadius * (CONE_LIFETIME_SECONDS - m_lifespan) / CONE_LIFETIME_SECONDS * SinDegrees( i * DEGREE_PER_TRI ), 0.f ), Rgba8( 255, 255, 102, (unsigned char)(50 /** m_lifespan*/) ), Vec2( 0.f, 0.f ) );
			coneVerts[3 * i + 2] = Vertex_PCU( Vec3( m_sectorRadius * (CONE_LIFETIME_SECONDS - m_lifespan) / CONE_LIFETIME_SECONDS * CosDegrees( (i + 1) * DEGREE_PER_TRI ), m_sectorRadius * (CONE_LIFETIME_SECONDS - m_lifespan) / CONE_LIFETIME_SECONDS * SinDegrees( (i + 1) * DEGREE_PER_TRI ), 0.f ), Rgba8( 255, 255, 102, (unsigned char)(50 /** m_lifespan*/) ), Vec2( 0.f, 0.f ) );
		}
	}

	if (!m_isEnemyAttack) {
		TransformVertexArrayXY3D( NUM_OF_VERTS, coneVerts, 1.f, m_ship->m_orientationDegrees - m_sectorApertureDegrees / 2, ((PlayerShip*)m_ship)->GetNosePosition() );
	}
	else {
		if (m_ship->IsAlive()) {
			TransformVertexArrayXY3D( NUM_OF_VERTS, coneVerts, 1.f, m_ship->m_orientationDegrees - m_sectorApertureDegrees / 2, m_ship->m_position + Vec2::MakeFromPolarDegrees( m_ship->m_orientationDegrees, m_ship->m_physicsRadius ) );
		}
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( NUM_OF_VERTS, coneVerts );
}

void ConeAttack::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}
