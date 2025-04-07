#include "Game/Convex.hpp"
#include "Game/BVH.hpp"

void AABB2Tree::BuildTree( std::vector<Convex2*> const& convexArray, int numOfRecursive, AABB2 const& totalBounds )
{
	// space partition
	m_nodes.clear();

	int numOfNodes = 0;
	for (int i = 0; i < numOfRecursive; ++i) {
		numOfNodes += IntPow( 2, i );
	}
	m_nodes.resize( numOfNodes );
	if (numOfNodes == 0) {
		return;
	}
	m_nodes[0].m_containingConvex = convexArray;

	int sumK = 0;
	for (int i = 0; i < numOfRecursive; ++i) {
		if (i == 0) {
			m_nodes[sumK].m_bounds = totalBounds;
			++sumK;
		}
		else {
			int numOfJInLevel = IntPow( 2, i );
			if (i == numOfRecursive - 1) {
				m_startOfLastLevel = sumK;
			}
			for (int j = 0; j < numOfJInLevel; ++j) {
				int parentIndex = GetParentIndex( sumK );
				if (sumK == parentIndex * 2 + 1) {
					AABB2 const& parentBounds = m_nodes[parentIndex].m_bounds;
					// vertical slice left
					if (i % 2 == 1) {
						float xPivot = (parentBounds.m_maxs.x - parentBounds.m_mins.x) * 0.5f;
						for (auto convex : m_nodes[parentIndex].m_containingConvex) {
							if (convex->m_boundingDiscCenter.x < xPivot) {
								m_nodes[sumK].m_containingConvex.push_back( convex );
							}
						}
					} // horizontal slice top
					else {
						float yPivot = (parentBounds.m_maxs.y - parentBounds.m_mins.y) * 0.5f;
						for (auto convex : m_nodes[parentIndex].m_containingConvex) {
							if (convex->m_boundingDiscCenter.y >= yPivot) {
								m_nodes[sumK].m_containingConvex.push_back( convex );
							}
						}
					}
				}
				else {
					AABB2 const& parentBounds = m_nodes[parentIndex].m_bounds;
					// vertical slice right
					if (i % 2 == 1) {
						float xPivot = (parentBounds.m_maxs.x - parentBounds.m_mins.x) * 0.5f;
						for (auto convex : m_nodes[parentIndex].m_containingConvex) {
							if (convex->m_boundingDiscCenter.x >= xPivot) {
								m_nodes[sumK].m_containingConvex.push_back( convex );
							}
						}
					} // horizontal slice down
					else {
						float yPivot = (parentBounds.m_maxs.y - parentBounds.m_mins.y) * 0.5f;
						for (auto convex : m_nodes[parentIndex].m_containingConvex) {
							if (convex->m_boundingDiscCenter.y < yPivot) {
								m_nodes[sumK].m_containingConvex.push_back( convex );
							}
						}
					}
				}
				if (!m_nodes[sumK].m_containingConvex.empty()) {
					float minX = FLT_MAX, maxX = -1.f, minY = FLT_MAX, maxY = -1.f;
					for (auto convex : m_nodes[sumK].m_containingConvex) {
						for (auto const& vert : convex->m_convexPoly.GetVertexArray()) {
							if (vert.x > maxX) {
								maxX = vert.x;
							}
							if (vert.x < minX) {
								minX = vert.x;
							}
							if (vert.y < minY) {
								minY = vert.y;
							}
							if (vert.y > maxY) {
								maxY = vert.y;
							}
						}
					}
					m_nodes[sumK].m_bounds = AABB2( Vec2( minX, minY ), Vec2( maxX, maxY ) );
				}
				else {
					m_nodes[sumK].m_bounds = AABB2( Vec2( -1.f, -1.f ), Vec2( 0.f, 0.f ) );
				}
				++sumK;
			}
		}
	}
}

void AABB2Tree::SolveRayResult( Vec2 const& startPos, Vec2 const& forwardVec, float maxDist, std::vector<Convex2*>& out_latentRes )
{
	int ptr = 0;
	while (ptr < (int)m_nodes.size()) {
		if (RayCastVsAABB2DResultOnly( startPos, forwardVec, maxDist, m_nodes[ptr].m_bounds )) {
			if (ptr >= m_startOfLastLevel) {
				for (auto convex : m_nodes[ptr].m_containingConvex) {
					out_latentRes.push_back( convex );
				}
				while (ptr % 2 == 0 && ptr != 0) {
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
				int child = ptr * 2 + 1;
				if (child >= (int)m_nodes.size()) {
					while (ptr % 2 == 0 && ptr != 0) {
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
			while (ptr % 2 == 0 && ptr != 0) {
				ptr = GetParentIndex( ptr );
			}
			if (ptr == 0) {
				break;
			}
			else {
				++ptr;
			}
		}
	}
}

int AABB2Tree::GetParentIndex( int index )
{
	if (index % 2 == 0) {
		return (index >> 1) - 1;
	}
	return index >> 1;
}

