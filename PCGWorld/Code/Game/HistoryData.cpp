#include "Game/HistoryData.hpp"
#include "Game/Map.hpp"
#include "Game/Army.hpp"
#include "Game/Country.hpp"
#include "Game/Culture.hpp"
#include "Game/Religion.hpp"
#include "Game/City.hpp"
#include "Game/CountryInstructions.hpp"
#include <filesystem>

HistoryData::HistoryData( Map* map )
{
	m_provinceData.resize( map->m_mapPolygonUnits.size() );
	for (int i = 0; i < (int)map->m_mapPolygonUnits.size(); i++) {
		Province* prov = map->m_mapPolygonUnits[i];
		if (prov->IsWater()) {
			m_provinceData[i].m_isWater = true;
		}
		else {
			m_provinceData[i].m_isWater = false;
			if (prov->m_owner) {
				m_provinceData[i].m_ownerCountryID = prov->m_owner->m_id;
			}
			else {
				m_provinceData[i].m_ownerCountryID = -1;
			}
			m_provinceData[i].m_population = prov->m_totalPopulation;
			m_provinceData[i].m_cultures.resize( prov->m_cultures.size() );
			for (int j = 0; j < (int)prov->m_cultures.size(); ++j) {
				m_provinceData[i].m_cultures[j] = std::pair<int, float>( prov->m_cultures[j].first->m_id, prov->m_cultures[j].second );
			}
			m_provinceData[i].m_religions.resize( prov->m_religions.size() );
			for (int j = 0; j < (int)prov->m_religions.size(); ++j) {
				m_provinceData[i].m_religions[j] = std::pair<int, float>( prov->m_religions[j].first->m_id, prov->m_religions[j].second );
			}
			for (int j = 0; j < (int)prov->m_legitimateCountries.size(); ++j) {
				//if (prov->m_legitimateCountries[j]->IsExist()) {
				m_provinceData[i].m_legalCountriesID.push_back( prov->m_legitimateCountries[j]->m_id );
				//}
			}
		}
	}
	m_cityData.resize( map->m_cities.size() );
	for (int i = 0; i < (int)map->m_cities.size(); i++) {
		City* city = map->m_cities[i];
		m_cityData[i].m_population = city->m_totalPopulation;
		m_cityData[i].m_cultures.resize( city->m_cultures.size() );
		for (int j = 0; j < (int)city->m_cultures.size(); ++j) {
			m_cityData[i].m_cultures[j] = std::pair<int, float>( city->m_cultures[j].first->m_id, city->m_cultures[j].second );
		}
		m_cityData[i].m_religions.resize( city->m_religions.size() );
		for (int j = 0; j < (int)city->m_religions.size(); ++j) {
			m_cityData[i].m_religions[j] = std::pair<int, float>( city->m_religions[j].first->m_id, city->m_religions[j].second );
		}
		m_cityData[i].m_defenseValue = city->m_defense;
		m_cityData[i].m_ownerID = city->m_owner->m_id;
		m_cityData[i].m_type = city->GetRawAttribute();
	}
	m_townData.resize( map->m_towns.size() );
	for (int i = 0; i < (int)map->m_towns.size(); i++) {
		Town* town = map->m_towns[i];
		m_townData[i].m_population = town->m_totalPopulation;
		m_townData[i].m_cultures.resize( town->m_cultures.size() );
		for (int j = 0; j < (int)town->m_cultures.size(); ++j) {
			m_townData[i].m_cultures[j] = std::pair<int, float>( town->m_cultures[j].first->m_id, town->m_cultures[j].second );
		}
		m_townData[i].m_religions.resize( town->m_religions.size() );
		for (int j = 0; j < (int)town->m_religions.size(); ++j) {
			m_townData[i].m_religions[j] = std::pair<int, float>( town->m_religions[j].first->m_id, town->m_religions[j].second );
		}
		m_townData[i].m_defenseValue = town->m_defense;
		m_townData[i].m_ownerID = town->m_owner->m_id;
	}
	m_countryData.resize( map->m_countries.size() );
	for (int i = 0; i < (int)map->m_countries.size(); i++) {
		Country* country = map->m_countries[i];
		m_countryData[i].m_exist = country->IsExist();
		if (!m_countryData[i].m_exist) {
			continue;
		}
		
		if (country->m_capitalCity) {
			m_countryData[i].m_capitalCityID = country->m_capitalCity->m_id;
		}
		else {
			m_countryData[i].m_capitalCityID = -1;
		}
		if (country->m_capitalProv) {
			m_countryData[i].m_capitalProvID = country->m_capitalProv->m_id;
		}
		else {
			m_countryData[i].m_capitalProvID = -1;
		}
		for (auto iterCountry : country->m_relationAllianceCountries) {
			m_countryData[i].m_allianceCountriesID.push_back( iterCountry->m_id );
		}
		for (auto iterCountry : country->m_relationFriendlyCountries) {
			m_countryData[i].m_friendlyCountriesID.push_back( iterCountry->m_id );
		}
		for (auto iterCountry : country->m_relationHostileCountries) {
			m_countryData[i].m_hostileCountriesID.push_back( iterCountry->m_id );
		}
		for (auto iterCountry : country->m_relationWarCountries) {
			m_countryData[i].m_warCountriesID.push_back( std::pair<int, int>( iterCountry->m_id, country->GetWarTimeWith( iterCountry ) ) );
		}
		for (auto iterCountry : country->m_relationTributaries) {
			m_countryData[i].m_tributaryCountriesID.push_back( iterCountry->m_id );
		}
		for (auto iterCountry : country->m_relationVassals) {
			m_countryData[i].m_vassalCountriesID.push_back( iterCountry->m_id );
		}
		m_countryData[i].m_isCelestial = country->m_isCelestial;
		if (country->m_relationCelestialEmpire) {
			m_countryData[i].m_celestialCountryID = country->m_relationCelestialEmpire->m_id;
		}
		if (country->m_relationSuzerain) {
			m_countryData[i].m_suzerainCountryID = country->m_relationSuzerain->m_id;
		}
		m_countryData[i].m_funds = country->m_funds;
		m_countryData[i].m_countryCultureID = country->m_countryCulture->m_id;
		m_countryData[i].m_countryReligionID = country->m_countryReligion->m_id;
		m_countryData[i].m_governmentType = (int)country->m_governmentType;
	}
	for (auto country : map->m_countries) {
		for (int i = 0; i < (int)country->m_armies.size(); i++) {
			Army* army = country->m_armies[i];
			if (army->m_goingTarget) {
				m_armyData.emplace_back( army->m_size, army->m_combatValue, army->m_provIn->m_id, army->m_globalID, army->m_owner->m_id, army->m_goingTarget->m_id );
			}
			else {
				m_armyData.emplace_back( army->m_size, army->m_combatValue, army->m_provIn->m_id, army->m_globalID, army->m_owner->m_id, -1 );
			}
		}
	}
	std::vector<HistoryCrisis*> copyedCrisisList;
	for (auto crisis : map->m_crisis) {
		if (crisis) {
			copyedCrisisList.push_back( crisis );
		}
	}
	m_crisisData.resize( copyedCrisisList.size() );
	for (int i = 0; i < (int)copyedCrisisList.size(); ++i) {
		HistoryCrisis* crisis = copyedCrisisList[i];
		m_crisisData[i].m_countryID = crisis->m_country->m_id;
		m_crisisData[i].m_type = (int)crisis->m_type;
		if (crisis->m_type == CrisisType::CultureConflict) {
			m_crisisData[i].m_cultureOrReligionID = ((Culture*)crisis->m_cultureOrReligion)->m_id;
		}
		else if (crisis->m_type == CrisisType::ReligionConflict) {
			m_crisisData[i].m_cultureOrReligionID = ((Religion*)crisis->m_cultureOrReligion)->m_id;
		}
		m_crisisData[i].m_globalID = crisis->m_globalID;
		m_crisisData[i].m_progress = crisis->m_progress;
	}
}

void HistoryData::PrintOutHistory( XmlDocument* document, XmlElement* rootElem, HistoryData* provMonth ) const
{
	XmlElement* elem = document->NewElement( "HistoryMonth" );
	rootElem->InsertEndChild( elem );
	UNUSED( provMonth );

}

