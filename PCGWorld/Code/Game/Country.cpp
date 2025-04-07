#include "Game/Country.hpp"
#include "Game/Map.hpp"
#include "Game/City.hpp"
#include "Game/Army.hpp"
#include "Game/CountryInstructions.hpp"
#include "Game/Battle.hpp"
#include "Game/Culture.hpp"
#include "Game/NameGenerator.hpp"

Country::Country()
{
	Map* map = GetCurMap();
	do {
		m_color = Rgba8( (unsigned char)map->m_mapRNG->RollRandomIntInRange( 50, 250 ), (unsigned char)map->m_mapRNG->RollRandomIntInRange( 50, 250 ), (unsigned char)map->m_mapRNG->RollRandomIntInRange( 50, 250 ) );
	} while (m_color == map->m_renderPreference.m_oceanColor);
}

Country::~Country()
{
	for (auto army : m_armies) {
		delete army;
	}
	delete m_edgeShowingBuffer;
	delete m_edgeShowingBuffer3D;
}

void Country::Reinitialize()
{
	m_funds = 0;
	m_economyValue = 0;
	m_totalMilitaryStrength = 0;

	std::vector<Army*> armiesCopy = m_armies;
	for (auto army : armiesCopy) {
		RemoveArmyFromMap( army, GetCurMap() );
	}
	RemoveAllRelations();

	m_capitalProv = nullptr;
	m_capitalCity = nullptr;
	m_provinces.clear();
	m_cities.clear();
	m_towns.clear();
	m_cultures.clear();
	m_religions.clear();
	m_majorCulture = nullptr;
	m_majorReligion = nullptr;
	m_totalPopulation = 0;
	m_countryCulture = nullptr;
	m_countryReligion = nullptr;
	m_armies.clear();

	m_relationFriendlyCountries.clear();
	m_relationAllianceCountries.clear();
	m_relationHostileCountries.clear();
	m_relationWarCountries.clear();
	m_warTime.clear();

	m_relationSuzerain = nullptr;
	m_relationVassals.clear();

	m_isCelestial = false;
	m_relationCelestialEmpire = nullptr;
	m_relationTributaries.clear();

	m_tribeUnions.clear();

}

void Country::GainProvince( MapPolygonUnit* prov, bool resetCapital )
{
	//m_mutex.lock();
	if (std::find( m_provinces.begin(), m_provinces.end(), prov ) != m_provinces.end()) {
		ERROR_RECOVERABLE( "Cannot add province that is already owned by this country!" );
	}
	else {
		m_provinces.push_back( prov );
		m_totalPopulation += prov->m_totalPopulation;
		AddCulturePopulation( prov->m_totalPopulation, prov->m_cultures );
		AddReligionPopulation( prov->m_totalPopulation, prov->m_religions );
		prov->m_owner = this;
		for (auto city : prov->m_cities) {
			m_cities.push_back( city );
			city->m_owner = this;
			m_totalPopulation += city->m_totalPopulation;
			AddCulturePopulation( city->m_totalPopulation, city->m_cultures );
			AddReligionPopulation( city->m_totalPopulation, city->m_religions );
		}
		for (auto town : prov->m_towns) {
			m_towns.push_back( town );
			town->m_owner = this;
			m_totalPopulation += town->m_totalPopulation;
			AddCulturePopulation( town->m_totalPopulation, town->m_cultures );
			AddReligionPopulation( town->m_totalPopulation, town->m_religions );
		}
		CalculateMajorCulture();
		CalculateMajorReligion();
		if ((int)m_provinces.size() == 1 && resetCapital) {
			m_capitalProv = m_provinces[0];
		}
	}
	//m_mutex.unlock();
}

void Country::LoseProvince( MapPolygonUnit* prov, bool resetCapital )
{
	//m_mutex.lock();
	auto iter = std::find( m_provinces.begin(), m_provinces.end(), prov );
	if (iter != m_provinces.end()) {
		prov->m_owner = nullptr;
		m_totalPopulation -= prov->m_totalPopulation;
		AddCulturePopulation( -prov->m_totalPopulation, prov->m_cultures );
		AddReligionPopulation( -prov->m_totalPopulation, prov->m_religions );
		for (auto city : prov->m_cities) {
			m_cities.erase( std::find( m_cities.begin(), m_cities.end(), city ) );
			city->m_owner = nullptr;
			m_totalPopulation -= city->m_totalPopulation;
			AddCulturePopulation( -city->m_totalPopulation, city->m_cultures );
			AddReligionPopulation( -city->m_totalPopulation, city->m_religions );
		}
		for (auto town : prov->m_towns) {
			m_towns.erase( std::find( m_towns.begin(), m_towns.end(), town ) );
			town->m_owner = nullptr;
			m_totalPopulation -= town->m_totalPopulation;
			AddCulturePopulation( -town->m_totalPopulation, town->m_cultures );
			AddReligionPopulation( -town->m_totalPopulation, town->m_religions );
		}
		m_provinces.erase( iter );
	}
	else {
		ERROR_RECOVERABLE( "Cannot lose province that is not owned by this country!" );
	}
	CalculateMajorCulture();
	CalculateMajorReligion();
	if ((int)m_provinces.size() == 0 && resetCapital) {
		RemoveAllRelations();
	}
	if (prov == m_capitalProv && resetCapital) {
		if (m_capitalCity) {
			m_capitalCity->RemoveAttribute( CityAttribute::Capital );
			m_capitalCity = nullptr;
		}
		ResetCapitalProvince();
	}
	
	//m_mutex.unlock();
}

void Country::AnnexCountry( Country* country )
{
	std::vector<MapPolygonUnit*> provs = country->m_provinces;
	//country->m_capitalProv = nullptr;
	//if (country->m_capitalCity) {
	//	country->m_capitalCity->m_type &= ~CITY_FLAG_CAPITAL;
	//	country->m_capitalCity = nullptr;
	//}
	for (auto prov : provs) {
		country->LoseProvince( prov, true );
	}
	for (auto prov : provs) {
		GainProvince( prov );
	}
}

void Country::GetAdjacentCountries( std::vector<Country*>& out_countries ) const
{
	out_countries.clear();
	for (auto prov : m_provinces) {
		for (auto adjProv : prov->m_adjacentUnits) {
			if(adjProv->m_owner && adjProv->m_owner != this && std::find(out_countries.begin(), out_countries.end(), adjProv->m_owner) == out_countries.end()){
				out_countries.push_back( adjProv->m_owner );
			}
		}
	}
}

bool Country::IsExist() const
{
	return (int)m_provinces.size() != 0;
}

bool Country::IsVassal() const
{
	return m_relationSuzerain != nullptr;
}

bool Country::IsTributary() const
{
	return m_relationCelestialEmpire != nullptr;
}

bool Country::IsIndependent() const
{
	return !IsVassal() && !IsTributary();
}

bool Country::IsCelestial() const
{
	return m_isCelestial;
}

bool Country::IsInWar() const
{
	return (int)m_relationWarCountries.size() > 0;
}

bool Country::HasVassal() const
{
	return (int)m_relationVassals.size() > 0;
}

bool Country::HasTributary() const
{
	return (int)m_relationTributaries.size() > 0;
}

bool Country::IsMemberOfUnion() const
{
	return (int)m_tribeUnions.size() > 0;
}

bool Country::IsAdjacentToCountry( Country* country ) const
{
	for (auto prov : m_provinces) {
		for (auto adjProv : prov->m_adjacentUnits) {
			if (adjProv->m_owner && adjProv->m_owner == country) {
				return true;
			}
		}
	}
	return false;
}

CountryRelationType Country::GetRelationTo( Country* country ) const
{
	if (country == this) {
		return CountryRelationType::Self;
	}
	else if (IsFriendlyWith( country )) {
		return CountryRelationType::Friendly;
	}
	else if (IsInWarWith( country )) {
		return CountryRelationType::War;
	}
	else if (IsHostileWith( country )) {
		return CountryRelationType::Hostile;
	}
	else if (IsAlliedWith( country )) {
		return CountryRelationType::Alliance;
	}
	else if (IsInTribeUnionWith( country )) {
		return CountryRelationType::TribeUnion;
	}
	else if (IsTributaryOf( country )) {
		return CountryRelationType::Tributary;
	}
	else if (IsSuzerainOf( country )) {
		return CountryRelationType::Suzerain;
	}
	else if (IsCelestialOf( country )) {
		return CountryRelationType::Celestial;
	}
	else if (IsVassalOf( country )) {
		return CountryRelationType::Vassal;
	}
	return CountryRelationType::None;
}

bool Country::HasNoRelationToCountry( Country* country ) const
{
	CountryRelationType relation = GetRelationTo( country );
	return relation == CountryRelationType::None;
}

bool Country::HasNoRelationToCountryList( std::vector<Country*> const& countries ) const
{
	for (auto country : countries) {
		if (!HasNoRelationToCountry( country )) {
			return false;
		}
	}
	return true;
}

void Country::RemoveAllRelations()
{
	while (!m_relationFriendlyCountries.empty()) {
		RemoveFriendlyRelation( m_relationFriendlyCountries[0] );
	}
	while (!m_relationAllianceCountries.empty()) {
		RemoveAllianceRelation( m_relationAllianceCountries[0] );
	}
	while (!m_relationHostileCountries.empty()) {
		RemoveHostileRelation( m_relationHostileCountries[0] );
	}
	while (!m_relationWarCountries.empty()) {
		RemoveWarRelation( m_relationWarCountries[0] );
	}
	while (!m_tribeUnions.empty()) {
		RemoveTribeUnionRelation( m_tribeUnions[0] );
	}
	if (m_relationSuzerain) {
		RemoveBeingVassalRelation( m_relationSuzerain );
	}
	if (m_relationCelestialEmpire) {
		RemoveBeingTributaryRelation( m_relationCelestialEmpire );
	}
	while (!m_relationTributaries.empty()) {
		RemoveTributaryRelation( m_relationTributaries[0] );
	}
	while (!m_relationVassals.empty()) {
		RemoveVassalRelation( m_relationVassals[0] );
	}
}

void Country::RemoveAllRelationWithCountry( Country* country )
{
	if (IsFriendlyWith(country)) {
		RemoveFriendlyRelation( country );
	}
	if (IsAlliedWith( country )) {
		RemoveAllianceRelation( country );
	}
	if (IsHostileWith( country )) {
		RemoveHostileRelation( country );
	}
	if (IsInWarWith( country )) {
		RemoveWarRelation( country );
	}
	if (IsVassalOf( country )) {
		RemoveBeingVassalRelation( country );
	}
	if (IsTributaryOf( country )) {
		RemoveBeingTributaryRelation( country );
	}
	if (IsCelestial()) {
		RemoveTributaryRelation( country );
	}
	if (IsSuzerainOf(country)) {
		RemoveVassalRelation( country );
	}
}

