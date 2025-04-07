#pragma once
#include "Game/GameCommon.hpp"

class City;
class Road;

class ProvinceRoadData {
public:
	std::vector<Road*> m_roadsPassing;
	std::vector<ProvinceRoadData*> m_neighbors;
	Vec2 m_position;
	void Connect( ProvinceRoadData* other );
	bool IsConnectedTo( ProvinceRoadData* other ) const;
};

class Road {
public:
	Road(City* city1, City* city2);
	~Road();

	void Initialize();

	City* m_city1 = nullptr;
	City* m_city2 = nullptr;
	std::vector<std::pair<Vec2, bool>> m_anchorPoints; // pos, is town\city
	std::vector<CatmullRomSpline2D> m_roadCurve;
	bool m_isGarbage = false;
};