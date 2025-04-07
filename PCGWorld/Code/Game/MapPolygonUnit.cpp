#include "Game/MapPolygonUnit.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Culture.hpp"
#include "Game/Religion.hpp"
#include "Game/River.hpp"
#include "Game/City.hpp"
#include "Game/Continent.hpp"
#include "Game/Country.hpp"
#include "Game/Region.hpp"
#include "Game/AStarHelper.hpp"
#include "Game/Road.hpp"
#include "Game/Army.hpp"
#include <set>

MapPolygonUnit::MapPolygonUnit( Vec2 const& centerPos )
	:m_centerPosition(centerPos)
{
	//m_pops.reserve( 100 );
}

MapPolygonUnit::~MapPolygonUnit()
{
	for (int i = 0; i < (int)m_edges.size(); i++) {
		delete m_edges[i];
	}
	delete m_edgeShowingBuffer;
	delete m_roadData;
	delete m_edgeShowingBuffer3D;
}

void MapPolygonUnit::AddLegitimateCountry( Country* countryToAdd )
{
	if (std::find( m_legitimateCountries.begin(), m_legitimateCountries.end(), countryToAdd ) == m_legitimateCountries.end()) {
		m_legitimateCountries.push_back( countryToAdd );
	}
}

bool MapPolygonUnit::IsLegitimateToCountry( Country* country ) const
{
	for (auto legitimateCountry : m_legitimateCountries) {
		if (legitimateCountry == country) {
			return true;
		}
	}
	return false;
}

bool MapPolygonUnit::IsProvOnOwnerCountryBoarder() const
{
	for (auto prov : m_adjacentUnits) {
		if (prov->m_owner && prov->m_owner != m_owner) {
			return true;
		}
	}
	return false;
}

void MapPolygonUnit::InitializeUnit()
{
	// calculate rough radius (be the max distance from center to edge vertexes)
	AABB2 const& AABB2bounds = GetCurMap()->m_bounds;
	if (!m_isFarAwayFakeUnit) {
		float maxDist = 0.f;
		for (auto edge : m_edges) {
			if (AABB2bounds.IsPointInside( edge->m_startPos )) {
				float distSquared = GetDistanceSquared2D( edge->m_startPos, m_centerPosition );
				if (distSquared > maxDist) {
					maxDist = distSquared;
				}
			}
			else {
				float distSquared = GetDistanceSquared2D( AABB2bounds.GetNearestPoint( edge->m_startPos ), m_centerPosition );
				if (distSquared > maxDist) {
					maxDist = distSquared;
				}
			}
		}
		m_roughRadius = sqrtf( maxDist );
	}

	// make edges counter-clockwise
	if (m_edges.size() > 2) {
		for (auto edge : m_edges) {
			Vec2 cTos = edge->m_startPos - m_centerPosition;
			Vec2 cToe = edge->m_endPos - m_centerPosition;
			if (CrossProduct2D( cTos, cToe ) < 0.f) {
				Vec2 temp = edge->m_startPos;
				edge->m_startPos = edge->m_endPos;
				edge->m_endPos = temp;
			}
		}
	}

	// add adjacent polygons to list
	for (auto edge : m_edges) {
		if (edge->m_opposite && edge->m_opposite->m_owner) {
			if (!edge->m_opposite->m_owner->m_isFarAwayFakeUnit && (AABB2bounds.IsPointInside( edge->m_startPos ) || AABB2bounds.IsPointInside(edge->m_endPos)) 
				&& std::find( m_adjacentUnits.begin(), m_adjacentUnits.end(), edge->m_opposite->m_owner ) == m_adjacentUnits.end()) {
				m_adjacentUnits.push_back( edge->m_opposite->m_owner );
			}
		}
	}

	CalculateGeometricCenter();

	// calculate height (very important and basic move)
	Map* curMap = GetCurMap();
	// scale is how big a Perlin noise wave is, so if fragment factor is bigger, there will be more likely a big continent on the map,
	// and if the factor is smaller, there will be likely more fragmented islands on the map
	float scale = Minf( curMap->m_dimensions.x, curMap->m_dimensions.y ) * curMap->m_generationSettings.m_fragmentFactor;
	// compute the perlin noise by 2D position, set the number of octaves to be 9 to make the noise more irregular
	// higher persistence will lead to less and smaller mountains
	// higher octave scale will lead the land more shattered
	m_height = 0.5f * (1.f + Compute2dPerlinNoise( m_centerPosition.x, m_centerPosition.y, scale, 
		/*number of octaves*/5, /*octave persistence*/0.5f, /*octave scale*/2.f, /*renormalize*/true, /*seed*/curMap->m_heightSeed));
	// if do this, higher polygon will be more higher, lower polygon will be more lower, it is not real(coasts will be very narrow), 
	// so do not do this now
	//m_height = SmoothStep3( m_height );

	// let bounds' height goes down
	Vec2 const& bounds = curMap->m_dimensions;
	// percent in X width
	float tInX = m_centerPosition.x / bounds.x;
	// -             -
	//  -           -
	//   -         -
	//     -     -
	//       - - 
	// produce a curve like this, so the polygon nears the bounds receive the most height subtract
	float XtoSub = SmoothStart6( 2.f * Absf( tInX - 0.5f ) );
	// percent in Y width
	float tInY = m_centerPosition.y / bounds.y;
	float YtoSub = SmoothStart6( 2.f * Absf( tInY - 0.5f ) );

	float influenceOfBoundsEdges = Maxf( XtoSub, YtoSub );

	m_height = GetClamped( m_height - influenceOfBoundsEdges, 0.f, 1.f );

	// land height is the minimal height of the lands, lower than this value is set to sea, so a higher value means less lands
	float landHeight = GetCurMap()->m_generationSettings.m_landRichnessFactor;
	// land
	if (m_height > landHeight) {
		// normalize the value higher than land height to 0-1
		float landAltitude = RangeMapClamped( m_height, landHeight, 1.f, 0.f, 1.f );
		// do a smooth step so that coast plain and highland will be larger
		landAltitude = SmoothStep3( landAltitude );
		// range map to real height
		m_height = RangeMapClamped( landAltitude, 0.f, 1.f, curMap->m_seaLevel, curMap->m_generationSettings.m_maxHeight );
	}
	// sea
	else {
		// normalize the value lower than land height to 0-1
		float seaAltitude = RangeMapClamped( m_height, 0.f, landHeight, 0.f, 1.f );
		// the same as above
		seaAltitude = SmoothStep3( seaAltitude );
		// range map to real height
		m_height = RangeMapClamped( seaAltitude, 0.f, 1.f, curMap->m_generationSettings.m_minHeight, curMap->m_seaLevel );
	}

	float seaLevel = curMap->m_seaLevel;
	if (m_height < seaLevel) {
		m_landform = LandformType::Ocean;
	}
	else {
		m_landform = LandformType::Land;
	}

	// calculate longitude
	m_longitude = RangeMapClamped( tInX, 0.f, 1.f, -180.f, 180.f );
	// calculate latitude
	m_latitude = RangeMapClamped( tInY, 0.f, 1.f, 90.f, -90.f );
}

void MapPolygonUnit::InitializeUnit2()
{
	SetLake();
	SetIsland();
}

void MapPolygonUnit::InitializeUnit3() {
	CalculateSeaDistance();
	CalculateClimate();
	CalculatePrecipitation();
	CalculateTemperature();
	SetLandform();
}

void MapPolygonUnit::CalculateRawPopulation()
{
	// calculate area size
	for (auto edge : m_edges) {
		Vec2 side1 = edge->m_startPos - m_geoCenterPos;
		Vec2 side2 = edge->m_endPos - m_geoCenterPos;
		float area = std::abs( CrossProduct2D( side1, side2 ) );
		m_areaSize += area;
	}
	if (IsWater()) {
		return;
	}
	// calculate population base density
	Map* map = GetCurMap();
	float maxPopulationBase = 0.f;
	// landform influence - decides the max amount of population living here
	if (m_landform == LandformType::Desert) {
		maxPopulationBase = 0.2f;
	}
	else if (m_landform == LandformType::Grassland) {
		maxPopulationBase = 2.5f;
	}
	else if (m_landform == LandformType::Highland_Hill) {
		maxPopulationBase = 0.8f;
	}
	else if (m_landform == LandformType::Highland_Plain) {
		maxPopulationBase = 1.6f;
	}
	else if (m_landform == LandformType::Icefield) {
		maxPopulationBase = 0.f;
	}
	else if (m_landform == LandformType::Island) {
		maxPopulationBase = 1.2f;
	}
	else if (m_landform == LandformType::Lowland_Hill) {
		maxPopulationBase = 2.4f;
	}
	else if (m_landform == LandformType::Lowland_Plain) {
		maxPopulationBase = 3.2f;
	}
	else if (m_landform == LandformType::Marsh) {
		maxPopulationBase = 1.f;
	}
	else if (m_landform == LandformType::Mountain) {
		maxPopulationBase = 0.3f;
	}
	else if (m_landform == LandformType::Rainforest) {
		maxPopulationBase = 0.75f;
	}
	else if (m_landform == LandformType::Savanna) {
		maxPopulationBase = 1.2f;
	}
	else if (m_landform == LandformType::Subarctic_Forest) {
		maxPopulationBase = 0.6f;
	}
	else if (m_landform == LandformType::Subtropical_Forest) {
		maxPopulationBase = 1.75f;
	}
	else if (m_landform == LandformType::Temperate_Forest) {
		maxPopulationBase = 1.75f;
	}
	else if (m_landform == LandformType::Tundra) {
		maxPopulationBase = 0.1f;
	}

	// precipitation influence - too many or too few rains are not good for population growth
	float precipitationInfluence = 0.f;
	float avgRainYear = 0.5f * (m_summerPrecipitation + m_winterPrecipitation);
	if (avgRainYear < 500.f) {
		float normalizedAvgRain = RangeMapClamped( avgRainYear, 0.f, 500.f, 0.f, 1.f );
		float shiftedAvgRain = SmoothStop2( normalizedAvgRain );
		precipitationInfluence = RangeMapClamped( shiftedAvgRain, 0.f, 1.f, -0.6f, 0.5f );
	}
	else if (avgRainYear <= 1500.f) {
		float normalizedAvgRain = RangeMapClamped( avgRainYear, 500.f, 1500.f, 0.f, 1.f );
		float shiftedAvgRain = -4.f * (normalizedAvgRain - 0.5f) * (normalizedAvgRain - 0.5f) + 1.f;
		precipitationInfluence = RangeMapClamped( shiftedAvgRain, 0.f, 1.f, 0.5f, 1.3f );
	}
	else {
		float normalizedAvgRain = RangeMapClamped( avgRainYear, 1500.f, 4000.f, 0.f, 1.f );
		float shiftedAvgRain = 1.f - SmoothStart2( normalizedAvgRain );
		precipitationInfluence = RangeMapClamped( shiftedAvgRain, 0.f, 1.f, -0.6f, 0.5f );
	}


	// river influence - people would like to live near rivers
	float riverInfluence = 0.f;
	if (m_riverOnThis != nullptr) {
		riverInfluence += 1.f;
	}
	else {
		riverInfluence = -0.1f;
	}

	// height influence
	float heightInfluence = 0.f;
	if (m_height < 50.f) {
		heightInfluence -= 0.4f;
	}
	else if (m_height < 1000.f) {
		heightInfluence += 0.5f;
	}
	else if (m_height < 2000.f) {
		heightInfluence -= 0.2f;
	}
	else if (m_height < 3000.f) {
		heightInfluence -= 0.5f;
	}
	else {
		heightInfluence -= 1.f;
	}

	// temperature influence - too hot or too cold is not good for population growth
	float temperatureInfluence = 0.f;
	float avgTemp = 0.5f * (m_summerAvgTemperature + m_winterAvgTemperature);
	if (avgTemp < 10.f) {
		temperatureInfluence -= 0.3f;
	}
	else if (avgTemp < 14.f) {
		temperatureInfluence += 0.2f;
	}
	else if (avgTemp < 22.f) {
		temperatureInfluence += 0.5f;
	}
	else if (avgTemp < 24.f) {
		temperatureInfluence -= 0.1f;
	}
	else {
		temperatureInfluence -= 0.3f;
	}

	// random noise influence - add a random noise to this value
	float scale = Minf( map->m_dimensions.x, map->m_dimensions.y ) * map->m_generationSettings.m_fragmentFactor;
	float noiseInfluence = 0.3f * 0.5f * Compute2dPerlinNoise( m_centerPosition.x, m_centerPosition.y, scale, 5, 0.5f, 2.f, true, map->m_populationSeed );

	maxPopulationBase = GetClamped( maxPopulationBase, 0.01f, 10.f );
	float maxPopulation = (2000.f * maxPopulationBase * m_areaSize);

	float populationBase = 1.f + precipitationInfluence + riverInfluence + temperatureInfluence + noiseInfluence + heightInfluence;
	populationBase = GetClamped( populationBase, 0.01f, 10.f );
	float rawPopulation = (2000.f * populationBase * m_areaSize);

	// check 
	//if (rawPopulation > maxPopulation) {
	//	m_totalPopulation = -1.f;
	//}
	//else {
	m_totalPopulation = (int)Minf( rawPopulation, maxPopulation );
	//}
}

