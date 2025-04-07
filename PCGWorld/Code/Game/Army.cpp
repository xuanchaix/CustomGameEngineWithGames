#include "Game/Army.hpp"
#include "Game/MapPolygonUnit.hpp"
#include "Game/City.hpp"
#include "Game/Country.hpp"
#include "Game/Map.hpp"

Army::Army( Country* owner, MapPolygonUnit* provIn, int size )
	:m_owner(owner), m_provIn(provIn), m_size(size)
{

}

Army::~Army()
{

}

Vec2 Army::GetPosition() const
{
	if ((int)m_provIn->m_cities.size() > 0) {
		return m_provIn->m_cities[0]->m_position;
	}
	else {
		return m_provIn->m_geoCenterPos;
	}
}

Vec3 Army::GetPosition3D() const
{
	if ((int)m_provIn->m_cities.size() > 0) {
		return Vec3( m_provIn->m_cities[0]->m_position, m_provIn->m_cities[0]->m_height );
	}
	else {
		return Vec3( m_provIn->m_geoCenterPos, m_provIn->m_height * HEIGHT_FACTOR );
	}
}

float Army::GetOuterRadius() const
{
	return CITY_ICON_SIDE_LENGTH * 1.3f;
}

float Army::GetInnerRadius() const
{
	return CITY_ICON_SIDE_LENGTH * 0.8f;
}

void Army::IntegrateArmy( Army* army )
{
	m_size += army->m_size;
}

void Army::GetAllProvincesCanGo( std::vector<MapPolygonUnit*>& out_provs )
{
	out_provs.clear();
	for (auto adjProv : m_provIn->m_adjacentUnits) {
		if ((adjProv->m_owner && (adjProv->m_owner == m_owner || adjProv->m_owner->IsInWarWith( m_owner ))) || adjProv->IsWater()) {
			out_provs.push_back( adjProv );
		}
	}
	//out_provs.push_back( m_provIn );
}

MapPolygonUnit* Army::FindNextProvinceToGo( MapPolygonUnit* target ) const
{
	Map* map = GetCurMap();
	std::vector<MapPolygonUnit*> route;
	bool hasRes = map->m_aStarHelper.CalculateRouteWaterBlockRouteAndHeightWeight( m_provIn, target, route );
	if (!hasRes || (int)route.size() <= 1 || route[route.size() - 1] != m_provIn) {
		if (!hasRes) {
			map->m_aStarHelper.CalculateRoute( m_provIn, target, route );
			if ((int)route.size() <= 1 || route[route.size() - 1] != m_provIn) {
				ERROR_RECOVERABLE( "Cannot reach here!" );
				return nullptr;
			}
			else {
				return route[route.size() - 2];
			}
		}
		else {
			ERROR_RECOVERABLE( "Cannot reach here!" );
			return nullptr;
		}
	}
	return route[route.size() - 2];
}
