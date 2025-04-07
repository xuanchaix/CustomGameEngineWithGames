#include "Game/Shield.hpp"
#include "Game/Game.hpp"

Shield::Shield( Vec2 startPos, Game* game, Entity* owner, float size, Rgba8 const& color )
	:Entity(startPos, game), m_owner(owner), m_size(size)
{
	m_color = color;
	m_cosmeticRadius = size * 0.5f;
}

Shield::~Shield()
{

}

void Shield::Update()
{
	if (m_isDead) {
		return;
	}
	if (m_owner->m_isDead) {
		Die();
		return;
	}
	m_position = m_owner->m_position;
}

void Shield::Render() const
{
	constexpr int NUM_OF_VERTS = 90;
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	for (int i = 0; i < NUM_OF_VERTS / 3; i++) {
		verts.emplace_back( m_position, Rgba8( m_color.r, m_color.g, m_color.b, 0 ) );
		verts.emplace_back( m_position + Vec2( CosRadians( 6 * PI / NUM_OF_VERTS * i ) * m_size, SinRadians( 6 * PI / NUM_OF_VERTS * i ) * m_size ), m_color );
		verts.emplace_back( m_position + Vec2( CosRadians( 6 * PI / NUM_OF_VERTS * (i + 1) ) * m_size, SinRadians( 6 * PI / NUM_OF_VERTS * (i + 1) ) * m_size ), m_color );
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void Shield::Die()
{
	m_isGarbage = true;
	m_isDead = true;
	m_game->SpawnDebris( this );
}