void Country::AddRelationWithCountry( Country* country, CountryRelationType relation )
{
	switch (relation)
	{
	case CountryRelationType::Alliance:
		AddAllianceRelation( country ); break;
	case CountryRelationType::Celestial:
		AddTributaryRelation( country ); break;
	case CountryRelationType::Friendly:
		AddFriendlyRelation( country ); break;
	case CountryRelationType::Hostile:
		AddHostileRelation( country ); break;
	case CountryRelationType::None:
		RemoveAllRelationWithCountry( country ); break;
	case CountryRelationType::Self:
		if (this != country) { ERROR_RECOVERABLE("cannot add self relation to another country"); }; break;
	case CountryRelationType::Suzerain:
		AddVassalRelation( country ); break;
	case CountryRelationType::TribeUnion:
		AddTribeUnionRelation( country ); break;
	case CountryRelationType::Tributary:
		AddBeingTributaryRelation( country ); break;
	case CountryRelationType::Vassal:
		AddBeingVassalRelation( country ); break;
	case CountryRelationType::War:
		AddWarRelation( country ); break;
	}
}

void Country::AddFriendlyRelation( Country* country )
{
	if (!country->IsExist() || !IsExist()) {
		return;
	}
	if (std::find( country->m_relationFriendlyCountries.begin(), country->m_relationFriendlyCountries.end(), this ) != country->m_relationFriendlyCountries.end()) {
		ERROR_RECOVERABLE( "This country is already in relationship list, are you intended to add it?" );
	}
	else {
		country->m_relationFriendlyCountries.push_back( this );
	}
	if (std::find( m_relationFriendlyCountries.begin(), m_relationFriendlyCountries.end(), country ) != m_relationFriendlyCountries.end()) {
		ERROR_RECOVERABLE( "This country is already in relationship list, are you intended to add it?" );
	}
	else {
		m_relationFriendlyCountries.push_back( country );
	}
}

void Country::RemoveFriendlyRelation( Country* country )
{
	bool findOneCheck = false;
	for (int i = 0; i < (int)m_relationFriendlyCountries.size(); i++) {
		if (country == m_relationFriendlyCountries[i]) {
			m_relationFriendlyCountries.erase( i + m_relationFriendlyCountries.begin() );
			findOneCheck = true;
			break;
		}
	}
	if (!findOneCheck) {
		ERROR_RECOVERABLE( "Cannot find a country friendly relationship to remove!" );
	}
	findOneCheck = false;
	for (int i = 0; i < (int)country->m_relationFriendlyCountries.size(); i++) {
		if (this == country->m_relationFriendlyCountries[i]) {
			country->m_relationFriendlyCountries.erase( i + country->m_relationFriendlyCountries.begin() );
			findOneCheck = true;
			break;
		}
	}
	if (!findOneCheck) {
		ERROR_RECOVERABLE( "Cannot find a country friendly relationship to remove!" );
	}
}

void Country::AddAllianceRelation( Country* country )
{
	if (!country->IsExist() || !IsExist()) {
		return;
	}
	if (std::find( country->m_relationAllianceCountries.begin(), country->m_relationAllianceCountries.end(), this ) != country->m_relationAllianceCountries.end()) {
		ERROR_RECOVERABLE( "This country is already in relationship list, are you intended to add it?" );
	}
	else {
		country->m_relationAllianceCountries.push_back( this );
	}
	if (std::find( m_relationAllianceCountries.begin(), m_relationAllianceCountries.end(), country ) != m_relationAllianceCountries.end()) {
		ERROR_RECOVERABLE( "This country is already in relationship list, are you intended to add it?" );
	}
	else {
		m_relationAllianceCountries.push_back( country );
	}
}

void Country::RemoveAllianceRelation( Country* country )
{
	bool findOneCheck = false;
	for (int i = 0; i < (int)m_relationAllianceCountries.size(); i++) {
		if (country == m_relationAllianceCountries[i]) {
			m_relationAllianceCountries.erase( i + m_relationAllianceCountries.begin() );
			findOneCheck = true;
			break;
		}
	}
	if (!findOneCheck) {
		ERROR_RECOVERABLE( "Cannot find a country alliance relationship to remove!" );
	}
	findOneCheck = false;
	for (int i = 0; i < (int)country->m_relationAllianceCountries.size(); i++) {
		if (this == country->m_relationAllianceCountries[i]) {
			country->m_relationAllianceCountries.erase( i + country->m_relationAllianceCountries.begin() );
			findOneCheck = true;
			break;
		}
	}
	if (!findOneCheck) {
		ERROR_RECOVERABLE( "Cannot find a country alliance relationship to remove!" );
	}
}

void Country::AddHostileRelation( Country* country )
{
	if (!country->IsExist() || !IsExist()) {
		return;
	}
	if (std::find( country->m_relationHostileCountries.begin(), country->m_relationHostileCountries.end(), this ) != country->m_relationHostileCountries.end()) {
		ERROR_RECOVERABLE( "This country is already in relationship list, are you intended to add it?" );
	}
	else {
		country->m_relationHostileCountries.push_back( this );
	}
	if (std::find( m_relationHostileCountries.begin(), m_relationHostileCountries.end(), country ) != m_relationHostileCountries.end()) {
		ERROR_RECOVERABLE( "This country is already in relationship list, are you intended to add it?" );
	}
	else {
		m_relationHostileCountries.push_back( country );
	}
}

void Country::RemoveHostileRelation( Country* country )
{
	bool findOneCheck = false;
	for (int i = 0; i < (int)m_relationHostileCountries.size(); i++) {
		if (country == m_relationHostileCountries[i]) {
			m_relationHostileCountries.erase( i + m_relationHostileCountries.begin() );
			findOneCheck = true;
			break;
		}
	}
	if (!findOneCheck) {
		ERROR_RECOVERABLE( "Cannot find a country hostile relationship to remove!" );
	}
	findOneCheck = false;
	for (int i = 0; i < (int)country->m_relationHostileCountries.size(); i++) {
		if (this == country->m_relationHostileCountries[i]) {
			country->m_relationHostileCountries.erase( i + country->m_relationHostileCountries.begin() );
			findOneCheck = true;
			break;
		}
	}
	if (!findOneCheck) {
		ERROR_RECOVERABLE( "Cannot find a country hostile relationship to remove!" );
	}
}

void Country::AddWarRelation( Country* country )
{
	if (!country->IsExist() || !IsExist()) {
		return;
	}
	if (std::find( country->m_relationWarCountries.begin(), country->m_relationWarCountries.end(), this ) != country->m_relationWarCountries.end()) {
		ERROR_RECOVERABLE( "This country is already in relationship list, are you intended to add it?" );
	}
	else {
		country->m_warTime.push_back( std::pair<Country*, int>( this, GetCurMap()->GetTotalMonthCount() ) );
		country->m_relationWarCountries.push_back( this );
	}
	if (std::find( m_relationWarCountries.begin(), m_relationWarCountries.end(), country ) != m_relationWarCountries.end()) {
		ERROR_RECOVERABLE( "This country is already in relationship list, are you intended to add it?" );
	}
	else {
		m_warTime.push_back( std::pair<Country*, int>( country, GetCurMap()->GetTotalMonthCount() ) );
		m_relationWarCountries.push_back( country );
	}
}

void Country::RemoveWarRelation( Country* country )
{
	bool findOneCheck = false;
	for (int i = 0; i < (int)m_relationWarCountries.size(); i++) {
		if (country == m_relationWarCountries[i]) {
			m_relationWarCountries.erase( i + m_relationWarCountries.begin() );
			findOneCheck = true;
			break;
		}
	}
	for (int i = 0; i < (int)m_warTime.size(); i++) {
		if (country == m_warTime[i].first) {
			m_warTime.erase( i + m_warTime.begin() );
			break;
		}
	}
	if (!findOneCheck) {
		ERROR_RECOVERABLE( "Cannot find a country war relationship to remove!" );
	}
	findOneCheck = false;
	for (int i = 0; i < (int)country->m_relationWarCountries.size(); i++) {
		if (this == country->m_relationWarCountries[i]) {
			country->m_relationWarCountries.erase( i + country->m_relationWarCountries.begin() );
			findOneCheck = true;
			break;
		}
	}
	for (int i = 0; i < (int)country->m_warTime.size(); i++) {
		if (this == country->m_warTime[i].first) {
			country->m_warTime.erase( i + country->m_warTime.begin() );
			break;
		}
	}
	if (!findOneCheck) {
		ERROR_RECOVERABLE( "Cannot find a country war relationship to remove!" );
	}
}

void Country::AddVassalRelation( Country* country )
{
	if (!country->IsExist() || !IsExist()) {
		return;
	}
	country->m_relationSuzerain = this;
	if (std::find( m_relationVassals.begin(), m_relationVassals.end(), country ) != m_relationVassals.end()) {
		ERROR_RECOVERABLE( "This country is already in relationship list, are you intended to add it?" );
	}
	else {
		m_relationVassals.push_back( country );
	}
}

void Country::RemoveVassalRelation( Country* country )
{
	bool findOneCheck = false;
	for (int i = 0; i < (int)m_relationVassals.size(); i++) {
		if (country == m_relationVassals[i]) {
			m_relationVassals.erase( i + m_relationVassals.begin() );
			findOneCheck = true;
			break;
		}
	}
	if (!findOneCheck) {
		ERROR_RECOVERABLE( "Cannot find a country vassal relationship to remove!" );
	}
	if (country->m_relationSuzerain == this) {
		country->m_relationSuzerain = nullptr;
	}
	else {
		ERROR_RECOVERABLE( "Removing a vassal country that it's owner is not this country!" );
	}
}

void Country::AddBeingVassalRelation( Country* country )
{
	if (!country->IsExist() || !IsExist()) {
		return;
	}
	m_relationSuzerain = country;
	if (std::find( country->m_relationVassals.begin(), country->m_relationVassals.end(), this ) != country->m_relationVassals.end()) {
		ERROR_RECOVERABLE( "This country is already in relationship list, are you intended to add it?" );
	}
	else {
		country->m_relationVassals.push_back( this );
	}
}

