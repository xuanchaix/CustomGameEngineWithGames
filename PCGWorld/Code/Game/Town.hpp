#pragma once
#include "Game/GameCommon.hpp"

class MapPolygonUnit;
class Culture;
class Religion;
class Country;
class Region;

class Town {
public:
	Town();
	~Town();

	virtual void Initialize();
	virtual void GrowPopulationOneMonth();
	virtual float GetCultureInfluence( Culture* culture );
	virtual float GetReligionInfluence( Religion* religion );
	void RecalculateMajorCulture();
	void RecalculateMajorReligion();

	void ResolveChangePopulation( int prevPopulaiton );
	void GetUnselectedCultures( std::vector<Culture*>& out_cultures );
	void GetUnselectedReligions( std::vector<Religion*>& out_religions );
	void SqueezeReligionInfluence( Religion* religion, float influenceToAdd, float prevValue );
	void SqueezeCultureInfluence( Culture* culture, float influenceToAdd, float prevValue );
public:
	int m_id = 0;
	Vec2 m_position;
	int m_totalPopulation = 0;
	float m_defense = 0.f;
	float m_maxDefense = 0.f;
	bool m_warFlag = false;
	MapPolygonUnit* m_provIn = nullptr;
	std::string m_name;

	Country* m_owner = nullptr;
	Region* m_region = nullptr;

	Culture* m_majorCulture = nullptr;
	std::vector<std::pair<Culture*, float>> m_cultures;
	Religion* m_majorReligion = nullptr;
	std::vector<std::pair<Religion*, float>> m_religions;
	AABB2 m_iconBounds;
	AABB2 m_biggerIconBounds;
	uint8_t m_dirtyBits;
};