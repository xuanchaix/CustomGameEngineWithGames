#include "Game/Widget.hpp"

SFWidget::SFWidget( AABB2 const& responseArea, std::string const& text, std::string const& eventNameToFire )
	:m_responseArea(responseArea)
	,m_text(text)
	,m_eventToFire(eventNameToFire)
{

}

void SFWidget::Execute( Vec2 const& cursorPos )
{
	if (m_isActive && m_responseArea.IsPointInside(cursorPos)) {
		FireEvent( m_eventToFire, m_args );
	}
}

void SFWidget::SetCenter( Vec2 newCenter )
{
	m_responseArea.SetCenter( newCenter );
}

void SFWidget::Render() const
{
	if (!m_isActive) {
		return;
	}
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, m_responseArea, Rgba8::WHITE );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants( Mat44::CreateTranslation3D( Vec3( 0.f, 0.f, 1.f ) ) );
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );

	if (m_type == SFWidgetType::Text) {
		std::vector<Vertex_PCU> textVerts;
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, m_responseArea, m_fontSize, m_text, Rgba8::BLACK );

		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
		g_theRenderer->SetModelConstants( Mat44::CreateTranslation3D( Vec3( 0.f, 0.f, 1.f ) ) );
		g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

		g_theRenderer->DrawVertexArray( textVerts );
	}
	else if (m_type == SFWidgetType::Graph) {
		verts.clear();
		AddVertsForAABB2D( verts, m_responseArea, Rgba8::WHITE, m_uv );

		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( m_texture );
		g_theRenderer->SetModelConstants( Mat44::CreateTranslation3D( Vec3( 0.f, 0.f, 1.f ) ) );
		g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

		g_theRenderer->DrawVertexArray( verts );
	}

}