void Country::RemoveBeingVassalRelation( Country* country )
{
	if (m_relationSuzerain == country) {
		m_relationSuzerain = nullptr;
	}
	else {
		ERROR_RECOVERABLE( "Removing a vassal country that it's owner is not this country!" );
	}
	bool findOneCheck = false;
	for (int i = 0; i < (int)country->m_relationVassals.size(); i++) {
		if (this == country->m_relationVassals[i]) {
			country->m_relationVassals.erase( i + country->m_relationVassals.begin() );
			findOneCheck = true;
			break;
		}
	}
	if (!findOneCheck) {
		ERROR_RECOVERABLE( "Cannot find a country vassal relationship to remove!" );
	}
}

void Country::AddTribeUnionRelation( Country* country )
{
	if (!country->IsExist() || !IsExist()) {
		return;
	}
	if (std::find( country->m_tribeUnions.begin(), country->m_tribeUnions.end(), this ) != country->m_tribeUnions.end()) {
		ERROR_RECOVERABLE( "This country is already in relationship list, are you intended to add it?" );
	}
	else {
		country->m_tribeUnions.push_back( this );
	}
	if (std::find( m_tribeUnions.begin(), m_tribeUnions.end(), country ) != m_tribeUnions.end()) {
		ERROR_RECOVERABLE( "This country is already in relationship list, are you intended to add it?" );
	}
	else {
		m_tribeUnions.push_back( country );
	}
}

void Country::RemoveTribeUnionRelation( Country* country )
{
	bool findOneCheck = false;
	for (int i = 0; i < (int)m_tribeUnions.size(); i++) {
		if (country == m_tribeUnions[i]) {
			m_tribeUnions.erase( i + m_tribeUnions.begin() );
			findOneCheck = true;
			break;
		}
	}
	if (!findOneCheck) {
		ERROR_RECOVERABLE( "Cannot find a country tribe union relationship to remove!" );
	}
	findOneCheck = false;
	for (int i = 0; i < (int)country->m_tribeUnions.size(); i++) {
		if (this == country->m_tribeUnions[i]) {
			country->m_tribeUnions.erase( i + country->m_tribeUnions.begin() );
			findOneCheck = true;
			break;
		}
	}
	if (!findOneCheck) {
		ERROR_RECOVERABLE( "Cannot find a country tribe union relationship to remove!" );
	}
}

void Country::AddTributaryRelation( Country* country )
{
	if (!country->IsExist() || !IsExist()) {
		return;
	}
	country->m_relationCelestialEmpire = this;
	if (std::find( m_relationTributaries.begin(), m_relationTributaries.end(), country ) != m_relationTributaries.end()) {
		ERROR_RECOVERABLE( "This country is already in relationship list, are you intended to add it?" );
	}
	else {
		m_relationTributaries.push_back( country );
	}
}

void Country::RemoveTributaryRelation( Country* country )
{
	bool findOneCheck = false;
	for (int i = 0; i < (int)m_relationTributaries.size(); i++) {
		if (country == m_relationTributaries[i]) {
			m_relationTributaries.erase( i + m_relationTributaries.begin() );
			findOneCheck = true;
			break;
		}
	}
// 	if (!findOneCheck) {
// 		ERROR_RECOVERABLE( "Cannot find a country tributary relationship to remove!" );
// 	}
	if (country->m_relationCelestialEmpire == this) {
		country->m_relationCelestialEmpire = nullptr;
	}
// 	else {
// 		ERROR_RECOVERABLE( "Removing a vassal country that it's owner is not this country!" );
// 	}
}

void Country::AddBeingTributaryRelation( Country* country )
{
	if (!country->IsExist() || !IsExist()) {
		return;
	}
	m_relationCelestialEmpire = country;
	if (std::find( country->m_relationTributaries.begin(), country->m_relationTributaries.end(), this ) != country->m_relationTributaries.end()) {
		ERROR_RECOVERABLE( "This country is already in relationship list, are you intended to add it?" );
	}
	else {
		country->m_relationTributaries.push_back( this );
	}
}

void Country::RemoveBeingTributaryRelation( Country* country )
{
	if (m_relationCelestialEmpire == country) {
		m_relationCelestialEmpire = nullptr;
	}
	else {
		ERROR_RECOVERABLE( "Removing a vassal country that it's owner is not this country!" );
	}
	bool findOneCheck = false;
	for (int i = 0; i < (int)country->m_relationTributaries.size(); i++) {
		if (this == country->m_relationTributaries[i]) {
			country->m_relationTributaries.erase( i + country->m_relationTributaries.begin() );
			findOneCheck = true;
			break;
		}
	}
	if (!findOneCheck) {
		ERROR_RECOVERABLE( "Cannot find a country tributary relationship to remove!" );
	}
}

int Country::GetCulturePopulation( Culture* culture ) const
{
	for (auto& pair : m_cultures) {
		if (pair.first == culture) {
			return pair.second;
		}
	}
	return 0;
}

int Country::GetReligionPopulation( Religion* religion ) const
{
	for (auto& pair : m_religions) {
		if (pair.first == religion) {
			return pair.second;
		}
	}
	return 0;
}

float Country::GetCultureInfluence( Culture* culture ) const
{
	for (auto& pair : m_cultures) {
		if (pair.first == culture) {
			return (float)pair.second / (float)m_totalPopulation;
		}
	}
	return 0.f;
}

float Country::GetReligionInfluence( Religion* religion ) const
{
	for (auto& pair : m_religions) {
		if (pair.first == religion) {
			return (float)pair.second / (float)m_totalPopulation;
		}
	}
	return 0.f;
}

void Country::SetUpSimulation()
{
	//CalculateEconomicValue();
	m_funds = 6 * m_economyValue;
}

