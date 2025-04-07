#include "Game/AIUtils.hpp"
#include "Game/Game.hpp"
#include "Game/Force.hpp"
#include "Game/Province.hpp"
#include "Game/Army.hpp"
#include "Game/Map.hpp"
#include <algorithm>

StupidMonarchAI::StupidMonarchAI( float difficulty, Force* controlledForce )
	:m_difficulty(difficulty)
	,m_controlledForce(controlledForce)
{
	m_personality = AIPersonality::AGGRESSIVE;
	//m_personality = (AIPersonality)g_theGame->m_randNumGen->RollRandomIntLessThan( (int)AIPersonality::COUNT );
}

void StupidMonarchAI::ConductAI()
{
	RandomNumberGenerator* rng = g_theGame->m_randNumGen;

	// switch target enemy force
	std::vector<Force*> adjForces;
	m_controlledForce->GetAllAdjancentForces(adjForces);
	if (m_targetEnemy == nullptr || m_targetEnemy->m_ownedProvs.size() == 0 || find( adjForces.begin(), adjForces.end(), m_targetEnemy ) == adjForces.end()) {
		//if (!adjForces.empty()) {
		//	m_targetEnemy = adjForces[rng->RollRandomIntLessThan( (int)adjForces.size() )];
		//}
		if (m_controlledForce->GetTotalArmySize() > 125) {
			for (auto force : adjForces) {
				if (m_targetEnemy == nullptr || !m_targetEnemy->isAlive() || force->GetTotalArmySize() > m_targetEnemy->GetTotalArmySize()) {
					m_targetEnemy = force;
				}
			}
		}
		else {
			for (auto force : adjForces) {
				if (m_targetEnemy == nullptr || !m_targetEnemy->isAlive() || force->GetTotalArmySize() < m_targetEnemy->GetTotalArmySize()) {
					m_targetEnemy = force;
				}
			}
		}
	}

	// AI action
	// AI never valid; only developer develop

	// calculate owned province value
	UpdateProvDesc();

	// build new army: must be done for any kind of AI at first because it is free
	BuildNewArmy();

	// get all provinces which are under sieging
	bool isAttackedByOtherCountry = false;
	std::vector<Province*> provinceBeSieged;
	for (auto prov : m_controlledForce->m_ownedProvs) {
		if (prov->IsSiegedByEnemy()) {
			isAttackedByOtherCountry = true;
			provinceBeSieged.push_back( prov );
		}
	}
	std::sort( provinceBeSieged.begin(), provinceBeSieged.end(), 
		[]( Province const* a, Province const* b ) -> bool
		{return a->m_armyOn->GetSize() > b->m_armyOn->GetSize(); } );

	std::vector<AIOperation> thisTurnOps;
	// build operation possibility table
	for (auto& prov : m_provDesc) {
		if (!prov.m_prov->IsLegal() || prov.m_prov->IsSiegedByEnemy()) {
			continue;
		}
		AIOperation opDesc;
		opDesc.m_provToOP = prov.m_prov;
		opDesc.m_type = AIOperationType::ATR;
		opDesc.m_possibility = 0.01f;
		if (m_personality == AIPersonality::DEVELOPER) {
			opDesc.m_possibility += 0.2f;
		}
		if (prov.m_value >= 100.f) {
			opDesc.m_possibility += 0.1f;
		}
		if (prov.m_prov->m_economy >= 6) {
			opDesc.m_possibility += 0.1f;
		}
		else if (prov.m_prov->m_economy >= 8) {
			opDesc.m_possibility += 0.2f;
		}
		else if (prov.m_prov->m_economy >= 12) {
			opDesc.m_possibility += 0.3f;
		}
		thisTurnOps.push_back( opDesc );
		opDesc.m_type = AIOperationType::DEF;
		opDesc.m_possibility = 0.2f;
		if (prov.m_prov->m_defenseRate + 500 > prov.m_prov->m_maxDefenseRate) {
			opDesc.m_possibility -= 0.3f;
		}
		else if (prov.m_prov->m_defenseRate == prov.m_prov->m_maxDefenseRate) {
			opDesc.m_possibility -= 1.f;
		}
		if (prov.m_value > 25.f) {
			opDesc.m_possibility += 0.1f;
		}
		else if (prov.m_value > 60.f) {
			opDesc.m_possibility += 0.2f;
		}
		if (m_personality == AIPersonality::DEFENSIVE) {
			opDesc.m_possibility += 0.2f;
		}
		thisTurnOps.push_back( opDesc );
		opDesc.m_type = AIOperationType::DEV;
		opDesc.m_possibility = 0.01f;
		if (m_personality == AIPersonality::DEVELOPER) {
			opDesc.m_possibility = 0.1f;
			if (prov.m_prov->m_isMajorProv) {
				opDesc.m_possibility += 0.1f;
			}
			if (prov.m_prov->m_isPlain) {
				opDesc.m_possibility += 0.2f;
			}
			if (prov.m_prov->m_isMountain) {
				opDesc.m_possibility -= 0.2f;
			}
		}
		if (prov.m_prov->m_developmentRate > 0) {
			opDesc.m_possibility += 0.1f;
		}
		if (prov.m_prov->m_developmentRate > 2) {
			opDesc.m_possibility += 0.1f;
		}
		thisTurnOps.push_back( opDesc );
	}

	for (auto army : m_controlledForce->m_armies) {
		if (!m_controlledForce->isProvOwnedAndValid( army->m_inProvince )) {
			continue;
		}
		AIOperation opDesc;
		opDesc.m_armyToOP = army;
		opDesc.m_type = AIOperationType::RCT5;
		opDesc.m_possibility = 0.1f;
		if (isAttackedByOtherCountry) {
			opDesc.m_possibility += 0.6f;
		}
		if (m_personality == AIPersonality::AGGRESSIVE) {
			opDesc.m_possibility += 0.3f;
		}
		if (m_personality == AIPersonality::DEFENSIVE) {
			opDesc.m_possibility += 0.1f;
		}
		if (army->GetSize() < 10) {
			opDesc.m_possibility += 0.5f;
		}
		else if (army->GetSize() < 20) {
			opDesc.m_possibility += 0.4f;
		}
		else if (army->GetSize() < 50) {
			opDesc.m_possibility += 0.1f;
		}
		if (m_controlledForce->GetTotalArmySize() < 100) {
			opDesc.m_possibility += 0.2f;
		}
		if (m_controlledForce->GetRatioOfArmyCommandPointCost() > 0.5f && !isAttackedByOtherCountry) {
			opDesc.m_possibility -= 0.5f;
		}

		thisTurnOps.push_back( opDesc );
	}

	std::sort( thisTurnOps.begin(), thisTurnOps.end(), 
		[]( AIOperation const& a, AIOperation const& b ) -> bool 
		{return a.m_possibility > b.m_possibility; } );

	// AI loop to spend command points
	int loopTime = 0;
	while (m_controlledForce->m_commandPointAmount >= 5 && loopTime < 50) {
		for (auto& op : thisTurnOps) {
			float rnd = rng->RollRandomFloatZeroToOne();
			if (rnd <= op.m_possibility) {
				// try to do this
				switch (op.m_type)
				{
				case AIOperationType::ATR:
				{
					if (m_controlledForce->m_commandPointAmount >= 5) {
						op.m_provToOP->Attract();
						m_controlledForce->m_commandPointAmount -= 5;
					}
					break;
				}
				case AIOperationType::DEF:
				{
					if (m_controlledForce->m_commandPointAmount >= 5 && op.m_provToOP->m_defenseRate < op.m_provToOP->m_maxDefenseRate) {
						op.m_provToOP->Defense();
						m_controlledForce->m_commandPointAmount -= 5;
					}
					break;
				}
				case AIOperationType::DEV:
				{
					if (m_controlledForce->m_commandPointAmount >= 5) {
						op.m_provToOP->Develop();
						m_controlledForce->m_commandPointAmount -= 5;
					}
					break;
				}
				case AIOperationType::RCT5:
				{
					if (m_controlledForce->m_commandPointAmount >= 5 && op.m_armyToOP->GetSize() + 5 <= op.m_armyToOP->GetMaxSize()) {
						m_controlledForce->m_commandPointAmount -= 5;
						op.m_armyToOP->m_size += 5;
					}
					break;
				}
				default:
					break;
				}
			}
			if (m_controlledForce->m_commandPointAmount < 5) {
				break;
			}
		}
		loopTime++;
	}

	// recruit for the rest action point
	bool isAdded = true;
	while (m_controlledForce->m_commandPointAmount >= 1 && isAdded) {
		isAdded = false;
		for (auto army : m_controlledForce->m_armies) {
			if (m_controlledForce->m_commandPointAmount >= 1 && army->GetSize() < army->GetMaxSize() && m_controlledForce->isProvOwnedAndValid( army->m_inProvince )) {
				m_controlledForce->m_commandPointAmount -= 1;
				army->m_size += 1;
				isAdded = true;
			}
		}
	}

	// decide where army go to
	std::vector<Province*> provsAssigned;
	for (auto army : m_controlledForce->m_armies) {
		// do not move army in the first round
		if (g_theGame->m_roundCount == 1) {
			break;
		}
		if (army->m_inProvince->m_owner != m_controlledForce) {
			continue;
		}
		std::vector<Province*> provsToGo;
		m_controlledForce->m_map->GetAllProvsArmyCanGo( provsToGo, army );
		std::vector<Province*> provsToGoAndAdjToEnemy;
		for (auto prov : provsToGo) {
			if (prov->IsBoarder(m_targetEnemy)) {
				provsToGoAndAdjToEnemy.push_back( prov );
			}
		}
		Province* provToGo = nullptr;
		// get most valuable province to go to
		if ((int)provsToGo.size() > 0) {
			// solve the attacking enemy
			if (isAttackedByOtherCountry) {
				for (int i = (int)provinceBeSieged.size() - 1; i >= 0; i--) {
					if (!army->IsProvValidToGo( provinceBeSieged[i] )) {
						continue;
					}
					if (army->m_size >= provinceBeSieged[i]->m_armyOn->m_size + 10) {
						if (find( provsAssigned.begin(), provsAssigned.end(), provinceBeSieged[i] ) == provsAssigned.end()) {
							provToGo = provinceBeSieged[i];
							provinceBeSieged.pop_back();
							break;
						}
					}
				}
			}
			else {
				if (m_targetEnemy && m_personality != AIPersonality::DEFENSIVE) {
					for (auto prov : provsToGo) {
						if (prov->m_owner == m_targetEnemy && (provToGo == nullptr || prov->GetEconomy() > provToGo->GetEconomy())) {
							if (find( provsAssigned.begin(), provsAssigned.end(), prov ) == provsAssigned.end()) {
								provToGo = prov;
							}
						}
					}
				}
				if (m_personality == AIPersonality::AGGRESSIVE && provToGo == nullptr) {
					for (auto prov : provsToGo) {
						if (prov->m_owner != m_controlledForce && (provToGo == nullptr || prov->GetEconomy() > provToGo->GetEconomy())) {
							if (find( provsAssigned.begin(), provsAssigned.end(), prov ) == provsAssigned.end()) {
								provToGo = prov;
							}
						}
					}
				}
			}
			if (provToGo == nullptr && !provsToGoAndAdjToEnemy.empty() 
				&& !army->m_inProvince->IsBoarder( m_targetEnemy )) {
				provToGo = provsToGoAndAdjToEnemy[rng->RollRandomIntLessThan( (int)provsToGoAndAdjToEnemy.size() )];
				if (find( provsAssigned.begin(), provsAssigned.end(), provToGo ) != provsAssigned.end() || provToGo == army->m_inProvince ) {
					provToGo = nullptr;
				}
			}
			if (provToGo == nullptr) {
				if (/*army->m_size >= 20 && */army->m_inProvince->IsBoarder()) {
					std::vector<Province*> provCanAttack;
					for (auto adjProv : provsToGo) {
						if (adjProv->m_owner != m_controlledForce && find( provsAssigned.begin(), provsAssigned.end(), adjProv ) == provsAssigned.end()) {
							provCanAttack.push_back( adjProv );
						}
					}
					if (!provCanAttack.empty()) {
						provToGo = provCanAttack[rng->RollRandomIntLessThan( (int)provCanAttack.size() )];
					}
				}
				else {
					provToGo = provsToGo[rng->RollRandomIntLessThan( (int)provsToGo.size() )];
					if ((provToGo->m_owner == m_controlledForce && !provToGo->IsLegal())
						|| provToGo->m_owner != m_controlledForce
						|| find( provsAssigned.begin(), provsAssigned.end(), provToGo ) != provsAssigned.end()
						|| provToGo == army->m_inProvince) {
						provToGo = nullptr;
					}
				}
			}
		}

		if (army->IsProvValidToGo( provToGo ) && provToGo != army->GetProvinceIn()) {
			army->SetProvToGo( provToGo );
			provsAssigned.push_back( provToGo );
		}
		else {
			army->SetProvToGo( nullptr );
		}
	}
	
	// change target
	if (!provinceBeSieged.empty()) {
		m_targetEnemy = provinceBeSieged[0]->m_armyOn->m_owner;
		for (auto prov : provinceBeSieged) {
			if (prov->m_armyOn->m_owner->GetTotalArmySize() < m_targetEnemy->GetTotalArmySize()) {
				m_targetEnemy = prov->m_armyOn->m_owner;
			}
		}
	}

	/*
	// defense
	for (auto prov : m_controlledForce->m_ownedProvs) {
		if (m_controlledForce->m_commandPointAmount >= 5 && prov->IsLegal() && !prov->IsSiegedByEnemy() && prov->m_defenseRate < prov->m_maxDefenseRate) {
			prov->Defense();
			m_controlledForce->m_commandPointAmount -= 5;
		}
	}

	// recruit
	bool isAdded = true;
	while (m_controlledForce->m_commandPointAmount >= 1 && isAdded) {
		isAdded = false;
		for (auto army : m_controlledForce->m_armies) {
			if (m_controlledForce->m_commandPointAmount >= 1 && army->GetSize() < army->GetMaxSize() / 2 && m_controlledForce->isProvOwnedAndValid( army->m_inProvince )) {
				m_controlledForce->m_commandPointAmount -= 1;
				army->m_size += 1;
				isAdded = true;
			}
		}
	}

	// attract
	for (auto prov : m_controlledForce->m_ownedProvs) {
		if (m_controlledForce->m_commandPointAmount >= 5 && prov->IsLegal() && !prov->IsSiegedByEnemy()) {
			prov->Attract();
			m_controlledForce->m_commandPointAmount -= 5;
		}
	}

	*/
}

