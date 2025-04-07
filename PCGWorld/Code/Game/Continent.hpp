#pragma once

#include "Game/GameCommon.hpp"

class MapPolygonUnit;

class Continent {
public:
	Continent();
	~Continent() { };

	void GetBoundsPointsForLabel( Vec2& out_startPos, Vec2& out_endPos ) const;

	std::vector<MapPolygonUnit*> m_containedUnits;
	Rgba8 m_color = Rgba8( 255, 255, 0 );
	bool m_goldInDesertFlag = false;
	bool m_newContinent = false;
	bool m_eastContinent = false;
	int m_id = 0;
	std::string m_name;
};