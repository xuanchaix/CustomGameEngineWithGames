#pragma once
#include "Game/GameCommon.hpp"

class Region {
public:
	Region();
	~Region();

	void GainProvince( MapPolygonUnit* prov );
	void LoseProvince( MapPolygonUnit* prov );
	void GetBoundsPointsForLabel( Vec2& out_startPos, Vec2& out_endPos ) const;

	std::vector<MapPolygonUnit*> m_containedUnits;
	Rgba8 m_color = Rgba8( 255, 255, 0 );
	VertexBuffer* m_edgeShowingBuffer = nullptr;
	std::string m_name = "default";

	int m_totalPopulation = 0;
	int m_id = 0;
};