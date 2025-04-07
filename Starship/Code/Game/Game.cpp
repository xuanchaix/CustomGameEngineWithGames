#include "Game/Game.hpp"
#include "Game/Asteroid.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Bullet.hpp"
#include "Game/Beetle.hpp"
#include "Game/Wasp.hpp"
#include "Game/Carrier.hpp"
#include "Game/Debris.hpp"
#include "Game/Wave.hpp"
#include "Game/ConeAttack.hpp"
#include "Game/App.hpp"
#include "Game/PowerUp.hpp"
#include "Game/LaserBeam.hpp"
#include "Game/LightSaber.hpp"
#include "Game/BossFirstExplorer.hpp"
#include "Game/Rocket.hpp"
#include "Game/SummonLight.hpp"
#include "Game/BossDestroyerTurret.hpp"
#include "Game/BossDestroyer.hpp"
#include "Game/BossMotherOfCarrier.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/EngineMath.hpp"

/*
enum class UpgradeType {
	fasterAmmoGrow, lessAmmoCost, fasterSpeed, shield, shieldLessTime, powerUpTime, superBulletWeapon, rocketAttack,
	rocketHigherDamage, moreLightSaber, coneAttack, coneAttackLargerRange, NUM, Empty
};
*/

std::string upgradeDescriptionArray[14] = {
	std::string( "Faster Ammo\nRegeneration Speed" ),
	std::string( "Less Ammo Cost per Attack" ),
	std::string( "Faster Ship Speed" ),
	std::string( "Auto Spawn Shield" ),
	std::string( "Faster Shield\nSpawn Speed" ),
	std::string( "Longer Power Up Time" ),
	std::string( "Super Bullet Weapon" ),
	std::string( "Acquire Rocket Attack\nR or XBox Y shoot" ),
	std::string( "Damage of Rocket\nBecome higher" ),
	std::string( "More Light Saber" ),
	std::string( "Acquire Cone Attack\nE or XBox B shoot" ),
	std::string( "Cone Attack Larger Range" ),
	std::string( "" ),
	std::string( "" ),
};

Game::Game()
{
	// load random number generator
	m_randNumGen = new RandomNumberGenerator();
	m_gameClock = new Clock();
	m_gameClock->Pause();

	m_worldCamera.m_mode = CameraMode::Orthographic;
	m_screenCamera.m_mode = CameraMode::Orthographic;
}

Game::~Game()
{
	// delete all pointers managed by Game
	delete m_randNumGen;
	m_randNumGen = nullptr;

	delete m_gameClock;
	m_gameClock = nullptr;

	// delete all entities
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (m_entities[i]) {
			delete m_entities[i];
			m_entities[i] = nullptr;
		}
	}

	// delete all debris
	for (int i = 0; i < MAX_DEBRIS; i++) {
		if (m_debris[i]) {
			delete m_debris[i];
			m_debris[i] = nullptr;
		}
	}

	// delete all levels
	for (int i = 0; i < NUM_OF_LEVELS; i++) {
		if (m_levels[i]) {
			delete m_levels[i];
			m_levels[i] = nullptr;
		}
	}
}

void Game::Startup()
{
	// set up camera
	m_screenCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );
	m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 1.f, -1.f );

	// load levels
	LoadLevel();

	// set up background
	MakeBackgroundStars();

	// set up the player ship
	m_playerShip = new PlayerShip( Vec2( WORLD_CENTER_X, WORLD_CENTER_Y ), this);
	m_entities[0] = m_playerShip; // player ship is in this particular position

	m_upgradeSystem = UpgradeSystem();

}

void Game::Update()
{
	float deltaTime = m_gameClock->GetDeltaSeconds();
	// update upgrade mode after finish a level
	if (m_gameState == GameState::Upgrading && m_waitForUpgradeMode > 0.f) {
		m_waitForUpgradeMode -= deltaTime;
		return;
	}
	if (m_gameState == GameState::Upgrading && m_waitForUpgradeMode <= 0.f) {
		if (!(m_upgradeSystem.m_isDealt)) {
			m_upgradeSystem.DealUpgrade( this );
		}
		XboxController& xboxcontorller = g_theInput->GetController( 0 );
		if (g_theInput->WasKeyJustPressed( 'J' ) || xboxcontorller.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_X )) {
			if (m_upgradeSystem.m_thisUpgrade[0] != UpgradeType::Empty) {
				m_upgradeSystem.Upgrade( m_upgradeSystem.m_thisUpgrade[0] );
				StartNewLevel();
				m_upgradeSystem.m_isDealt = false;
			}
		}
		else if (g_theInput->WasKeyJustPressed( 'K' ) || xboxcontorller.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_A )) {
			if (m_upgradeSystem.m_thisUpgrade[0] != UpgradeType::Empty) {
				m_upgradeSystem.Upgrade( m_upgradeSystem.m_thisUpgrade[1] );
				StartNewLevel();
				m_upgradeSystem.m_isDealt = false;
			}
		}
		else if (g_theInput->WasKeyJustPressed( 'L' ) || xboxcontorller.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_B )) {
			if (m_upgradeSystem.m_thisUpgrade[0] != UpgradeType::Empty) {
				m_upgradeSystem.Upgrade( m_upgradeSystem.m_thisUpgrade[2] );
				StartNewLevel();
				m_upgradeSystem.m_isDealt = false;
			}
		}
		if (m_currentLevel == 4) {
			m_upgradeSystem.m_canUpgrade[7] = UpgradeType::coneAttack;
			m_upgradeSystem.Upgrade( UpgradeType::coneAttack );
		}
		return;
	}

	// update level and wave
	if (m_currentLevel < NUM_OF_LEVELS) {
		m_levels[m_currentLevel]->Update( deltaTime );
		Wave* newWave = m_levels[m_currentLevel]->IsWaveBegin();
		if (newWave) {
			StartNewWave(newWave);
		}
	}
	if (m_curEnemyAmount <= 0 && !m_isBossFight) {
		// pass all levels
		if (m_currentLevel >= NUM_OF_LEVELS) {
			if (m_endGameTimeCount == 0.f) {
				g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::victory ), false, 3.f );
			}
			// cannot start a new level, go to the attract mode
			m_endGameTimeCount += deltaTime;
			if (m_endGameTimeCount > 3.f) {
				g_theApp->ToAttractMode();
			}
		}
		// pass current level
		else if (m_levels[m_currentLevel]->IsWaveFinished()) {
			// first level no upgrade(instead give light saber)
			if (m_currentLevel == 0) {
				m_playerShip->m_allowLighrSaber = true;
				//m_waitForUpgradeMode = 0.1f;
				//CreateSaber( Vec2( 0, 0 ), nullptr, 0.f, 0.f );
				StartNewLevel();
			}
			else {
				if (m_currentLevel < NUM_OF_LEVELS - 1) {
					m_waitForUpgradeMode = 1.5f;
					m_gameState = GameState::Upgrading;
					if (m_playerShip->m_isLightSaberStart) {
						CreateSaber( Vec2( 0, 0 ), nullptr, 0.f, 0.f );
						m_playerShip->m_isLightSaberStart = false;
					}
				}
				else {
					m_currentLevel++;
				}
			}
			//StartNewLevel();
		}
	}

	// deal with collision
	DealCollision();

	// update all entities
	if (deltaTime != 0.f) {
		UpdateEntityArray( m_entities, MAX_ENTITIES );
		UpdateEntityArray( m_debris, MAX_DEBRIS );
	}

#ifdef DEBUG_MODE
	// spawn asteroid when I pressed
	if (g_theInput->WasKeyJustPressed( 0x49 )) {
		SpawnNewEntity( new Asteroid( MakeRandomOffMapPosition( ASTEROID_COSMETIC_RADIUS ), this ) );
	}

	// kill all(except player ship)
	if (g_theInput->WasKeyJustPressed( 'M' )) {
		for (int i = 0; i < MAX_ENTITIES; i++) {
			if (m_entities[i] && m_entities[i]->IsAlive() && m_entities[i]->m_type != EntityType::playerShip) {
				m_entities[i]->BeAttacked( 100000 );
			}
		}
	}

	if (g_theInput->WasKeyJustPressed( 0x4F )) {// O key run a single frame and pauses
		m_gameClock->StepSingleFrame();
	}

	if (g_theInput->WasKeyJustPressed( 0x54 )) // T key slow the game
	{
		m_gameClock->SetTimeScale( 0.1f );
	}
	if (g_theInput->WasKeyJustReleased( 0x54 )) // T key slow the game
	{
		m_gameClock->SetTimeScale( 1.f );
	}

	if (g_theInput->WasKeyJustPressed( 0x50 )) // P key pause the game; handle the pause problem
	{
		m_gameClock->TogglePause();
	}
#endif // DEBUG_MODE
	if (g_theInput->WasKeyJustPressed( 'N' ) && m_currentLevel == 0) {
		m_levels[m_currentLevel]->m_timeElapsed = 65.f;
		m_playerShip->m_ammoAmount = m_playerShip->m_maxAmmo;
	}
#ifdef DEBUG_MODE
	else if (g_theInput->WasKeyJustPressed( 'N' )) {
		m_levels[m_currentLevel]->m_timeElapsed += 10.f;
	}
#endif // DEBUG_MODE
	if (g_theInput->WasKeyJustPressed( 'P' )) {
		if (m_gameClock->IsPaused()) {
			m_showKeyTutorial = true;
		}
		else{
			m_showKeyTutorial = false;
		}
	}
	// reborn the player ship if N pressed and player ship is dead
	// not in use now because player ship will not disappear from the screen now
