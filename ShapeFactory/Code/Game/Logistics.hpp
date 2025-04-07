#pragma once
#include "Game/Building.hpp"

class Resource;
constexpr int LogisticBuildingMaxCapacity = 3;

struct ResourceInBuilding {
	Resource* m_resource;
	float m_enterTime;
};

class LogisticBuilding : public Building {
public:
	LogisticBuilding( IntVec2 const& LBCoords );
	virtual ~LogisticBuilding();
	virtual void Render() const = 0;
	virtual void Produce() = 0;
	virtual bool AddResource( Resource* resource );
	virtual bool AddResource( int resourceID );
	virtual bool IsFull();
public:
	std::deque<ResourceInBuilding> m_resources;
	Direction m_dir = Direction::None;
protected:
	virtual bool TryGenerateResource( Direction dir, Resource* resourceToGive );
	float m_conveyTime = 1.f / CONVEY_SPEED;
};

class Selector : public LogisticBuilding {
public:
	Selector( IntVec2 const& LBCoords );
	virtual ~Selector();
	virtual void Render() const;
	virtual void Produce();

	bool CanResourceMoveInto( Resource* resource );

	bool ChooseFilter( EventArgs& args );

public:
	ProcessDirection m_priorityDir = ProcessDirection::Forward;
	int m_selectingTypeID = 0;
protected:
	bool m_goLeft = true;
};

class Router : public LogisticBuilding {
public:
	Router( IntVec2 const& LBCoords );
	virtual ~Router();
	virtual void Render() const;
	virtual void Produce();

public:
protected:
	ProcessDirection m_nextDir = ProcessDirection::Forward;
};


// if the front is full, then choose right or left
class OverflowGate : public LogisticBuilding {
public:
	OverflowGate( IntVec2 const& LBCoords );
	virtual ~OverflowGate();
	virtual void Render() const;
	virtual void Produce();

	bool CanResourceMoveInto(Resource* resource);
public:
protected:
	ProcessDirection m_nextDir = ProcessDirection::Left;
};

class Exporter : public LogisticBuilding {
public:
	Exporter( IntVec2 const& LBCoords );
	virtual ~Exporter();
	virtual void Render() const;
	virtual void Produce();

	bool ChooseFilter( EventArgs& args );
public:
	int m_typeToExportID = 0;
protected:
	ProcessDirection m_nextDir = ProcessDirection::Forward;
};

class Junction : public LogisticBuilding {
public:
	Junction( IntVec2 const& LBCoords );
	virtual ~Junction();
	virtual void Render() const;
	virtual void Produce();
	virtual bool AddResource( Resource* resource );
	virtual bool AddResource( int resourceID );
	virtual bool IsFull();
	bool CanResourceMoveInto( Resource* resource );
	std::deque<ResourceInBuilding> m_resourcesForwardBack;
	std::deque<ResourceInBuilding> m_resourcesLeftRight;
protected:
	void ProduceDirection( std::deque<ResourceInBuilding>& array, bool isForwardBack );
	virtual bool TryGenerateResource( Direction dir, Resource* resourceToGive, std::deque<ResourceInBuilding>& array );
	
};


class Bridge : public LogisticBuilding {
public:
	Bridge( IntVec2 const& LBCoords );
	virtual ~Bridge();
	virtual void Render() const;
	virtual void Produce();
	virtual bool AddResource( Resource* resource );
	virtual bool AddResource( int resourceID );
	bool CanResourceMoveInto( Resource* resource );
	bool m_isInput = false;
	int m_bridgeLength = 0;
	Bridge* m_theOtherHead = nullptr;
	virtual bool IsFull();
protected:
	virtual bool TryGenerateResource( Direction dir, Resource* resourceToGive );
};