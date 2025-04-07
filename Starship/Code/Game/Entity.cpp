#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"


Entity::Entity( Vec2 startPos, Game* game):
	m_position(startPos),
	m_game(game)
{
	m_angularVelocity = 0.f;
	m_cosmeticRadius = 0.f;
	m_physicsRadius = 0.f;
	m_orientationDegrees = 0.f;
	m_health = 1;
	m_color = Rgba8();
	m_maxHealth = 1;
}

Entity::~Entity() {}

void Entity::DebugRender() const
{
	DebugDrawLine( m_position, m_orientationDegrees, m_cosmeticRadius, 0.2f, Rgba8( 255, 0, 0, 255 ) );
	DebugDrawLine( m_position, m_orientationDegrees + 90.f, m_cosmeticRadius, 0.2f, Rgba8( 0, 255, 0, 255 ) );
	DebugDrawRing( m_position, m_cosmeticRadius, 0.2f, Rgba8( 255, 0, 255, 255 ) );
	DebugDrawRing( m_position, m_physicsRadius, 0.2f, Rgba8( 0, 255, 255, 255 ) );
	DebugDrawLine( m_position, Atan2Degrees( m_velocity.y, m_velocity.x ), m_velocity.GetLength(), 0.2f, Rgba8( 255, 255, 0, 255 ) );
}


void Entity::RenderUI() const
{
	if (m_isDead || m_isGarbage || m_health <= 0.f) {
		return;
	}
	if (m_health == m_maxHealth) {
		return;
	}
	if (m_type == EntityType::playerShip) {
		return;
	}
	float remainHealthRatio = (float)m_health / (float)m_maxHealth;
	if (remainHealthRatio < 0.f) {
		remainHealthRatio = 0.f;
	}
	Vertex_PCU healthBarVerts[12];
	healthBarVerts[0] = Vertex_PCU( Vec3( -1.f, -0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[1] = Vertex_PCU( Vec3( 1.f, -0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[2] = Vertex_PCU( Vec3( -1.f, 0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[3] = Vertex_PCU( Vec3( 1.f, -0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[4] = Vertex_PCU( Vec3( -1.f, 0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[5] = Vertex_PCU( Vec3( 1.f, 0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[6] = Vertex_PCU( Vec3( -1.f, -0.2f, 0 ), Rgba8( 0, 255, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[7] = Vertex_PCU( Vec3( 2 * remainHealthRatio - 1, -0.2f, 0 ), Rgba8( 0, 255, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[8] = Vertex_PCU( Vec3( -1.f, 0.2f, 0 ), Rgba8( 0, 255, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[9] = Vertex_PCU( Vec3( 2 * remainHealthRatio - 1, -0.2f, 0 ), Rgba8( 0, 255, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[10] = Vertex_PCU( Vec3( -1.f, 0.2f, 0 ), Rgba8( 0, 255, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[11] = Vertex_PCU( Vec3( 2 * remainHealthRatio - 1, 0.2f, 0 ), Rgba8( 0, 255, 0, 255 ), Vec2( 0, 0 ) );
	TransformVertexArrayXY3D( 12, healthBarVerts, 1.2f, 0.f, m_position + Vec2( 0, m_cosmeticRadius ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 12, healthBarVerts );
}

bool Entity::IsOffscreen() const
{
	return m_position.x - m_cosmeticRadius > WORLD_SIZE_X || m_position.x + m_cosmeticRadius < 0.f 
		|| m_position.y - m_cosmeticRadius > WORLD_SIZE_Y || m_position.y + m_cosmeticRadius < 0.f;
}

Vec2 Entity::GetForwardNormal() const
{
	return m_velocity.GetNormalized();
}

bool Entity::IsAlive() const
{
	return !m_isDead;
}

bool Entity::IsEnemy() const
{
	return m_isEnemy;
}

bool Entity::IsActor() const
{
	return m_isActor;
}

void Entity::BeAttacked( int hit )
{
	m_health -= hit;
	if (m_health > m_maxHealth) {
		m_health = m_maxHealth;
	}
}