void HistoryData::DumpToBinaryFormat( std::vector<uint8_t>& bin ) const
{
	size_t writePtr = 0;
	bin.clear();
	bin.reserve( 10000 );
	// count how much data it needs
	size_t sizeOfBuffer = 0;
	sizeOfBuffer += 24;
	for (auto const& provHistory : m_provinceData) {
		if (provHistory.m_isWater) {
			sizeOfBuffer += 1;
		}
		else {
			sizeOfBuffer += 9;// stand alone values
			sizeOfBuffer += 12;// size
			sizeOfBuffer += provHistory.m_legalCountriesID.size() * 4;
			sizeOfBuffer += provHistory.m_cultures.size() * 8;
			sizeOfBuffer += provHistory.m_religions.size() * 8;
		}
	}
	for (auto const& cityHistory : m_cityData) {
		sizeOfBuffer += 14;// stand alone values
		sizeOfBuffer += 8;// size
		sizeOfBuffer += cityHistory.m_cultures.size() * 8;
		sizeOfBuffer += cityHistory.m_religions.size() * 8;
	}
	for (auto const& townHistory : m_townData) {
		sizeOfBuffer += 12;// stand alone values
		sizeOfBuffer += 8;// size
		sizeOfBuffer += townHistory.m_cultures.size() * 8;
		sizeOfBuffer += townHistory.m_religions.size() * 8;
	}
	for (auto const& countryHistory : m_countryData) {
		if (!countryHistory.m_exist) {
			sizeOfBuffer += 1;
		}
		else {
			sizeOfBuffer += 31;
			sizeOfBuffer += 24;// size
			sizeOfBuffer += countryHistory.m_friendlyCountriesID.size() * 4;
			sizeOfBuffer += countryHistory.m_allianceCountriesID.size() * 4;
			sizeOfBuffer += countryHistory.m_warCountriesID.size() * 8;
			sizeOfBuffer += countryHistory.m_hostileCountriesID.size() * 4;
			sizeOfBuffer += countryHistory.m_vassalCountriesID.size() * 4;
			sizeOfBuffer += countryHistory.m_tributaryCountriesID.size() * 4;
		}
	}
	sizeOfBuffer += m_armyData.size() * 24;
	sizeOfBuffer += m_crisisData.size() * 20;
	bin.resize( sizeOfBuffer );

	int size = (int)m_provinceData.size();
	memcpy( bin.data() + writePtr, &size, sizeof( int ) );
	writePtr += sizeof( int );
	for (auto const& provHistory : m_provinceData) {
		if (provHistory.m_isWater) {
			memcpy( bin.data() + writePtr, &provHistory.m_isWater, sizeof( bool ) );
			writePtr += sizeof( bool );
		}
		else {
			memcpy( bin.data() + writePtr, &provHistory.m_isWater, sizeof( bool ) );
			writePtr += sizeof( bool );
			memcpy( bin.data() + writePtr, &provHistory.m_population, sizeof( int ) );
			writePtr += sizeof( int );
			memcpy( bin.data() + writePtr, &provHistory.m_ownerCountryID, sizeof( int ) );
			writePtr += sizeof( int );
			size = (int)provHistory.m_legalCountriesID.size();
			memcpy( bin.data() + writePtr, &size, sizeof( int ) );
			writePtr += sizeof( int );
			for (int i = 0; i < (int)provHistory.m_legalCountriesID.size(); ++i) {
				memcpy( bin.data() + writePtr, &provHistory.m_legalCountriesID[i], sizeof( int ) );
				writePtr += sizeof( int );
			}
			size = (int)provHistory.m_cultures.size();
			memcpy( bin.data() + writePtr, &size, sizeof( int ) );
			writePtr += sizeof( int );
			for (int i = 0; i < (int)provHistory.m_cultures.size(); ++i) {
				memcpy( bin.data() + writePtr, &provHistory.m_cultures[i], sizeof( std::pair<int, float> ) );
				writePtr += sizeof( std::pair<int, float> );
			}
			size = (int)provHistory.m_religions.size();
			memcpy( bin.data() + writePtr, &size, sizeof( int ) );
			writePtr += sizeof( int );
			for (int i = 0; i < (int)provHistory.m_religions.size(); ++i) {
				memcpy( bin.data() + writePtr, &provHistory.m_religions[i], sizeof( std::pair<int, float> ) );
				writePtr += sizeof( std::pair<int, float> );
			}
		}
	}
	size = (int)m_cityData.size();
	memcpy( bin.data() + writePtr, &size, sizeof( int ) );
	writePtr += sizeof( int );
	for (auto const& cityHistory : m_cityData) {
		memcpy( bin.data() + writePtr, &cityHistory.m_population, sizeof( int ) );
		writePtr += sizeof( int );
		memcpy( bin.data() + writePtr, &cityHistory.m_ownerID, sizeof( int ) );
		writePtr += sizeof( int );
		memcpy( bin.data() + writePtr, &cityHistory.m_defenseValue, sizeof( float ) );
		writePtr += sizeof( float );
		memcpy( bin.data() + writePtr, &cityHistory.m_type, sizeof( uint16_t ) );
		writePtr += sizeof( uint16_t );

		size =(int) cityHistory.m_cultures.size();
		memcpy( bin.data() + writePtr, &size, sizeof( int ) );
		writePtr += sizeof( int );
		for (int i = 0; i < (int)cityHistory.m_cultures.size(); ++i) {
			memcpy( bin.data() + writePtr, &cityHistory.m_cultures[i], sizeof( std::pair<int, float> ) );
			writePtr += sizeof( std::pair<int, float> );
		}
		size = (int)cityHistory.m_religions.size();
		memcpy( bin.data() + writePtr, &size, sizeof( int ) );
		writePtr += sizeof( int );
		for (int i = 0; i < (int)cityHistory.m_religions.size(); ++i) {
			memcpy( bin.data() + writePtr, &cityHistory.m_religions[i], sizeof( std::pair<int, float> ) );
			writePtr += sizeof( std::pair<int, float> );
		}
	}
	size = (int)m_townData.size();
	memcpy( bin.data() + writePtr, &size, sizeof( int ) );
	writePtr += sizeof( int );
	for (auto const& townHistory : m_townData) {
		memcpy( bin.data() + writePtr, &townHistory.m_population, sizeof( int ) );
		writePtr += sizeof( int );
		memcpy( bin.data() + writePtr, &townHistory.m_ownerID, sizeof( int ) );
		writePtr += sizeof( int );
		memcpy( bin.data() + writePtr, &townHistory.m_defenseValue, sizeof( float ) );
		writePtr += sizeof( float );

		size = (int)townHistory.m_cultures.size();
		memcpy( bin.data() + writePtr, &size, sizeof( int ) );
		writePtr += sizeof( int );
		for (int i = 0; i < (int)townHistory.m_cultures.size(); ++i) {
			memcpy( bin.data() + writePtr, &townHistory.m_cultures[i], sizeof( std::pair<int, float> ) );
			writePtr += sizeof( std::pair<int, float> );
		}
		size = (int)townHistory.m_religions.size();
		memcpy( bin.data() + writePtr, &size, sizeof( int ) );
		writePtr += sizeof( int );
		for (int i = 0; i < (int)townHistory.m_religions.size(); ++i) {
			memcpy( bin.data() + writePtr, &townHistory.m_religions[i], sizeof( std::pair<int, float> ) );
			writePtr += sizeof( std::pair<int, float> );
		}
	}
	size = (int)m_countryData.size();
	memcpy( bin.data() + writePtr, &size, sizeof( int ) );
	writePtr += sizeof( int );
	for (auto const& countryHistory : m_countryData) {
		memcpy( bin.data() + writePtr, &countryHistory.m_exist, sizeof( bool ) );
		writePtr += sizeof( bool );
		if (countryHistory.m_exist) {
			memcpy( bin.data() + writePtr, &countryHistory.m_capitalCityID, sizeof( int ) );
			writePtr += sizeof( int );
			memcpy( bin.data() + writePtr, &countryHistory.m_capitalProvID, sizeof( int ) );
			writePtr += sizeof( int );
			memcpy( bin.data() + writePtr, &countryHistory.m_celestialCountryID, sizeof( int ) );
			writePtr += sizeof( int );
			memcpy( bin.data() + writePtr, &countryHistory.m_countryCultureID, sizeof( int ) );
			writePtr += sizeof( int );
			memcpy( bin.data() + writePtr, &countryHistory.m_countryReligionID, sizeof( int ) );
			writePtr += sizeof( int );
			memcpy( bin.data() + writePtr, &countryHistory.m_funds, sizeof( int ) );
			writePtr += sizeof( int );
			memcpy( bin.data() + writePtr, &countryHistory.m_suzerainCountryID, sizeof( int ) );
			writePtr += sizeof( int );
			memcpy( bin.data() + writePtr, &countryHistory.m_isCelestial, sizeof( bool ) );
			writePtr += sizeof( bool );
			memcpy( bin.data() + writePtr, &countryHistory.m_governmentType, sizeof( unsigned char ) );
			writePtr += sizeof( unsigned char );

			size = (int)countryHistory.m_friendlyCountriesID.size();
			memcpy( bin.data() + writePtr, &size, sizeof( int ) );
			writePtr += sizeof( int );
			for (int i = 0; i < (int)countryHistory.m_friendlyCountriesID.size(); ++i) {
				memcpy( bin.data() + writePtr, &countryHistory.m_friendlyCountriesID[i], sizeof( int ) );
				writePtr += sizeof( int );
			}
			size = (int)countryHistory.m_allianceCountriesID.size();
			memcpy( bin.data() + writePtr, &size, sizeof( int ) );
			writePtr += sizeof( int );
			for (int i = 0; i < (int)countryHistory.m_allianceCountriesID.size(); ++i) {
				memcpy( bin.data() + writePtr, &countryHistory.m_allianceCountriesID[i], sizeof( int ) );
				writePtr += sizeof( int );
			}
			size = (int)countryHistory.m_hostileCountriesID.size();
			memcpy( bin.data() + writePtr, &size, sizeof( int ) );
			writePtr += sizeof( int );
			for (int i = 0; i < (int)countryHistory.m_hostileCountriesID.size(); ++i) {
				memcpy( bin.data() + writePtr, &countryHistory.m_hostileCountriesID[i], sizeof( int ) );
				writePtr += sizeof( int );
			}
			size = (int)countryHistory.m_warCountriesID.size();
			memcpy( bin.data() + writePtr, &size, sizeof( int ) );
			writePtr += sizeof( int );
			for (int i = 0; i < (int)countryHistory.m_warCountriesID.size(); ++i) {
				memcpy( bin.data() + writePtr, &countryHistory.m_warCountriesID[i], sizeof( std::pair<int, int> ) );
				writePtr += sizeof( std::pair<int, int> );
			}
			size = (int)countryHistory.m_vassalCountriesID.size();
			memcpy( bin.data() + writePtr, &size, sizeof( int ) );
			writePtr += sizeof( int );
			for (int i = 0; i < (int)countryHistory.m_vassalCountriesID.size(); ++i) {
				memcpy( bin.data() + writePtr, &countryHistory.m_vassalCountriesID[i], sizeof( int ) );
				writePtr += sizeof( int );
			}
			size = (int)countryHistory.m_tributaryCountriesID.size();
			memcpy( bin.data() + writePtr, &size, sizeof( int ) );
			writePtr += sizeof( int );
			for (int i = 0; i < (int)countryHistory.m_tributaryCountriesID.size(); ++i) {
				memcpy( bin.data() + writePtr, &countryHistory.m_tributaryCountriesID[i], sizeof( int ) );
				writePtr += sizeof( int );
			}
		}
	}
	size = (int)m_armyData.size();
	memcpy( bin.data() + writePtr, &size, sizeof( int ) );
	writePtr += sizeof( int );
	for (auto const& armyHistory : m_armyData) {
		memcpy( bin.data() + writePtr, &armyHistory.m_size, sizeof( int ) );
		writePtr += sizeof( int );
		memcpy( bin.data() + writePtr, &armyHistory.m_globalID, sizeof( unsigned int ) );
		writePtr += sizeof( unsigned int );
		memcpy( bin.data() + writePtr, &armyHistory.m_combatValue, sizeof( float ) );
		writePtr += sizeof( float );
		memcpy( bin.data() + writePtr, &armyHistory.m_ownerID, sizeof( int ) );
		writePtr += sizeof( int );
		memcpy( bin.data() + writePtr, &armyHistory.m_provInID, sizeof( int ) );
		writePtr += sizeof( int );
		memcpy( bin.data() + writePtr, &armyHistory.m_targetProvID, sizeof( int ) );
		writePtr += sizeof( int );
	}
	size = (int)m_crisisData.size();
	memcpy( bin.data() + writePtr, &size, sizeof( int ) );
	writePtr += sizeof( int );
	for (auto const& crisisHistory : m_crisisData) {
		memcpy( bin.data() + writePtr, &crisisHistory.m_countryID, sizeof( int ) );
		writePtr += sizeof( int );
		memcpy( bin.data() + writePtr, &crisisHistory.m_globalID, sizeof( unsigned int ) );
		writePtr += sizeof( unsigned int );
		memcpy( bin.data() + writePtr, &crisisHistory.m_cultureOrReligionID, sizeof( int ) );
		writePtr += sizeof( int );
		memcpy( bin.data() + writePtr, &crisisHistory.m_progress, sizeof( float ) );
		writePtr += sizeof( float );
		memcpy( bin.data() + writePtr, &crisisHistory.m_type, sizeof( int ) );
		writePtr += sizeof( int );
	}
}