void MapPolygonUnit::CalculateLowerAdjacentUnits()
{
	for (auto unit : m_adjacentUnits) {
		if (unit->m_height < m_height) {
			m_adjLowerUnits.push_back( unit );
		}
	}
	for (auto unit : m_adjacentUnits) {
		if (unit->m_height >= m_height) {
			m_adjKindOfLowerUnits.push_back( unit );
		}
	}

	// if no lower neighbor, set self to lake
	// do not do this, make the map weird
	//if ((int)m_adjLowerUnits.size() == 0) {
	//	m_landform = LandformType::Lake;
	//}
	//std::sort( m_adjLowerUnits.begin(), m_adjLowerUnits.end(), []( MapPolygonUnit* a, MapPolygonUnit* b ) { return a->m_height < b->m_height; } );
}

void MapPolygonUnit::CalculateGeometricCenter()
{
	Vec2 totalPos = Vec2();
	Map* map = GetCurMap();
	AABB2 const& bounds = map->m_bounds;
	for (auto edge : m_edges) {
		if (bounds.IsPointInside( edge->m_startPos )) {
			totalPos += edge->m_startPos;
		}
		else {
			Vec2 startPos = edge->m_startPos;
			Vec2 endPos = edge->m_endPos;
			map->PM_ClampLinesIntoBounds( startPos, endPos );
			totalPos += startPos;
		}
	}
	m_geoCenterPos = totalPos / (float)m_edges.size();
	m_geoCenterPos.x = GetClamped( m_geoCenterPos.x, 0.f, GetCurMap()->m_bounds.m_maxs.x );
	m_geoCenterPos.y = GetClamped( m_geoCenterPos.y, 0.f, GetCurMap()->m_bounds.m_maxs.y );
}

float MapPolygonUnit::CalculateGeometricDistance( MapPolygonUnit* unit )
{
	std::vector<MapPolygonUnit*> route;
	std::map<size_t, MapPolygonUnit*> dirtyMap;
	GetOneRouteToProvince( route, this, unit );
	float totalDistance = 0.f;
	for (int i = 0; i < int(route.size() - 1); i++) {
		if (route[i]->IsWater() || route[i + 1]->IsWater()) {
			totalDistance += 4.f * GetDistance2D( route[i]->m_geoCenterPos, route[i + 1]->m_geoCenterPos );
		}
		else {
			float heightDifference = abs( route[i]->m_height - route[i + 1]->m_height );
			float heightInfluence = RangeMapClamped( heightDifference, 0.f, GetCurMap()->m_generationSettings.m_maxHeight, 1.f, 10.f );
			totalDistance += heightInfluence * GetDistance2D( route[i]->m_geoCenterPos, route[i + 1]->m_geoCenterPos );;
		}
	}
	return totalDistance;
}

struct ProvDFSStackObj {
	MapPolygonUnit* m_unit = nullptr;
	ProvDFSStackObj* m_parent = nullptr;
};

void MapPolygonUnit::GetOneRouteToProvince( std::vector<MapPolygonUnit*>& out_route, MapPolygonUnit* startUnit, MapPolygonUnit* destUnit )
{
	// We use A star for now
	Map* map = GetCurMap();
	map->m_aStarHelper.CalculateRoute( startUnit, destUnit, out_route );
	// BFS
	/*std::map<size_t, MapPolygonUnit*> dirtyMap;
	std::deque<ProvDFSStackObj> queue;
	queue.push_back( ProvDFSStackObj( startUnit, nullptr ) );
	MapPolygonUnit* curUnit = nullptr;
	int frontIndex = 0;
	while (frontIndex < queue.size()) {
		curUnit = queue[frontIndex].m_unit;
		if (curUnit == destUnit) {
			break;
		}
		else {
			++frontIndex;
			for (auto adjUnit : curUnit->m_adjacentUnits) {
				if (!adjUnit->IsWater() && dirtyMap.find( (size_t)adjUnit ) == dirtyMap.end()) {
					dirtyMap[(size_t)adjUnit] = adjUnit;
					queue.push_back( ProvDFSStackObj( adjUnit, &queue.back() ) );
				}
			}
		}
	}
	if (frontIndex == queue.size()) {
		dirtyMap.clear();
		queue.clear();
		queue.push_back( ProvDFSStackObj( startUnit, nullptr ) );
		curUnit = nullptr;
		frontIndex = 0;
		while (frontIndex < queue.size()) {
			curUnit = queue[frontIndex].m_unit;
			if (curUnit == destUnit) {
				break;
			}
			else {
				++frontIndex;
				for (auto adjUnit : curUnit->m_adjacentUnits) {
					if (dirtyMap.find( (size_t)adjUnit ) == dirtyMap.end()) {
						dirtyMap[(size_t)adjUnit] = adjUnit;
						queue.push_back( ProvDFSStackObj( adjUnit, &queue.back() ) );
					}
				}
			}
		}
	}
	ProvDFSStackObj* iter = &queue[frontIndex];
	while (iter != nullptr) {
		out_route.push_back( iter->m_unit );
		iter = iter->m_parent;
	}*/
}

void MapPolygonUnit::SetRenderNoColor()
{
	if (m_curColor != Rgba8::WHITE) {
		SetColorOfPolygonFace( Rgba8::WHITE );
	}

}

void MapPolygonUnit::SetRenderHeightColor()
{
	Map* map = GetCurMap();
	float seaLevel = map->m_seaLevel;
	if (IsWater()) {
		SetColorOfPolygonFace( Rgba8::Interpolate( map->m_renderPreference.m_lowestOceanHeightColor, map->m_renderPreference.m_highestOceanHeightColor, RangeMapClamped( m_height, GetCurMap()->m_generationSettings.m_minHeight, seaLevel, 0.f, 1.f ) ) );
	}
	else {
		SetColorOfPolygonFace( Rgba8::Interpolate( map->m_renderPreference.m_lowestLandHeightColor, map->m_renderPreference.m_highestLandHeightColor, RangeMapClamped( m_height, seaLevel, GetCurMap()->m_generationSettings.m_maxHeight, 0.f, 1.f ) ) );
	}
}

void MapPolygonUnit::SetRenderClimateColor()
{
	if (IsOcean()) {
		Map* map = GetCurMap();
		SetColorOfPolygonFace( map->m_renderPreference.m_oceanColor );
		return;
	}
	SetColorOfPolygonFace( GetCurMap()->m_renderPreference.m_climateColorMap[(int)m_climate] );
}

void MapPolygonUnit::SetRenderPrecipitationColor( bool summer )
{
	Map* map = GetCurMap();
	if (IsWater()) {
		SetColorOfPolygonFace( map->m_renderPreference.m_oceanColor );
		return;
	}
	if (summer) {
		if (m_latitude < 0.f) {
			SetColorOfPolygonFace( Rgba8::Interpolate( map->m_renderPreference.m_lowestPrecipitationColor, map->m_renderPreference.m_highestPrecipitationColor, RangeMapClamped( m_summerPrecipitation, 0.f, 2000.f, 0.f, 1.f ) ) );
		}
		else {
			SetColorOfPolygonFace( Rgba8::Interpolate( map->m_renderPreference.m_lowestPrecipitationColor, map->m_renderPreference.m_highestPrecipitationColor, RangeMapClamped( m_winterPrecipitation, 0.f, 2000.f, 0.f, 1.f ) ) );
		}
	}
	else {
		if (m_latitude < 0.f) {
			SetColorOfPolygonFace( Rgba8::Interpolate( map->m_renderPreference.m_lowestPrecipitationColor, map->m_renderPreference.m_highestPrecipitationColor, RangeMapClamped( m_winterPrecipitation, 0.f, 2000.f, 0.f, 1.f ) ) );

		}
		else {
			SetColorOfPolygonFace( Rgba8::Interpolate( map->m_renderPreference.m_lowestPrecipitationColor, map->m_renderPreference.m_highestPrecipitationColor, RangeMapClamped( m_summerPrecipitation, 0.f, 2000.f, 0.f, 1.f ) ) );

		}
	}
}

void MapPolygonUnit::SetRenderTemperatureColor( bool summer )
{
	Map* map = GetCurMap();
	if (IsWater()) {
		SetColorOfPolygonFace( map->m_renderPreference.m_oceanColor );
		return;
	}
	if (summer) {
		if (m_latitude < 0.f) {
			SetColorOfPolygonFace( Rgba8::Interpolate( map->m_renderPreference.m_lowestTemperatureColor, map->m_renderPreference.m_highestTemperatureColor, RangeMapClamped( m_summerAvgTemperature, -10.f, 30.f, 0.f, 1.f ) ) );

		}
		else {
			SetColorOfPolygonFace( Rgba8::Interpolate( map->m_renderPreference.m_lowestTemperatureColor, map->m_renderPreference.m_highestTemperatureColor, RangeMapClamped( m_winterAvgTemperature, -10.f, 30.f, 0.f, 1.f ) ) );

		}
	}
	else {
		if (m_latitude < 0.f) {
			SetColorOfPolygonFace( Rgba8::Interpolate( map->m_renderPreference.m_lowestTemperatureColor, map->m_renderPreference.m_highestTemperatureColor, RangeMapClamped( m_winterAvgTemperature, -10.f, 30.f, 0.f, 1.f ) ) );
		}
		else {
			SetColorOfPolygonFace( Rgba8::Interpolate( map->m_renderPreference.m_lowestTemperatureColor, map->m_renderPreference.m_highestTemperatureColor, RangeMapClamped( m_summerAvgTemperature, -10.f, 30.f, 0.f, 1.f ) ) );
		}
	}
}

void MapPolygonUnit::SetRenderViewingColor( bool isViewing )
{
	if (isViewing) {
		m_colorBeforeClick = m_curColor;
		SetColorOfPolygonFace( Rgba8(
			(unsigned char)GetClamped( m_curColor.r + 50, 0, 255 ),
			(unsigned char)GetClamped( m_curColor.g + 50, 0, 255 ),
			(unsigned char)GetClamped( m_curColor.b + 50, 0, 255 ),
			m_curColor.a ) );
	}
	else {
		SetColorOfPolygonFace( Rgba8(
			(unsigned char)GetClamped( m_colorBeforeClick.r, 0, 255 ),
			(unsigned char)GetClamped( m_colorBeforeClick.g, 0, 255 ),
			(unsigned char)GetClamped( m_colorBeforeClick.b, 0, 255 ),
			m_colorBeforeClick.a ) );
	}
}

void MapPolygonUnit::SetRenderLandformColor()
{
	SetColorOfPolygonFace( GetCurMap()->m_renderPreference.m_landformColorMap[(int)m_landform] );
}

void MapPolygonUnit::SetRenderPopulationColor()
{
	Map* map = GetCurMap();
	if (IsWater()) {
		SetColorOfPolygonFace( map->m_renderPreference.m_oceanColor );
	}
	else {
		SetColorOfPolygonFace( Rgba8::Interpolate( map->m_renderPreference.m_lowestPopulationColor, map->m_renderPreference.m_highestPopulationColor, RangeMapClamped( (float)m_totalPopulation, 0.f, 80000.f, 0.f, 1.f ) ) );
		//SetColorOfPolygonFace( Rgba8::Interpolate( Rgba8( 90, 25, 50 ), Rgba8( 50, 255, 50 ), RangeMapClamped( (float)m_totalPopulation, 0.f, 30000.f, 0.f, 1.f ) ) );
	}
}

void MapPolygonUnit::SetRenderCultureColor()
{
	if (IsWater()) {
		SetColorOfPolygonFace( GetCurMap()->m_renderPreference.m_oceanColor );
	}
	else if (m_majorCulture) {
		SetColorOfPolygonFace( m_majorCulture->m_color );
	}
	else {
		SetColorOfPolygonFace( Rgba8( 0, 0, 0 ) );
	}
}

void MapPolygonUnit::SetRenderReligionColor()
{
	if (IsWater()) {
		SetColorOfPolygonFace( GetCurMap()->m_renderPreference.m_oceanColor );
	}
	else if (m_majorCulture) {
		SetColorOfPolygonFace( m_majorReligion->m_color );
	}
	else {
		SetColorOfPolygonFace( Rgba8( 0, 0, 0 ) );
	}
}

