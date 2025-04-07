#include "Game/Army.hpp"
#include "Game/Province.hpp"
#include "Game/Force.hpp"
#include "Game/Game.hpp"
#include "Game/BattleReport.hpp"
#include "Game/Map.hpp"
#include "Game/SM_BitMapFont.hpp"
#include "Game/TranslationUtils.hpp"
#include <vector>

Army::Army( Force* owner, Province* position, int maxSize, int size )
	:m_owner(owner), m_inProvince(position), m_maxSize(maxSize), m_size(size)
{
	position->AddArmyOnto( this );
	Rgba8 const& ownerColor = m_owner->GetForceColor();
	m_color.r = ownerColor.r - 50 < 0 ? 0 : ownerColor.r - 50;
	m_color.g = ownerColor.g - 50 < 0 ? 0 : ownerColor.g - 50;
	m_color.b = ownerColor.b - 50 < 0 ? 0 : ownerColor.b - 50;
	m_color.a = 200;
}

Army::~Army()
{

}

void Army::Render()
{
	std::vector<Vertex_PCU> verts;
	float renderSize = Maxf( (float)m_size / 20.f, 1.5f );
	AddVertsForDisc2D( verts, m_inProvince->GetCenter(), renderSize, m_color );
	if (m_provToGoNext) {
		AddVertsForArrow2D( verts, m_inProvince->GetCenter(), m_provToGoNext->GetCenter(), 0.8f, 0.5f, Rgba8( 200, 0, 0 ) );
		LinkType type = m_owner->m_map->GetProvLinkType( m_inProvince, m_provToGoNext );
		//if ((m_provToGoNext->m_armyOn && m_provToGoNext->m_armyOn->m_owner != m_owner) || m_provToGoNext->m_owner != m_owner) {
			if (type == LinkType::River) {
				AddVertsForDisc2D( verts, (m_inProvince->GetCenter() + m_provToGoNext->GetCenter()) * 0.5f, 1.5f, Rgba8( 0, 0, 200 ) );
			}
			else if (type == LinkType::BigRiver) {
				AddVertsForDisc2D( verts, (m_inProvince->GetCenter() + m_provToGoNext->GetCenter()) * 0.5f, 1.5f, Rgba8( 100, 100, 200 ) );
			}
			if (m_provToGoNext->IsMountain()) {
				Vec2 pivot = m_inProvince->GetCenter() * 0.25f + m_provToGoNext->GetCenter() * 0.75f;
				AddVertsForTriangle2D( verts, pivot + Vec2::MakeFromPolarDegrees( -30.f, 3.f ), pivot + Vec2::MakeFromPolarDegrees( 90.f, 3.f ), pivot + Vec2::MakeFromPolarDegrees( 210.f, 3.f ), Rgba8( 102, 51, 0 ) );
			}
		//}
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
	std::vector<Vertex_PCU> idVerts;
	AddVertsForTextPlaceHolder( idVerts, AABB2( m_inProvince->GetCenter() - Vec2(renderSize * 0.707f, renderSize * 0.707f), m_inProvince->GetCenter() + Vec2( renderSize * 0.707f, renderSize * 0.707f ) )
		, renderSize, Stringf( "%d", m_owner->GetArmyId( this ) + 1 ), Rgba8( 0, 0, 0 ) );
	if (g_gameLanguage == SM_GameLanguage::ENGLISH) {
		g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	}
	else if (g_gameLanguage == SM_GameLanguage::ZH) {
		g_theRenderer->BindTexture( &g_chineseFont->GetTexture() );
	}		
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( idVerts );
}

bool Army::IsProvValidToGo( Province* provToGo ) const
{
	if (provToGo && ((m_owner->m_map->IsTwoProvsConnected(m_inProvince, provToGo, m_owner)) || m_inProvince->IsAdjacent( provToGo ) || m_inProvince == provToGo)) {
		return true;
	}
	else {
		return false;
	}

}

Vec2 const Army::GetCenter() const
{
	return m_inProvince->GetCenter();
}

float Army::GetRadius() const
{
	return Maxf( (float)m_size / 20.f, 2.f );
}

Province* Army::GetProvinceIn() const
{
	return m_inProvince;
}

Force* Army::GetOwner() const
{
	return m_owner;
}

int Army::GetSize() const
{
	return m_size;
}

int Army::GetMaxSize() const
{
	return m_maxSize;
}

void Army::SetSize( int size )
{
	m_size = size;
	if (m_size > m_maxSize) {
		m_size = m_maxSize;
	}
	else if (m_size < 0) {
		m_size = 0;
	}
}

void Army::SetProvIn( Province* provNowIn )
{
	m_inProvince = provNowIn;
}

void Army::SetProvToGo( Province* provToGo )
{
	m_provToGoNext = provToGo;
}

// CRT
/*
	   1:3  1:2  1:1  2:1  3:1  4:1  5:1  6:1
	0   0	 2	  4    6    8    10   12   14
	1   1	 3    5    7    9	 11	  13   15
	2   2	 4    6    8    10	 12	  14   16
	3   3	 5    7    9    11	 13	  15   17
	4   4	 6    8    10   12	 14	  16   18
	5   5	 7    9    11   13	 15	  17   19
	6   6	 8    10   12   14	 16	  18   20
	7   7	 9    11   13   15	 17	  19   21
	8   8	 10   12   14   16	 18	  20   22
	9   9	 11   13   15   17	 19	  21   23

						  +1.5/+1						+1.5/+1.5						 +1/+1.5
	           0	1	2	3	4	5 | 6	7	8	9	10	11	12	13 |14	15	16	17	18	19	20	21	22	23
  0-20			3/0 3/0	2/0	2/0	2/0	2/1	2/1	2/1	1/1	1/1 1/1 1/2 1/2 1/2 0/2 0/2 0/2 0/3 0/3 0/3 0/4 0/4 0/4 0/5
  21-40
  41-60
  61-80
  81-100
  101-120
  121-140
  141-160
  161-180
  180+
*/

void Army::NextTurn()
{
	if (m_provToGoNext) {
		Army* holdingArmy = m_provToGoNext->GetArmyOn();
		// meet army
		if (holdingArmy) {
			// enemy
			if (m_owner != holdingArmy->GetOwner()) {
				BattleReport* battleReport = nullptr;
				if (!m_owner->isAI() || !holdingArmy->GetOwner()->isAI()) {
					battleReport = new BattleReport();
					battleReport->m_battlePosition = m_provToGoNext->GetCenter();
					battleReport->m_type = BattleType::COMBAT;
					battleReport->m_provID = m_provToGoNext->m_id;
				}
				int enemySize = holdingArmy->GetSize();
				float enemyHuhuaness = (float)holdingArmy->GetOwner()->GetHuhuaness() / 100.f;
				float enemyStrengthFactor = -0.4f * enemyHuhuaness * enemyHuhuaness + 0.8f * enemyHuhuaness + 0.6f;
				int selfSize = m_size;
				float selfHuhuaness = (float)m_owner->GetHuhuaness() / 100.f;
				float selfStrengthFactor = -0.4f * selfHuhuaness * selfHuhuaness + 0.8f * selfHuhuaness + 0.6f;
				RandomNumberGenerator* rng = g_theGame->m_randNumGen;
				// modifiers: cross river, go into mountain
				int modifyRow = 0;
				int modifyRoll = 0;
				// is defender
				bool isDefender = false;
				if (m_provToGoNext->GetOwner() == m_owner) {
					isDefender = true;
				}
				// advantages
				if (isDefender) {
					if (m_provToGoNext->IsMountain()) {
						modifyRow++;
					}
					if (m_owner->m_map->GetProvLinkType( m_inProvince, m_provToGoNext ) == LinkType::River) {
						modifyRow += 1;
					}
					else if (m_owner->m_map->GetProvLinkType( m_inProvince, m_provToGoNext ) == LinkType::BigRiver) {
						modifyRow += 2;
					}
					if (!m_owner->isAI() || !holdingArmy->GetOwner()->isAI()) {
						battleReport->m_attackerForceId = holdingArmy->m_owner->m_id;
						battleReport->m_defenderForceId = m_owner->m_id;
					}
				}
				else {
					if (m_provToGoNext->IsMountain()) {
						modifyRow--;
					}
					if (m_owner->m_map->GetProvLinkType( m_inProvince, m_provToGoNext ) == LinkType::River) {
						modifyRow -= 1;
					}
					else if (m_owner->m_map->GetProvLinkType( m_inProvince, m_provToGoNext ) == LinkType::BigRiver) {
						modifyRow -= 2;
					}
					if (!m_owner->isAI() || !holdingArmy->GetOwner()->isAI()) {
						battleReport->m_attackerForceId = m_owner->m_id;
						battleReport->m_defenderForceId = holdingArmy->m_owner->m_id;
					}
				}
				int totalSize = enemySize + selfSize;
				if (totalSize >= 30) {
					BattleInfluenceOnProvPop( m_provToGoNext );
				}
				// 3 rounds battle stage
				for (int _ = 0; _ < 3; _++) {
					int selfStrength = (int)((float)selfSize * selfStrengthFactor);
					int enemyStrength = (int)((float)enemySize * enemyStrengthFactor);
					if (selfStrength == 0) {
						selfStrength = 1;
					}
					if (enemyStrength == 0) {
						enemyStrength = 1;
					}
					int row;
					if (selfStrength >= enemyStrength) {
						row = GetClamped( GetClamped( selfStrength / enemyStrength, 1, 6 ) + 1 + modifyRow, 0, 7 );
					}
					else {
						row = GetClamped( 2 - GetClamped( enemyStrength / selfStrength, 1, 2 ) + modifyRow, 0, 7 );
					}
					int rollRes = GetClamped( rng->RollRandomIntInRange( 0, 9 ) - modifyRoll, 0, 9 );
					int tableCol = rollRes + row * 2;
					int selfCasualties = 0;
					int enemyCasualties = 0;
					int tableRow = totalSize / 20;
					if (tableCol <= 5) {// 0-5
						if (tableCol == 0 || tableCol == 1) {
							selfCasualties = 3;
							enemyCasualties = 0;
						}
						if (tableCol == 2 || tableCol == 3 || tableCol == 4) {
							selfCasualties = 2;
							enemyCasualties = 0;
						}
						if (tableCol == 5) {
							selfCasualties = 2;
							enemyCasualties = 1;
						}
						selfCasualties += tableRow + tableRow / 2;
						enemyCasualties += tableRow;
					}
					else if (tableCol <= 13) { // 6-13
						if (tableCol == 6 || tableCol == 7) {
							selfCasualties = 2;
							enemyCasualties = 1;
						}
						if (tableCol == 8 || tableCol == 9 || tableCol == 10) {
							selfCasualties = 1;
							enemyCasualties = 1;
						}
						if (tableCol == 11 || tableCol == 12 || tableCol == 13) {
							selfCasualties = 1;
							enemyCasualties = 2;
						}
						selfCasualties += tableRow + tableRow / 2;
						enemyCasualties += tableRow + tableRow / 2;
					}
					else { // 14-23
						if (tableCol == 14 || tableCol == 15 || tableCol == 16) {
							selfCasualties = 0;
							enemyCasualties = 2;
						}
						if (tableCol == 17 || tableCol == 18 || tableCol == 19) {
							selfCasualties = 0;
							enemyCasualties = 3;
						}
						if (tableCol == 20 || tableCol == 21 || tableCol == 22) {
							selfCasualties = 0;
							enemyCasualties = 4;
						}
						if (tableCol == 23) {
							selfCasualties = 0;
							enemyCasualties = 5;
						}
						selfCasualties += tableRow;
						enemyCasualties += tableRow + tableRow / 2;
					}
					if (!m_owner->isAI() || !holdingArmy->GetOwner()->isAI()) {
						if (isDefender) {
							battleReport->AddBattleReport( selfSize, enemySize, rollRes, enemyCasualties, selfCasualties );
						}
						else {
							battleReport->AddBattleReport( selfSize, enemySize, rollRes, selfCasualties, enemyCasualties );
						}
					}
					selfCasualties = selfCasualties > enemySize ? enemySize : selfCasualties;
					enemyCasualties = enemyCasualties > selfSize ? selfSize : enemyCasualties;
					selfSize = selfSize - selfCasualties > 0 ? selfSize - selfCasualties : 0;
					enemySize = enemySize - enemyCasualties > 0 ? enemySize - enemyCasualties : 0;
					if (selfSize == 0 || enemySize == 0) {
						break;
					}
				}
				// decide who win
				bool isSelfWin;
				if (selfSize == 0) {
					isSelfWin = false;
					if (enemySize == 0) {
						enemySize = 1;
					}
				}
				else if (enemySize == 0) {
					isSelfWin = true;
				}
				else {
					int selfTotalCasualties = m_size - selfSize;
					int enemyTotalCasualties = holdingArmy->GetSize() - enemySize;
					if (selfTotalCasualties >= enemyTotalCasualties) {
						isSelfWin = false;
					}
					else {
						isSelfWin = true;
					}
				}
				if (!m_owner->isAI() || !holdingArmy->GetOwner()->isAI()) {
					if (isDefender) {
						battleReport->m_battleResult = !isSelfWin;
						battleReport->m_selfRemainSize = selfSize;
						battleReport->m_enemyRemainSize = enemySize;
						g_devConsole->AddLine( Rgba8::WHITE, Stringf( "Battle happens in provID %d! Attacker: %s Defender: %s", battleReport->m_provID, holdingArmy->m_owner->m_nickName.c_str(), m_owner->m_nickName.c_str() ) );
					}
					else {
						battleReport->m_battleResult = isSelfWin;
						battleReport->m_selfRemainSize = selfSize;
						battleReport->m_enemyRemainSize = enemySize;
						g_devConsole->AddLine( Rgba8::WHITE, Stringf( "Battle happens in provID %d! Attacker: %s Defender: %s", battleReport->m_provID, m_owner->m_nickName.c_str(), holdingArmy->m_owner->m_nickName.c_str() ) );
					}
					g_devConsole->AddLine( Rgba8::WHITE, Stringf( "Attacker Come From:%d Attacker Size:%d Defender Size:%d Attacker Remain:%d Defender Remain:%d", m_inProvince->m_id, m_size, holdingArmy->m_size, selfSize, enemySize ) );
				}
				// retreat stage
				if (isSelfWin) {
					m_size = selfSize;
					bool isEnemyDestroyed = false;
					Province* provToRetreat = nullptr;
					if (enemySize <= 0) {
						// enemy destroyed
						m_provToGoNext->RemoveArmyOn();
						holdingArmy->GetOwner()->RemoveArmy( holdingArmy );
						isEnemyDestroyed = true;
					}
					else {
						// enemy retreat
						holdingArmy->SetSize( enemySize );
						std::vector<Province*> possibleProvs = m_provToGoNext->GetAdjacentProvinces();
						for (auto prov : possibleProvs) {
							if (prov->GetOwner() == holdingArmy->GetOwner() && prov->GetArmyOn() == nullptr && prov != m_inProvince) {
								// goto that province
								holdingArmy->SetProvIn( prov );
								provToRetreat = prov;
								prov->AddArmyOnto( holdingArmy );
								break;
							}
						}
						// if has no place to go (still in that prov)
						if (holdingArmy->GetProvinceIn() == m_provToGoNext) {
							// destroy enemy
							m_provToGoNext->RemoveArmyOn();
							holdingArmy->GetOwner()->RemoveArmy( holdingArmy );
							isEnemyDestroyed = true;
						}
					}
					if (!m_owner->isAI() || !holdingArmy->GetOwner()->isAI()) {
						if (enemySize <= 0) {
							g_devConsole->AddLine( Rgba8::WHITE, Stringf( "Attacker Win! Defender Destroyed!" ) );
						}
						else if (isEnemyDestroyed) {
							g_devConsole->AddLine( Rgba8::WHITE, Stringf( "Attacker Win! Defender has no prov to retreat, destroyed!" ) );
						}
						else {
							g_devConsole->AddLine( Rgba8::WHITE, Stringf( "Attacker Win! Defender retreats to %d", provToRetreat->m_id ) );
						}
						g_theGame->AddBattleReport( battleReport );
					}
					// self go to province to go
					m_inProvince->RemoveArmyOn();
					m_provToGoNext->AddArmyOnto( this );
					m_inProvince = m_provToGoNext;
					m_provToGoNext = nullptr;
					if (m_inProvince->GetOwner() != m_owner) {
						// siege
						PerformSiege();
					}
				}
				else {
					holdingArmy->SetSize( enemySize );
					bool isSelfDestroyed = false;
					Province* provToRetreat = nullptr;
					if (selfSize == 0) {
						m_size = 0;
						isSelfDestroyed = true;
						// self destroyed
						// delete self in force function
					}
					else {
						m_size = selfSize;
						// self retreat
						std::vector<Province*> possibleProvs = m_provToGoNext->GetAdjacentProvinces();
						for (auto prov : possibleProvs) {
							if (prov->GetOwner() == m_owner && (prov->GetArmyOn() == nullptr || prov == m_inProvince)) {
								// goto that province
								m_inProvince->RemoveArmyOn();
								m_inProvince = prov;
								provToRetreat = prov;
								m_provToGoNext = nullptr;
								m_inProvince->AddArmyOnto( this );
								break;
							}
						}
						// if has no place to go (still in that prov)
						if (m_provToGoNext != nullptr) {
							// destroy self
							m_size = 0;
							isSelfDestroyed = true;
							// delete self in force function
						}
					}
					if (!m_owner->isAI() || !holdingArmy->GetOwner()->isAI()) {
						if (enemySize <= 0) {
							g_devConsole->AddLine( Rgba8::WHITE, Stringf( "Defender Win! Attacker Destroyed!" ) );
						}
						else if (isSelfDestroyed) {
							g_devConsole->AddLine( Rgba8::WHITE, Stringf( "Defender Win! Attacker has no prov to retreat, destroyed!" ) );
						}
						else {
							g_devConsole->AddLine( Rgba8::WHITE, Stringf( "Defender Win! Attacker retreats to %d", provToRetreat->m_id ) );
						}
						g_theGame->AddBattleReport( battleReport );
					}
				}
			}
			// friend
			else {
				holdingArmy->SetSize( m_size + holdingArmy->GetSize() );
				m_size = 0;
				m_inProvince->RemoveArmyOn();
				//m_owner->RemoveArmy( holdingArmy );
			}
		}
		else {
			// self go to province to go
			m_inProvince->RemoveArmyOn();
			m_provToGoNext->AddArmyOnto( this );
			m_inProvince = m_provToGoNext;
			m_provToGoNext = nullptr;
			if (m_inProvince->GetOwner() != m_owner) {
				if (m_size >= 25) {
					BattleInfluenceOnProvPop( m_inProvince );
				}
				// siege
				PerformSiege();
			}
		}
	}
	else if (m_inProvince->GetOwner() != m_owner) {
		if (m_size >= 25) {
			BattleInfluenceOnProvPop( m_inProvince );
		}
		PerformSiege();
	}
}
// Siege result table
/*
	1:2 1:1 2:1 3:1 4:1 5:1 6:1 7:1 8:1 9:1 10:1
	10	9	8	7	6	5	4	3	2	1	0
*/

void Army::PerformSiege()
{
	BattleReport* battleReport = nullptr;
	bool needBattleReport = !m_owner->isAI() || !m_inProvince->GetOwner()->isAI();
	if (needBattleReport) {
		battleReport = new BattleReport();
		battleReport->m_battlePosition = m_inProvince->GetCenter();
		battleReport->m_type = BattleType::SIEGE;
	}
	// siege
	int rollRes = g_theGame->m_randNumGen->RollRandomIntInRange( 0, 9 );
	float curDef = m_inProvince->GetDefense();
	if (needBattleReport) {
		battleReport->m_provID = m_inProvince->m_id;
		battleReport->m_beginSize = m_size;
		battleReport->m_beginDefense = (int)curDef;
		battleReport->m_diceRollRes = rollRes;
		battleReport->m_attackerForceId = m_owner->m_id;
		battleReport->m_defenderForceId = m_inProvince->m_owner->m_id;
	}
	float reducedDef = curDef - m_size * 20;
	if (reducedDef < 0.f) {
		reducedDef = 0.f;
	}
	if (needBattleReport) {
		battleReport->m_endDefense = (int)reducedDef;
	}
	m_inProvince->SetDefense( reducedDef );
	float normalizedDef = reducedDef / 100.f;
	if (normalizedDef < 1.f) {
		normalizedDef = 1.f;
	}
	if (m_size >= normalizedDef) {
		if (m_inProvince->IsMountain()) {
			rollRes -= 1;
		}
		int minResToCapture = 10 - GetClamped( m_size / (int)normalizedDef, 1, 10 );
		if (rollRes >= minResToCapture) {
			// surrender and capture
			if (needBattleReport) {
				battleReport->m_isSuccess = true;
			}
			m_inProvince->GetOwner()->LoseProvince( m_inProvince );
			m_owner->GainProvince( m_inProvince );
			m_inProvince->SetOwner( m_owner );
			rollRes = g_theGame->m_randNumGen->RollRandomIntInRange( 0, 9 );
			//if (rollRes == 0) {
			//	rollRes = g_theGame->m_randNumGen->RollRandomIntInRange( 0, 9 );
			//	if (rollRes <= 4) {
			//		m_inProvince->SubstractEconomy( 1 );
			//	}
			//}
		}
		else {
			rollRes = g_theGame->m_randNumGen->RollRandomIntInRange( 0, 9 );
			if (rollRes <= 4 && m_size > 1) {
				m_size--;
			}
		}
	}
	if (needBattleReport) {
		battleReport->m_endSize = m_size;
		g_theGame->AddBattleReport( battleReport );
	}
}

void Army::BattleInfluenceOnProvPop( Province* prov )
{
	float popToLose = (float)prov->GetPopulation() * 0.10f;
	prov->SubstractPopulation( popToLose );
	prov->SpreadPopulation( popToLose * 0.8f );
}

PreBattleStat const GetPreBattleStat( Army* selfArmy, Army* enemyArmy, Province* prov ) {
	PreBattleStat stat;
	stat.enemySize = enemyArmy->GetSize();
	float enemyHuhuaness = (float)enemyArmy->GetOwner()->GetHuhuaness() / 100.f;
	stat.enemyStrengthFactor = -0.4f * enemyHuhuaness * enemyHuhuaness + 0.8f * enemyHuhuaness + 0.6f;
	stat.selfSize = selfArmy->GetSize();
	float selfHuhuaness = (float)selfArmy->GetOwner()->GetHuhuaness() / 100.f;
	stat.selfStrengthFactor = -0.4f * selfHuhuaness * selfHuhuaness + 0.8f * selfHuhuaness + 0.6f;
	// modifiers: cross river, go into mountain
	// is defender
	bool isDefender = false;
	if (prov->GetOwner() == selfArmy->GetOwner()) {
		isDefender = true;
	}
	// advantages
	if (isDefender) {
		if (prov->IsMountain()) {
			stat.modifyRow++;
		}
		if (g_theGame->m_map->GetProvLinkType( selfArmy->GetProvinceIn(), prov ) == LinkType::River) {
			stat.modifyRow += 1;
		}
		else if (g_theGame->m_map->GetProvLinkType( selfArmy->GetProvinceIn(), prov ) == LinkType::BigRiver) {
			stat.modifyRow += 2;
		}
	}
	else {
		if (prov->IsMountain()) {
			stat.modifyRow--;
		}
		if (g_theGame->m_map->GetProvLinkType( selfArmy->GetProvinceIn(), prov ) == LinkType::River) {
			stat.modifyRow -= 1;
		}
		else if (g_theGame->m_map->GetProvLinkType( selfArmy->GetProvinceIn(), prov ) == LinkType::BigRiver) {
			stat.modifyRow -= 2;
		}
	}
	return stat;
}