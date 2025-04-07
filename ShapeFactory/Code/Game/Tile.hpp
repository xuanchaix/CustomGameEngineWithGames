#pragma once
#include "Game/GameCommon.hpp"

enum class TileType {
	Land, Wall, Iron, Stone, Copper, Coal, Catalyst, Num, None
};

class TileDefinition {
public:
	TileType m_tileType = TileType::None;
	std::string m_tileTypeStr;
	Rgba8 m_tintColor = Rgba8( 255, 255, 255, 255 );
	AABB2 m_uv = AABB2::IDENTITY;
	bool m_isSolid = false;
	static TileDefinition const& GetDef( std::string const& name );
	static void SetUpDefinitions();
	static std::vector<TileDefinition> s_definitions;
};

struct Tile {
	Tile( TileDefinition const& def );
	TileDefinition const& GetDef() const;
	void SetDef( TileDefinition const& def );
protected:
	TileDefinition const* m_def;

};