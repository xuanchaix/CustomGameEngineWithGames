#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include <vector>
#include <string>


constexpr int TERRAIN_TEXTURE_SPRITE_X = 8;
constexpr int TERRAIN_TEXTURE_SPRITE_Y = 8;

class TileDefinition {
public:
	TileDefinition( XmlElement* xmlIter );
	TileDefinition( std::string const& tileType, int spriteSheetUVIndex, bool isSolid, bool isWater, bool isDestructible, std::string typeAfterDestruction, float maxHealth, Rgba8 const& tintColor = Rgba8( 255, 0, 255, 255 ), Rgba8 const& mapImageColor = Rgba8(0, 0, 0, 0) );
	std::string m_tileType;
	Rgba8 m_tintColor = Rgba8( 255, 0, 255, 255 );
	int m_spriteSheetUVIndex = -1;
	bool m_isSolid = false;
	bool m_isWater = false;
	bool m_isDestructible = false;
	std::string m_tileTypeAfterDestruction;
	float m_maxHealth;
	Rgba8 m_mapImageColor = Rgba8( 0, 0, 0, 0 );
	static std::vector<TileDefinition> SetUpTileTypes();
	static std::vector<TileDefinition> const s_definitions;
};

class Tile {
public:
	Tile();
	Tile( TileDefinition const& def, IntVec2 const& coords );
	void ChangeType( TileDefinition const& def );

public:
	TileDefinition const* m_tileDefinition;
	IntVec2 m_coords = IntVec2( -1, -1 );
	float m_health = 0.f;
};

