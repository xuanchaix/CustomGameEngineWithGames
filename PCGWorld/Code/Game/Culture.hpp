#pragma once
#include "Game/GameCommon.hpp"

class MapPolygonUnit;
class CountryNameGenerator;
class ProvinceNameGenerator;
class CityTownNameGenerator;

enum class CultureGeoOrigin {
	/*Origins*/RiverOrigin, MountainOrigin, OceanOrigin, GrasslandOrigin, ForestOrigin, Num,
	/*
	river origin: The people from this culture is relying on their mother river to cultivate and live. The flood plain is fertilized by the big river and provide numerous home lands for them.
	mountain origin: The people from this culture lives in huge mountains. By living in such cold and severe environment they are warlike and nomadic.
	ocean origin: The people from this culture lives near the sea. They are more likely to be merchants and be the commercial civilization.
	grassland origin: The people from this culture lives on grasslands. Since it's hard to cultivate, they are mostly nomadic and belligerent.
	forest origin: The people from this culture lives near or in the forests. They live by hunting and thus they form a unified organization.
	*/
};

enum class CultureTrait {
	Industrious, /*f+10% production*/
	Commercial, /*f+10% city tax*/
	Agrarian, /*f+10% province population growth*/
	//Ingenious, /**/
	Thrifty, /*f-1% cost as country culture*/
	Traditional, /*f+5% tax, -5% city tax*/
	Docile, /*f-50% culture conflict monthly progress as province culture*/
	Quarrelsome, /*f-5% combat damage as country culture*/
	Irritable, /*f+10% combat damage +10% damage sustained as country culture*/
	Filial, /*f(+5% population growth -30% civil war monthly progress) as country culture*/
	Free, /*f+50% culture conflict monthly progress as province culture*/
	Alarmist, /*f-10% province production*/
	Unruly, /*f-25% assimilation rate to this culture*/
	Nomadic, /*f(-10% population growth +10% combat damage) as country culture*/
	Sedentary, /*f+5% population growth as province culture -20% assimilation ability as country culture*/
	Communal, /*f+5% population growth in cities and towns, -5% damage sustained as country culture*/
	//Conformists, /**/
	//Deviants, /**/
	Vulnerable, /*f+10% damage sustained as country culture*/
	//Decadent, /**/
	Resilient, /*f+20% population growth in province with less population density*/
	Miserly, /*f(+5% tax, +50% relation instruction cost) as country culture*/
	Matrilineal, /*f+5% tax as province culture, +5% combat damage as country culture*/
	Conservationist, /*f-5% production, +5% tax as country culture*/
	Jinxed, /*f-5% city tax as province culture*/
	Egalitarian, /*f+10% production as province culture (-10% tax, -5% damage sustained as country culture)*/
	Xenophile, /*f-50% relation instruction cost, +25% recruit cost as country culture*/
	Xenophobe, /*f+100% relation instruction cost, -10% recruit cost, -10% army maintenance cost, +20% culture conflict monthly progress as country culture*/
	Pacifistic, /*f-20% relation instruction cost, +50% recruit cost, -20% army maintenance cost as country culture*/
	Militaristic, /*f-10% army maintenance cost as country culture*/
	Tolerant, /*f-20% culture conflict monthly progress -20% religion conflict monthly progress as country culture*/
	Patriarchy, /*f+5% population growth as province culture, +5% production as country culture*/
	NUM,
	None,
};

class Culture {
public:
	Culture(int id);
 
	int GetNumOfMajorCultureProvs() const;
	void GetAllMajorCultureProvs( std::vector<MapPolygonUnit*>& out_provs ) const;
	void GetBoundsPointsForLabel( Vec2& out_startPos, Vec2& out_endPos ) const;

	bool HasTrait( CultureTrait trait ) const;

	int m_id = -1;
	float m_influence = 1.f;
	int m_initialState = 0;
	MapPolygonUnit* m_cultureOriginUnit = nullptr;
	CultureGeoOrigin m_origin;
	CultureTrait m_traits[4] = { CultureTrait::None };
	Rgba8 m_color;
	CountryNameGenerator* m_countryNameGenerator = nullptr;
	ProvinceNameGenerator* m_provinceNameGenerator = nullptr;
	CityTownNameGenerator* m_cityTownNameGenerator = nullptr;
	std::string m_name;

	static std::vector<std::string> s_defaultNames;
};
