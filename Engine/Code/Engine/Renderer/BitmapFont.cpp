#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Curves.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Renderer/Renderer.hpp"

BitmapFont::BitmapFont( char const* fontFilePathNameWithNoExtension, Texture& fontTexture, BitmapFontType type, Renderer* renderer )
	:m_fontFilePathNameWithNoExtension( fontFilePathNameWithNoExtension )
	,m_fontGlyphsSpriteSheet( fontTexture, IntVec2( 16, 16 ) ) 
	,m_type(type)
{
	m_glyphs.resize( 256 );
	if (type == BitmapFontType::FntType) {
		XmlDocument xmlDocument;
		XmlError errorCode = xmlDocument.LoadFile( fontFilePathNameWithNoExtension );
		GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Loading Xml Document failed!" );
		XmlElement* root = xmlDocument.FirstChildElement();
		XmlElement* xmlIter = root->FirstChildElement();
		int size = ParseXmlAttribute( *xmlIter, "size", -1 );
		//IntVec2 spacing = ParseXmlAttribute( *xmlIter, "spacing", IntVec2( -1, -1 ) );
		xmlIter = xmlIter->NextSiblingElement();
		int base = ParseXmlAttribute( *xmlIter, "base", -1 );
		m_fontSize = (float)size;
		m_fontBase = (float)(size - base) / (float)size;
		int scaleW = ParseXmlAttribute( *xmlIter, "scaleW", -1 );
		int scaleH = ParseXmlAttribute( *xmlIter, "scaleH", -1 );
		xmlIter = xmlIter->NextSiblingElement();
		std::string texturePath = ParseXmlAttribute( *xmlIter->FirstChildElement(), "file", "None" );
		std::string fntPath = std::string( fontFilePathNameWithNoExtension );
		fntPath = fntPath.substr( 0, fntPath.find_last_of( '/' ) + 1 );
		m_texture = renderer->CreateOrGetTextureFromFile( (fntPath + texturePath).c_str() );
		xmlIter = xmlIter->NextSiblingElement();
		xmlIter = xmlIter->FirstChildElement();
		while (xmlIter != nullptr) {
			int id = ParseXmlAttribute( *xmlIter, "id", -1 );
			int x = ParseXmlAttribute( *xmlIter, "x", -1 );
			int y = ParseXmlAttribute( *xmlIter, "y", -1 );
			int width = ParseXmlAttribute( *xmlIter, "width", -1 );
			int height = ParseXmlAttribute( *xmlIter, "height", -1 );
			float yOffset = (float)ParseXmlAttribute( *xmlIter, "yoffset", -1 );
			float xOffset = (float)ParseXmlAttribute( *xmlIter, "xoffset", -1 );
			float xAdvance = (float)ParseXmlAttribute( *xmlIter, "xadvance", -1 );
			m_glyphs[id].m_widthAspect = (float)width / (float)size;
			m_glyphs[id].m_yOffset = (float)yOffset / (float)size;
			m_glyphs[id].m_heightAspect = (float)height / (float)size;
			m_glyphs[id].m_xOffset = (float)xOffset / (float)size;
			m_glyphs[id].m_xAdvance = (float)xAdvance / (float)size;
			m_glyphs[id].m_uv = AABB2( Vec2( (float)x / scaleW, 1.f - (float)(y + height) / scaleH ), Vec2( (float)(x + width) / scaleW, 1.f - (float)y / scaleH ) );
			xmlIter = xmlIter->NextSiblingElement();
		}
	}
}

Texture& BitmapFont::GetTexture() const
{
	if (m_type == BitmapFontType::PNGTextureType) {
		return m_fontGlyphsSpriteSheet.GetTexture();
	}
	else if (m_type == BitmapFontType::FntType) {
		return *m_texture;
	}
	return *(Texture*)nullptr;
}

