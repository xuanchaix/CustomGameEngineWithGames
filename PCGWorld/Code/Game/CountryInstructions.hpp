#pragma once
#include "Game/GameCommon.hpp"
class MapPolygonUnit;
class Army;

enum class InstructionType {
	MoveArmy, Develop, BuildArmy, RecruitArmy, BuildRelationship, Convert, Assimilate, Legalize, Num, None
};
enum class CrisisType {
	CultureConflict, ReligionConflict, CivilWar, None,
};

class CountryInstruction {
public:
	CountryInstruction() {};
	virtual~CountryInstruction() {};
	InstructionType m_type = InstructionType::None;
};

class InstructionMoveArmy : public CountryInstruction {
public:
	InstructionMoveArmy() { m_type = InstructionType::MoveArmy; };
	virtual ~InstructionMoveArmy() {};
	MapPolygonUnit* m_fromProv = nullptr;
	MapPolygonUnit* m_toProv = nullptr;
	Army* m_army = nullptr;
};

class InstructionDevelop : public CountryInstruction {
public:
	InstructionDevelop() { m_type = InstructionType::Develop; };
	virtual ~InstructionDevelop() {};
	MapPolygonUnit* m_prov = nullptr;
};

class InstructionBuildArmy : public CountryInstruction {
public:
	InstructionBuildArmy() { m_type = InstructionType::BuildArmy; };
	virtual ~InstructionBuildArmy() {};
	MapPolygonUnit* m_prov = nullptr;
	int m_numOfSoldiersRecruited = 0;
};

class InstructionRecruitArmy : public CountryInstruction {
public:
	InstructionRecruitArmy() { m_type = InstructionType::RecruitArmy; };
	virtual ~InstructionRecruitArmy() {};
	Army* m_army = nullptr;
	int m_numOfSoldiersRecruited = 0;
};

class InstructionRelationship : public CountryInstruction {
public:
	InstructionRelationship() { m_type = InstructionType::BuildRelationship; };
	virtual ~InstructionRelationship() {};
	CountryRelationType m_relationWant = CountryRelationType::None;
	Country* countryFrom = nullptr;
	Country* countryTo = nullptr;
};

class InstructionConvert : public CountryInstruction {
public:
	InstructionConvert() { m_type = InstructionType::Convert; };
	virtual ~InstructionConvert() {};
	Province* m_province = nullptr;
	Religion* m_religion = nullptr;
};

class InstructionAssimilate : public CountryInstruction {
public:
	InstructionAssimilate() { m_type = InstructionType::Assimilate; };
	virtual ~InstructionAssimilate() {};
	Province* m_province = nullptr;
	Culture* m_culture = nullptr;
};

class InstructionLegalize : public CountryInstruction {
public:
	InstructionLegalize() { m_type = InstructionType::Legalize; };
	virtual ~InstructionLegalize() {};
	Province* m_province = nullptr;
	Country* m_country = nullptr;
};

class HistoryCrisis {
public:
	CrisisType m_type = CrisisType::None;
	float m_progress = 0.f;
	Country* m_country = nullptr;
	void* m_cultureOrReligion = nullptr;
	unsigned int m_globalID = (unsigned int)-1;
	bool m_isActive = true;
};