#pragma once
#include "Game/Building.hpp"
#include "Game/PowerBuilding.hpp"

class TowerBase : public PowerBuilding {
public:
	TowerBase( IntVec2 const& LBCoords );
	virtual ~TowerBase();
	virtual void Render() const = 0;
	virtual void Shoot() = 0;
	virtual void Produce() = 0;
	virtual bool IsFull();
	virtual bool AddResource( Resource* resource );
	virtual bool AddResource( int resourceID );
	virtual bool CanAddResource( ProductDefinition const& resourceDef );
	virtual bool CanResourceMoveInto( Resource* resource );
	virtual bool DoHaveProjectileProducts() const;
	virtual int ConsumeProjectile();

	int m_numOfProjectileLeft = 0;
	int m_curProjectileType = -1;
	int m_numOfResource[NumOfProductTypes] = { };
	Direction m_dir = Direction::None;
	Timer m_shootTimer;
	Clock m_towerClock;
};

class Wall : public Building {
public:
	Wall( IntVec2 const& LBCoords );
	virtual ~Wall();
	virtual void Render() const;
	Direction m_dir = Direction::None;
};

class Mortar : public TowerBase {
public:
	Mortar( IntVec2 const& LBCoords );
	virtual ~Mortar();
	virtual void Render() const;
	virtual void Produce();
	virtual void Shoot();
	virtual Vec2 GetCenterPos() const override;
	virtual AABB2 GetPhysicsBounds() const override;
};

class GuidedMissile : public TowerBase {
public:
	GuidedMissile( IntVec2 const& LBCoords );
	virtual ~GuidedMissile();
	virtual void Render() const;
	virtual void Produce();
	virtual void Shoot();
	virtual Vec2 GetCenterPos() const override;
	virtual AABB2 GetPhysicsBounds() const override;

};

class StraightArcher : public TowerBase {
public:
	StraightArcher( IntVec2 const& LBCoords );
	virtual ~StraightArcher();
	virtual void Render() const;
	virtual void Produce();
	virtual void Shoot();

};

class ThreeDirectionsPike : public TowerBase {
public:
	ThreeDirectionsPike( IntVec2 const& LBCoords );
	virtual ~ThreeDirectionsPike();
	virtual void Render() const;
	virtual void Produce();
	virtual void Shoot();

};

class Laser : public TowerBase {
public:
	Laser( IntVec2 const& LBCoords );
	virtual ~Laser();
	virtual void Render() const;
	virtual void Produce();
	virtual void Shoot();

};