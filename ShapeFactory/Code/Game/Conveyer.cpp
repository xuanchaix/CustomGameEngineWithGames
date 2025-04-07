#include "Game/Conveyer.hpp"
#include "Game/Widget.hpp"


Conveyer::Conveyer( IntVec2 const& LBCoords )
	:Building(LBCoords)
{
	m_buildingType = BuildingType::Conveyer;
}

Conveyer::~Conveyer()
{

}

void Conveyer::Render() const
{

	std::vector<Vertex_PCU> verts;
	DrawConveyer( this, m_leftBottomCoords, m_dir, Rgba8::WHITE );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );
}

bool Conveyer::HasMoreThanOneEntrance() const
{
	int count = 0;
	if (m_rear) {
		count++;
	}
	if (m_left) {
		count++;
	}
	if (m_right) {
		count++;
	}
	return count >= 2;
}

Vec2 Conveyer::GetExitPos() const
{
	if (m_dir == Direction::Down) {
		return Vec2( m_leftBottomCoords ) + Vec2( 0.5f, 0.f );
	}
	else if (m_dir == Direction::Left) {
		return Vec2( m_leftBottomCoords ) + Vec2( 0.f, 0.5f );
	}
	else if (m_dir == Direction::Right) {
		return Vec2( m_leftBottomCoords ) + Vec2( 1.f, 0.5f );
	}
	else if (m_dir == Direction::Up) {
		return Vec2( m_leftBottomCoords ) + Vec2( 0.5f, 1.f );
	}
	return Vec2(m_leftBottomCoords) + Vec2( 0.5f, 0.5f );
}

