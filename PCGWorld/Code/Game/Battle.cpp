#include "Game/Battle.hpp"
#include "Game/Army.hpp"
#include "Game/Country.hpp"
#include "Game/MapPolygonUnit.hpp"
#include "Game/Map.hpp"
#include "Game/City.hpp"
#include "Game/Culture.hpp"

void RemoveArmyFromMap( Army* army, Map* map )
{
	army->m_owner->RemoveArmyFromList( army );
	for (int i = 0; i < (int)army->m_provIn->m_armiesOnProv.size(); i++) {
		if (army->m_provIn->m_armiesOnProv[i].first == army) {
			army->m_provIn->m_armiesOnProv.erase( army->m_provIn->m_armiesOnProv.begin() + i );
			break;
		}
	}
	if (map->m_curViewingArmy == army) {
		map->m_curViewingArmy = nullptr;
	}
	delete army;
}

BattleResult ResolveBattleArmyFromMap( ArmyBattleWrapper* defender, ArmyBattleWrapper* attacker, Map* map )
{
	UNUSED( map );
	float attackerCultureFactor = 1.f;
	float defenderCultureFactor = 1.f;
	if (attacker->m_country->m_countryCulture->HasTrait( CultureTrait::Quarrelsome )) {
		attackerCultureFactor -= 0.05f;
	}
	if (attacker->m_country->m_countryCulture->HasTrait( CultureTrait::Communal )) {
		defenderCultureFactor -= 0.05f;
	}
	if (defender->m_country->m_countryCulture->HasTrait( CultureTrait::Communal )) {
		attackerCultureFactor -= 0.05f;
	}
	if (attacker->m_country->m_countryCulture->HasTrait( CultureTrait::Vulnerable )) {
		defenderCultureFactor += 0.1f;
	}
	if (defender->m_country->m_countryCulture->HasTrait( CultureTrait::Vulnerable )) {
		attackerCultureFactor += 0.1f;
	}
	if (attacker->m_country->m_countryCulture->HasTrait( CultureTrait::Matrilineal )) {
		attackerCultureFactor += 0.05f;
	}
	if (defender->m_country->m_countryCulture->HasTrait( CultureTrait::Matrilineal )) {
		defenderCultureFactor += 0.05f;
	}
	
	if (attacker->m_country->m_countryCulture->HasTrait( CultureTrait::Irritable )) {
		attackerCultureFactor += 0.1f;
		defenderCultureFactor += 0.1f;
	}
	if (attacker->m_country->m_countryCulture->HasTrait( CultureTrait::Nomadic )) {
		attackerCultureFactor += 0.1f;
	}
	if (defender->m_country->m_countryCulture->HasTrait( CultureTrait::Irritable )) {
		attackerCultureFactor += 0.1f;
		defenderCultureFactor += 0.1f;
	}
	if (defender->m_country->m_countryCulture->HasTrait( CultureTrait::Quarrelsome )) {
		defenderCultureFactor -= 0.05f;
	}
	if (defender->m_country->m_countryCulture->HasTrait( CultureTrait::Nomadic )) {
		defenderCultureFactor += 0.1f;
	}
	int defenderAttackDamage = int(defenderCultureFactor * (float)defender->m_totalSize / 2.f * ((Hesitate3( map->m_historyRNG->RollRandomFloatZeroToOne() ) - 0.5f) * 0.5f + 0.5f) * defender->m_country->GetCombatValueMultiplier() );
	int attackerAttackDamage = int(attackerCultureFactor * (float)attacker->m_totalSize / 4.f * ((Hesitate3( map->m_historyRNG->RollRandomFloatZeroToOne() ) - 0.5f) * 0.5f + 0.5f) * attacker->m_country->GetCombatValueMultiplier() );
	defender->m_totalSize -= attackerAttackDamage;
	attacker->m_totalSize -= defenderAttackDamage;
	if (attacker->m_totalSize <= 0 && defender->m_totalSize <= 0) {
		defender->m_totalSize = 1;
		return BattleResult::Defender_Win_Attacker_Eliminated;
	}
	else if (attacker->m_totalSize <= 0) {
		return BattleResult::Defender_Win_Attacker_Eliminated;
	}
	else if (defender->m_totalSize <= 0) {
		return BattleResult::Attacker_Win_Defender_Eliminated;
	}
	else if (attackerAttackDamage <= defenderAttackDamage) {
		return BattleResult::Defender_Win_Attacker_Retreat;
	}
	else if (attackerAttackDamage > defenderAttackDamage) {
		return BattleResult::Attacker_Win_Defender_Retreat;
	}
	else {
		ERROR_AND_DIE( "Cannot reach this code!" );
	}
}

