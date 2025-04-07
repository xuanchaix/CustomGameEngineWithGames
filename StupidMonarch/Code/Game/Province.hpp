#pragma once
#include <vector>
#include "Game/GameCommon.hpp"
class Map;
class Force;
class Army;

class Province {
friend class Game;
friend class Map;
friend class Force;
friend class StupidMonarchAI;
public:
	Province( Map* inputMap, int id );
	~Province();

	void StartUp( bool isMajor, bool isPlain, bool isMountain, int maxDef, float huhuaness, std::string const& name, std::string const& capitalName );
	void SetHistory( int economy, int population, Force* owner, Force* legalForce );
	void Update();

	void NextTurn();

	std::vector<Province*> const& GetAdjacentProvinces() const;
	void AddAdjacentProvince( Province* prov );
	void AddLegalForce( Force* legalForce );
	void RemoveLegalForce( Force* legalForce );
	void AddArmyOnto( Army* army );
	void RemoveArmyOn();
	void SetDefense( float newDef );
	void SetOwner( Force* newOwner );
	void SubstractPopulation( float populationToSubstract );
	void SubstractEconomy( int amount );
	float CalculateMaxLegalProgress() const;
	void AddLegalProgress( float progressToAdd );
	void SpreadPopulation( float totalPopulation );

	bool IsSiegedByEnemy() const;
	bool IsMajorProvince() const;
	bool IsMountain() const;
	bool IsLegal() const;
	bool IsAdjacent( Province* prov ) const;
	bool IsBoarder( Force* otherForce ) const;
	bool IsBoarder() const;

	Force* GetOwner() const;
	int GetId() const;
	Vec2 const& GetCenter();
	float GetPopulation() const;
	int GetEconomy() const;
	float GetEconomyCorrectedPoint() const;
	std::vector<Province*> GetAdjacentProvs() const;
	Army* GetArmyOn() const;
	float GetDefense() const;
	std::string const& GetName() const;
	float GetLegalProgress() const;

	void Develop();
	void Defense();
	void Attract();

private:
	void CalculateMaxDevRate();
public:
	std::string m_name = "Default";
	std::string m_capitalName = "Default";
	int m_id = -1;
	int m_economy = 1;
	float m_population = 1000;
	Force* m_owner = nullptr;
	Army* m_armyOn = nullptr;
	bool m_isMajorProv = false;
	bool m_isPlain = false;
	bool m_isMountain = false;
	Vec2 m_centerPos = Vec2( -1.f, -1.f );
	std::vector<Province*> m_adjacentProvinces;
	std::vector<Force*> m_legalForces;
	IntVec2 m_sumOfPixelPos;
	int m_numOfPixels;
	Map* m_map = nullptr;
	Rgba8 m_color;

	float m_defenseRate = 1000.f;
	float m_maxDefenseRate = 1000.f;
	int m_developmentRate = 0;
	int m_maxDevelopmentRate = 0;
	float m_populationGrowthRate = 0.001f;
	bool m_isAttractingPopulation = false;
	float m_huhuaness = 0.f;

	float m_legalProgress;
	float m_maxLegalProgress;
	bool m_legalIsAddedThisTurn = false;

	int m_startOfVertexBuffer;
	int m_numOfVerts;

	Vec2 m_leftBottomPos;
	Vec2 m_rightTopPos;
};