#pragma once
#include "Game/GameCommon.hpp"

class MapPolygonUnit;

class Religion {
public:
	Religion( int id );
	void GetBoundsPointsForLabel( Vec2& out_startPos, Vec2& out_endPos ) const;

	int m_id = -1;
	float m_influence = 1.f;
	MapPolygonUnit* m_religionOriginUnit = nullptr;

	Rgba8 m_color;
	std::string m_name;
};