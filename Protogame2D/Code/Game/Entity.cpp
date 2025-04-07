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
}

Vec2 Entity::GetForwardNormal() const
{
	return m_velocity.GetNormalized();
}

bool Entity::IsAlive() const
{
	return !m_isDead;
}

void Entity::BeAttacked( int hit )
{
	m_health -= hit;
	if (m_health > m_maxHealth) {
		m_health = m_maxHealth;
	}
}

