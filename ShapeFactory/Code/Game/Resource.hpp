#pragma once
#include "Game/GameCommon.hpp"
class Building;
class Conveyer;

struct ProductRecipe {
	int m_productCount = 1;
	float m_productionTime = 1.f;
	std::vector<std::pair<int, int>> m_idCountPair;
};

class ProductDefinition {
public:
	std::string m_name;
	std::string m_shortType;
	bool m_isRaw = true;
	bool m_isProjectile = false;
	BuildingType m_factory = BuildingType::None;
	int m_id = -1;
	int m_normalProjectileID = 0;
	int m_guidedProjectileID = 0;
	int m_shellProjectileID = 0;
	Texture* m_texture = nullptr;

	ProductRecipe m_recipe;

	ProductDefinition();
	ProductDefinition( XmlElement* xmlIter );
	static void SetUpProductDefinitions();
	int GetNumOfBasicResource( unsigned char basicResourceShortType ) const;
	static std::vector<ProductDefinition> s_definitions;
	static ProductDefinition const& GetDefinition( std::string const& nameOrShortType, bool isShortType = true );
	static ProductDefinition const& GetDefinition( int id );
	static ProductDefinition const& GetNullDef();
};

class Resource {

public:
	Resource( ProductDefinition const& def );
	virtual void Update();
	virtual void Render() const;

	virtual void PreMove( Conveyer* conveyerOn, float deltaSeconds );
	virtual void ReconsiderMovement( Resource* ignoreResource = nullptr );
	virtual void Move();

	virtual bool IsResourceType( std::string const& typeShort ) const;
	virtual bool GetNumOfBasicResource( unsigned char basicResourceType ) const;

	ProductDefinition const& m_def;
	Conveyer* m_conveyerOn = nullptr;
	Vec2 m_centerPos;

	Vec2 m_movementTargetPos;
	bool m_disabledMovement = false;
	bool m_isGarbage = false;
	bool m_isCollected = false;
	bool m_render = true;
	bool m_hasPriority = false;
};