#include "Game/Force.hpp"
#include "Game/Province.hpp"
#include "Game/Army.hpp"
#include "Game/AIUtils.hpp"
#include "Game/TranslationUtils.hpp"
#include "Game/SM_BitMapFont.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"

Force::Force( Map* map, int id, Rgba8 const& color, std::string const& nickName, Province* capital )
	:m_map(map), m_id(id), m_color(color), m_nickName(nickName), m_capitalProv(capital)
{
	m_ai = new StupidMonarchAI( 1.f, this );
}

Force::~Force()
{
	for (int i = 0; i < (int)m_armies.size(); i++) {
		if (m_armies[i]) {
			delete m_armies[i];
			m_armies[i] = nullptr;
		}
	}
	delete m_ai;
}

void Force::StartUp( bool loadFromSave )
{
	if (!loadFromSave) {
		if (m_ownedProvs.size() > 0) {
			// add armies
			std::vector<Province*> majorProvs;
			for (auto prov : m_ownedProvs) {
				if (prov->m_isMajorProv) {
					majorProvs.push_back( prov );
				}
			}
			if (majorProvs.size() == 0) {
				m_armies.push_back( new Army( this, m_capitalProv, 100, int( GetTotalPopulation() / 8000.f ) ) );
			}
			else {
				for (auto prov : majorProvs) {
					m_armies.push_back( new Army( this, prov, 100, int( GetTotalPopulation() / 8000.f / (float)majorProvs.size() ) ) );
				}
			}
			m_maxArmyAmount = (int)majorProvs.size();
			if (m_maxArmyAmount == 0) {
				m_maxArmyAmount = 1;
			}
		}
		m_commandPointAmount = CalculateCommandPoint();
	}
}

void Force::Update()
{
	if (m_capitalProv->m_owner != this && (int)m_ownedProvs.size() >= 1) {
		int economy = 0;
		for (auto prov : m_ownedProvs) {
			if (prov->GetEconomyCorrectedPoint() > economy) {
				economy = (int)prov->GetEconomyCorrectedPoint();
				m_capitalProv = prov;
			}
		}
	}
}

void Force::Render()
{
	for (auto army : m_armies) {
		army->Render();
	}
	if (g_theGame->m_worldCameraScale >= 1.4f) {
		std::vector<Vertex_PCU> textVerts;
		AABB2 bounds = GetBounds();
		float xSize = bounds.m_maxs.x - bounds.m_mins.x;
		float ySize = bounds.m_maxs.y - bounds.m_mins.y;
		float xScale = 0.5f;
		float yScale = 0.3f;
		Vec2 center = m_capitalProv->GetCenter();
		bounds.m_maxs = center + Vec2( xSize * 0.5f * xScale, ySize * 0.5f * yScale );
		bounds.m_mins = center - Vec2( xSize * 0.5f * xScale, ySize * 0.5f * yScale );

		AddVertsForTextPlaceHolder( textVerts, bounds, 1000000.f,
			Stringf( "$(force%d)", m_id ), Rgba8( 255, 255, 255, 160 ) );
		bounds.Translate( Vec2( 0.2f, 0.2f ) );
		AddVertsForTextPlaceHolder( textVerts, bounds, 1000000.f,
			Stringf( "$(force%d)", m_id ), Rgba8( 0, 0, 0 ) );
		if (g_gameLanguage == SM_GameLanguage::ENGLISH) {
			g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
		}
		else if (g_gameLanguage == SM_GameLanguage::ZH) {
			g_theRenderer->BindTexture( &g_chineseFont->GetTexture() );
		}
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->DrawVertexArray( textVerts );
	}
}

void Force::NextTurn()
{
	//if (m_nickName == "FX") {
	//	m_ai->ConductAI();
	//}
	if (!m_isPlayer && m_ai) {
		m_ai->ConductAI();
	}

	for (auto province : m_ownedProvs) {
		province->NextTurn();
	}

	for (int i = 0; i < (int)m_armies.size(); i++) {
		for (int j = i + 1; j < (int)m_armies.size(); j++) {
			if (m_armies[i]->m_provToGoNext == m_armies[j]->m_provToGoNext && m_armies[i]->m_provToGoNext != nullptr) {
				m_armies[i]->m_size += m_armies[j]->m_size;
				if (m_armies[i]->m_size > 100) {
					m_armies[i]->m_size = 100;
				}
				if (m_armies[j]->m_inProvince->m_armyOn == m_armies[j]) {
					m_armies[j]->m_inProvince->m_armyOn = nullptr;
				}
				delete m_armies[j];
				m_armies.erase( m_armies.begin() + j );
				j--;
			}
		}
	}

	for (int i = 0; i < (int)m_armies.size(); i++) {
		m_armies[i]->NextTurn();
		if (m_armies[i]->m_size == 0) {
			if (m_armies[i]->m_inProvince->m_armyOn == m_armies[i]) {
				m_armies[i]->m_inProvince->m_armyOn = nullptr;
			}
			delete m_armies[i];
			m_armies.erase( m_armies.begin() + i );
			i--;
		}
	}

	m_commandPointAmount = CalculateCommandPoint();
}

