#pragma once
#include "Game/Building.hpp"
#include "Game/PowerBuilding.hpp"
#include "Game/Map.hpp"

class ProductDefinition;

constexpr int FactoryMaxResource = 20;
constexpr float BlenderProductionTime = 3.f;
constexpr float RefineryProductionTime = 3.f;
constexpr float FactoryEnergyConsumption = 300.f;

class Factory: public PowerBuilding {
public:
	Factory( IntVec2 const& LBCoords );
	virtual ~Factory();
	virtual void Render() const = 0;
	virtual void Produce();
	virtual Vec2 GetCenterPos() const;
	virtual AABB2 GetPhysicsBounds() const;
	virtual bool AddResource( Resource* resource );
	virtual bool AddResource( int resourceID );
	virtual bool IsFull();
	bool IsCoordsInBuilding( IntVec2 const& coords ) const;
	virtual bool CanResourceMoveInto( Resource* resource );

	bool ChooseProduct( EventArgs& args );
public:
	int m_numOfResource[NumOfProductTypes] = {};
	Direction m_dir = Direction::None;
	int m_numOfResourceInInventory = 0;
	float m_animationTime = 0.f;
protected:
	virtual bool TryGenerateResource( Vec2 const& pos, Resource* resourceToGive );
	int GetNumOfResources() const;
	bool CanAddResource( int resourceID ) const;
	int GetResourceNumNeededForSingleProduct( int resourceID ) const;
	bool DoProductionConsumeResource();

	bool GenerateResourceLeftTop( int resourceID ) {
		return GetCurMap()->CreateNewResource( resourceID, Vec2( m_leftBottomCoords ) + Vec2( -0.001f, 1.5f ), this );
	}
	bool GenerateResourceLeftBottom( int resourceID ) {
		return GetCurMap()->CreateNewResource( resourceID, Vec2( m_leftBottomCoords ) + Vec2( -0.001f, 0.5f ), this );
	}
	bool GenerateResourceRightTop( int resourceID ) {
		return GetCurMap()->CreateNewResource( resourceID, Vec2( m_leftBottomCoords ) + Vec2( 2.001f, 1.5f ), this );
	}
	bool GenerateResourceRightBottom( int resourceID ) {
		return GetCurMap()->CreateNewResource( resourceID, Vec2( m_leftBottomCoords ) + Vec2( 2.001f, 0.5f ), this );
	}
	bool GenerateResourceTopRight( int resourceID ) {
		return GetCurMap()->CreateNewResource( resourceID, Vec2( m_leftBottomCoords ) + Vec2( 1.5f, 2.001f ), this );
	}
	bool GenerateResourceTopLeft( int resourceID ) {
		return GetCurMap()->CreateNewResource( resourceID, Vec2( m_leftBottomCoords ) + Vec2( 0.5f, 2.001f ), this );
	}
	bool GenerateResourceBottomRight( int resourceID ) {
		return GetCurMap()->CreateNewResource( resourceID, Vec2( m_leftBottomCoords ) + Vec2( 1.5f, -0.001f ), this );
	}
	bool GenerateResourceBottomLeft( int resourceID ) {
		return GetCurMap()->CreateNewResource( resourceID, Vec2( m_leftBottomCoords ) + Vec2( 0.5f, -0.001f ), this );
	}
	bool(Factory::* m_generationFuncList[8])(int) = {
		&Factory::GenerateResourceTopLeft, &Factory::GenerateResourceTopRight, &Factory::GenerateResourceRightTop, &Factory::GenerateResourceRightBottom,
		&Factory::GenerateResourceBottomRight, &Factory::GenerateResourceBottomLeft, &Factory::GenerateResourceLeftBottom, &Factory::GenerateResourceLeftTop,
	};
	int m_curTryPos = 0;
	ProductDefinition const* m_productionType = nullptr;
	Timer m_productionTimer;
	Clock m_factoryClock;
};


class Blender : public Factory {
public:
	Blender( IntVec2 const& LBCoords );
	virtual ~Blender();
	virtual void Render() const;
	SpriteAnimDefinition m_animationSprite;
	SpriteSheet m_sprite;
};

class Refinery : public Factory {
public:
	Refinery( IntVec2 const& LBCoords );
	virtual ~Refinery();
	virtual void Render() const;

public:
protected:
};