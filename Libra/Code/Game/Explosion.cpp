#include "Game/Explosion.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

Explosion::Explosion( Vec2 const& startPos, Map* map, float sizeFactor, float lifeSpanSeconds )
	:Entity(startPos, map)
	,m_lifeSpanSeconds(lifeSpanSeconds)
{
	m_orientationDegrees = g_theGame->m_randNumGen->RollRandomFloatInRange( 0.f, 360.f );
	m_angularVelocity = 60.f;
	m_physicsRadius = 0.f;
	m_cosmeticRadius = EXPLOSION_COSMETIC_RADIUS * sizeFactor;
	m_type = EntityType::_EXPLOSION;

	m_isPushedByEntities = false;
	m_doesPushEntities = false;
	m_isPushedByWalls = false;
	m_isHitByBullets = false;
	m_isActor = true;

	if (lifeSpanSeconds >= 0.6f) {
		m_anim = g_theApp->GetAnimation( AnimationName::EntityExplosion );
	}
	else {
		m_anim = g_theApp->GetAnimation( AnimationName::BulletExplosion );
	}
}

Explosion::~Explosion()
{

}

void Explosion::Update( float deltaTime )
{
	if (m_ageSeconds >= m_lifeSpanSeconds) {
		Die();
		return;
	}
	m_ageSeconds += deltaTime;
}

void Explosion::Render() const
{
	g_theRenderer->SetBlendMode( BlendMode::ADDITIVE );
	std::vector<Vertex_PCU> verts;
	verts.reserve( 6 );
	AddVertsForOBB2D( verts, OBB2( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees ), Vec2( m_cosmeticRadius, m_cosmeticRadius ) ),
		Rgba8( 255, 255, 255, (unsigned char)(255.f * Maxf( (1 - m_ageSeconds / m_lifeSpanSeconds), 0.f )) ),
		m_anim->GetSpriteDefAtTime(m_ageSeconds).GetUVs() );
	g_theRenderer->BindTexture( m_anim->GetTexture() );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
}

void Explosion::Die()
{
	m_isGarbage = true;
	m_isDead = true;
}

