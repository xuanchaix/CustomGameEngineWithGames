#pragma once

#include "Game/Model.hpp"

class Tile;

enum class UnitType {
	Tank, Artillery, Num, None,
};

struct UnitDefinition {
	UnitDefinition( XmlElement* elem );
	char m_symbol = 'b';
	std::string m_name = "Bison";
	std::string m_imageFileName = "Data/Images/Tanks/Bison.png";
	Texture* m_imageFile = nullptr;
	std::string m_modelFileName = "Data/Models/Bison/Bison.xml";
	UnitType m_type = UnitType::Tank;
	int m_groundAttackDamage = 50;
	int m_groundAttackRangeMin = 1;
	int m_groundAttackRangeMax = 1;
	int m_movementRange = 6;
	int m_defense = 40;
	int m_health = 8;

	static UnitDefinition const& GetDefinition( std::string const& defName );
	static UnitDefinition const& GetDefinition( char symbol );

	static std::vector<UnitDefinition> s_definitions;
};

class Unit : public Model {
public:
	Unit( Game* game, UnitDefinition const& def );
	virtual ~Unit();

	virtual void Update() override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void DebugRender() const override;
	void CalculateLegitimateMovingDest();

	void MoveTo( Tile* tile, bool skipAnimation = false );
	bool IsUnitInAttackRange( Unit* unit ) const;
	void Fire( Unit* target );
	void ResolveDamage( int damage, Unit* source );
	void GetAllEnemyUnitsInRange( std::vector<Unit*>& out_units ) const;
	bool ReachTargetDir() const;
	Vec3 GetTurretPosition() const;

	UnitDefinition const& m_def;
	Tile* m_tileOn = nullptr;
	Tile* m_prevTile = nullptr;
	Tile* m_targetTile = nullptr;
	int m_faction = 0;
	int m_health = 0;
	UnitDirection m_dir = UnitDirection::Right;
	UnitDirection m_targetDir = UnitDirection::None;
	UnitDirection m_prevDir = UnitDirection::None;
	float m_customYawDegrees = 0.f;

	bool m_performedAction = false;
	bool m_endedAttackAnim = false;
	Unit* m_targetUnit = nullptr;
	std::vector<Tile*> m_legitimateMovingDest;
	std::vector<Tile*> m_currentRouteToTargetTile;
	static std::vector<float> s_unitDirYawMap;
	Timer* m_movingAnimTimer = nullptr;
	Timer* m_attackingAnimTimer = nullptr;
	CatmullRomSpline2D m_spline;
};