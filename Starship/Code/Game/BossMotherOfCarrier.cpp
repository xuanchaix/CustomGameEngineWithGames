#include "Game/BossMotherOfCarrier.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/App.hpp"
#include "Game/SummonLight.hpp"
#include "Game/Carrier.hpp"
#include "Game/Fighter.hpp"
#include "Game/Shield.hpp"

BossMotherOfCarrier::BossMotherOfCarrier( Vec2 startPos, Game* game, PlayerShip* playerShip )
	:Entity( startPos, game ), m_playerShip(playerShip)
{
	m_accelerateVelocity = Vec2( 0, 0 );
	m_angularVelocity = 90.f;
	m_physicsRadius = BOSS_MOTHER_PHYSICS_RADIUS;
	m_cosmeticRadius = BOSS_MOTHER_COSMETIC_RADIUS;
	m_health = BOSS_MOTHER_HEALTH;
	m_maxHealth = BOSS_MOTHER_HEALTH;
	m_type = EntityType::boss;
	m_color = Rgba8( 229, 204, 255, 255 );
	m_isActor = true;
	m_isEnemy = true;
	m_state = 1;
	do {
		m_targetWonderingDestination = Vec2( game->m_randNumGen->RollRandomFloatInRange( 5.f, WORLD_SIZE_X - 5.f ), game->m_randNumGen->RollRandomFloatInRange( 5.f, WORLD_SIZE_Y - 5.f ) );
	} while (GetDistanceSquared2D( m_targetWonderingDestination, playerShip->m_position ) < 3600.f);
	m_orientationDegrees = (m_targetWonderingDestination - m_position).GetOrientationDegrees();
	m_velocity = (m_targetWonderingDestination - m_position).GetNormalized() * BOSS_MOTHER_SPEED;
}

BossMotherOfCarrier::~BossMotherOfCarrier()
{

}

void BossMotherOfCarrier::Update()
{
	float deltaTime = g_theGame->m_gameClock->GetDeltaSeconds();
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_health <= 0) {
		Die();
		return;
	}

	m_internalTimer += deltaTime;
	if (m_rushTimer > 0.f) {
		m_rushTimer -= deltaTime;
	}
	// go to wondering destination
	if (m_state == 1) {
		// check if enter destination or too near player ship
		if (IsPointInsideDisc2D( m_targetWonderingDestination, m_position, m_physicsRadius ) || GetDistanceSquared2D( m_targetWonderingDestination, m_playerShip->m_position ) < 1600.f) {
			// choose another position
			do {
				m_targetWonderingDestination = Vec2( m_game->m_randNumGen->RollRandomFloatInRange( 5.f, WORLD_SIZE_X - 5.f ), m_game->m_randNumGen->RollRandomFloatInRange( 5.f, WORLD_SIZE_Y - 5.f ) );
			} while (GetDistanceSquared2D( m_targetWonderingDestination, m_playerShip->m_position ) < 3600.f);
		}
		Vec2 vecTowardsDest = m_targetWonderingDestination - m_position;
		if (abs( GetShortestAngularDispDegrees( vecTowardsDest.GetOrientationDegrees(), m_orientationDegrees ) ) > 15.f) {
			// only rotate
			m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, vecTowardsDest.GetOrientationDegrees(), m_angularVelocity * deltaTime );
			m_velocity = Vec2( 0, 0 );
		}
		else {
			m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, vecTowardsDest.GetOrientationDegrees(), m_angularVelocity * deltaTime );
			m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, BOSS_MOTHER_SPEED );
		}

		if (m_rushTimer <= 0.f && GetDistanceSquared2D( m_position, m_playerShip->m_position ) < 400.f) {
			m_state = 4;
			m_internalTimer = 0.f;
		}
		else if (m_internalTimer > m_state1Time) {
			if (m_shouldGoToState3) {
				m_state = 3;
			}
			else {
				m_state = 2;
				constexpr int numOfCarriersToSpawn = 5;
				for (int i = 0; i < numOfCarriersToSpawn; i++) {
					Vec2 targetPos = m_position + Vec2::MakeFromPolarDegrees( 360.f / (float)numOfCarriersToSpawn * i, 3.f );
					Entity* effect = new SummonLight( m_position, m_game, targetPos, m_state2Time, new Carrier( targetPos, m_game, m_playerShip ), Rgba8( 153, 51, 255 ) );
					m_game->SpawnNewDebris( effect );
				}
			}
			m_internalTimer = 0.f;
		}
		m_position += m_velocity * deltaTime;
	}
	// spawn carrier
	else if (m_state == 2) {
		// spawn 5 carriers
		m_velocity = Vec2( 0, 0 );

		if (m_internalTimer > m_state2Time) {
			m_state = 1;
			m_shouldGoToState3 = true;
			m_internalTimer = 0.f;
		}
	}
	// spawn fighter and go to wondering destination
	else if (m_state == 3) {
		// check if enter destination or too near player ship
		if (IsPointInsideDisc2D( m_targetWonderingDestination, m_position, m_physicsRadius ) || GetDistanceSquared2D( m_targetWonderingDestination, m_playerShip->m_position ) < 1600.f) {
			// choose another position
			do {
				m_targetWonderingDestination = Vec2( m_game->m_randNumGen->RollRandomFloatInRange( 5.f, WORLD_SIZE_X - 5.f ), m_game->m_randNumGen->RollRandomFloatInRange( 5.f, WORLD_SIZE_Y - 5.f ) );
			} while (GetDistanceSquared2D( m_targetWonderingDestination, m_playerShip->m_position ) < 3600.f);
		}
		Vec2 vecTowardsDest = m_targetWonderingDestination - m_position;
		if (abs( GetShortestAngularDispDegrees( vecTowardsDest.GetOrientationDegrees(), m_orientationDegrees ) ) > 15.f) {
			// only rotate
			m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, vecTowardsDest.GetOrientationDegrees(), m_angularVelocity * deltaTime );
			m_velocity = Vec2( 0, 0 );
		}
		else {
			m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, vecTowardsDest.GetOrientationDegrees(), m_angularVelocity * deltaTime );
			m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, BOSS_MOTHER_SPEED );
		}

		// spawn fighters
		m_spawnFighterTimer -= deltaTime;
		if (m_spawnFighterTimer <= 0.f) {
			m_spawnFighterTimer = 0.18f;
			m_game->SpawnNewEntity( new Fighter( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + m_game->m_randNumGen->RollRandomFloatInRange( 0.f, 360.f ), 2.f ), m_game, m_playerShip ) );
		}

		if (m_internalTimer > m_state3Time) {
			m_state = 1;
			m_shouldGoToState3 = false;
			m_internalTimer = 0.f;
		}
		m_position += m_velocity * deltaTime;
	}
	// rush to player
	else if (m_state == 4) {
		if (m_internalTimer < 2.5f) {
			Vec2 vecTowardsDest = m_playerShip->m_position - m_position;
			m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, vecTowardsDest.GetOrientationDegrees(), m_angularVelocity * deltaTime );
			m_rushTargetPos = m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees, 50.f );
			m_rushStartPos = m_position;
			DebugDrawLine( m_position, m_rushTargetPos, 5.f, Rgba8( 224, 224, 244, 200 ) );
		}
		else {
			float ratio = (m_internalTimer - 2.5f) / (m_state4Time - 2.5f);
			m_position = (1 - ratio) * m_rushStartPos + ratio * m_rushTargetPos;
			DebugDrawLine( m_position, m_rushTargetPos, 5.f, Rgba8( 224, 224, 244, 200 ) );
		}

		if (m_internalTimer > m_state4Time) {
			m_rushTimer = m_rushCoolDown;
			if (m_shouldGoToState3) {
				m_state = 3;
				m_internalTimer = 0.f;
			}
			else {
				m_state = 1;
				m_internalTimer = 2.f;
			}
		}
	}

	// shield
	if (!m_hasShield) {
		m_shieldCoolDownTimer += deltaTime;
	}
	else {
		m_shieldLastTimer += deltaTime;
		m_shieldAddHealthTimer += deltaTime;
		if (m_shieldAddHealthTimer > 0.59f) {
			m_shieldAddHealthTimer = 0.f;
			m_health = m_health + 1 > m_maxHealth ? m_maxHealth : m_health + 1;
		}
		if (m_shieldLastTimer > 3.f) {
			m_shieldLastTimer = 0.f;
			m_hasShield = false;
			m_shieldEntity->Die();
			m_shieldEntity = nullptr;
		}
	}
}

