#include "Game/Town.hpp"
#include "Game/MapPolygonUnit.hpp"
#include "Game/Map.hpp"
#include "Game/label.hpp"
#include "Game/Culture.hpp"
#include "Game/NameGenerator.hpp"
#include "Game/Country.hpp"

Town::Town()
{
}

Town::~Town()
{

}

void Town::Initialize()
{
	Map* map = GetCurMap();
	m_name = Stringf( "Town%d", m_id );

	int basePopulation = RoundDownToInt( m_provIn->m_totalPopulation * 0.02f );

	for (auto unit : m_provIn->m_adjacentUnits) {
		basePopulation += RoundDownToInt( unit->m_totalPopulation * 0.003f );
	}

	m_totalPopulation = basePopulation;
	m_cultures = m_provIn->m_cultures;
	m_religions = m_provIn->m_religions;
	m_majorCulture = m_provIn->m_majorCulture;
	m_majorReligion = m_provIn->m_majorReligion;

	map->m_townLabels.push_back( new Label( this, LabelType::Town ) );
	m_name = m_majorCulture->m_cityTownNameGenerator->GenerateTownName( this );
}

void Town::GrowPopulationOneMonth()
{
	if (m_provIn->IsBeingSieged()) {
		m_totalPopulation = int( m_totalPopulation * 0.99f );
		return;
	}
	if (m_warFlag) {
		m_warFlag = false;
		m_totalPopulation = int( m_totalPopulation * 0.95f );
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
	if (m_provIn->m_totalPopulation > m_totalPopulation * 4) {
		numOfImmigrants = m_provIn->m_totalPopulation / 50000;
		m_provIn->m_totalPopulation -= numOfImmigrants;
		m_totalPopulation += numOfImmigrants;
	}

	if (GetCurMap()->m_historyRNG->RollRandomFloatZeroToOne() < 0.06f) {
		m_totalPopulation = int( m_totalPopulation * 0.994f );
	}
	/*m_owner->m_totalPopulation += (numOfImmigrants + numOfGrowth);
	m_owner->AddCulturePopulation( numOfImmigrants + numOfGrowth, m_cultures );
	m_owner->AddReligionPopulation( numOfImmigrants + numOfGrowth, m_religions );*/
}

float Town::GetCultureInfluence(Culture* culture)
{
	for (auto& pair : m_cultures) {
		if (pair.first == culture) {
			return pair.second;
		}
	}
	return 0.f;
}

float Town::GetReligionInfluence(Religion* religion)
{
	for (auto& pair : m_religions) {
		if (pair.first == religion) {
			return pair.second;
		}
	}
	return 0.f;
}

void Town::RecalculateMajorCulture()
{
	std::sort( m_cultures.begin(), m_cultures.end(), []( std::pair<Culture*, float> const& a, std::pair<Culture*, float> const& b ) { return a.second > b.second; } );
	if (m_cultures.size() > 0) {
		m_majorCulture = m_cultures[0].first;
	}
}

void Town::RecalculateMajorReligion()
{
	std::sort( m_religions.begin(), m_religions.end(), []( std::pair<Religion*, float> const& a, std::pair<Religion*, float> const& b ) { return a.second > b.second; } );
	if (m_religions.size() > 0) {
		m_majorReligion = m_religions[0].first;
	}
}

void Town::ResolveChangePopulation( int prevPopulaiton )
{
	int popDiff = m_totalPopulation - prevPopulaiton;
	if (m_owner) {
		m_owner->m_totalPopulation += popDiff;
		m_owner->AddCulturePopulation( popDiff, m_cultures );
		m_owner->AddReligionPopulation( popDiff, m_religions );
		m_owner->CalculateMajorCulture();
		m_owner->CalculateMajorReligion();
	}
}

void Town::GetUnselectedCultures( std::vector<Culture*>& out_cultures )
{
	out_cultures.clear();
	Map* map = GetCurMap();
	for (auto culture : map->m_cultures) {
		if (GetCultureInfluence( culture ) == 0.f) {
			out_cultures.push_back( culture );
		}
	}
}

void Town::GetUnselectedReligions( std::vector<Religion*>& out_religions )
{
	out_religions.clear();
	Map* map = GetCurMap();
	for (auto religion : map->m_religions) {
		if (GetReligionInfluence( religion ) == 0.f) {
			out_religions.push_back( religion );
		}
	}
}

void Town::SqueezeReligionInfluence( Religion* religion, float influenceToAdd, float prevValue )
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

void Town::SqueezeCultureInfluence( Culture* culture, float influenceToAdd, float prevValue )
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

