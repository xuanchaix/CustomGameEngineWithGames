#include "Game/BossFirstExplorer.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Game.hpp"

BossFirstExplorer::BossFirstExplorer( Vec2 startPos, Game* game, PlayerShip* playerShip )
	:Entity(startPos, game)
{
	Vec2 forwardVector = playerShip->m_position - startPos;
	m_playerShip = playerShip;
	m_orientationDegrees = Atan2Degrees( forwardVector.y, forwardVector.x );
	m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, BOSS_EXPLORER_SPEED );
	m_accelerateVelocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, BOSS_EXPLORER_ACCELERATE_SPEED );
	m_angularVelocity = 300.f;
	m_physicsRadius = BOSS_EXPLORER_PHYSICS_RADIUS;
	m_cosmeticRadius = BOSS_EXPLORER_COSMETIC_RADIUS;
	m_health = BOSS_EXPLORER_HEALTH;
	m_maxHealth = BOSS_EXPLORER_HEALTH;
	m_type = EntityType::boss;
	m_color = Rgba8( 0, 153, 0, 255 );
	m_isActor = true;
	m_isEnemy = true;
}

BossFirstExplorer::~BossFirstExplorer()
{

}

void BossFirstExplorer::Update()
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
	m_position += deltaTime * m_velocity;

	Vec2 forwardVector = m_playerShip->m_position - m_position;
	float targetOrientationDegrees = Atan2Degrees( forwardVector.y, forwardVector.x );
	m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, targetOrientationDegrees, ENEMY_TURN_SPEED * deltaTime );
	//Vec2 forwardVector = m_playerShip->m_position - m_position;
	//m_orientationDegrees = Atan2Degrees( forwardVector.y, forwardVector.x );
	DealCollidingEdge();

	// shoot bullet
	m_shootTimeCount -= deltaTime;
	m_timeAfterLastshoot += deltaTime;
	if (m_shootTimeCount < 0.f && m_shootDuration <= 0.f) {
		m_shootDuration = 1.f;
		m_shootTimeCount = 999999.f;
	}
	if (m_shootDuration > 0.f) {
		m_shootDuration -= deltaTime;
		if (m_timeAfterLastshoot > 0.02f) {
			float rand = m_game->m_randNumGen->RollRandomFloatInRange( -2.f, 2.f );
			Vec2 sideVector = Vec2::MakeFromPolarDegrees( m_orientationDegrees + 90.f, rand );
			m_game->CreateBullet( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_cosmeticRadius ) + sideVector, Vec2::MakeFromPolarDegrees( m_orientationDegrees ) * BULLET_SPEED * 2.5f, m_orientationDegrees, true );
			m_timeAfterLastshoot = 0.f;
		}
		if (m_shootDuration <= 0.f) {
			m_shootTimeCount = m_shootCooldown;
		}
	}

	// cone attack
	m_coneTimeCount -= deltaTime;
	if (m_coneTimeCount < 0.f) {
		m_coneTimeCount = m_coneCooldown;
		m_game->CreateCone( m_position, m_orientationDegrees, true, this, CONE_RADIUS + 5.f, CONE_APERTURE_DEGREES );
	}

	// spawn enemies
	if (m_health <= BOSS_SECOND_STAGE_HEALTH) {
		m_color = Rgba8( 255, 100, 100, 255 );
		m_spawnTimeCount -= deltaTime;
		if (m_spawnTimeCount < 0.f) {
			m_spawnTimeCount = m_spawnCooldown;
			m_game->ExplorerSpawnNewEnemy( this );
		}
	}

	if (m_health <= BOSS_THIRD_STAGE_HEALTH) {
		m_spawnCooldown = 3.f;
		m_healTimeCount -= deltaTime;
		if (m_healTimeCount < 0.f) {
			m_healTimeCount = m_healCooldown;
			m_game->ExplorerHealSelf( this, 50.f );
		}
	}
}

void BossFirstExplorer::Render() const
{
	if (m_health <= BOSS_THIRD_STAGE_HEALTH) {
		Vertex_PCU healthLine[300] = {};
		Vec2* posArray = m_game->GetEnemiesPosInRange( m_position, 50.f );
		int j = 0;
		for (int i = 0; i < 100; i++) {
			if (posArray[i] == Vec2( -1, -1 )) {
				break;
			}
			else {
				Vec2 basicNormal = (posArray[i] - m_position).GetRotated90Degrees().GetNormalized();
				healthLine[3 * i] = Vertex_PCU( Vec3( m_position.x + basicNormal.x, m_position.y + basicNormal.y, 0.f ), Rgba8( 255, 0, 0, 255 ), Vec2( 0.f, 0.f ) );
				healthLine[3 * i + 1] = Vertex_PCU( Vec3( m_position.x - basicNormal.x, m_position.y - basicNormal.y, 0.f ), Rgba8( 255, 0, 0, 255 ), Vec2( 0.f, 0.f ) );
				healthLine[3 * i + 2] = Vertex_PCU( Vec3( posArray[i].x, posArray[i].y, 0.f ), Rgba8( 255, 0, 0, 100 ), Vec2( 0.f, 0.f ) );
				j += 3;
			}
		}
		//delete posArray;
		delete[] posArray;
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( j, healthLine );
	}

	Rgba8 displayColor = m_color;
	if (m_coneTimeCount < 2.f) {//255, 51, 153
		float ratio = m_coneTimeCount / 2.f;
		displayColor.r = (unsigned char)Interpolate( 255, m_color.r, ratio );
		displayColor.g = (unsigned char)Interpolate( 255, m_color.g, ratio );
		displayColor.b = (unsigned char)Interpolate( 102, m_color.b, ratio );
		displayColor.a = (unsigned char)Interpolate( 255, m_color.a, ratio );
	}

	if (m_shootTimeCount < 2.f) {
		DebugDrawRing( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees ), m_shootTimeCount * 0.3f, m_shootTimeCount * 0.6f, Rgba8( 0, 0, 255 ) );
	}

	Vertex_PCU shipVerts[21] = {
		Vertex_PCU( Vec3( 0.f,  2.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 2.f,  1.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f,  1.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f,  1.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f, -1.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f,  1.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f, -1.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -1.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f,  1.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f,  1.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 1.f,  0.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -1.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f, -1.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 2.f, -1.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -2.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),

		Vertex_PCU( Vec3( 1.3f, -2.6f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 2.5f, -0.3f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -2.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.7f, -1.f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.9f, -3.3f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.1f, -1.8f, 0.f ), displayColor, Vec2( 0.f, 0.f ) ),
	};
	TransformVertexArrayXY3D( 21, shipVerts, 1.5f, m_orientationDegrees, m_position );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 21, shipVerts );
}

void BossFirstExplorer::Die()
{
	m_game->SpawnDebris( this );
	m_isDead = true;
}

/*void BossFirstExplorer::RenderUI() const
{

}*/

void BossFirstExplorer::DealCollidingEdge()
{
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