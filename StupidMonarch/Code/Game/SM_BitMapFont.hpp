#pragma once
#include "Game/GameCommon.hpp"

class SM_BitmapFont {
public:
	SM_BitmapFont( char const* fontFilePathNameWithNoExtension, Texture& fontTexture );

	Texture& GetTexture() const;

	void AddVertsForText2D( std::vector<Vertex_PCU>& verts, Vec2 const& textMins,
		float cellHeight, std::wstring const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 0.618f ) const;

	void AddVertsForTextInBox2D(std::vector<Vertex_PCU>& verts, AABB2 const& box, float cellHeight,
		std::wstring const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 0.618f,
		Vec2 const& alignment = Vec2( .5f, .5f ), TextBoxMode mode = TextBoxMode::SHRINK_TO_FIT, int maxGlyphsToDraw = INT_MAX ) const;
	
	float GetTextWidth( float cellHeight, std::wstring const& text, float cellAspect = 1.f ) const;
	/*
	/// No Change Lines
	void AddVertsForText3DAtOriginXForward( std::vector<Vertex_PCU>& verts, float cellHeight, std::string const& text,
		Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f, Vec2 const& alignment = Vec2( 0.5f, 0.5f ), int maxGlyphsToDraw = 999999999 );*/
protected:
	float GetGlyphAspect( int glyphUnicode ) const; // For now this will always return 1.0f!!!
	AABB2 GetWcharSpriteUVs( wchar_t character ) const;

protected:
	std::string	m_fontFilePathNameWithNoExtension;
	Texture& m_texture;
	std::vector<AABB2> m_bitmapFontCharUVMapping;
};