#include "Game/River.hpp"
#include "Game/MapPolygonUnit.hpp"
#include "Game/Map.hpp"

River::River()
{

}

River::~River()
{

}

void River::CalculateRiverRoute()
{
	m_anchorPoint.reserve( 20 );
	m_routeUnits.reserve( 20 );
	if ((int)m_routeUnits.size() != 1) {
		return;
	}
	m_startUnit = m_routeUnits[0];
	// if starts in water or starts in a river route, no need to have this river
	if (m_startUnit->IsWater()) {
		m_isGarbage = true;
		return;
	}
	if (m_startUnit->m_riverOnThis) {
		m_isGarbage = true;
		return;
	}
	Map* map = GetCurMap();
	MapPolygonUnit* curUnit = m_startUnit;
	curUnit->m_riverOnThis = this;
	curUnit->m_riverAnchorPos = m_anchorPoint[0];
	while (1) {
		// if no lower neighbor units, try to go to higher neighbors
		if ((int)curUnit->m_adjLowerUnits.size() == 0 && (int)curUnit->m_adjKindOfLowerUnits.size() > 0) {
			MapPolygonUnit* testUnit = nullptr;
			int count = TestAdjKindOfLowerUnits( curUnit, &testUnit );
			// if exceed the max test count, end
			if (count >= 10000) {
				curUnit->m_landform = LandformType::Lake;
				// make a lake here
				while ((int)m_routeUnits.size() >= 1) {
					if (m_routeUnits[m_routeUnits.size() - 1]->IsAdjacentToUnit( curUnit )) {
						m_routeUnits[m_routeUnits.size() - 1]->m_landform = LandformType::Lake;
						m_routeUnits.pop_back();
					}
					else {
						break;
					}
				}
				if ((int)m_routeUnits.size() >= 1) {
					m_endUnit = m_routeUnits[m_routeUnits.size() - 1];
				}
				else {
					m_isGarbage = true;
				}
				break;
			}
			else {
				curUnit = testUnit;
			}
		}
		else if ((int)curUnit->m_adjLowerUnits.size() > 0) {
			MapPolygonUnit* testUnit = nullptr; // the final result variable (where the river goes next)
			int count = 0;
			bool passDirTest = false;
			do {
				// roll a random adjacent province
				testUnit = curUnit->m_adjLowerUnits[map->m_mapRNG->RollRandomIntLessThan( (int)curUnit->m_adjLowerUnits.size() )];
				count++;
				Vec2 dir = testUnit->m_geoCenterPos - curUnit->m_geoCenterPos;
				// if near to east coast, the river is more likely to go east
				if (count <= 2000 && curUnit->m_nearestSeaDir == Direction::East && dir.x > 0.f && abs( dir.x ) > 0.8f * abs( dir.y )) {
					passDirTest = true;
				}
				// if near to west coast, the river is more likely to go west
				else if (count <= 2000 && curUnit->m_nearestSeaDir == Direction::West && dir.x < 0.f && abs( dir.x ) > 0.8f * abs( dir.y )) {
					passDirTest = true;
				}
				// do not have sharp turns
				if (GetPrevUnitOnRoute( curUnit )
					&& DotProduct2D( (testUnit->m_geoCenterPos - curUnit->m_geoCenterPos).GetNormalized(),
						(curUnit->m_geoCenterPos - GetPrevUnitOnRoute( curUnit )->m_geoCenterPos).GetNormalized()) < -0.2f) {
					passDirTest = false;
				}
				// if there is already a river in that province
				if (testUnit->m_riverOnThis && testUnit->m_riverOnThis->m_startUnit != testUnit) {
					// if two river converge together, it would be better to have a similar flowing direction
					MapPolygonUnit* prevUnit = testUnit->m_riverOnThis->GetPrevUnitOnRoute( testUnit );
					if (prevUnit) {
						Vec2 dirToConverge = (testUnit->m_geoCenterPos - prevUnit->m_geoCenterPos).GetNormalized();
						Vec2 thisRiverDir = dir.GetNormalized(); // the below value 0.f can be changed, if it stays in 0.f, the normalize operation can be optimized
						if (DotProduct2D( thisRiverDir, dirToConverge ) <= 0.f) { // if the two river is not going to the similar direction
							if (count >= 10000) {
								break; // give up to find a result in lower adjacent provinces
							}
							else {
								continue; // start a new try
							}
						}
					}
				}
				// if in the coast, it should try to just go into the sea
				if (passDirTest && curUnit->m_isCoast && !testUnit->IsWater()) {
					passDirTest = false;
				}
				// if exceed the max test count, try to find higher adjacent provinces
				if (count >= 10000) {
					break;
				}
				// if the unit has not been passed by this river
				if (testUnit->m_riverOnThis != this) {
					// pass the above tests
					if (passDirTest) {
						break;
					}
					// or reach certain test times
					if (count > 2000) {
						break;
					}
				}
			} while (1);
			// if do not find a result and reach the maximum test counts and break the loop
			if (count >= 10000) {
				// try find higher neighbor units
				if ((int)curUnit->m_adjKindOfLowerUnits.size() > 0) {
					count = TestAdjKindOfLowerUnits( curUnit, &testUnit );
				}
				else {
					count = 10000;
				}
				// if even no higher suited neighbors
				if (count >= 10000) {
					// make a lake here
					curUnit->m_landform = LandformType::Lake;
					m_routeUnits.pop_back();
					m_anchorPoint.pop_back();
					// ToDo: There will be some issue(other rivers may interrupted by this)
					/*while ((int)m_routeUnits.size() >= 2) {
						if (m_routeUnits[m_routeUnits.size() - 2]->IsAdjacentToUnit( curUnit )) {
							//m_routeUnits[m_routeUnits.size() - 2]->m_landform = LandformType::Lake;
							m_routeUnits[m_routeUnits.size() - 2]->m_riverOnThis = nullptr;
							m_routeUnits.pop_back();
							m_anchorPoint.pop_back();
						}
						else {
							break;
						}
					}*/
					m_routeUnits.push_back( curUnit );
					m_anchorPoint.push_back( curUnit->m_geoCenterPos );
					if ((int)m_routeUnits.size() >= 1) {
						m_endUnit = m_routeUnits[m_routeUnits.size() - 1];
					}
					else {
						m_isGarbage = true;
					}
					break;
				}
				else {
					curUnit = testUnit;
				}
			}
			else {
				curUnit = testUnit;
			}
		}
		// if no higher neighbors and no lower neighbors, stop
		else {
			m_endUnit = curUnit;
			break;
		}

		// if meet another river, stops here
		if (curUnit->m_riverOnThis != nullptr) {
			// if meet the source of the river, inherit that river
			if (curUnit == curUnit->m_riverOnThis->m_startUnit && !curUnit->m_riverOnThis->m_isGarbage) {
				curUnit->m_riverOnThis->m_isGarbage = true;
				for (int i = 0; i < (int)curUnit->m_riverOnThis->m_routeUnits.size(); i++) {
					if (i != 0) {
						curUnit->m_riverOnThis->m_routeUnits[i]->m_riverOnThis = this;
					}
					m_routeUnits.push_back( curUnit->m_riverOnThis->m_routeUnits[i] );
					m_anchorPoint.push_back( curUnit->m_riverOnThis->m_routeUnits[i]->m_riverAnchorPos );
				}
				curUnit->m_riverOnThis = this;
				m_endUnit = curUnit->m_riverOnThis->m_endUnit;
			}
			// if it's not the source, just stop
			else {
				// not converged in a ocean\lake
				if (!curUnit->IsWater()) {
					if (curUnit->m_riverOnThis->m_riverConvergeTo) {
						curUnit->m_riverOnThis->m_riverConvergeTo->m_branches.push_back( this );
						m_riverConvergeTo = curUnit->m_riverOnThis->m_riverConvergeTo;
					}
					else {
						curUnit->m_riverOnThis->m_branches.push_back( this );
						m_riverConvergeTo = curUnit->m_riverOnThis;
					}
				}

				m_endUnit = curUnit;
				m_anchorPoint.push_back( curUnit->m_riverAnchorPos );
				m_routeUnits.push_back( curUnit );
			}
			break;
		}
		else { // else, if not meet any river, set the value for the unit and continue
			curUnit->m_riverOnThis = this;
			curUnit->m_riverAnchorPos = curUnit->GetRandomPointNearCenter();
			m_anchorPoint.push_back( curUnit->m_riverAnchorPos );
			m_routeUnits.push_back( curUnit );
			// if go in to sea, stop
			if (curUnit->IsOcean()) {
				m_endUnit = curUnit;
				break;
			}
			// for lake, give a chance to stop
			if (curUnit->m_landform == LandformType::Lake /*&& map->m_mapRNG->RollRandomFloatZeroToOne() < 0.5f*/) {
				m_endUnit = curUnit;
				break;
			}
		}
	}

	if ((int)m_routeUnits.size() > 0) {
		m_endUnit = m_routeUnits[m_routeUnits.size() - 1];
	}

	// add more anchor points to make the river feel better
	for (int i = 0; i < (int)m_routeUnits.size() - 1; i++) {
		StarEdge* sharedEdge = GetSharedEdge( m_routeUnits[i], m_routeUnits[i + 1] );
		if (sharedEdge) {
			float t = GetCurMap()->m_mapRNG->RollRandomFloatInRange( 0.4f, 0.6f );
			Vec2 newAnchorPoint = Interpolate( sharedEdge->m_startPos, sharedEdge->m_endPos, t );
			m_anchorPoint.insert( m_anchorPoint.begin() + 2 * i + 1, newAnchorPoint );
		}

		//Vec2 newAnchorPoint = Interpolate( m_routeUnits[i]->m_geoCenterPos, m_routeUnits[i + 1]->m_geoCenterPos, 0.5f );
		//m_anchorPoint.insert( m_anchorPoint.begin() + 2 * i + 1, newAnchorPoint );
	}

	// compute the curve
	m_riverCurve.ResetAllPoints( m_anchorPoint );

	m_length = m_riverCurve.GetApproximateLength( 64 );

	// add sandiness
	// the longer the river is, the more the sands are
	m_sandiness += m_length;
	float approximateLengthPerUnit = m_length / (float)m_routeUnits.size();
	for (auto unit : m_routeUnits) {
		if (!unit->IsWater()) {
			if (unit->m_landform == LandformType::Desert) {
				m_sandiness += approximateLengthPerUnit * 3.f;
			}
			else if (unit->m_landform == LandformType::Grassland) {
				m_sandiness += approximateLengthPerUnit * 1.5f;
			}
			else if (unit->m_landform == LandformType::Lowland_Plain) {
				m_sandiness += approximateLengthPerUnit * 0.5f;
			}
			else if (unit->m_landform == LandformType::Highland_Plain) {
				m_sandiness += approximateLengthPerUnit * 0.3f;
			}
			else if (unit->m_landform == LandformType::Savanna) {
				m_sandiness += approximateLengthPerUnit;
			}
			else if (unit->m_landform == LandformType::Tundra) {
				m_sandiness += approximateLengthPerUnit * 0.4f;
			}
		}
	}
}

