#include "Game/Wasp.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/App.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

Wasp::Wasp( Vec2 startPos, Game* game, PlayerShip* playership ):Entity( startPos, game ), m_targetPlayerShip( playership )
{
	Vec2 forwardVector = playership->m_position - startPos;
	m_orientationDegrees = Atan2Degrees( forwardVector.y, forwardVector.x );
	m_accelerateVelocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, WASP_ACCELERATE_SPEED );
	m_velocity = Vec2( 0, 0 );
	m_angularVelocity = 0.f;
	m_physicsRadius = WASP_PHYSICS_RADIUS;
	m_cosmeticRadius = WASP_COSMETIC_RADIUS;
	m_health = WASP_HEALTH;
	m_maxHealth = WASP_HEALTH;
	m_type = EntityType::wasp;
	m_color = Rgba8( 51, 51, 255, 255 );
	shootTimeCount = game->m_randNumGen->RollRandomFloatInRange( 1.f, shootCooldown );
	m_isActor = true;
	m_isEnemy = true;
}

Wasp::~Wasp()
{

}

void Wasp::Update()
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
	m_velocity += m_accelerateVelocity * deltaTime;
	m_velocity.ClampLength( WASP_MAX_SPEED );
	m_position += m_velocity * deltaTime;
	//enemy shoot
	shootTimeCount -= deltaTime;
	if (shootTimeCount < 0.f) {
		shootTimeCount = shootCooldown;
		constexpr float bulletAngle = 30.f;
		m_game->CreateBullet( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_cosmeticRadius ),
			Vec2::MakeFromPolarDegrees( m_orientationDegrees + bulletAngle ) * BULLET_SPEED,
			m_orientationDegrees + bulletAngle, true );
		m_game->CreateBullet( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_cosmeticRadius ),
			Vec2::MakeFromPolarDegrees( m_orientationDegrees - bulletAngle ) * BULLET_SPEED,
			m_orientationDegrees - bulletAngle, true );
		m_game->CreateBullet( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_cosmeticRadius ),
			Vec2::MakeFromPolarDegrees( m_orientationDegrees ) * BULLET_SPEED,
			m_orientationDegrees, true );
	}

	// follow the player ship
	// turn and refresh accelerate speed
	Vec2 forwardVector = m_targetPlayerShip->m_position - m_position;
	float targetOrientationDegrees = Atan2Degrees( forwardVector.y, forwardVector.x );
	m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, targetOrientationDegrees, ENEMY_TURN_SPEED * deltaTime );
	m_accelerateVelocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, WASP_ACCELERATE_SPEED );

	// bounce
	if (m_position.x + m_physicsRadius > WORLD_SIZE_X) {
		m_position.x = WORLD_SIZE_X - m_physicsRadius;
		m_velocity.x = -m_velocity.x;
	}
	if (m_position.x - m_physicsRadius < 0) {
		m_position.x = m_physicsRadius;
		m_velocity.x = -m_velocity.x;
	}
	if (m_position.y + m_physicsRadius > WORLD_SIZE_Y) {
		m_position.y = WORLD_SIZE_Y - m_physicsRadius;
		m_velocity.y = -m_velocity.y;
	}
	if (m_position.y - m_physicsRadius < 0) {
		m_position.y = m_physicsRadius;
		m_velocity.y = -m_velocity.y;
	}
	
}

void Wasp::Render() const
{
	Vertex_PCU waspVerts[6] = {
		Vertex_PCU( Vec3( 0.f, 2.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -2.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 1.5f, 0.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, 1.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -1.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.5f, 0.f, 0.f ), m_color, Vec2( 0.f, 0.f ) ),
	};
	TransformVertexArrayXY3D( 6, waspVerts, 1.f, m_orientationDegrees, m_position );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 6, waspVerts );
}

void Wasp::Die()
{
	m_game->SpawnDebris( this );
	m_isDead = true;
	g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::enemyDie ) );
}

