#include "Game/Road.hpp"
#include "Game/City.hpp"
#include "Game/MapPolygonUnit.hpp"
#include "Game/Map.hpp"
#include "Game/AStarHelper.hpp"

Road::Road( City* city1, City* city2 )
	:m_city1(city1), m_city2(city2)
{

}

Road::~Road()
{

}

void Road::Initialize()
{
	Map* map = GetCurMap();
	MapPolygonUnit* prov1 = m_city1->m_provIn;
	MapPolygonUnit* prov2 = m_city2->m_provIn;
	std::vector<MapPolygonUnit*> route;
	if (!map->m_aStarHelper.CalculateRouteWaterBlockRouteAndHeightWeight( prov1, prov2, route )) {
		map->m_aStarHelper.CalculateRoute( prov1, prov2, route );
	}

	bool tempExpandFromZero = false;
	for (int i = 0; i < (int)route.size(); i++) {
		MapPolygonUnit* prov = route[i];
		// if meet another road in province
		if (prov->m_roadData) {
			// if it is the start province
			if (i == 0) { // no problem! try start
				m_anchorPoints.push_back( std::pair<Vec2, bool>( prov->m_roadData->m_position, true ) );
				tempExpandFromZero = true;
			}
			else {
				if (tempExpandFromZero && i == 1) { // try start failed, find the same route which other road has
					if (prov->m_roadData->IsConnectedTo(route[0]->m_roadData)) { // if the first and second provinces are connected
						m_isGarbage = true; // do nothing, waste road
					}
					else { // not connected? connect them two
						prov->m_roadData->m_roadsPassing.push_back( this );
						prov->m_roadData->Connect( route[i - 1]->m_roadData );
						if ((int)prov->m_cities.size() > 0 || (int)prov->m_towns.size() > 0) {
							m_anchorPoints.push_back( std::pair<Vec2, bool>( prov->m_roadData->m_position, true ) );
						}
						else {
							m_anchorPoints.push_back( std::pair<Vec2, bool>( prov->m_roadData->m_position, false ) );
						}
					}
					break;
				}
				else { // just stop
					prov->m_roadData->m_roadsPassing.push_back( this );
					prov->m_roadData->Connect( route[i - 1]->m_roadData );
					if ((int)prov->m_cities.size() > 0 || (int)prov->m_towns.size() > 0) {
						m_anchorPoints.push_back( std::pair<Vec2, bool>( prov->m_roadData->m_position, true ) );
					}
					else {
						m_anchorPoints.push_back( std::pair<Vec2, bool>( prov->m_roadData->m_position, false ) );
					}
					break;
				}
			}
		}
		else {
			prov->m_roadData = new ProvinceRoadData();
			prov->m_roadData->m_roadsPassing.push_back( this );
			if (i != 0) {
				prov->m_roadData->Connect( route[i - 1]->m_roadData );
			}
			if ((int)prov->m_cities.size() > 0) {
				prov->m_roadData->m_position = prov->m_cities[0]->m_position;
				m_anchorPoints.push_back( std::pair<Vec2, bool>( prov->m_roadData->m_position, true ) );
			}
			else if((int)prov->m_towns.size() > 0){
				prov->m_roadData->m_position = prov->m_towns[0]->m_position;
				m_anchorPoints.push_back( std::pair<Vec2, bool>( prov->m_roadData->m_position, true ) );
			}
			else {
				prov->m_roadData->m_position = prov->GetRandomPointNearCenter();
				m_anchorPoints.push_back( std::pair<Vec2, bool>( prov->m_roadData->m_position, false ) );
			}
		}
	}

	// add more anchor points to make the road follow the right path
	int anchorSize = (int)m_anchorPoints.size();
	int curInsertIndex = 1;
	for (int i = 0; i < anchorSize - 1; i++) {
		StarEdge* sharedEdge = GetSharedEdge( route[i], route[i + 1] );
		if (sharedEdge) {
			RayCastResult2D res;
			Vec2 disp = m_anchorPoints[curInsertIndex].first - m_anchorPoints[curInsertIndex - 1].first;
			float length = disp.GetLength();
			Vec2 normal = disp / length;
			Vec2 leftNormal = Vec2( normal.y, -normal.x );
			Vec2 SS = sharedEdge->m_startPos - m_anchorPoints[curInsertIndex - 1].first;
			Vec2 SE = sharedEdge->m_endPos - m_anchorPoints[curInsertIndex - 1].first;
			float SSOnLeft = DotProduct2D( SS, leftNormal );
			float SEOnLeft = DotProduct2D( SE, leftNormal );
			if (SSOnLeft >= 0.f && SEOnLeft >= 0.f) {
				float t = 0.1f;
				if (SSOnLeft > SEOnLeft) {
					t = 0.9f;
				}
				Vec2 newAnchorPoint = Interpolate( sharedEdge->m_startPos, sharedEdge->m_endPos, t );
				m_anchorPoints.insert( m_anchorPoints.begin() + curInsertIndex, std::pair<Vec2, bool>( newAnchorPoint, false ) );
				curInsertIndex += 2;
			}
			else if (SSOnLeft <= 0.f && SEOnLeft <= 0.f) {
				float t = 0.1f;
				if (SSOnLeft < SEOnLeft) {
					t = 0.9f;
				}
				Vec2 newAnchorPoint = Interpolate( sharedEdge->m_startPos, sharedEdge->m_endPos, t );
				m_anchorPoints.insert( m_anchorPoints.begin() + curInsertIndex, std::pair<Vec2, bool>( newAnchorPoint, false ) );
				curInsertIndex += 2;
			}
			else {
				++curInsertIndex;
			}
		}
	}

	std::vector<Vec2> points;
	for (int i = 0; i < (int)m_anchorPoints.size(); ++i) {
		auto const& pair = m_anchorPoints[i];
		if (pair.second && i != 0) {
			points.push_back( pair.first );
			m_roadCurve.push_back( CatmullRomSpline2D( points ) );
			points.clear();
			points.push_back( pair.first );
		}
		else {
			points.push_back( pair.first );
		}
	}
	if ((int)points.size() > 0) {
		m_roadCurve.push_back( CatmullRomSpline2D( points ) );
	}
	//m_roadCurve.ResetAllPoints( m_anchorPoints );
}

void ProvinceRoadData::Connect( ProvinceRoadData* other )
{
	m_neighbors.push_back( other );
	other->m_neighbors.push_back( this );
}

bool ProvinceRoadData::IsConnectedTo( ProvinceRoadData* other ) const
{
	for (auto roadData : m_neighbors) {
		if (roadData == other) {
			return true;
		}
	}
	return false;
}