/*if ((g_theInput->WasKeyJustPressed(0x4E)
	|| g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_START ))
	&& m_playerShip && !(m_playerShip->IsAlive())) {
	if (m_currentLife > 0) {
		m_playerShip->m_isDead = false;
		m_playerShip->m_position = Vec2( WORLD_CENTER_X, WORLD_CENTER_Y );
		m_playerShip->m_orientationDegrees = 0.f;
		m_playerShip->m_velocity = Vec2( 0.f, 0.f );
		m_playerShip->m_accelerateVelocity = Vec2( 0.f, 0.f );
		m_screenShakeCountSeconds = SCREEN_SHAKE_SECONDS;
		m_playerShip->m_health = PLAYER_SHIP_HEALTH;
		m_currentLife--;
		g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::playerReborn ) );
	}
}*/

	// delete garbage
	DeleteGarbageInEntityArray( m_entities, MAX_ENTITIES );
	DeleteGarbageInEntityArray( m_debris, MAX_DEBRIS );

	// if player has no life, go to attract mode
	if (!m_playerShip->IsAlive()) {
		if (m_endGameTimeCount == 0.f) {
			g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::defeat ), false, 3.f );
		}
		m_endGameTimeCount += deltaTime;
		if (m_endGameTimeCount > 3.f) {
			g_theAudio->StopSound( g_theApp->m_bossMusic );
			g_theApp->ToAttractMode();
		}
	}

	// reset the camera aspect
	m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 1.f, -1.f );
	// screen shake when player ship is died
	if (m_screenShakeCountSeconds > 0.f) {
		m_screenShakeCountSeconds -= deltaTime;
		m_worldCamera.Translate2D( Vec2( m_randNumGen->RollRandomFloatInRange( -4.f, 4.f ), m_randNumGen->RollRandomFloatInRange( -4.f, 4.f ) ) );
	}

	// add time to rendering text
	m_timeElapsedRenderingText += deltaTime;

	if (IsUpgraded( UpgradeType::powerUpTime )) {
		m_powerUpTime = 20.f;
	}

	if (IsUpgraded( UpgradeType::moreLightSaber )) {
		m_numOfSabers = 3;
	}
}

void Game::Render() const
{
	// do not render game in upgrade mode
	if (!(m_gameState == GameState::Upgrading && m_upgradeSystem.m_isDealt)) {
		// game camera
		g_theRenderer->BeginCamera( m_worldCamera );
		// render background stars
		RenderBackground();
		// render all the entities
		RenderEntityArray( m_entities, MAX_ENTITIES );
		RenderEntityArray( m_debris, MAX_DEBRIS );
		g_theRenderer->EndCamera( m_worldCamera );
	}

	// UI camera
	g_theRenderer->BeginCamera( m_screenCamera );
	RenderUI();
	g_theRenderer->EndCamera( m_screenCamera );
}

Bullet* Game::CreateBullet( Vec2 startPos, Vec2 velocity, float orientationDegrees, bool isEnemyTrigger )
{
	Bullet* newBullet = new Bullet( startPos, this, velocity, orientationDegrees, isEnemyTrigger );
	if (SpawnNewEntity( newBullet )) {
		return newBullet;
	}
	return nullptr;
}

void Game::CreateCone( Vec2 startPos, float orientationDegrees, bool isEnemyTrigger, Entity const* ship, float radius, float apertureDegrees )
{
	ConeAttack* newCone = new ConeAttack( startPos, this, orientationDegrees, ship, isEnemyTrigger, radius, apertureDegrees );
	if (SpawnNewDebris( newCone )) {
		if (!isEnemyTrigger) { // player cone damage deal
			for (int i = 0; i < MAX_ENTITIES; i++) {
				if (m_entities[i]
					&& m_entities[i]->IsAlive()
					&& m_entities[i]->m_type != EntityType::playerShip
					&& m_entities[i]->m_type != EntityType::entity
					&& m_entities[i]->m_type != EntityType::bullet
					&& m_entities[i]->m_type != EntityType::powerUp
					&& DoDiscOverlapOrientedSector2D(m_entities[i]->m_position, m_entities[i]->m_physicsRadius, 
						startPos, orientationDegrees, apertureDegrees, radius )) {
					if (m_entities[i]->m_type != EntityType::boss) {
						m_entities[i]->BeAttacked( 1 );
					}
					else { // higher damage to boss for balance consideration
						m_entities[i]->BeAttacked( 3 );
					}
				}
			}
		}
		else { // enemy cone damage deal
			for (int i = 0; i < MAX_ENTITIES; i++) {
				if (m_entities[i]
					&& m_entities[i]->IsAlive()
					&& (m_entities[i]->m_type == EntityType::playerShip
					|| m_entities[i]->m_type == EntityType::bullet
					|| m_entities[i]->m_type == EntityType::asteroid)
					&& DoDiscOverlapOrientedSector2D( m_entities[i]->m_position, m_entities[i]->m_physicsRadius,
						startPos, orientationDegrees, apertureDegrees, radius )) {
					m_entities[i]->BeAttacked( 1 );
				}
			}
		}
	}
}

void Game::CreateLaser( Vec2 startPos, float orientationDegrees, bool isEnemyTrigger, Entity const* ship, float range )
{
	LaserBeam* newLaser = new LaserBeam( startPos, this, orientationDegrees, ship, isEnemyTrigger, range );
	SpawnNewDebris( newLaser );
}

void Game::CreateSaber( Vec2 startPos, Entity const* ship, float length, float cost )
{
	// create 3 or 5 sabers
	// if sabers exist, call this function to delete them
	if (m_playerSaberIndex[0] != -1) {
		for (int i = 0; i < m_numOfSabers; i++) {
			delete m_debris[m_playerSaberIndex[i]];
			m_debris[m_playerSaberIndex[i]] = nullptr;
			m_playerSaberIndex[i] = -1;
		}
		return;
	}
	else if (!m_playerShip->IsAlive()) {
		return;
	}
	else if (m_playerShip->m_ammoAmount < cost) {
		return;
	}
	else if (m_waitForUpgradeMode > 0.f) {
		return;
	}
	else { // create new sabers
		int numAllocated = 0;
		for (int i = 0; i < MAX_DEBRIS; i++) {
			if (!m_debris[i]) {
				m_debris[i] = new LightSaber( startPos, this, (360.f / (float)m_numOfSabers) * (float)numAllocated, ship, length );
				m_playerSaberIndex[numAllocated] = i;
				numAllocated++;
				if (numAllocated == m_numOfSabers) {
					break;
				}
			}
		}
		if (numAllocated != m_numOfSabers) {
			ERROR_RECOVERABLE( "Cannot spawn a new entity; all slots are full!" );
		}
		g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::lightSaberOn ), false, 2.f );
	}
}

void Game::CreateRocket( Vec2 startPos, Vec2 velocity, float orientationDegrees, bool isEnemyTrigger, float range )
{
	Rocket* newRocket = new Rocket( startPos, this, velocity, orientationDegrees, range, isEnemyTrigger );
	SpawnNewEntity( newRocket );
}

void Game::SpawnDebris( Entity const* deadEntity )
{
	if (deadEntity->m_type == EntityType::playerShip) {
		// 20-30 debris
		int num = m_randNumGen->RollRandomIntInRange( 10, 20 );
		for (int i = 0; i < num; i++) {
			if (!SpawnNewDebris( new Debris( deadEntity->m_position, this, deadEntity->m_velocity, deadEntity->m_cosmeticRadius, deadEntity->m_color ) )) {
				return;
			}
		}
	}
	else if (deadEntity->m_type == EntityType::bullet || deadEntity->m_type == EntityType::enemyBullet || deadEntity->m_type == EntityType::fighter) {
		// 3-5 debris
		int num = m_randNumGen->RollRandomIntInRange( 3, 5 );
		for (int i = 0; i < num; i++) {
			if (!SpawnNewDebris( new Debris( deadEntity->m_position, this, -deadEntity->m_velocity * 0.5f, deadEntity->m_cosmeticRadius * 0.6f, deadEntity->m_color ) )) {
				return;
			}
		}
	}
	else if (deadEntity->m_type == EntityType::boss) {
		for (int i = 0; i < 100; i++) {
			if (!SpawnNewDebris( new Debris( deadEntity->m_position, this, deadEntity->m_velocity, deadEntity->m_cosmeticRadius, deadEntity->m_color ) )) {
				return;
			}
		}
	}
	else {
		// 10-15 debris
		int num = m_randNumGen->RollRandomIntInRange( 10, 15 );
		for (int i = 0; i < num; i++) {
			if (!SpawnNewDebris( new Debris( deadEntity->m_position, this, deadEntity->m_velocity, deadEntity->m_cosmeticRadius, deadEntity->m_color ) )) {
				return;
			}
		}
	}
}

void Game::SpawnRocketDebris( Rocket const* rocket )
{
	for (int i = 0; i < 50; i++) {
		if (!SpawnNewDebris( new Debris( rocket->m_position, this, Vec2( 0, 0 ), rocket->m_cosmeticRadius * 0.6f, rocket->m_color ) )) {
			return;
		}
	}
}

void Game::SpawnPowerUp( Entity const* deadEntity )
{
	// don't spawn power up in first level
	if (m_currentLevel == 0) {
		return;
	}
	float rndNum = m_randNumGen->RollRandomFloatZeroToOne();
	if (rndNum < 0.1f) {
		PowerUp* newPowerUp = new PowerUp( deadEntity->m_position, this, PowerUpType::enhance, deadEntity );
		SpawnNewEntity( newPowerUp );
	}
	else if (rndNum < 0.15f) {
		m_healthTutorial = true;
		PowerUp* newPowerUp = new PowerUp( deadEntity->m_position, this, PowerUpType::health, deadEntity );
		SpawnNewEntity( newPowerUp );
	}
	else if (rndNum < 0.2f) {
		PowerUp* newPowerUp = new PowerUp( deadEntity->m_position, this, PowerUpType::ammo, deadEntity );
		SpawnNewEntity( newPowerUp );
	}
}

void Game::SpawnBossDestroyer()
{
	if (m_entities[MAX_ENTITIES - 1]) {
		bool isAllocated = false;
		for (int i = 0; i < MAX_ENTITIES - 1; i++) {
			if (!m_entities[i]) {
				m_entities[i] = m_entities[MAX_ENTITIES - 1];
				m_entities[MAX_ENTITIES - 1] = new BossDestroyer( Vec2( 300, 50 ), this );
				m_curEnemyAmount++;
				isAllocated = true;
				break;
			}
		}
		if (!isAllocated) {
			ERROR_RECOVERABLE( "Cannot spawn a new boss; all slots are full!" );
		}
	}
	else {
		m_entities[MAX_ENTITIES - 1] = new BossDestroyer( Vec2( 300, 50 ), this );
	}
}

Entity** Game::BossDestroyerSpawnEnemy()
{
	Entity** entityArray = new Entity* [DESTROYER_ENEMY_AMOUNT];
	for (int i = 0; i < DESTROYER_ENEMY_AMOUNT; i++) {
		float rnd = m_randNumGen->RollRandomFloatZeroToOne();
		if (rnd < 0.5f) {
			entityArray[i] = SpawnNewEntity( new Beetle( MakeRandomOffMapPosition( BEETLE_COSMETIC_RADIUS ), this, m_playerShip ) );
		}
		else {
			entityArray[i] = SpawnNewEntity( new Wasp( MakeRandomOffMapPosition( WASP_COSMETIC_RADIUS ), this, m_playerShip ) );
		}
	}
	return entityArray;
}