void Country::ExecuteCountryBehavior()
{
	//if (m_funds < -m_economyValue) {
	//	return;
	//}

	//if (!m_capitalProv) {
	//	ResetCapitalProvince();
	//}

	Map* map = GetCurMap();
	int income = GetMonthlyIncome();
	// simple drunk ai
	// build new army if not reach the max
	int costToAddThisMonth = 0;
	if (m_funds >= m_economyValue && income > 0) {
		int numOfArmies = std::max( (int)m_provinces.size() / 20, 1 );
		if ((int)m_armies.size() < numOfArmies && (income - ARMY_COST - costToAddThisMonth > 0) && m_funds >= BUILD_ARMY_COST) {
			bool buildArmySuccessfully = false;
			for (auto city : m_cities) {
				if (city->m_provIn->IsLegitimateToCountry( this ) && (int)city->m_provIn->m_armiesOnProv.size() == 0) {
					InstructionBuildArmy* newInstr = new InstructionBuildArmy();
					newInstr->m_prov = city->m_provIn;
					newInstr->m_numOfSoldiersRecruited = 1;
					if ((int)m_armies.size() > 0) {
						SpendMoney( BUILD_ARMY_COST );
						//m_funds -= BUILD_ARMY_COST;
					}
					buildArmySuccessfully = true;
					map->AddInstructionToQueue( newInstr );
					costToAddThisMonth += ARMY_COST;
					break;
				}
			}
			if (!buildArmySuccessfully && m_funds >= BUILD_ARMY_COST) {
				if (m_capitalProv && m_capitalProv->IsLegitimateToCountry( this ) && m_capitalProv->m_armiesOnProv.size() == 0) {
					InstructionBuildArmy* newInstr = new InstructionBuildArmy();
					newInstr->m_prov = m_capitalProv;
					newInstr->m_numOfSoldiersRecruited = 1;
					if ((int)m_armies.size() > 0) {
						SpendMoney( BUILD_ARMY_COST );
					}
					buildArmySuccessfully = true;
					map->AddInstructionToQueue( newInstr );
					costToAddThisMonth += ARMY_COST;
				}
			}
		}
	}
	
	// recruit army
	if (m_funds >= m_economyValue && (int)m_armies.size() != 0 && income > 0) {
		int numOfSoldierToRecruit = std::min( m_funds / (int)m_armies.size() / RECRUIT_SOLDIER_COST / 4, 1000 );
		std::vector<Army*> armyCopy = m_armies;
		std::sort( armyCopy.begin(), armyCopy.end(), []( Army* a, Army* b ) {return a->m_size < b->m_size; } );
		if (numOfSoldierToRecruit > 0) {
			for (auto army : armyCopy) {
				if (m_funds >= m_economyValue && army->m_provIn->IsLegitimateToCountry( this ) && army->m_provIn->m_owner == this) {
					if ((IsInWar() && income - numOfSoldierToRecruit * ARMY_SOLDIER_COST - costToAddThisMonth > -m_funds / 24) 
						|| (!IsInWar() && income - numOfSoldierToRecruit * ARMY_SOLDIER_COST - costToAddThisMonth > m_economyValue / 5)) {
						InstructionRecruitArmy* newInstr = new InstructionRecruitArmy();
						newInstr->m_army = army;
						newInstr->m_numOfSoldiersRecruited = numOfSoldierToRecruit;
						float cultureFactor = 1.f;
						if (m_countryCulture->HasTrait( CultureTrait::Xenophile )) {
							cultureFactor += 0.25f;
						}
						if (m_countryCulture->HasTrait( CultureTrait::Xenophobe )) {
							cultureFactor -= 0.1f;
						}
						if (m_countryCulture->HasTrait( CultureTrait::Pacifistic )) {
							cultureFactor += 0.5f;
						}
						SpendMoney( int( float( numOfSoldierToRecruit * RECRUIT_SOLDIER_COST ) * cultureFactor ) );
						map->AddInstructionToQueue( newInstr );
						costToAddThisMonth += int( float( numOfSoldierToRecruit * ARMY_SOLDIER_COST ) );
					}
				}
			}
		}
	}
	// cut army
	if (income < 0 && !IsInWar()) {
		for (auto army : m_armies) {
			army->m_size = army->m_size / 10 * 8;
		}
	}
	if (m_funds < 0) {
		for (auto army : m_armies) {
			army->m_size = army->m_size / 10 * 8;
		}
	}

	// all army randomly move to adjacent province
	// army move
	std::vector<Province*> provCanGo;
	provCanGo.reserve( 10 );
	
	for (auto army : m_armies) {
		army->GetAllProvincesCanGo( provCanGo );
		if (army->m_goingTarget == army->m_provIn || army->m_goingTarget == nullptr) {
			std::vector<Province*> provTargets;
			provTargets.reserve( 10 );
			// if sieging a province, do not move, will consider to move in the future
			if (army->m_provIn->m_owner && army->m_provIn->m_owner != this) {
				continue;
			}
			if (army->m_provIn->m_armiesOnProv.size() == 0 || (!army->m_provIn->IsWater() && army != army->m_provIn->m_armiesOnProv[0].first)) {
				ERROR_RECOVERABLE( "Hit a BAD Point!" );
			}
			// find the province need to go
			for (auto prov : m_provinces) {
				// protect province
				if (prov->IsBeingSieged()) {
					provTargets.push_back( prov );
				}
			}
			for (auto country : m_relationWarCountries) {
				for (auto prov : country->m_provinces) {
					if (prov->IsAdjacentToCountry( this )) {
						provTargets.push_back( prov );
					}
				}
			}
			MapPolygonUnit* finalTarget = nullptr;
			// has target to go
			if ((int)provTargets.size() > 0) {
				// find the most valuable target
				float maxProvValue = 0.f;
				for (auto testProv : provTargets) {
					float provValue = 0.f;
					// distance
					provValue += GetClamped(0.05f - GetDistance2D( army->m_provIn->m_geoCenterPos, testProv->m_geoCenterPos ) / map->m_diagonalLength, 0.f, 1.f) * 50000.f;
					if (m_capitalProv) {
						provValue -= GetClamped( GetDistance2D( m_capitalProv->m_geoCenterPos, testProv->m_geoCenterPos ) / map->m_diagonalLength, 0.f, 1.f ) * 10000.f;
					}
					if (testProv->m_owner == this) {
						// defend: more important
						provValue += testProv->GetEconomyValue() * 3.f;
					}
					else {
						// attack
						provValue += testProv->GetEconomyValue();
						MapPolygonUnit* nearestProv = GetNearestProvinceToEnemyProvince( testProv );
						if (nearestProv) {
							provValue += GetDistance2D(nearestProv->m_geoCenterPos, testProv->m_geoCenterPos) / map->m_diagonalLength * 100000.f;
						}
					}
					if (provValue > maxProvValue) {
						maxProvValue = provValue;
						finalTarget = testProv;
					}
				}
			}
			// no target to go
			else {
				// go to the local max neighbor province
				float maxEco = 0.f;
				for (auto testProv : provCanGo) {
					if (!testProv->IsWater()) {
						float economyValue = testProv->GetEconomyValue();
						if (economyValue > maxEco) {
							maxEco = economyValue;
							finalTarget = testProv;
						}
					}
				}
				if (army->m_provIn->GetEconomyValue() > maxEco) {
					finalTarget = nullptr;
				}
			}
			if (finalTarget && finalTarget != army->m_provIn) {
				MapPolygonUnit* provToGo = army->FindNextProvinceToGo( finalTarget );
				if (std::find( provCanGo.begin(), provCanGo.end(), provToGo ) == provCanGo.end()) {
					provToGo = nullptr;
				}
				if (provToGo) {
					InstructionMoveArmy* newInstr = new InstructionMoveArmy();
					newInstr->m_army = army;
					newInstr->m_fromProv = army->m_provIn;
					newInstr->m_toProv = provToGo;
					map->AddInstructionToQueue( newInstr );
					army->m_goingTarget = finalTarget;
				}
			}
			/*if ((int)provCanGo.size() > 0) {
				InstructionMoveArmy* newInstr = new InstructionMoveArmy();
				newInstr->m_army = army;
				newInstr->m_fromProv = army->m_provIn;
				newInstr->m_toProv = provCanGo[map->m_historyRNG->RollRandomIntLessThan( (int)provCanGo.size() )];
				map->AddInstructionToQueue( newInstr );
			}*/
		}
		else { // going to target
			MapPolygonUnit* provToGo = army->FindNextProvinceToGo( army->m_goingTarget );
			if (std::find( provCanGo.begin(), provCanGo.end(), provToGo ) == provCanGo.end()) {
				provToGo = nullptr;
				army->m_goingTarget = nullptr;
			}
			if (provToGo) {
				InstructionMoveArmy* newInstr = new InstructionMoveArmy();
				newInstr->m_army = army;
				newInstr->m_fromProv = army->m_provIn;
				newInstr->m_toProv = provToGo;
				map->AddInstructionToQueue( newInstr );
			}
		}
	}

	// assimilate
	if (m_funds >= m_economyValue) {
		for (auto prov : m_provinces) {
			if (prov->HD_GetCultureInfluence( m_countryCulture ) < 0.6f && m_funds > 10 * ASSIMILATE_COST) {
				SpendMoney( ASSIMILATE_COST );
				InstructionAssimilate* newInstr = new InstructionAssimilate();
				newInstr->m_province = prov;
				newInstr->m_culture = m_countryCulture;
				map->AddInstructionToQueue( newInstr );
			}
		}
	}

	// convert
	if (m_funds >= m_economyValue) {
		for (auto prov : m_provinces) {
			if (prov->HD_GetReligionInfluence( m_countryReligion ) < 0.6f && m_funds > 10 * CONVERT_COST) {
				SpendMoney( CONVERT_COST );
				InstructionConvert* newInstr = new InstructionConvert();
				newInstr->m_province = prov;
				newInstr->m_religion = m_countryReligion;
				map->AddInstructionToQueue( newInstr );
			}
		}
	}

	// development
	if (m_funds >= m_economyValue) {
		for (auto prov : m_provinces) {
			if (m_funds > DEV_COST && m_funds >= m_economyValue) {
				if (prov->IsLegitimateToCountry( this ) && !prov->IsBeingSieged() &&
					((prov->m_totalPopulation < 10000 && map->m_productPrice[(int)prov->m_productType] > 4.f)
						|| (prov->m_totalPopulation > 30000))
					) {
					SpendMoney( DEV_COST );
					InstructionDevelop* newInstr = new InstructionDevelop();
					newInstr->m_prov = prov;
					map->AddInstructionToQueue( newInstr );
				}
			}
		}
	}

	// assimilate more
	if (m_funds >= m_economyValue) {
		for (auto prov : m_provinces) {
			if (prov->HD_GetCultureInfluence( m_countryCulture ) < 1.f && m_funds > 100 * ASSIMILATE_COST) {
				SpendMoney( ASSIMILATE_COST );
				InstructionAssimilate* newInstr = new InstructionAssimilate();
				newInstr->m_province = prov;
				newInstr->m_culture = m_countryCulture;
				map->AddInstructionToQueue( newInstr );
			}
		}
	}
	// convert more
	if (m_funds >= m_economyValue) {
		for (auto prov : m_provinces) {
			if (prov->HD_GetReligionInfluence( m_countryReligion ) < 1.f && m_funds > 100 * CONVERT_COST) {
				SpendMoney( CONVERT_COST );
				InstructionConvert* newInstr = new InstructionConvert();
				newInstr->m_province = prov;
				newInstr->m_religion = m_countryReligion;
				map->AddInstructionToQueue( newInstr );
			}
		}
	}
	
	// legalize
	for (auto prov : m_provinces) {
		if (!prov->IsLegitimateToCountry( this ) && prov->m_majorCulture == m_countryCulture && prov->m_majorReligion == m_countryReligion) {
			InstructionLegalize* newInstr = new InstructionLegalize();
			newInstr->m_province = prov;
			newInstr->m_country = this;
			map->AddInstructionToQueue( newInstr );
		}
	}

	// change country culture\religion
// 	if (m_cultures[0].first != m_countryCulture && (float)m_cultures[0].second / m_totalPopulation > 0.7f) {
// 		m_countryCulture = m_cultures[0].first;
// 		m_funds /= 10;
// 	}
// 	if (m_religions[0].first != m_countryReligion && (float)m_religions[0].second / m_totalPopulation > 0.7f) {
// 		m_countryReligion = m_religions[0].first;
// 		m_funds /= 10;
// 	}


	// relationship
	// chance to become worse
	for (auto otherCountry : map->m_countries) {
		if (otherCountry->IsExist() && otherCountry->m_capitalProv->m_continent == m_capitalProv->m_continent) {
			CountryRelationType relation = GetRelationTo( otherCountry );
			if (map->m_historyRNG->RollRandomFloatZeroToOne() < 0.002f) {
				if (relation == CountryRelationType::None) {
					InstructionRelationship* newInstr = new InstructionRelationship();
					newInstr->countryFrom = this;
					newInstr->countryTo = otherCountry;
					newInstr->m_relationWant = CountryRelationType::Hostile;
					map->AddInstructionToQueue( newInstr );
				}
				else if (relation == CountryRelationType::Friendly && map->m_historyRNG->RollRandomFloatZeroToOne() < 0.3f) {
					InstructionRelationship* newInstr = new InstructionRelationship();
					newInstr->countryFrom = this;
					newInstr->countryTo = otherCountry;
					newInstr->m_relationWant = CountryRelationType::None;
					map->AddInstructionToQueue( newInstr );
				}
				else if (relation == CountryRelationType::Alliance && map->m_historyRNG->RollRandomFloatZeroToOne() < 0.1f) {
					InstructionRelationship* newInstr = new InstructionRelationship();
					newInstr->countryFrom = this;
					newInstr->countryTo = otherCountry;
					newInstr->m_relationWant = CountryRelationType::Friendly;
					map->AddInstructionToQueue( newInstr );
				}
			}
		}
	}

	// build better relation
	if (m_funds >= m_economyValue) {
		for (auto otherCountry : map->m_countries) {
			if (m_funds < 10 * BUILD_FRIENDLY_RELATION_COAT) {
				break;
			}
			if (otherCountry->IsExist()) {
				CountryRelationType relation = GetRelationTo( otherCountry );
				if (otherCountry->m_countryCulture == m_countryCulture || otherCountry->m_countryReligion == m_countryReligion) {
					bool shouldBeMoreFriendly = true;
					for (auto prov : m_provinces) {
						if (prov->IsLegitimateToCountry( otherCountry )) {
							shouldBeMoreFriendly = false;
							break;
						}
					}
					for (auto prov : otherCountry->m_provinces) {
						if (prov->IsLegitimateToCountry( this )) {
							shouldBeMoreFriendly = false;
							break;
						}
					}
					if (CompareStrengthToCountryDuel( otherCountry ) < 0.5f) {
						shouldBeMoreFriendly = false;
					}
					if (!shouldBeMoreFriendly) {
						continue;
					}
					if (map->m_historyRNG->RollRandomFloatZeroToOne() < 0.01f) {
						float cultureFactor = 1.f;
						if (m_countryCulture->HasTrait( CultureTrait::Miserly )) {
							cultureFactor += 0.5f;
						}
						if (m_countryCulture->HasTrait( CultureTrait::Xenophile )) {
							cultureFactor -= 0.5f;
						}
						if (m_countryCulture->HasTrait( CultureTrait::Xenophobe )) {
							cultureFactor += 1.f;
						}
						if (m_countryCulture->HasTrait( CultureTrait::Pacifistic )) {
							cultureFactor -= 0.2f;
						}
						if (relation == CountryRelationType::None) {
							InstructionRelationship* newInstr = new InstructionRelationship();
							newInstr->countryFrom = this;
							newInstr->countryTo = otherCountry;
							newInstr->m_relationWant = CountryRelationType::Friendly;
							SpendMoney( int( (float)BUILD_FRIENDLY_RELATION_COAT * cultureFactor ) );
							map->AddInstructionToQueue( newInstr );
						}
						else if (relation == CountryRelationType::Friendly) {
							InstructionRelationship* newInstr = new InstructionRelationship();
							newInstr->countryFrom = this;
							newInstr->countryTo = otherCountry;
							newInstr->m_relationWant = CountryRelationType::Alliance;
							SpendMoney( int( (float)BUILD_FRIENDLY_RELATION_COAT * cultureFactor ) );
							map->AddInstructionToQueue( newInstr );
						}
						else if (relation == CountryRelationType::Hostile) {
							InstructionRelationship* newInstr = new InstructionRelationship();
							newInstr->countryFrom = this;
							newInstr->countryTo = otherCountry;
							newInstr->m_relationWant = CountryRelationType::None;
							SpendMoney( int( (float)BUILD_FRIENDLY_RELATION_COAT * cultureFactor ) );
							map->AddInstructionToQueue( newInstr );
						}
					}
				}
			}
		}
	}

	// become vassal
	for (auto otherCountry : m_relationAllianceCountries) {
		if (otherCountry->m_economyValue * 25 < m_economyValue && otherCountry->m_totalMilitaryStrength * 5 < m_totalMilitaryStrength) {
			InstructionRelationship* newInstr = new InstructionRelationship();
			newInstr->countryFrom = this;
			newInstr->countryTo = otherCountry;
			newInstr->m_relationWant = CountryRelationType::Vassal;
			map->AddInstructionToQueue( newInstr );
		}
	}

	// declare war
	std::vector<Country*> adjCountries;
	GetAdjacentCountries( adjCountries );
	std::sort( adjCountries.begin(), adjCountries.end(), []( Country* a, Country* b ) { return a->m_id < b->m_id; } );
	bool declareWar = false;
	bool willingToWar = false;
	float aggressiveValue = GetAggressiveValue();
	if (map->m_historyRNG->RollRandomFloatZeroToOne() < aggressiveValue) {
		willingToWar = true;
	}
	if (willingToWar) {
		for (auto adjCountry : adjCountries) {
			// unite the same culture first
			if (adjCountry->m_countryCulture == m_countryCulture) {
				CountryRelationType relation = GetRelationTo( adjCountry );
				if (ReadyToWarWithCountry( adjCountry ) && map->IsTwoCountryPassEndWarTime( this, adjCountry )) {
					if (relation == CountryRelationType::Hostile || relation == CountryRelationType::None) {
						InstructionRelationship* newInstr = new InstructionRelationship();
						newInstr->countryFrom = this;
						newInstr->countryTo = adjCountry;
						newInstr->m_relationWant = CountryRelationType::War;
						declareWar = true;
						map->AddInstructionToQueue( newInstr );
						break;
					}
					else if (relation == CountryRelationType::Friendly) {
						if (map->m_historyRNG->RollRandomFloatZeroToOne() < aggressiveValue) {
							InstructionRelationship* newInstr = new InstructionRelationship();
							newInstr->countryFrom = this;
							newInstr->countryTo = adjCountry;
							newInstr->m_relationWant = CountryRelationType::Hostile;
							declareWar = true;
							map->AddInstructionToQueue( newInstr );
							break;
						}
					}
					else if (relation == CountryRelationType::Alliance) {
						if (map->m_historyRNG->RollRandomFloatZeroToOne() < 0.5f * aggressiveValue) {
							InstructionRelationship* newInstr = new InstructionRelationship();
							newInstr->countryFrom = this;
							newInstr->countryTo = adjCountry;
							newInstr->m_relationWant = CountryRelationType::Hostile;
							declareWar = true;
							map->AddInstructionToQueue( newInstr );
							break;
						}
					}
				}
			}
		}
	}
	if (!declareWar && willingToWar) {
		for (auto adjCountry : adjCountries) {
			// conquer the other culture second
			if (adjCountry->m_countryCulture != m_countryCulture) {
				CountryRelationType relation = GetRelationTo( adjCountry );
				if (relation == CountryRelationType::Hostile || relation == CountryRelationType::None) {
					if (ReadyToWarWithCountry( adjCountry ) && map->IsTwoCountryPassEndWarTime(this, adjCountry)) {
						InstructionRelationship* newInstr = new InstructionRelationship();
						newInstr->countryFrom = this;
						newInstr->countryTo = adjCountry;
						newInstr->m_relationWant = CountryRelationType::War;
						declareWar = true;
						map->AddInstructionToQueue( newInstr );
						break;
					}
				}
			}
		}
	}

	// core province
	std::vector<Country*> alreadyDiplomacyList;
	if (!declareWar && willingToWar) {
		for (auto prov : map->m_mapPolygonUnits) {
			if (prov->m_owner && prov->m_owner != this && prov->IsLegitimateToCountry( this )
				&& std::find(alreadyDiplomacyList.begin(), alreadyDiplomacyList.end(), prov->m_owner) == alreadyDiplomacyList.end()) {
				if (ReadyToWarWithCountry( prov->m_owner )) {
					InstructionRelationship* newInstr = nullptr;
					CountryRelationType relation = GetRelationTo( prov->m_owner );
					if (relation == CountryRelationType::Alliance) {
						newInstr = new InstructionRelationship();
						newInstr->countryFrom = this;
						newInstr->countryTo = prov->m_owner;
						newInstr->m_relationWant = CountryRelationType::Friendly;
					}
					else if (relation == CountryRelationType::Friendly) {
						newInstr = new InstructionRelationship();
						newInstr->countryFrom = this;
						newInstr->countryTo = prov->m_owner;
						newInstr->m_relationWant = CountryRelationType::None;
					}
					else if (relation == CountryRelationType::None) {
						newInstr = new InstructionRelationship();
						newInstr->countryFrom = this;
						newInstr->countryTo = prov->m_owner;
						newInstr->m_relationWant = CountryRelationType::Hostile;
					}
					else if (relation == CountryRelationType::Hostile && map->IsTwoCountryPassEndWarTime(this, prov->m_owner)) {
						newInstr = new InstructionRelationship();
						newInstr->countryFrom = this;
						newInstr->countryTo = prov->m_owner;
						newInstr->m_relationWant = CountryRelationType::War;
					}
					if (newInstr) {
						map->AddInstructionToQueue( newInstr );
						alreadyDiplomacyList.push_back( prov->m_owner );
					}
				}
			}
		}
	}

	// end war
	for (auto country : m_relationWarCountries) {
		if (IsExist() && map->GetTotalMonthCount() - GetWarTimeWith(country) >= 18 && CannotAffordWarWithCountry( country ) && GetCountryMilitaryStatus() < 0.5f) {
			bool doEndWar = false;
			bool isAnnexed = false;
			// no condition peace
			if (GetCountryMilitaryStatus() > 0.1f) {
				// the other country will acquire some provinces
				std::vector<Province*> provToLose;
				for (auto province : m_provinces) {
					if (province->IsAdjacentToCountry( country )) {
						int count = 0;
						int totalCount = 0;
						for (auto adjProv : province->m_adjacentUnits) {
							if (adjProv->m_owner && adjProv->m_owner != this) {
								count++;
							}
							if (adjProv->m_owner) {
								totalCount++;
							}
						}
						if (count >= std::max( RoundDownToInt( totalCount * 0.75f ), 0 )) {
							provToLose.push_back( province );
						}
					}
				}
				for (auto province : provToLose) {
					LoseProvince( province, true );
					country->GainProvince( province );
				}
				InstructionRelationship* newInstr = new InstructionRelationship();
				newInstr->countryFrom = this;
				newInstr->countryTo = country;
				newInstr->m_relationWant = CountryRelationType::Hostile;
				map->AddInstructionToQueue( newInstr );
				doEndWar = true;
			}
			// become tributary
			else if (country->IsCelestial()) {
				InstructionRelationship* newInstr = new InstructionRelationship();
				newInstr->countryFrom = this;
				newInstr->countryTo = country;
				newInstr->m_relationWant = CountryRelationType::Tributary;
				map->AddInstructionToQueue( newInstr );
				doEndWar = true;
			}
			// become vassal
			else if (m_economyValue * 20 < country->m_economyValue && (int)country->m_relationVassals.size() < 10 && map->m_historyRNG->RollRandomFloatZeroToOne() < 0.2f) {
				// the other country will acquire some provinces
				std::vector<Province*> provToLose;
				for (auto province : m_provinces) {
					if (province->IsAdjacentToCountry( country )) {
						int count = 0;
						int totalCount = 0;
						for (auto adjProv : province->m_adjacentUnits) {
							if (adjProv->m_owner && adjProv->m_owner != this) {
								count++;
							}
							if (adjProv->m_owner) {
								totalCount++;
							}
						}
						if (count >= std::max( RoundDownToInt( totalCount * 0.75f ), 0 )) {
							provToLose.push_back( province );
						}
					}
				}
				for (auto province : provToLose) {
					LoseProvince( province, true );
					country->GainProvince( province );
				}
				InstructionRelationship* newInstr = new InstructionRelationship();
				newInstr->countryFrom = this;
				newInstr->countryTo = country;
				newInstr->m_relationWant = CountryRelationType::Vassal;
				map->AddInstructionToQueue( newInstr );
				doEndWar = true;
			}
			// annexed by the other
			else if(m_economyValue * 5 < country->m_economyValue && CompareStrengthToCountryDuel(country) > 20.f && IsAdjacentToCountry(country)){
				//for (auto army : m_armies) {
				//	RemoveArmyFromMap( army, map );
				//}
				country->AnnexCountry( this );
				InstructionRelationship* newInstr = new InstructionRelationship();
				newInstr->countryFrom = this;
				newInstr->countryTo = country;
				newInstr->m_relationWant = CountryRelationType::None;
				map->AddInstructionToQueue( newInstr );
				doEndWar = true;
				isAnnexed = true;
			}
			// else no peace

			// end war with vassals
			if (doEndWar) {
				for (auto vassal : m_relationVassals) {
					InstructionRelationship* newInstr = new InstructionRelationship();
					newInstr->countryFrom = vassal;
					newInstr->countryTo = country;
					newInstr->m_relationWant = CountryRelationType::Hostile;
					map->AddInstructionToQueue( newInstr );
				}
				for (auto vassal : country->m_relationVassals) {
					InstructionRelationship* newInstr = new InstructionRelationship();
					newInstr->countryFrom = this;
					newInstr->countryTo = vassal;
					newInstr->m_relationWant = CountryRelationType::Hostile;
					map->AddInstructionToQueue( newInstr );
				}
			}
			if (isAnnexed) {
				return;
			}
		}
	}
	// call allies for help
	for (auto country : m_relationWarCountries) {
		for (auto ally : m_relationAllianceCountries) {
			CountryRelationType relation = country->GetRelationTo( ally );
			if ((relation == CountryRelationType::Hostile || relation == CountryRelationType::None) && ally->IsAdjacentToCountry(country)
				&& map->IsTwoCountryPassEndWarTime(country, ally)) {
				InstructionRelationship* newInstr = new InstructionRelationship();
				newInstr->countryFrom = ally;
				newInstr->countryTo = country;
				newInstr->m_relationWant = CountryRelationType::War;
				map->AddInstructionToQueue( newInstr );
			}
		}
		for (auto ally : m_relationVassals) {
			if (ally != country && ally->IsAdjacentToCountry( country )) {
				InstructionRelationship* newInstr = new InstructionRelationship();
				newInstr->countryFrom = ally;
				newInstr->countryTo = country;
				newInstr->m_relationWant = CountryRelationType::War;
				map->AddInstructionToQueue( newInstr );
			}
		}
		if (m_relationSuzerain) {
			CountryRelationType relation = country->GetRelationTo( m_relationSuzerain );
			if (relation == CountryRelationType::Hostile || relation == CountryRelationType::None || relation == CountryRelationType::Friendly) {
				InstructionRelationship* newInstr = new InstructionRelationship();
				newInstr->countryFrom = m_relationSuzerain;
				newInstr->countryTo = country;
				newInstr->m_relationWant = CountryRelationType::War;
				map->AddInstructionToQueue( newInstr );
			}
		}
	}

	// war of free vassal
	if (IsVassal() && ReadyToWarWithCountry( m_relationSuzerain ) && map->IsTwoCountryPassEndWarTime( this, m_relationSuzerain )) {
		InstructionRelationship* newInstr = new InstructionRelationship();
		newInstr->countryFrom = this;
		newInstr->countryTo = m_relationSuzerain;
		newInstr->m_relationWant = CountryRelationType::War;
		map->AddInstructionToQueue( newInstr );
		m_relationSuzerain->RemoveVassalRelation( this );
		m_relationSuzerain = nullptr;
	}

	// annex vassal
	for (auto vassal : m_relationVassals) {
		vassal->GetAdjacentCountries( adjCountries );
		if ((int)adjCountries.size() == 1 && adjCountries[0] == this) {
			AnnexCountry( vassal );
			break;
		}
		if (vassal->m_countryCulture == m_countryCulture && vassal->m_countryReligion == m_countryReligion) {
			if (map->m_mapRNG->RollRandomFloatZeroToOne() < 0.001f) {
				AnnexCountry( vassal );
				break;
			}
		}
		else {
			if (map->m_mapRNG->RollRandomFloatZeroToOne() < 0.0001f) {
				AnnexCountry( vassal );
				break;
			}
		}
	}

	if (m_governmentType == CountryGovernmentType::Separatism && m_relationWarCountries.empty()) {
		m_governmentType = CountryGovernmentType::Autocracy;
	}
}

