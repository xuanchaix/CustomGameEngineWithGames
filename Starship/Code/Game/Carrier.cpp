#include "Game/Carrier.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Fighter.hpp"

Carrier::Carrier( Vec2 startPos, Game* game, PlayerShip* playership )
	:Entity(startPos, game), m_targetPlayerShip(playership)
{
	m_accelerateVelocity = Vec2( 0, 0 );
	m_angularVelocity = 60.f;
	m_physicsRadius = CARRIER_PHYSICS_RADIUS;
	m_cosmeticRadius = CARRIER_COSMETIC_RADIUS;
	m_health = CARRIER_HEALTH;
	m_maxHealth = CARRIER_HEALTH;
	m_type = EntityType::carrier;
	m_color = Rgba8( 204, 153, 255, 255 );
	m_isActor = true;
	m_isEnemy = true;
	m_state = 1;
	do {
		m_targetWonderingDestination = Vec2( game->m_randNumGen->RollRandomFloatInRange( 5.f, WORLD_SIZE_X - 5.f ), game->m_randNumGen->RollRandomFloatInRange( 5.f, WORLD_SIZE_Y - 5.f ) );
	} while (GetDistanceSquared2D( m_targetWonderingDestination, playership->m_position ) < 1600.f);
	m_orientationDegrees = (m_targetWonderingDestination - m_position).GetOrientationDegrees();
	m_velocity = (m_targetWonderingDestination - m_position).GetNormalized() * CARRIER_SPEED;
}

Carrier::~Carrier()
{

}

void Carrier::Update()
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
	constexpr float MAX_DODGE_DISTANCE = 400.f;
	constexpr float DODGE_ACCELERATE = 30.f;
	if (m_state == 1) {
		Entity* nearestBullet = m_game->GetNearestPlayerBullet( m_position );
		if (nearestBullet && GetDistanceSquared2D( nearestBullet->m_position, m_position ) < MAX_DODGE_DISTANCE) {
			m_state = 2;
			float forwardDegrees = (nearestBullet->m_position - m_position).GetOrientationDegrees();
			float degreesBetweenForward = GetShortestAngularDispDegrees( m_orientationDegrees, forwardDegrees );
			float angularDegrees = 0.f;
			if (degreesBetweenForward > 0.f) {
				angularDegrees = forwardDegrees - 90.f;
			}
			else {
				angularDegrees = forwardDegrees + 90.f;
			}
			m_velocity += Vec2::MakeFromPolarDegrees( angularDegrees, DODGE_ACCELERATE ) * deltaTime;
			m_velocity.ClampLength( CARRIER_SPEED );
			m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, angularDegrees, m_angularVelocity * deltaTime );
		}
		else {
			// check if enter destination
			if (IsPointInsideDisc2D( m_targetWonderingDestination, m_position, m_physicsRadius ) || GetDistanceSquared2D( m_targetWonderingDestination, m_targetPlayerShip->m_position ) < 900.f) {
				// choose another position
				do {
					m_targetWonderingDestination = Vec2( m_game->m_randNumGen->RollRandomFloatInRange( 5.f, WORLD_SIZE_X - 5.f ), m_game->m_randNumGen->RollRandomFloatInRange( 5.f, WORLD_SIZE_Y - 5.f ) );
				} while (GetDistanceSquared2D( m_targetWonderingDestination, m_targetPlayerShip->m_position ) < 1600.f);
			}
			Vec2 vecTowardsDest = m_targetWonderingDestination - m_position;
			if (abs( GetShortestAngularDispDegrees( vecTowardsDest.GetOrientationDegrees(), m_orientationDegrees ) ) > 10.f) {
				// only rotate
				m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, vecTowardsDest.GetOrientationDegrees(), m_angularVelocity * deltaTime );
				m_velocity = Vec2( 0, 0 );
			}
			else {
				m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, vecTowardsDest.GetOrientationDegrees(), m_angularVelocity * deltaTime );
				m_velocity = vecTowardsDest.GetNormalized() * CARRIER_SPEED;
			}
		}
	}
	else if (m_state == 2) {
		// dodging
		Entity* nearestBullet = m_game->GetNearestPlayerBullet( m_position );
		if (nearestBullet && GetDistanceSquared2D( nearestBullet->m_position, m_position ) < MAX_DODGE_DISTANCE) {
			float forwardDegrees = (nearestBullet->m_position - m_position).GetOrientationDegrees();
			float degreesBetweenForward = GetShortestAngularDispDegrees( m_orientationDegrees, forwardDegrees );
			float angularDegrees = 0.f;
			if (degreesBetweenForward > 0.f) {
				angularDegrees = forwardDegrees - 90.f;
			}
			else {
				angularDegrees = forwardDegrees + 90.f;
			}
			m_velocity += Vec2::MakeFromPolarDegrees( angularDegrees, DODGE_ACCELERATE ) * deltaTime;
			m_velocity.ClampLength( CARRIER_SPEED );
			m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, angularDegrees, m_angularVelocity * deltaTime );
		}
		else {
			m_state = 1;
		}
	}

	m_position += m_velocity * deltaTime;

	m_fighterSpawnTimer -= deltaTime;
	if (m_fighterSpawnTimer <= 0.f) {
		m_fighterSpawnTimer = m_fighterSpawnCooldown;
		m_game->SpawnNewEntity( new Fighter( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + 90.f, 2.f ), m_game, m_targetPlayerShip ) );
		m_game->SpawnNewEntity( new Fighter( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees - 90.f, 2.f ), m_game, m_targetPlayerShip ) );
	}
}

void Carrier::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 300 );
	AddVertsForCapsule2D( verts, m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees, 1.6f ), m_position - Vec2::MakeFromPolarDegrees( m_orientationDegrees, 1.6f ), 1.2f, m_color );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void Carrier::Die()
{
	m_game->SpawnDebris( this );
	m_isDead = true;
	g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::enemyDie ) );
}

