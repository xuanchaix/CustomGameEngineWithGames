#pragma once
#include "Game/GameCommon.hpp"

std::wstring GetWStringTranslation( std::string placeHolder, SM_GameLanguage language = SM_GameLanguage::DEFAULT );

void StartUpTranslation( SM_GameLanguage defaultLanguage );

void AddVertsForTextPlaceHolder( std::vector<Vertex_PCU>& verts, AABB2 const& box, float cellHeight,
	std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 0.618f,
	Vec2 const& alignment = Vec2( .5f, .5f ), TextBoxMode mode = TextBoxMode::SHRINK_TO_FIT, int maxGlyphsToDraw = INT_MAX );