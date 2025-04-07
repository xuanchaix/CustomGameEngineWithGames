#pragma once
#include "Game/Building.hpp"

constexpr float PowerPlantOutPut = 10000.f;
constexpr float PowerPlantEfficiency = 2.f;
constexpr int PowerPlantMaxCapacity = 10;
constexpr float PowerProviderRange = 6.f;

class IPowerBuilding;
class PowerBuilding;

class PowerNetwork {
public:
	void Update();
	bool SeparateNetwork( std::vector<PowerNetwork*>& out_networks );
	bool IsEmpty() const;
	void RemoveProvider( PowerBuilding* provider );
	void JoinNetwork( PowerNetwork* networkToJoin );
	float GetPowerPercentage() const;
	std::vector<PowerBuilding*> m_buildingInNetWork;

	float m_netWorkPower = 0.f;
	float m_powerGenerated = 0.f;
	float m_powerConsumed = 0.f;
private:
	void BFSBuildNetwork( PowerBuilding* startProvider, std::vector<PowerBuilding*>& out_network );
};

class IPowerBuilding {
public:
	float m_powerRange = PowerProviderRange;
	PowerNetwork* m_netWork = nullptr;
	void RemoveInRangeProvider( PowerBuilding* provider );
	std::vector<PowerBuilding*> m_otherPowerBuildingInRange;
	std::vector<Building*> m_otherConsumerBuildingInRange;

	bool m_powerInput = false;
	bool m_powerOutput = false;
	bool m_flagForCheck = false;
	float m_powerConsumption = 0.f;
	float m_powerGeneration = 0.f;
};

class PowerBuilding : public Building, public IPowerBuilding {
public:
	PowerBuilding( IntVec2 const& LBCoords );
	virtual ~PowerBuilding();
};


class PowerPlant : public PowerBuilding {
public:
	PowerPlant( IntVec2 const& LBCoords );
	virtual ~PowerPlant();
	virtual void Render() const;
	virtual bool IsFull();
	virtual void Produce();
	virtual bool AddResource( Resource* resource );
	virtual bool AddResource( int resource );
	bool CanResourceMoveInto( Resource* resource );

	int m_resourceCount = 0;
	Timer m_burnTimer;
};

class PowerNode : public PowerBuilding {
public:
	PowerNode( IntVec2 const& LBCoords );
	virtual ~PowerNode();
	virtual void Render() const;
	virtual Vec2 GetCenterPos() const;
};