SiegeResult ResolveSiegeFromMap( Army* army, Province* prov, Map* map )
{
	if ((int)prov->m_cities.size() == 0) {
		if (map->m_historyRNG->RollRandomFloatZeroToOne() < 0.5f) {
			return SiegeResult::Defender_Surrender;
		}
		else {
			return SiegeResult::Defender_Keep_Defending;
		}
	}
	float totalDefense = 0.f;
	for (auto city : prov->m_cities) {
		totalDefense += city->m_defense;
	}
	float finalDefense = totalDefense - (float)army->m_size * 0.01f;
	float multiplier = 0.f;
	if (totalDefense != 0.f) {
		multiplier = std::max( 1.f - (float)army->m_size * 0.05f / totalDefense, 0.f );
	}
	else {
		multiplier = 0.f;
	}
	for (auto city : prov->m_cities) {
		city->m_defense *= multiplier;
	}
	int surrenderLevel = std::max( RoundDownToInt( finalDefense * 0.005f ), 4 );
	int rndRes = map->m_historyRNG->RollRandomIntInRange( 0, 9 );
	if (rndRes > surrenderLevel) {
		return SiegeResult::Defender_Surrender;
	}
	if (rndRes == 0) {
		return SiegeResult::Attacker_Lose_Morale;
	}
	if (rndRes == 9) {
		for (auto city : prov->m_cities) {
			city->m_defense *= 0.9f;
		}
		return SiegeResult::Defender_Lose_Morale;
	}
	return SiegeResult::Defender_Keep_Defending;
}

void RetreatFromProvince( Army* army, Map* map )
{
	MapPolygonUnit* prov = army->m_provIn;
	// remove the army from current province
	for (int i = 0; i < (int)prov->m_armiesOnProv.size(); i++) {
		if (prov->m_armiesOnProv[i].first == army) {
			prov->m_armiesOnProv.erase( prov->m_armiesOnProv.begin() + i );
		}
	}

	std::vector<MapPolygonUnit*> provsToRetreat;
	for (auto adjProv : prov->m_adjacentUnits) {
		// must retreat to friendly province without army
		if (!adjProv->IsWater() 
			&& ((int)adjProv->m_armiesOnProv.size() == 0 
				|| ((int)adjProv->m_armiesOnProv.size() == 1 && adjProv->m_armiesOnProv[0].first->m_owner == army->m_owner))
			&& adjProv->m_owner == army->m_owner) {
			provsToRetreat.push_back( adjProv );
		}
	}
	// no place to retreat, destroy this army
	if (provsToRetreat.size() == 0) {
		RemoveArmyFromMap( army, map );
	}
	else {
		MapPolygonUnit* provRetreat = provsToRetreat[map->m_historyRNG->RollRandomIntLessThan( (int)provsToRetreat.size() )];
		if ((int)provRetreat->m_armiesOnProv.size() == 0) {
			army->m_provIn = provRetreat;
			provRetreat->m_armiesOnProv.push_back( std::pair<Army*, float>( army, 0.f ) );
		}
		else {
			// integrate to the first army
			army->m_provIn = provRetreat;
			provRetreat->m_armiesOnProv[0].first->IntegrateArmy( army );
			RemoveArmyFromMap( army, map );
		}
	}
}
