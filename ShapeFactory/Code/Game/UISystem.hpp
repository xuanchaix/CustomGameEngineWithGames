#pragma once
#include "Game/GameCommon.hpp"

class SFWidget;

class SFUISystem {
public:
	~SFUISystem();
	bool ExecuteClickEvent( Vec2 const& screenPos );
	SFWidget* AddWidgetToSystem( AABB2 const& responseArea, std::string const& text, std::string const& eventNameToFire );
	void Render() const;

public:
	std::vector<SFWidget*> m_widgets;
};