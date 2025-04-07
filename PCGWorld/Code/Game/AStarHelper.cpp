#include "Game/AStarHelper.hpp"
#include "Game\MapPolygonUnit.hpp"

void AStarHelper::AStarHelperInit( std::vector<MapPolygonUnit*> const& units )
{
	m_nodes.reserve( units.size() );
	for (auto unit : units) {
		if (!unit->m_isFarAwayFakeUnit) {
			m_nodes.emplace_back( unit );
			m_unitToNodeMap[unit] = &(m_nodes[m_nodes.size() - 1]);
		}
	}
	for (auto unit : units) {
		if (!unit->m_isFarAwayFakeUnit) {
			AStarNode* node = (*m_unitToNodeMap.find( unit )).second;
			for (auto adjUnit : unit->m_adjacentUnits) {
				AStarNode* adjNode = (*m_unitToNodeMap.find( adjUnit )).second;
				node->m_neighbors.emplace_back( adjNode, GetDistance2D( unit->m_geoCenterPos, adjUnit->m_geoCenterPos ) );
			}
		}
	}

}

void AStarHelper::CalculateRoute( MapPolygonUnit const* start, MapPolygonUnit const* end, std::vector<MapPolygonUnit*>& out_route )
{
	out_route.clear();
	for (auto& node : m_nodes) {
		node.m_cost = FLT_MAX;
	}
	std::priority_queue < AStarNode*, std::vector<AStarNode*>, decltype(m_compFunc) > openList;
	//std::vector<AStarNode*> closedList;
	AStarNode* startNode = (*m_unitToNodeMap.find( start )).second;
	startNode->m_cost = 0.f;
	startNode->m_distCost = 0.f;
	startNode->m_heuristicCost = 0.f;
	startNode->m_parent = nullptr;
	openList.push( startNode );

	AStarNode* endNode = (*m_unitToNodeMap.find( end )).second;

	while (1) {
		AStarNode* thisNode = openList.top();
		if (thisNode == endNode) {
			//closedList.push_back( thisNode );
			break;
		}
		else {
			openList.pop();
			//closedList.push_back( thisNode );
			for (auto& adjNodePair : thisNode->m_neighbors) {
				// haven't visited
				if (adjNodePair.first->m_cost == FLT_MAX) {
					adjNodePair.first->m_distCost = thisNode->m_distCost + adjNodePair.second;
					adjNodePair.first->m_heuristicCost = CalculateHeuristicCost( adjNodePair.first, endNode );
					adjNodePair.first->m_cost = adjNodePair.first->m_distCost + adjNodePair.first->m_heuristicCost;
					adjNodePair.first->m_parent = thisNode;
					openList.push( adjNodePair.first );
				}
				// visited
				else {
					// has a smaller connection option
					if (adjNodePair.first->m_distCost > thisNode->m_distCost + adjNodePair.second) {
						adjNodePair.first->m_distCost = thisNode->m_distCost + adjNodePair.second;
						adjNodePair.first->m_cost = adjNodePair.first->m_distCost + adjNodePair.first->m_heuristicCost;
						adjNodePair.first->m_parent = thisNode;
					}
				}
			}
		}
	}

	AStarNode* nodeIter = endNode;
	out_route.clear();
	while (nodeIter != nullptr) {
		out_route.push_back( const_cast<MapPolygonUnit*>(nodeIter->m_base) );
		nodeIter = nodeIter->m_parent;
	}
}

bool AStarHelper::CalculateRouteWaterBlockRouteAndHeightWeight( MapPolygonUnit const* start, MapPolygonUnit const* end, std::vector<MapPolygonUnit*>& out_route )
{
	out_route.clear();
	for (auto& node : m_nodes) {
		node.m_cost = FLT_MAX;
	}
	std::priority_queue < AStarNode*, std::vector<AStarNode*>, decltype(m_compFunc) > openList;
	AStarNode* startNode = (*m_unitToNodeMap.find( start )).second;
	startNode->m_cost = 0.f;
	startNode->m_distCost = 0.f;
	startNode->m_distCostHeight = 0.f;
	startNode->m_heuristicCost = 0.f;
	startNode->m_parent = nullptr;
	openList.push( startNode );

	AStarNode* endNode = (*m_unitToNodeMap.find( end )).second;

	while (1) {
		if (openList.empty()) {
			return false;
		}
		AStarNode* thisNode = openList.top();
		if (thisNode == endNode) {
			//closedList.push_back( thisNode );
			break;
		}
		else {
			openList.pop();
			//closedList.push_back( thisNode );
			for (auto& adjNodePair : thisNode->m_neighbors) {
				// check if it is water
				if (!adjNodePair.first->m_base->IsWater()) { // haven't visited
					if (adjNodePair.first->m_cost == FLT_MAX) {
						adjNodePair.first->m_distCostHeight = thisNode->GetDistCostByHeight( thisNode ) + adjNodePair.second;
						adjNodePair.first->m_heuristicCost = CalculateHeuristicCost( adjNodePair.first, endNode );
						adjNodePair.first->m_cost = adjNodePair.first->m_distCostHeight + adjNodePair.first->m_heuristicCost;
						adjNodePair.first->m_parent = thisNode;
						openList.push( adjNodePair.first );
					}
					// visited
					else {
						// has a smaller connection option
						if (adjNodePair.first->m_distCostHeight > thisNode->GetDistCostByHeight( thisNode ) + adjNodePair.second) {
							adjNodePair.first->m_distCostHeight = thisNode->GetDistCostByHeight( thisNode ) + adjNodePair.second;
							adjNodePair.first->m_cost = adjNodePair.first->m_distCostHeight + adjNodePair.first->m_heuristicCost;
							adjNodePair.first->m_parent = thisNode;
						}
					}
				}
			}
		}
	}

	AStarNode* nodeIter = endNode;
	out_route.clear();
	while (nodeIter != nullptr) {
		out_route.push_back( const_cast<MapPolygonUnit*>(nodeIter->m_base) );
		nodeIter = nodeIter->m_parent;
	}
	return true;
}

float AStarHelper::CalculateHeuristicCost( AStarNode* a, AStarNode* b )
{
	return GetDistance2D( a->m_base->m_geoCenterPos, b->m_base->m_geoCenterPos );
}

float AStarNode::GetDistCostByHeight( AStarNode* parent ) const
{
	return parent->m_distCostHeight + abs( m_base->m_height - parent->m_base->m_height ) / 200.f;
}
