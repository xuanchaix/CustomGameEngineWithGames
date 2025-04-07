#include "Game/Tile.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
std::vector<TileDefinition> const TileDefinition::s_definitions = SetUpTileTypes();

TileDefinition const& TileDefinition::GetTileDefinition( std::string name )
{
	for (auto const& def : s_definitions) {
		if (def.m_tileType == name) {
			return def;
		}
	}
	ERROR_AND_DIE( Stringf( "Cannot find Tile called %s!", name.c_str() ) );
}

TileDefinition const& TileDefinition::GetTileDefinition( Rgba8 const& mapImageColor )
{
	for (auto const& def : s_definitions) {
		if (def.m_mapImageColor == mapImageColor) {
			return def;
		}
	}
	ERROR_AND_DIE( Stringf( "Cannot find Tile in color RGBA %d %d %d %d!", mapImageColor.r, mapImageColor.g, mapImageColor.b, mapImageColor.a ) );
}

Tile::Tile( TileDefinition const& type, IntVec2 const& coords ) :
	m_tileDefinition( &type ),
	m_coords( coords )
{
}

Tile::Tile()
{

}

void Tile::ChangeType( TileDefinition const& type )
{
	m_tileDefinition = &type;
}

TileDefinition::TileDefinition( XmlElement* xmlIter )
{
	m_tileType = ParseXmlAttribute( *xmlIter, "name", "Default" );
	m_tintColor = ParseXmlAttribute( *xmlIter, "tintColor", Rgba8( 255, 255, 255, 255 ) );
	m_isSolid = ParseXmlAttribute( *xmlIter, "isSolid", false );
	m_coordsInTextureForWall = ParseXmlAttribute( *xmlIter, "wallSpriteCoords", IntVec2( -1, -1 ) );
	m_coordsInTextureForFloor = ParseXmlAttribute( *xmlIter, "floorSpriteCoords", IntVec2( -1, -1 ) );
	m_coordsInTextureForCeiling = ParseXmlAttribute( *xmlIter, "ceilingSpriteCoords", IntVec2( -1, -1 ) );
	m_mapImageColor = ParseXmlAttribute( *xmlIter, "mapImagePixelColor", Rgba8( 255, 255, 255 ) );
}

std::vector<TileDefinition> TileDefinition::SetUpTileTypes()
{
	std::vector<TileDefinition> retDefs;
	retDefs.reserve( 64 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Definitions/TileDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document TileDefinitions.xml error" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "Definitions" ), "Syntax Error! Name of the root of TileDefinitions.xml should be \"Definitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "TileDefinition" ), "Syntax Error! Names of the elements of TileDefinitions.xml should be \"TileDefinition\" " );
		retDefs.emplace_back( xmlIter );
		xmlIter = xmlIter->NextSiblingElement();
	}
	return retDefs;
}
