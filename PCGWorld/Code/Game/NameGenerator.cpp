#include "Game/NameGenerator.hpp"
#include "Game/Country.hpp"
#include "Game/Culture.hpp"
#include "Game/Map.hpp"
#include "Game/City.hpp"

const std::vector<std::string> consonantClusters = {
	"th", "wh", "ch", "sh", "cr", "dr", "fr", "gl", "br", "pl", "sn", "sp", "st"
};
const std::string vowels = "aeiou";
const std::string consonants = "bcdfghjklmnpqrstvwxyz";
const std::vector<std::string> countrySuffixEmpire = {
	"Empire", "Caliphate", "Sultanate", "Commonwealth", "Celestial Empire", "Tsardom"
};
const std::vector<std::string> countrySuffixKingdom = {
	"Kingdom", "Sultanate", "Shahdom", "Chieftaincy", "Pashalik", "Beylik",
};
const std::vector<std::string> countrySuffixDuchy = {
	"Duchy", "Emirate", "Grand Duchy", "County", "Marquisate", "Lordship", "Raj", "Wilayah", "Realm", 
};
const std::vector<std::string> nameSuffixes = {
	"ion", "ia", "a", "y", "th", "an", "land", "ce", "m", "t", "dor", "k", "s",
};
const std::vector<std::string> cityNameSuffixes = {
	"ion", "ia", "a", "y", "th", "an", "land", "ce", "m", "t", "dor", "k", "s", "sk", "bolu", "burg"
};
const std::vector<std::string> religionNameSuffixes = {
	"ious", "ian", "t", "c", "x", "an", "ism",
};

NameGenerator::NameGenerator( unsigned int seed )
	:m_seed(seed)
{
	m_rng.SetSeed( seed );
}

CityTownNameGenerator::CityTownNameGenerator( unsigned int seed, std::string const& cultureName )
	:NameGenerator(seed), m_cultureName(cultureName)
{
	for (int i = 0; i < 200; ++i) {
		m_prefixes.push_back( GenerateRandomString( m_rng.RollRandomIntInRange( 2, 6 ), true ) );
		m_middleSegments.push_back( GenerateRandomString( m_rng.RollRandomIntInRange( 1, 2 ), false ) );
		m_suffixes = cityNameSuffixes;
	}
}

std::string CityTownNameGenerator::GenerateCityName( City* city )
{
	if (city->m_provIn->m_cities[0] == city) {
		if (m_rng.RollRandomFloatZeroToOne() < 0.1f) {
			return city->m_provIn->m_name;
		}
	}
	// Randomly decide whether to include a middle segment
	bool includeMiddle = m_rng.RollRandomIntLessThan( 2 ); // 50% chance to include a middle segment

	std::string name = m_prefixes[m_rng.RollRandomIntLessThan( (int)m_prefixes.size() )];

	if (includeMiddle) {
		name += m_middleSegments[m_rng.RollRandomIntLessThan( (int)m_middleSegments.size() )];
	}

	name += m_suffixes[m_rng.RollRandomIntLessThan( (int)m_suffixes.size() )];

	return name;
}

std::string CityTownNameGenerator::GenerateTownName( Town* town )
{
	UNUSED( town );
	// Randomly decide whether to include a middle segment
	bool includeMiddle = m_rng.RollRandomIntLessThan( 2 ); // 50% chance to include a middle segment

	std::string name = m_prefixes[m_rng.RollRandomIntLessThan( (int)m_prefixes.size() )];

	if (includeMiddle) {
		name += m_middleSegments[m_rng.RollRandomIntLessThan( (int)m_middleSegments.size() )];
	}

	name += m_suffixes[m_rng.RollRandomIntLessThan( (int)m_suffixes.size() )];

	return name;
}

std::string CityTownNameGenerator::GenerateRandomString( int length, bool isPrefix )
{
	std::string result;

	// Alternate between consonants and vowels
	for (int i = 0; i < length; ++i) {
		if (i % 2 == 0) {
			if (m_rng.RollRandomFloatZeroToOne() < 0.75f) {
				result += consonants[m_rng.RollRandomIntLessThan( (int)consonants.size() )];
			}
			else {
				result += consonantClusters[m_rng.RollRandomIntLessThan( (int)consonantClusters.size() )];
			}
		}
		else {
			result += vowels[m_rng.RollRandomIntLessThan( (int)vowels.size() )];
		}
	}

	// Capitalize the first letter for prefixes
	if (isPrefix) {
		result[0] = (char)toupper( result[0] );
	}

	return result;
}

HumanNameGenerator::HumanNameGenerator( unsigned int seed, std::string const& cultureName )
	:NameGenerator( seed ), m_cultureName(cultureName)
{
}

