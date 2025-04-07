#pragma once
#include "Game/GameCommon.hpp"

class MapPolygonUnit;

struct StarEdge {
	Vec2 m_startPos;
	Vec2 m_endPos;
	StarEdge* m_opposite = nullptr;
	StarEdge* m_prev = nullptr;
	StarEdge* m_next = nullptr;
	MapPolygonUnit* m_owner = nullptr;

};

class MapPolygonUnit {
public:
	MapPolygonUnit();
	~MapPolygonUnit();

	std::vector<StarEdge*> m_edges;
	Vec2 m_centerPosition;
};