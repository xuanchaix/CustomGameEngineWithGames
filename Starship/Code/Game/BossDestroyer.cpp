#include "Game/BossDestroyer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/BossDestroyerTurret.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

BossDestroyer::BossDestroyer( Vec2 startPos, Game* game )
	:Entity(startPos, game)
{
	m_health = DESTROYER_HEALTH;
	m_maxHealth = DESTROYER_HEALTH;
	m_type = EntityType::boss;
	m_color = Rgba8( 255, 100, 100, 255 );

	m_physicsRadius = BOSS_EXPLORER_PHYSICS_RADIUS;
	m_cosmeticRadius = BOSS_EXPLORER_COSMETIC_RADIUS;
	m_spawnEnemies = game->BossDestroyerSpawnEnemy();
	m_turret = game->BossDestroyerSpawnTurret( this );

	m_nextLaserY1 = game->m_randNumGen->RollRandomFloatInRange( 0.f, 100.f );
	m_nextLaserY2 = game->m_randNumGen->RollRandomFloatInRange( 0.f, 100.f );
	m_repairingTurret = m_game->m_randNumGen->RollRandomIntInRange( 0, 7 );
	m_isActor = true;
	m_isEnemy = true;
}

BossDestroyer::~BossDestroyer()
{

}

void BossDestroyer::Update()
{
	float deltaTime = g_theGame->m_gameClock->GetDeltaSeconds();
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_stage == 1) {
		bool hasEnemy = false;
		for (int i = 0; i < DESTROYER_ENEMY_AMOUNT; i++) {
			if (m_spawnEnemies[i] && m_spawnEnemies[i]->m_isGarbage) {
				m_spawnEnemies[i] = nullptr;
			}
			else if (m_spawnEnemies[i]) {
				hasEnemy = true;
			}
		}
		if (!hasEnemy) {
			if (m_stage1Count < 1) {
				m_stage1Count++;
				delete m_spawnEnemies;
				m_spawnEnemies = nullptr;
				m_spawnEnemies = m_game->BossDestroyerSpawnEnemy();
			}
			else {
				delete m_spawnEnemies;
				m_spawnEnemies = nullptr;
				m_stage++;
				for (int i = 0; i < DESTROYER_TURRET_AMOUNT; i++) {
					m_turret[i]->m_doNotShoot = false;
				}
			}
		}
	}
	else if (m_stage == 2) {
		// move to position
		if (m_position.x > 180.f) {
			m_position.x -= 8.f * deltaTime;
			for (int i = 0; i < DESTROYER_TURRET_AMOUNT; i++) {
				if (m_turret[i]) {
					m_turret[i]->m_position.x -= 8.f * deltaTime;
				}
			}
		}

		// spawn beetles
		m_spawnBeetleTimer -= deltaTime;
		if (m_spawnBeetleTimer <= 0.f) {
			m_spawnBeetleTimer = m_spawnBeetleCoolDown;
			m_game->BossDestroyerSpawnBeetle();
		}

		bool hasTurret = false;
		for (int i = 0; i < DESTROYER_TURRET_AMOUNT; i++) {
			if (m_turret[i] && !m_turret[i]->m_isDeatroyed) {
				hasTurret = true;
				break;
			}
		}
		if (!hasTurret) {
			m_stage++;
		}
	}
	else if (m_stage == 3) {
		// move to position
		if (m_position.x > 180.f) {
			m_position.x -= 8.f * deltaTime;
			for (int i = 0; i < DESTROYER_TURRET_AMOUNT; i++) {
				if (m_turret[i]) {
					m_turret[i]->m_position.x -= 8.f * deltaTime;
				}
			}
		}

		// spawn beetles
		m_spawnBeetleTimer -= deltaTime;
		if (m_spawnBeetleTimer <= 0.f) {
			m_spawnBeetleTimer = m_spawnBeetleCoolDown;
			m_game->BossDestroyerSpawnBeetle();
		}

		// shield
		m_shieldHealth += deltaTime * m_shieldHealthGrowthPerSecond;
		m_shieldHealth = m_shieldHealth > m_maxShieldHealth ? m_maxShieldHealth : m_shieldHealth;

		// repair turret
		m_repairTime -= deltaTime;
		m_healthAccumulation += 3.f * deltaTime;
		if (m_healthAccumulation >= 1.f) {
			m_healthAccumulation -= 1.f;
			m_turret[m_repairingTurret]->m_health += 1;
			if (m_turret[m_repairingTurret]->m_health > TURRET_HEALTH) {
				m_turret[m_repairingTurret]->m_health = TURRET_HEALTH;
			}
		}
		if (m_repairTime <= 0.f) {
			m_repairTime = m_repairChange;
			int m = m_game->m_randNumGen->RollRandomIntInRange( 0, 7 );
			if (m_turret[m] && m_turret[m]->m_isDeatroyed) {
				m_turret[m_repairingTurret]->m_isRepairing = false;
				m_repairingTurret = m;
				m_turret[m]->m_isDeatroyed = false;
				m_turret[m]->m_isRepairing = true;
			}
			else if (m_turret[m]) {
				m_turret[m_repairingTurret]->m_isDeatroyed = false;
				m_turret[m_repairingTurret]->m_isRepairing = true;
			}
		}

		// shoot laser
		m_laserTime -= deltaTime;
		if (m_laserTime <= 0.f) {
			m_game->BossDestroyerShootLaser( this );
			m_laserTime = m_laserCoolDown;
			m_nextLaserY1 = m_game->m_randNumGen->RollRandomFloatInRange( 0.f, 100.f );
			m_nextLaserY2 = m_game->m_randNumGen->RollRandomFloatInRange( 0.f, 100.f );
		}
		if (m_health < DESTROYER_HEALTH * 0.8f) {
			m_spawnEnemies = m_game->BossDestroyerSpawnEnemy();
			m_turret[m_repairingTurret]->m_isDeatroyed = false;
			m_turret[m_repairingTurret]->m_isRepairing = true;
			m_stage++;
		}
	}
	else if (m_stage == 4) {
		if (m_position.x > 180.f) {
			m_position.x -= 8.f * deltaTime;
			for (int i = 0; i < DESTROYER_TURRET_AMOUNT; i++) {
				if (m_turret[i]) {
					m_turret[i]->m_position.x -= 8.f * deltaTime;
				}
			}
		}

		// check enemy
		for (int i = 0; i < DESTROYER_ENEMY_AMOUNT; i++) {
			if (m_spawnEnemies[i] && m_spawnEnemies[i]->m_isGarbage) {
				m_spawnEnemies[i] = nullptr;
			}
		}

		// shoot laser
		m_laserTime -= deltaTime;
		if (m_laserTime <= 0.f) {
			m_game->BossDestroyerShootLaser( this );
			m_laserTime = m_laserCoolDown;
			m_nextLaserY1 = m_game->m_randNumGen->RollRandomFloatInRange( 0.f, 100.f );
			m_nextLaserY2 = m_game->m_randNumGen->RollRandomFloatInRange( 0.f, 100.f );
		}

		m_stage4SpawnSpawnTime -= deltaTime;
		if (m_stage4SpawnSpawnTime <= 0.f) {
			m_game->BossDestroyerSpawnEnemy();
			m_stage4SpawnSpawnTime = m_stage4SpawnCooldown;
		}

		// repair turret
		m_repairTime -= deltaTime;
		m_healthAccumulation += 4.f * deltaTime;
		if (m_healthAccumulation >= 1.f) {
			m_healthAccumulation -= 1.f;
			m_turret[m_repairingTurret]->m_health += 1;
			if (m_turret[m_repairingTurret]->m_health > TURRET_HEALTH) {
				m_turret[m_repairingTurret]->m_health = TURRET_HEALTH;
			}
		}
		if (m_repairTime <= 0.f) {
			m_repairTime = m_repairChange;
			int m = m_game->m_randNumGen->RollRandomIntInRange( 0, 7 );
			if (m_turret[m] && m_turret[m]->m_isDeatroyed) {
				m_turret[m_repairingTurret]->m_isRepairing = false;
				m_repairingTurret = m;
				m_turret[m]->m_isDeatroyed = false;
				m_turret[m]->m_isRepairing = true;
			}
			else if(m_turret[m]){
				m_turret[m_repairingTurret]->m_isDeatroyed = false;
				m_turret[m_repairingTurret]->m_isRepairing = true;
			}
		}

		if (m_health <= 0) {
			for (int i = 0; i < DESTROYER_TURRET_AMOUNT; i++) {
				if (m_turret[i]) {
					m_turret[i]->Die();
				}
			}
			for (int i = 0; i < DESTROYER_ENEMY_AMOUNT; i++) {
				if (m_spawnEnemies[i]) {
					m_spawnEnemies[i]->Die();
				}
			}
			Die();
		}
	}
}