CountryNameGenerator::CountryNameGenerator( unsigned int seed, std::string const& cultureName )
	:NameGenerator( seed ), m_cultureName(cultureName)
{
	for (int i = 0; i < 200; ++i) {
		m_prefixes.push_back( GenerateRandomString( m_rng.RollRandomIntInRange( 2, 6 ), true ) );
		m_middleSegments.push_back( GenerateRandomString( m_rng.RollRandomIntInRange( 1, 2 ), false ) );
		m_suffixes = nameSuffixes;
	}
	if (GetCurGenerationSettings().m_onlyUseWesternCountryPrefix) {
		m_suffixDuchy = "Duchy";
		m_suffixKingdom = "Kingdom";
		m_suffixEmpire = "Empire";
	}
	else {
		m_suffixDuchy = countrySuffixDuchy[m_rng.RollRandomIntLessThan( (int)countrySuffixDuchy.size() )];
		m_suffixKingdom = countrySuffixKingdom[m_rng.RollRandomIntLessThan( (int)countrySuffixKingdom.size() )];
		m_suffixEmpire = countrySuffixEmpire[m_rng.RollRandomIntLessThan( (int)countrySuffixEmpire.size() )];
	}
}

std::string CountryNameGenerator::GenerateCountryName( Country* country )
{
	// suffix: empire, kingdom...

	// empire
	if ((country->m_totalPopulation > 1500000 && (float)country->m_cultures[0].second / (float)country->m_totalPopulation < 0.75f) || country->m_countryCulture->m_initialState == 6) {
		return m_suffixEmpire + " of " + country->m_countryCulture->m_name;
	}
	// unified kingdom
	else if (country->m_countryCulture->m_initialState == 3 || (country->m_countryCulture->m_initialState == 5 && country->HasVassal())) {
		return m_suffixKingdom + " of " + country->m_countryCulture->m_name;
	}
	else { // duchy
		bool includeMiddle = m_rng.RollRandomIntLessThan( 2 ); // 50% chance to include a middle segment

		std::string name = m_suffixDuchy + " of ";

		if (m_rng.RollRandomFloatZeroToOne() < 0.3f || country->m_capitalProv == nullptr) {
			name += m_prefixes[m_rng.RollRandomIntLessThan( (int)m_prefixes.size() )];

			if (includeMiddle) {
				name += m_middleSegments[m_rng.RollRandomIntLessThan( (int)m_middleSegments.size() )];
			}

			name += m_suffixes[m_rng.RollRandomIntLessThan( (int)m_suffixes.size() )];
		}
		else {
			name += country->m_capitalProv->m_name;
		}

		return name;
	}
}

std::string CountryNameGenerator::GenerateRandomString( int length, bool isPrefix )
{
	std::string result;

	// Alternate between consonants and vowels
	for (int i = 0; i < length; ++i) {
		if (i % 2 == 0) {
			if (m_rng.RollRandomFloatZeroToOne() < 0.75f) {
				result += consonants[m_rng.RollRandomIntLessThan( (int)consonants.size() )];
			}
			else {
				result += consonantClusters[m_rng.RollRandomIntLessThan( (int)consonantClusters.size() )];
			}
		}
		else {
			result += vowels[m_rng.RollRandomIntLessThan( (int)vowels.size() )];
		}
	}

	// Capitalize the first letter for prefixes
	if (isPrefix) {
		result[0] = (char)toupper( result[0] );
	}

	return result;
}

CultureNameGenerator::CultureNameGenerator( unsigned int seed )
	:NameGenerator( seed )
{
	for (int i = 0; i < 100; ++i) {
		m_prefixes.push_back( GenerateRandomString( m_rng.RollRandomIntInRange( 2, 6 ) + 2, true ) ); // Length 2-4
		m_middleSegments.push_back( GenerateRandomString( m_rng.RollRandomIntInRange( 1, 2 ) + 1, false ) ); // Length 1-2
		//m_suffixes.push_back( GenerateRandomString( m_rng.RollRandomIntInRange(0, 2) + 2, false ) ); // Length 2-4
	}
	m_suffixes = nameSuffixes;
}


std::string CultureNameGenerator::GenerateRandomString( int length, bool isPrefix ) {
	std::string result;

	// Alternate between consonants and vowels
	for (int i = 0; i < length; ++i) {
		if (i % 2 == 0) {
			if (m_rng.RollRandomFloatZeroToOne() < 0.75f) {
				result += consonants[m_rng.RollRandomIntLessThan( (int)consonants.size() )];
			}
			else {
				result += consonantClusters[m_rng.RollRandomIntLessThan( (int)consonantClusters.size() )];
			}
		}
		else {
			result += vowels[m_rng.RollRandomIntLessThan( (int)vowels.size() )];
		}
	}

	// Capitalize the first letter for prefixes
	if (isPrefix) {
		result[0] = (char)toupper( result[0] );
	}

	return result;
}

