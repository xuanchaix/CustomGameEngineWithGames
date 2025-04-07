#include "Game/GameModes.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerController.hpp"
#include "Game/AIController.hpp"

GameMode::GameMode( Map* map )
	:Actor(map, s_gameModeDef)
{

}

GameMode::~GameMode()
{

}

void GameMode::BeginPlay()
{

}

void GameMode::Update()
{

}

ActorDefinition const GameMode::s_gameModeDef = ActorDefinition();

SurvivalGameMode::SurvivalGameMode( Map* map )
	:GameMode(map)
{
	m_nextWaveCountDownTimer = 5.f;
}

SurvivalGameMode::~SurvivalGameMode()
{

}

void SurvivalGameMode::BeginPlay()
{

}

void SurvivalGameMode::Update()
{
	if (m_curWave == 6) {
		g_theGame->EnterState( GameState::VICTORY );
		g_theGame->m_curMap->m_blockUpdate = true;
		return;
	}
	if (m_playerDeath == 3) {
		g_theGame->EnterState( GameState::VICTORY );
		g_theGame->m_curMap->m_blockUpdate = true;
		for (auto player : g_theGame->m_players) {
			if (player) {
				player->m_isVictory = false;
			}
		}
		return;
	}

	float deltaSeconds = g_theGame->m_gameClock->GetDeltaSeconds();
	if (m_nextWaveCountDownTimer == 0.f) {
		// check if spawn new enemies
		if (m_curNumOfEnemySpawned < m_curWave * 2) {
			if (m_waveTimer <= 0.f) {
				m_curNumOfEnemySpawned++;
				m_waveTimer = 4.f;
				ActorUID spawnPoint = m_map->GetRandomEnemySpawnPoint();
				float rnd = g_theGame->m_randNumGen->RollRandomFloatZeroToOne();
				ActorUID uid;
				if (rnd < 0.6f) {
					uid = g_theGame->m_curMap->SpawnActorToMap( ActorDefinition::GetActorDefinition( "Demon" ), spawnPoint->m_position, spawnPoint->m_orientation );
				}
				else {
					uid = g_theGame->m_curMap->SpawnActorToMap( ActorDefinition::GetActorDefinition( "EnemyMarine" ), spawnPoint->m_position, spawnPoint->m_orientation );
				}
				AIController* thisAIController = g_theGame->CreateNewAIController( uid->m_def.m_aiBehavior );
				thisAIController->Possess( uid );
			}
			else {
				m_waveTimer -= deltaSeconds;
			}
		}
		else {
			// check if all enemies are dead
			std::vector<Actor*> const& allActors = g_theGame->m_curMap->m_actors;
			for (auto actor : allActors) {
				if (actor && actor->m_def.m_faction == ActorFaction::ENEMY) {
					return;
				}
			}
			m_nextWaveCountDownTimer = 5.f;

		}
	}
	else {
		m_nextWaveCountDownTimer -= deltaSeconds;
		if (m_nextWaveCountDownTimer <= 0.f) {
			m_nextWaveCountDownTimer = 0.f;
			StartNewWave();
		}
	}
}

void SurvivalGameMode::StartNewWave()
{
	m_curWave += 1;
	m_curNumOfEnemySpawned = 0;
}

KillAllGameMode::KillAllGameMode( Map* map )
	: GameMode( map )
{

}

KillAllGameMode::~KillAllGameMode()
{

}

void KillAllGameMode::BeginPlay()
{

}

void KillAllGameMode::Update()
{
	std::vector<Actor*> const& allActors = g_theGame->m_curMap->m_actors;
	for (auto actor : allActors) {
		if (actor && actor->m_def.m_faction == ActorFaction::ENEMY) {
			return;
		}
	}
	g_theGame->EnterState( GameState::VICTORY );
	g_theGame->m_curMap->m_blockUpdate = true;
}


KillOpponentGameMode::KillOpponentGameMode( Map* map )
	: GameMode( map )
{

}

KillOpponentGameMode::~KillOpponentGameMode()
{

}

void KillOpponentGameMode::BeginPlay()
{

}

void KillOpponentGameMode::Update()
{
	std::vector<PlayerController*> const& allPlayers = g_theGame->m_players;
	for (auto player : allPlayers) {
		if (player && player->m_numOfDeaths >= 5) {
			player->m_isVictory = false;
			g_theGame->EnterState( GameState::VICTORY );
			g_theGame->m_curMap->m_blockUpdate = true;
		}
	}
}