void BitmapFont::AddVertsForText2D( std::vector<Vertex_PCU>& verts, Vec2 const& textMins, float cellHeight, std::string const& text, 
	Rgba8 const& tint /*= Rgba8::WHITE*/, float cellAspect /*= 1.f */ ) const
{
	Vec2 cellMins = textMins;
	for (int i = 0; i < (int)text.length(); i++)
	{
		unsigned char glyph = text[i];
		// need to consider cell aspect
		float cellWidth = 0.f;

		cellWidth = cellHeight * cellAspect * GetGlyphAspect( (int)glyph );
		
		if (glyph == '\n') {
			cellMins.x = textMins.x;
			cellMins.y -= cellHeight;
		}
		else {
			//GUARANTEE_OR_DIE( pos >= 0 && pos <= 255, "Cannot parse string!" );
			float maxHeight = 1.f - m_glyphs[glyph].m_yOffset;
			AddVertsForAABB2D( verts, AABB2( Vec2( m_glyphs[glyph].m_xOffset * cellHeight * cellAspect + cellMins.x, (maxHeight - m_glyphs[glyph].m_heightAspect) * cellHeight + cellMins.y ), Vec2( m_glyphs[glyph].m_xOffset * cellHeight * cellAspect + cellMins.x + cellWidth, maxHeight * cellHeight + cellMins.y ) ), tint, GetUVAtASCII( glyph ) );
			cellMins.x +=  (m_glyphs[glyph].m_xOffset + m_glyphs[glyph].m_xAdvance) * cellHeight * cellAspect;
		}
	}
}

void BitmapFont::AddVertsForTextInBox2D( std::vector<Vertex_PCU>& verts, AABB2 const& box, float cellHeight, std::string const& text, 
	Rgba8 const& tint /*= Rgba8::WHITE*/, float cellAspect /*= 1.f*/, Vec2 const& alignment /*= Vec2( .5f, .5f )*/, 
	TextBoxMode mode /*= TextBoxMode::SHRINK_TO_FIT*/, int maxGlyphsToDraw /*= INT_MAX */ ) const
{
	// split string by \n
	Strings strings;
	int numOfLines;
	float boxWidth = box.m_maxs.x - box.m_mins.x;
	if (mode == TextBoxMode::AUTO_NEW_LINE) {
		std::string newStr = text;
		float widthSum = 0.f;
		for (int i = 0; i < (int)text.size(); i++) {
			if (text[i] == '\n') {
				widthSum = 0.f;
				continue;
			}
			widthSum += GetGlyphAspectWithSpacing( text[i] ) * cellHeight * cellAspect;
			if (widthSum > boxWidth) {
				newStr = newStr.substr( 0, i ) + "\n" + newStr.substr( i );
				widthSum = GetGlyphAspectWithSpacing( text[i] ) * cellHeight * cellAspect;
			}
		}
		numOfLines = SplitStringOnDelimiter( strings, newStr, '\n' );
	}
	else {
		numOfLines = SplitStringOnDelimiter( strings, text, '\n' );
	}
	// calculate text box height and width
	float boxHeight = box.m_maxs.y - box.m_mins.y;
	float allTextHeight = cellHeight * numOfLines;
	float scalingFactor = 1.f;
	if (mode == TextBoxMode::SHRINK_TO_FIT) {
		float allTextMaxWidth = -1;
		// find max width of all lines of string
		for (std::string const& str : strings) {
			float textWidth = GetTextWidth( cellHeight, str, cellAspect );
			if (textWidth > allTextMaxWidth) {
				allTextMaxWidth = textWidth;
			}
		}
		// find the smallest scaling factor
		float heightScalingFactor = 1.f;
		if (allTextHeight > boxHeight) {
			heightScalingFactor = boxHeight / allTextHeight;
		}
		float widthScalingFactor = 1.f;
		if (allTextMaxWidth > boxWidth) {
			widthScalingFactor = boxWidth / allTextMaxWidth;
		}
		scalingFactor = Minf( heightScalingFactor, widthScalingFactor );
	}

	int drawGlyphsCount = 0;
	for (int i = 0; i < (int)strings.size(); i++) {
		std::string const& str = strings[i];
		float stringWidth = GetTextWidth( cellHeight, str, cellAspect ) * scalingFactor;
		// xOffset: offset from left
		float remainSpaceX = boxWidth - stringWidth;
		float xOffset = remainSpaceX * alignment.x;
		// yOffset: offset from top because text is start from top (but AABB starts from bottom)
		float remainSpaceY = boxHeight - allTextHeight * scalingFactor;
		float yOffset = remainSpaceY * (1 - alignment.y);
		// check if reach max glyph to draw
		if (drawGlyphsCount + (int)str.size() <= maxGlyphsToDraw) {
			AddVertsForText2D( verts, Vec2( box.m_mins.x + xOffset, box.m_maxs.y - yOffset - cellHeight * scalingFactor * (i + 1) ), cellHeight * scalingFactor, str, tint, cellAspect );
			drawGlyphsCount += (int)str.size();
		}
		else {
			AddVertsForText2D( verts, Vec2( box.m_mins.x + xOffset, box.m_maxs.y - yOffset - cellHeight * scalingFactor * (i + 1) ), cellHeight * scalingFactor, str.substr(0, (size_t)maxGlyphsToDraw - drawGlyphsCount), tint, cellAspect );
			return;
		}
	}
}

