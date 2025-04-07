#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/RendererUtils.hpp"

#include <vector>
#include <string>
class Texture;
struct Vec2;
struct Vertex_PCU;
class CubicBezierCurve2D;
class Renderer;

enum class TextBoxMode {
	SHRINK_TO_FIT,
	OVERRUN,
	AUTO_NEW_LINE,
};

struct BitmapGlyph {
	AABB2 m_uv;
	float m_widthAspect = 1.f;
	float m_heightAspect = 1.f;
	float m_xOffset = 0.f;
	float m_xAdvance = 1.f;
	float m_yOffset = 0.f;
};

class BitmapFont {
	friend class DX11Renderer; // Only the Renderer can create new BitmapFont objects!
	friend class DX12Renderer;
private:
	BitmapFont( char const* fontFilePathNameWithNoExtension, Texture& fontTexture, BitmapFontType type = BitmapFontType::PNGTextureType, Renderer* renderer = nullptr );

public:
	Texture& GetTexture() const;

	void AddVertsForText2D( std::vector<Vertex_PCU>& verts, Vec2 const& textMins,
		float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 0.618f ) const;

	void AddVertsForTextInBox2D( std::vector<Vertex_PCU>& verts, AABB2 const& box, float cellHeight,
		std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 0.618f,
		Vec2 const& alignment = Vec2( .5f, .5f ), TextBoxMode mode = TextBoxMode::SHRINK_TO_FIT, int maxGlyphsToDraw = INT_MAX ) const;

	void AddVertsForCurveText2D( std::vector<Vertex_PCU>& verts, CubicBezierCurve2D const& curve,
		float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 0.618f ) const;

	float GetTextWidth( float cellHeight, std::string const& text, float cellAspect = 1.f ) const;

	/// No Change Lines
	void AddVertsForText3DAtOriginXForward( std::vector<Vertex_PCU>& verts, float cellHeight, std::string const& text, 
		Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f, Vec2 const& alignment = Vec2( 0.5f, 0.5f ), int maxGlyphsToDraw = 999999999 );
protected:
	float GetGlyphAspect( int glyphUnicode ) const;
	float GetGlyphAspectWithSpacing( int glyphUnicode ) const;

	AABB2 GetUVAtASCII( unsigned char asciiCode ) const;

protected:
	std::string	m_fontFilePathNameWithNoExtension;
	SpriteSheet	m_fontGlyphsSpriteSheet;
	std::vector<BitmapGlyph> m_glyphs;
	float m_fontBase = 0.f;
	float m_fontSize = 0.f;
	BitmapFontType m_type = BitmapFontType::PNGTextureType;
	Texture* m_texture = nullptr;
};