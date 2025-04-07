#pragma once
#include "Game/GameCommon.hpp"
class Force;
class Province;

/*
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
				}
*/
struct PreBattleStat {
	int enemySize = 0;
	float enemyStrengthFactor = 0.f;
	int selfSize = 0;
	float selfStrengthFactor = 0.f;
	int modifyRow = 0;
	int modifyRoll = 0;

	int finalRow = 0;
	int totalSize = 0;
};

class Army {
friend class Map;
friend class Force;
friend class StupidMonarchAI;
public:
	Army( Force* owner, Province* position, int maxSize, int size );
	~Army();

	void Render();

	bool IsProvValidToGo( Province* provToGo ) const;
	void GoToNextProv();
	void SetSelfDeath();

	Vec2 const GetCenter() const;
	float GetRadius() const;
	Province* GetProvinceIn() const;
	Force* GetOwner() const;
	int GetSize() const;
	int GetMaxSize() const;

	void SetSize( int size );
	void SetProvIn( Province* provNowIn );
	void SetProvToGo( Province* provToGo );
	void NextTurn();
private:
	void PerformSiege();
	void BattleInfluenceOnProvPop( Province* prov );
private:
	Rgba8 m_color;
	Force* m_owner;
	Province* m_inProvince;
	Province* m_provToGoNext = nullptr;
	int m_maxSize = 100;
	int m_size = 1;
};

PreBattleStat const GetPreBattleStat( Army* selfArmy, Army* enemyArmy, Province* prov );