void Country::BeginTurn()
{
	if (!IsExist()) {
		delete m_edgeShowingBuffer;
		m_edgeShowingBuffer = nullptr;
		delete m_edgeShowingBuffer3D;
		m_edgeShowingBuffer3D = nullptr;
	}

	// for same sort order: sort all lists
	std::sort( m_provinces.begin(), m_provinces.end(), []( Province* p1, Province* p2 ) { return p1->m_id < p2->m_id; } );
	std::sort( m_cities.begin(), m_cities.end(), []( City* a, City* b ) { return a->m_id < b->m_id; } );
	std::sort( m_towns.begin(), m_towns.end(), []( Town* a, Town* b ) { return a->m_id < b->m_id; } );
	std::sort( m_armies.begin(), m_armies.end(), []( Army* a, Army* b ) { return a->m_globalID < b->m_globalID; } );
	std::sort( m_relationFriendlyCountries.begin(), m_relationFriendlyCountries.end(), []( Country* a, Country* b ) { return a->m_id < b->m_id; } );
	std::sort( m_relationAllianceCountries.begin(), m_relationAllianceCountries.end(), []( Country* a, Country* b ) { return a->m_id < b->m_id; } );
	std::sort( m_relationHostileCountries.begin(), m_relationHostileCountries.end(), []( Country* a, Country* b ) { return a->m_id < b->m_id; } );
	std::sort( m_relationWarCountries.begin(), m_relationWarCountries.end(), []( Country* a, Country* b ) { return a->m_id < b->m_id; } );
	std::sort( m_relationVassals.begin(), m_relationVassals.end(), []( Country* a, Country* b ) { return a->m_id < b->m_id; } );
	std::sort( m_relationTributaries.begin(), m_relationTributaries.end(), []( Country* a, Country* b ) { return a->m_id < b->m_id; } );
	std::sort( m_tribeUnions.begin(), m_tribeUnions.end(), []( Country* a, Country* b ) { return a->m_id < b->m_id; } );
	std::sort( m_warTime.begin(), m_warTime.end(), []( auto const& a, auto const& b ) { return a.first->m_id < b.first->m_id; } );

	// safety check and insurance: remove all inactive(pending to be deleted) army from country's army list
	for (int i = 0; i < (int)m_armies.size();) {
		if (!m_armies[i]->m_isActive) {
			RemoveArmyFromMap( m_armies[i], GetCurMap() );
		}
		else {
			++i;
		}
	}

	// gain funds
	m_funds += m_economyValue;
	float cultureInfluence = 1.f;
	if (m_countryCulture->HasTrait( CultureTrait::Xenophobe )) {
		cultureInfluence -= 0.05f;
	}
	if (m_countryCulture->HasTrait( CultureTrait::Pacifistic )) {
		cultureInfluence -= 0.1f;
	}
	if (m_countryCulture->HasTrait( CultureTrait::Militaristic )) {
		cultureInfluence -= 0.05f;
	}
	// subtract military maintenance fee
	SpendMoney( int( float( m_totalMilitaryStrength * ARMY_SOLDIER_COST ) * cultureInfluence ) );
	//m_funds -= std::max( ((int)m_armies.size() - 1), 0 ) * ARMY_COST;
	SpendMoney( (int)m_armies.size() * ARMY_COST );

	// bankruptcy
	if (m_funds < -m_economyValue * 12) {
		m_funds = 0;
		for (auto army : m_armies) {
			army->m_size /= 4;
		}
	}
	CalculateMilitaryStats();
}

