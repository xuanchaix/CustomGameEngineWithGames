#pragma once
#include "Game/GameCommon.hpp"

class Force;
class Province;
class Army;

struct AIProvinceDescription {
	bool m_isBoarder = false;
	float m_value = 0.f;
	Province* m_prov = nullptr;
};

enum class AIOperationType {
	DEF,
	DEV,
	ATR,
	RCT5,
	COUNT,
};

struct AIOperation {
	AIOperationType m_type = AIOperationType::COUNT;
	Province* m_provToOP = nullptr;
	Army* m_armyToOP = nullptr;
	float m_possibility = 0.f;
};

enum class AIPersonality {
	AGGRESSIVE,
	MEAN,
	DEFENSIVE,
	BESIEGER,
	DEVELOPER,
	COUNT,
};

class StupidMonarchAI {
public:
	StupidMonarchAI( float difficulty, Force* controlledForce );
	void ConductAI();

	AIPersonality m_personality = AIPersonality::COUNT;
	float m_difficulty = 1.f;
	Force* m_controlledForce = nullptr;
	Force* m_targetEnemy;

private:
	void UpdateProvDesc();
	void BuildNewArmy();
	std::vector<AIProvinceDescription> m_provDesc;
};