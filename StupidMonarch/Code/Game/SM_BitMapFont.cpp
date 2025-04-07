#include "Game/SM_BitMapFont.hpp"

#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

SM_BitmapFont::SM_BitmapFont( char const* fontFilePathName, Texture& fontTexture /*= IntVec2( 1, 1 ) */ )
	:m_fontFilePathNameWithNoExtension( fontFilePathName )
	,m_texture(fontTexture)
{
	if (!strcmp( fontFilePathName, "Data/Fonts/Alvin'sStupidChineseFont.fnt" )) {
		XmlDocument xmlDocument;
		XmlError errorCode = xmlDocument.LoadFile( fontFilePathName );
		GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Loading Xml Document failed!" );
		XmlElement* root = xmlDocument.FirstChildElement();
		XmlElement* xmlIter = root->FirstChildElement();
		//int size = ParseXmlAttribute( *xmlIter, "size", -1 );
		IntVec2 spacing = ParseXmlAttribute( *xmlIter, "spacing", IntVec2( -1, -1 ) );
		xmlIter = xmlIter->NextSiblingElement();
		int scaleW = ParseXmlAttribute( *xmlIter, "scaleW", -1 );
		int scaleH = ParseXmlAttribute( *xmlIter, "scaleH", -1 );
		xmlIter = xmlIter->NextSiblingElement()->NextSiblingElement();
		xmlIter = xmlIter->FirstChildElement();
		m_bitmapFontCharUVMapping.resize( 100000 );
		while (xmlIter != nullptr) {
			int id = ParseXmlAttribute( *xmlIter, "id", -1 );
			int x = ParseXmlAttribute( *xmlIter, "x", -1 );
			int y = ParseXmlAttribute( *xmlIter, "y", -1 );
			int width = ParseXmlAttribute( *xmlIter, "width", -1 );
			int height = ParseXmlAttribute( *xmlIter, "height", -1 );
			m_bitmapFontCharUVMapping[id] = AABB2( Vec2( (float)x / scaleW, 1.f - (float)(y + height) / scaleH ), Vec2( (float)(x + width) / scaleW, 1.f - (float)y / scaleH ) );
			xmlIter = xmlIter->NextSiblingElement();
		}
	}
}

Texture& SM_BitmapFont::GetTexture() const
{
	return m_texture;
}

void SM_BitmapFont::AddVertsForText2D( std::vector<Vertex_PCU>& verts, Vec2 const& textMins, float cellHeight, std::wstring const& text, Rgba8 const& tint /*= Rgba8::WHITE*/, float cellAspect /*= 0.618f */ ) const
{
	Vec2 cellMins = textMins;

	for (int i = 0; i < (int)text.length(); i++)
	{
		wchar_t glyph = text[i];
		// need to consider cell aspect
		float cellWidth = cellHeight * cellAspect * GetGlyphAspect( (int)glyph );
		Vec2 cellSize( cellWidth, cellHeight );

		if (glyph == L'\n') {
			cellMins.x = textMins.x;
			cellMins.y -= cellHeight;
		}
		else {
			//if ((g_gameLanguage == SM_GameLanguage::ZH || g_gameLanguage == SM_GameLanguage::ZH_CN)
			//	&& (int)glyph >= 33 && (int)glyph <= 126) {
			//	acutalSize.x = acutalSize.x * 0.618f;
			//}
			//GUARANTEE_OR_DIE( pos >= 0 && pos <= 255, "Cannot parse string!" );
			AddVertsForAABB2D( verts, AABB2( cellMins, cellMins + cellSize ), tint, GetWcharSpriteUVs( glyph ) );
			cellMins.x += cellSize.x;
		}
	}
}

void SM_BitmapFont::AddVertsForTextInBox2D( std::vector<Vertex_PCU>& verts, AABB2 const& box, float cellHeight, std::wstring const& text, Rgba8 const& tint /*= Rgba8::WHITE*/, float cellAspect /*= 0.618f*/, Vec2 const& alignment /*= Vec2( .5f, .5f )*/, TextBoxMode mode /*= TextBoxMode::SHRINK_TO_FIT*/, int maxGlyphsToDraw /*= INT_MAX */ ) const
{
	// split string by \n
	WStrings strings;
	int numOfLines;
	float boxWidth = box.m_maxs.x - box.m_mins.x;
	if (mode == TextBoxMode::AUTO_NEW_LINE) {
		std::wstring newStr = text;
		//float actualWidth = boxWidth * (1.f - alignment.x * 2.f);
		//GUARANTEE_OR_DIE( actualWidth > 0.f, Stringf( "AddVertsForTextInBox2D: AUTO_NEW_LINE mode error! Alignment.x is too big" ) );
		float widthSum = 0.f;
		for (int i = 0; i < (int)text.size(); i++) {
			if (text[i] == L'\n') {
				widthSum = 0.f;
				continue;
			}
			widthSum += GetGlyphAspect( text[i] ) * cellHeight * cellAspect;
			if (widthSum > boxWidth) {
				newStr = newStr.substr( 0, i ) + std::wstring(L"\n") + newStr.substr( i );
				widthSum = GetGlyphAspect( text[i] ) * cellHeight * cellAspect;
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
		for (std::wstring const& str : strings) {
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
		std::wstring const& str = strings[i];
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
			AddVertsForText2D( verts, Vec2( box.m_mins.x + xOffset, box.m_maxs.y - yOffset - cellHeight * scalingFactor * (i + 1) ), cellHeight * scalingFactor, str.substr( 0, (size_t)maxGlyphsToDraw - drawGlyphsCount ), tint, cellAspect );
			return;
		}
	}
}

float SM_BitmapFont::GetTextWidth( float cellHeight, std::wstring const& text, float cellAspect /*= 1.f */ ) const
{
	float sum = 0.f;
	for (int i = 0; i < (int)text.size(); i++) {
		sum += GetGlyphAspect( (int)text[i] );
	}
	return sum * cellHeight * cellAspect;
}

float SM_BitmapFont::GetGlyphAspect( int glyphUnicode ) const
{
	// now is only 1.f for each glyph			
	if ((g_gameLanguage == SM_GameLanguage::ZH || g_gameLanguage == SM_GameLanguage::ZH_CN) 
		&& (int)glyphUnicode >= 33 && (int)glyphUnicode <= 126) {
		return 0.6f;
	}
	return 1.f;
}

AABB2 SM_BitmapFont::GetWcharSpriteUVs( wchar_t character ) const
{	// 33 65281 126
	int c = (int)character;
	/*if (g_gameLanguage != SM_GameLanguage::ENGLISH) {
		if (c >= 33 && c <= 126) {
			c += 65248;
		}
		if (c == 32) {
			c = 12288;
		}
	}*/
	if (g_gameLanguage != SM_GameLanguage::ENGLISH) {
		if (c == 58) {
			c += 65248;
		}
	}
	return m_bitmapFontCharUVMapping[c];
}
