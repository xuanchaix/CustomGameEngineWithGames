#include "Game/Tile.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
std::vector<TileDefinition> const TileDefinition::s_definitions = SetUpTileTypes();

Tile::Tile( TileDefinition const& type, IntVec2 const& coords ):
	m_tileDefinition(&type),
	m_coords(coords)
{
	m_health = m_tileDefinition->m_maxHealth;
}

Tile::Tile()
{

}

void Tile::ChangeType( TileDefinition const& type )
{
	m_tileDefinition = &type;
	m_health = m_tileDefinition->m_maxHealth;
}


TileDefinition::TileDefinition( std::string const& tileType, int spriteSheetUVIndex, bool isSolid, bool isWater, bool isDestructible, std::string typeAfterDestruction, float maxHealth, Rgba8 const& tintColor /*= Rgba8( 255, 255, 255, 255 ) */, Rgba8 const& mapImageColor )
	:m_tileType(tileType)
	,m_tintColor(tintColor)
	,m_isSolid(isSolid)
	,m_isWater(isWater)
	,m_spriteSheetUVIndex(spriteSheetUVIndex)
	,m_mapImageColor(mapImageColor)
	,m_isDestructible(isDestructible)
	,m_tileTypeAfterDestruction(typeAfterDestruction)
	,m_maxHealth(maxHealth)
{
}

TileDefinition::TileDefinition( XmlElement* xmlIter )
{
	m_tileType = ParseXmlAttribute( *xmlIter, "type", "Default" );
	m_tintColor = ParseXmlAttribute( *xmlIter, "tintColor", Rgba8( 255, 255, 255, 255 ) );
	m_isSolid = ParseXmlAttribute( *xmlIter, "isSolid", false );
	m_isWater = ParseXmlAttribute( *xmlIter, "isWater", false );
	IntVec2 coordsInTexture = ParseXmlAttribute( *xmlIter, "coordsInTexture", IntVec2( -1, -1 ) );
	GUARANTEE_OR_DIE( coordsInTexture.x >= 0 && coordsInTexture.x <= TERRAIN_TEXTURE_SPRITE_X - 1
		&& coordsInTexture.y >= 0 && coordsInTexture.y <= TERRAIN_TEXTURE_SPRITE_Y - 1,
		Stringf( "Syntax Error! Wrong texture coordinate uv in type %s ", m_tileType.c_str() ) );
	m_spriteSheetUVIndex = coordsInTexture.x + coordsInTexture.y * TERRAIN_TEXTURE_SPRITE_X;
	m_mapImageColor = ParseXmlAttribute( *xmlIter, "mapImageColor", Rgba8( 0, 0, 0, 0 ) );
	m_isDestructible = ParseXmlAttribute( *xmlIter, "isDestructible", false );
	m_tileTypeAfterDestruction = ParseXmlAttribute( *xmlIter, "typeAfterDestruction", "Default" );
	m_maxHealth = ParseXmlAttribute( *xmlIter, "maxHealth", 0.f );
}

std::vector<TileDefinition> TileDefinition::SetUpTileTypes()
{
	std::vector<TileDefinition> retDefs;
	retDefs.reserve( 64 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Definitions/TileDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document TileDefinitions.xml error" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "TileDefinitions" ), "Syntax Error! Name of the root of TileDefinitions.xml should be \"TileDefinitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "TileDefinition" ), "Syntax Error! Names of the elements of TileDefinitions.xml should be \"TileDefinition\" " );
		retDefs.emplace_back( xmlIter );
		xmlIter = xmlIter->NextSiblingElement();
	}
	return retDefs;
}
