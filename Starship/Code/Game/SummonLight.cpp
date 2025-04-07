#include "Game/SummonLight.hpp"
#include "Game/Game.hpp"

SummonLight::SummonLight( Vec2 startPos, Game* game, Vec2 const& targetPos, float travelTime, Entity* summonEntity, Rgba8 const& color )
	:Entity(startPos, game)
	,m_startPosition(startPos)
	,m_targetPosition(targetPos)
	,m_summonEntity(summonEntity)
	,m_travelTime(travelTime)
{
	m_color = color;
	m_type = EntityType::debris;
	m_orientationDegrees = (targetPos - startPos).GetOrientationDegrees();
}

SummonLight::~SummonLight()
{

}

void SummonLight::Update()
{
	float deltaTime = g_theGame->m_gameClock->GetDeltaSeconds();
	m_timer += deltaTime;
	if (m_timer >= m_travelTime) {
		if (m_summonEntity) {
			m_game->SpawnNewEntity( m_summonEntity );
		}
		Die();
	}
	else {
		float ratio = m_timer / m_travelTime;
		m_position = m_startPosition * (1 - ratio) + m_targetPosition * ratio;
	}
}

void SummonLight::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForCapsule2D( verts, m_position, m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees, 5.f ), 0.5f, m_color );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void SummonLight::Die()
{
	m_isDead = true;
	m_isGarbage = true;
	if (m_summonEntity) {
		m_game->SpawnDebris( m_summonEntity );
	}
}
