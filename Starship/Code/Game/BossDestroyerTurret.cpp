#include "Game/BossDestroyerTurret.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/App.hpp"
#include "Game/Shield.hpp"

BossDestroyerTurret::BossDestroyerTurret( Vec2 startPos, Game* game, PlayerShip* playership )
	:Entity(startPos, game)
{
	m_targetPlayerShip = playership;
	m_shootTimeCount = game->m_randNumGen->RollRandomFloatInRange( 0.f, m_shootCooldown );
	Vec2 forwardVector = playership->m_position - startPos;
	m_orientationDegrees = Atan2Degrees( forwardVector.y, forwardVector.x );
	m_velocity = Vec2( 0, 0 );
	m_accelerateVelocity = Vec2( 0, 0 );
	m_angularVelocity = 0.f;
	m_physicsRadius = TURRET_PHYSICS_RADIUS;
	m_cosmeticRadius = TURRET_COSMETIC_RADIUS;
	m_health = TURRET_HEALTH;
	m_maxHealth = TURRET_HEALTH;
	m_type = EntityType::bossTurret;
	m_color = Rgba8( 0x85, 0x00, 0x4b, 255 );
	m_isActor = true;
	m_shieldEntity = new Shield( m_position, m_game, this, m_physicsRadius * 1.6f, Rgba8( 0, 255, 255, 150 ) );
	m_game->SpawnNewDebris( m_shieldEntity );
}

BossDestroyerTurret::~BossDestroyerTurret()
{

}

void BossDestroyerTurret::Update()
{
	float deltaTime = g_theGame->m_gameClock->GetDeltaSeconds();
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_doNotShoot || m_isDeatroyed) {
		return;
	}
	if (m_health <= 0 && !m_isRepairing) {
		m_health = 0;
		m_isDeatroyed = true;
		return;
	}
	// face the player ship
	Vec2 forwardVector = m_targetPlayerShip->m_position - m_position;
	float targetOrientationDegrees = Atan2Degrees( forwardVector.y, forwardVector.x );
	m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, targetOrientationDegrees, ENEMY_TURN_SPEED * deltaTime );
	m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, BEETLE_SPEED );
	
	m_shootTimeCount -= deltaTime;
	if (m_shootTimeCount < 0.f) {
		m_shootTimeCount = m_shootCooldown;
		m_game->CreateBullet( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_cosmeticRadius ),
			Vec2::MakeFromPolarDegrees( m_orientationDegrees ) * BULLET_SPEED,
			m_orientationDegrees, true );
	}

	if (!m_hasShield && m_shieldTimeCount > 0.f) {
		m_shieldTimeCount -= deltaTime;
		if (m_shieldTimeCount <= 0.f) {
			m_hasShield = true;
			m_shieldEntity = new Shield( m_position, m_game, this, m_physicsRadius * 1.6f, Rgba8( 0, 255, 255, 150 ) );
			m_game->SpawnNewDebris( m_shieldEntity );
		}
	}

}

void BossDestroyerTurret::Render() const
{
	Vertex_PCU turretVerts[60];
	for (int i = 0; i < 16; i++) {
		turretVerts[3 * i].m_position = Vec3( 0, 0, 0 );
		turretVerts[3 * i].m_color = m_color;
		turretVerts[3 * i].m_uvTexCoords = Vec2( 0, 0 );
		turretVerts[3 * i + 1].m_position = Vec3( CosRadians( 6 * PI / 48 * i ) * m_cosmeticRadius, SinRadians( 6 * PI / 48 * i ) * m_cosmeticRadius, 0 );
		turretVerts[3 * i + 1].m_color = m_color;
		turretVerts[3 * i + 1].m_uvTexCoords = Vec2( 0, 0 );
		turretVerts[3 * i + 2].m_position = Vec3( CosRadians( 6 * PI / 48 * (i + 1) ) * m_cosmeticRadius, SinRadians( 6 * PI / 48 * (i + 1) ) * m_cosmeticRadius, 0 );
		turretVerts[3 * i + 2].m_color = m_color;
		turretVerts[3 * i + 2].m_uvTexCoords = Vec2( 0, 0 );
	}
	if (!m_isDeatroyed) {
		Rgba8 gunColor( 0x20, 0xcc, 0x20, 255 );
		Rgba8 turretColor( 0xff, 0xff, 0x50, 255 );
		turretVerts[48] = Vertex_PCU( Vec2( -1, -1 ), turretColor );
		turretVerts[49] = Vertex_PCU( Vec2( 1, -1 ), turretColor );
		turretVerts[50] = Vertex_PCU( Vec2( -1, 1 ), turretColor );
		turretVerts[51] = Vertex_PCU( Vec2( 1, -1 ), turretColor );
		turretVerts[52] = Vertex_PCU( Vec2( -1, 1 ), turretColor );
		turretVerts[53] = Vertex_PCU( Vec2( 1, 1 ), turretColor );


		turretVerts[54] = Vertex_PCU( Vec2( 1.f, 0.5f ), gunColor );
		turretVerts[55] = Vertex_PCU( Vec2( 1.f, -0.5f ), gunColor );
		turretVerts[56] = Vertex_PCU( Vec2( 4.f, 0.5f ), gunColor );
		turretVerts[57] = Vertex_PCU( Vec2( 4.f, 0.5f ), gunColor );
		turretVerts[58] = Vertex_PCU( Vec2( 4.f, -0.5f ), gunColor );
		turretVerts[59] = Vertex_PCU( Vec2( 1.f, -0.5f ), gunColor );
		TransformVertexArrayXY3D( 60, turretVerts, 1.f, m_orientationDegrees, m_position );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( 60, turretVerts );
	}
	else {
		TransformVertexArrayXY3D( 48, turretVerts, 1.f, m_orientationDegrees, m_position );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( 48, turretVerts );
	}

	//if (m_hasShield) {
		//DebugDrawRing( m_position, m_cosmeticRadius + 0.3f, 0.4f, Rgba8( 100, 100, 255, 128 ) );
	//}
}

void BossDestroyerTurret::Die()
{
	m_game->SpawnDebris( this );
	m_isDead = true;
	g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::enemyDie ) );
}

void BossDestroyerTurret::BeAttacked( int hit )
{
	if (m_doNotShoot || m_isDeatroyed) {
		return;
	}
	if (m_hasShield) {
		m_hasShield = false;
		m_shieldEntity->Die();
		m_shieldEntity = nullptr;
		m_shieldTimeCount = m_shieldCoolDown;
	}
	else {
		m_health -= hit;
	}
}
