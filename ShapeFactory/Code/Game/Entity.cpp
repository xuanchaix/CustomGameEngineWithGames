#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"


Entity::Entity( Vec2 const& startPos, Game* game):
	m_position(startPos),
	m_game(game)
{
}

Entity::~Entity() {}

void Entity::DebugRender() const
{
}


void Entity::RenderUI() const
{
}

Mat44 Entity::GetModelConstants() const
{
	Mat44 ret = Mat44::CreateTranslation2D( m_position );
	ret.Append( Mat44::CreateZRotationDegrees( m_orientationDegrees ) );
	return ret;
}

Mat44 Entity::GetPositionModelConstants() const
{
	return Mat44::CreateTranslation2D( m_position );
}

void Entity::EntityAddVertsForHealthBar( std::vector<Vertex_PCU>& verts ) const
{
	if (m_entityType == EntityType::Enemy && !m_isDead) {
		constexpr float healthBarHeight = 0.1f;
		constexpr float healthBarHalfHeight = healthBarHeight * 0.5f;
		constexpr float healthBarAboveHeight = 0.2f;
		constexpr float healthBarRatio = 0.8f;
		float width = (m_physicsBounds.m_maxs.x - m_physicsBounds.m_mins.x);
		float healthBarBaseY = GetWorldPhysicsBounds().m_maxs.y + healthBarAboveHeight;
		float healthBarLeftX = GetWorldPhysicsBounds().m_mins.x + (1.f - healthBarRatio) * 0.5f * width;
		width *= healthBarRatio;
		float curHealthRatio = m_health / m_maxHealth;

		AddVertsForAABB2D( verts, AABB2( Vec2( healthBarLeftX, healthBarBaseY - healthBarHalfHeight ), Vec2( healthBarLeftX + width, healthBarBaseY + healthBarHalfHeight ) ), Rgba8::RED );
		AddVertsForAABB2D( verts, AABB2( Vec2( healthBarLeftX + width * (1.f - curHealthRatio), healthBarBaseY - healthBarHalfHeight ), Vec2( healthBarLeftX + width, healthBarBaseY + healthBarHalfHeight ) ), Rgba8( 0, 255, 0 ) );
	}
}

Vec2 Entity::GetForwardNormal() const
{
	return Vec2::MakeFromPolarDegrees( m_orientationDegrees );
}

bool Entity::IsAlive() const
{
	return !m_isDead;
}

void Entity::BeAttacked( float hit )
{
	m_health -= hit;
	if (m_health > m_maxHealth) {
		m_health = m_maxHealth;
	}
	if (m_entityType == EntityType::Projectile) {
		if (m_health <= 0.f) {
			m_isDead = true;
			return;
		}
	}
}

AABB2 Entity::GetWorldPhysicsBounds() const
{
	return AABB2( m_position - m_physicsBounds.m_maxs * 0.5f, m_position + m_physicsBounds.m_maxs * 0.5f );
}