void BitmapFont::AddVertsForCurveText2D( std::vector<Vertex_PCU>& verts, CubicBezierCurve2D const& curve, float cellHeight, std::string const& text, Rgba8 const& tint /*= Rgba8::WHITE*/, float cellAspect /*= 0.618f */ ) const
{
	if ((int)text.size() == 0) {
		return;
	}
	else if ((int)text.size() == 1) {
		AddVertsForOBB2D( verts, OBB2( (curve.m_startPos + curve.m_endPos) * 0.5f, (curve.m_endPos - curve.m_startPos).GetNormalized(), Vec2( cellHeight * cellAspect * 0.5f, cellHeight * 0.5f ) ), tint, m_fontGlyphsSpriteSheet.GetSpriteUVs( text[0] ) );
		return;
	}
	// no new lines
	float distance = curve.GetApproximateLength();
	for (int i = 0; i < (int)text.size(); ++i) {
		Vec2 prevCenterPos = curve.EvaluateAtApproximateDistance( (float)i / (float)text.size() * distance );
		Vec2 centerPos = curve.EvaluateAtApproximateDistance( (float)(i + 0.1f) / (float)text.size() * distance );
		AddVertsForOBB2D( verts, OBB2( centerPos, (centerPos - prevCenterPos).GetNormalized(), Vec2( cellHeight * cellAspect * 0.5f, cellHeight * 0.5f ) ), tint, m_fontGlyphsSpriteSheet.GetSpriteUVs( text[i] ) );
	}
}

float BitmapFont::GetTextWidth( float cellHeight, std::string const& text, float cellAspect /*= 1.f */ ) const
{
	float sum = 0.f;
	for (int i = 0; i < (int)text.size(); i++) {
		sum += GetGlyphAspectWithSpacing( (int)text[i] );
	}
	return sum * cellHeight * cellAspect;
}

void BitmapFont::AddVertsForText3DAtOriginXForward( std::vector<Vertex_PCU>& verts, float cellHeight, std::string const& text, Rgba8 const& tint /*= Rgba8::WHITE*/, float cellAspect /*= 1.f*/, Vec2 const& alignment /*= Vec2( 0.5f, 0.5f )*/, int maxGlyphsToDraw /*= 999999999 */ )
{
	//UNUSED( alignment );
	float textWidth = GetTextWidth( cellHeight, text, cellAspect );
	float currentY = -textWidth * alignment.x;
	float bottomZ = -cellHeight * alignment.y;
	float topZ = cellHeight * 0.5f;
	float glyphWidth = cellAspect * cellHeight;

	int numOfGlyph = maxGlyphsToDraw > (int)text.length() ? (int)text.length() : maxGlyphsToDraw;
	for (int i = 0; i < numOfGlyph; i++)
	{
		unsigned char glyph = text[i];
		if (glyph == '\n') {
			// do nothing
		}
		else {
			int pos = (int)glyph;
			//GUARANTEE_OR_DIE( pos >= 0 && pos <= 255, "Cannot parse string!" );
			AddVertsForQuad3D( verts, Vec3( 0.f, currentY, bottomZ ), Vec3( 0.f, currentY + glyphWidth * GetGlyphAspect( pos ), bottomZ ), Vec3( 0.f, currentY + glyphWidth * GetGlyphAspect( pos ), topZ ), Vec3( 0.f, currentY, topZ ), tint, m_fontGlyphsSpriteSheet.GetSpriteUVs( pos ) );
			currentY += glyphWidth * GetGlyphAspect( pos );
		}
	}
}

float BitmapFont::GetGlyphAspect( int glyphUnicode ) const
{
	return m_glyphs[glyphUnicode].m_widthAspect;
}

float BitmapFont::GetGlyphAspectWithSpacing( int glyphUnicode ) const
{
	return m_glyphs[glyphUnicode].m_xOffset + m_glyphs[glyphUnicode].m_xAdvance;
}

AABB2 BitmapFont::GetUVAtASCII( unsigned char asciiCode ) const
{
	if (m_type == BitmapFontType::PNGTextureType) {
		return m_fontGlyphsSpriteSheet.GetSpriteUVs( (int)asciiCode );
	}
	else if (m_type == BitmapFontType::FntType) {
		return m_glyphs[asciiCode].m_uv;
	}
	return AABB2();
}
