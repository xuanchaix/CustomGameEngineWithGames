#pragma once
#include "Game/GameCommon.hpp"

class MapPolygonUnit;

class River {
public:
	River();
	virtual ~River();

	void CalculateRiverRoute();
	void CalculateRiverStats();
	MapPolygonUnit* GetPrevUnitOnRoute( MapPolygonUnit* refUnit );
	float GetDistanceToRiver( Vec2 const& refPos ) const;
	float GetRiverStartEndDistSquared() const;

	void AddVertsForRiver( std::vector<Vertex_PCU>& verts, float thickness, Rgba8 const& color, int numSubdivisions );

	MapPolygonUnit* m_startUnit = nullptr;
	MapPolygonUnit* m_endUnit = nullptr;
	River* m_riverConvergeTo = nullptr;
	std::vector<MapPolygonUnit*> m_routeUnits;
	std::vector<Vec2> m_anchorPoint;
	std::vector<River*> m_branches;
	CatmullRomSpline2D m_riverCurve;
	bool m_isGarbage = false;

	float m_quantityOfFlow = 0.f;
	float m_length = 0.f;
	bool m_hasDelta = false;
	float m_sandiness = 0.f;
protected:
	int TestAdjKindOfLowerUnits( MapPolygonUnit* curUnit, MapPolygonUnit** out_resUnit );
};