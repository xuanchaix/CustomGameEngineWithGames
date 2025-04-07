#include "Game/Building.hpp"
#include "Game/GameCommon.hpp"

Building::Building( Vec2 const& startPos, Map* map )
	:Entity( startPos, map )
{
	m_speed = 0.f;
	m_velocity = Vec2( 0, 0 );
	m_orientationDegrees = 0.f;
	m_angularVelocity = 0.f;

	m_physicsRadius = 1.f;
	m_cosmeticRadius = 1.f;

	m_maxHealth = 100.f;
	m_health = m_maxHealth;

	m_type = EntityType::_BUILDING;
	m_faction = EntityFaction::FACTION_GOOD;

	m_target = nullptr;
	m_curState = 1;

	m_shootTime = 0.f;
	m_damage = 0.f;

	m_isPushedByEntities = false;
	m_doesPushEntities = true;
	m_isPushedByWalls = false;
	m_isHitByBullets = true;
	m_isActor = true;
	m_isRanged = false;

	m_driveHalfAngleDegrees = 0.f;
	m_shootHalfAngleDegrees = 0.f;
}

Building::~Building()
{

}

void Building::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}

	if (m_health < 0.f) {
		m_isDead = true;
		return;
	}
}

void Building::Render() const
{
	if (m_isDead == true) {
		return;
	}

	std::vector<Vertex_PCU> verts;
	verts.reserve( 300 );
	AddVertsForDisc2D( verts, m_position, m_physicsRadius, Rgba8( 153, 204, 255 ) );
	AddVertsForDisc2D( verts, m_position, m_physicsRadius * 0.75f, Rgba8( 255, 153, 153 ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void Building::Die()
{
	m_isDead = true;
}

void Building::RenderUI() const
{
	if (m_isDead || m_isGarbage || m_health <= 0.f) {
		return;
	}
	float remainHealthRatio = (float)m_health / (float)m_maxHealth;
	if (remainHealthRatio < 0.f) {
		remainHealthRatio = 0.f;
	}
	std::vector<Vertex_PCU> healthBarVerts;
	healthBarVerts.reserve( 12 );
	AddVertsForAABB2D( healthBarVerts, AABB2( m_position + Vec2( -1.f, m_cosmeticRadius + 0.1f ), m_position + Vec2( 1.f, m_cosmeticRadius + 0.2f ) ), Rgba8( 255, 0, 0, 255 ), AABB2::IDENTITY );
	AddVertsForAABB2D( healthBarVerts, AABB2( m_position + Vec2( -1.f, m_cosmeticRadius + 0.1f ), m_position + Vec2( 1.f - 2.f * (1 - remainHealthRatio), m_cosmeticRadius + 0.2f ) ), Rgba8( 0, 255, 0, 255 ), AABB2::IDENTITY );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( healthBarVerts );
}

void Building::BeAttacked( float hit )
{
	m_health -= hit;
}