std::string CultureNameGenerator::GenerateCultureName()
{
	bool includeMiddle = m_rng.RollRandomIntLessThan( 2 ); // 50% chance to include a middle segment

	std::string name = m_prefixes[m_rng.RollRandomIntLessThan( (int)m_prefixes.size() )];

	if (includeMiddle) {
		name += m_middleSegments[m_rng.RollRandomIntLessThan( (int)m_middleSegments.size() )];
	}

	name += m_suffixes[m_rng.RollRandomIntLessThan( (int)m_suffixes.size() )];

	return name;
	
}

ReligionNameGenerator::ReligionNameGenerator( unsigned int seed )
	:NameGenerator( seed )
{
	for (int i = 0; i < 100; ++i) {
		m_prefixes.push_back( GenerateRandomString( m_rng.RollRandomIntInRange( 2, 6 ) + 2, true ) ); // Length 2-4
		m_middleSegments.push_back( GenerateRandomString( m_rng.RollRandomIntInRange( 1, 2 ) + 1, false ) ); // Length 1-2
		//m_suffixes.push_back( GenerateRandomString( m_rng.RollRandomIntInRange(0, 2) + 2, false ) ); // Length 2-4
	}
	m_suffixes = religionNameSuffixes;
}

std::string ReligionNameGenerator::GenerateRandomString( int length, bool isPrefix )
{
	std::string result;
	// Alternate between consonants and vowels
	for (int i = 0; i < length; ++i) {
		if (i % 2 == 0) {
			if (m_rng.RollRandomFloatZeroToOne() < 0.75f) {
				result += consonants[m_rng.RollRandomIntLessThan( (int)consonants.size() )];
			}
			else {
				result += consonantClusters[m_rng.RollRandomIntLessThan( (int)consonantClusters.size() )];
			}
		}
		else {
			result += vowels[m_rng.RollRandomIntLessThan( (int)vowels.size() )];
		}
	}

	// Capitalize the first letter for prefixes
	if (isPrefix) {
		result[0] = (char)toupper( result[0] );
	}

	return result;
}

std::string ReligionNameGenerator::GenerateReligionName()
{
	bool includeMiddle = m_rng.RollRandomIntLessThan( 2 ); // 50% chance to include a middle segment

	std::string name = m_prefixes[m_rng.RollRandomIntLessThan( (int)m_prefixes.size() )];

	if (includeMiddle) {
		name += m_middleSegments[m_rng.RollRandomIntLessThan( (int)m_middleSegments.size() )];
	}

	name += m_suffixes[m_rng.RollRandomIntLessThan( (int)m_suffixes.size() )];

	return name;
}

ProvinceNameGenerator::ProvinceNameGenerator( unsigned int seed, std::string const& cultureName )
	:NameGenerator( seed ), m_cultureName( cultureName )
{
	for (int i = 0; i < 200; ++i) {
		m_prefixes.push_back( GenerateRandomString( m_rng.RollRandomIntInRange( 2, 6 ), true ) );
		m_middleSegments.push_back( GenerateRandomString( m_rng.RollRandomIntInRange( 1, 2 ), false ) );
		m_suffixes = nameSuffixes;
	}
}

std::string ProvinceNameGenerator::GenerateProvinceName( MapPolygonUnit* prov )
{
	UNUSED( prov );
	// Randomly decide whether to include a middle segment
	bool includeMiddle = m_rng.RollRandomIntLessThan( 2 ); // 50% chance to include a middle segment

	std::string name = m_prefixes[m_rng.RollRandomIntLessThan( (int)m_prefixes.size() )];

	if (includeMiddle) {
		name += m_middleSegments[m_rng.RollRandomIntLessThan( (int)m_middleSegments.size() )];
	}

	name += m_suffixes[m_rng.RollRandomIntLessThan( (int)m_suffixes.size() )];

	return name;
}

std::string ProvinceNameGenerator::GenerateRandomString( int length, bool isPrefix )
{
	std::string result;

	// Alternate between consonants and vowels
	for (int i = 0; i < length; ++i) {
		if (i % 2 == 0) {
			if (m_rng.RollRandomFloatZeroToOne() < 0.75f) {
				result += consonants[m_rng.RollRandomIntLessThan( (int)consonants.size() )];
			}
			else {
				result += consonantClusters[m_rng.RollRandomIntLessThan( (int)consonantClusters.size() )];
			}
		}
		else {
			result += vowels[m_rng.RollRandomIntLessThan( (int)vowels.size() )];
		}
	}

	// Capitalize the first letter for prefixes
	if (isPrefix) {
		result[0] = (char)toupper( result[0] );
	}

	return result;
}
