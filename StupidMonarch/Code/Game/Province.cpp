#include "Game/Province.hpp"
#include "Game/Force.hpp"
#include "Game/Army.hpp"

Province::Province( Map* inputMap, int id )
	:m_map(inputMap), m_id(id)
{

}

Province::~Province()
{

}

void Province::StartUp( bool isMajor, bool isPlain, bool isMountain, int maxDef, float huhuaness, std::string const& name, std::string const& capitalName )
{
	m_isMajorProv = isMajor;
	m_maxDefenseRate = (float)maxDef;
	m_defenseRate = m_maxDefenseRate / 2;
	m_huhuaness = huhuaness;
	m_name = name;
	m_capitalName = capitalName;
	m_isPlain = isPlain;
	m_isMountain = isMountain;
	CalculateMaxDevRate();
	if (!IsLegal()) {
		m_legalProgress = 0.f;
		m_maxLegalProgress = CalculateMaxLegalProgress();
	}
}

void Province::SetHistory( int economy, int population, Force* owner, Force* legalForce )
{
	m_economy = economy;
	m_population = (float)population;
	m_owner = owner;
	if (legalForce) {
		AddLegalForce( legalForce );
		//m_legalForces.push_back( legalForce );
	}
}

void Province::Update()
{

}

void Province::NextTurn()
{
	if (m_developmentRate >= m_maxDevelopmentRate) {
		m_developmentRate -= m_maxDevelopmentRate;
		m_economy++;
		m_huhuaness -= 0.01f;
	}
	if (m_isAttractingPopulation) {
		m_isAttractingPopulation = false;
		m_huhuaness += 0.01f;
	}
	if (!IsLegal()) {
		m_legalProgress += 5.f;
		if (m_legalProgress >= m_maxLegalProgress) {
			AddLegalForce( m_owner );
		}
	}
	m_legalIsAddedThisTurn = false;
	m_population = m_population * (1.f + m_populationGrowthRate);
	m_population = GetClamped( m_population, 0.f, 3000000.f );
	m_huhuaness -= 0.001f;
	m_huhuaness = m_huhuaness < 0.f ? 0.f : m_huhuaness;
	m_huhuaness = m_huhuaness > 1.f ? 1.f : m_huhuaness;
	if (!IsSiegedByEnemy()) {
		m_defenseRate += 20.f;
	}
	if (m_defenseRate > m_maxDefenseRate) {
		m_defenseRate = m_maxDefenseRate;
	}
	if (m_population >= 1000000.f || (m_population >= 500000.f && m_isMountain)) {
		m_populationGrowthRate = 0.f;
	}
	else {
		m_populationGrowthRate = 0.001f;
	}
	CalculateMaxDevRate();
}

std::vector<Province*> const& Province::GetAdjacentProvinces() const
{
	return m_adjacentProvinces;
}

void Province::AddAdjacentProvince( Province* prov )
{
	bool isInVector = false;
	for (int k = 0; k < (int)m_adjacentProvinces.size(); k++) {
		if (m_adjacentProvinces[k] == prov) {
			isInVector = true;
			break;
		}
	}
	if (!isInVector) {
		m_adjacentProvinces.push_back( prov );
	}
}

void Province::AddLegalForce( Force* legalForce )
{
	for (int i = 0; i < (int)m_legalForces.size(); i++) {
		if (m_legalForces[i] == legalForce) {
			return;
		}
	}
	m_legalForces.push_back( legalForce );
}

void Province::RemoveLegalForce( Force* legalForce )
{
	for (int i = 0; i < (int)m_legalForces.size(); i++) {
		if (m_legalForces[i] == legalForce) {
			m_legalForces.erase( m_legalForces.begin() + i );
			return;
		}
	}
}

void Province::AddArmyOnto( Army* army )
{
	m_armyOn = army;
}

void Province::RemoveArmyOn()
{
	m_armyOn = nullptr;
}

void Province::SetDefense( float newDef )
{
	m_defenseRate = newDef;
}

void Province::SetOwner( Force* newOwner )
{
	m_owner = newOwner;
	if (!IsLegal()) {
		m_legalProgress = 0.f;
		m_maxLegalProgress = CalculateMaxLegalProgress();
	}
}

