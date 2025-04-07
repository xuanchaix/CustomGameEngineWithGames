#pragma once
#include "Game/GameCommon.hpp"
// need further construction
// UI System

class RectButton {
public:
	RectButton( AABB2 const& bounds, std::string const& name, Rgba8 const& color, std::string const& eventToFireOnClick = "None", std::string const& eventToFireOnRelease = "None", std::string const& text="", Texture* texture = nullptr);
	~RectButton();

	bool IsPointInside( Vec2 const& point ) const;

	void OnButtonClicked( EventArgs& args );
	void OnCursorHover();
	void OnButtonReleased( EventArgs& args );
	void SetDisable( bool disable );
	void SetVisible( bool visible );
	void ToggleDisable();
	void ToggleVisible();

	void BeginFrame();
	void Render() const;

public:
	std::string m_name;

private:
	Texture* m_texture = nullptr;
	AABB2 m_bounds;
	Rgba8 m_baseColor = Rgba8( 255, 0, 255 );
	Rgba8 m_renderColor = Rgba8( 255, 0, 255 );
	bool m_disable = false;
	bool m_visible = true;
	std::string m_eventToFireOnClick;
	std::string m_eventToFireOnRelease;
	std::string m_text;
};