void BossMotherOfCarrier::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 300 );
	AddVertsForCapsule2D( verts, Vec2( 3.f, 0.f ), Vec2( -3.f, 0.f ), 2.5f, m_color );
	AddVertsForAABB2D( verts, AABB2( Vec2( 3.f, 1.f ), Vec2( 4.f, 2.f ) ), Rgba8( 255, 30, 30 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( 3.f, -1.f ), Vec2( 4.f, -2.f ) ), Rgba8( 255, 30, 30 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( 1.5f, 1.f ), Vec2( 2.5f, 2.f ) ), Rgba8( 255, 30, 30 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( 1.5f, -1.f ), Vec2( 2.5f, -2.f ) ), Rgba8( 255, 30, 30 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( 0.f, 1.f ), Vec2( 1.f, 2.f ) ), Rgba8( 255, 30, 30 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( 0.f, -1.f ), Vec2( 1.f, -2.f ) ), Rgba8( 255, 30, 30 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( -1.5f, 1.f ), Vec2( -0.5f, 2.f ) ), Rgba8( 255, 30, 30 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( -1.5f, -1.f ), Vec2( -0.5f, -2.f ) ), Rgba8( 255, 30, 30 ) );

	TransformVertexArrayXY3D( (int)verts.size(), verts.data(), 1.f, m_orientationDegrees, m_position );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void BossMotherOfCarrier::Die()
{
	m_game->SpawnDebris( this );
	m_isDead = true;
}

void BossMotherOfCarrier::BeAttacked( int hit )
{
	if (m_hasShield) {
		return;
	}
	if (m_shieldCoolDownTimer > m_shieldCoolDown && !m_hasShield) {
		m_hasShield = true;
		m_shieldEntity = new Shield( m_position, m_game, this, m_physicsRadius * 1.5f, Rgba8( 0, 255, 255, 150 ) );
		m_game->SpawnNewDebris( m_shieldEntity );
		m_shieldCoolDownTimer = 0.f;
	}
	else {
		m_health -= hit;
	}
}