void Country::EndTurn()
{
	DeleteSmallCultureAndReligion();
	// recalculate all provinces for country
	std::vector<MapPolygonUnit*> provs = m_provinces;
	m_provinces.clear();
	m_cities.clear();
	m_towns.clear();
	m_totalPopulation = 0;
	m_cultures.clear();
	m_religions.clear();
	for (auto prov : provs) {
		GainProvince( prov, false );
	}
	CalculateMilitaryStats();
	ReCalculateCultureAndReligion();
	CalculateMajorCulture();
	CalculateMajorReligion();
	CalculateEconomicValue();
}

void Country::CalculateMilitaryStats()
{
	m_totalMilitaryStrength = 0;
	for (auto army : m_armies) {
		m_totalMilitaryStrength += army->m_size;
	}
}

void Country::CalculateEconomicValue()
{
	// 1. population
	// culture population
	float populationCount = 0.f;
	for (auto& pair : m_cultures) {
		if (pair.first == m_countryCulture) {
			populationCount += (float)pair.second * TAX_PERSON_SAME_CULTURE;
		}
		else { // not country culture, give less money
			populationCount += (float)pair.second * TAX_PERSON_DIFF_CULTURE;
		}
	}
	float personTaxCultureFactor = 1.f;
	if (m_countryCulture->HasTrait( CultureTrait::Egalitarian )) {
		personTaxCultureFactor -= 0.1f;
	}
	if (m_countryCulture->HasTrait( CultureTrait::Traditional )) {
		personTaxCultureFactor += 0.05f;
	}
	populationCount *= personTaxCultureFactor;

	// 2. cities
	float cityTaxCount = 0.f;
	for (auto city : m_cities) {
		if (!(city->HasAttribute(CityAttribute::Fort))) {
			float rawTax = city->HasAttribute(CityAttribute::Commercial) ? (float)city->m_totalPopulation * TAX_PERSON_COMMERCIAL_CITY : (float)city->m_totalPopulation * TAX_PERSON_NORMAL_CITY;
			if (city->m_majorCulture != m_countryCulture) {
				rawTax *= 0.1f;
			}
			float cultureFactor = 1.f;
			if (city->m_majorCulture->HasTrait( CultureTrait::Commercial )) {
				cultureFactor += 0.1f;
			}
			if (city->m_majorCulture->HasTrait( CultureTrait::Traditional )) {
				cultureFactor -= 0.05f;
			}
			if (m_countryCulture->HasTrait( CultureTrait::Miserly )) {
				cultureFactor += 0.05f;
			}
			if (m_countryCulture->HasTrait( CultureTrait::Conservationist )) {
				cultureFactor += 0.05f;
			}
			if (m_countryCulture->HasTrait( CultureTrait::Matrilineal )) {
				cultureFactor += 0.05f;
			}
			if (city->m_majorCulture->HasTrait( CultureTrait::Jinxed )) {
				cultureFactor -= 0.05f;
			}

			cityTaxCount += rawTax * cultureFactor;
		}
	}
	for (auto town : m_towns) {
		float rawTax = (float)town->m_totalPopulation * TAX_PERSON_NORMAL_CITY;
		if (town->m_majorCulture != m_countryCulture) {
			rawTax *= 0.1f;
		}
		float cultureFactor = 1.f;
		if (town->m_majorCulture->HasTrait( CultureTrait::Commercial )) {
			cultureFactor += 0.1f;
		}
		if (town->m_majorCulture->HasTrait( CultureTrait::Traditional )) {
			cultureFactor -= 0.05f;
		}
		if (m_countryCulture->HasTrait( CultureTrait::Miserly )) {
			cultureFactor += 0.05f;
		}
		if (m_countryCulture->HasTrait( CultureTrait::Conservationist )) {
			cultureFactor += 0.05f;
		}
		if (m_countryCulture->HasTrait( CultureTrait::Matrilineal )) {
			cultureFactor += 0.05f;
		}
		if (town->m_majorCulture->HasTrait( CultureTrait::Jinxed )) {
			cultureFactor -= 0.05f;
		}

		cityTaxCount += rawTax * cultureFactor;
	}

	// 3. production
	float productionCount = 0.f;
	for (auto prov : m_provinces) {
		float productValue = (GetCurMap()->m_productPrice[(int)prov->m_productType] * GetClamped( (float)prov->m_totalPopulation, 1000.f, 10000.f ));
		if (!prov->IsLegitimateToCountry( this )) {
			productValue *= 0.1f;
		}
		float cultureFactor = 1.f;
		if (prov->m_majorCulture->HasTrait( CultureTrait::Industrious )) {
			cultureFactor += 0.1f;
		}
		if (prov->m_majorCulture->HasTrait( CultureTrait::Alarmist )) {
			cultureFactor -= 0.1f;
		}
		if (m_countryCulture->HasTrait( CultureTrait::Conservationist )) {
			cultureFactor -= 0.05f;
		}
		if (prov->m_majorCulture->HasTrait( CultureTrait::Egalitarian )) {
			cultureFactor += 0.1f;
		}
		if (m_countryCulture->HasTrait( CultureTrait::Patriarchy )) {
			cultureFactor += 0.05f;
		}
		
		cultureFactor = GetClamped( cultureFactor, 0.1f, cultureFactor );
		productionCount += (productValue * PRODUCTION_VALUE_PER_PERSON * cultureFactor);
	}

	m_economyValue = 2000 + static_cast<int>(std::round( (productionCount + cityTaxCount + populationCount) * GetEconomicValueMultiplier() )) / 1000 * 1000;
}

