#include "Game/City.hpp"
#include "Game/MapPolygonUnit.hpp"
#include "Game/Map.hpp"
#include "Game/River.hpp"
#include "Game/label.hpp"
#include "Game/Culture.hpp"
#include "Game/NameGenerator.hpp"
#include "Game/Country.hpp"
#include "Game/Army.hpp"

City::City()
{

}

void City::Initialize()
{
	Map* map = GetCurMap();

	if (((m_type & CITY_FLAG_ADJ_RIVER) && m_provIn->m_riverOnThis->GetRiverStartEndDistSquared() > 100.f) || (m_type & CITY_FLAG_ADJ_SEA)) {
		AddAttribute( CityAttribute::Port );
	}

	float commercialChance = map->m_mapRNG->RollRandomFloatZeroToOne();
	if (m_provIn->m_height < 500.f) {
		commercialChance += 0.1f;
	}
	else if (m_provIn->m_height > 2000.f) {
		commercialChance -= 0.2f;
	}
	if (m_type & CITY_FLAG_ADJ_RIVER) {
		commercialChance += 0.3f;
	}
	if (m_type & CITY_FLAG_ADJ_SEA) {
		commercialChance += 0.1f;
	}
	if (m_type & CITY_FLAG_PORT) {
		commercialChance += 0.1f;
	}
	if (commercialChance > 0.8f) {
		AddAttribute( CityAttribute::Commercial );
	}

	float fortChance = map->m_mapRNG->RollRandomFloatZeroToOne();
	if (m_provIn->m_height > 1000.f) {
		fortChance += 0.2f;
	}
	if (fortChance > 0.9f) {
		AddAttribute( CityAttribute::Fort );
	}

	int basePopulation = RoundDownToInt( m_provIn->m_totalPopulation * 0.06f );

	for (auto unit : m_provIn->m_adjacentUnits) {
		if (unit->m_cities.size() > 0) {
			basePopulation += RoundDownToInt( unit->m_totalPopulation * 0.005f );
		}
		else {
			basePopulation += RoundDownToInt( unit->m_totalPopulation * 0.02f );
		}
	}

	m_defense = 1000.f;

	if (m_type & CITY_FLAG_FORT) {
		basePopulation = int( basePopulation * 0.75f );
		m_defense += 2000.f;
	}
	if (m_type & CITY_FLAG_COMMERCIAL) {
		basePopulation = int( basePopulation * 1.3f );
	}
	if (m_type & CITY_FLAG_PORT) {
		basePopulation = int( basePopulation * 1.5f );
	}

	m_totalPopulation = basePopulation;
	m_cultures = m_provIn->m_cultures;
	m_religions = m_provIn->m_religions;
	m_majorCulture = m_provIn->m_majorCulture;
	m_majorReligion = m_provIn->m_majorReligion;

	if (m_type & CITY_FLAG_CAPITAL) {
		m_defense += 1000.f;
	}
	m_maxDefense = m_defense;
	map->m_cityLabels.push_back( new Label( this, LabelType::City ) );

	// calculate city height
	// find the triangle where the city is in
	// use barycentric coordinate
	for (auto edge : m_provIn->m_edges) {
		Vec3 barycentric = GetBarycentricCoordinate( m_position, edge->m_startPos, edge->m_endPos, m_provIn->m_geoCenterPos );
		if (barycentric.x >= 0.f && barycentric.y >= 0.f && barycentric.z >= 0.f) {
			m_height = m_provIn->m_height * HEIGHT_FACTOR * barycentric.z + edge->m_startHeight * barycentric.x + edge->m_next->m_startHeight * barycentric.y;
			break;
		}
	}
	if (m_height == 0.f) {
		m_height = m_provIn->m_height;
	}
}