void River::CalculateRiverStats()
{
	// decide if the river has delta
	float totalLength = m_length;
	int totalRouteUnits = (int)m_routeUnits.size();
	for (auto river : m_branches) {
		m_sandiness += river->m_sandiness;
		totalLength += river->m_length;
		totalRouteUnits += (int)river->m_routeUnits.size();
	}

	if (m_sandiness > 1.5f * totalLength && totalRouteUnits >= 8) {
		m_hasDelta = true;
	}
	// calculate quantity of flow
	// decide by precipitation of passing units and height of the mountains
	m_quantityOfFlow = m_startUnit->m_height * 0.1f;
	for (auto unit : m_routeUnits) {
		m_quantityOfFlow += ((unit->m_summerPrecipitation + unit->m_winterPrecipitation) * 0.5f);
	}

	if (m_endUnit->IsWater()) {
		m_anchorPoint.pop_back();
	}

	// add more anchor\pivot points for the river
	std::vector<Vec2> prevArray = m_anchorPoint;
	m_anchorPoint.clear();
	for (int i = 0; i < (int)(prevArray.size() - 1); ++i) {
		Vec2 forwardVec = prevArray[i + 1] - prevArray[i];
		float dist = forwardVec.GetLength();
		forwardVec /= dist;
		Vec2 leftVec = forwardVec.GetRotated90Degrees();
		Vec2 firstAddedPoint = Interpolate( prevArray[i], prevArray[i + 1], 1.f / 3.f ) + (GetCurMap()->m_mapRNG->RollRandomFloatZeroToOne() - 0.5f) * 2.f * leftVec * dist * 0.2f;
		if (i == 0) {
			m_anchorPoint.push_back( prevArray[i] );
		}
		//		else {
		// 			bool doNotMove = false;
		// 			for (int j = 0; j < (int)m_branches.size(); ++j) {
		// 				if (m_branches[j]->m_anchorPoint.size() > 0 && prevArray[i] == m_branches[j]->m_anchorPoint[m_branches[j]->m_anchorPoint.size() - 1]) {
		// 					doNotMove = true;
		// 					break;
		// 				}
		// 			}
		// 			if (doNotMove) {
		//				m_anchorPoint.push_back( prevArray[i] );
		// 			}
		// 			else {
		// 				m_anchorPoint.push_back( (m_anchorPoint[i * 3 - 1] + firstAddedPoint + prevArray[i]) / 3.f );
		// 			}
		//		}
		m_anchorPoint.push_back( firstAddedPoint );
		m_anchorPoint.push_back( Interpolate( prevArray[i], prevArray[i + 1], 2.f / 3.f ) + (GetCurMap()->m_mapRNG->RollRandomFloatZeroToOne() - 0.5f) * 2.f * leftVec * dist * 0.2f );
		//if (i == (int)(prevArray.size() - 2)) {
		m_anchorPoint.push_back( prevArray[i + 1] );
		//}
	}
}

