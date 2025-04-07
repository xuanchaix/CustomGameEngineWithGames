#include "Game/Button.hpp"
#include "Game/TranslationUtils.hpp"
#include "Game/SM_BitMapFont.hpp"

RectButton::RectButton( AABB2 const& bounds, std::string const& name, Rgba8 const& color, std::string const& eventToFireOnClick, std::string const& eventToFireOnRelease, std::string const& text, Texture* texture )
	:m_bounds(bounds)
	,m_name(name)
	,m_baseColor(color)
	,m_eventToFireOnClick(eventToFireOnClick)
	,m_eventToFireOnRelease(eventToFireOnRelease)
	,m_texture(texture)
	,m_text(text)
{

}

RectButton::~RectButton()
{

}

bool RectButton::IsPointInside( Vec2 const& point ) const
{
	if (m_visible && !m_disable) {
		return m_bounds.IsPointInside( point );
	}
	return false;
}

void RectButton::OnButtonClicked( EventArgs & args )
{
	FireEvent( m_eventToFireOnClick, args );
}

void RectButton::OnCursorHover()
{
	if (m_disable) {
		return;
	}
	m_renderColor.r = m_renderColor.r + 20 > 255 ? 255 : m_renderColor.r + 20;
	m_renderColor.g = m_renderColor.g + 20 > 255 ? 255 : m_renderColor.r + 20;
	m_renderColor.b = m_renderColor.b + 20 > 255 ? 255 : m_renderColor.r + 20;
}

void RectButton::OnButtonReleased( EventArgs& args )
{
	g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Click.wav" ) );
	FireEvent( m_eventToFireOnRelease, args );
}

void RectButton::SetDisable( bool disable )
{
	m_disable = disable;
}

void RectButton::SetVisible( bool visible )
{
	m_visible = visible;
}

void RectButton::ToggleDisable()
{
	m_disable = !m_disable;
}

void RectButton::ToggleVisible()
{
	m_visible = !m_visible;
}

void RectButton::BeginFrame()
{
	m_renderColor = m_baseColor;
	if (m_disable) {
		m_renderColor = Rgba8( 192, 192, 192 );
	}
}

void RectButton::Render() const
{
	if (!m_visible) {
		return;
	}
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForAABB2D( verts, m_bounds, m_renderColor, AABB2::IDENTITY );

	g_theRenderer->BindTexture( m_texture );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
	verts.clear();
	AddVertsForTextPlaceHolder( verts, m_bounds, 20.f, m_text, Rgba8( 0, 0, 0 ), 0.618f, Vec2( 0.5f, 0.5f ), TextBoxMode::AUTO_NEW_LINE );
	if (g_gameLanguage == SM_GameLanguage::ENGLISH) {
		g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	}
	else if (g_gameLanguage == SM_GameLanguage::ZH) {
		g_theRenderer->BindTexture( &g_chineseFont->GetTexture() );
	}
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}
