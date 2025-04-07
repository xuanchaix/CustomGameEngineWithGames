#include "Game/BattleReport.hpp"
#include "Game/TranslationUtils.hpp"
#include "Game/SM_BitMapFont.hpp"

BattleReport::BattleReport()
{

}

BattleReport::~BattleReport()
{

}

void BattleReport::AddBattleReport( int selfSize, int enemySize, int diceResult, int selfCasualties, int enemyCasualties )
{
	BattleRound br;
	br.m_selfSize = selfSize;
	br.m_enemySize = enemySize;
	br.m_diceResult = diceResult;
	br.m_selfCasualties = selfCasualties;
	br.m_enemyCasualties = enemyCasualties;
	m_battleRounds.push_back( br );
}

Vec2 const BattleReport::GetPosition() const
{
	return m_battlePosition;
}

void BattleReport::Render() const
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;
	verts.reserve( 6 );
	textVerts.reserve( 1000 );
	if (m_type == BattleType::COMBAT) {
		AddVertsForAABB2D( verts, AABB2( Vec2( 300.f, 100.f ), Vec2( 1300.f, 600.f ) ), Rgba8( 160, 160, 160, 128 ), AABB2::IDENTITY );
		AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 250.f, 510.f ), Vec2( 1350.f, 590.f ) ), 30.f, Stringf(
			"$(br_battle_of) $(prov%d)\n$(force%d)                    $(force%d)",
			m_provID, m_attackerForceId, m_defenderForceId ),
			Rgba8( 0, 0, 0 ) );
		AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 250.f, 400.f ), Vec2( 1350.f, 480.f ) ), 30.f, Stringf(
			"$(br_attacker_size):%d                 $(br_defender_size):%d\n$(br_dice_roll_result):%d\n$(br_attacker_casualities):%d       $(br_defender_casualities):%d",
			m_battleRounds[0].m_selfSize, m_battleRounds[0].m_enemySize, m_battleRounds[0].m_diceResult, m_battleRounds[0].m_selfCasualties, m_battleRounds[0].m_enemyCasualties ),
			Rgba8( 0, 0, 0 ) );
		if ((int)m_battleRounds.size() > 1) {
			AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 250.f, 290.f ), Vec2( 1350.f, 370.f ) ), 30.f, Stringf(
				"$(br_attacker_size):%d                 $(br_defender_size):%d\n$(br_dice_roll_result):%d\n$(br_attacker_casualities):%d       $(br_defender_casualities):%d",
				m_battleRounds[1].m_selfSize, m_battleRounds[1].m_enemySize, m_battleRounds[1].m_diceResult, m_battleRounds[1].m_selfCasualties, m_battleRounds[1].m_enemyCasualties ),
				Rgba8( 0, 0, 0 ) );
			if ((int)m_battleRounds.size() > 2) {
				AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 250.f, 180.f ), Vec2( 1350.f, 260.f ) ), 30.f, Stringf(
					"$(br_attacker_size):%d                 $(br_defender_size):%d\n$(br_dice_roll_result):%d\n$(br_attacker_casualities):%d       $(br_defender_casualities):%d",
					m_battleRounds[2].m_selfSize, m_battleRounds[2].m_enemySize, m_battleRounds[2].m_diceResult, m_battleRounds[2].m_selfCasualties, m_battleRounds[2].m_enemyCasualties ),
					Rgba8( 0, 0, 0 ) );
			}
		}
		if (m_battleResult) {
			AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 250.f, 110.f ), Vec2( 1350.f, 170.f ) ), 30.f, Stringf( "$(br_remain_size):%d        $(br_remain_size):%d\n$(br_win)               $(br_lose)", m_selfRemainSize, m_enemyRemainSize ), Rgba8( 0, 0, 0 ) );
		}
		else {
			AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 250.f, 110.f ), Vec2( 1350.f, 170.f ) ), 30.f, Stringf( "$(br_remain_size):%d        $(br_remain_size):%d\n$(br_lose)               $(br_win)", m_selfRemainSize, m_enemyRemainSize ), Rgba8( 0, 0, 0 ) );
		}
	}
	else if (m_type == BattleType::SIEGE) {
		AddVertsForAABB2D( verts, AABB2( Vec2( 300.f, 200.f ), Vec2( 1300.f, 600.f ) ), Rgba8( 160, 160, 160, 128 ), AABB2::IDENTITY );
		AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 250.f, 520.f ), Vec2( 1350.f, 580.f ) ), 30.f, Stringf(
			"$(br_siege_of) $(prov%d)", m_provID ),
			Rgba8( 0, 0, 0 ) );
		AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 250.f, 300.f ), Vec2( 1350.f, 500.f ) ), 30.f, Stringf(
			"$(br_attacker_size):%d               $(br_province_defense):%d\n$(br_dice_roll_result):%d\n$(br_attacker_remain_size):%d  $(br_province_remain_defense):%d",
			m_beginSize, m_beginDefense, m_diceRollRes, m_endSize, m_endDefense),
			Rgba8( 0, 0, 0 ) );
		if (m_isSuccess) {
			AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 250.f, 210.f ), Vec2( 1350.f, 270.f ) ), 30.f, Stringf( "$(br_province_surrendered)" ), Rgba8( 0, 0, 0 ) );
		}
		else {
			AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 250.f, 210.f ), Vec2( 1350.f, 270.f ) ), 30.f, Stringf( "$(br_keep_sieging)" ), Rgba8( 0, 0, 0 ) );
		}
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
	if (g_gameLanguage == SM_GameLanguage::ENGLISH) {
		g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	}
	else if (g_gameLanguage == SM_GameLanguage::ZH) {
		g_theRenderer->BindTexture( &g_chineseFont->GetTexture() );
	}		
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );
}