void BossDestroyer::Render() const
{
	Rgba8 shipColor( 128, 128, 128, 255 );

	Vertex_PCU shipVerts[33] = {
		Vertex_PCU( Vec3( -65.f,  5.f, 0.f ), shipColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -65.f,  -5.f, 0.f ), shipColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -10.f,  40.f, 0.f ), shipColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -65.f,  -5.f, 0.f ), shipColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -10.f, 40.f, 0.f ), shipColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -10.f,  -40.f, 0.f ), shipColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -10.f, 40.f, 0.f ), shipColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 30.f, 40.f, 0.f ), shipColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 30.f,  -40.f, 0.f ), shipColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -10.f, 40.f, 0.f ), shipColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -10.f, -40.f, 0.f ), shipColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 30.f,  -40.f, 0.f ), shipColor, Vec2( 0.f, 0.f ) ),

		Vertex_PCU( Vec3( 0.f,  2.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 2.f,  1.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f,  1.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f,  1.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f, -1.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f,  1.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f, -1.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -1.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f,  1.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f,  1.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 1.f,  0.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -1.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f, -1.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 2.f, -1.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -2.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),

		Vertex_PCU( Vec3( 1.3f, -2.6f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 2.5f, -0.3f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -2.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.7f, -1.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.9f, -3.3f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.1f, -1.8f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
	};
	TransformVertexArrayXY3D( 12, shipVerts, 1.f, 0.f, m_position );
	TransformVertexArrayXY3D( 21, &shipVerts[12], 1.5f, m_orientationDegrees + 180.f, m_position );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 33, shipVerts );

	if (m_laserTime <= m_laserCoolDown / 2.f) {
		DebugDrawLine( Vec2( 300, m_nextLaserY1 ), Vec2( -100, m_nextLaserY1 ), 1, Rgba8( 255, 0, 0, 128 ) );
		DebugDrawLine( Vec2( 300, m_nextLaserY2 ), Vec2( -100, m_nextLaserY2 ), 1, Rgba8( 255, 0, 0, 128 ) );
	}
}

