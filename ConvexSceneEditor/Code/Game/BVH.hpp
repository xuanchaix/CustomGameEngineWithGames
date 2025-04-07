#pragma once
#include "Game/GameCommon.hpp"

struct Convex2;

struct AABB2TreeNode {
	AABB2 m_bounds;
	std::vector<Convex2*> m_containingConvex;
};


class AABB2Tree {
public:
	void BuildTree( std::vector<Convex2*> const& convexArray, int numOfRecursive, AABB2 const& totalBounds );
	void SolveRayResult( Vec2 const& startPos, Vec2 const& forwardVec, float maxDist, std::vector<Convex2*>& out_latentRes );

	std::vector<AABB2TreeNode> m_nodes;

protected:
	int GetParentIndex( int index );
	int m_startOfLastLevel = 0;
};