float Country::CompareStrengthToCountry( Country* country ) const
{
	// consider everything including funds, economy income, army amount, alliance, relation
	// funds
	float fundsRatio = GetClamped( (float)country->m_funds / (float)m_funds, 0.f, 5.f );
	if (m_funds > m_economyValue * 12) {
		fundsRatio -= 0.3f;
	}
	else if (m_funds > m_economyValue * 6) {
		fundsRatio -= 0.15f;
	}
	// economy
	float economyRatio = (float)country->m_economyValue / (float)m_economyValue;
	// army amount
	int allyArmySize = m_totalMilitaryStrength;
	for (auto allyCountry : m_relationAllianceCountries) {
		if (allyCountry->IsAdjacentToCountry( country )) {
			allyArmySize += allyCountry->m_totalMilitaryStrength;
		}
	}
	int enemyArmySize = country->m_totalMilitaryStrength;
	for (auto allyCountry : country->m_relationAllianceCountries) {
		if (IsAdjacentToCountry( allyCountry ) && !IsInWarWith( allyCountry )) {
			enemyArmySize += allyCountry->m_totalMilitaryStrength;
		}
	}
	for (auto warCountry : m_relationWarCountries) {
		if (IsAdjacentToCountry( warCountry )) {
			enemyArmySize += warCountry->m_totalMilitaryStrength;
		}
	}
	float armyRatio = (float)enemyArmySize / (float)allyArmySize;
	return (armyRatio * 5.f + economyRatio * 10.f + fundsRatio * 1.f) / 8.f;
}

float Country::CompareStrengthToCountryDuel( Country* country ) const
{
	// if vassal: 

	// economy
	float economyRatio = (float)country->m_economyValue / (float)m_economyValue;
	// army amount
	int allyArmySize = m_totalMilitaryStrength;
	int enemyArmySize = country->m_totalMilitaryStrength;
	
	float armyRatio = (float)enemyArmySize / (float)allyArmySize;
	return (armyRatio * 5.f + economyRatio * 10.f) / 7.5f;
}

bool Country::ReadyToWarWithCountry( Country* country ) const
{
	float relativeStrength = CompareStrengthToCountry( country );
	bool readyToWar = false;
	float modifier = 0.f;
	if (this->m_countryCulture == country->m_majorCulture) {
		modifier = 0.2f;
	}
	if (relativeStrength < 0.2f) {
		if (GetCurMap()->m_historyRNG->RollRandomFloatZeroToOne() > RangeMapClamped( relativeStrength, 0.f, 0.2f, 0.f, 0.4f ) - modifier) {
			readyToWar = true;
		}
	}
	else if (relativeStrength <= 0.6f) {
		if (GetCurMap()->m_historyRNG->RollRandomFloatZeroToOne() > RangeMapClamped(relativeStrength, 0.2f, 0.6f, 0.4f, 0.8f) - modifier) {
			readyToWar = true;
		}
	}

	return readyToWar;
}

bool Country::CannotAffordWarWithCountry( Country* country ) const
{
	if (m_funds < -m_economyValue * 4) {
		return true;
	}
	float relativeStrength = CompareStrengthToCountry( country );
	if (relativeStrength > 5.f) {
		return true;
	}
	return false;
}

float Country::GetAggressiveValue()
{
	return RangeMapClamped( SmoothStart2( RangeMapClamped( (float)m_totalPopulation, 0.f, 10000000, 0.f, 1.f ) ), 0.f, 1.f, 0.0014f, 0.0007f );
}

float Country::GetCountryMilitaryStatus() const
{
	int allyArmySize = m_totalMilitaryStrength;
	int enemyArmySize = 0;
	for (auto allyCountry : m_relationAllianceCountries) {
		allyArmySize += allyCountry->m_totalMilitaryStrength / 2;
	}
	for (auto warCountry : m_relationWarCountries) {
		if (IsAdjacentToCountry( warCountry )) {
			enemyArmySize += warCountry->m_totalMilitaryStrength;
		}
	}
	return (float)allyArmySize / (float)enemyArmySize;
}

int Country::GetMonthlyIncome() const
{
	int result = m_economyValue;
	result -= m_totalMilitaryStrength * ARMY_SOLDIER_COST;
	result -= (int)m_armies.size() * ARMY_COST;
	//result -= std::max( ((int)m_armies.size() - 1), 0 ) * ARMY_COST;
	return result;
}

/*int Country::GetMonthlyCost() const
{
	int res = 0;
	res += m_totalMilitaryStrength * ARMY_SOLDIER_COST;
	res += (int)m_armies.size() * ARMY_COST;
	return res;
}*/

void Country::GetAllCountriesWithMilitaryAccess( std::vector<Country*> out_countries )
{
	out_countries.clear();
	out_countries.push_back( this );
	for (auto country : m_relationWarCountries) {
		out_countries.push_back( country );
	}
}

bool Country::DoesCountryHaveMilitaryAccess( Country* country ) const
{
	if (country == this) {
		return true;
	}
	if (std::find( m_relationWarCountries.begin(), m_relationWarCountries.end(), country ) != m_relationWarCountries.end()) {
		return true;
	}
	return false;
}

float Country::GetCombatValueMultiplier() const
{
	float initialValue = 1.f;
	if (m_governmentType == CountryGovernmentType::Nomad) {
		initialValue *= 1.5f;
	}
	else if (m_governmentType == CountryGovernmentType::Autocracy) {
		initialValue *= 1.05f;
	}
	else if (m_governmentType == CountryGovernmentType::Oligarchy) {
		initialValue *= 0.9f;
	}
	else if (m_governmentType == CountryGovernmentType::Parliamentarism) {
		initialValue *= 0.95f;
	}
	else if (m_governmentType == CountryGovernmentType::Separatism) {
		initialValue *= 1.15f;
	}

	return initialValue;
}

float Country::GetEconomicValueMultiplier() const
{
	float initialValue = 1.f;
	if (m_governmentType == CountryGovernmentType::Nomad) {
		initialValue *= 0.85f;
	}
	else if (m_governmentType == CountryGovernmentType::Autocracy) {
		initialValue *= 0.9f;
	}
	else if (m_governmentType == CountryGovernmentType::Oligarchy) {
		initialValue *= 1.2f;
	}
	else if (m_governmentType == CountryGovernmentType::Parliamentarism) {
		initialValue *= 1.1f;
	}
	else if (m_governmentType == CountryGovernmentType::Separatism) {
		initialValue *= 0.9f;
	}

	return initialValue;
}

void Country::RemoveArmyFromList( Army* army )
{
	for (int i = 0; i < (int)m_armies.size(); i++) {
		if (m_armies[i] == army) {
			m_armies.erase( m_armies.begin() + i );
			return;
		}
	}
	ERROR_RECOVERABLE( "Cannot remove an army that is not in this country's list!" );
	return;
}

bool Country::ChangeNameTitle()
{
	Strings nameSplit = SplitStringOnDelimiter( m_name, ' ' );
	if ((int)nameSplit.size() == 3) {
		// if unite the whole culture, add kingdom to the title
		// if unite the whole culture and has some other cultures in the country, add empire to the title
		bool uniteWholeCulture = true;
		bool hasChangedName = false;
		for (auto country : GetCurMap()->m_countries) {
			if (country->IsExist() && country->m_countryCulture == m_countryCulture && !IsSuzerainOf( country )
				&& country->m_economyValue * 5 > m_economyValue) {
				uniteWholeCulture = false;
			}
		}
		if (uniteWholeCulture) {
			if (GetCultureInfluence( m_countryCulture ) < 0.7f) {
				nameSplit[0] = m_countryCulture->m_countryNameGenerator->m_suffixEmpire;
				hasChangedName = true;
			}
			else {
				nameSplit[0] = m_countryCulture->m_countryNameGenerator->m_suffixKingdom;
				hasChangedName = true;
			}
		}

		if (hasChangedName) {
			m_name = nameSplit[0] + " " + nameSplit[1] + " " + nameSplit[2];
		}
		return hasChangedName;
	}
	return false;
}