void MapPolygonUnit::SetRenderContinentColor()
{
	if (m_continent) {
		SetColorOfPolygonFace( m_continent->m_color );
	}
	else {
		SetColorOfPolygonFace( Rgba8( 48, 48, 48 ) );
	}
}

void MapPolygonUnit::SetRenderProductColor()
{
	if (m_productType == ProductType::None) {
		SetColorOfPolygonFace( Rgba8( 48, 48, 48 ) );
	}
	else {
		SetColorOfPolygonFace( GetCurMap()->m_renderPreference.m_productColorMap[(int)m_productType] );
	}
}

void MapPolygonUnit::SetRenderCountryColor()
{
	if (m_owner == nullptr) {
		SetColorOfPolygonFace( GetCurMap()->m_renderPreference.m_oceanColor );
	}
	else {
		SetColorOfPolygonFace( m_owner->m_color );
	}
}

void MapPolygonUnit::SetRenderRegionColor()
{
	if (m_region == nullptr) {
		SetColorOfPolygonFace( GetCurMap()->m_renderPreference.m_oceanColor );
	}
	else {
		SetColorOfPolygonFace( m_region->m_color );
	}
}

void MapPolygonUnit::SetRenderRelationColor()
{
	if (IsWater()) {
		SetColorOfPolygonFace( GetCurMap()->m_renderPreference.m_oceanColor );
		return;
	}
	MapPolygonUnit* unit = GetCurMap()->m_curViewingUnit;
	CountryRelationType relation = CountryRelationType::None;
	if (unit && m_owner && unit->m_owner) {
		if (unit->m_owner == m_owner && !IsLegitimateToCountry(m_owner)) {
			SetColorOfPolygonFace( GetCurMap()->m_renderPreference.m_invalidProvince );
			return;
		}
		relation = m_owner->GetRelationTo( unit->m_owner );
	}
	
	SetColorOfPolygonFace( GetCurMap()->m_renderPreference.m_relationColorMap[(int)relation] );
}

void MapPolygonUnit::SetRenderProvEditColor()
{
	if (m_owner == nullptr) {
		SetColorOfPolygonFace( GetCurMap()->m_renderPreference.m_oceanColor );
	}
	else {
		SetColorOfPolygonFace( m_owner->m_color );
	}
}

void MapPolygonUnit::SetColorOfPolygonFace( Rgba8 color )
{
	for (size_t i = m_startInVertexBuffer; i < m_startInVertexBuffer + m_sizeInVertexBuffer; i++) {
		GetCurMap()->m_colorVertexArray[i] = color;
	}
	m_curColor = color;
	//m_isColorDirty = true;
}

float MapPolygonUnit::EvaluateProvinceArmyScore()
{
	float value = 0.f;
	float populationValue = SmoothStop2( RangeMapClamped( (float)m_totalPopulation, 0.f, 40000.f, 0.f, 1.f ) );
	value += populationValue;
	value += (int)m_cities.size() * 0.5f;
	value += (int)m_towns.size() * 0.1f;
	if (m_majorCulture != m_owner->m_countryCulture) {
		value += populationValue;
	}
	if (m_majorReligion != m_owner->m_countryReligion) {
		value += populationValue;
	}
	if (IsCapitalProv()) {
		value *= 2.f;
	}
	m_provinceArmyScore = value;
	return value;
}

bool MapPolygonUnit::IsOcean() const
{
	return m_landform == LandformType::Ocean;
}

bool MapPolygonUnit::IsLand() const
{
	return !IsWater();
}

bool MapPolygonUnit::IsWater() const
{
	return m_landform == LandformType::Ocean || m_landform == LandformType::Lake;
}

bool MapPolygonUnit::IsCapitalProv() const
{
	if (m_owner && m_owner->m_capitalProv == this) {
		return true;
	}
	return false;
}

bool MapPolygonUnit::IsPointInsideConvexPolygon( Vec2 const& point ) const
{
	if ((int)m_edges.size() <= 2) {
		return false;
	}

	for (int i = 0; i < (int)m_edges.size(); i++) {
		Vec2 pTos = m_edges[i]->m_startPos - point;
		Vec2 pToe = m_edges[i]->m_endPos - point;
		if (CrossProduct2D( pTos, pToe ) < 0.f) {
			return false;
		}
	}
	return true;
}

bool MapPolygonUnit::IsPointInsideConcavePolygon( Vec2 const& point ) const
{
	if ((int)m_edges.size() <= 2) {
		return false;
	}

	double pointX = point.x;
	double pointY = point.y;
	int count = 0;
	for (auto& edge : m_edges) {
		if (edge->m_noisyEdges.size() >= 2) {
			for (int i = 0; i < (int)(edge->m_noisyEdges.size() - 1); i++) {
				double p1x = edge->m_noisyEdges[i].x;
				double p1y = edge->m_noisyEdges[i].y;
				double p2x = edge->m_noisyEdges[i + 1].x;
				double p2y = edge->m_noisyEdges[i + 1].y;
				if ((pointY < p1y != pointY < p2y) && (pointX < p1x + (pointY - p1y) / (p2y - p1y) * (p2x - p1x))) {
					++count;
				}
			}
		}
		else {
			double p1x = edge->m_startPos.x;
			double p1y = edge->m_startPos.y;
			double p2x = edge->m_endPos.x;
			double p2y = edge->m_endPos.y;
			if ((pointY < p1y != pointY < p2y) && (pointX < p1x + (pointY - p1y) / (p2y - p1y) * (p2x - p1x))) {
				++count;
			}
		}
	}

	return count % 2 == 1;
}

Vec2 MapPolygonUnit::GetRandomPointNearCenter() const
{
	/*	Vec2 point;
	float length;
	do {
		point.x = GetCurMap()->m_mapRNG->RollRandomFloatZeroToOne() * 2.f - 1.f;
		point.y = GetCurMap()->m_mapRNG->RollRandomFloatZeroToOne() * 2.f - 1.f;
		length = point.GetLengthSquared();
	} while (length > 1.f);
	return point * m_roughRadius * 0.3f + m_centerPosition;
	*/
	// return the center now
	return m_geoCenterPos;
}

bool MapPolygonUnit::IsAdjacentToUnit( MapPolygonUnit* theOtherUnit ) const
{
	if (theOtherUnit->m_adjacentUnits.size() > m_adjacentUnits.size()) {
		for (auto unit : m_adjacentUnits) {
			if (unit == theOtherUnit) {
				return true;
			}
		}
	}
	else {
		for (auto unit : theOtherUnit->m_adjacentUnits) {
			if (unit == this) {
				return true;
			}
		}
	}
	return false;
}

bool MapPolygonUnit::IsAdjacentToCountry( Country* country ) const
{
	for (auto unit : m_adjacentUnits) {
		if (unit->m_owner == country) {
			return true;
		}
	}
	return false;
}

void MapPolygonUnit::HD_AddCultureInfluence( Culture* cultureToAdd )
{
	// if there is currently no culture influence in this province, just convert it into this culture
	// push back a culture-influence pair
	if (m_cultures.size() == 0) {
		m_cultures.push_back( std::pair<Culture*, float>( cultureToAdd, 1.f ) );
		m_majorCulture = cultureToAdd;
		return;
	}
	// get the total population of this culture in adjacent provinces
	float allInfluence = 0.f;
	for (auto unit : m_adjacentUnits) {
		allInfluence += (unit->HD_GetCultureInfluence( cultureToAdd ) * unit->m_totalPopulation);
	}
	// modifier: influence ability
	allInfluence *= cultureToAdd->m_influence;
	// safe divide
	if (allInfluence == 0.f && m_totalPopulation == 0.f) {
		allInfluence = 0.f;
	}
	else {
		allInfluence /= m_totalPopulation;
	}
	
	float finalInfluence = 0.f;
	// affect each culture in the province
	for (auto& pair : m_cultures) {
		if (pair.first != cultureToAdd) { // do not affect the same culture
			float rawInfluence = 0.f;
			if (allInfluence == 0.f && pair.second == 0.f) {
				rawInfluence = 0.f; // safe division, do not produce nan
			}
			else {
				rawInfluence = allInfluence / pair.second / pair.first->m_influence;
			}
			// convert division result to percentage (0 - 1)
			float thisCultureInfluence = RangeMapClamped( rawInfluence, 0.f, 5.f, 0.f, 0.3f );
			// culture is hard to enter in complex terrain
			if (m_landform == LandformType::Desert || m_landform == LandformType::Icefield || m_landform == LandformType::Mountain
				|| m_landform == LandformType::Rainforest || m_landform == LandformType::Tundra) {
				thisCultureInfluence *= 0.2f;
			}
			// culture is hard to enter in low population area
			if (m_totalPopulation <= 3000) {
				thisCultureInfluence *= 0.1f;
			}
			float splitValue = pair.second * thisCultureInfluence;
			pair.second -= splitValue;
			finalInfluence += splitValue;
		}
	}
	// if the final result influence is too low, it is assimilated to the dominant culture
	if (finalInfluence <= 0.001f) {
		if (m_cultures.size() > 0) {
			m_cultures[0].second += finalInfluence;
		}
		return;
	}
	// add the total influence this culture get this turn to this culture
	for (auto& pair : m_cultures) {
		if (pair.first == cultureToAdd) {
			pair.second += finalInfluence;
			//RecalculateMajorCulture();
			return;
		}
	}
	// if the province has no this culture influence, push back a culture-influence pair
	m_cultures.push_back( std::pair<Culture*, float>( cultureToAdd, finalInfluence ) );
	//RecalculateMajorCulture();
}

void MapPolygonUnit::RecalculateMajorCulture()
{
	std::sort( m_cultures.begin(), m_cultures.end(), []( std::pair<Culture*, float> const& a, std::pair<Culture*, float> const& b ) { return a.second > b.second; } );
	if (m_cultures.size() > 0) {
		m_majorCulture = m_cultures[0].first;
	}
}

float MapPolygonUnit::HD_GetCultureInfluence( Culture* culture )
{
	for (auto& pair : m_cultures) {
		if (pair.first == culture) {
			return pair.second;
		}
	}
	return 0.f;
}

void MapPolygonUnit::HD_AddReligionInfluence( Religion* religionToAdd )
{
	if (m_religions.size() == 0) {
		m_religions.push_back( std::pair<Religion*, float>( religionToAdd, 1.f ) );
		m_majorReligion = religionToAdd;
		return;
	}
	float allInfluence = 0.f;
	for (auto unit : m_adjacentUnits) {
		float thisInfluence = (unit->HD_GetReligionInfluence( religionToAdd ) * unit->m_totalPopulation);
		if (unit->m_majorCulture != m_majorCulture) {
			thisInfluence *= 0.1f;
		}
		else {
			thisInfluence *= 1.2f;
		}
		allInfluence += thisInfluence;
	}
	allInfluence *= religionToAdd->m_influence;
	if (allInfluence == 0.f && m_totalPopulation == 0.f) {
		allInfluence = 0.f;
	}
	else {
		allInfluence /= m_totalPopulation;
	}

	float finalInfluence = 0.f;
	for (auto& pair : m_religions) {
		if (pair.first != religionToAdd) {
			float rawInfluence = 0.f;
			if (allInfluence == 0.f && pair.second == 0.f) {
				rawInfluence = 0.f;
			}
			else {
				rawInfluence = allInfluence / pair.second / pair.first->m_influence;
			}
			float thisReligionInfluence = RangeMapClamped( rawInfluence, 0.f, 5.f, 0.f, 0.8f );
			if (m_landform == LandformType::Desert || m_landform == LandformType::Icefield || m_landform == LandformType::Mountain
				|| m_landform == LandformType::Rainforest || m_landform == LandformType::Tundra) {
				thisReligionInfluence *= 0.1f;
			}
			if (m_totalPopulation <= 3000) {
				thisReligionInfluence *= 0.1f;
			}
			float splitValue = pair.second * thisReligionInfluence;
			pair.second -= splitValue;
			finalInfluence += splitValue;
		}
	}
	if (finalInfluence <= 0.001f) {
		if (m_religions.size() > 0) {
			m_religions[0].second += finalInfluence;
		}
		return;
	}
	for (auto& pair : m_religions) {
		if (pair.first == religionToAdd) {
			pair.second += finalInfluence;
			//RecalculateMajorCulture();
			return;
		}
	}
	m_religions.push_back( std::pair<Religion*, float>( religionToAdd, finalInfluence ) );
	//RecalculateMajorCulture();
}

