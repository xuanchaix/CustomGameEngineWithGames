#include "Game/Culture.hpp"
#include "Game/Map.hpp"
#include "Game/NameGenerator.hpp"

std::vector<std::string> Culture::s_defaultNames = { 
	"Valdarans", "Cyrithians", "Tyrinthians", "Aelorian", "Myrnathi",
	"Zephyrites", "Galadrians", "Rostrumian", "Vardunians", "Hesperians",
	"Eldrinthians", "Calthorians", "Falariths", "Nythari", "Thalrunians",
	"Yronvans", "Serephiri", "Draegothans", "Verenthals", "Morrigai",
	"Valindorians", "Draconari", "Lycanthrians", "Elysari", "Cyndrathans",
	"Talarans", "Nexorian", "Korrinthians", "Mythrandir", "Aurelianis",
};

Culture::Culture(int id)
	:m_id(id)
{
	Map* map = GetCurMap();
	m_color = Rgba8( (unsigned char)map->m_mapRNG->RollRandomIntInRange( 100, 200 ), (unsigned char)map->m_mapRNG->RollRandomIntInRange( 100, 200 ), (unsigned char)map->m_mapRNG->RollRandomIntInRange( 100, 200 ) );
	m_name = map->m_cultureNameGenerator->GenerateCultureName();
	m_countryNameGenerator = new CountryNameGenerator( map->m_cultureNameSeed + 3 + m_id, m_name );
	m_provinceNameGenerator = new ProvinceNameGenerator( map->m_cultureNameSeed + 3 + (int)map->m_cultures.size() + m_id, m_name );
	m_cityTownNameGenerator = new CityTownNameGenerator( map->m_cultureNameSeed + 3 + 2 * (int)map->m_cultures.size() + m_id, m_name );
	/*if (map->m_unchosenCultureNames.size() > 0) {
		int nameIndex = map->m_mapRNG->RollRandomIntLessThan( (int)map->m_unchosenCultureNames.size() );
		m_name = map->m_unchosenCultureNames[nameIndex];
		map->m_unchosenCultureNames.erase( map->m_unchosenCultureNames.begin() + nameIndex );
	}
	else {
		m_name = Stringf( "Culture%d", m_id );
	}*/

	// generate culture traits
	int numOfTraits = 0;
 	while (numOfTraits < 4) {
// 		if (m_origin == CultureGeoOrigin::ForestOrigin) {
// 
// 		}
// 		else if (m_origin == CultureGeoOrigin::GrasslandOrigin) {
// 
// 		}
// 		else if (m_origin == CultureGeoOrigin::MountainOrigin) {
// 
// 		}
// 		else if (m_origin == CultureGeoOrigin::OceanOrigin) {
// 
// 		}
// 		else if (m_origin == CultureGeoOrigin::RiverOrigin) {
// 
// 		}
		int index = 0;
		do {
			index = GetCurMap()->m_mapRNG->RollRandomIntLessThan( (int)CultureTrait::NUM );
		} while (HasTrait( CultureTrait( index ) ));
		m_traits[numOfTraits] = (CultureTrait)index;
		++numOfTraits;
	}
}

int Culture::GetNumOfMajorCultureProvs() const
{
	Map* map = GetCurMap();
	int retValue = 0;
	for (auto prov : map->m_mapPolygonUnits) {
		if (prov->m_majorCulture == this && !prov->IsWater() && !prov->m_isFarAwayFakeUnit) {
			++retValue;
		}
	}
	return retValue;
}

void Culture::GetAllMajorCultureProvs( std::vector<MapPolygonUnit*>& out_provs ) const
{
	Map* map = GetCurMap();
	out_provs.clear();
	for (auto prov : map->m_mapPolygonUnits) {
		if (prov->m_majorCulture == this && !prov->IsWater() && !prov->m_isFarAwayFakeUnit) {
			out_provs.push_back( prov );
		}
	}
}

void Culture::GetBoundsPointsForLabel( Vec2& out_startPos, Vec2& out_endPos ) const
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
		if (prov->m_majorCulture == this && !prov->m_isFarAwayFakeUnit && !prov->IsWater()) {
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
		out_startPos = Interpolate( intrStartPos, intrEndPos, 0.1f );
		out_endPos = Interpolate( intrStartPos, intrEndPos, 0.9f );
	}
	else if (LBToRBDist == std::max( LTToRTDist, std::max( LBToRBDist, std::max( LTToRBDist, LBToRTDist ) ) )) {
		Vec2 intrStartPos = Interpolate( leftTopPos, leftBottomPos, 0.5f );
		Vec2 intrEndPos = Interpolate( rightTopPos, rightBottomPos, 0.5f );
		out_startPos = Interpolate( intrStartPos, intrEndPos, 0.1f );
		out_endPos = Interpolate( intrStartPos, intrEndPos, 0.9f );
	}
	else if (LTToRBDist == std::max( LTToRTDist, std::max( LBToRBDist, std::max( LTToRBDist, LBToRTDist ) ) )) {
		out_startPos = Interpolate( leftTopPos, rightBottomPos, 0.1f );
		out_endPos = Interpolate( leftTopPos, rightBottomPos, 0.9f );
	}
	else {
		out_startPos = Interpolate( leftBottomPos, rightTopPos, 0.1f );
		out_endPos = Interpolate( leftBottomPos, rightTopPos, 0.9f );
	}

}

bool Culture::HasTrait( CultureTrait trait ) const
{
	if (m_traits[0] == trait) {
		return true;
	}
	if (m_traits[1] == trait) {
		return true;
	}
	if (m_traits[2] == trait) {
		return true;
	}
	if (m_traits[3] == trait) {
		return true;
	}
	return false;
}
