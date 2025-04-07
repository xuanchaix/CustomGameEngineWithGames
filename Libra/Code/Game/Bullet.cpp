#include "Game/Bullet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"

Bullet::Bullet( Vec2 const& startPos, Map* map, float orientationDegrees, EntityFaction faction, EntityType type, float damage,
	int bounceTimes, float inheritedSpeed, Entity* target )
	:Entity(startPos, map)
{
	if (type != EntityType::_FLAME_BULLET) {
		m_speed = g_gameConfigBlackboard.GetValue( "defaultBulletSpeed", 4.f ) + inheritedSpeed;
		m_velocity = Vec2::MakeFromPolarDegrees( orientationDegrees, m_speed );
		m_orientationDegrees = orientationDegrees;
		m_angularVelocity = 60.f;
		m_physicsRadius = BULLET_PHYSICS_RADIUS;
		m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
		m_maxHealth = 1;
		m_health = m_maxHealth;
		m_type = type;
		m_faction = faction;
		m_curState = 1;
		m_damage = damage;
		m_target = target;
	}
	// flame bullet
	else {
		m_speed = 3.f + inheritedSpeed;
		m_velocity = Vec2::MakeFromPolarDegrees( orientationDegrees, m_speed );
		m_orientationDegrees = g_theGame->m_randNumGen->RollRandomFloatInRange( 0.f, 360.f );
		m_angularVelocity = g_theGame->m_randNumGen->RollRandomFloatInRange( 100.f, 500.f ) * (float)(g_theGame->m_randNumGen->RollRandomIntInRange(0, 1) * 2 - 1);
		m_physicsRadius = BULLET_PHYSICS_RADIUS;
		m_cosmeticRadius = BULLET_COSMETIC_RADIUS * 5;
		m_maxHealth = 1;
		m_health = m_maxHealth;
		m_type = type;
		m_faction = faction;
		m_target = nullptr;
		m_curState = 1;
		m_damage = damage;
	}

	if (type == EntityType::_FLAME_BULLET) {
		m_anim = g_theApp->GetAnimation( AnimationName::FlameBullet );
	}
	else if (faction == EntityFaction::FACTION_EVIL) {
		if (type == EntityType::_BULLET) {
			m_texture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyBullet.png" );
		}
		else if (type == EntityType::_GUIDED_BULLET) {
			m_speed = 1.f;
			m_texture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyShell.png" );
		}
	}
	else if (faction == EntityFaction::FACTION_GOOD) {
		m_texture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyBullet.png" );
	}

	m_bounceTimesLeft = bounceTimes;

	m_lastFramePosition = startPos;
	m_isPushedByEntities = false;
	m_doesPushEntities = false;
	m_isPushedByWalls = false;
	m_isHitByBullets = false;
	m_isActor = false;
	//m_isAmphibious = true;

	m_bounceHalfVarianceDegrees = g_gameConfigBlackboard.GetValue( "bulletBounceHalfVarianceDegrees", 3.f );
}

Bullet::~Bullet()
{
}

void Bullet::Update( float deltaTime )
{
	if (m_type == EntityType::_FLAME_BULLET) {
		m_orientationDegrees += m_angularVelocity * deltaTime;
		m_lifeSpanSeconds += deltaTime;
		if (m_lifeSpanSeconds > 1.f) {
			Die();
		}
		if (m_isDead) {
			m_cosmeticRadius = BULLET_COSMETIC_RADIUS * 3.f;
		}
	}

	if (!IsAlive() && m_type != EntityType::_FLAME_BULLET) {
		return;
	}

	m_lastFramePosition = m_position;
	if (m_type == EntityType::_GUIDED_BULLET && m_target && m_target->IsAlive()) {
		m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, (m_target->m_position - m_position).GetOrientationDegrees(), m_angularVelocity * deltaTime );
		m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_speed );
	}
	m_position += m_velocity * deltaTime;
}

void Bullet::Render() const
{
	if (m_type == EntityType::_FLAME_BULLET) {
		g_theRenderer->SetBlendMode( BlendMode::ADDITIVE );
		std::vector<Vertex_PCU> verts;
		verts.reserve( 6 );
		AddVertsForOBB2D( verts, OBB2( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees ), Vec2( m_cosmeticRadius, m_cosmeticRadius ) ),
			Rgba8( 255, 255, 255, (unsigned char)(255.f * Maxf( (1 - m_lifeSpanSeconds / 1.f), 0.f )) ),
			m_anim->GetSpriteDefAtTime( m_lifeSpanSeconds ).GetUVs() );
		g_theRenderer->BindTexture( m_anim->GetTexture() );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( verts );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	}
	else {
		std::vector<Vertex_PCU> verts;
		verts.reserve( 6 );
		AddVertsForOBB2D( verts, OBB2( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees ), Vec2( m_cosmeticRadius, m_cosmeticRadius * 0.5f ) ), Rgba8( 255, 255, 255, 255 ) );
		g_theRenderer->BindTexture( m_texture );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( verts );
	}
}

void Bullet::Die()
{
	if (m_type != EntityType::_FLAME_BULLET) {
		m_map->SpawnNewEntity( EntityType::_EXPLOSION, m_position, m_faction, this );
	}
	m_isDead = true;
	m_isGarbage = true;
}

//void Bullet::DebugRender() const
//{

//}

void Bullet::RenderUI() const
{

}

void Bullet::BounceOff( Vec2 const& normal )
{
	if (m_bounceTimesLeft <= 0) {
		if (m_type == EntityType::_FLAME_BULLET) {
			m_isDead = true;
		}
		else {
			Die();
		}
		return;
	}

	m_map->PlaySound( AudioName::BulletBounce, m_position );
	m_bounceTimesLeft--;
	m_velocity.Reflect( normal );
	m_orientationDegrees += GetShortestAngularDispDegrees( m_orientationDegrees, m_velocity.GetOrientationDegrees() );
	m_orientationDegrees += g_theGame->m_randNumGen->RollRandomFloatInRange( -m_bounceHalfVarianceDegrees, m_bounceHalfVarianceDegrees );
	m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_speed );
}

void Bullet::BeAttacked( float hit )
{
	m_health -= hit;
}