void MapPolygonUnit::RecalculateMajorReligion()
{
	std::sort( m_religions.begin(), m_religions.end(), []( std::pair<Religion*, float> const& a, std::pair<Religion*, float> const& b ) { return a.second > b.second; } );
	if (m_religions.size() > 0) {
		m_majorReligion = m_religions[0].first;
	}
}

float MapPolygonUnit::HD_GetReligionInfluence( Religion* religion )
{
	for (auto& pair : m_religions) {
		if (pair.first == religion) {
			return pair.second;
		}
	}
	return 0.f;
}

void MapPolygonUnit::HD_GenerateCities( bool forceOperation/*=false*/, int tryAmountOverride/*=0*/ )
{
	if (IsWater()) {
		return;
	}
	Map* map = GetCurMap();
	int cityTryAmount = 0;
	if (tryAmountOverride != 0) {
		cityTryAmount = tryAmountOverride;
	}
	else {
		cityTryAmount = RoundDownToInt( 10.f * map->m_generationSettings.m_cityRichness ); // get the max try times
	}
	float noiseScale = 4.f * Maxf( map->m_dimensions.x, map->m_dimensions.y ) / map->m_generationSettings.m_basePolygons;

	// first calculate these scores because they are same in the same province
	// calculate the terrain score
	float terrainScore = 0.f;
	if (m_landform == LandformType::Lowland_Plain) {
		terrainScore += 0.3f;
	}
	else if (m_landform == LandformType::Highland_Plain || m_landform == LandformType::Lowland_Hill) {
		terrainScore += 0.2f;
	}
	else if (m_landform == LandformType::Grassland || m_landform == LandformType::Temperate_Forest || m_landform == LandformType::Subtropical_Forest) {
		terrainScore = 0.1f;
	}
	else if (m_landform == LandformType::Desert || m_landform == LandformType::Icefield || m_landform == LandformType::Mountain) {
		terrainScore = -0.3f;
	}
	else if (m_landform == LandformType::Subarctic_Forest || m_landform == LandformType::Rainforest || m_landform == LandformType::Tundra || m_landform == LandformType::Marsh ) {
		terrainScore = -0.1f;
	}
	// calculate the population score, if it's crowded, there will more likely be a city
	float populationScore = 0.f;
	if (m_totalPopulation > 20000.f) {
		populationScore = 0.3f;
	}
	if (m_height >= 3000.f) {
		terrainScore -= 1.f;
	}
	else if (m_height >= 2000.f) {
		terrainScore -= 0.6f;
	}
	// initialize the counter of cities generated to reduce too many cities generated in one province
	int numOfCitiesGenerated = 0;
	float reduceChangeEachCity = 2.f / (float)cityTryAmount;
	for (int i = 0; i < cityTryAmount; i++) {
		// roll a random position inside rough disc range
		Vec2 cityTestPos = map->MP_GetRandomPointInDisc2D( m_centerPosition, m_roughRadius );
		if (IsPointInsideConcavePolygon( cityTestPos )) { // if the point is actually inside the polygon
			// calculate the score of this try
			float totalScore = 0.1f;
			// calculate the random noise score
			float noiseScore = Compute2dPerlinNoise( cityTestPos.x, cityTestPos.y, noiseScale, 9, 0.5f, 2.f, true, map->m_citySeed );
			totalScore += noiseScore;
			bool nearRiver = false, nearSea = false; // set the city flag if successfully generate a city
			// calculate the distance to the river in this province, near to river gets more score
			if (m_riverOnThis && m_roughRadius > 0.f) {
				float distToRiver = m_riverOnThis->GetDistanceToRiver( cityTestPos );
				float riverScore = 1.5f * SmoothStart4( 1.f - GetClamped( distToRiver / m_roughRadius * 2.f, 0.f, 1.f ) );
				if (riverScore > 0.4f) {
					nearRiver = true;
				}
				totalScore += riverScore;
			}
			// calculate the distance to the sea
			if (m_isCoast && m_roughRadius > 0.f) {
				float distToSea = GetDistanceToSeaFromThisUnit( cityTestPos );
				float seaScore = SmoothStart4( 1.f - GetClamped( distToSea / m_roughRadius, 0.f, 1.f ) );
				if (seaScore > 0.5f) {
					nearSea = true;
				}
				totalScore += seaScore;
			}
			totalScore += terrainScore;
			totalScore += populationScore;
			totalScore -= (float)(numOfCitiesGenerated * reduceChangeEachCity);
			
			if (totalScore > 0.8f || (forceOperation && i == cityTryAmount - 1)) {
				++numOfCitiesGenerated;
				City* newCity = new City();
				newCity->m_provIn = this;
				newCity->m_position = cityTestPos;
				newCity->m_id = (int)map->m_cities.size();
				newCity->m_iconBounds.m_mins = cityTestPos - Vec2( CITY_ICON_SIDE_LENGTH, CITY_ICON_SIDE_LENGTH );
				newCity->m_iconBounds.m_maxs = cityTestPos + Vec2( CITY_ICON_SIDE_LENGTH, CITY_ICON_SIDE_LENGTH );
				newCity->m_biggerIconBounds.m_mins = cityTestPos - Vec2( CITY_ICON_SIDE_LENGTH, CITY_ICON_SIDE_LENGTH ) * 1.5f;
				newCity->m_biggerIconBounds.m_maxs = cityTestPos + Vec2( CITY_ICON_SIDE_LENGTH, CITY_ICON_SIDE_LENGTH ) * 1.5f;
				if (nearRiver) newCity->AddAttribute(CityAttribute::AdjToRiver);
				if (nearSea) newCity->AddAttribute(CityAttribute::AdjToSea);
				map->m_cities.push_back( newCity );
				m_cities.push_back( newCity );
			}
		}
		else {
			--i; // this do not count, we need at least some valid tests
		}
	}
}

void MapPolygonUnit::HD_GenerateTowns()
{
	if (IsWater()) {
		return;
	}
	Map* map = GetCurMap();
	int townTryAmount = RoundDownToInt( 10.f * map->m_generationSettings.m_townRichness ); // get the max try times
	float noiseScale = 4.f * Maxf( map->m_dimensions.x, map->m_dimensions.y ) / map->m_generationSettings.m_basePolygons;

	// first calculate these scores because they are same in the same province
	// calculate the terrain score
	float terrainScore = 0.f;
	if (m_landform == LandformType::Lowland_Plain) {
		terrainScore += 0.3f;
	}
	else if (m_landform == LandformType::Highland_Plain || m_landform == LandformType::Lowland_Hill) {
		terrainScore += 0.2f;
	}
	else if (m_landform == LandformType::Grassland || m_landform == LandformType::Temperate_Forest || m_landform == LandformType::Subtropical_Forest) {
		terrainScore = 0.1f;
	}
	else if (m_landform == LandformType::Desert || m_landform == LandformType::Icefield || m_landform == LandformType::Mountain) {
		terrainScore = -0.1f;
	}
	// calculate the population score, if it's crowded, there will more likely be a city
	float populationScore = 0.f;
	if (m_totalPopulation > 10000.f) {
		populationScore = 0.1f;
	}
	if (m_height >= 3000.f) {
		terrainScore -= 0.6f;
	}
	else if (m_height >= 2000.f) {
		terrainScore -= 0.2f;
	}
	// initialize the counter of towns generated to reduce too many towns generated in one province
	int numOfTownsGenerated = 0;
	float reduceChangeEachCity = 2.f / (float)townTryAmount;
	for (int i = 0; i < townTryAmount; i++) {
		// roll a random position inside rough disc range
		Vec2 townTestPos = map->MP_GetRandomPointInDisc2D( m_centerPosition, m_roughRadius );
		if (IsPointInsideConcavePolygon( townTestPos )) { // if the point is actually inside the polygon
			// calculate the score of this try
			float totalScore = 0.4f;
			// calculate the random noise score
			float noiseScore = Compute2dPerlinNoise( townTestPos.x, townTestPos.y, noiseScale, 9, 0.5f, 2.f, true, map->m_townSeed );
			totalScore += noiseScore;
			// calculate the distance to the river in this province, near to river gets more score
			if (m_riverOnThis && m_roughRadius > 0.f) {
				float distToRiver = m_riverOnThis->GetDistanceToRiver( townTestPos );
				float riverScore = SmoothStart4( 1.f - GetClamped( distToRiver / m_roughRadius, 0.f, 1.f ) );
				totalScore += riverScore;
			}
			// calculate the distance to the sea
			if (m_isCoast && m_roughRadius > 0.f) {
				float distToSea = GetDistanceToSeaFromThisUnit( townTestPos );
				float seaScore = SmoothStart4( 1.f - GetClamped( distToSea / m_roughRadius, 0.f, 1.f ) );
				totalScore += seaScore;
			}
			// calculate the distance to the city, too near or too far away decrease the score, or increase the score
			float cityScore = -0.25f;
			if (m_roughRadius > 0.f) {
				float nearestDist = FLT_MAX;
				for (auto city : m_cities) {
					float distanceToCity = GetDistance2D( city->m_position, townTestPos );
					if (distanceToCity < nearestDist) {
						nearestDist = distanceToCity;
						cityScore = 0.75f - 2.f * abs( GetClamped( distanceToCity / m_roughRadius * 2.f, 0.f, 1.f ) - 0.5f );
					}
				}
			}
			totalScore += cityScore;
			totalScore += terrainScore;
			totalScore += populationScore;
			totalScore -= (float)(numOfTownsGenerated * reduceChangeEachCity);
			if (totalScore > 0.8f) {
				++numOfTownsGenerated;
				Town* newTown = new Town();
				newTown->m_provIn = this;
				newTown->m_position = townTestPos;
				newTown->m_id = (int)map->m_towns.size();
				newTown->m_iconBounds.m_mins = townTestPos - Vec2( TOWN_ICON_SIDE_LENGTH, TOWN_ICON_SIDE_LENGTH );
				newTown->m_iconBounds.m_maxs = townTestPos + Vec2( TOWN_ICON_SIDE_LENGTH, TOWN_ICON_SIDE_LENGTH );
				newTown->m_biggerIconBounds.m_mins = townTestPos - Vec2( TOWN_ICON_SIDE_LENGTH, TOWN_ICON_SIDE_LENGTH ) * 1.5f;
				newTown->m_biggerIconBounds.m_maxs = townTestPos + Vec2( TOWN_ICON_SIDE_LENGTH, TOWN_ICON_SIDE_LENGTH ) * 1.5f;
				map->m_towns.push_back( newTown );
				m_towns.push_back( newTown );
			}
		}
		else {
			--i; // this do not count, we need at least some valid tests
		}
	}
}