void HistoryData::LoadFromBinaryFormat( std::vector<uint8_t> const& bin )
{
	size_t readPtr = 0;
	int size = 0;
	int totalArraySize = *(int*)(bin.data() + readPtr);
	readPtr += sizeof( int );
	// province
	for (int i = 0; i < totalArraySize; ++i) {
		HistoryProvinceData data;
		data.m_isWater = *(bool*)(bin.data() + readPtr);
		readPtr += sizeof( bool );
		if (!data.m_isWater) {
			data.m_population = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			data.m_ownerCountryID = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			size = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			for (int j = 0; j < size; ++j) {
				data.m_legalCountriesID.push_back( *(int*)(bin.data() + readPtr) );
				readPtr += sizeof( int );
			}
			size = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			for (int j = 0; j < size; ++j) {
				std::pair<int, float> pair;
				pair.first = *(int*)(bin.data() + readPtr);
				readPtr += sizeof( int );
				pair.second = *(float*)(bin.data() + readPtr);
				readPtr += sizeof( float );
				data.m_cultures.push_back( pair );
			}
			size = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			for (int j = 0; j < size; ++j) {
				std::pair<int, float> pair;
				pair.first = *(int*)(bin.data() + readPtr);
				readPtr += sizeof( int );
				pair.second = *(float*)(bin.data() + readPtr);
				readPtr += sizeof( float );
				data.m_religions.push_back( pair );
			}
		}
		m_provinceData.push_back( data );
	}
	// city
	totalArraySize = *(int*)(bin.data() + readPtr);
	readPtr += sizeof( int );
	for (int i = 0; i < totalArraySize; ++i) {
		HistoryCityData data;
		data.m_population = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		data.m_ownerID = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		data.m_defenseValue = *(float*)(bin.data() + readPtr);
		readPtr += sizeof( float );
		data.m_type = *(uint16_t*)(bin.data() + readPtr);
		readPtr += sizeof( uint16_t );

		size = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		for (int j = 0; j < size; ++j) {
			std::pair<int, float> pair;
			pair.first = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			pair.second = *(float*)(bin.data() + readPtr);
			readPtr += sizeof( float );
			data.m_cultures.push_back( pair );
		}
		size = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		for (int j = 0; j < size; ++j) {
			std::pair<int, float> pair;
			pair.first = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			pair.second = *(float*)(bin.data() + readPtr);
			readPtr += sizeof( float );
			data.m_religions.push_back( pair );
		}
		m_cityData.push_back( data );
	}

	// town
	totalArraySize = *(int*)(bin.data() + readPtr);
	readPtr += sizeof( int );
	for (int i = 0; i < totalArraySize; ++i) {
		HistoryTownData data;
		data.m_population = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		data.m_ownerID = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		data.m_defenseValue = *(float*)(bin.data() + readPtr);
		readPtr += sizeof( float );

		size = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		for (int j = 0; j < size; ++j) {
			std::pair<int, float> pair;
			pair.first = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			pair.second = *(float*)(bin.data() + readPtr);
			readPtr += sizeof( float );
			data.m_cultures.push_back( pair );
		}
		size = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		for (int j = 0; j < size; ++j) {
			std::pair<int, float> pair;
			pair.first = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			pair.second = *(float*)(bin.data() + readPtr);
			readPtr += sizeof( float );
			data.m_religions.push_back( pair );
		}
		m_townData.push_back( data );
	}

	// country
	totalArraySize = *(int*)(bin.data() + readPtr);
	readPtr += sizeof( int );
	for (int i = 0; i < totalArraySize; ++i) {
		HistoryCountryData data;
		data.m_exist = *(bool*)(bin.data() + readPtr);
		readPtr += sizeof( bool );
		if (data.m_exist) {
			data.m_capitalCityID = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			data.m_capitalProvID = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			data.m_celestialCountryID = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			data.m_countryCultureID = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			data.m_countryReligionID = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			data.m_funds = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			data.m_suzerainCountryID = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			data.m_isCelestial = *(bool*)(bin.data() + readPtr);
			readPtr += sizeof( bool );
			data.m_governmentType = (int)*(unsigned char*)(bin.data() + readPtr);
			readPtr += sizeof( unsigned char );

			size = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			for (int j = 0; j < size; ++j) {
				data.m_friendlyCountriesID.push_back( *(int*)(bin.data() + readPtr) );
				readPtr += sizeof( int );
			}
			size = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			for (int j = 0; j < size; ++j) {
				data.m_allianceCountriesID.push_back( *(int*)(bin.data() + readPtr) );
				readPtr += sizeof( int );
			}
			size = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			for (int j = 0; j < size; ++j) {
				data.m_hostileCountriesID.push_back( *(int*)(bin.data() + readPtr) );
				readPtr += sizeof( int );
			}
			size = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			for (int j = 0; j < size; ++j) {
				//data.m_warCountriesID.push_back( std::pair<int, int>( *(int*)(bin.data() + readPtr), *(int*)(bin.data() + readPtr + sizeof( int )) ) );
				std::pair<int, int> pair;
				pair.first = *(int*)(bin.data() + readPtr);
				readPtr += sizeof( int );
				pair.second = *(int*)(bin.data() + readPtr);
				readPtr += sizeof( int );
				data.m_warCountriesID.push_back( pair );
			}
			size = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			for (int j = 0; j < size; ++j) {
				data.m_vassalCountriesID.push_back( *(int*)(bin.data() + readPtr) );
				readPtr += sizeof( int );
			}
			size = *(int*)(bin.data() + readPtr);
			readPtr += sizeof( int );
			for (int j = 0; j < size; ++j) {
				data.m_tributaryCountriesID.push_back( *(int*)(bin.data() + readPtr) );
				readPtr += sizeof( int );
			}
		}
		m_countryData.push_back( data );
	}

	// army
	totalArraySize = *(int*)(bin.data() + readPtr);
	readPtr += sizeof( int );
	for (int i = 0; i < totalArraySize; ++i) {
		HistoryArmyData data;
		data.m_size = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		data.m_globalID = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		data.m_combatValue = *(float*)(bin.data() + readPtr);
		readPtr += sizeof( float );
		data.m_ownerID = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		data.m_provInID = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		data.m_targetProvID = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		m_armyData.push_back( data );
	}


	totalArraySize = *(int*)(bin.data() + readPtr);
	readPtr += sizeof( int );
	for (int i = 0; i < totalArraySize; ++i) {
		HistoryCrisisData data;
		data.m_countryID = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		data.m_globalID = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		data.m_cultureOrReligionID = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		data.m_progress = *(float*)(bin.data() + readPtr);
		readPtr += sizeof( float );
		data.m_type = *(int*)(bin.data() + readPtr);
		readPtr += sizeof( int );
		m_crisisData.push_back( data );
	}
}

