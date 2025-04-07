#pragma once
#include "Game/GameCommon.hpp"

struct Convex2;

struct SymmetricQuadTreeNode {
	AABB2 m_bounds;
	std::vector<Convex2*> m_containingConvex;
};

class SymmetricQuadTree {
public:
	void BuildTree( std::vector<Convex2*> const& convexArray, int numOfRecursive, AABB2 const& totalBounds );
	void SolveRayResult( Vec2 const& startPos, Vec2 const& forwardVec, float maxDist, std::vector<Convex2*> const& convexArray, std::vector<Convex2*>& out_latentRes );

protected:
	int GetFirstLBChild( int index );
	int GetSecondRBChild( int index );
	int GetThirdLTChild( int index );
	int GetForthRTChild( int index );
	int GetParentIndex( int index );

	std::vector<SymmetricQuadTreeNode> m_nodes;

};