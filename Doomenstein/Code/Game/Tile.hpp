#pragma once
#include "Game/GameCommon.hpp"

class TileDefinition {
public:
	TileDefinition( XmlElement* xmlIter );
	std::string m_tileType;
	Rgba8 m_tintColor = Rgba8( 255, 255, 255 );
	IntVec2 m_coordsInTextureForWall;
	IntVec2 m_coordsInTextureForFloor;
	IntVec2 m_coordsInTextureForCeiling;
	bool m_isSolid = false;
	Rgba8 m_mapImageColor = Rgba8( 0, 0, 0, 0 );
	static std::vector<TileDefinition> SetUpTileTypes();
	static std::vector<TileDefinition> const s_definitions;
	static TileDefinition const& GetTileDefinition( std::string name );
	static TileDefinition const& GetTileDefinition( Rgba8 const& mapImageColor );
};

class Tile {
public:
	Tile();
	Tile( TileDefinition const& def, IntVec2 const& coords );
	void ChangeType( TileDefinition const& def );
public:
	TileDefinition const* m_tileDefinition;
	IntVec2 m_coords = IntVec2( -1, -1 );
};