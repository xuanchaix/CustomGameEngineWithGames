#pragma once
#include "Game/GameCommon.hpp"

enum class SFWidgetType {
	Text, Graph,
};

class SFWidget {
public:
	SFWidget( AABB2 const& responseArea, std::string const& text, std::string const& eventNameToFire );

	void Execute( Vec2 const& cursorPos );
	void SetCenter( Vec2 newCenter );

	void Render() const;
public:
	SFWidgetType m_type = SFWidgetType::Text;
	EventArgs m_args;
	AABB2 m_responseArea;
	Texture* m_texture = nullptr;
	AABB2 m_uv;
	std::string m_text;
	std::string m_eventToFire;
	bool m_isActive = false;
	float m_fontSize = 15.f;
};