void Province::SubstractPopulation( float populationToSubstract )
{
	m_population -= populationToSubstract;
}

void Province::SubstractEconomy( int amount )
{
	if (m_economy > amount) {
		m_economy -= amount;
	}
}

float Province::CalculateMaxLegalProgress() const
{
	return 5.f + GetEconomyCorrectedPoint() / 10000.f;
}

void Province::AddLegalProgress( float progressToAdd )
{
	m_legalProgress += progressToAdd;
	m_legalIsAddedThisTurn = true;
	if (m_legalProgress >= m_maxLegalProgress) {
		AddLegalForce( m_owner );
	}
}

void Province::SpreadPopulation( float totalPopulation )
{
	float popForEach = totalPopulation / (float)m_adjacentProvinces.size();
	for (auto prov : m_adjacentProvinces) {
		prov->SubstractPopulation( -popForEach );
	}
}

bool Province::IsSiegedByEnemy() const
{
	if (m_armyOn) {
		return m_armyOn->GetOwner() != m_owner;
	}
	return false;
}

bool Province::IsMajorProvince() const
{
	return m_isMajorProv;
}

bool Province::IsMountain() const
{
	return m_isMountain;
}

bool Province::IsLegal() const
{
	for (auto force : m_legalForces) {
		if (m_owner == force) {
			return true;
		}
	}
	return false;
}

bool Province::IsAdjacent( Province* prov ) const
{
	for (auto adjprov : m_adjacentProvinces) {
		if (adjprov == prov) {
			return true;
		}
	}
	return false;
}

bool Province::IsBoarder( Force* otherForce ) const
{
	for (auto prov : m_adjacentProvinces) {
		if (prov->m_owner == otherForce) {
			return true;
		}
	}
	return false;
}

bool Province::IsBoarder() const
{
	for (auto prov : m_adjacentProvinces) {
		if (prov->m_owner != m_owner) {
			return true;
		}
	}
	return false;
}

Force* Province::GetOwner() const
{
	return m_owner;
}

int Province::GetId() const
{
	return m_id;
}

Vec2 const& Province::GetCenter()
{
	if (m_centerPos == Vec2( -1.f, -1.f )) {
		m_centerPos = Vec2( (float)m_sumOfPixelPos.x / m_numOfPixels, (float)m_sumOfPixelPos.y / m_numOfPixels );
	}
	return m_centerPos;
}

float Province::GetPopulation() const
{
	return m_population;
}

int Province::GetEconomy() const
{
	return m_economy;
}

float Province::GetEconomyCorrectedPoint() const
{
	return (float)m_economy * m_population * (1.f - m_huhuaness * 0.4f);
}

std::vector<Province*> Province::GetAdjacentProvs() const
{
	return m_adjacentProvinces;
}

Army* Province::GetArmyOn() const
{
	return m_armyOn;
}

float Province::GetDefense() const
{
	return m_defenseRate;
}

std::string const& Province::GetName() const
{
	return m_name;
}

float Province::GetLegalProgress() const
{
	if (IsLegal()) {
		return 1.f;
	}
	else {
		return m_legalProgress / m_maxLegalProgress;
	}
}

void Province::Develop()
{
	m_developmentRate += 1;
}

void Province::Defense()
{
	m_defenseRate = m_defenseRate + 500.f > m_maxDefenseRate ? m_maxDefenseRate : m_defenseRate + 500.f;
}

void Province::Attract()
{
	m_isAttractingPopulation = true;
	m_populationGrowthRate += 0.01f;
}

void Province::CalculateMaxDevRate()
{
	m_maxDevelopmentRate = (m_population / 10000.f) > 2 ? int(m_population / 10000) : 2;
	if (m_isMajorProv) {
		m_maxDevelopmentRate = m_maxDevelopmentRate - 1 < 2 ? 2 : m_maxDevelopmentRate - 1;
	}
	if (m_isPlain) {
		m_maxDevelopmentRate = (int)(m_maxDevelopmentRate * 0.8f) < 2 ? 2 : (int)(m_maxDevelopmentRate * 0.8f);
	}
	if (m_isMountain) {
		m_maxDevelopmentRate += 1;
	}
}