BossDestroyerTurret** Game::BossDestroyerSpawnTurret( Entity* boss )
{
	BossDestroyerTurret** turretArray = new BossDestroyerTurret* [DESTROYER_TURRET_AMOUNT];
	turretArray[0] = (BossDestroyerTurret*)SpawnNewEntity( new BossDestroyerTurret( Vec2( boss->m_position.x - 55, boss->m_position.y + 5 ), this, m_playerShip ) );
	turretArray[1] = (BossDestroyerTurret*)SpawnNewEntity( new BossDestroyerTurret( Vec2( boss->m_position.x - 45, boss->m_position.y + 10 ), this, m_playerShip ) );
	turretArray[2] = (BossDestroyerTurret*)SpawnNewEntity( new BossDestroyerTurret( Vec2( boss->m_position.x - 35, boss->m_position.y + 15 ), this, m_playerShip ) );
	turretArray[3] = (BossDestroyerTurret*)SpawnNewEntity( new BossDestroyerTurret( Vec2( boss->m_position.x - 25, boss->m_position.y + 20 ), this, m_playerShip ) );
	turretArray[4] = (BossDestroyerTurret*)SpawnNewEntity( new BossDestroyerTurret( Vec2( boss->m_position.x - 55, boss->m_position.y - 5 ), this, m_playerShip ) );
	turretArray[5] = (BossDestroyerTurret*)SpawnNewEntity( new BossDestroyerTurret( Vec2( boss->m_position.x - 45, boss->m_position.y - 10 ), this, m_playerShip ) );
	turretArray[6] = (BossDestroyerTurret*)SpawnNewEntity( new BossDestroyerTurret( Vec2( boss->m_position.x - 35, boss->m_position.y - 15 ), this, m_playerShip ) );
	turretArray[7] = (BossDestroyerTurret*)SpawnNewEntity( new BossDestroyerTurret( Vec2( boss->m_position.x - 25, boss->m_position.y - 20 ), this, m_playerShip ) );
	return turretArray;
}

void Game::BossDestroyerSpawnBeetle()
{
	SpawnNewEntity( new Beetle( MakeRandomOffMapPosition( BEETLE_COSMETIC_RADIUS ), this, m_playerShip ) );
	SpawnNewEntity( new Beetle( MakeRandomOffMapPosition( BEETLE_COSMETIC_RADIUS ), this, m_playerShip ) );
	SpawnNewEntity( new Beetle( MakeRandomOffMapPosition( BEETLE_COSMETIC_RADIUS ), this, m_playerShip ) );
}

void Game::BossDestroyerShootLaser( BossDestroyer* boss )
{
	g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::laser ) );
	m_screenShakeCountSeconds = SCREEN_SHAKE_SECONDS;
	CreateLaser( Vec2( 300, boss->m_nextLaserY1 ), 180.f, true, boss, 6.f );
	CreateLaser( Vec2( 300, boss->m_nextLaserY2 ), 180.f, true, boss, 6.f );
}


void Game::DealLaserDamage( LaserBeam const* laser )
{
	/*for (int i = 0; i < MAX_ENTITIES; i++) {
		if (m_entities[i]
			&& m_entities[i]->IsAlive()
			&& m_entities[i]->m_type != EntityType::playerShip
			&& m_entities[i]->m_type != EntityType::entity
			&& m_entities[i]->m_type != EntityType::bullet
			&& m_entities[i]->m_type != EntityType::powerUp
			) {
			// put point in local AABB position
			AABB2 laserAABB2( Vec2( 0.f, -laser->m_range - 3.f ), Vec2( 300.f, laser->m_range + 3.f ) );
			Vec2 entityTransformedPos = m_entities[i]->m_position - laser->m_position;
			entityTransformedPos.RotateDegrees( -laser->m_orientationDegrees );
			if (laserAABB2.IsPointInside( entityTransformedPos )) {
				m_entities[i]->BeAttacked( 1 );
			}
		}
	}*/
	AABB2 laserAABB2( Vec2( 0.f, -laser->m_range ), Vec2( 300.f, laser->m_range ) );
	Vec2 entityTransformedPos = m_playerShip->m_position - laser->m_position;
	entityTransformedPos.RotateDegrees( -laser->m_orientationDegrees );
	if (DoDiscOverlapAABB2D( entityTransformedPos, m_playerShip->m_physicsRadius, laserAABB2 )) {
		m_playerShip->BeAttacked( 1 );
	}
}

void Game::DealSaberDamage( LightSaber const* saber )
{
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (m_entities[i]
			&& m_entities[i]->IsAlive()
			&& m_entities[i]->m_type != EntityType::playerShip
			&& m_entities[i]->m_type != EntityType::entity
			&& m_entities[i]->m_type != EntityType::bullet
			&& m_entities[i]->m_type != EntityType::powerUp
			&& m_entities[i]->m_type != EntityType::boss
			) {
			// put point in local AABB position
			AABB2 laserAABB2( Vec2( 0.f, -1.f ), Vec2( saber->m_length + 3.f, 1.f ) );
			Vec2 entityTransformedPos = m_entities[i]->m_position - saber->m_position;
			entityTransformedPos.RotateDegrees( -saber->m_orientationDegrees );
			if (DoDiscOverlapAABB2D( entityTransformedPos, m_entities[i]->m_physicsRadius, laserAABB2 )) {
				m_entities[i]->BeAttacked( 2 );
			}
		}
	}
}

void Game::DealRocketDamage( Vec2 const& rocketPos, float range )
{
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (m_entities[i] && m_entities[i]->IsAlive()
			&& (m_entities[i]->m_type == EntityType::asteroid
				|| m_entities[i]->m_type == EntityType::wasp
				|| m_entities[i]->m_type == EntityType::beetle
				|| m_entities[i]->m_type == EntityType::boss
				|| m_entities[i]->m_type == EntityType::bossTurret
				|| m_entities[i]->m_type == EntityType::carrier
				|| m_entities[i]->m_type == EntityType::fighter)
			&& GetDistanceSquared2D( m_entities[i]->m_position, rocketPos ) < range * range
			) {
			if (IsUpgraded( UpgradeType::rocketLargerRange )) {
				m_entities[i]->BeAttacked( 2 );
			}
			else {
				m_entities[i]->BeAttacked( 1 );
			}
		}
	}
}

float Game::GetDeltaSeconds() const
{
	return m_gameClock->GetDeltaSeconds();
}

void Game::ExplorerSpawnNewEnemy( BossFirstExplorer const* explorer )
{
	Vec2 pos = Vec2( m_randNumGen->RollRandomFloatInRange( 0.f, 200.f ), m_randNumGen->RollRandomFloatInRange( 0.f, 100.f ) );
	float randNum = m_randNumGen->RollRandomFloatZeroToOne();
	if (randNum < 0.35f) {
		Entity* wasp = new Wasp( pos, this, m_playerShip );
		Entity* summonLight = new SummonLight( explorer->m_position, this, pos, GetDistance2D( pos, explorer->m_position ) / 80.f, wasp, Rgba8( 102, 178, 255, 150 ) );
		SpawnNewDebris( summonLight );
	}
	else if (randNum < 0.7f) {
		Entity* beetle = new Beetle( pos, this, m_playerShip );
		Entity* summonLight = new SummonLight( explorer->m_position, this, pos, GetDistance2D( pos, explorer->m_position ) / 80.f, beetle, Rgba8( 255, 255, 153, 150 ) );
		SpawnNewDebris( summonLight );
	}
	else {
		Entity* asteroid = new Asteroid( pos, this );
		SpawnNewEntity( asteroid );
		SpawnDebris( asteroid );
	}
}

void Game::ExplorerHealSelf( BossFirstExplorer* explorer, float range )
{
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (m_entities[i] && m_entities[i]->IsAlive() 
			&&(m_entities[i]->m_type == EntityType::asteroid
			|| m_entities[i]->m_type == EntityType::wasp
			|| m_entities[i]->m_type == EntityType::beetle)
			&& GetDistanceSquared2D( m_entities[i]->m_position, explorer->m_position ) < range * range
			) {
			m_entities[i]->BeAttacked( 1 );
			explorer->BeAttacked( -1 );
		}
	}
}

Vec2* Game::GetEnemiesPosInRange( Vec2 const& pos, float range )
{
	Vec2* posArray = new Vec2[100];
	for (int i = 0; i < 100; i++) {
		posArray[i] = Vec2( -1, -1 );
	}
	int j = 0;
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (m_entities[i] && m_entities[i]->IsAlive()
			&& (m_entities[i]->m_type == EntityType::asteroid
				|| m_entities[i]->m_type == EntityType::wasp
				|| m_entities[i]->m_type == EntityType::beetle)
			&& GetDistanceSquared2D( m_entities[i]->m_position, pos ) < range * range
			) {
			posArray[j] = m_entities[i]->m_position;
			j++;
			if (j >= 100) {
				break;
			}
		}
	}
	return posArray;
}

Entity* Game::SpawnNewEntity( Entity* entity )
{
	bool isAllocated = false;
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (!m_entities[i]) {
			m_entities[i] = entity;
			isAllocated = true;
			if (entity->IsEnemy()) {
				m_curEnemyAmount++;
			}
			break;
		}
	}
	if (!isAllocated) {
		ERROR_RECOVERABLE( "Cannot spawn a new entity; all slots are full!" );
		delete entity;
		return nullptr;
	}
	return entity;
}

bool Game::SpawnNewDebris( Entity* entity )
{
	bool isAllocated = false;
	for (int i = 0; i < MAX_DEBRIS; i++) {
		if (!m_debris[i]) {
			m_debris[i] = entity;
			isAllocated = true;
			break;
		}
	}
	if (!isAllocated) {
		ERROR_RECOVERABLE( "Cannot spawn a new debris; all slots are full!" );
		delete entity;
		return false;
	}
	return true;
}

bool Game::Command_SetTimeScale( EventArgs& args )
{
	if (g_theGame) {
		g_theGame->m_gameClock->SetTimeScale( args.GetValue( "scale", 1.f ) );
	}
	else {
		EventArgs newArgs;
		newArgs.SetValue( "EchoArg", "Cannot set the time scale of the game because game is not start!" );
		FireEvent( "Command_Echo", newArgs );
	}
	return true;
}