HistoryArmyData::HistoryArmyData( int size, float combatValue, int provInID, int globalID, int ownerID, int targetProvID )
	:m_size(size), m_combatValue(combatValue), m_provInID(provInID), m_globalID(globalID), m_ownerID(ownerID), m_targetProvID(targetProvID)
{

}

class SaveSingleMonthHistoryJob :public Job {
public:
	SaveSingleMonthHistoryJob( HistorySavingSolver* historySolver, int year, int month )
		:m_historySolver(historySolver), m_year(year), m_month(month)
	{};
	virtual ~SaveSingleMonthHistoryJob() {};
	virtual void Execute() override;

	HistorySavingSolver* m_historySolver = nullptr;
	int m_year = 0;
	int m_month = 0;
protected:
	void GetCountryRelationDifference( 
		std::vector<int>& loseRelationCountries, std::vector<int>& gainRelationCountries,
		std::vector<int>& prevCountryID, std::vector<int>& fwdCountryID );
};

void SaveSingleMonthHistoryJob::Execute()
{
	Map* map = m_historySolver->m_map;
	// read two history data
	HistoryData dataThis, dataFwd;
	int fwdYear, fwdMonth = 0;
	fwdYear = m_year;
	fwdMonth = m_month + 1;
	if (fwdMonth == 13) {
		fwdMonth = 1;
		fwdYear += 1;
	}
	map->GetHistoryData( m_year, m_month, dataThis );
	map->GetHistoryData( fwdYear, fwdMonth, dataFwd );

	// compare and generate xml
	XmlDocument history;
	XmlElement* rootElem = history.NewElement( "History" );
	SetXmlAttribute( rootElem, "Year", m_year );
	SetXmlAttribute( rootElem, "Month", m_month );
	// provinces
	history.InsertFirstChild( rootElem );
	XmlElement* provRootElem = history.NewElement( "Provinces" );
	rootElem->InsertEndChild( provRootElem );
	for (int i = 0; i < dataFwd.m_provinceData.size(); ++i) {
		HistoryProvinceData const& provFwd = dataFwd.m_provinceData[i];
		HistoryProvinceData const& provPrev = dataThis.m_provinceData[i];
		if (!provFwd.m_isWater) {
			XmlElement* provElem = history.NewElement( "Prov" );
			provElem->SetAttribute( "ID", i );
			provElem->SetAttribute( "Name", map->m_mapPolygonUnits[i]->m_name.c_str() );
			bool hasChanges = false;
			if (provFwd.m_population != provPrev.m_population) {
				provElem->SetAttribute( "PrevPop", provPrev.m_population );
				provElem->SetAttribute( "NewPop", provFwd.m_population );
				hasChanges = true;
			}
			if (provFwd.m_ownerCountryID != provPrev.m_ownerCountryID) {
				provElem->SetAttribute( "PrevOwner", map->m_countries[provPrev.m_ownerCountryID]->m_name.c_str() );
				provElem->SetAttribute( "NewOwner", map->m_countries[provFwd.m_ownerCountryID]->m_name.c_str() );
				hasChanges = true;
			}
			if (provFwd.m_cultures != provPrev.m_cultures) {
				XmlElement* prevCultures = history.NewElement( "PrevCultures" );
				provElem->InsertEndChild( prevCultures );
				for (auto& pair : provPrev.m_cultures) {
					XmlElement* prevCulture = history.NewElement( "Culture" );
					prevCultures->InsertEndChild( prevCulture );
					prevCulture->SetAttribute( "Name", map->m_cultures[pair.first]->m_name.c_str() );
					prevCulture->SetAttribute( "Influence", pair.second );
				}
				XmlElement* newCultures = history.NewElement( "NewCultures" );
				provElem->InsertEndChild( newCultures );
				for (auto& pair : provFwd.m_cultures) {
					XmlElement* newCulture = history.NewElement( "Culture" );
					newCultures->InsertEndChild( newCulture );
					newCulture->SetAttribute( "Name", map->m_cultures[pair.first]->m_name.c_str() );
					newCulture->SetAttribute( "Influence", pair.second );
				}
				hasChanges = true;
			}
			if (provFwd.m_religions != provPrev.m_religions) {
				XmlElement* prevReligions = history.NewElement( "PrevReligions" );
				provElem->InsertEndChild( prevReligions );
				for (auto& pair : provPrev.m_religions) {
					XmlElement* prevReligion = history.NewElement( "Religion" );
					prevReligions->InsertEndChild( prevReligion );
					prevReligion->SetAttribute( "Name", map->m_religions[pair.first]->m_name.c_str() );
					prevReligion->SetAttribute( "Influence", pair.second );
				}
				XmlElement* newReligions = history.NewElement( "NewReligions" );
				provElem->InsertEndChild( newReligions );
				for (auto& pair : provFwd.m_religions) {
					XmlElement* newReligion = history.NewElement( "Religion" );
					newReligions->InsertEndChild( newReligion );
					newReligion->SetAttribute( "Name", map->m_religions[pair.first]->m_name.c_str() );
					newReligion->SetAttribute( "Influence", pair.second );
				}
				hasChanges = true;
			}
			if (provFwd.m_legalCountriesID != provPrev.m_legalCountriesID) {
				XmlElement* prevLegalCountries = history.NewElement( "PrevLegitimateCountries" );
				provElem->InsertEndChild( prevLegalCountries );
				for (auto id : provPrev.m_legalCountriesID) {
					XmlElement* prevLegalCountry = history.NewElement( "Country" );
					prevLegalCountries->InsertEndChild( prevLegalCountry );
					prevLegalCountry->SetAttribute( "Name", map->m_countries[id]->m_name.c_str() );
				}
				XmlElement* newLegalCountries = history.NewElement( "NewLegitimateCountries" );
				provElem->InsertEndChild( newLegalCountries );
				for (auto id : provFwd.m_legalCountriesID) {
					XmlElement* newLegalCountry = history.NewElement( "Religion" );
					newLegalCountries->InsertEndChild( newLegalCountry );
					newLegalCountry->SetAttribute( "Name", map->m_countries[id]->m_name.c_str() );
				}
				hasChanges = true;
			}
			if (hasChanges) {
				provRootElem->InsertEndChild( provElem );
			}
		}
	}

	// cities
	XmlElement* cityRootElem = history.NewElement( "Cities" );
	rootElem->InsertEndChild( cityRootElem );
	for (int i = 0; i < dataFwd.m_cityData.size(); ++i) {
		HistoryCityData const& cityFwd = dataFwd.m_cityData[i];
		HistoryCityData const& cityPrev = dataThis.m_cityData[i];
		XmlElement* cityElem = history.NewElement( "City" );
		cityElem->SetAttribute( "ID", i );
		cityElem->SetAttribute( "Name", map->m_cities[i]->m_name.c_str() );
		bool hasChanges = false;
		if (cityFwd.m_population != cityPrev.m_population) {
			cityElem->SetAttribute( "PrevPop", cityPrev.m_population );
			cityElem->SetAttribute( "NewPop", cityFwd.m_population );
			hasChanges = true;
		}
		if (cityFwd.m_ownerID != cityPrev.m_ownerID) {
			cityElem->SetAttribute( "PrevOwner", map->m_countries[cityPrev.m_ownerID]->m_name.c_str() );
			cityElem->SetAttribute( "NewOwner", map->m_countries[cityFwd.m_ownerID]->m_name.c_str() );
			hasChanges = true;
		}
		if (cityFwd.m_cultures != cityPrev.m_cultures) {
			XmlElement* prevCultures = history.NewElement( "PrevCultures" );
			cityElem->InsertEndChild( prevCultures );
			for (auto& pair : cityPrev.m_cultures) {
				XmlElement* prevCulture = history.NewElement( "Culture" );
				prevCultures->InsertEndChild( prevCulture );
				prevCulture->SetAttribute( "Name", map->m_cultures[pair.first]->m_name.c_str() );
				prevCulture->SetAttribute( "Influence", pair.second );
			}
			XmlElement* newCultures = history.NewElement( "NewCultures" );
			cityElem->InsertEndChild( newCultures );
			for (auto& pair : cityFwd.m_cultures) {
				XmlElement* newCulture = history.NewElement( "Culture" );
				newCultures->InsertEndChild( newCulture );
				newCulture->SetAttribute( "Name", map->m_cultures[pair.first]->m_name.c_str() );
				newCulture->SetAttribute( "Influence", pair.second );
			}
			hasChanges = true;
		}
		if (cityFwd.m_religions != cityPrev.m_religions) {
			XmlElement* prevReligions = history.NewElement( "PrevReligions" );
			cityElem->InsertEndChild( prevReligions );
			for (auto& pair : cityPrev.m_religions) {
				XmlElement* prevReligion = history.NewElement( "Religion" );
				prevReligions->InsertEndChild( prevReligion );
				prevReligion->SetAttribute( "Name", map->m_religions[pair.first]->m_name.c_str() );
				prevReligion->SetAttribute( "Influence", pair.second );
			}
			XmlElement* newReligions = history.NewElement( "NewReligions" );
			cityElem->InsertEndChild( newReligions );
			for (auto& pair : cityFwd.m_religions) {
				XmlElement* newReligion = history.NewElement( "Religion" );
				newReligions->InsertEndChild( newReligion );
				newReligion->SetAttribute( "Name", map->m_religions[pair.first]->m_name.c_str() );
				newReligion->SetAttribute( "Influence", pair.second );
			}
			hasChanges = true;
		}
		if (cityFwd.m_defenseValue != cityPrev.m_defenseValue) {
			cityElem->SetAttribute( "PrevDef", cityPrev.m_population );
			cityElem->SetAttribute( "NewDef", cityFwd.m_population );
			hasChanges = true;
		}
		if (cityFwd.m_type != cityPrev.m_type) {
			XmlElement* attributes = history.NewElement( "Attributes" );
			cityElem->InsertEndChild( attributes );
			if ((cityFwd.m_type & CITY_FLAG_CAPITAL) != (cityPrev.m_type & CITY_FLAG_CAPITAL)) {
				if (cityFwd.m_type & CITY_FLAG_CAPITAL) {
					XmlElement* attribute = history.NewElement( "GainAttr" );
					attributes->InsertEndChild( attribute );
					attribute->SetAttribute( "Name", "Capital" );
				}
				else if (cityPrev.m_type & CITY_FLAG_CAPITAL) {
					XmlElement* attribute = history.NewElement( "LoseAttr" );
					attributes->InsertEndChild( attribute );
					attribute->SetAttribute( "Name", "Capital" );
				}
			}
		}
		if (hasChanges) {
			provRootElem->InsertEndChild( cityElem );
		}
	}

	// towns
	XmlElement* townRootElem = history.NewElement( "Towns" );
	rootElem->InsertEndChild( townRootElem );
	for (int i = 0; i < dataFwd.m_townData.size(); ++i) {
		HistoryTownData const& townFwd = dataFwd.m_townData[i];
		HistoryTownData const& townPrev = dataThis.m_townData[i];
		XmlElement* townElem = history.NewElement( "Town" );
		townElem->SetAttribute( "ID", i );
		townElem->SetAttribute( "Name", map->m_towns[i]->m_name.c_str() );
		bool hasChanges = false;
		if (townFwd.m_population != townPrev.m_population) {
			townElem->SetAttribute( "PrevPop", townPrev.m_population );
			townElem->SetAttribute( "NewPop", townFwd.m_population );
			hasChanges = true;
		}
		if (townFwd.m_ownerID != townPrev.m_ownerID) {
			townElem->SetAttribute( "PrevOwner", map->m_countries[townPrev.m_ownerID]->m_name.c_str() );
			townElem->SetAttribute( "NewOwner", map->m_countries[townFwd.m_ownerID]->m_name.c_str() );
			hasChanges = true;
		}
		if (townFwd.m_cultures != townPrev.m_cultures) {
			XmlElement* prevCultures = history.NewElement( "PrevCultures" );
			townElem->InsertEndChild( prevCultures );
			for (auto& pair : townPrev.m_cultures) {
				XmlElement* prevCulture = history.NewElement( "Culture" );
				prevCultures->InsertEndChild( prevCulture );
				prevCulture->SetAttribute( "Name", map->m_cultures[pair.first]->m_name.c_str() );
				prevCulture->SetAttribute( "Influence", pair.second );
			}
			XmlElement* newCultures = history.NewElement( "NewCultures" );
			townElem->InsertEndChild( newCultures );
			for (auto& pair : townFwd.m_cultures) {
				XmlElement* newCulture = history.NewElement( "Culture" );
				newCultures->InsertEndChild( newCulture );
				newCulture->SetAttribute( "Name", map->m_cultures[pair.first]->m_name.c_str() );
				newCulture->SetAttribute( "Influence", pair.second );
			}
			hasChanges = true;
		}
		if (townFwd.m_religions != townPrev.m_religions) {
			XmlElement* prevReligions = history.NewElement( "PrevReligions" );
			townElem->InsertEndChild( prevReligions );
			for (auto& pair : townPrev.m_religions) {
				XmlElement* prevReligion = history.NewElement( "Religion" );
				prevReligions->InsertEndChild( prevReligion );
				prevReligion->SetAttribute( "Name", map->m_religions[pair.first]->m_name.c_str() );
				prevReligion->SetAttribute( "Influence", pair.second );
			}
			XmlElement* newReligions = history.NewElement( "NewReligions" );
			townElem->InsertEndChild( newReligions );
			for (auto& pair : townFwd.m_religions) {
				XmlElement* newReligion = history.NewElement( "Religion" );
				newReligions->InsertEndChild( newReligion );
				newReligion->SetAttribute( "Name", map->m_religions[pair.first]->m_name.c_str() );
				newReligion->SetAttribute( "Influence", pair.second );
			}
			hasChanges = true;
		}
		if (townFwd.m_defenseValue != townPrev.m_defenseValue) {
			townElem->SetAttribute( "PrevDef", townPrev.m_population );
			townElem->SetAttribute( "NewDef", townFwd.m_population );
			hasChanges = true;
		}
		if (hasChanges) {
			provRootElem->InsertEndChild( townElem );
		}
	}

	// countries
	XmlElement* countryRootElem = history.NewElement( "Countries" );
	rootElem->InsertEndChild( countryRootElem );
	for (int i = 0; i < dataFwd.m_countryData.size(); ++i) {
		HistoryCountryData const& countryFwd = dataFwd.m_countryData[i];
		if (countryFwd.m_exist) {
			// new country
			if ((int)dataThis.m_countryData.size() <= i) {
				XmlElement* countryElem = history.NewElement( "Country" );
				countryElem->SetAttribute( "ID", i );
				countryElem->SetAttribute( "Name", map->m_countries[i]->m_name.c_str() );
				countryElem->SetAttribute( "New", true );
				countryElem->SetAttribute( "Exist", true );

			}
			else { // old exist country
				HistoryCountryData const& countryPrev = dataThis.m_countryData[i];
				XmlElement* countryElem = history.NewElement( "Country" );
				countryElem->SetAttribute( "ID", i );
				countryElem->SetAttribute( "Name", map->m_countries[i]->m_name.c_str() );
				countryElem->SetAttribute( "New", false );
				countryElem->SetAttribute( "Exist", true );
				bool hasChanges = false;
				if (countryFwd.m_funds != countryPrev.m_funds) {
					countryElem->SetAttribute( "PrevFunds", countryPrev.m_funds );
					countryElem->SetAttribute( "NewFunds", countryFwd.m_funds );
					hasChanges = true;
				}
				if (countryFwd.m_countryCultureID != countryPrev.m_countryCultureID) {
					countryElem->SetAttribute( "PrevCountryCulture", map->m_cultures[countryPrev.m_countryCultureID] );
					countryElem->SetAttribute( "NewCountryCulture", map->m_cultures[countryFwd.m_countryCultureID] );
					hasChanges = true;
				}
				if (countryFwd.m_countryReligionID != countryPrev.m_countryReligionID) {
					countryElem->SetAttribute( "PrevReligionCulture", map->m_religions[countryPrev.m_countryReligionID] );
					countryElem->SetAttribute( "NewReligionCulture", map->m_religions[countryFwd.m_countryReligionID] );
					hasChanges = true;
				}
				if (countryFwd.m_capitalCityID != countryPrev.m_capitalCityID) {
					if (countryPrev.m_capitalCityID != -1) {
						countryElem->SetAttribute( "PrevCapitalCity", map->m_cities[countryPrev.m_capitalCityID]->m_name.c_str() );
					}
					if (countryFwd.m_capitalCityID != -1) {
						countryElem->SetAttribute( "NewCapitalCity", map->m_cities[countryFwd.m_capitalCityID]->m_name.c_str() );
					}
					hasChanges = true;
				}
				if (countryFwd.m_capitalProvID != countryPrev.m_capitalProvID) {
					countryElem->SetAttribute( "PrevCapitalProv", map->m_mapPolygonUnits[countryPrev.m_capitalProvID]->m_name.c_str() );
					countryElem->SetAttribute( "NewCapitalProv", map->m_mapPolygonUnits[countryFwd.m_capitalProvID]->m_name.c_str() );
					hasChanges = true;
				}

				if (countryFwd.m_isCelestial != countryPrev.m_isCelestial) {
					countryElem->SetAttribute( "PrevCelestial", countryPrev.m_isCelestial );
					countryElem->SetAttribute( "NewCelestial", countryFwd.m_isCelestial );
					hasChanges = true;
				}
				if (countryFwd.m_governmentType != countryPrev.m_governmentType) {
					countryElem->SetAttribute( "PrevGovernmentType", g_governmentTypeDict[countryPrev.m_governmentType] );
					countryElem->SetAttribute( "NewGovernmentType", g_governmentTypeDict[countryFwd.m_governmentType] );
					hasChanges = true;
				}

				bool hasRelationChanges = false;
				XmlElement* countryRelations = history.NewElement( "CountryRelations" );
				if (countryFwd.m_friendlyCountriesID != countryPrev.m_friendlyCountriesID) {
					std::vector<int> loseRelationCountries;
					std::vector<int> gainRelationCountries;
					std::vector<int> prevCountryID = countryPrev.m_friendlyCountriesID;
					std::vector<int> fwdCountryID = countryFwd.m_friendlyCountriesID;
					GetCountryRelationDifference( loseRelationCountries, gainRelationCountries, prevCountryID, fwdCountryID );
					for (auto id : loseRelationCountries) {
						XmlElement* countryRelation = history.NewElement( "LoseRelation" );
						countryRelation->SetAttribute( "Type", "Friendly" );
						countryRelation->SetAttribute( "Target", map->m_countries[id]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						hasRelationChanges = true;
					}
					for (auto id : gainRelationCountries) {
						XmlElement* countryRelation = history.NewElement( "GainRelation" );
						countryRelation->SetAttribute( "Type", "Friendly" );
						countryRelation->SetAttribute( "Target", map->m_countries[id]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						hasRelationChanges = true;
					}
				}
				if (countryFwd.m_allianceCountriesID != countryPrev.m_allianceCountriesID) {
					std::vector<int> loseRelationCountries;
					std::vector<int> gainRelationCountries;
					std::vector<int> prevCountryID = countryPrev.m_allianceCountriesID;
					std::vector<int> fwdCountryID = countryFwd.m_allianceCountriesID;
					GetCountryRelationDifference( loseRelationCountries, gainRelationCountries, prevCountryID, fwdCountryID );
					for (auto id : loseRelationCountries) {
						XmlElement* countryRelation = history.NewElement( "LoseRelation" );
						countryRelation->SetAttribute( "Type", "Alliance" );
						countryRelation->SetAttribute( "Target", map->m_countries[id]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						hasRelationChanges = true;
					}
					for (auto id : gainRelationCountries) {
						XmlElement* countryRelation = history.NewElement( "GainRelation" );
						countryRelation->SetAttribute( "Type", "Alliance" );
						countryRelation->SetAttribute( "Target", map->m_countries[id]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						hasRelationChanges = true;
					}
				}
				if (countryFwd.m_hostileCountriesID != countryPrev.m_hostileCountriesID) {
					std::vector<int> loseRelationCountries;
					std::vector<int> gainRelationCountries;
					std::vector<int> prevCountryID = countryPrev.m_hostileCountriesID;
					std::vector<int> fwdCountryID = countryFwd.m_hostileCountriesID;
					GetCountryRelationDifference( loseRelationCountries, gainRelationCountries, prevCountryID, fwdCountryID );
					for (auto id : loseRelationCountries) {
						XmlElement* countryRelation = history.NewElement( "LoseRelation" );
						countryRelation->SetAttribute( "Type", "Hostile" );
						countryRelation->SetAttribute( "Target", map->m_countries[id]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						hasRelationChanges = true;
					}
					for (auto id : gainRelationCountries) {
						XmlElement* countryRelation = history.NewElement( "GainRelation" );
						countryRelation->SetAttribute( "Type", "Hostile" );
						countryRelation->SetAttribute( "Target", map->m_countries[id]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						hasRelationChanges = true;
					}
				}
				if (countryFwd.m_warCountriesID != countryPrev.m_warCountriesID) {
					std::vector<int> loseRelationCountries;
					std::vector<int> gainRelationCountries;
					std::vector<int> prevCountryID; 
					prevCountryID.reserve( countryPrev.m_warCountriesID.size() );
					for (auto& pair : countryPrev.m_warCountriesID) {
						prevCountryID.push_back( pair.first );
					}
					std::vector<int> fwdCountryID;
					fwdCountryID.reserve( countryFwd.m_warCountriesID.size() );
					for (auto& pair : countryFwd.m_warCountriesID) {
						fwdCountryID.push_back( pair.first );
					}
					GetCountryRelationDifference( loseRelationCountries, gainRelationCountries, prevCountryID, fwdCountryID );
					for (auto id : loseRelationCountries) {
						XmlElement* countryRelation = history.NewElement( "LoseRelation" );
						countryRelation->SetAttribute( "Type", "War" );
						countryRelation->SetAttribute( "Target", map->m_countries[id]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						hasRelationChanges = true;
					}
					for (auto id : gainRelationCountries) {
						XmlElement* countryRelation = history.NewElement( "GainRelation" );
						countryRelation->SetAttribute( "Type", "War" );
						countryRelation->SetAttribute( "Target", map->m_countries[id]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						hasRelationChanges = true;
					}
				}
				if (countryFwd.m_suzerainCountryID != countryPrev.m_suzerainCountryID) {
					if (countryPrev.m_suzerainCountryID != -1 && countryFwd.m_suzerainCountryID == -1) {
						XmlElement* countryRelation = history.NewElement( "LoseRelation" );
						countryRelation->SetAttribute( "Type", "BeingVassal" );
						countryRelation->SetAttribute( "Target", map->m_countries[countryPrev.m_suzerainCountryID]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						hasRelationChanges = true;
					}
					else if (countryPrev.m_suzerainCountryID == -1 && countryFwd.m_suzerainCountryID != -1) {
						XmlElement* countryRelation = history.NewElement( "GainRelation" );
						countryRelation->SetAttribute( "Type", "BeingVassal" );
						countryRelation->SetAttribute( "Target", map->m_countries[countryFwd.m_suzerainCountryID]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						hasRelationChanges = true;
					}
					else {
						XmlElement* countryRelation = history.NewElement( "LoseRelation" );
						countryRelation->SetAttribute( "Type", "BeingVassal" );
						countryRelation->SetAttribute( "Target", map->m_countries[countryPrev.m_suzerainCountryID]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						countryRelation = history.NewElement( "GainRelation" );
						countryRelation->SetAttribute( "Type", "BeingVassal" );
						countryRelation->SetAttribute( "Target", map->m_countries[countryFwd.m_suzerainCountryID]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						hasRelationChanges = true;
					}
				}
				if (countryFwd.m_vassalCountriesID != countryPrev.m_vassalCountriesID) {
					std::vector<int> loseRelationCountries;
					std::vector<int> gainRelationCountries;
					std::vector<int> prevCountryID = countryPrev.m_vassalCountriesID;
					std::vector<int> fwdCountryID = countryFwd.m_vassalCountriesID;
					GetCountryRelationDifference( loseRelationCountries, gainRelationCountries, prevCountryID, fwdCountryID );
					for (auto id : loseRelationCountries) {
						XmlElement* countryRelation = history.NewElement( "LoseRelation" );
						countryRelation->SetAttribute( "Type", "Vassal" );
						countryRelation->SetAttribute( "Target", map->m_countries[id]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						hasRelationChanges = true;
					}
					for (auto id : gainRelationCountries) {
						XmlElement* countryRelation = history.NewElement( "GainRelation" );
						countryRelation->SetAttribute( "Type", "Vassal" );
						countryRelation->SetAttribute( "Target", map->m_countries[id]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						hasRelationChanges = true;
					}
				}
				if (countryFwd.m_celestialCountryID != countryPrev.m_celestialCountryID) {
					if (countryPrev.m_celestialCountryID != -1 && countryFwd.m_celestialCountryID == -1) {
						XmlElement* countryRelation = history.NewElement( "LoseRelation" );
						countryRelation->SetAttribute( "Type", "BeingTributary" );
						countryRelation->SetAttribute( "Target", map->m_countries[countryPrev.m_celestialCountryID]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
					}
					else if (countryPrev.m_celestialCountryID == -1 && countryFwd.m_celestialCountryID != -1) {
						XmlElement* countryRelation = history.NewElement( "GainRelation" );
						countryRelation->SetAttribute( "Type", "BeingTributary" );
						countryRelation->SetAttribute( "Target", map->m_countries[countryFwd.m_celestialCountryID]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
					}
					else {
						XmlElement* countryRelation = history.NewElement( "LoseRelation" );
						countryRelation->SetAttribute( "Type", "BeingTributary" );
						countryRelation->SetAttribute( "Target", map->m_countries[countryPrev.m_celestialCountryID]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						countryRelation = history.NewElement( "GainRelation" );
						countryRelation->SetAttribute( "Type", "BeingTributary" );
						countryRelation->SetAttribute( "Target", map->m_countries[countryFwd.m_celestialCountryID]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
					}
					hasRelationChanges = true;
				}
				if (countryFwd.m_tributaryCountriesID != countryPrev.m_tributaryCountriesID) {
					std::vector<int> loseRelationCountries;
					std::vector<int> gainRelationCountries;
					std::vector<int> prevCountryID = countryPrev.m_tributaryCountriesID;
					std::vector<int> fwdCountryID = countryFwd.m_tributaryCountriesID;
					GetCountryRelationDifference( loseRelationCountries, gainRelationCountries, prevCountryID, fwdCountryID );
					for (auto id : loseRelationCountries) {
						XmlElement* countryRelation = history.NewElement( "LoseRelation" );
						countryRelation->SetAttribute( "Type", "Tributary" );
						countryRelation->SetAttribute( "Target", map->m_countries[id]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						hasRelationChanges = true;
					}
					for (auto id : gainRelationCountries) {
						XmlElement* countryRelation = history.NewElement( "GainRelation" );
						countryRelation->SetAttribute( "Type", "Tributary" );
						countryRelation->SetAttribute( "Target", map->m_countries[id]->m_name.c_str() );
						countryRelations->InsertEndChild( countryRelation );
						hasRelationChanges = true;
					}
				}
				if (hasRelationChanges) {
					hasChanges = true;
					countryElem->InsertEndChild( countryRelations );
				}
				if (hasChanges) {
					countryRootElem->InsertEndChild( countryElem );
				}
			}
		}
		else {
			if ((int)dataThis.m_countryData.size() <= i) { // new country but not exist
				XmlElement* countryElem = history.NewElement( "Country" );
				provRootElem->InsertEndChild( countryElem );
				countryElem->SetAttribute( "New", true );
				countryElem->SetAttribute( "Exist", false );
				countryRootElem->InsertEndChild( countryElem );
			}
			else if (dataThis.m_countryData[i].m_exist) { // country become not exist
				XmlElement* countryElem = history.NewElement( "Country" );
				countryElem->SetAttribute( "New", false );
				countryElem->SetAttribute( "Exist", false );
				countryRootElem->InsertEndChild( countryElem );
			}
		}
	}

	// Armies
	XmlElement* armyRootElem = history.NewElement( "Armies" );
	rootElem->InsertEndChild( armyRootElem );
	for (int i = 0; i < dataThis.m_armyData.size(); ++i) {
		XmlElement* armyElem = history.NewElement( "Army" );
		HistoryArmyData const& armyPrev = dataThis.m_armyData[i];
		bool findInFwd = false;
		bool isChanged = false;
		for (int j = 0; j < dataFwd.m_armyData.size(); ++j) {
			// exist in both history file
			if (dataFwd.m_armyData[j].m_globalID == armyPrev.m_globalID) {
				HistoryArmyData const& armyFwd = dataFwd.m_armyData[j];
				findInFwd = true;
				if (armyPrev.m_combatValue != armyFwd.m_combatValue) {
					armyElem->SetAttribute( "PrevCombatValue", armyPrev.m_combatValue );
					armyElem->SetAttribute( "NewCombatValue", armyFwd.m_combatValue );
					isChanged = true;
				}
				if (armyPrev.m_size != armyFwd.m_size) {
					armyElem->SetAttribute( "PrevSize", armyPrev.m_size );
					armyElem->SetAttribute( "NewSize", armyFwd.m_size );
					isChanged = true;
				}
				if (armyPrev.m_provInID != armyFwd.m_provInID) {
					armyElem->SetAttribute( "PrevProvinceID", armyPrev.m_provInID );
					armyElem->SetAttribute( "NewProvinceID", armyFwd.m_provInID );
					isChanged = true;
				}
				break;
			}
		}
		if (!findInFwd) {
			// only exist in prev history file
			armyElem->SetAttribute( "IsDismissed", true );
			isChanged = true;
		}
		if (isChanged) {
			armyRootElem->InsertEndChild( armyElem );
		}
	}
	for (int i = 0; i < dataFwd.m_armyData.size(); ++i) {
		XmlElement* armyElem = history.NewElement( "Army" );
		HistoryArmyData const& armyFwd = dataFwd.m_armyData[i];
		bool findInFwd = false;
		for (int j = 0; j < dataThis.m_armyData.size(); ++j) {
			if (dataThis.m_armyData[j].m_globalID == armyFwd.m_globalID) {
				findInFwd = true;
			}
		}
		if (!findInFwd) {
			armyElem->SetAttribute( "IsBuilt", true );
			armyElem->SetAttribute( "NewCombatValue", armyFwd.m_combatValue );
			armyElem->SetAttribute( "NewSize", armyFwd.m_size );
			armyElem->SetAttribute( "NewProvinceID", armyFwd.m_provInID );
			armyElem->SetAttribute( "NewProvinceName", map->m_mapPolygonUnits[armyFwd.m_provInID]->m_name.c_str() );
			armyRootElem->InsertEndChild( armyElem );
		}
	}

	// Crisis
	XmlElement* crisisRootElem = history.NewElement( "AllCrisis" );
	rootElem->InsertEndChild( crisisRootElem );
	for (int i = 0; i < dataThis.m_crisisData.size(); ++i) {
		XmlElement* crisisElem = history.NewElement( "Crisis" );
		HistoryCrisisData const& crisisPrev = dataThis.m_crisisData[i];
		bool findInFwd = false;
		bool isChanged = false;
		for (int j = 0; j < dataFwd.m_crisisData.size(); ++j) {
			// exist in both history file
			if (dataFwd.m_crisisData[j].m_globalID == crisisPrev.m_globalID) {
				HistoryCrisisData const& crisisFwd = dataThis.m_crisisData[j];
				findInFwd = true;
				if (crisisPrev.m_progress != crisisFwd.m_progress) {
					crisisElem->SetAttribute( "PrevProgress", crisisPrev.m_progress );
					crisisElem->SetAttribute( "NewProgress", crisisFwd.m_progress );
					isChanged = true;
				}
				break;
			}
		}
		if (!findInFwd) {
			// only exist in prev history file
			crisisElem->SetAttribute( "IsTriggered", true );
			isChanged = true;
		}
		if (isChanged) {
			crisisRootElem->InsertEndChild( crisisElem );
		}
	}
	for (int i = 0; i < dataFwd.m_crisisData.size(); ++i) {
		XmlElement* crisisElem = history.NewElement( "Crisis" );
		HistoryCrisisData const& crisisFwd = dataFwd.m_crisisData[i];
		bool findInFwd = false;
		for (int j = 0; j < dataThis.m_crisisData.size(); ++j) {
			if (dataThis.m_crisisData[j].m_globalID == crisisFwd.m_globalID) {
				findInFwd = true;
			}
		}
		if (!findInFwd) {
			crisisElem->SetAttribute( "IsBegan", true );
			if (crisisFwd.m_type == 2) {
				crisisElem->SetAttribute( "Type", "CivilWar" );
			}
			else if (crisisFwd.m_type == 0) {
				crisisElem->SetAttribute( "Type", "CulturalConflict" );
				crisisElem->SetAttribute( "CultureID", crisisFwd.m_cultureOrReligionID );
				crisisElem->SetAttribute( "CultureName", map->m_countries[crisisFwd.m_cultureOrReligionID]->m_name.c_str() );
			}
			else if (crisisFwd.m_type == 1) {
				crisisElem->SetAttribute( "Type", "ReligionConflict" );
				crisisElem->SetAttribute( "Religion", crisisFwd.m_cultureOrReligionID );
				crisisElem->SetAttribute( "ReligionName", map->m_religions[crisisFwd.m_cultureOrReligionID]->m_name.c_str() );
			}
			crisisElem->SetAttribute( "NewProgress", crisisFwd.m_progress );
			crisisElem->SetAttribute( "CountryID", crisisFwd.m_countryID );
			crisisElem->SetAttribute( "CountryName", map->m_countries[crisisFwd.m_countryID]->m_name.c_str() );
			crisisRootElem->InsertEndChild( crisisElem );
		}
	}


	// output data
	std::filesystem::create_directory( Stringf( "Saves/%d/HistorySaves", map->m_generationSettings.m_seed ).c_str() );
	XmlError errorCode = history.SaveFile( Stringf( "Saves/%d/HistorySaves/World_Year%d_Month%dHistory.xml", map->m_generationSettings.m_seed, m_year, m_month ).c_str() );
	if (errorCode != tinyxml2::XML_SUCCESS) {
		ERROR_RECOVERABLE( "Cannot save settings" );
	}

	// change the state of history solver
	m_historySolver->m_mutex.lock();
	m_historySolver->m_savingFlag[m_year * 12 + m_month - 1] = true;
	m_historySolver->m_mutex.unlock();
}

void SaveSingleMonthHistoryJob::GetCountryRelationDifference( std::vector<int>& loseRelationCountries, std::vector<int>& gainRelationCountries, std::vector<int>& prevCountryID, std::vector<int>& fwdCountryID )
{
	loseRelationCountries.clear();
	gainRelationCountries.clear();
	std::sort( prevCountryID.begin(), prevCountryID.end() );
	std::sort( fwdCountryID.begin(), fwdCountryID.end() );
	int i = 0, j = 0;
	for (;;) {
		if (i == (int)prevCountryID.size()) {
			while (j < (int)fwdCountryID.size()) {
				gainRelationCountries.push_back( fwdCountryID[j] );
				++j;
			}
			break;
		}
		else if (j == (int)fwdCountryID.size()) {
			while (i < (int)prevCountryID.size()) {
				loseRelationCountries.push_back( prevCountryID[i] );
				++i;
			}
			break;
		}
		else if (prevCountryID[i] == fwdCountryID[j]) {
			++i; ++j;
		}
		else if (prevCountryID[i] > fwdCountryID[j]) {
			gainRelationCountries.push_back( fwdCountryID[j] );
			++j;
		}
		else if (prevCountryID[i] < fwdCountryID[j]) {
			loseRelationCountries.push_back( prevCountryID[i] );
			++i;
		}
	}
}

HistorySavingSolver::HistorySavingSolver( Map* map )
	:m_map(map)
{

}

void HistorySavingSolver::StartSave()
{
	m_isSaving = true;
	while (m_savingFlag.size() < m_map->GetTotalMonthCount()) {
		m_savingFlag.push_back( false );
	}
	for (int i = 0; i < (int)m_savingFlag.size(); ++i) {
		if (m_savingFlag[i] == false) {
			int year = 0; int month = 0;
			m_map->GetYearAndMonthFromTotalMonth( year, month, i );
			SaveSingleMonthHistoryJob* job = new SaveSingleMonthHistoryJob( this, year, month );
			//job->m_type = Loading_Job;
			g_theJobSystem->AddJob( job );
			m_jobList.push_back( job );
		}
	}
}

bool HistorySavingSolver::IsSaving() const
{
	return m_isSaving;
}

void HistorySavingSolver::GetSavingProgress( int& curHistory, int& totalHistory )
{
	m_mutex.lock();
	curHistory = 0;
	totalHistory = (int)m_savingFlag.size();
	for (int i = 0; i < (int)m_savingFlag.size(); ++i) {
		if (m_savingFlag[i] == true) {
			++curHistory;
		}
	}
	m_mutex.unlock();
}

void HistorySavingSolver::Update()
{
	for (;;) {
		if (!m_jobList.empty()) {
			if (g_theJobSystem->RetrieveJob( m_jobList[0] )) {
				delete m_jobList[0];
				m_jobList.pop_front();
			}
		}
		else {
			break;
		}
	}
	m_isSaving = false;
}