void MapPolygonUnit::CalculateProduct()
{
	if (IsWater()) {
		return;
	}
	Map* map = GetCurMap();

	// special products
	// tea and coffee
	if ((m_climate == ClimateType::TropicalMonsoon || m_climate == ClimateType::HumidSubtropical || m_climate == ClimateType::TropicalRainforest)
		&& m_height > 500.f) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.2f) {
			if (m_continent && m_continent->m_eastContinent) {
				m_productType = ProductType::Tea;
			}
			else {
				m_productType = ProductType::Coffee;
			}
			return;
		}
	}

	// wet and hot places
	if (abs( m_latitude ) < 30.f && m_summerPrecipitation > 800.f && m_climate != ClimateType::TropicalSavanna) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.15f) {
			m_productType = ProductType::Spice;
			return;
		}
	}

	// if is coast, 20% chance to be fish
	if (m_isCoast) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.3f) {
			m_productType = ProductType::Fish;
			return;
		}
		else if (rnd < 0.4f) {
			m_productType = ProductType::Salt;
			return;
		}
	}

	if (m_continent && m_continent->m_newContinent && abs( m_latitude ) < 40.f && m_height < 500.f) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.3f) {
			m_productType = ProductType::Tobacco;
			return;
		}
	}
	
	/* Grain,Fruit,Wine,Sugar,Salt,Wax,Fur,Wood,Cotton,
	Livestock,Wool,Tobacco*/
	// mostly decided by the landform\temperature\precipitation\continent
	if (m_landform == LandformType::Desert) { // desert can only have some poor products
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.02f) { // a little chance to be gold
			if (m_continent && m_continent->m_goldInDesertFlag) {
				m_productType = ProductType::Gold;
			}
			else {
				if (rnd < 0.01f) {
					m_productType = ProductType::Iron;
				}
				else {
					m_productType = ProductType::Copper;
				}
			}
		}
		else if (rnd < 0.07f) {
			m_productType = ProductType::Ivory;
		}
		else if (rnd < 0.35f) {
			m_productType = ProductType::Wool;
		}
		else if (rnd < 0.8f) {
			m_productType = ProductType::Livestock;
		}
		else {
			m_productType = ProductType::Salt;
		}
	}
	else if (m_landform == LandformType::Grassland) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.2f) {
			m_productType = ProductType::WarHorse;
		}
		else if (rnd < 0.25f) {
			m_productType = ProductType::Grain;
		}
		else if (rnd < 0.35f) {
			if (m_climate == ClimateType::MediterraneanClimate || m_climate == ClimateType::Oceanic) {
				m_productType = ProductType::Wine;
			}
			else {
				m_productType = ProductType::Fruit;
			}
		}
		else if (rnd < 0.4f) {
			m_productType = ProductType::Cotton;
		}
		else if (rnd < 0.5f) {
			CalculateMetalProduct();
		}
		else if (rnd < 0.7f) {
			m_productType = ProductType::Livestock;
		}
		else if (rnd < 0.9f) {
			m_productType = ProductType::Wool;
		}
		else {
			CalculateArtificialProduct();
		}
	}
	else if (m_landform == LandformType::Highland_Hill) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.1f) {
			m_productType = ProductType::Fur;
		}
		else if (rnd < 0.25f) {
			m_productType = ProductType::Gem;
		}
		else if (rnd < 0.75f) {
			CalculateMetalProduct();
		}
		else if (rnd < 0.94f) {
			m_productType = ProductType::Wool;
		}
		else {
			m_productType = ProductType::Livestock;
		}
	}
	else if (m_landform == LandformType::Highland_Plain) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.1f) {
			m_productType = ProductType::Fur;
		}
		else if (rnd < 0.2f) {
			m_productType = ProductType::Grain;
		}
		else if (rnd < 0.3f) {
			m_productType = ProductType::Cotton;
		}
		else if (rnd < 0.45f) {
			CalculateMetalProduct();
		}
		else if (rnd < 0.6f) {
			CalculateArtificialProduct();
		}
		else if (rnd < 0.75f) {
			m_productType = ProductType::Wool;
		}
		else if (rnd < 0.9f) {
			m_productType = ProductType::Livestock;
		}
		else {
			m_productType = ProductType::Salt;
		}
	}
	else if (m_landform == LandformType::Icefield) {
		m_productType = ProductType::Fur;
	}
	else if (m_landform == LandformType::Island) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.25f) {
			if (m_climate == ClimateType::MediterraneanClimate || m_climate == ClimateType::Oceanic) {
				m_productType = ProductType::Wine;
			}
			else {
				m_productType = ProductType::Fruit;
			}
		}
		else if (rnd < 0.4f) {
			m_productType = ProductType::Grain;
		}
		else if (rnd < 0.5f) {
			CalculateArtificialProduct();
		}
		else if (rnd < 0.7f) {
			m_productType = ProductType::Salt;
		}
		else if (rnd < 0.9f) {
			m_productType = ProductType::Sugar;
		}
		else {
			m_productType = ProductType::Livestock;
		}
	}
	else if (m_landform == LandformType::Lowland_Hill) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.25f) {
			CalculateArtificialProduct();
		}
		else if (rnd < 0.4f) {
			if (m_climate == ClimateType::MediterraneanClimate || m_climate == ClimateType::Oceanic) {
				m_productType = ProductType::Wine;
			}
			else {
				m_productType = ProductType::Fruit;
			}
		}
		else if (rnd < 0.5f) {
			m_productType = ProductType::Cotton;
		}
		else if (rnd < 0.6f) {
			m_productType = ProductType::Wool;
		}
		else if (rnd < 0.75f) {
			m_productType = ProductType::Grain;
		}
		else if (rnd < 0.9f) {
			m_productType = ProductType::Livestock;
		}
		else {
			CalculateMetalProduct();
		}
	}
	else if (m_landform == LandformType::Lowland_Plain) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.3f) {
			CalculateArtificialProduct();
		}
		else if (rnd < 0.8f) {
			m_productType = ProductType::Grain;
		}
		else {
			m_productType = ProductType::Livestock;
		}
	}
	else if (m_landform == LandformType::Marsh) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.6f) {
			m_productType = ProductType::Salt;
		}
		else if (rnd < 0.8f) {
			m_productType = ProductType::Livestock;
		}
		else {
			m_productType = ProductType::Wood;
		}
	}
	else if (m_landform == LandformType::Mountain) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.6f) {
			CalculateMetalProduct();
		}
		else if (rnd < 0.7f) {
			m_productType = ProductType::Fur;
		}
		else if (rnd < 0.9f) {
			m_productType = ProductType::Wood;
		}
		else {
			m_productType = ProductType::Wool;
		}
	}
	else if (m_landform == LandformType::Rainforest) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.1f) {
			CalculateMetalProduct();
		}
		else if (rnd < 0.7f) {
			m_productType = ProductType::Wood;
		}
		else if (rnd < 0.8f) {
			m_productType = ProductType::Fur;
		}
		else if (rnd < 0.9f) {
			m_productType = ProductType::Fruit;
		}
		else {
			m_productType = ProductType::Sugar;
		}
	}
	else if (m_landform == LandformType::Savanna) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.6f) {
			m_productType = ProductType::Ivory;
		}
		else if (rnd < 0.65f) {
			CalculateArtificialProduct();
		}
		else if (rnd < 0.8f) {
			m_productType = ProductType::Fur;
		}
		else if (rnd < 0.9f) {
			m_productType = ProductType::WarHorse;
		}
		else {
			m_productType = ProductType::Wool;
		}
	}
	else if (m_landform == LandformType::Subarctic_Forest) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.1f) {
			CalculateMetalProduct();
		}
		else if (rnd < 0.6f) {
			m_productType = ProductType::Wood;
		}
		else if (rnd < 0.9f) {
			m_productType = ProductType::Wax;
		}
		else {
			m_productType = ProductType::Fur;
		}

	}
	else if (m_landform == LandformType::Subtropical_Forest) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.1f) {
			CalculateArtificialProduct();
		}
		else if (rnd < 0.6f) {
			m_productType = ProductType::Wood;
		}
		else if (rnd < 0.9f) {
			m_productType = ProductType::Fruit;
		}
		else {
			m_productType = ProductType::Fur;
		}
	}
	else if (m_landform == LandformType::Temperate_Forest) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.1f) {
			CalculateArtificialProduct();
		}
		else if (rnd < 0.6f) {
			m_productType = ProductType::Wood;
		}
		else if (rnd < 0.9f) {
			m_productType = ProductType::Livestock;
		}
		else {
			m_productType = ProductType::Fur;
		}
	}
	else if (m_landform == LandformType::Tundra) {
		float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
		if (rnd < 0.7f) {
			m_productType = ProductType::Fur;
		}
		else {
			m_productType = ProductType::Wax;
		}
	}
}

void MapPolygonUnit::CalculateArtificialProduct()
{
	/*Glass,Silk,Jade,Porcelain,Cloth,Gem*/
	Map* map = GetCurMap();
	float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
	if (rnd < 0.2f) {
		if (m_continent && m_continent->m_eastContinent) {
			m_productType = ProductType::Porcelain;
		}
		else {
			m_productType = ProductType::Glass;
		}
	}
	else if (rnd < 0.8f) {
		if (m_continent && m_continent->m_eastContinent) {
			m_productType = ProductType::Silk;
		}
		else {
			m_productType = ProductType::Cloth;
		}
	}
	else if (rnd < 0.9f) {
		m_productType = ProductType::Sword;
	}
	else {
		if (m_continent && m_continent->m_eastContinent) {
			m_productType = ProductType::Jade;
		}
		else {
			m_productType = ProductType::Gem;
		}
	}

}

void MapPolygonUnit::CalculateMetalProduct()
{
	/* Iron,Gold,Copper,Silver */
	Map* map = GetCurMap();
	float rnd = map->m_productRNG->RollRandomFloatZeroToOne();
	if (rnd < 0.55f) {
		m_productType = ProductType::Iron;
	}
	else if (rnd < 0.9f) {
		m_productType = ProductType::Copper;
	}
	else if (rnd < 0.98f) {
		m_productType = ProductType::Silver;
	}
	else {
		m_productType = ProductType::Gold;
	}
}

void MapPolygonUnit::GrowPopulationOneMonth()
{
	int numOfGrowth = 0;
	float epidemicValue = 0.f;
	if (GetCurMap()->m_historyRNG->RollRandomFloatZeroToOne() < 0.06f) {
		epidemicValue = -0.002f;
	}
	float ratio = SmoothStop2( RangeMapClamped( (float)m_totalPopulation, 0.f, 100000.f, 1.f, 0.1f ) );
	float cultureFactor = 1.f;
	if (m_majorCulture->HasTrait( CultureTrait::Agrarian )) {
		cultureFactor += 0.1f;
	}
	if (m_owner->m_countryCulture->HasTrait( CultureTrait::Filial )) {
		cultureFactor += 0.05f;
	}
	if (m_owner->m_majorCulture->HasTrait( CultureTrait::Nomadic )) {
		cultureFactor -= 0.1f;
	}
	if (m_majorCulture->HasTrait( CultureTrait::Sedentary )) {
		cultureFactor += 0.05f;
	}
	if (m_majorCulture->HasTrait( CultureTrait::Resilient ) && m_totalPopulation / m_areaSize < 2000) {
		cultureFactor += 0.2f;
	}
	if (m_majorCulture->HasTrait( CultureTrait::Patriarchy )) {
		cultureFactor += 0.05f;
	}
		
	if (m_warFlag) {
		m_developmentFlag = false;
		m_warFlag = false;
		numOfGrowth = int( m_totalPopulation * (0.01f - epidemicValue) );
		if (numOfGrowth == 0) {
			numOfGrowth = 1;
		}
		m_totalPopulation -= numOfGrowth;
		ratio = 0.f;
	}
	else if (m_developmentFlag) {
		m_developmentFlag = false;
		numOfGrowth = int( m_totalPopulation * (0.0005f + epidemicValue) * ratio * cultureFactor );
		if (numOfGrowth == 0) {
			numOfGrowth = 1;
		}
		m_totalPopulation += numOfGrowth;
	}
	else {
		numOfGrowth = int( m_totalPopulation * (0.0002f + epidemicValue) * ratio * cultureFactor );
		if (numOfGrowth == 0) {
			numOfGrowth = 1;
		}
		m_totalPopulation += numOfGrowth;
	}

	// spread population
	MapPolygonUnit* minPopulationProv = nullptr;
	int minPopulation = INT_MAX;
	for (auto prov : m_adjacentUnits) {
		if (prov && prov->m_totalPopulation > 0 && prov->m_totalPopulation < minPopulation) {
			minPopulationProv = prov;
			minPopulation = prov->m_totalPopulation;
		}
	}
	if (minPopulationProv && minPopulation < 30000 && m_totalPopulation / minPopulation > 5) {
		int populationMovedOut = int( m_totalPopulation * 0.0003f * (1.f - ratio) );
		if (populationMovedOut == 0) {
			populationMovedOut = 1;
		}
		if (populationMovedOut >= m_totalPopulation) {
			populationMovedOut = m_totalPopulation - 1;
		}
		m_totalPopulation -= populationMovedOut;
		minPopulationProv->m_totalPopulation += populationMovedOut;
	}
	/*m_owner->m_totalPopulation += numOfGrowth;
	m_owner->AddCulturePopulation( numOfGrowth, m_cultures );
	m_owner->AddReligionPopulation( numOfGrowth, m_religions );*/
}

bool MapPolygonUnit::IsBeingSieged() const
{
	return m_owner && (int)m_armiesOnProv.size() > 0 && m_armiesOnProv[0].first->m_owner != m_owner;
}

bool MapPolygonUnit::IsConnectedByRoad(MapPolygonUnit* other) const
{
	if (m_roadData && other->m_roadData) {
		return m_roadData->IsConnectedTo( other->m_roadData );
	}
	return false;
}

void MapPolygonUnit::RemoveUnqualifiedLegitimateCountry()
{
	m_legitimateCountries.erase( std::remove_if(
		m_legitimateCountries.begin(), m_legitimateCountries.end(),
		[this]( Country* country ) {
			return !(country->m_countryCulture == m_majorCulture
				&& country->m_countryReligion == m_majorReligion);
		} ), m_legitimateCountries.end() );
}