void City::GrowPopulationOneMonth()
{
	if (m_provIn->IsBeingSieged()) {
		m_totalPopulation = int( m_totalPopulation * 0.992f );
		return;
	}
	if (m_warFlag) {
		m_warFlag = false;
		m_totalPopulation = int( m_totalPopulation * 0.9f );
		return;
	}
	float cultureFactor = 1.f;
	if (m_owner->m_countryCulture->HasTrait( CultureTrait::Filial )) {
		cultureFactor += 0.05f;
	}
	if (m_owner->m_countryCulture->HasTrait( CultureTrait::Nomadic )) {
		cultureFactor -= 0.1f;
	}
	if (m_majorCulture->HasTrait( CultureTrait::Sedentary )) {
		cultureFactor += 0.05f;
	}
	if (m_majorCulture->HasTrait( CultureTrait::Communal )) {
		cultureFactor += 0.05f;
	}
	int numOfGrowth = int( m_totalPopulation * 0.0002f * cultureFactor );
	if (numOfGrowth == 0) {
		numOfGrowth = 1;
	}
	m_totalPopulation += numOfGrowth;
	int numOfImmigrants = 0;
	for(auto prov: m_provIn->m_adjacentUnits) {
		if (!prov->IsWater() && prov->m_owner == m_owner && !prov->m_isFarAwayFakeUnit) {
			int rate = 50000;
			if (m_type & CITY_FLAG_PORT) {
				rate = 35000;
			}
			else if (m_type & CITY_FLAG_COMMERCIAL) {
				rate = 40000;
			}
			numOfImmigrants = RoundDownToInt( (float)prov->m_totalPopulation / (float)rate * (1.f - GetClamped( (float)m_totalPopulation / 100000.f, 0.f, 1.f )) );
			prov->m_totalPopulation -= numOfImmigrants;
			m_totalPopulation += numOfImmigrants;
		}
	}
	// pandemic
	if (GetCurMap()->m_historyRNG->RollRandomFloatZeroToOne() <  0.06f) {
		m_totalPopulation = int( m_totalPopulation * 0.993f );
	}
	/*m_owner->m_totalPopulation += (numOfImmigrants + numOfGrowth);
	m_owner->AddCulturePopulation( numOfImmigrants + numOfGrowth, m_cultures );
	m_owner->AddReligionPopulation( numOfImmigrants + numOfGrowth, m_religions );*/
}

void City::Restore()
{
	if ((int)m_provIn->m_armiesOnProv.size() == 0 || (m_provIn->m_armiesOnProv[0].first->m_owner == m_owner)) {
		m_defense += m_maxDefense * 0.1f;
		m_defense = GetClamped( m_defense, 0.f, m_maxDefense );
	}
}
/*
#define CITY_FLAG_COMMERCIAL				0x0001
#define CITY_FLAG_FORT						0x0002
#define CITY_FLAG_NOMAD						0x0004
#define CITY_FLAG_TRIBE						0x0008
#define CITY_FLAG_CAPITAL					0x0010
#define CITY_FLAG_POLITICAL_CENTER			0x0020
#define CITY_FLAG_PORT						0x0040
#define CITY_FLAG_HOLY_CITY					0x0080
#define CITY_FLAG_ADJ_RIVER					0x0100
#define CITY_FLAG_ADJ_SEA					0x0200
*/
std::string City::GetCityAttributeAsString() const
{
	std::string attributes;
	if (m_type & CITY_FLAG_COMMERCIAL) {
		attributes += "Commercial ";
	}
	if (m_type & CITY_FLAG_FORT) {
		attributes += "Fort ";
	}
	if (m_type & CITY_FLAG_NOMAD) {
		attributes += "Nomad ";
	}
	if (m_type & CITY_FLAG_TRIBE) {
		attributes += "Tribe ";
	}
	if (m_type & CITY_FLAG_CAPITAL) {
		attributes += "Capital ";
	}
	if (m_type & CITY_FLAG_POLITICAL_CENTER) {
		attributes += "Political_Center ";
	}
	if (m_type & CITY_FLAG_PORT) {
		attributes += "Port ";
	}
	if (m_type & CITY_FLAG_HOLY_CITY) {
		attributes += "Holy_City ";
	}
	if (m_type & CITY_FLAG_ADJ_RIVER) {
		attributes += "Adjacent_River ";
	}
	if (m_type & CITY_FLAG_ADJ_SEA) {
		attributes += "Adjacent_Sea ";
	}

	return attributes.substr( 0, attributes.size() - 1 );
}

void City::GetUnownedAttributes( std::vector<CityAttribute>& out_attributes ) const
{
	out_attributes.clear();
	if ((m_type & CITY_FLAG_COMMERCIAL) == 0) {
		out_attributes.push_back( CityAttribute::Commercial );
	}
	if ((m_type & CITY_FLAG_FORT) == 0) {
		out_attributes.push_back( CityAttribute::Fort );
	}
	if ((m_type & CITY_FLAG_CAPITAL) == 0) {
		out_attributes.push_back( CityAttribute::Capital );
	}
	if ((m_type & CITY_FLAG_PORT) == 0) {
		out_attributes.push_back( CityAttribute::Port );
	}
	if ((m_type & CITY_FLAG_ADJ_RIVER) == 0) {
		out_attributes.push_back( CityAttribute::AdjToRiver );
	}
	if ((m_type & CITY_FLAG_ADJ_SEA) == 0) {
		out_attributes.push_back( CityAttribute::AdjToSea );
	}
}

