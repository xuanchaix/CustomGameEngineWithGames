#include "Game/Fighter.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"

Fighter::Fighter( Vec2 startPos, Game* game, PlayerShip* playership )
	:Entity( startPos, game ), m_targetPlayerShip( playership )
{
	Vec2 forwardVector = playership->m_position - startPos;
	m_orientationDegrees = (m_targetPlayerShip->m_position - m_position).GetOrientationDegrees();
	m_velocity = (m_targetPlayerShip->m_position - m_position).GetNormalized() * 0.5f;
	m_accelerateVelocity = Vec2( 0, 0 );
	m_angularVelocity = 180.f;
	m_physicsRadius = FIGHTER_PHYSICS_RADIUS;
	m_cosmeticRadius = FIGHTER_COSMETIC_RADIUS;
	m_health = FIGHTER_HEALTH;
	m_maxHealth = FIGHTER_HEALTH;
	m_type = EntityType::fighter;
	m_color = Rgba8( 127, 0, 255, 255 );
	m_isActor = true;
	m_isEnemy = true;
	m_state = 1;
	m_shootTimer = m_game->m_randNumGen->RollRandomFloatInRange( 1.f, m_shootCooldown );
}

Fighter::~Fighter()
{

}

void Fighter::Update()
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
	Vec2 vecTowardsDest = m_targetPlayerShip->m_position - m_position;
	if (abs( GetShortestAngularDispDegrees( vecTowardsDest.GetOrientationDegrees(), m_orientationDegrees ) ) > 15.f) {
		// speed down and rotate
		m_velocity -= m_velocity * 0.5f * deltaTime;
		if (m_velocity.GetLengthSquared() < 100.f) {
			m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, vecTowardsDest.GetOrientationDegrees(), m_angularVelocity * deltaTime );
		}
	}
	else {
		m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, vecTowardsDest.GetOrientationDegrees(), m_angularVelocity * deltaTime );
		m_velocity += vecTowardsDest.GetNormalized() * 20.f * deltaTime;
		m_velocity.ClampLength( FIGHTER_SPEED );
	}
	m_position += m_velocity * deltaTime;

	m_shootTimer -= deltaTime;
	if (m_shootTimer < 0.f) {
		m_shootTimer = m_shootCooldown;
		m_game->CreateBullet( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_cosmeticRadius ),
			Vec2::MakeFromPolarDegrees( m_orientationDegrees ) * BULLET_SPEED,
			m_orientationDegrees, true );
	}
}

void Fighter::Render() const
{
	Vertex_PCU fighterVerts[3] = {
		Vertex_PCU( Vec2( 0.f, 1.f ), m_color ),
		Vertex_PCU( Vec2( 0.f, -1.f ), m_color ),
		Vertex_PCU( Vec2( 2.f, 0.f ), m_color ),
	};
	TransformVertexArrayXY3D( 3, fighterVerts, 1.f, m_orientationDegrees, m_position );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 3, fighterVerts );
}

void Fighter::Die()
{
	m_game->SpawnDebris( this );
	m_isDead = true;
	g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::enemyDie ) );
}
