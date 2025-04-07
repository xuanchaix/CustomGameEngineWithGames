#pragma once
#include "Game/GameCommon.hpp"

class Province;

enum class BattleType
{
	SIEGE, COMBAT
};

struct BattleRound {
	int m_selfSize;
	int m_enemySize;
	int m_diceResult;
	int m_selfCasualties;
	int m_enemyCasualties;
};

class BattleReport {
friend class Game;
friend class Army;
friend class StupidMonarchAI;
public:
	BattleReport();
	~BattleReport();

	void AddBattleReport( int selfSize, int enemySize, int diceResult, int selfCasualties, int enemyCasualties );
	Vec2 const GetPosition() const;
	void Render() const;
private:
	// combat
	std::vector<BattleRound> m_battleRounds;
	bool m_selfIsAttacker;
	Province* m_attackerFrom;
	int m_attackerForceId;
	int m_defenderForceId;
	int m_selfRemainSize;
	int m_enemyRemainSize;
	bool m_battleResult;
	Vec2 m_battlePosition;
	BattleType m_type;
	// siege
	int m_provID = -1;
	int m_diceRollRes;
	int m_beginSize;
	int m_endSize;
	int m_beginDefense;
	int m_endDefense;
	bool m_isSuccess = false;
};