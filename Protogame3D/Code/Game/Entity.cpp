#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"


Entity::Entity( Game* game ):
	m_game(game)
{
	m_position = Vec3( 0, 0, 0 );
	m_angularVelocity = EulerAngles();
	m_cosmeticRadius = 0.f;
	m_physicsRadius = 0.f;
	m_orientation = EulerAngles();
	m_health = 1;
	m_color = Rgba8();
	m_maxHealth = 1;
}

Entity::~Entity() {}

void Entity::DebugRender() const
{
	//DebugDrawLine( m_position, m_orientationDegrees, m_cosmeticRadius, 0.2f, Rgba8( 255, 0, 0, 255 ) );
	//DebugDrawLine( m_position, m_orientationDegrees + 90.f, m_cosmeticRadius, 0.2f, Rgba8( 0, 255, 0, 255 ) );
	//DebugDrawRing( m_position, m_cosmeticRadius, 0.2f, Rgba8( 255, 0, 255, 255 ) );
	//DebugDrawRing( m_position, m_physicsRadius, 0.2f, Rgba8( 0, 255, 255, 255 ) );
	//DebugDrawLine( m_position, Atan2Degrees( m_velocity.y, m_velocity.x ), m_velocity.GetLength(), 0.2f, Rgba8( 255, 255, 0, 255 ) );
}


void Entity::RenderUI() const
{
}

Vec3 Entity::GetForwardNormal() const
{
	return m_orientation.GetIFwd();
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

Mat44 Entity::GetModelMatrix() const
{
	Mat44 retMat = Mat44::CreateTranslation3D( m_position );
	retMat.Append( m_orientation.GetAsMatrix_IFwd_JLeft_KUp() );
	return retMat;
}

