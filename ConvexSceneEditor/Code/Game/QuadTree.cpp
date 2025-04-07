#include "Game/QuadTree.hpp"
#include "Game/Convex.hpp"

void SymmetricQuadTree::BuildTree( std::vector<Convex2*> const& convexArray, int numOfRecursive, AABB2 const& totalBounds )
{
	m_nodes.clear();

	int numOfNodes = 0;
	for (int i = 0; i < numOfRecursive; ++i) {
		numOfNodes += IntPow( 4, i );
	}
	m_nodes.resize( numOfNodes );

	int sumK = 0;
	for (int i = 0; i < numOfRecursive; ++i) {
		if (i == 0) {
			m_nodes[sumK].m_bounds = totalBounds;
			++sumK;
		}
		else if (i != numOfRecursive - 1) {
			int numOfJInLevel = IntPow( 4, i );
			for (int j = 0; j < numOfJInLevel; ++j) {
				int parentIndex = GetParentIndex( sumK );
				if (sumK == GetFirstLBChild( parentIndex )) {
					AABB2 const& parentBounds = m_nodes[parentIndex].m_bounds;
					m_nodes[sumK].m_bounds = AABB2( parentBounds.m_mins, parentBounds.m_mins + parentBounds.GetDimensions() * 0.5f);
				}
				else if (sumK == GetSecondRBChild( parentIndex )) {
					AABB2 const& parentBounds = m_nodes[parentIndex].m_bounds;
					Vec2 minsPos = parentBounds.m_mins + Vec2(parentBounds.m_maxs.x - parentBounds.m_mins.x, 0.f) * 0.5f;
					m_nodes[sumK].m_bounds = AABB2( minsPos, minsPos + parentBounds.GetDimensions() * 0.5f );
				}
				else if (sumK == GetThirdLTChild( parentIndex )) {
					AABB2 const& parentBounds = m_nodes[parentIndex].m_bounds;
					Vec2 minsPos = parentBounds.m_mins + Vec2( 0.f, parentBounds.m_maxs.y - parentBounds.m_mins.y ) * 0.5f;
					m_nodes[sumK].m_bounds = AABB2( minsPos, minsPos + parentBounds.GetDimensions() * 0.5f );
				}
				else {
					AABB2 const& parentBounds = m_nodes[parentIndex].m_bounds;
					Vec2 minsPos = parentBounds.m_mins + Vec2( 0.f, parentBounds.m_maxs.y - parentBounds.m_mins.y ) * 0.5f;
					m_nodes[sumK].m_bounds = AABB2( parentBounds.m_mins + parentBounds.GetDimensions() * 0.5f, parentBounds.m_maxs );
				}
				++sumK;
			}
		}
		else { // the last level
			int numOfJInLevel = IntPow( 4, i );
			for (int j = 0; j < numOfJInLevel; ++j) {
				int parentIndex = GetParentIndex( sumK );
				if (sumK == GetFirstLBChild( parentIndex )) {
					AABB2 const& parentBounds = m_nodes[parentIndex].m_bounds;
					m_nodes[sumK].m_bounds = AABB2( parentBounds.m_mins, parentBounds.m_mins + parentBounds.GetDimensions() * 0.5f );
				}
				else if (sumK == GetSecondRBChild( parentIndex )) {
					AABB2 const& parentBounds = m_nodes[parentIndex].m_bounds;
					Vec2 minsPos = parentBounds.m_mins + Vec2( parentBounds.m_maxs.x - parentBounds.m_mins.x, 0.f ) * 0.5f;
					m_nodes[sumK].m_bounds = AABB2( minsPos, minsPos + parentBounds.GetDimensions() * 0.5f );
				}
				else if (sumK == GetThirdLTChild( parentIndex )) {
					AABB2 const& parentBounds = m_nodes[parentIndex].m_bounds;
					Vec2 minsPos = parentBounds.m_mins + Vec2( 0.f, parentBounds.m_maxs.y - parentBounds.m_mins.y ) * 0.5f;
					m_nodes[sumK].m_bounds = AABB2( minsPos, minsPos + parentBounds.GetDimensions() * 0.5f );
				}
				else {
					AABB2 const& parentBounds = m_nodes[parentIndex].m_bounds;
					Vec2 minsPos = parentBounds.m_mins + Vec2( 0.f, parentBounds.m_maxs.y - parentBounds.m_mins.y ) * 0.5f;
					m_nodes[sumK].m_bounds = AABB2( parentBounds.m_mins + parentBounds.GetDimensions() * 0.5f, parentBounds.m_maxs );
				}
				for (auto convex : convexArray) {
					if (DoAABB2sOverlap2D( convex->m_boundingAABB, m_nodes[sumK].m_bounds )) {
						m_nodes[sumK].m_containingConvex.push_back( convex );
					}
				}
				++sumK;
			}
		}
	}
}

void SymmetricQuadTree::SolveRayResult( Vec2 const& startPos, Vec2 const& forwardVec, float maxDist, std::vector<Convex2*> const& convexArray, std::vector<Convex2*>& out_latentRes )
{
	for (auto convex : convexArray) {
		convex->m_symmetricQuadTreeFlag = false;
	}

	int ptr = 0;
	while (ptr < (int)m_nodes.size()) {
		if (RayCastVsAABB2DResultOnly( startPos, forwardVec, maxDist, m_nodes[ptr].m_bounds )) {
			if ((int)m_nodes[ptr].m_containingConvex.size() > 0) {
				for (auto convex : m_nodes[ptr].m_containingConvex) {
					if (convex->m_symmetricQuadTreeFlag == false) {
						convex->m_symmetricQuadTreeFlag = true;
						out_latentRes.push_back( convex );
					}
				}
				while (ptr % 4 == 0 && ptr != 0) {
					ptr = GetParentIndex( ptr );
				}
				if (ptr == 0) {
					break;
				}
				else {
					++ptr;
				}
			}
			else {
				int child = GetFirstLBChild( ptr );
				if (child >= (int)m_nodes.size()) {
					while (ptr % 4 == 0 && ptr != 0) {
						ptr = GetParentIndex( ptr );
					}
					if (ptr == 0) {
						break;
					}
					else {
						++ptr;
					}
				}
				else {
					ptr = child;
				}
			}
		}
		else {
			while (ptr % 4 == 0 && ptr != 0) {
				ptr = GetParentIndex( ptr );
			}
			if(ptr == 0){
				break;
			}
			else {
				++ptr;
			}
		}
	}
}

int SymmetricQuadTree::GetFirstLBChild( int index )
{
	return index * 4 + 1;
}

int SymmetricQuadTree::GetSecondRBChild( int index )
{
	return index * 4 + 2;
}

int SymmetricQuadTree::GetThirdLTChild( int index )
{
	return index * 4 + 3;
}

int SymmetricQuadTree::GetForthRTChild( int index )
{
	return index * 4 + 4;
}

int SymmetricQuadTree::GetParentIndex( int index )
{
	if (index % 4 == 0) {
		return index / 4 - 1;
	}
	return index / 4;
}