void Force::GainProvince( Province* provToGain )
{
	for (int i = 0; i < (int)m_ownedProvs.size(); i++) {
		if (m_ownedProvs[i] == provToGain) {
			return;
		}
	}
	if (provToGain->IsMajorProvince()) {
		m_maxArmyAmount++;
	}
	m_ownedProvs.push_back( provToGain );
	if (!isProvOwned( m_capitalProv ) && m_armies.size() != 0) {
		m_capitalProv = provToGain;
	}
}

void Force::LoseProvince( Province* provToLose )
{
	for (int i = 0; i < (int)m_ownedProvs.size(); i++) {
		if (m_ownedProvs[i] == provToLose) {
			m_ownedProvs.erase( m_ownedProvs.begin() + i );
			if (provToLose->IsMajorProvince()) {
				m_maxArmyAmount--;
				if (m_maxArmyAmount == 0) {
					m_maxArmyAmount = 1;
				}
			}
			return;
		}
	}
	//ERROR_RECOVERABLE( Stringf( "Cannot remove province id %d from %s", provToLose->GetId(), m_nickName.c_str() ) );
}

void Force::RemoveArmy( Army* armyToRemove )
{
	for (int i = 0; i < (int)m_armies.size(); i++) {
		if (m_armies[i] == armyToRemove) {
			delete armyToRemove;
			m_armies.erase( m_armies.begin() + i );
			break;
		}
	}
}

void Force::BuildAnNewArmy( Province* provinceToBuild )
{
	Army* arm = new Army( this, provinceToBuild, 100, 1 );
	m_armies.push_back( arm );
	provinceToBuild->AddArmyOnto( arm );
}

int Force::CalculateCommandPoint() const
{
	int res = GetMaxCommandPoint() - GetTotalArmySize() * GetTotalArmySize() / 1500 - GetArmyAmount();
	if (res < 0) {
		res = 0;
	}
	return res;
}

float Force::GetRatioOfArmyCommandPointCost() const
{
	float res = (float)(GetTotalArmySize() * GetTotalArmySize() / 1500 + GetArmyAmount()) / (float)GetMaxCommandPoint();
	res = GetClamped( res, 0.f, 1.f );
	return res;
}

bool Force::isProvOwned( Province* provToCheck ) const
{
	for (int i = 0; i < (int)m_ownedProvs.size(); i++) {
		if (m_ownedProvs[i] == provToCheck) {
			return true;
		}
	}
	return false;
}

bool Force::isProvOwnedAndValid( Province* provToCheck ) const
{
	return isProvOwned( provToCheck ) && provToCheck->IsLegal();
}

bool Force::isCalledNickName( std::string const& compareName ) const
{
	return compareName == m_nickName;
}

bool Force::isAI() const
{
	return !m_isPlayer;
}

bool Force::isAlive() const
{
	return (int)m_ownedProvs.size() != 0 || (int)m_armies.size() != 0;
}

bool Force::isAdjancentToForce( Force* force ) const
{
	std::vector<Force*> adjForces;
	GetAllAdjancentForces(adjForces);
	if (find( adjForces.begin(), adjForces.end(), force ) == adjForces.end()) {
		return false;
	}
	return true;
}

void Force::SetAsPlayer( bool isPlayer )
{
	m_isPlayer = isPlayer;
}

float Force::GetTotalPopulation() const
{
	float sumPop = 0;
	for (auto prov : m_ownedProvs) {
		sumPop += prov->GetPopulation();
	}
	return sumPop;
}