MapPolygonUnit* River::GetPrevUnitOnRoute( MapPolygonUnit* refUnit )
{
	for (int i = 1; i < (int)m_routeUnits.size(); i++) {
		if (m_routeUnits[i] == refUnit) {
			return m_routeUnits[i - 1];
		}
	}
	return nullptr;
}

float River::GetDistanceToRiver( Vec2 const& refPos ) const
{
	float minDistSquared = FLT_MAX;
	for (int i = 0; i < m_riverCurve.GetNumOfCurves(); i++) {
		for (int j = 0; j < 16; j++) {
			Vec2 anchorPos = m_riverCurve.EvaluateAtParametric( (float)i + (float)j / 16.f );
			float thisDist = GetDistanceSquared2D( anchorPos, refPos );
			if (thisDist < minDistSquared) {
				minDistSquared = thisDist;
			}
		}
	}
	return sqrtf( minDistSquared );
}

float River::GetRiverStartEndDistSquared() const
{
	return GetDistanceSquared2D( m_startUnit->m_geoCenterPos, m_endUnit->m_geoCenterPos );
}

void River::AddVertsForRiver( std::vector<Vertex_PCU>& verts, float thickness, Rgba8 const& color, int numSubdivisions )
{
	// cut the end of the river to let it not touch the sea
// 	int startPosIndex = m_riverCurve.GetNumOfPoints() - 2;
// 	constexpr int numOfSteps = 16;
// 	constexpr float step = 1.f / (float)numOfSteps;
// 	for (int i = 0; i <= numOfSteps; i++) {
// 		Vec2 testPos = m_riverCurve.EvaluateAtParametric( step * (float)i + (float)startPosIndex );
// 		MapPolygonUnit* unit = GetCurMap()->GetUnitByPos( testPos );
// 		if (unit && unit->m_height <= 0.f) {
// 			m_anchorPoint[m_anchorPoint.size() - 1] = testPos;
// 			break;
// 		}
// 	}

	// add verts for river
	m_riverCurve.ResetAllPoints( m_anchorPoint );
	m_riverCurve.AddVertsForCurve2D( verts, thickness, color, numSubdivisions );
	m_length = m_riverCurve.GetApproximateLength( 64 );
// 
// 	for (int i = 0; i < m_riverCurve.GetNumOfPoints(); ++i) {
// 		AddVertsForDisc2D( verts,  m_riverCurve.GetPointAtIndex(i), 0.05f, Rgba8::RED );
// 	}
}