float MapPolygonUnit::GetEconomyValue() const
{
	if (IsWater()) {
		return 0;
	}
	// 1. population
	// culture population
	float populationCount = 0.f;
	for (auto& pair : m_cultures) {
		if (pair.first == m_owner->m_countryCulture) {
			populationCount += (float)pair.second * TAX_PERSON_SAME_CULTURE;
		}
		else { // not country culture, give less money
			populationCount += (float)pair.second * TAX_PERSON_DIFF_CULTURE;
		}
	}

	// 2. cities
	float cityTaxCount = 0.f;
	for (auto city : m_cities) {
		if (!(city->HasAttribute(CityAttribute::Fort))) {
			cityTaxCount += (city->HasAttribute(CityAttribute::Commercial) ? (float)city->m_totalPopulation * TAX_PERSON_COMMERCIAL_CITY : (float)city->m_totalPopulation) * TAX_PERSON_NORMAL_CITY;
		}
	}
	for (auto town : m_towns) {
		cityTaxCount += (float)town->m_totalPopulation * TAX_PERSON_NORMAL_CITY;
	}

	// 3. production
	float productionCount = 0.f;
	float productValue = (GetCurMap()->m_productPrice[(int)m_productType] * GetClamped( (float)m_totalPopulation, 1000.f, 10000.f ));
	if (!IsLegitimateToCountry( m_owner )) {
		productValue *= 0.1f;
	}
	productionCount += (productValue * PRODUCTION_VALUE_PER_PERSON);

	return productionCount + cityTaxCount + populationCount;
}

void MapPolygonUnit::ResolveChangePopulation( int prevTotalPopulation )
{
	int popDiff = m_totalPopulation - prevTotalPopulation;
	if (m_owner) {
		m_owner->m_totalPopulation += popDiff;
		m_owner->AddCulturePopulation( popDiff, m_cultures );
		m_owner->AddReligionPopulation( popDiff, m_religions );
		m_owner->CalculateMajorCulture();
		m_owner->CalculateMajorReligion();
	}
}

void MapPolygonUnit::GetUnselectedCultures( std::vector<Culture*>& out_cultures )
{
	out_cultures.clear();
	Map* map = GetCurMap();
	for (auto culture : map->m_cultures) {
		if (HD_GetCultureInfluence( culture ) == 0.f) {
			out_cultures.push_back( culture );
		}
	}
}

void MapPolygonUnit::GetUnselectedReligions( std::vector<Religion*>& out_religions )
{
	out_religions.clear();
	Map* map = GetCurMap();
	for (auto religion : map->m_religions) {
		if (HD_GetReligionInfluence( religion ) == 0.f) {
			out_religions.push_back( religion );
		}
	}
}

void MapPolygonUnit::SqueezeReligionInfluence( Religion* religion, float influenceToAdd, float prevValue )
{
	float restOfInfluence = 1.f - prevValue;
	if (restOfInfluence == 0.f) {
		return;
	}
	for (auto& pair : m_religions) {
		if (pair.first != religion) {
			pair.second -= (pair.second / restOfInfluence * influenceToAdd);
		}
	}
}

void MapPolygonUnit::SqueezeCultureInfluence( Culture* culture, float influenceToAdd, float prevValue )
{
	float restOfInfluence = 1.f - prevValue;
	if (restOfInfluence == 0.f) {
		return;
	}
	for (auto& pair : m_cultures) {
		if (pair.first != culture) {
			pair.second -= (pair.second / restOfInfluence * influenceToAdd);
		}
	}
}

/*void MapPolygonUnit::AddHistoryData(void* startLocation) const
{
	void* curLocation = startLocation;
	memcpy( curLocation, &m_id, sizeof( m_id ) );
	curLocation = (void*)((size_t)curLocation + sizeof( m_id ));
	memcpy( curLocation, &m_dirtyBits, sizeof( m_dirtyBits ) );
	curLocation = (void*)((size_t)curLocation + sizeof( m_dirtyBits ));
	if (m_dirtyBits & Prov_Dirty_Flag_Populaton) {
		memcpy( curLocation, &m_totalPopulation, sizeof( m_totalPopulation ) );
		curLocation = (void*)((size_t)curLocation + sizeof( m_totalPopulation ));
	}
	if (m_dirtyBits & Prov_Dirty_Flag_Major_Culture) {
		// do nothing, calculated by culture data
	}
	if (m_dirtyBits & Prov_Dirty_Flag_Cultures) {
		int numOfCultures = (int)m_cultures.size();
		memcpy( curLocation, &numOfCultures, sizeof( numOfCultures ) );
		curLocation = (void*)((size_t)curLocation + sizeof( numOfCultures ));
		for (auto& pair : m_cultures) {
			memcpy( curLocation, &pair.first->m_id, sizeof( pair.first->m_id ) );
			curLocation = (void*)((size_t)curLocation + sizeof( pair.first->m_id ));
			memcpy( curLocation, &pair.second, sizeof( pair.second ) );
			curLocation = (void*)((size_t)curLocation + sizeof( pair.second ));
		}
	}
	if (m_dirtyBits & Prov_Dirty_Flag_Major_Religion) {
		// do nothing, calculated by religion data
	}
	if (m_dirtyBits & Prov_Dirty_Flag_Religions) {
		int numOfReligions = (int)m_religions.size();
		memcpy( curLocation, &numOfReligions, sizeof( numOfReligions ) );
		curLocation = (void*)((size_t)curLocation + sizeof( numOfReligions ));
		for (auto& pair : m_religions) {
			memcpy( curLocation, &pair.first->m_id, sizeof( pair.first->m_id ) );
			curLocation = (void*)((size_t)curLocation + sizeof( pair.first->m_id ));
			memcpy( curLocation, &pair.second, sizeof( pair.second ) );
			curLocation = (void*)((size_t)curLocation + sizeof( pair.second ));
		}
	}
	if (m_dirtyBits & Prov_Dirty_Flag_Owner) {
		memcpy( curLocation, &m_owner->m_id, sizeof( m_owner->m_id ) );
		curLocation = (void*)((size_t)curLocation + sizeof( m_owner->m_id ));
	}
	if (m_dirtyBits & Prov_Dirty_Flag_Legal_Countries) {

	}
	if (m_dirtyBits & Prov_Dirty_Flag_Army_On) {

	}
}*/

void MapPolygonUnit::SetLake()
{
	if (m_landform == LandformType::Ocean) {
		std::set<MapPolygonUnit*>& dirtySet = GetCurMap()->m_oceanDirtySet;
		if (dirtySet.find( this ) != dirtySet.end()) {
			return;
		}
		std::deque<MapPolygonUnit*> queue;
		queue.push_back( this );
		std::vector<MapPolygonUnit*> unitCollection;
		unitCollection.push_back( this );
		dirtySet.insert( this );
		while (!queue.empty()) {
			MapPolygonUnit* front = queue.front();
			queue.pop_front();
			for (int i = 0; i < (int)front->m_adjacentUnits.size(); i++) {
				if (front->m_adjacentUnits[i]->IsOcean() && dirtySet.find( front->m_adjacentUnits[i] ) == dirtySet.end()) {
					dirtySet.insert( front->m_adjacentUnits[i] );
					unitCollection.push_back( front->m_adjacentUnits[i] );
					queue.push_back( front->m_adjacentUnits[i] );
				}
			}
		}
		if ((int)unitCollection.size() <= GetCurMap()->m_generationSettings.m_numOfUnitsToHaveLake) {
			for (auto unit : unitCollection) {
				unit->m_landform = LandformType::Lake;
			}
		}
	}
}

void MapPolygonUnit::SetIsland()
{
	if (m_landform == LandformType::Land) {
		std::set<MapPolygonUnit*>& dirtySet = GetCurMap()->m_islandDirtySet;
		if (dirtySet.find( this ) != dirtySet.end()) {
			return;
		}
		std::deque<MapPolygonUnit*> queue;
		queue.push_back( this );
		std::vector<MapPolygonUnit*> unitCollection;
		unitCollection.push_back( this );
		dirtySet.insert( this );
		while (!queue.empty()) {
			MapPolygonUnit* front = queue.front();
			queue.pop_front();
			for (int i = 0; i < (int)front->m_adjacentUnits.size(); i++) {
				if (front->m_adjacentUnits[i]->IsLand() && dirtySet.find( front->m_adjacentUnits[i] ) == dirtySet.end()) {
					dirtySet.insert( front->m_adjacentUnits[i] );
					unitCollection.push_back( front->m_adjacentUnits[i] );
					queue.push_back( front->m_adjacentUnits[i] );
				}
			}
		}
		if ((int)unitCollection.size() <= GetCurMap()->m_generationSettings.m_numOfUnitsToHaveIsland) {
			for (auto unit : unitCollection) {
				unit->m_landform = LandformType::Island;
			}
		}
	}
}

void MapPolygonUnit::CalculateSeaDistance()
{
	if (IsOcean()) {
		m_nearestSeaDir = Direction::Middle;
		m_nearestSeaDistance = 0.f;
		return;
	}
	Map* map = GetCurMap();
	float stepDist = 0.5f * Minf( map->GetDimensions().x, map->GetDimensions().y) / (float)GetCurGenerationSettings().m_sqrtBasePolygons;
	AABB2 const& bounds = map->m_bounds;
	int numOfSteps = 100;
	float inversedNumOfSteps = 1.f / (float)numOfSteps;

	float eastOceanness = 0.f; // how much influenced by east ocean
	float eastOceanDist = 0.f; // total east ocean distance
	float accumedEastOceanDist = 0.f; // accumulated ocean distance (not add to the total distance)
	bool eastMeetOcean = false; // if step into a sea unit when searching
	for (int i = 0; i < numOfSteps; i++) {
		// go east
		Vec2 thisProbePos = m_centerPosition + Vec2( stepDist * i, 0.f );
		accumedEastOceanDist += stepDist;
		// if test position inside map bounds
		if (bounds.IsPointInside( thisProbePos )) {
			MapPolygonUnit* hitUnit = map->GetUnitByPosFast( thisProbePos );
			if (!hitUnit) {
				continue;
			}
			// if meet an ocean unit
			if (hitUnit->IsOcean()) {
				eastOceanness += (0.5f + (0.5f * (1.f - SmoothStart2( (float)i * inversedNumOfSteps ))));
				eastMeetOcean = true;
			}
			// if meet a land and the previous sea is not so big
			if (hitUnit->IsLand() && accumedEastOceanDist < map->m_diagonalLength * 0.05f) {
				eastOceanDist += accumedEastOceanDist;
				accumedEastOceanDist = 0.f;
			}
			// if meet a higher land
			if (hitUnit->m_height > m_height) {
				eastOceanness -= 0.5f * (0.5f + (0.5f * (1.f - SmoothStart2( (float)i * inversedNumOfSteps ))));
			}
		}
		// if test position outside map bounds, see it as ocean
		else {
			eastOceanness += (0.5f + (0.5f * (1.f - SmoothStart2( (float)i * inversedNumOfSteps ))));
			eastMeetOcean = true;
		}
	}

	float westOceanness = 0.f;
	float westOceanDist = 0.f;
	float accumedWestOceanDist = 0.f;
	bool westMeetOcean = false;
	// go west
	for (int i = 0; i < numOfSteps; i++) {
		accumedWestOceanDist += stepDist;
		Vec2 thisProbePos = m_centerPosition - Vec2( stepDist * i, 0.f );
		if (bounds.IsPointInside( thisProbePos )) {
			MapPolygonUnit* hitUnit = map->GetUnitByPosFast( thisProbePos );
			if (!hitUnit) {
				continue;
			}
			if (hitUnit->IsOcean()) {
				westOceanness += (0.5f + (0.5f * (1.f - SmoothStart2( (float)i * inversedNumOfSteps ))));
				westMeetOcean = true;
			}
			if (hitUnit->IsLand() && accumedWestOceanDist < map->m_diagonalLength * 0.05f) {
				westOceanDist += accumedWestOceanDist;
				accumedWestOceanDist = 0.f;
			}
			if (hitUnit->m_height > m_height) {
				westOceanness -= 0.5f * (0.5f + (0.5f * (1.f - SmoothStart2( (float)i * inversedNumOfSteps ))));
			}
		}
		else {
			westOceanness += (0.5f + (0.5f * (1.f - SmoothStart2( (float)i * inversedNumOfSteps ))));
			westMeetOcean = true;
		}
	}

	if (!westMeetOcean && !eastMeetOcean) {
		m_nearestSeaDir = Direction::Middle;
		m_nearestSeaDistance = 0.f;
	}
	else if (westOceanness > eastOceanness) {
		m_nearestSeaDir = Direction::West;
		m_nearestSeaDistance = westOceanDist;
	}
	else {
		m_nearestSeaDir = Direction::East;
		m_nearestSeaDistance = eastOceanDist;
	}
	m_nearestSeaDistance /= map->m_diagonalLength;

	for (auto unit : m_adjacentUnits) {
		if (unit->IsOcean()) {
			m_isCoast = true;
			break;
		}
	}

	// use BFS to get the sea distance
	/*std::deque<MapPolygonUnit*> queue;
	queue.push_back( this );
	std::set<MapPolygonUnit*> dirtySet;
	bool coastFlag = true;
	dirtySet.insert( this );
	while (!queue.empty()) {
		MapPolygonUnit* front = queue.front();
		queue.pop_front();
		for (int i = 0; i < (int)front->m_adjacentUnits.size(); i++) {
			if (dirtySet.find( front->m_adjacentUnits[i] ) == dirtySet.end()) {
				Vec2 dirToSea = front->m_adjacentUnits[i]->m_centerPosition - m_centerPosition;
				if (front->m_adjacentUnits[i]->IsOcean() && (abs(dirToSea.x) > abs(dirToSea.y))) {
					m_nearestSeaDistance = GetDistance2D( m_centerPosition, front->m_adjacentUnits[i]->m_centerPosition ) / GetCurMap()->m_diagonalLength;
					m_isCoast = coastFlag;
					m_nearestSeaPos = front->m_adjacentUnits[i]->m_centerPosition;
					m_nearestSeaDir = dirToSea.x > 0 ? Direction::East : Direction::West;
					return;
				}
				dirtySet.insert( front->m_adjacentUnits[i] );
				queue.push_back( front->m_adjacentUnits[i] );
			}
		}
		coastFlag = false;
	}*/
}