void Game::UpdateEntityArray( Entity** entityArray, int num )
{
	for (int i = 0; i < num; i++) {
		if (entityArray[i]) {
			entityArray[i]->Update();
		}
	}
}

void Game::RenderEntityArray( Entity* const* entityArray, int num ) const
{
	for (int i = num - 1; i >= 0; i--) {
		if (entityArray[i] && entityArray[i]->IsAlive()) {
			entityArray[i]->Render();
			// render in the debug mode
			if (g_theApp->IsDebugMode() && entityArray[i]->m_type != EntityType::debris) {
				entityArray[i]->DebugRender();
				//render gray lines from player ship to other entities
				if (m_playerShip->IsAlive() && entityArray[i]->m_type != EntityType::playerShip) {
					DebugDrawLine( m_playerShip->m_position, entityArray[i]->m_position, 0.2f, Rgba8( 50, 50, 50, 255 ) );
				}
			}
			entityArray[i]->RenderUI();
		}
	}
}

void Game::DeleteGarbageInEntityArray( Entity** entityArray, int num )
{
	for (int i = 0; i < num; i++) {
		if (entityArray[i] && entityArray[i]->m_isGarbage) {
			if (entityArray[i]->IsEnemy()) {
				m_curEnemyAmount--;
			}
			if(  !(entityArray[i]->IsOffscreen())
				&&(entityArray[i]->m_type == EntityType::asteroid
				|| entityArray[i]->m_type == EntityType::beetle
				|| entityArray[i]->m_type == EntityType::wasp
				|| entityArray[i]->m_type == EntityType::carrier
				)) {
				SpawnPowerUp( entityArray[i] );
			}
			if (entityArray[i]->m_type == EntityType::boss) {
				g_theAudio->StopSound( g_theApp->m_bossMusic );
				g_theApp->m_gameModeMusic = g_theAudio->StartSound( g_theApp->m_audioDictionary[(int)AudioName::battle], true );
				m_isBossFight = false;
			}
			delete entityArray[i];
			entityArray[i] = 0;
		}
	}
}

