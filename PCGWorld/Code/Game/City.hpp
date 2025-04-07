#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Town.hpp"

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

enum class CityAttribute {
	Commercial, Fort, Capital, Port, AdjToRiver, AdjToSea,
};

constexpr auto City_Dirty_Flag_Population = 0x01u;
constexpr auto City_Dirty_Flag_Major_Culture = 0x02u;
constexpr auto City_Dirty_Flag_Cultures = 0x04u;
constexpr auto City_Dirty_Flag_Major_Religion = 0x08u;
constexpr auto City_Dirty_Flag_Religions = 0x10u;
constexpr auto City_Dirty_Flag_Owner = 0x20u;
constexpr auto City_Dirty_Flag_Defense = 0x40u;
constexpr auto City_Dirty_Flag_Type = 0x80u;

class City : public Town {
public:
	City();

	virtual void Initialize();
	virtual void GrowPopulationOneMonth();
	void Restore();
	std::string GetCityAttributeAsString() const;
	void GetUnownedAttributes( std::vector<CityAttribute>& out_attributes ) const;
	void AddAttribute( CityAttribute attr );
	bool HasAttribute( CityAttribute attr ) const;
	void RemoveAttribute( CityAttribute attr );
	void SyncAttributes();
	uint16_t GetRawAttribute() const;
	void SetRawAttribute( uint16_t attr );
	std::vector<CityAttribute> m_attributes;
	float m_height = 0.f;
protected:
	uint16_t m_type = 0;
};