void MapPolygonUnit::CalculateClimate()
{
	if (IsOcean()) {
		m_climate = ClimateType::Water;
	}
	else if (m_landform == LandformType::Island) {
		if (abs( m_latitude ) < 7.5f) {
			m_climate = ClimateType::TropicalRainforest;
		}
		else if (abs( m_latitude ) < 20.f) {
			m_climate = ClimateType::TropicalSavanna;
		}
		else if (abs( m_latitude ) < 40.f) {
			m_climate = ClimateType::HotDesert;
		}
		else if (abs( m_latitude ) < 70.f) {
			m_climate = ClimateType::Oceanic;
		}
		else {
			m_climate = ClimateType::Tundra;
		}
	}
	else if (abs( m_latitude ) < 7.5f) {
		if (m_height < 2500.f) {
			m_climate = ClimateType::TropicalRainforest;
		}
		else if (m_height < 3000.f) {
			m_climate = ClimateType::TropicalSavanna;
		}
		else if (m_height < 5000.f) {
			m_climate = ClimateType::Subarctic;
		}
		else if (m_height < 5500.f) {
			m_climate = ClimateType::Tundra;
		}
		else {
			m_climate = ClimateType::IceCap;
		}
	}
	else if (abs( m_latitude ) >= 7.5f && abs( m_latitude ) < 20.f) {
		if (m_height < 2500.f) {
			if (m_nearestSeaDir == Direction::West || m_nearestSeaDir == Direction::Middle) {
				m_climate = ClimateType::TropicalSavanna;
			}
			else {
				m_climate = ClimateType::TropicalMonsoon;
			}
		}
		else if (m_height < 5000.f) {
			m_climate = ClimateType::Subarctic;
		}
		else if (m_height < 6000.f) {
			m_climate = ClimateType::Tundra;
		}
		else {
			m_climate = ClimateType::IceCap;
		}
	}
	else if (abs( m_latitude ) >= 20.f && abs( m_latitude ) < 35.f) {
		if (m_height < 3000.f) {
			if (m_nearestSeaDir == Direction::West || m_nearestSeaDir == Direction::Middle) {
				m_climate = ClimateType::HotDesert;
			}
			else {
				m_climate = ClimateType::HumidSubtropical;
			}
		}
		else if (m_height < 4000.f) {
			m_climate = ClimateType::Subarctic;
		}
		else if (m_height < 5500.f) {
			m_climate = ClimateType::Tundra;
		}
		else {
			m_climate = ClimateType::IceCap;
		}
	}
	else if (abs( m_latitude ) >= 35.f && abs( m_latitude ) < 45.f) {
		if (m_height < 2500.f) {
			if (m_nearestSeaDir == Direction::West) {
				m_climate = ClimateType::MediterraneanClimate;
			}
			else if (m_nearestSeaDir == Direction::Middle) {
				m_climate = ClimateType::HotDesert;
			}
			else {
				m_climate = ClimateType::HumidSubtropical;
			}
		}
		else if (m_height < 3000.f) {
			m_climate = ClimateType::HotDesert;
		}
		else if (m_height < 4000.f) {
			m_climate = ClimateType::Subarctic;
		}
		else if (m_height < 5500.f) {
			m_climate = ClimateType::Tundra;
		}
		else {
			m_climate = ClimateType::IceCap;
		}
	}
	else if (abs( m_latitude ) >= 45.f && abs( m_latitude ) < 60.f) {
		if (m_height < 2500.f) {
			if (m_nearestSeaDir == Direction::Middle || (m_nearestSeaDir == Direction::West && m_nearestSeaDistance > 0.1f) || (m_nearestSeaDir == Direction::East && m_nearestSeaDistance > 0.05f)) {
				m_climate = ClimateType::HumidContinental;
			}
			else if (m_nearestSeaDir == Direction::West) {
				m_climate = ClimateType::Oceanic;
			}
			else {
				m_climate = ClimateType::HumidContinentalMonsoon;
			}
		}
		else if (m_height < 3000.f) {
			m_climate = ClimateType::ColdDesert;
		}
		else if (m_height < 4000.f) {
			m_climate = ClimateType::Subarctic;
		}
		else if (m_height < 5500.f) {
			m_climate = ClimateType::Tundra;
		}
		else {
			m_climate = ClimateType::IceCap;
		}
	}
	else if (abs( m_latitude ) >= 60.f && abs( m_latitude ) < 75.f) {
		m_climate = ClimateType::Subarctic;
	}
	else if (abs( m_latitude ) >= 75.f) {
		if (m_isCoast) {
			m_climate = ClimateType::Tundra;
		}
		else {
			m_climate = ClimateType::IceCap;
		}
	}
}

void MapPolygonUnit::CalculatePrecipitation()
{
	// precipitation is influenced by climate and distance from the nearest sea
	float basicSummerPrecipitation = 0.f;
	float basicWinterPrecipitation = 0.f;
	if (m_climate == ClimateType::TropicalRainforest) {
		// not very influenced by the sea distance
		float seaDistanceSub = m_nearestSeaDistance * 300.f;
		basicSummerPrecipitation = 2030.f - seaDistanceSub;
		basicWinterPrecipitation = 2030.f - seaDistanceSub;
	}
	else if (m_climate == ClimateType::TropicalSavanna) {
		// not very influenced by the sea distance
		float seaDistanceSub = m_nearestSeaDistance * 600.f;
		basicSummerPrecipitation = 2100.f - seaDistanceSub;
		basicWinterPrecipitation = GetClamped( 100.f - seaDistanceSub, 0.f, 100.f );
	}
	else if (m_climate == ClimateType::TropicalMonsoon) {
		// not very influenced by the sea distance
		float seaDistanceSub = m_nearestSeaDistance * 500.f;
		basicSummerPrecipitation = 2400.f - seaDistanceSub;
		basicWinterPrecipitation = GetClamped( 300.f - seaDistanceSub, 0.f, 300.f );
	}
	else if (m_climate == ClimateType::HotDesert) {
		// very influenced by the sea
		float seaDistanceSub = m_nearestSeaDistance * 1000.f;
		basicSummerPrecipitation = GetClamped( 100.f - seaDistanceSub, 0.f, 100.f );
		basicWinterPrecipitation = GetClamped( 100.f - seaDistanceSub, 0.f, 100.f );
	}
	else if (m_climate == ClimateType::ColdDesert) {
		float seaDistanceSub = m_nearestSeaDistance * 1000.f;
		basicSummerPrecipitation = GetClamped( 100.f - seaDistanceSub, 0.f, 100.f );
		basicWinterPrecipitation = GetClamped( 100.f - seaDistanceSub, 0.f, 100.f );
	}
	else if (m_climate == ClimateType::HumidSubtropical) {
		float seaDistanceSub = m_nearestSeaDistance * 600.f;
		basicSummerPrecipitation = 2000.f - seaDistanceSub;
		basicWinterPrecipitation = GetClamped( 300.f - seaDistanceSub, 0.f, 300.f );
	}
	else if (m_climate == ClimateType::MediterraneanClimate) {
		float seaDistanceSub = m_nearestSeaDistance * 600.f;
		basicSummerPrecipitation = GetClamped( 100.f - seaDistanceSub, 0.f, 100.f );
		basicWinterPrecipitation = 1300.f - seaDistanceSub;
	}
	else if (m_climate == ClimateType::Oceanic) {
		float seaDistanceSub = m_nearestSeaDistance * 400.f;
		basicSummerPrecipitation = 800.f - seaDistanceSub;
		basicWinterPrecipitation = 900.f - seaDistanceSub;
	}
	else if (m_climate == ClimateType::HumidContinental) {
		float seaDistanceSub = m_nearestSeaDistance * 600.f;
		basicSummerPrecipitation = 500.f - seaDistanceSub;
		basicWinterPrecipitation = 500.f - seaDistanceSub;
	}
	else if (m_climate == ClimateType::HumidContinentalMonsoon) {
		float seaDistanceSub = m_nearestSeaDistance * 400.f;
		basicSummerPrecipitation = 1200.f - seaDistanceSub;
		basicWinterPrecipitation = 200.f - seaDistanceSub;
	}
	else if (m_climate == ClimateType::IceCap) {
		float seaDistanceSub = m_nearestSeaDistance * 100.f;
		basicSummerPrecipitation = 150.f - seaDistanceSub;
		basicWinterPrecipitation = 150.f - seaDistanceSub;
	}
	else if (m_climate == ClimateType::Tundra) {
		float seaDistanceSub = m_nearestSeaDistance * 200.f;
		basicSummerPrecipitation = 250.f - seaDistanceSub;
		basicWinterPrecipitation = 250.f - seaDistanceSub;
	}
	else if (m_climate == ClimateType::Subarctic) {
		/* m_nearestSeaDistance is from 0 to 1 */
		float seaDistanceSub = m_nearestSeaDistance * 300.f;
		/* this climate has more rain in summer */
		basicSummerPrecipitation = 630.f - seaDistanceSub;
		basicWinterPrecipitation = 320.f - seaDistanceSub;
	}
	/* add a perlin noise to the final result */
	unsigned int seed = GetCurMap()->m_precipitationSeed;
	m_summerPrecipitation = Maxf( 0.f, basicSummerPrecipitation + 100.f * 
		Compute2dPerlinNoise( m_centerPosition.x, m_centerPosition.y, 1.f, 3, 0.5f, 2.0f, true, seed ) );
	m_winterPrecipitation = Maxf( 0.f, basicWinterPrecipitation + 100.f * 
		Compute2dPerlinNoise( m_centerPosition.x, m_centerPosition.y, 1.f, 3, 0.5f, 2.0f, true, seed ) );
}

