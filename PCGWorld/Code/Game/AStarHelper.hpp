#pragma once
#include "Game/GameCommon.hpp"
#include <queue>

class MapPolygonUnit;

struct AStarNode {
	AStarNode( MapPolygonUnit* base ) :m_base( base ) {};
	
	float GetDistCostByHeight( AStarNode* parent ) const;

	MapPolygonUnit const* m_base = nullptr;
	std::vector<std::pair<AStarNode*, float>> m_neighbors;
	AStarNode* m_parent = nullptr;
	float m_distCost = FLT_MAX;
	float m_distCostHeight = FLT_MAX;
	float m_heuristicCost = FLT_MAX;
	float m_cost = FLT_MAX;
};

class AStarHelper {
public:
	AStarHelper() {};
	void AStarHelperInit( std::vector<MapPolygonUnit*> const& units );
	void CalculateRoute( MapPolygonUnit const* start, MapPolygonUnit const* end, std::vector<MapPolygonUnit*>& out_route );
	//void CalculateRouteWaterBlockRoute( MapPolygonUnit const* start, MapPolygonUnit const* end, std::vector<MapPolygonUnit*>& out_route );
	bool CalculateRouteWaterBlockRouteAndHeightWeight( MapPolygonUnit const* start, MapPolygonUnit const* end, std::vector<MapPolygonUnit*>& out_route );
protected:
	float CalculateHeuristicCost( AStarNode* a, AStarNode* b );

	std::vector<AStarNode> m_nodes;
	std::map<MapPolygonUnit const*, AStarNode*> m_unitToNodeMap;
	static inline auto const m_compFunc = []( AStarNode* a, AStarNode* b )->bool { return a->m_cost > b->m_cost; };

};