void City::AddAttribute( CityAttribute attr )
{
	if (attr == CityAttribute::AdjToRiver) {
		m_type |= CITY_FLAG_ADJ_RIVER;
	}
	else if (attr == CityAttribute::AdjToSea) {
		m_type |= CITY_FLAG_ADJ_SEA;
	}
	else if (attr == CityAttribute::Capital) {
		m_type |= CITY_FLAG_CAPITAL;
	}
	else if (attr == CityAttribute::Commercial) {
		m_type |= CITY_FLAG_COMMERCIAL;
	}
	else if (attr == CityAttribute::Fort) {
		m_type |= CITY_FLAG_FORT;
	}
	else if (attr == CityAttribute::Port) {
		m_type |= CITY_FLAG_PORT;
	}
	if (std::find( m_attributes.begin(), m_attributes.end(), attr ) == m_attributes.end()) {
		m_attributes.push_back( attr );
	}
}

bool City::HasAttribute( CityAttribute attr ) const
{
	if (attr == CityAttribute::AdjToRiver) {
		return m_type & CITY_FLAG_ADJ_RIVER;
	}
	else if (attr == CityAttribute::AdjToSea) {
		return m_type & CITY_FLAG_ADJ_SEA;
	}
	else if (attr == CityAttribute::Capital) {
		return m_type & CITY_FLAG_CAPITAL;
	}
	else if (attr == CityAttribute::Commercial) {
		return m_type & CITY_FLAG_COMMERCIAL;
	}
	else if (attr == CityAttribute::Fort) {
		return m_type & CITY_FLAG_FORT;
	}
	else if (attr == CityAttribute::Port) {
		return m_type & CITY_FLAG_PORT;
	}
	return false;
}

void City::RemoveAttribute( CityAttribute attr )
{
	auto foundIter = std::find( m_attributes.begin(), m_attributes.end(), attr );
	if (foundIter != m_attributes.end()) {
		m_attributes.erase( foundIter );
	}
	if (attr == CityAttribute::AdjToRiver) {
		m_type ^= CITY_FLAG_ADJ_RIVER;
	}
	else if (attr == CityAttribute::AdjToSea) {
		m_type ^= CITY_FLAG_ADJ_SEA;
	}
	else if (attr == CityAttribute::Capital) {
		m_type ^= CITY_FLAG_CAPITAL;
	}
	else if (attr == CityAttribute::Commercial) {
		m_type ^= CITY_FLAG_COMMERCIAL;
	}
	else if (attr == CityAttribute::Fort) {
		m_type ^= CITY_FLAG_FORT;
	}
	else if (attr == CityAttribute::Port) {
		m_type ^= CITY_FLAG_PORT;
	}
}

void City::SyncAttributes()
{
	if (m_type & CITY_FLAG_PORT) {
		if (std::find( m_attributes.begin(), m_attributes.end(), CityAttribute::Port ) == m_attributes.end()) {
			m_attributes.push_back( CityAttribute::Port );
		}
	}
	if (m_type & CITY_FLAG_FORT) {
		if (std::find( m_attributes.begin(), m_attributes.end(), CityAttribute::Fort ) == m_attributes.end()) {
			m_attributes.push_back( CityAttribute::Fort );
		}
	}
	if (m_type & CITY_FLAG_CAPITAL) {
		if (std::find( m_attributes.begin(), m_attributes.end(), CityAttribute::Capital ) == m_attributes.end()) {
			m_attributes.push_back( CityAttribute::Capital );
		}
	}
	if (m_type & CITY_FLAG_COMMERCIAL) {
		if (std::find( m_attributes.begin(), m_attributes.end(), CityAttribute::Commercial ) == m_attributes.end()) {
			m_attributes.push_back( CityAttribute::Commercial );
		}
	}
	if (m_type & CITY_FLAG_ADJ_SEA) {
		if (std::find( m_attributes.begin(), m_attributes.end(), CityAttribute::AdjToSea ) == m_attributes.end()) {
			m_attributes.push_back( CityAttribute::AdjToSea );
		}
	}
	if (m_type & CITY_FLAG_ADJ_RIVER) {
		if (std::find( m_attributes.begin(), m_attributes.end(), CityAttribute::AdjToRiver ) == m_attributes.end()) {
			m_attributes.push_back( CityAttribute::AdjToRiver );
		}
	}
}

uint16_t City::GetRawAttribute() const
{
	return m_type;
}

void City::SetRawAttribute( uint16_t attr )
{
	m_type = attr;
}
