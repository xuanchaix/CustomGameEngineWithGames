#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"

struct Tile;
class Resource;

class Building : public Entity {
public:
	Building( IntVec2 const& LBCoords );
	virtual ~Building();
	virtual void Update( float deltaTime );
	virtual void Die();
	virtual void Render() const = 0;
	virtual void Produce();
	virtual bool IsFull();
	virtual Vec2 GetCenterPos() const;
	virtual bool AddResource( Resource* resource );
	virtual bool AddResource( int resourceID );
	virtual AABB2 GetPhysicsBounds() const;
	virtual void BuildingAddVertsForHealthBar(std::vector<Vertex_PCU>& verts) const;

	bool m_neverFull = false;
	bool m_hasRendered = false;
	bool m_canProduce = false;
	bool m_hasHealthBar = true;
	IntVec2 m_leftBottomCoords;
	Tile* m_tileLBOn = nullptr;
	BuildingType m_buildingType = BuildingType::Building;
};

class Base : public Building {
public:
	Base( IntVec2 const& LBCoords );
	virtual ~Base();
	virtual void Render() const;
	virtual bool AddResource( Resource* resource );
	virtual bool AddResource( int resourceID );
	virtual void Produce();
	virtual Vec2 GetCenterPos() const;
	virtual AABB2 GetPhysicsBounds() const;
protected:
	void CheckBuildingToExport( Building* building );
};

class Drill : public Building {
public:
	Drill( IntVec2 const& LBCoords );
	virtual ~Drill();

	virtual void Produce();
	virtual void Render() const;
	virtual bool IsFull();
	virtual bool AddResource( Resource* resource );
	virtual bool AddResource( int resourceID );
	bool CanResourceMoveInto( Resource* resource );

	int m_numInInventory = 0;
	int m_drillingType = -1;
	float m_drillingSpeed = 0.5f;
	Direction m_dir;
	Timer m_productionTimer;
};

class WareHouse : public Building {
public:
	WareHouse( IntVec2 const& LBCoords );
	virtual ~WareHouse();
	virtual void Render() const;
	virtual void Produce();
	virtual bool AddResource( Resource* resource );
	virtual bool AddResource( int resourceID );

	int m_numOfResource[NumOfProductTypes] = {};
protected:
	void CheckBuildingToExport( Building* building );
};

class ResearchCenter : public Building {

};