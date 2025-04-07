#pragma once
#include "Game/GameCommon.hpp"
class Country;
class MapPolygonUnit;

#define Army_Dirty_Flag_Size				0x01
#define Army_Dirty_Flag_Combat_Value		0x02
#define Army_Dirty_Flag_Prov_In				0x04
#define Army_Dirty_Flag_Existence			0x08
#define Army_Dirty_Flag_GlobalID			0x10

class Army {
public:
	Army( Country* owner, MapPolygonUnit* provIn, int size );
	~Army();

	Vec2 GetPosition() const;
	Vec3 GetPosition3D() const;
	float GetOuterRadius() const;
	float GetInnerRadius() const;
	void IntegrateArmy( Army* army );
	void GetAllProvincesCanGo( std::vector<MapPolygonUnit*>& out_provs );
	MapPolygonUnit* FindNextProvinceToGo(MapPolygonUnit* target) const;

	int m_size = 0;
	unsigned int m_globalID = 0;
	float m_combatValue = 0.5f;
	MapPolygonUnit* m_provIn = nullptr;
	Country* m_owner = nullptr;
	uint8_t m_dirtyBits = 0xff;
	bool m_isActive = true;

	MapPolygonUnit* m_goingTarget = nullptr;
};