void Game::RenderUI() const
{
	// initialize text verts
	std::vector<Vertex_PCU> textVerts;
	textVerts.reserve( 1000 );

	// render the rest lives
	for (int i = 0; i < m_playerShip->m_health; i++) {
		Vertex_PCU tempShipVerts[NUM_OF_PLAYER_SHIP_VERTS];
		PlayerShip::GetShipVerts( tempShipVerts );
		TransformVertexArrayXY3D( 15, tempShipVerts, 7.2f, 0.f, Vec2( 32.f + 48.f * (float)i, 768.f ) );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( 15, tempShipVerts );
	}

	// render the rest ammo
	Vertex_PCU bulletVerts[6] = {
		Vertex_PCU( Vec3( 0.f, 0.5f, 0.f ), Rgba8( 255, 255, 0, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.5f, 0.f, 0.f ), Rgba8( 255, 255, 0, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -0.5f, 0.f ), Rgba8( 255, 255, 0, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, 0.5f, 0.f ), Rgba8( 255, 0, 0, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -0.5f, 0.f ), Rgba8( 255, 0, 0, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f, 0.f, 0.f ), Rgba8( 255, 0, 0, 0 ), Vec2( 0.f, 0.f ) ),
	};
	TransformVertexArrayXY3D( 6, bulletVerts, 30.f, 90.f, Vec2( 1430.f, 760.f ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 6, bulletVerts );
	//g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 1340.f, 726.f ), 40.f, "AMMO", Rgba8( 51, 255, 255, 255 ) );
	//g_theRenderer->DrawVertexArray( (int)textVerts.size(), textVerts.data() );
	float remainAmmoRatio = m_playerShip->m_ammoAmount / m_playerShip->m_maxAmmo;
	if (remainAmmoRatio < 0.f) {
		remainAmmoRatio = 0.f;
	}
	Vertex_PCU ammoBarVerts[12];
	ammoBarVerts[0] = Vertex_PCU( Vec3( -1.f, -0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	ammoBarVerts[1] = Vertex_PCU( Vec3( 1.f, -0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	ammoBarVerts[2] = Vertex_PCU( Vec3( -1.f, 0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	ammoBarVerts[3] = Vertex_PCU( Vec3( 1.f, -0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	ammoBarVerts[4] = Vertex_PCU( Vec3( -1.f, 0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	ammoBarVerts[5] = Vertex_PCU( Vec3( 1.f, 0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	ammoBarVerts[6] = Vertex_PCU( Vec3( -1.f, -0.2f, 0 ), Rgba8( 255, 255, 0, 255 ), Vec2( 0, 0 ) );
	ammoBarVerts[7] = Vertex_PCU( Vec3( 2 * remainAmmoRatio - 1, -0.2f, 0 ), Rgba8( 255, 255, 0, 255 ), Vec2( 0, 0 ) );
	ammoBarVerts[8] = Vertex_PCU( Vec3( -1.f, 0.2f, 0 ), Rgba8( 255, 255, 0, 255 ), Vec2( 0, 0 ) );
	ammoBarVerts[9] = Vertex_PCU( Vec3( 2 * remainAmmoRatio - 1, -0.2f, 0 ), Rgba8( 255, 255, 0, 255 ), Vec2( 0, 0 ) );
	ammoBarVerts[10] = Vertex_PCU( Vec3( -1.f, 0.2f, 0 ), Rgba8( 255, 255, 0, 255 ), Vec2( 0, 0 ) );
	ammoBarVerts[11] = Vertex_PCU( Vec3( 2 * remainAmmoRatio - 1, 0.2f, 0 ), Rgba8( 255, 255, 0, 255 ), Vec2( 0, 0 ) );
	TransformVertexArrayXY3D( 12, ammoBarVerts, 40.f, 0.f, Vec2( 1500.f, 750.f ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 12, ammoBarVerts );

	// render boss remain health
	if (m_isBossFight) {
		float ratio = GetCurBossHealthRatio();
		std::vector<Vertex_PCU> healthVerts;
		healthVerts.reserve( 12 );
		AddVertsForAABB2D( healthVerts, AABB2( Vec2( 30.f, 100.f ), Vec2( 50.f, 700.f ) ), Rgba8::RED );
		AddVertsForAABB2D( healthVerts, AABB2( Vec2( 30.f, 100.f ), Vec2( 50.f, 100.f + 600.f * ratio ) ), Rgba8( 50, 200, 50 ) );

		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( healthVerts );
	}

	// render control
	if (m_currentLevel == 0) {
		if (m_showKeyTutorial) {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 600.f, 30.f ), Vec2( 1000.f, 60.f ) ), 30.f, "PRESS P TO CONTINUE", Rgba8( 192, 192, 192 ) );
		}
		else {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 600.f, 30.f ), Vec2( 1000.f, 60.f ) ), 30.f, "PRESS P TO OPEN MANUAL", Rgba8( 192, 192, 192 ) );
			g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 1050.f, 30.f ), 20.f, "PRESS N TO SKIP TUTORIAL", Rgba8( 192, 192, 192, 255 ) );
		}
	}

	// render content on top
	if (m_isRenderingText) {
		int numOfRendering = int(m_timeElapsedRenderingText / ((m_renderingText->m_lifespanTime - 4.f) / (float)m_renderingText->m_content.size()));
		//float length = m_renderingText->m_content.substr( 0, m_renderingText->m_content.find_first_of( '\n' ) ).length() * 8.f;
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 400.f, 700.f ), Vec2( 1200.f, 780.f ) ), 20.f, m_renderingText->m_content, m_renderingText->m_color, 0.618f, Vec2( 0.5f, 1.f ), TextBoxMode::OVERRUN, numOfRendering );
	}

	// render update mode
	if (m_gameState == GameState::Upgrading && m_upgradeSystem.m_isDealt) {
		std::vector<Vertex_PCU> AABB2Vert;
		AABB2Vert.reserve( 6 );
		AddVertsForAABB2D( AABB2Vert, AABB2( Vec2( 200.f, 200.f ), Vec2( 1400.f, 600.f ) ), Rgba8( 255, 255, 153, 255 ), AABB2::IDENTITY );

		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( AABB2Vert );
		Rgba8 color = Rgba8( 64, 64, 64, 255 );
		std::string upgradeString1 = upgradeDescriptionArray[(int)m_upgradeSystem.m_thisUpgrade[0]];
		std::string upgradeString2 = upgradeDescriptionArray[(int)m_upgradeSystem.m_thisUpgrade[1]];
		std::string upgradeString3 = upgradeDescriptionArray[(int)m_upgradeSystem.m_thisUpgrade[2]];
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 250.f, 350.f ), Vec2( 550.f, 500.f ) ), 30.f, upgradeString1, color);
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 650.f, 350.f ), Vec2( 950.f, 500.f ) ), 30.f, upgradeString2, color);
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1050.f, 350.f ), Vec2( 1350.f, 500.f ) ), 30.f, upgradeString3, color);

		if (m_playerShip->m_isControllerPlugIn) {
			DrawXBoxButton( Vec2( 400.f, 300.f ), 30.f, XboxButtonID::XBOX_BUTTON_X, Rgba8::WHITE );
			DrawXBoxButton( Vec2( 800.f, 300.f ), 30.f, XboxButtonID::XBOX_BUTTON_A, Rgba8::WHITE );
			DrawXBoxButton( Vec2( 1200.f, 300.f ), 30.f, XboxButtonID::XBOX_BUTTON_B, Rgba8::WHITE );
		}
		else {
			DrawKeyBoardButtom( AABB2( Vec2( 375.f, 275.f ), Vec2( 425.f, 325.f ) ), 'J', Rgba8( 0, 0, 0 ) );
			DrawKeyBoardButtom( AABB2( Vec2( 775.f, 275.f ), Vec2( 825.f, 325.f ) ), 'K', Rgba8( 0, 0, 0 ) );
			DrawKeyBoardButtom( AABB2( Vec2( 1175.f, 275.f ), Vec2( 1225.f, 325.f ) ), 'L', Rgba8( 0, 0, 0 ) );
		}
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 400.f, 600.f ), Vec2( 1200.f, 700.f ) ), 60.f, "CHOOSE ONE TO UPGRADE", Rgba8( 200, 150, 50, 255 ) );
	}

	//render super weapon cool down
	if (m_playerShip->m_superBulletWeapon) {
		Vertex_PCU localWeaponVerts[18];
		localWeaponVerts[0] = Vertex_PCU( Vec2( -1.1f, -0.1f ), Rgba8( 255, 102, 102 ) );
		localWeaponVerts[1] = Vertex_PCU( Vec2( -1.1f, 2.1f ), Rgba8( 255, 102, 102 ) );
		localWeaponVerts[2] = Vertex_PCU( Vec2( 1.1f, -0.1f ), Rgba8( 255, 102, 102 ) );
		localWeaponVerts[3] = Vertex_PCU( Vec2( 1.1f, -0.1f ), Rgba8( 255, 102, 102 ) );
		localWeaponVerts[4] = Vertex_PCU( Vec2( -1.1f, 2.1f ), Rgba8( 255, 102, 102 ) );
		localWeaponVerts[5] = Vertex_PCU( Vec2( 1.1f, 2.1f ), Rgba8( 255, 102, 102 ) );

		localWeaponVerts[6] = Vertex_PCU( Vec2( -1.f, 0.f ), Rgba8( 0, 0, 0 ) );
		localWeaponVerts[7] = Vertex_PCU( Vec2( -1.f, 2.f ), Rgba8( 0, 0, 0 ) );
		localWeaponVerts[8] = Vertex_PCU( Vec2( 1.f, 0.f ), Rgba8( 0, 0, 0 ) );
		localWeaponVerts[9] = Vertex_PCU( Vec2( 1.f, 0.f ), Rgba8( 0, 0, 0 ) );
		localWeaponVerts[10] = Vertex_PCU( Vec2( -1.f, 2.f ), Rgba8( 0, 0, 0 ) );
		localWeaponVerts[11] = Vertex_PCU( Vec2( 1.f, 2.f ), Rgba8( 0, 0, 0 ) );

		localWeaponVerts[12] = Vertex_PCU( Vec2( -1.f, 0.f ), Rgba8( 160, 160, 160, 100 ) );
		localWeaponVerts[13] = Vertex_PCU( Vec2( -1.f, 2.f * m_playerShip->m_superWeaponTimer / m_playerShip->m_superWeaponCoolDownTime ), Rgba8( 160, 160, 160, 100 ) );
		localWeaponVerts[14] = Vertex_PCU( Vec2( 1.f, 0.f ), Rgba8( 160, 160, 160, 100 ) );
		localWeaponVerts[15] = Vertex_PCU( Vec2( -1.f, 2.f * m_playerShip->m_superWeaponTimer / m_playerShip->m_superWeaponCoolDownTime ), Rgba8( 160, 160, 160, 100 ) );
		localWeaponVerts[16] = Vertex_PCU( Vec2( 1.f, 0.f ), Rgba8( 160, 160, 160, 100 ) );
		localWeaponVerts[17] = Vertex_PCU( Vec2( 1.f, 2.f * m_playerShip->m_superWeaponTimer / m_playerShip->m_superWeaponCoolDownTime ), Rgba8( 160, 160, 160, 100 ) );
		TransformVertexArrayXY3D( 18, localWeaponVerts, 40.f, 0.f, Vec2( 1510.f, 620.f ) );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( 18, localWeaponVerts );

		Vertex_PCU bulletPicVerts[36] = {
			Vertex_PCU( Vec2( 2.f, 0.5f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.5f, 0.f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.f, -0.5f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.f, 0.5f ), Rgba8( 255, 0, 0 ) ),
			Vertex_PCU( Vec2( 2.f, -0.5f ), Rgba8( 255, 0, 0 ) ),
			Vertex_PCU( Vec2( 0.f, 0.f ), Rgba8( 255, 0, 0, 0 ) ),
			Vertex_PCU( Vec2( 2.f, 0.5f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.5f, 0.f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.f, -0.5f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.f, 0.5f ), Rgba8( 255, 0, 0 ) ),
			Vertex_PCU( Vec2( 2.f, -0.5f ), Rgba8( 255, 0, 0 ) ),
			Vertex_PCU( Vec2( 0.f, 0.f ), Rgba8( 255, 0, 0, 0 ) ),
			Vertex_PCU( Vec2( 2.f, 0.5f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.5f, 0.f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.f, -0.5f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.f, 0.5f ), Rgba8( 255, 0, 0 ) ),
			Vertex_PCU( Vec2( 2.f, -0.5f ), Rgba8( 255, 0, 0 ) ),
			Vertex_PCU( Vec2( 0.f, 0.f ), Rgba8( 255, 0, 0, 0 ) ),
			Vertex_PCU( Vec2( 2.f, 0.5f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.5f, 0.f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.f, -0.5f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.f, 0.5f ), Rgba8( 255, 0, 0 ) ),
			Vertex_PCU( Vec2( 2.f, -0.5f ), Rgba8( 255, 0, 0 ) ),
			Vertex_PCU( Vec2( 0.f, 0.f ), Rgba8( 255, 0, 0, 0 ) ),
			Vertex_PCU( Vec2( 2.f, 0.5f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.5f, 0.f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.f, -0.5f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.f, 0.5f ), Rgba8( 255, 0, 0 ) ),
			Vertex_PCU( Vec2( 2.f, -0.5f ), Rgba8( 255, 0, 0 ) ),
			Vertex_PCU( Vec2( 0.f, 0.f ), Rgba8( 255, 0, 0, 0 ) ),
			Vertex_PCU( Vec2( 2.f, 0.5f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.5f, 0.f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.f, -0.5f ), Rgba8( 255, 255, 0 ) ),
			Vertex_PCU( Vec2( 2.f, 0.5f ), Rgba8( 255, 0, 0 ) ),
			Vertex_PCU( Vec2( 2.f, -0.5f ), Rgba8( 255, 0, 0 ) ),
			Vertex_PCU( Vec2( 0.f, 0.f ), Rgba8( 255, 0, 0, 0 ) ),
		};
		Vec2 bulletPicWorldPos = Vec2( 1510.f, 660.f );
		TransformVertexArrayXY3D( 6, &bulletPicVerts[0], 15.f, 0.f, bulletPicWorldPos );
		TransformVertexArrayXY3D( 6, &bulletPicVerts[6], 15.f, 60.f, bulletPicWorldPos );
		TransformVertexArrayXY3D( 6, &bulletPicVerts[12], 15.f, 120.f, bulletPicWorldPos );
		TransformVertexArrayXY3D( 6, &bulletPicVerts[18], 15.f, 180.f, bulletPicWorldPos );
		TransformVertexArrayXY3D( 6, &bulletPicVerts[24], 15.f, 240.f, bulletPicWorldPos );
		TransformVertexArrayXY3D( 6, &bulletPicVerts[30], 15.f, 300.f, bulletPicWorldPos );

		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( 36, bulletPicVerts );
		//g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 1477.f, 660.f ), 18.f, "Super\nWeapon", Rgba8( 255, 0, 0, 255 ) );
	}

	// render tutorial
	if (m_showKeyTutorial) {
		std::vector<Vertex_PCU> verts;
		verts.reserve( 1000 );
		AddVertsForAABB2D( verts, m_screenCamera.m_cameraBox, Rgba8( 100, 100, 100, 100 ), AABB2::IDENTITY );
		AddVertsForLineSegment2D( verts, Vec2( 840.f, 800.f ), Vec2( 840.f, 0.f ), 0.5f, Rgba8( 96, 96, 96 ) );
		
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 100.f, 610.f ), Vec2( 800.f, 650.f ) ), 40.f, "Keyboard", Rgba8( 200, 150, 50 ) );

		//Q
		DrawKeyBoardButtom( AABB2( Vec2( 140.f, 430.f ), Vec2( 180.f, 470.f ) ), 'Q', Rgba8::WHITE );
		AddVertsForLineSegment2D( verts, Vec2( 160.f, 475.f ), Vec2( 160.f, 505.f ), 2.f, Rgba8( 255, 255, 102 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 90.f, 510.f ), Vec2( 230.f, 530.f ) ), 20.f, "Shoot Bullet" );
		
		//W
		DrawKeyBoardButtom( AABB2( Vec2( 180.f, 430.f ), Vec2( 220.f, 470.f ) ), 'W', Rgba8::WHITE );
		AddVertsForLineSegment2D( verts, Vec2( 200.f, 425.f ), Vec2( 200.f, 390.f ), 2.f, Rgba8( 255, 255, 102 ) );
		std::string lightSaberText;
		if (m_playerShip->m_allowLighrSaber) {
			lightSaberText = "Light Saber";
		}
		else {
			lightSaberText = "Light Saber\nNot Owned";
		}
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 130.f, 345.f ), Vec2( 270.f, 385.f ) ), 20.f, lightSaberText, Rgba8::WHITE, 0.618f, Vec2( 0.5f, 1.f ) );

		//E
		DrawKeyBoardButtom( AABB2( Vec2( 220.f, 430.f ), Vec2( 260.f, 470.f ) ), 'E', Rgba8::WHITE );
		AddVertsForLineSegment2D( verts, Vec2( 240.f, 475.f ), Vec2( 240.f, 555.f ), 2.f, Rgba8( 255, 255, 102 ) );
		std::string coneAttackText;
		if (IsUpgraded( UpgradeType::coneAttack )) {
			coneAttackText = "Sector Attack";
		}
		else {
			coneAttackText = "Sector Attack\nNot Owned";
		}
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 30.f, 560.f ), Vec2( 450.f, 600.f ) ), 20.f, coneAttackText, Rgba8::WHITE, 0.618f, Vec2( 0.5f, 0.f ) );
		
		//R
		DrawKeyBoardButtom( AABB2( Vec2( 260.f, 430.f ), Vec2( 300.f, 470.f ) ), 'R', Rgba8::WHITE );
		AddVertsForLineSegment2D( verts, Vec2( 280.f, 425.f ), Vec2( 280.f, 345.f ), 2.f, Rgba8( 255, 255, 102 ) );
		std::string rocketAttackText;
		if (IsUpgraded( UpgradeType::rocketAttack )) {
			rocketAttackText = "Shoot Rocket";
		}
		else {
			rocketAttackText = "Shoot Rocket\nNot Owned";
		}
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 130.f, 300.f ), Vec2( 430.f, 340.f ) ), 20.f, rocketAttackText, Rgba8::WHITE, 0.618f, Vec2( 0.5f, 1.f ) );
		
		//A
		DrawKeyBoardButtom( AABB2( Vec2( 145.f, 390.f ), Vec2( 185.f, 430.f ) ), 'A', Rgba8::WHITE );
		AddVertsForLineSegment2D( verts, Vec2( 140.f, 410.f ), Vec2( 100.f, 410.f ), 2.f, Rgba8( 255, 255, 102 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 400.f ), Vec2( 115.f, 420.f ) ), 20.f, "Thrust", Rgba8::WHITE, 0.618f, Vec2( 0.5f, 1.f ) );

		/*
		//D
		DrawKeyBoardButtom( AABB2( Vec2( 295.f, 390.f ), Vec2( 335.f, 430.f ) ), 'D', Rgba8::WHITE );
		AddVertsForLineSegment2D( verts, Vec2( 315.f, 435.f ), Vec2( 315.f, 470.f ), 2.f, Rgba8( 255, 255, 102 ) );
		AddVertsForLineSegment2D( verts, Vec2( 315.f, 470.f ), Vec2( 355.f, 510.f ), 2.f, Rgba8( 255, 255, 102 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 260.f, 510.f ), Vec2( 460.f, 540.f ) ), 20.f, "Turn Right" );
		//A
		DrawKeyBoardButtom( AABB2( Vec2( 215.f, 390.f ), Vec2( 255.f, 430.f ) ), 'A', Rgba8::WHITE );
		AddVertsForLineSegment2D( verts, Vec2( 235.f, 435.f ), Vec2( 235.f, 470.f ), 2.f, Rgba8( 255, 255, 102 ) );
		AddVertsForLineSegment2D( verts, Vec2( 235.f, 470.f ), Vec2( 195.f, 510.f ), 2.f, Rgba8( 255, 255, 102 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 85.f, 510.f ), Vec2( 295.f, 540.f ) ), 20.f, "Turn Left" );
		*/
		//space
		AddVertsForAABB2D( verts, AABB2( Vec2( 215.f, 250.f ), Vec2( 415.f, 290.f ) ), Rgba8::WHITE );
		g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 272.f, 253.f ), 30.f, "SPACE", Rgba8( 0, 0, 0 ) );
		AddVertsForLineSegment2D( verts, Vec2( 315.f, 245.f ), Vec2( 315.f, 195.f ), 2.5f, Rgba8( 255, 255, 102 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 15.f, 165.f ), Vec2( 615.f, 195.f ) ), 20.1f, "Thrust" );
		//P
		DrawKeyBoardButtom( AABB2( Vec2( 470.f, 430.f ), Vec2( 510.f, 470.f ) ), 'P', Rgba8::WHITE );
		AddVertsForLineSegment2D( verts, Vec2( 490.f, 475.f ), Vec2( 490.f, 525.f ), 2.f, Rgba8( 255, 255, 102 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 455.f, 530.f ), Vec2( 525.f, 550.f ) ), 20.f, "Pause" );

		// mouse
		std::vector<Vertex_PCU> leftMouseKeyVerts;
		leftMouseKeyVerts.reserve( 1000 );
		AddVertsForSector2D( leftMouseKeyVerts, Vec2( 0, 0 ), 135.f, 90.f, 60.f, Rgba8::WHITE );
		Mat44 leftTransformMat = Mat44::CreateTranslation2D( Vec2( 700.f, 450.f ) );
		leftTransformMat.AppendScaleNonUniform2D( Vec2( 1.f, 1.8f ) );
		for (Vertex_PCU& vertPCU : leftMouseKeyVerts) {
			vertPCU.m_position = leftTransformMat.TransformPosition3D( vertPCU.m_position );
		}

		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( leftMouseKeyVerts );

		std::vector<Vertex_PCU> rightMouseKeyVerts;
		rightMouseKeyVerts.reserve( 1000 );
		AddVertsForSector2D( rightMouseKeyVerts, Vec2( 0, 0 ), 45.f, 90.f, 60.f, Rgba8::WHITE );
		Mat44 rightTransformMat = Mat44::CreateTranslation2D( Vec2( 704.f, 450.f ) );
		rightTransformMat.AppendScaleNonUniform2D( Vec2( 1.f, 1.8f ) );
		for (Vertex_PCU& vertPCU : rightMouseKeyVerts) {
			vertPCU.m_position = rightTransformMat.TransformPosition3D( vertPCU.m_position );
		}

		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( rightMouseKeyVerts );

		AddVertsForCapsule2D( verts, Vec2( 702.f, 495.f ), Vec2( 702.f, 520.f ), 3.5f, Rgba8::WHITE );

		AddVertsForLineSegment2D( verts, Vec2( 680.f, 510.f ), Vec2( 695.f, 600.f ), 2.f, Rgba8( 255, 255, 102 ) );
		AddVertsForLineSegment2D( verts, Vec2( 724.f, 510.f ), Vec2( 709.f, 600.f ), 2.f, Rgba8( 255, 255, 102 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 502.f, 600.f ), Vec2( 902.f, 620.f ) ), 20.f, "Thrust" );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 550.f, 300.f ), Vec2( 840.f, 360.f ) ), 20.f, "Mouse cursor position\ncontrols\nship orientation" );

		/*
		//L
		DrawKeyBoardButtom( AABB2( Vec2( 535.f, 390.f ), Vec2( 575.f, 430.f ) ), 'L', Rgba8::WHITE ); // light saber
		AddVertsForLineSegment2D( verts, Vec2( 555.f, 385.f ), Vec2( 595.f, 345.f ), 2.f, Rgba8( 255, 255, 102 ) );
		AddVertsForLineSegment2D( verts, Vec2( 595.f, 345.f ), Vec2( 655.f, 345.f ), 2.f, Rgba8( 255, 255, 102 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 660.f, 335.f ), Vec2( 810.f, 355.f ) ), 20.f, "Light Saber" );
		//K
		DrawKeyBoardButtom( AABB2( Vec2( 495.f, 390.f ), Vec2( 535.f, 430.f ) ), 'K', Rgba8::WHITE ); // rocket
		AddVertsForLineSegment2D( verts, Vec2( 515.f, 385.f ), Vec2( 595.f, 305.f ), 2.f, Rgba8( 255, 255, 102 ) );
		AddVertsForLineSegment2D( verts, Vec2( 595.f, 305.f ), Vec2( 655.f, 305.f ), 2.f, Rgba8( 255, 255, 102 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 660.f, 295.f ), Vec2( 820.f, 315.f ) ), 20.f, "Shoot Rocket" );
		//J
		DrawKeyBoardButtom( AABB2( Vec2( 455.f, 390.f ), Vec2( 495.f, 430.f ) ), 'J', Rgba8::WHITE ); // cone
		AddVertsForLineSegment2D( verts, Vec2( 475.f, 385.f ), Vec2( 595.f, 265.f ), 2.f, Rgba8( 255, 255, 102 ) );
		AddVertsForLineSegment2D( verts, Vec2( 595.f, 265.f ), Vec2( 655.f, 265.f ), 2.f, Rgba8( 255, 255, 102 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 660.f, 255.f ), Vec2( 830.f, 275.f ) ), 20.f, "Sector Attack" );
		*/

		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 800.f, 610.f ), Vec2( 1500.f, 650.f ) ), 40.f, "Controller", Rgba8( 200, 150, 50 ) );
		//joystick
		DrawJoyStick( Vec2( 900.f, 400.f ), 40.f, Rgba8::WHITE );
		AddVertsForLineSegment2D( verts, Vec2( 900.f, 450.f ), Vec2( 900.f, 515.f ), 2.f, Rgba8( 255, 255, 102 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 700.f, 520.f ), Vec2( 1100.f, 545.f ) ), 25.f, "Move" );
		DrawJoyStick( Vec2( 1210.f, 310.f ), 40.f, Rgba8::WHITE );

		// XBox X
		DrawXBoxButton( Vec2( 1260.f, 400.f ), 30.f, XboxButtonID::XBOX_BUTTON_X, Rgba8( 0, 0, 0 ) ); // light saber
		AddVertsForLineSegment2D( verts, Vec2( 1225.f, 400.f ), Vec2( 1175.f, 400.f ), 2.f, Rgba8( 255, 255, 102 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1025.f, 380.f ), Vec2( 1175.f, 420.f ) ), 20.f, lightSaberText );
	
		// XBox Y
		DrawXBoxButton( Vec2( 1300.f, 440.f ), 30.f, XboxButtonID::XBOX_BUTTON_Y, Rgba8( 0, 0, 0 ) ); // rocket
		AddVertsForLineSegment2D( verts, Vec2( 1300.f, 475.f ), Vec2( 1300.f, 525.f ), 2.f, Rgba8( 255, 255, 102 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1200.f, 530.f ), Vec2( 1400.f, 570.f ) ), 20.f, rocketAttackText );
	
		// XBox A
		DrawXBoxButton( Vec2( 1300.f, 360.f ), 30.f, XboxButtonID::XBOX_BUTTON_A, Rgba8( 0, 0, 0 ) ); // bullet
		AddVertsForLineSegment2D( verts, Vec2( 1300.f, 325.f ), Vec2( 1300.f, 275.f ), 2.f, Rgba8( 255, 255, 102 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1200.f, 250.f ), Vec2( 1500.f, 270.f ) ), 20.1f, "Shoot Bullet" );
	
		// XBox B
		DrawXBoxButton( Vec2( 1340.f, 400.f ), 30.f, XboxButtonID::XBOX_BUTTON_B, Rgba8( 0, 0, 0 ) ); // cone
		AddVertsForLineSegment2D( verts, Vec2( 1375.f, 400.f ), Vec2( 1425.f, 400.f ), 2.f, Rgba8( 255, 255, 102 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1425.f, 380.f ), Vec2( 1600.f, 420.f ) ), 20.f, coneAttackText );

		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 20.f, 720.f ), Vec2( 330.f, 745.f ) ), 25.f, "Remaining Health(Max 7)" );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1400.f, 700.f ), Vec2( 1600.f, 735.f ) ), 35.f, "Ammo" );
			

		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( verts );
	}

	if (m_waitForUpgradeMode > 0.f) {
		if (m_waitForUpgradeMode < 0.5f) {
			float ratio = (0.5f - m_waitForUpgradeMode) * 2.f;
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 500.f, 400.f + ratio * 220.f ), Vec2( 1100.f, 460.f + ratio * 220.f ) ), 60.f, "MISSION COMPLETE", Rgba8( 200, 150, 50 ), 0.618f, Vec2( 0.f, 0.f ) );
			std::vector<Vertex_PCU> AABB2Vert;
			AABB2Vert.reserve( 6 );
			AddVertsForAABB2D( AABB2Vert, AABB2( Vec2( 800.f - 600.f * ratio, 200.f ), Vec2( 800.f + 600.f * ratio, 600.f ) ), Rgba8( 255, 255, 153, (unsigned char)(255 * ratio) ), AABB2::IDENTITY );
			g_theRenderer->BindTexture( nullptr );
			g_theRenderer->SetModelConstants();
			g_theRenderer->DrawVertexArray( AABB2Vert );
		}
		else {
			int num = (int)((1.5f - m_waitForUpgradeMode) * 16.f);
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 500.f, 400.f ), Vec2( 1100.f, 460.f ) ), 60.f, "MISSION COMPLETE", Rgba8( 200, 150, 50 ), 0.618f, Vec2( 0.f, 0.f ), TextBoxMode::SHRINK_TO_FIT, num );
		}
	}

	// draw all texts
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( (int)textVerts.size(), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}

