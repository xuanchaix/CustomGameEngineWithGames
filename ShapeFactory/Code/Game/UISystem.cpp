#include "Game/UISystem.hpp"
#include "Game/Widget.hpp"

SFUISystem::~SFUISystem()
{
	for (auto widget : m_widgets) {
		delete widget;
	}
}

bool SFUISystem::ExecuteClickEvent( Vec2 const& screenPos )
{
	for (auto widget : m_widgets) {
		if (widget->m_isActive && widget->m_responseArea.IsPointInside( screenPos )) {
			FireEvent( widget->m_eventToFire, widget->m_args );
			return true;
		}
	}
	return false;
}

SFWidget* SFUISystem::AddWidgetToSystem( AABB2 const& responseArea, std::string const& text, std::string const& eventNameToFire )
{
	SFWidget* widget = new SFWidget( responseArea, text, eventNameToFire );
	m_widgets.push_back( widget );
	return widget;
}

void SFUISystem::Render() const
{
	for (auto widget : m_widgets) {
		widget->Render();
	}
}
