#include "Game/Tile.hpp"
std::vector<TileDefinition> TileDefinition::s_definitions;
#define GetSpriteIndexFromCoords(x,y) x + y * 64

TileDefinition const& TileDefinition::GetDef( std::string const& name )
{
	for (auto const& def : s_definitions) {
		if (def.m_tileTypeStr == name) {
			return def;
		}
	}
	ERROR_AND_DIE( Stringf( "Cannot find a tile definition called %s", name.c_str() ) );
}

void TileDefinition::SetUpDefinitions()
{
	s_definitions.resize( 7 );
	s_definitions[0].m_tileType = TileType::Land;
	s_definitions[0].m_tileTypeStr = "Land";
	s_definitions[0].m_isSolid = false;
	s_definitions[0].m_tintColor = Rgba8::WHITE;

	s_definitions[1].m_tileType = TileType::Wall;
	s_definitions[1].m_tileTypeStr = "Wall";
	s_definitions[1].m_isSolid = true;
	s_definitions[1].m_tintColor = Rgba8::WHITE;

	s_definitions[2].m_tileType = TileType::Iron;
	s_definitions[2].m_tileTypeStr = "Iron";
	s_definitions[2].m_isSolid = false;
	s_definitions[2].m_tintColor = Rgba8::WHITE;

	s_definitions[3].m_tileType = TileType::Stone;
	s_definitions[3].m_tileTypeStr = "Stone";
	s_definitions[3].m_isSolid = false;
	s_definitions[3].m_tintColor = Rgba8::WHITE;

	s_definitions[4].m_tileType = TileType::Copper;
	s_definitions[4].m_tileTypeStr = "Copper";
	s_definitions[4].m_isSolid = false;
	s_definitions[4].m_tintColor = Rgba8::WHITE;

	s_definitions[5].m_tileType = TileType::Coal;
	s_definitions[5].m_tileTypeStr = "Coal";
	s_definitions[5].m_isSolid = false;
	s_definitions[5].m_tintColor = Rgba8::WHITE;

	s_definitions[6].m_tileType = TileType::Catalyst;
	s_definitions[6].m_tileTypeStr = "Catalyst";
	s_definitions[6].m_isSolid = false;
	s_definitions[6].m_tintColor = Rgba8::WHITE;
}

Tile::Tile( TileDefinition const& def )
	:m_def(&def)
{

}

TileDefinition const& Tile::GetDef() const
{
	return *m_def;
}

void Tile::SetDef( TileDefinition const& def )
{
	m_def = &def;
}
