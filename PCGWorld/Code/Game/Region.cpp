#include "Game/Region.hpp"
#include "Game/Map.hpp"
#include "Game/City.hpp"
#include "Game/Town.hpp"
#include "Game/MapPolygonUnit.hpp"
#include "Game/NameGenerator.hpp"

Region::Region()
{
	Map* map = GetCurMap();
	m_color = Rgba8( (unsigned char)map->m_mapRNG->RollRandomIntInRange( 5, 250 ), (unsigned char)map->m_mapRNG->RollRandomIntInRange( 5, 250 ), (unsigned char)map->m_mapRNG->RollRandomIntInRange( 5, 250 ) );
	m_name = map->m_regionContinentNameGenerator->GenerateProvinceName( nullptr );
}

Region::~Region()
{
	delete m_edgeShowingBuffer;
}

void Region::GainProvince( MapPolygonUnit* prov )
{
	if (std::find( m_containedUnits.begin(), m_containedUnits.end(), prov ) != m_containedUnits.end()) {
		ERROR_RECOVERABLE( "Cannot add province that is already owned by this country!" );
	}
	else {
		m_containedUnits.push_back( prov );
		m_totalPopulation += prov->m_totalPopulation;
		prov->m_region = this;
		for (auto city : prov->m_cities) {
			city->m_region = this;
			m_totalPopulation += city->m_totalPopulation;
		}
		for (auto town : prov->m_towns) {
			town->m_region = this;
			m_totalPopulation += town->m_totalPopulation;
		}
	}
}

void Region::LoseProvince( MapPolygonUnit* prov )
{
	auto iter = std::find( m_containedUnits.begin(), m_containedUnits.end(), prov );
	if (iter != m_containedUnits.end()) {
		prov->m_region = nullptr;
		m_totalPopulation -= prov->m_totalPopulation;
		for (auto city : prov->m_cities) {
			city->m_region = nullptr;
			m_totalPopulation -= city->m_totalPopulation;
		}
		for (auto town : prov->m_towns) {
			town->m_region = nullptr;
			m_totalPopulation -= town->m_totalPopulation;
		}
		m_containedUnits.erase( iter );
	}
	else {
		ERROR_RECOVERABLE( "Cannot lose province that is not owned by this country!" );
	}
}

void Region::GetBoundsPointsForLabel( Vec2& out_startPos, Vec2& out_endPos ) const
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
	for (auto prov : m_containedUnits) {
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