void Game::RenderBackground() const
{
	for (int i = 0; i < MAX_BACKGROUND_STARS; i++) {
		float l = m_backGroundStarSize[i];
		Vertex_PCU starVerts[18] = {
			Vertex_PCU( Vec3( 0.5f * l, -0.5f * l, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( 0.5f * l, 0.5f * l, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( -0.5f * l, 0.5f * l, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( -0.5f * l, -0.5f * l, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( 0.5f * l, -0.5f * l, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( -0.5f * l, 0.5f * l, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( 0.5f * l, 0.5f * l, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( 0.5f * l, -0.5f * l, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( l, 0, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( -0.5f * l, 0.5f * l, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( -0.5f * l, -0.5f * l, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( -l, 0, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( 0.5f * l, 0.5f * l, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( -0.5f * l, 0.5f * l, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( 0, l, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( 0.5f * l, -0.5f * l, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( -0.5f * l, -0.5f * l, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
			Vertex_PCU( Vec3( 0, -l, 0 ), Rgba8( 255, 255, 255, 255 ), Vec2( 0, 0 ) ),
		};
		TransformVertexArrayXY3D( 18, starVerts, 1.f, 0.f, m_backGroundStarPosition[i] - (m_playerShip->m_position - Vec2( 100.f, 50.f )) / 5 );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( 18, starVerts );
	}
}

void Game::RegisterRenderText( Text* str )
{
	m_timeElapsedRenderingText = 0.f;
	m_renderingText = str;
	m_isRenderingText = true;
}

void Game::StopRenderText()
{
	m_isRenderingText = false;
}

bool Game::IsUpgraded( UpgradeType type ) const
{
	return m_upgradeSystem.m_upgradeStatus[(int)type];
}

Entity* Game::GetNearestEnemyActor( Vec2 const& refPos ) const
{
	float minSquaredDistance = FLT_MAX;
	Entity* res = nullptr;
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (m_entities[i] && m_entities[i]->IsAlive() && m_entities[i]->IsActor()) {
			float squaredDistance = GetDistanceSquared2D( refPos, m_entities[i]->m_position );
			if (squaredDistance < minSquaredDistance) {
				minSquaredDistance = squaredDistance;
				res = m_entities[i];
			}
		}
	}
	return res;
}

Entity* Game::GetNearestPlayerBullet( Vec2 const& refPos ) const
{
	float minSquaredDistance = FLT_MAX;
	Entity* res = nullptr;
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (m_entities[i] && m_entities[i]->IsAlive() && m_entities[i]->m_type == EntityType::bullet) {
			float squaredDistance = GetDistanceSquared2D( refPos, m_entities[i]->m_position );
			if (squaredDistance < minSquaredDistance) {
				minSquaredDistance = squaredDistance;
				res = m_entities[i];
			}
		}
	}
	return res;
}

Entity* Game::GetRandomEnemyActor() const
{
	Entity* enemies[MAX_ENTITIES];
	int k = 0;
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (m_entities[i] && m_entities[i]->IsAlive() && m_entities[i]->IsActor() && (m_entities[i]->m_type != EntityType::bossTurret || !((BossDestroyerTurret*)m_entities[i])->m_isDeatroyed)) {
			enemies[k] = m_entities[i];
			k++;
		}
	}
	if (k == 0) {
		return nullptr;
	}
	return enemies[m_randNumGen->RollRandomIntLessThan( k )];
}

void Game::DealCollision()
{
	// each 2 entities counts for once
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (m_entities[i] && m_entities[i]->IsAlive()) {
			Entity& thisA = *m_entities[i];
			for (int j = i + 1; j < MAX_ENTITIES; j++) {
				if (m_entities[j] && m_entities[j]->IsAlive()) {
					Entity& thisB = *m_entities[j];
					if (IsCollide( thisA.m_type, thisB.m_type ) && 
						DoDiscsOverlap( thisA.m_position, thisA.m_physicsRadius, thisB.m_position, thisB.m_physicsRadius )) {
						// code basic problem makes these code block complex
						// todo: if have time, redo it
						if (thisB.m_type == EntityType::powerUp) {
							GUARANTEE_OR_DIE( thisA.m_type == EntityType::playerShip, "powerup can only collide with playership" );
							if (((PowerUp*)(&thisB))->m_powerUpType == PowerUpType::enhance) {
								m_playerShip->m_enhanceTimeLeft += m_powerUpTime;
								g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::powerUp ) );
							}
							else if (((PowerUp*)(&thisB))->m_powerUpType == PowerUpType::health) {
								m_playerShip->m_health = (m_playerShip->m_health + 1) > 7 ? 7 : (m_playerShip->m_health + 1);
								g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::health ) );
							}
							else if (((PowerUp*)(&thisB))->m_powerUpType == PowerUpType::ammo) {
								m_playerShip->m_ammoAmount = (m_playerShip->m_ammoAmount + 50) > m_playerShip->m_maxAmmo ? m_playerShip->m_maxAmmo : (m_playerShip->m_ammoAmount + 50);
								g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::health ) );
							}
							thisB.BeAttacked( 1 );
						}
						/*else if ((thisA.m_type == EntityType::bullet && thisB.m_type == EntityType::boss)
							|| (thisB.m_type == EntityType::bullet && thisA.m_type == EntityType::boss)) {
							thisA.BeAttacked( 2 );
							thisB.BeAttacked( 2 );
						}*/
						else {
							if (thisA.m_type == EntityType::playerShip && ((PlayerShip*)&thisA)->m_invincibleTimeLeft > 0.f){
								return;
							}
							thisA.BeAttacked( 1 );
							thisB.BeAttacked( 1 );
						}
					}
				}
			}
		}
	}
}

bool Game::IsCollide( EntityType type1, EntityType type2 ) const
{
	// make type1 <= type2
	if (type1 > type2) {
		EntityType temp = type2;
		type2 = type1;
		type1 = temp;
	}

	// check if they can collide with each other
	if (   type1 == EntityType::playerShip && type2 == EntityType::asteroid
		|| type1 == EntityType::playerShip && type2 == EntityType::beetle
		|| type1 == EntityType::playerShip && type2 == EntityType::wasp
		|| type1 == EntityType::playerShip && type2 == EntityType::enemyBullet
		|| type1 == EntityType::playerShip && type2 == EntityType::boss
		|| type1 == EntityType::playerShip && type2 == EntityType::powerUp
		|| type1 == EntityType::playerShip && type2 == EntityType::carrier
		|| type1 == EntityType::playerShip && type2 == EntityType::fighter
		|| type1 == EntityType::bullet && type2 == EntityType::asteroid
		|| type1 == EntityType::bullet && type2 == EntityType::beetle
		|| type1 == EntityType::bullet && type2 == EntityType::wasp
		|| type1 == EntityType::bullet && type2 == EntityType::boss
		|| type1 == EntityType::bullet && type2 == EntityType::enemyBullet
		|| type1 == EntityType::bullet && type2 == EntityType::bossTurret
		|| type1 == EntityType::bullet && type2 == EntityType::carrier
		|| type1 == EntityType::bullet && type2 == EntityType::fighter
		|| type1 == EntityType::asteroid && type2 == EntityType::enemyBullet
		) {
		return true;
	}
	return false;
}

void Game::LoadLevel()
{
	// text hard code
	// lifespan time must greater than 4 seconds, 7-9 seconds are best
	// either callback function or start time must be set, see Wave.hpp
	Text* textArray[NUM_OF_LEVELS][30] = {
	{
		new Text( 0, 7.f, std::string( "Hi, Captain! You are assigned to explore a lost galaxy called poxolis." ), 1.f, Rgba8( 204, 153, 255, 255 ) ),
		new Text( 0, 7.f, std::string( "And I'm your assistant and technician of this ship.\nYou can call me Theodora.\nI'm glad to fight along with you~" ), 8.f, Rgba8( 204, 153, 255, 255 ) ),
		new Text( 0, 7.f, std::string( "Press P to pause the game and\n see how to drive our space ship." ), 15.f, Rgba8( 204, 153, 255, 255 ) ),
		new Text( 0, 7.f, std::string( "We are the second to explore here, there was a first explorer ship." ), 22.f, Rgba8( 204, 153, 255, 255 ) ),
		new Text( 0, 7.f, std::string( "We lost connection with them 10 days ago, so high council sent us to find them." ), 29.f, Rgba8( 204, 153, 255, 255 ) ),
		new Text( 0, 7.f, std::string( "Oh! Asteroids! Try not to crash on them!\nShoot to destroy them or there will be more." ), 36.f, Rgba8( 204, 153, 255, 255 ) ),
		new Text( 0, 7.f, std::string( "Shooting will consume ammo, please always reserve ammo for future fight~" ), 43.f, Rgba8( 204, 153, 255, 255 ) ),
		new Text( 0, 6.f, std::string( "A beetle? It seems not friendly to us! Prepare to fight!" ), 50.f, Rgba8( 204, 153, 255, 255 ) ),
		new Text( 0, 5.f, std::string( "What is that blue evil thing?" ), 57.f, Rgba8( 204, 153, 255, 255 ) ),
		new Text( 0, 9.f, std::string( "Searching in the dictionary quickly...I haven't seen a wasp before ^o^\nIt can thrust and shoot in three directions." ), 64.f, Rgba8( 204, 153, 255, 255 ) ),
	},
	{
		new Text( 1, 10.f, std::string( "Do you remember how to use light saber?\nOnce master GULU taught us.\nPress W or XBox X to use it." ), 0.f, Rgba8( 204, 153, 255, 255 ) ),
		new Text( 1, 6.f, std::string( "Light saber costs us a lot of ammo, so do not always open it." ), 11.f, Rgba8( 204, 153, 255, 255 ) ),
		new Text( 1, 10.f, std::string( "Shiny green thing, maybe used to repair our ship!" ), 0.f, Rgba8( 204, 153, 255, 255 ), IsHealthTutorial ),
	},
	{
		new Text( 2, 10.f, std::string( "Shiny green thing, maybe used to repair our ship!" ), 0.f, Rgba8( 204, 153, 255, 255 ), IsHealthTutorial ),
	},
	{
		new Text( 3, 13.f, std::string( "Look that green ship! It's first explorer! Our friends are alive!\n^o^ They looks unfriendly!\nThey are attacking us. What happened?" ), 23.f, Rgba8( 204, 153, 255, 255 ) ),
		new Text( 1, 7.f, std::string( "They know how to eliminate our light saber attack, do not try to defeat bosses by light saber." ), 36.f, Rgba8( 204, 153, 255, 255 ) ),
		new Text( 3, 10.f, std::string( "Shiny green thing, maybe used to repair our ship!" ), 0.f, Rgba8( 204, 153, 255, 255 ), IsHealthTutorial ),
	},
	{
		new Text( 3, 10.f, std::string( "We get the tech of Cone Attack from first explorer.\nPress E or XBox B to use it." ), 0.f, Rgba8( 204, 153, 255, 255 ) ),
		new Text( 4, 10.f, std::string( "Shiny green thing, maybe used to repair our ship!" ), 0.f, Rgba8( 204, 153, 255, 255 ), IsHealthTutorial ),
	},
	{
		new Text( 5, 10.f, std::string( "Shiny green thing, maybe used to repair our ship!" ), 0.f, Rgba8( 204, 153, 255, 255 ), IsHealthTutorial ),
	},
	{
		new Text( 6, 10.f, std::string( "Shiny green thing, maybe used to repair our ship!" ), 0.f, Rgba8( 204, 153, 255, 255 ), IsHealthTutorial ),
	},
	{
		new Text( 7, 10.f, std::string( "Shiny green thing, maybe used to repair our ship!" ), 0.f, Rgba8( 204, 153, 255, 255 ), IsHealthTutorial ),
	},
	{
		new Text( 7, 10.f, std::string( "Shiny green thing, maybe used to repair our ship!" ), 0.f, Rgba8( 204, 153, 255, 255 ), IsHealthTutorial ),
	}
	};
	for (int i = 0; i < NUM_OF_LEVELS; i++) {
		Wave* waveArray[MAX_WAVES_IN_LEVEL] = {};
		for (int j = 0; j < NUM_OF_WAVES_IN_LEVEL[i]; j++) {
			waveArray[j] = new Wave( LEVEL_INFO[i][j] );
		}
		m_levels[i] = new Level( NUM_OF_WAVES_IN_LEVEL[i], waveArray, this, NUM_OF_TEXTS_IN_LEVEL[i], textArray[i] );
	}
}

bool Game::StartNewWave( Wave* wave )
{
	//if (m_currentLevel == 0) {
	//	DestroyerSpawnTurret();
	//}

	// spawn all the enemy entities
	for (int i = 0; i < wave->m_numOfAsteroids; i++) {
		SpawnNewEntity( new Asteroid( MakeRandomOffMapPosition( ASTEROID_COSMETIC_RADIUS ), this ) );
	}

	for (int i = 0; i < wave->m_numOfBeetles; i++) {
		SpawnNewEntity( new Beetle( MakeRandomOffMapPosition( BEETLE_COSMETIC_RADIUS ), this, m_playerShip ) );
	}

	for (int i = 0; i < wave->m_numOfWasps; i++) {
		SpawnNewEntity( new Wasp( MakeRandomOffMapPosition( WASP_COSMETIC_RADIUS ), this, m_playerShip ) );
	}

	for (int i = 0; i < wave->m_numOfCarrier; i++) {
		SpawnNewEntity( new Carrier( MakeRandomOffMapPosition( CARRIER_COSMETIC_RADIUS ), this, m_playerShip ) );
	}

	for (int i = 0; i < wave->m_numOfFirstExplorer; i++) {
		SpawnNewEntity( new BossFirstExplorer( MakeRandomOffMapPosition( BOSS_EXPLORER_COSMETIC_RADIUS ), this, m_playerShip ) );
		g_theAudio->StopSound( g_theApp->m_gameModeMusic );
		g_theApp->m_bossMusic = g_theAudio->StartSound( g_theApp->m_audioDictionary[(int)AudioName::boss], true );
		m_isBossFight = true;
	}

	for (int i = 0; i < wave->m_numOfDestroyer; i++) {
		SpawnBossDestroyer();
		g_theAudio->StopSound( g_theApp->m_gameModeMusic );
		g_theApp->m_bossMusic = g_theAudio->StartSound( g_theApp->m_audioDictionary[(int)AudioName::boss], true );
		m_isBossFight = true;
	}

	for (int i = 0; i < wave->m_numOfMotherOfCarrier; i++) {
		SpawnNewEntity( new BossMotherOfCarrier( MakeRandomOffMapPosition( BOSS_MOTHER_COSMETIC_RADIUS ), this, m_playerShip ) );
		g_theAudio->StopSound( g_theApp->m_gameModeMusic );
		g_theApp->m_bossMusic = g_theAudio->StartSound( g_theApp->m_audioDictionary[(int)AudioName::boss], true );
		m_isBossFight = true;
	}

	return true;
}

void Game::StartNewLevel()
{
	m_gameState = GameState::Playing;
	m_currentLevel++;
	StopRenderText();
	g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::newLevel ), false, 3.f );
}

void Game::MakeBackgroundStars()
{
	for (int i = 0; i < MAX_BACKGROUND_STARS; i++) {
		m_backGroundStarPosition[i].x = m_randNumGen->RollRandomFloatInRange( -20.f, 220.f );
		m_backGroundStarPosition[i].y = m_randNumGen->RollRandomFloatInRange( -10.f, 110.f );
		m_backGroundStarSize[i] = m_randNumGen->RollRandomFloatInRange( 0.1f, 0.3f );
	}
}

Vec2 Game::MakeRandomOffMapPosition( float cosmeticRadius )
{
	// get a random position
	float x = m_randNumGen->RollRandomFloatZeroToOne() * WORLD_SIZE_X;
	float y = m_randNumGen->RollRandomFloatZeroToOne() * WORLD_SIZE_Y;

	// put the entity to the position just off screen
	if (x < WORLD_SIZE_X - x && x < WORLD_SIZE_Y - y && x < y) {
		x = -cosmeticRadius + 0.1f;
	}
	else if (WORLD_SIZE_X - x < x && WORLD_SIZE_X - x < y && WORLD_SIZE_X - x < WORLD_SIZE_Y - y) {
		x = WORLD_SIZE_X + cosmeticRadius - 0.1f;
	}
	else if (y < WORLD_SIZE_X - x && y < x && y < WORLD_SIZE_Y - y) {
		y = -cosmeticRadius + 0.1f;
	}
	else {
		y = WORLD_SIZE_Y + cosmeticRadius - 0.1f;
	}
	return Vec2( x, y );
}

float Game::GetCurBossHealthRatio() const
{
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (m_entities[i] && m_entities[i]->IsAlive() && m_entities[i]->m_type == EntityType::boss) {
			return (float)m_entities[i]->m_health / (float)m_entities[i]->m_maxHealth;
		}
	}
	return 0.f;
}

void UpgradeSystem::DealUpgrade(Game* game)
{
	UpgradeType canUpgradeArray[8] = { UpgradeType::Empty };
	int count = 0;
	int j = 0;
	for (int i = 0; i < 8; i++) {
		while (j < (int)UpgradeType::NUM) {
			if (m_canUpgrade[j] != UpgradeType::Empty) {
				canUpgradeArray[i] = m_canUpgrade[j];
				count++;
				j++;
				break;
			}
			j++;
		}
	}
	if (count == 0) {
		m_thisUpgrade[0] = UpgradeType::Empty;
		m_thisUpgrade[1] = UpgradeType::Empty;
		m_thisUpgrade[2] = UpgradeType::Empty;
	}
	else if (count == 1) {
		m_thisUpgrade[0] = canUpgradeArray[0];
		m_thisUpgrade[1] = UpgradeType::Empty;
		m_thisUpgrade[2] = UpgradeType::Empty;

	}
	else if (count == 2) {
		m_thisUpgrade[0] = canUpgradeArray[0];
		m_thisUpgrade[1] = canUpgradeArray[1];
		m_thisUpgrade[2] = UpgradeType::Empty;
	}
	else if (count == 3) {
		m_thisUpgrade[0] = canUpgradeArray[0];
		m_thisUpgrade[1] = canUpgradeArray[1];
		m_thisUpgrade[2] = canUpgradeArray[2];
	}
	else {
		int num1 = game->m_randNumGen->RollRandomIntInRange( 0, count - 1 );
		int num2 = game->m_randNumGen->RollRandomIntInRange( 0, count - 1 );
		while (num1 == num2) {
			num2 = game->m_randNumGen->RollRandomIntInRange( 0, count - 1 );
		}
		int num3 = game->m_randNumGen->RollRandomIntInRange( 0, count - 1 );
		while (num1 == num3 || num2 == num3) {
			num3 = game->m_randNumGen->RollRandomIntInRange( 0, count - 1 );
		}
		m_thisUpgrade[0] = canUpgradeArray[num1];
		m_thisUpgrade[1] = canUpgradeArray[num2];
		m_thisUpgrade[2] = canUpgradeArray[num3];
	}
	m_isDealt = true;
}

void UpgradeSystem::Upgrade( UpgradeType type )
{
	GUARANTEE_OR_DIE( type != UpgradeType::Empty, "Upgrade type cannot be empty" );
	for (int i = 0; i < (int)UpgradeType::NUM; i++) {
		if (m_canUpgrade[i] == type) {
			if (type == UpgradeType::coneAttack) {
				m_canUpgrade[i] = UpgradeType::coneAttackLargerRange;
			}
			else if (type == UpgradeType::fasterAmmoGrow) {
				m_canUpgrade[i] = UpgradeType::lessAmmoCost;
			}
			else if (type == UpgradeType::rocketAttack) {
				m_canUpgrade[i] = UpgradeType::rocketLargerRange;
			}
			else if (type == UpgradeType::shield) {
				m_canUpgrade[i] = UpgradeType::shieldLessTime;
			}
			else {
				m_canUpgrade[i] = UpgradeType::Empty;
			}
			m_upgradeStatus[(int)type] = true;
			break;
		}
	}
}
