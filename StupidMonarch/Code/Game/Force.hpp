#pragma once
#include <string>
#include <vector>
#include "Game/GameCommon.hpp"

class Map;
class Province;
class Army;
class StupidMonarchAI;

// most of the independent powers are not country so I call them force

class Force {
	friend class Game;
	friend class StupidMonarchAI;
	friend class Army;
public:
	Force( Map* map, int id, Rgba8 const& color, std::string const& nickName, Province* capital );
	~Force();

	void StartUp( bool loadFromSave );
	void Update();
	void Render();

	void NextTurn();

	void GainProvince( Province* provToGain );
	void LoseProvince( Province* provToLose );
	void RemoveArmy( Army* armyToRemove );
	void BuildAnNewArmy( Province* provinceToBuild );

	int CalculateCommandPoint() const;
	float GetRatioOfArmyCommandPointCost() const;

	bool isProvOwned( Province* provToCheck ) const;
	bool isProvOwnedAndValid( Province* provToCheck ) const;
	bool isCalledNickName( std::string const& compareName ) const;
	bool isAI() const;
	bool isAlive() const;
	bool isAdjancentToForce( Force* force ) const;

	void SetAsPlayer(bool isPlayer);

	float GetTotalPopulation() const;
	float GetCorrectedEconomyPoint() const;
	Rgba8 const& GetForceColor() const;
	std::string const& GetNickName() const;
	int GetCommandPoint() const;
	int GetMaxCommandPoint() const;
	int GetArmyAmount() const;
	int GetMaxArmyAmount() const;
	Province* GetCapitalProv() const;
	Army* GetArmyAtWorldPos( Vec2 const& worldPos ) const;
	std::vector<Province*> GetOwnedProvs() const;
	int GetHuhuaness() const;
	int GetTotalArmySize() const;
	int GetArmyId( Army* army ) const;
	void GetAllAdjancentForces(std::vector<Force*>& adjForces) const;

private:
	AABB2 GetBounds() const;

public:
	Map* m_map;
	int m_id = -1;
	Rgba8 m_color;
	std::string m_nickName;
	std::string m_willingPersonality = "DEFAULT";
	std::vector<Province*> m_ownedProvs;
	std::vector<Army*> m_armies;
	Province* m_capitalProv = nullptr;

	int m_maxArmyAmount = 0;
	int m_commandPointAmount = 0;

	bool m_isPlayer = false;
	StupidMonarchAI* m_ai = nullptr;
};