int River::TestAdjKindOfLowerUnits( MapPolygonUnit* curUnit, MapPolygonUnit** out_resUnit )
{
	Map* map = GetCurMap();
	MapPolygonUnit* testUnit = nullptr;
	int count = 0;
	bool passDirTest = false;
	do {
		passDirTest = false;
		testUnit = curUnit->m_adjKindOfLowerUnits[map->m_mapRNG->RollRandomIntLessThan( (int)curUnit->m_adjKindOfLowerUnits.size() )];
		count++;
		Vec2 dir = testUnit->m_geoCenterPos - curUnit->m_geoCenterPos;
		if (count <= 2000 && curUnit->m_nearestSeaDir == Direction::East && dir.x > 0.f && abs( dir.x ) > 0.8f * abs( dir.y )) {
			passDirTest = true;
		}
		else if (count <= 2000 && curUnit->m_nearestSeaDir == Direction::West && dir.x < 0.f && abs( dir.x ) > 0.8f * abs( dir.y )) {
			passDirTest = true;
		}
		// do not have sharp turns
		if (GetPrevUnitOnRoute( curUnit )
			&& DotProduct2D( (testUnit->m_geoCenterPos - curUnit->m_geoCenterPos).GetNormalized(),
				(curUnit->m_geoCenterPos - GetPrevUnitOnRoute( curUnit )->m_geoCenterPos).GetNormalized() ) < -0.2f) {
			passDirTest = false;
		}

		bool passConvergeDirTest = true;
		if (testUnit->m_riverOnThis && count <= 5000) {
			if (testUnit->m_riverOnThis->m_startUnit != testUnit) {
				MapPolygonUnit* prevUnit = testUnit->m_riverOnThis->GetPrevUnitOnRoute( testUnit );
				if (prevUnit) {
					Vec2 dirToConverge = (testUnit->m_geoCenterPos - prevUnit->m_geoCenterPos).GetNormalized();
					Vec2 thisRiverDir = dir.GetNormalized();
					if (DotProduct2D( thisRiverDir, dirToConverge ) <= 0.f) {
						passConvergeDirTest = false;
					}
				}
			}
		}
		// if in the coast, just go into the sea
		if (passConvergeDirTest) {
			if (curUnit->m_isCoast && !testUnit->IsWater()) {
				passConvergeDirTest = false;
			}
		}
		if (count >= 10000) {
			break;
		}
		if (testUnit->m_riverOnThis != this) {
			if (count <= 2000 && passDirTest && passConvergeDirTest) {
				break;
			}
			else if (count >= 2000 && passConvergeDirTest) {
				break;
			}
		}
	} while (1);
	*out_resUnit = testUnit;
	return count;
}

