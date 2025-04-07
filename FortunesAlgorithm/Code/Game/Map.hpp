#pragma once
#include "Game/GameCommon.hpp"
#include "Game/MapPolygonUnit.hpp"

class Map {
public:
	Map();
	~Map();

	void Startup();
	void Update();
	void Render() const;

	void PopulateMapWithPolygons();
	std::vector<MapPolygonUnit> m_mapPolygonUnits;
};