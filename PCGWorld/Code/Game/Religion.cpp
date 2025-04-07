#include "Game/Religion.hpp"
#include "Game/Map.hpp"
#include "Game/NameGenerator.hpp"

Religion::Religion(int id)
	:m_id(id)
{
	Map* map = GetCurMap();
	m_color = Rgba8( (unsigned char)map->m_mapRNG->RollRandomIntInRange( 100, 200 ), (unsigned char)map->m_mapRNG->RollRandomIntInRange( 100, 200 ), (unsigned char)map->m_mapRNG->RollRandomIntInRange( 100, 200 ) );
	m_name = map->m_religionNameGenerator->GenerateReligionName();
}

void Religion::GetBoundsPointsForLabel( Vec2& out_startPos, Vec2& out_endPos ) const
{
	Map* map = GetCurMap();
	Vec2 leftTopPos, leftBottomPos, rightTopPos, rightBottomPos;
	float distToLT = FLT_MAX, distToLB = FLT_MAX, distToRT = FLT_MAX, distToRB = FLT_MAX;
	float minX = map->m_bounds.m_mins.x;
	float minY = map->m_bounds.m_mins.y;
	float maxX = map->m_bounds.m_maxs.x;
	float maxY = map->m_bounds.m_maxs.y;
	Vec2 const leftTopBounds = Vec2( minX, maxY );
	Vec2 const rightTopBounds = Vec2( maxX, maxY );
	Vec2 const leftBottomBounds = Vec2( minX, minY );
	Vec2 const rightBottomBounds = Vec2( maxX, minY );
	for (auto prov : map->m_mapPolygonUnits) {
		if (prov->m_majorReligion == this && !prov->m_isFarAwayFakeUnit && !prov->IsWater()) {
			for (auto edge : prov->m_edges) {
				float thisDistToLT = GetDistanceSquared2D( leftTopBounds, edge->m_startPos );
				float thisDistToRT = GetDistanceSquared2D( rightTopBounds, edge->m_startPos );
				float thisDistToLB = GetDistanceSquared2D( leftBottomBounds, edge->m_startPos );
				float thisDistToRB = GetDistanceSquared2D( rightBottomBounds, edge->m_startPos );
				if (thisDistToLT < distToLT) {
					distToLT = thisDistToLT;
					leftTopPos = edge->m_startPos;
				}
				if (thisDistToLB < distToLB) {
					distToLB = thisDistToLB;
					leftBottomPos = edge->m_startPos;
				}
				if (thisDistToRT < distToRT) {
					distToRT = thisDistToRT;
					rightTopPos = edge->m_startPos;
				}
				if (thisDistToRB < distToRB) {
					distToRB = thisDistToRB;
					rightBottomPos = edge->m_startPos;
				}
			}
		}
	}
	// only 4 possible cases
	// 1 left top - right top
	float LTToRTDist = GetDistanceSquared2D( leftTopPos, rightTopPos );
	// 2 left bottom - right bottom
	float LBToRBDist = GetDistanceSquared2D( leftBottomPos, rightBottomPos );
	// 3 left top - right bottom
	float LTToRBDist = GetDistanceSquared2D( leftTopPos, rightBottomPos );
	// 4 left bottom - right top
	float LBToRTDist = GetDistanceSquared2D( leftBottomPos, rightTopPos );

	if (LTToRTDist == std::max( LTToRTDist, std::max( LBToRBDist, std::max( LTToRBDist, LBToRTDist ) ) )) {
		Vec2 intrStartPos = Interpolate( leftTopPos, leftBottomPos, 0.5f );
		Vec2 intrEndPos = Interpolate( rightTopPos, rightBottomPos, 0.5f );
		out_startPos = Interpolate( intrStartPos, intrEndPos, 0.2f );
		out_endPos = Interpolate( intrStartPos, intrEndPos, 0.8f );
	}
	else if (LBToRBDist == std::max( LTToRTDist, std::max( LBToRBDist, std::max( LTToRBDist, LBToRTDist ) ) )) {
		Vec2 intrStartPos = Interpolate( leftTopPos, leftBottomPos, 0.5f );
		Vec2 intrEndPos = Interpolate( rightTopPos, rightBottomPos, 0.5f );
		out_startPos = Interpolate( intrStartPos, intrEndPos, 0.2f );
		out_endPos = Interpolate( intrStartPos, intrEndPos, 0.8f );
	}
	else if (LTToRBDist == std::max( LTToRTDist, std::max( LBToRBDist, std::max( LTToRBDist, LBToRTDist ) ) )) {
		out_startPos = Interpolate( leftTopPos, rightBottomPos, 0.2f );
		out_endPos = Interpolate( leftTopPos, rightBottomPos, 0.8f );
	}
	else {
		out_startPos = Interpolate( leftBottomPos, rightTopPos, 0.2f );
		out_endPos = Interpolate( leftBottomPos, rightTopPos, 0.8f );
	}

}
