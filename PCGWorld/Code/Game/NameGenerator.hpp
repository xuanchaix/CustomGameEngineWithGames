#pragma once
#include "Game/GameCommon.hpp"

class MapPolygonUnit;
class Country;
class Religion;
class City;
class Town;

class NameGenerator {
public:
	NameGenerator( unsigned int seed );


	unsigned int m_seed = 0;
	RandomNumberGenerator m_rng;
};

class CultureNameGenerator : public NameGenerator {
public:
	CultureNameGenerator( unsigned int seed );
	std::string GenerateRandomString( int length, bool isPrefix );
	std::string GenerateCultureName();
	std::vector<std::string> m_prefixes;
	std::vector<std::string> m_middleSegments;
	std::vector<std::string> m_suffixes;
};

class ReligionNameGenerator : public NameGenerator {
public:
	ReligionNameGenerator( unsigned int seed );
	std::string GenerateRandomString( int length, bool isPrefix );
	std::string GenerateReligionName();
	std::vector<std::string> m_prefixes;
	std::vector<std::string> m_middleSegments;
	std::vector<std::string> m_suffixes;
};

class CountryNameGenerator : public NameGenerator {
public:
	CountryNameGenerator( unsigned int seed, std::string const& cultureName );
	std::string GenerateCountryName( Country* country );
	std::string GenerateRandomString( int length, bool isPrefix );

	std::string m_cultureName;
	std::vector<std::string> m_prefixes;
	std::vector<std::string> m_middleSegments;
	std::vector<std::string> m_suffixes;
	std::string m_suffixEmpire;
	std::string m_suffixDuchy;
	std::string m_suffixKingdom;
};

class CityTownNameGenerator : public NameGenerator {
public:
	CityTownNameGenerator( unsigned int seed, std::string const& cultureName );
	std::string GenerateCityName( City* city );
	std::string GenerateTownName( Town* town );
	std::string GenerateRandomString( int length, bool isPrefix );

	std::string m_cultureName;
	std::vector<std::string> m_prefixes;
	std::vector<std::string> m_middleSegments;
	std::vector<std::string> m_suffixes;
};

class ProvinceNameGenerator : public NameGenerator {
public:
	ProvinceNameGenerator( unsigned int seed, std::string const& cultureName );
	std::string GenerateProvinceName( MapPolygonUnit* city );
	std::string GenerateRandomString( int length, bool isPrefix );

	std::string m_cultureName;
	std::vector<std::string> m_prefixes;
	std::vector<std::string> m_middleSegments;
	std::vector<std::string> m_suffixes;
};

class HumanNameGenerator : public NameGenerator {
public:
	HumanNameGenerator( unsigned int seed, std::string const& cultureName );

	std::string m_cultureName;
};
