#pragma once
#include "Game/GameCommon.hpp"

class Army;
class Country;
typedef MapPolygonUnit Province;

struct ArmyBattleWrapper {
	Country* m_country = nullptr;
	std::vector<Army*> m_army;
	int m_totalSize;
	float m_distance;

};

enum class BattleResult {
	Attacker_Win_Defender_Retreat,
	Attacker_Win_Defender_Eliminated,
	Defender_Win_Attacker_Retreat,
	Defender_Win_Attacker_Eliminated,
};

enum class SiegeResult {
	Defender_Surrender,
	Defender_Keep_Defending,
	Defender_Lose_Morale,
	Attacker_Lose_Morale,
};

void RemoveArmyFromMap( Army* army, Map* map );
BattleResult ResolveBattleArmyFromMap( ArmyBattleWrapper* defender, ArmyBattleWrapper* attacker, Map* map );
SiegeResult ResolveSiegeFromMap( Army* army, Province* prov, Map* map );
void RetreatFromProvince( Army* army, Map* map );