void StupidMonarchAI::UpdateProvDesc()
{
	m_provDesc.clear();
	m_provDesc.resize( (int)m_controlledForce->m_ownedProvs.size() );
	for (int i = 0; i < (int)m_controlledForce->m_ownedProvs.size(); i++) {
		Province* prov = m_controlledForce->m_ownedProvs[i];
		for (auto adjProv : prov->m_adjacentProvinces) {
			if (prov->m_owner != adjProv->m_owner) {
				m_provDesc[i].m_isBoarder = true;
				break;
			}
		}
		m_provDesc[i].m_prov = prov;
		m_provDesc[i].m_value = prov->m_economy * prov->m_population * 0.0001f;
		if (!prov->IsLegal()) {
			m_provDesc[i].m_value *= 0.1f;
		}
	}
	//std::sort( m_provDesc.begin(), m_provDesc.end(), 
	//	[]( AIProvinceDescription const& a, AIProvinceDescription const& b )->bool
	//	{ return a.m_value > b.m_value; } );
}

void StupidMonarchAI::BuildNewArmy()
{
	int loopTime = 0;
	while (m_controlledForce->GetArmyAmount() < m_controlledForce->m_maxArmyAmount && loopTime < 100 && m_controlledForce->m_commandPointAmount > 0) {
		AIProvinceDescription* armySpawnOn = nullptr;
		float maxValue = 0.f;
		for (auto& provDesc : m_provDesc) {
			float actualValue = provDesc.m_isBoarder ? provDesc.m_value - 10.f : provDesc.m_value;
			if (provDesc.m_prov->m_armyOn == nullptr && provDesc.m_prov->IsLegal() && actualValue > maxValue) {
				maxValue = actualValue;
				armySpawnOn = &provDesc;
			}
		}
		if (armySpawnOn != nullptr) {
			m_controlledForce->BuildAnNewArmy( armySpawnOn->m_prov );
			m_controlledForce->m_commandPointAmount -= 1;
		}
		loopTime++;
	}
}