void Country::AddCulturePopulation( int totalPopulation, std::vector<std::pair<Culture*, float>> const& cultureInfluence )
{
	for (auto& addPair : cultureInfluence) {
		bool addSuccess = false;
		for (auto& resPair : m_cultures) {
			if (resPair.first == addPair.first) {
				addSuccess = true;
				resPair.second += (int)round(addPair.second * totalPopulation);
			}
		}
		if (!addSuccess) {
			m_cultures.push_back( std::pair<Culture*, int>( addPair.first, (int)round(addPair.second * totalPopulation) ) );
		}
	}
}

void Country::AddReligionPopulation( int totalPopulation, std::vector<std::pair<Religion*, float>> const& religionInfluence )
{
	for (auto& addPair : religionInfluence) {
		bool addSuccess = false;
		for (auto& resPair : m_religions) {
			if (resPair.first == addPair.first) {
				addSuccess = true;
				resPair.second += (int)round(addPair.second * totalPopulation);
			}
		}
		if (!addSuccess) {
			m_religions.push_back( std::pair<Religion*, int>( addPair.first, (int)round(addPair.second * totalPopulation) ) );
		}
	}
}

void Country::CalculateMajorCulture()
{
	std::sort( m_cultures.begin(), m_cultures.end(), []( std::pair<Culture*, int> const& a, std::pair<Culture*, int> const& b ) { return a.second > b.second; } );
	if (m_cultures.size() > 0) {
		m_majorCulture = m_cultures[0].first;
	}
}

void Country::CalculateMajorReligion()
{
	std::sort( m_religions.begin(), m_religions.end(), []( std::pair<Religion*, int> const& a, std::pair<Religion*, int> const& b ) { return a.second > b.second; } );
	if (m_religions.size() > 0) {
		m_majorReligion = m_religions[0].first;
	}
}

void Country::ReCalculateCultureAndReligion()
{
	m_totalPopulation = 0;
	for (auto prov : m_provinces) {
		m_totalPopulation += prov->m_totalPopulation;
	}
	for (auto city : m_cities) {
		m_totalPopulation += city->m_totalPopulation;
	}
	for (auto town : m_towns) {
		m_totalPopulation += town->m_totalPopulation;
	}
	for (auto& pair : m_cultures) {
		float peopleOfCulture = 0.f;
		for (auto prov : m_provinces) {
			peopleOfCulture += (float)(prov->m_totalPopulation * prov->HD_GetCultureInfluence( pair.first ));
		}
		for (auto city : m_cities) {
			peopleOfCulture += (float)(city->m_totalPopulation * city->GetCultureInfluence( pair.first ));
		}
		for (auto town : m_towns) {
			peopleOfCulture += (float)(town->m_totalPopulation * town->GetCultureInfluence( pair.first ));
		}
		pair.second = (int)round( peopleOfCulture );
	}
	for (auto& pair : m_religions) {
		float peopleOfReligion = 0.f;
		for (auto prov : m_provinces) {
			peopleOfReligion += (float)(prov->m_totalPopulation * prov->HD_GetReligionInfluence( pair.first ));
		}
		for (auto city : m_cities) {
			peopleOfReligion += (float)(city->m_totalPopulation * city->GetReligionInfluence( pair.first ));
		}
		for (auto town : m_towns) {
			peopleOfReligion += (float)(town->m_totalPopulation * town->GetReligionInfluence( pair.first ));
		}
		pair.second = (int)round( peopleOfReligion );
	}
}

void Country::ResetCapitalProvince()
{
	float maxProvScore = 0.f;
	m_capitalProv = nullptr;
	for (auto prov : m_provinces) {
		// ToDo: province needs to be in the major part of the country
		float provScore = 0.f;
		provScore += SmoothStop2( RangeMapClamped( (float)prov->m_totalPopulation, 0.f, 30000.f, 0.f, 1.f ) );
		if (prov->m_cities.size() > 0) {
			for (auto city : prov->m_cities) {
				if (city->HasAttribute(CityAttribute::Fort)) {
					provScore += 0.3f;
				}
				if (city->HasAttribute(CityAttribute::Commercial)) {
					provScore += 0.3f;
				}
				if (city->HasAttribute(CityAttribute::Port)) {
					provScore += 0.3f;
				}
				if (city->m_totalPopulation > 10000) {
					provScore += 0.2f;
				}
			}
		}
		if (m_originProv) {
			provScore += (1.f - GetClamped( GetDistance2D( prov->m_geoCenterPos, m_originProv->m_geoCenterPos ) / 25.f, 0.f, 1.f ));
		}
		if (provScore >= maxProvScore) {
			maxProvScore = provScore;
			m_capitalProv = prov;
		}
	}

	// if there is a city, reset the capital city
	if (m_capitalProv && m_capitalProv->m_cities.size() > 0) {
		std::sort( m_capitalProv->m_cities.begin(), m_capitalProv->m_cities.end(), []( City const* a, City const* b ) {return a->m_totalPopulation > b->m_totalPopulation; } );
		City* mostPopulusCity = m_capitalProv->m_cities[0];
		mostPopulusCity->AddAttribute(CityAttribute::Capital);
		//mostPopulusCity->m_totalPopulation = (int)((float)mostPopulusCity->m_totalPopulation * 1.25f);
		m_capitalCity = m_capitalProv->m_cities[0];
	}
	else {
		m_capitalCity = nullptr;
	}
}

void Country::GetBoundsPointsForLabel( Vec2& out_startPos, Vec2& out_endPos ) const
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
	for (auto prov : m_provinces) {
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

	/*
	// 3 left top - right bottom
	float LTToRBDist = GetDistanceSquared2D( leftTopPos, rightBottomPos );
	// 4 left bottom - right top
	float LBToRTDist = GetDistanceSquared2D( leftBottomPos, rightTopPos );
	if (LTToRBDist > LBToRTDist) {
		out_startPos = Interpolate( leftTopPos, rightBottomPos, 0.2f );
		out_endPos = Interpolate( leftTopPos, rightBottomPos, 0.8f );
	}
	else {
		out_startPos = Interpolate( leftBottomPos, rightTopPos, 0.2f );
		out_endPos = Interpolate( leftBottomPos, rightTopPos, 0.8f );
	}*/
}

Vec2 Country::GetGeometricCenter() const
{
	Vec2 sumOfProvGeoCenter( 0.f, 0.f );
	for (auto prov : m_provinces) {
		sumOfProvGeoCenter += prov->m_geoCenterPos;
	}
	return sumOfProvGeoCenter / (float)m_provinces.size();
}

bool Country::IsFriendlyWith( Country* country ) const
{
	for (auto countryIter : m_relationFriendlyCountries) {
		if (countryIter == country) {
			return true;
		}
	}
	return false;
}

bool Country::IsHostileWith( Country* country ) const
{
	for (auto countryIter : m_relationHostileCountries) {
		if (countryIter == country) {
			return true;
		}
	}
	return false;
}

bool Country::IsInWarWith( Country* country ) const
{
	for (auto countryIter : m_relationWarCountries) {
		if (countryIter == country) {
			return true;
		}
	}
	return false;
}

bool Country::IsAlliedWith( Country* country ) const
{
	for (auto countryIter : m_relationAllianceCountries) {
		if (countryIter == country) {
			return true;
		}
	}
	return false;
}

bool Country::IsInTribeUnionWith( Country* country ) const
{
	for (auto countryIter : m_tribeUnions) {
		if (countryIter == country) {
			return true;
		}
	}
	return false;
}

bool Country::IsTributaryOf( Country* country ) const
{
	if (m_relationCelestialEmpire == country) {
		return true;
	}
	return false;
}

bool Country::IsSuzerainOf( Country* country ) const
{
	for (auto countryIter : m_relationVassals) {
		if (countryIter == country) {
			return true;
		}
	}
	return false;
}

bool Country::IsVassalOf( Country* country ) const
{
	if (m_relationSuzerain == country) {
		return true;
	}
	return false;
}

bool Country::IsCelestialOf( Country* country ) const
{
	for (auto countryIter : m_relationTributaries) {
		if (countryIter == country) {
			return true;
		}
	}
	return false;
}

void Country::SpendMoney( int amount )
{
	float factor = 1.f;
	if (m_countryCulture->HasTrait( CultureTrait::Thrifty )) {
		factor -= 0.01f;
	}
	m_funds -= static_cast<int>(std::round( ((float)amount * factor) ));
}

int Country::GetWarTimeWith( Country* country ) const
{
	for (auto& pair : m_warTime) {
		if (pair.first == country) {
			return pair.second;
		}
	}
	ERROR_RECOVERABLE( "Cannot reach here!" );
	return -1;
}

void Country::DeleteSmallCultureAndReligion()
{
	for (auto prov : m_provinces) {
		prov->m_cultures.erase( std::remove_if(
			prov->m_cultures.begin(), prov->m_cultures.end(),
			[&]( std::pair<Culture*, float> const& pair ) {
				if (pair.second < 0.001f) {
					prov->m_cultures[0].second += pair.second;
					return true;
				}
				return false;
			} ), prov->m_cultures.end() );
		prov->m_religions.erase( std::remove_if(
			prov->m_religions.begin(), prov->m_religions.end(),
			[&]( std::pair<Religion*, float> const& pair ) {
				if (pair.second < 0.001f) {
					prov->m_religions[0].second += pair.second;
					return true;
				}
				return false;
			} ), prov->m_religions.end() );
		/*for (auto it = prov->m_cultures.begin(); it != prov->m_cultures.end();) {
			if (it->second < 0.001f) {
				prov->m_cultures[0].second += it->second;
				prov->m_cultures.erase( it );
			}
			else {
				++it;
			}
		}*/
		/*for (auto it = prov->m_religions.begin(); it != prov->m_religions.end();) {
			if (it->second < 0.001f) {
				prov->m_religions[0].second += it->second;
				prov->m_religions.erase( it );
			}
			else {
				++it;
			}
		}*/
	}
}

MapPolygonUnit* Country::GetNearestProvinceToEnemyProvince( MapPolygonUnit* enemyProv ) const
{
	MapPolygonUnit* retProv = nullptr;
	float minDistSquared = FLT_MAX;
	for (auto prov : m_provinces) {
		float distSquared = GetDistanceSquared2D( prov->m_geoCenterPos, enemyProv->m_geoCenterPos );
		if (distSquared < minDistSquared) {
			retProv = prov;
			minDistSquared = distSquared;
		}
	}
	return retProv;
}
