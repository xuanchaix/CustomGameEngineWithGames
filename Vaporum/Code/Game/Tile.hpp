#pragma once
#include "Game/GameCommon.hpp"
class Unit;
class Map;

struct TileDefinition {
	TileDefinition( XmlElement* elem );
	char m_symbol = 'X';
	std::string m_name = "Blocked";
	bool m_isBlocked = true;

	static TileDefinition const& GetDefinition( std::string const& name );
	static TileDefinition const& GetDefinition( char symbol );
	static std::vector<TileDefinition> s_definitions;
};

class Tile {
public:
	Tile( IntVec2 const& coords, TileDefinition const& def );
	
	void InitializeNeighbors();

	bool IsPointInsideHexagon( Vec2 const& pos );
	Vec3 GetCenter() const;
	Vec3 GetUnitPosition() const;
	void GetAllNeighbors( std::vector<Tile*>& out_neighbors ) const;
	UnitDirection GetDirectionToTile( Tile* tile ) const;

	TileDefinition const& m_def;
	Vec3 m_center;
	IntVec2 m_coords;
	Unit* m_unitOnTile = nullptr;
	std::vector<Vec2> m_vertexes;
	Tile* m_plusX = nullptr;
	Tile* m_plusY = nullptr;
	Tile* m_minusX = nullptr;
	Tile* m_minusY = nullptr;
	Tile* m_plusZ = nullptr;
	Tile* m_minusZ = nullptr;
	Map* m_map = nullptr;
};