void MapPolygonUnit::CalculateTemperature()
{
	float basicSummerTemperature = 0.f;
	float basicWinterTemperature = 0.f;
	float height = GetClamped( m_height, 0.f, GetCurGenerationSettings().m_maxHeight );
	float heightSub = height * 0.0006f;
	if (m_climate == ClimateType::TropicalRainforest) {
		float latitudeSub = Absf( m_latitude ) * 0.3f;
		basicSummerTemperature = 28.f - heightSub - latitudeSub;
		basicWinterTemperature = 28.f - heightSub - latitudeSub;
	}
	else if (m_climate == ClimateType::TropicalSavanna) {
		float latitudeSub = Absf( m_latitude - 7.5f ) * 0.1f;
		basicSummerTemperature = 25.f - heightSub - latitudeSub;
		basicWinterTemperature = 24.f - heightSub - latitudeSub;
	}
	else if (m_climate == ClimateType::TropicalMonsoon) {
		float latitudeSub = Absf( m_latitude - 7.5f ) * 0.1f;
		basicSummerTemperature = 24.f - heightSub - latitudeSub;
		basicWinterTemperature = 22.f - heightSub - latitudeSub;
	}
	else if (m_climate == ClimateType::HotDesert) {
		float latitudeSub = Absf( m_latitude - 20.f ) * 0.1f;
		basicSummerTemperature = 23.f - heightSub - latitudeSub;
		basicWinterTemperature = 21.f - heightSub - latitudeSub;
	}
	else if (m_climate == ClimateType::ColdDesert) {
		basicSummerTemperature = 16.f - heightSub;
		basicWinterTemperature = 14.f - heightSub;
	}
	else if (m_climate == ClimateType::HumidSubtropical) {
		basicSummerTemperature = 21.f - heightSub;
		basicWinterTemperature = 8.f - heightSub;
	}
	else if (m_climate == ClimateType::MediterraneanClimate) {
		basicSummerTemperature = 23.f - heightSub;
		basicWinterTemperature = 11.f - heightSub;
	}
	else if (m_climate == ClimateType::Oceanic) {
		basicSummerTemperature = 21.f - heightSub;
		basicWinterTemperature = 9.f - heightSub;
	}
	else if (m_climate == ClimateType::HumidContinental) {
		float latitudeSub = Absf( m_latitude - 45.f ) * 0.5f;
		basicSummerTemperature = 20.f - heightSub - latitudeSub;
		basicWinterTemperature = 3.f - heightSub - latitudeSub;
	}
	else if (m_climate == ClimateType::HumidContinentalMonsoon) {
		float latitudeSub = Absf( m_latitude - 45.f ) * 0.5f;
		basicSummerTemperature = 21.f - heightSub - latitudeSub;
		basicWinterTemperature = 2.f - heightSub - latitudeSub;
	}
	else if (m_climate == ClimateType::IceCap) {
		basicSummerTemperature = 0.f - heightSub;
		basicWinterTemperature = -20.f - heightSub;
	}
	else if (m_climate == ClimateType::Tundra) {
		basicSummerTemperature = 8.f - heightSub;
		basicWinterTemperature = -10.f - heightSub;
	}
	else if (m_climate == ClimateType::Subarctic) {
		float latitudeSub = Absf( m_latitude - 60.f ) * 0.5f;
		basicSummerTemperature = 12.f - heightSub - latitudeSub;
		basicWinterTemperature = 0.f - heightSub - latitudeSub;
	}
	/* add perlin noise */
	unsigned int seed = GetCurMap()->m_temperatureSeed;
	m_summerAvgTemperature = basicSummerTemperature + 3.f *
		Compute2dPerlinNoise( m_centerPosition.x, m_centerPosition.y, 1.f, 3, 0.5f, 2.0f, true, seed );
	m_winterAvgTemperature = basicWinterTemperature + 3.f * 
		Compute2dPerlinNoise( m_centerPosition.x, m_centerPosition.y, 1.f, 3, 0.5f, 2.0f, true, seed );
}

void MapPolygonUnit::SetLandform()
{
	unsigned int seed = GetCurMap()->m_landformSeed;
	float rndNum = 0.5f + 0.5f * Compute2dPerlinNoise( m_centerPosition.x, m_centerPosition.y, 1.f, 3, 0.5f, 2.0f, true, seed );
	if (m_landform == LandformType::Ocean || m_landform == LandformType::Island || m_landform == LandformType::Lake) {
		return;
	}
	if (m_climate == ClimateType::ColdDesert || m_climate == ClimateType::HotDesert) {
		m_landform = LandformType::Desert;
	}
	else if (m_climate == ClimateType::Tundra) {
		m_landform = LandformType::Tundra;
	}
	else if (m_climate == ClimateType::IceCap) {
		m_landform = LandformType::Icefield;
	}
	else if (m_climate == ClimateType::HumidContinental) {
		if (rndNum < 0.6f) {
			m_landform = LandformType::Grassland;
		}
		else if (rndNum < 0.85f) {
			if (m_height < 1000.f) {
				m_landform = LandformType::Lowland_Plain;
			}
			else {
				m_landform = LandformType::Highland_Plain;
			}
		}
		else {
			if (m_height < 1000.f) {
				m_landform = LandformType::Lowland_Hill;
			}
			else {
				m_landform = LandformType::Highland_Hill;
			}
		}
	}
	else if (m_climate == ClimateType::HumidContinentalMonsoon) {
		rndNum -= m_summerPrecipitation / 10000.f;
		if (rndNum < 0.3f) {
			m_landform = LandformType::Temperate_Forest;
		}
		else if (rndNum < 0.8f) {
			if (m_height < 1000.f) {
				m_landform = LandformType::Lowland_Plain;
			}
			else {
				m_landform = LandformType::Highland_Plain;
			}
		}
		else {
			if (m_height < 1000.f) {
				m_landform = LandformType::Lowland_Hill;
			}
			else {
				m_landform = LandformType::Highland_Hill;
			}
		}
	}
	else if (m_climate == ClimateType::HumidSubtropical) {
		rndNum -= m_summerPrecipitation / 10000.f;
		if (rndNum < 0.01f) {
			m_landform = LandformType::Marsh;
		}
		else if (rndNum < 0.3f) {
			m_landform = LandformType::Subtropical_Forest;
		}
		else if (rndNum < 0.8f) {
			if (m_height < 1000.f) {
				m_landform = LandformType::Lowland_Plain;
			}
			else {
				m_landform = LandformType::Highland_Plain;
			}
		}
		else {
			if (m_height < 1000.f) {
				m_landform = LandformType::Lowland_Hill;
			}
			else {
				m_landform = LandformType::Highland_Hill;
			}
		}
	}
	else if (m_climate == ClimateType::MediterraneanClimate) {
		if (rndNum < 0.1f) {
			m_landform = LandformType::Subtropical_Forest;
		}
		else if (rndNum < 0.6f) {
			if (m_height < 1000.f) {
				m_landform = LandformType::Lowland_Plain;
			}
			else {
				m_landform = LandformType::Highland_Plain;
			}
		}
		else {
			if (m_height < 1000.f) {
				m_landform = LandformType::Lowland_Hill;
			}
			else {
				m_landform = LandformType::Highland_Hill;
			}
		}
	}
	else if (m_climate == ClimateType::Oceanic) {
		rndNum -= m_summerPrecipitation / 10000.f;
		if (rndNum < 0.01f) {
			m_landform = LandformType::Marsh;
		}
		else if (rndNum < 0.5f) {
			m_landform = LandformType::Temperate_Forest;
		}
		else if (rndNum < 0.65f) {
			if (m_height < 1000.f) {
				m_landform = LandformType::Lowland_Plain;
			}
			else {
				m_landform = LandformType::Highland_Plain;
			}
		}
		else if (rndNum < 0.8f) {
			if (m_height < 1000.f) {
				m_landform = LandformType::Lowland_Hill;
			}
			else {
				m_landform = LandformType::Highland_Hill;
			}
		}
		else {
			m_landform = LandformType::Grassland;
		}
	}
	else if (m_climate == ClimateType::Subarctic) {
		if (rndNum < 0.1f) {
			if (Absf( m_latitude ) > 60.f) {
				m_landform = LandformType::Marsh;
			}
			else {
				m_landform = LandformType::Subarctic_Forest;
			}
		}
		else if (rndNum < 0.6f) {
			m_landform = LandformType::Subarctic_Forest;
		}
		else if (rndNum < 0.65f) {
			m_landform = LandformType::Mountain;
		}
		else if (rndNum < 0.8f) {
			m_landform = LandformType::Grassland;
		}
		else {
			m_landform = LandformType::Tundra;
		}
	}
	else if (m_climate == ClimateType::TropicalMonsoon) {
		if (rndNum < 0.1f) {
			m_landform = LandformType::Marsh;
		}
		else if (rndNum < 0.6f) {
			m_landform = LandformType::Rainforest;
		}
		else if (rndNum < 0.9f) {
			if (m_height < 1000.f) {
				m_landform = LandformType::Lowland_Plain;
			}
			else {
				m_landform = LandformType::Highland_Plain;
			}
		}
		else {
			if (m_height < 1000.f) {
				m_landform = LandformType::Lowland_Hill;
			}
			else {
				m_landform = LandformType::Highland_Hill;
			}
		}
	}
	else if (m_climate == ClimateType::TropicalRainforest) {
		m_landform = LandformType::Rainforest;
	}
	else if (m_climate == ClimateType::TropicalSavanna) {
		m_landform = LandformType::Savanna;
	}
}

float MapPolygonUnit::GetDistanceToSeaFromThisUnit( Vec2 const& pos ) const
{
	float minDist = FLT_MAX;
	for (auto edge : m_edges) {
		if (edge->m_opposite && edge->m_opposite->m_owner && edge->m_opposite->m_owner->IsOcean()) {
			float thisDist = GetPointDistanceToLineSegmentSquared2D( pos, edge->m_startPos, edge->m_endPos );
			if (thisDist < minDist) {
				minDist = thisDist;
			}
		}
	}
	return sqrtf( minDist );
}

void StarEdge::DoNoisyEdge( int recursionTimes /*= 4*/, float ratio /*= 0.25f */ )
{
	if (!m_opposite) {
		return;
	}
	if (m_opposite->m_owner->m_isFarAwayFakeUnit || m_owner->m_isFarAwayFakeUnit) {
		return;
	}

	if ((int)m_opposite->m_noisyEdges.size() > 0) {
		for (int i = (int)m_opposite->m_noisyEdges.size() - 1; i >= 0; --i) {
			m_noisyEdges.push_back( m_opposite->m_noisyEdges[i] );
		}
		return;
	}


	Vec2 startPos = m_startPos;
	Vec2 endPos = m_endPos;

	m_noisyEdges.push_back( startPos );
	Vec2 normal = (endPos - startPos).GetRotated90Degrees();

	Vec2 refPointTop = (startPos + endPos + normal * 0.4f) * 0.5f;
	Vec2 refPointDown = (startPos + endPos - normal * 0.4f) * 0.5f;

	RecursiveCalculateNoisyEdge( recursionTimes, startPos, endPos, refPointTop, refPointDown, recursionTimes % 2 == 0 ? ratio:1.f - ratio );
	m_noisyEdges.push_back( endPos );
}

void StarEdge::RecursiveCalculateNoisyEdge( int numOfCurRecursive, Vec2 const& lineSegLeft, Vec2 const& lineSegRight, Vec2 const& refPointTop, Vec2 const& refPointDown, float ratio, bool considerRiverAlready )
{
	if (numOfCurRecursive == 0) {
		return;
	}
	else {
		float thisRatio = ratio;
		// consider river
		if (m_owner->m_riverOnThis && m_owner->IsLand() && !considerRiverAlready/* && m_owner->m_riverOnThis->m_sandiness / m_owner->m_riverOnThis->m_length < 4.5f */ ) {
			Vec2 const& riverEndPos = *(m_owner->m_riverOnThis->m_anchorPoint.end() - 1);
			Vec2 nearestPoint = GetNearestPointOnLineSegment2D( riverEndPos, refPointTop, refPointDown );
			float distSquared = GetDistance2D( riverEndPos, nearestPoint );
			if (distSquared < 0.2f) {
				float fraction = GetFractionWithinRange( nearestPoint.x, refPointTop.x, refPointDown.x );
				if (fraction > thisRatio) {
					thisRatio = thisRatio - 0.3f;
				}
				else {
					thisRatio = fraction - 0.3f;
				}
				Vec2 preCenterPoint = refPointTop * (1 - thisRatio) + refPointDown * thisRatio;
				while ((int)m_owner->m_riverOnThis->m_anchorPoint.size() >= 2 && 
					GetDistanceSquared2D( m_owner->m_riverOnThis->m_anchorPoint[ m_owner->m_riverOnThis->m_anchorPoint.size() - 1], preCenterPoint) >
						GetDistanceSquared2D( m_owner->m_riverOnThis->m_anchorPoint[ m_owner->m_riverOnThis->m_anchorPoint.size() - 2], preCenterPoint)) {
					m_owner->m_riverOnThis->m_anchorPoint.pop_back();
				}
				m_owner->m_riverOnThis->m_anchorPoint.pop_back();
				m_owner->m_riverOnThis->m_anchorPoint.push_back( preCenterPoint );
				considerRiverAlready = true;
			}
		}

		Vec2 topLeftPoint = (lineSegLeft + refPointTop) * 0.5f;
		Vec2 topRightPoint = (lineSegRight + refPointTop) * 0.5f;
		Vec2 bottomLeftPoint = (lineSegLeft + refPointDown) * 0.5f;
		Vec2 bottomRightPoint = (lineSegRight + refPointDown) * 0.5f;
		Vec2 centerPoint = refPointTop * (1 - thisRatio) + refPointDown * thisRatio;

		float nextRatio;
		if (numOfCurRecursive % 2 == 0) {
			nextRatio = 1.f - ratio;
		}
		else {
			nextRatio = ratio;
		}
		RecursiveCalculateNoisyEdge( numOfCurRecursive - 1, lineSegLeft, centerPoint, topLeftPoint, bottomLeftPoint, nextRatio, considerRiverAlready );
		m_noisyEdges.push_back( centerPoint );
		RecursiveCalculateNoisyEdge( numOfCurRecursive - 1, centerPoint, lineSegRight, topRightPoint, bottomRightPoint, nextRatio, considerRiverAlready );
	}
}