void BossDestroyer::Die()
{
	m_game->SpawnDebris( this );
	m_isDead = true;
}

void BossDestroyer::RenderUI() const
{
	if (m_isDead || m_isGarbage || m_health <= 0.f) {
		return;
	}
	float remainHealthRatio = (float)m_health / (float)m_maxHealth;
	if (remainHealthRatio < 0.f) {
		remainHealthRatio = 0.f;
	}
	float remainShieldRatio = (float)m_shieldHealth / (float)m_maxShieldHealth;
	if (remainShieldRatio < 0.f) {
		remainShieldRatio = 0.f;
	}
	Vertex_PCU healthBarVerts[18];
	healthBarVerts[0] = Vertex_PCU( Vec3( -1.f, -0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[1] = Vertex_PCU( Vec3( 1.f, -0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[2] = Vertex_PCU( Vec3( -1.f, 0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[3] = Vertex_PCU( Vec3( 1.f, -0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[4] = Vertex_PCU( Vec3( -1.f, 0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[5] = Vertex_PCU( Vec3( 1.f, 0.2f, 0 ), Rgba8( 255, 0, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[6] = Vertex_PCU( Vec3( -1.f, -0.2f, 0 ), Rgba8( 0, 255, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[7] = Vertex_PCU( Vec3( 2 * remainHealthRatio - 1, -0.2f, 0 ), Rgba8( 0, 255, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[8] = Vertex_PCU( Vec3( -1.f, 0.2f, 0 ), Rgba8( 0, 255, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[9] = Vertex_PCU( Vec3( 2 * remainHealthRatio - 1, -0.2f, 0 ), Rgba8( 0, 255, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[10] = Vertex_PCU( Vec3( -1.f, 0.2f, 0 ), Rgba8( 0, 255, 0, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[11] = Vertex_PCU( Vec3( 2 * remainHealthRatio - 1, 0.2f, 0 ), Rgba8( 0, 255, 0, 255 ), Vec2( 0, 0 ) );

	healthBarVerts[12] = Vertex_PCU( Vec3( -1.f, -0.2f, 0 ), Rgba8( 102, 178, 255, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[13] = Vertex_PCU( Vec3( 2 * remainShieldRatio - 1, -0.2f, 0 ), Rgba8( 102, 178, 255, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[14] = Vertex_PCU( Vec3( -1.f, 0.2f, 0 ), Rgba8( 102, 178, 255, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[15] = Vertex_PCU( Vec3( 2 * remainShieldRatio - 1, -0.2f, 0 ), Rgba8( 102, 178, 255, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[16] = Vertex_PCU( Vec3( -1.f, 0.2f, 0 ), Rgba8( 102, 178, 255, 255 ), Vec2( 0, 0 ) );
	healthBarVerts[17] = Vertex_PCU( Vec3( 2 * remainShieldRatio - 1, 0.2f, 0 ), Rgba8( 102, 178, 255, 255 ), Vec2( 0, 0 ) );
	if (m_maxShieldHealth == 0.f) {
		TransformVertexArrayXY3D( 12, healthBarVerts, 2.f, 0.f, m_position + Vec2( 0, m_cosmeticRadius + 0.5f ) );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( 12, healthBarVerts );
	}
	else {
		TransformVertexArrayXY3D( 18, healthBarVerts, 2.f, 0.f, m_position + Vec2( 0, m_cosmeticRadius + 0.5f ) );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( 18, healthBarVerts );
	}
}

void BossDestroyer::BeAttacked( int hit )
{
	if (m_stage < 3) {
		return;
	}
	if (m_shieldHealth > 0.f) {
		m_shieldHealth -= (float)hit;
		if (m_shieldHealth <= 0.f) {
			m_maxShieldHealth = 0.f;
		}
	}
	else {
		m_health -= hit;
	}
}