float Force::GetCorrectedEconomyPoint() const
{
	float sumEco = 0.f;
	for (auto prov : m_ownedProvs) {
		if (prov->IsLegal()) {
			sumEco += prov->GetEconomyCorrectedPoint();
		}
		else {
			sumEco += prov->GetEconomyCorrectedPoint() * 0.1f;
		}
	}
	return sumEco;
}

Rgba8 const& Force::GetForceColor() const
{
	return m_color;
}

std::string const& Force::GetNickName() const
{
	return m_nickName;
}

int Force::GetCommandPoint() const
{
	return m_commandPointAmount;
}

int Force::GetMaxCommandPoint() const
{
	return 5 + int( GetCorrectedEconomyPoint() / 200000.f );
}

int Force::GetArmyAmount() const
{
	return (int)m_armies.size();
}

int Force::GetMaxArmyAmount() const
{
	return m_maxArmyAmount;
}

Province* Force::GetCapitalProv() const
{
	return m_capitalProv;
}

Army* Force::GetArmyAtWorldPos( Vec2 const& worldPos ) const
{
	for (auto army : m_armies) {
		if (IsPointInsideDisc2D( worldPos, army->GetCenter(), army->GetRadius() )) {
			return army;
		}
	}
	return nullptr;
}

std::vector<Province*> Force::GetOwnedProvs() const
{
	return m_ownedProvs;
}

int Force::GetHuhuaness() const
{
	float totalPop = 0;
	float huhuaPop = 0;
	for (auto prov : m_ownedProvs) {
		totalPop += prov->m_population;
		huhuaPop += prov->m_population * prov->m_huhuaness;
	}
	if (totalPop == 0) {
		return 0;
	}
	return int(huhuaPop * 100.f / totalPop);
}

int Force::GetTotalArmySize() const
{
	int res = 0;
	for (auto army : m_armies) {
		res += army->GetSize();
	}
	return res;
}

int Force::GetArmyId( Army* army ) const
{
	for (int i = 0; i < (int)m_armies.size(); i++) {
		if (m_armies[i] == army) {
			return i;
		}
	}
	return -1;
}

void Force::GetAllAdjancentForces( std::vector<Force*>& adjForces ) const
{
	adjForces.clear();
	for (auto prov : m_ownedProvs) {
		for (auto adjProv : prov->m_adjacentProvinces) {
			if (adjProv->m_owner != this && find( adjForces.begin(), adjForces.end(), adjProv->m_owner ) == adjForces.end()) {
				adjForces.push_back( adjProv->m_owner );
			}
		}
	}
}

AABB2 Force::GetBounds() const
{
	float minX = FLT_MAX;
	float maxX = 0.f;
	float minY = FLT_MAX;
	float maxY = 0.f;
	Vec2 capitalCenter = GetCapitalProv()->GetCenter();
	float maxBLDistance = 0.f;
	float maxTRDistance = 0.f;
	for (auto prov : m_ownedProvs) {
		if (m_map->IsTwoProvsConnected( m_capitalProv, prov, const_cast<Force*>(this) )) {
			if (prov->m_leftBottomPos.x <= capitalCenter.x && prov->m_leftBottomPos.y <= capitalCenter.y) {
				float taxicabDis = GetTaxicabDistance2D( prov->m_leftBottomPos, capitalCenter );
				if (taxicabDis > maxBLDistance) {
					maxBLDistance = taxicabDis;
					minX = prov->m_leftBottomPos.x;
					minY = prov->m_leftBottomPos.y;
				}
			}
			if (prov->m_rightTopPos.x >= capitalCenter.x && prov->m_rightTopPos.y >= capitalCenter.y) {
				float taxicabDis = GetTaxicabDistance2D( prov->m_rightTopPos, capitalCenter );
				if (taxicabDis > maxTRDistance) {
					maxTRDistance = taxicabDis;
					maxX = prov->m_rightTopPos.x;
					maxY = prov->m_rightTopPos.y;
				}
			}
			/*
			if (prov->m_leftBottomPos.x < minX) {
				minX = prov->m_leftBottomPos.x;
			}
			if (prov->m_rightTopPos.x > maxX) {
				maxX = prov->m_rightTopPos.x;
			}
			if (prov->m_leftBottomPos.y < minY) {
				minY = prov->m_leftBottomPos.y;
			}
			if (prov->m_rightTopPos.y > maxY) {
				maxY = prov->m_rightTopPos.y;
			}
			*/
		}
	}
	return AABB2( Vec2( minX, minY ), Vec2( maxX, maxY ) );
}
