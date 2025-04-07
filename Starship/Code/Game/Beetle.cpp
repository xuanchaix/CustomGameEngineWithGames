#include "Game/Beetle.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/App.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"


Beetle::Beetle( Vec2 startPos, Game* game, PlayerShip* playership ):Entity(startPos, game), m_targetPlayerShip(playership)
{
	Vec2 forwardVector = playership->m_position - startPos;
	m_orientationDegrees = Atan2Degrees( forwardVector.y, forwardVector.x );
	m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, BEETLE_SPEED );
	m_accelerateVelocity = Vec2( 0, 0 );
	m_angularVelocity = 0.f;
	m_physicsRadius = BEETLE_PHYSICS_RADIUS;
	m_cosmeticRadius = BEETLE_COSMETIC_RADIUS;
	m_health = BEETLE_HEALTH;
	m_maxHealth = BEETLE_HEALTH;
	m_type = EntityType::beetle;
	m_color = Rgba8( 255, 255, 51, 255 );
	shootTimeCount = game->m_randNumGen->RollRandomFloatInRange( 1.f, shootCooldown );
	m_isActor = true;
	m_isEnemy = true;
}

Beetle::~Beetle()
{
}

void Beetle::Update()
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
	m_position += m_velocity * deltaTime;
	float disSquared = GetDistanceSquared2D( m_position, m_targetPlayerShip->m_position );
	// follow the player ship
	if (m_targetPlayerShip->IsAlive() && disSquared > 625.f) {
		Vec2 forwardVector = m_targetPlayerShip->m_position - m_position;
		float targetOrientationDegrees = Atan2Degrees( forwardVector.y, forwardVector.x );
		m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, targetOrientationDegrees, ENEMY_TURN_SPEED * deltaTime );
		m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, BEETLE_SPEED );
		//m_orientationDegrees = Atan2Degrees( forwardVector.y, forwardVector.x );
		//m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, BEETLE_SPEED );
	}
	else if (m_targetPlayerShip->IsAlive() && disSquared <= 625.f) {
		Vec2 forwardVector = m_targetPlayerShip->m_position - m_position;
		float targetOrientationDegrees = Atan2Degrees( forwardVector.y, forwardVector.x );
		m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, targetOrientationDegrees, ENEMY_TURN_SPEED * deltaTime );
		m_velocity = Vec2( 0.f, 0.f );
	}

	shootTimeCount -= deltaTime;
	if (shootTimeCount < 0.f) {
		shootTimeCount = shootCooldown;
		m_game->CreateBullet( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_cosmeticRadius ),
			Vec2::MakeFromPolarDegrees( m_orientationDegrees ) * BULLET_SPEED,
			m_orientationDegrees, true );
	}

}

void Beetle::Render() const
{
	Vertex_PCU beetleVerts[6] = {
		Vertex_PCU( Vec3( -1.5f, 2.0f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -1.5f, -2.0f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 1.5f, -1.0f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -1.5f, 2.0f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 1.5f, 1.0f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 1.5f, -1.0f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
	};
	TransformVertexArrayXY3D( 6, beetleVerts, 1.f, m_orientationDegrees, m_position );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 6, beetleVerts );
}

void Beetle::Die()
{
	m_game->SpawnDebris( this );
	m_isDead = true;
	g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::enemyDie ) );
}


