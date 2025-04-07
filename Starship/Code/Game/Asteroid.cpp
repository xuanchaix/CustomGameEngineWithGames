#include "Game/Asteroid.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"

Asteroid::Asteroid( Vec2 startPos, Game* game):Entity(startPos, game)
{
	m_type = EntityType::asteroid;
	m_orientationDegrees = game->m_randNumGen->RollRandomFloatZeroToOne() * 360.f;

	m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees, 10.f);
	m_accelerateVelocity = Vec2( 0.f, 0.f );
	m_angularVelocity = game->m_randNumGen->RollRandomFloatInRange( -200.f, 200.f );
	m_physicsRadius = ASTEROID_PHYSICS_RADIUS;
	m_cosmeticRadius = ASTEROID_COSMETIC_RADIUS;
	m_health = ASTEROID_HEALTH;
	m_maxHealth = ASTEROID_HEALTH;

	m_color = Rgba8( 100, 100, 100, 255 );
	float randR[NUM_OF_ASTEROID_VERTS / 3];
	for (int i = 0; i < NUM_OF_ASTEROID_VERTS / 3; i++) {
		randR[i] = game->m_randNumGen->RollRandomFloatInRange( m_physicsRadius, m_cosmeticRadius );
	}
	for (int i = 0; i < NUM_OF_ASTEROID_VERTS / 3; i++) {
		asteroidVerts[3 * i].m_position = Vec3( 0, 0, 0 );
		asteroidVerts[3 * i].m_color = Rgba8( 100, 100, 100, 255 );
		asteroidVerts[3 * i].m_uvTexCoords = Vec2( 0, 0 );
		asteroidVerts[3 * i + 1].m_position = Vec3( CosRadians( 6 * PI / NUM_OF_ASTEROID_VERTS * i ) * randR[i], SinRadians( 6 * PI / NUM_OF_ASTEROID_VERTS * i ) * randR[i], 0 );
		asteroidVerts[3 * i + 1].m_color = Rgba8( 100, 100, 100, 255 );
		asteroidVerts[3 * i + 1].m_uvTexCoords = Vec2( 0, 0 );
		asteroidVerts[3 * i + 2].m_position = Vec3( CosRadians( 6 * PI / NUM_OF_ASTEROID_VERTS * (i + 1) ) * randR[(i + 1) % (NUM_OF_ASTEROID_VERTS / 3)], SinRadians( 6 * PI / NUM_OF_ASTEROID_VERTS * (i + 1) ) * randR[(i + 1) % (NUM_OF_ASTEROID_VERTS / 3)], 0 );
		asteroidVerts[3 * i + 2].m_color = Rgba8( 100, 100, 100, 255 );
		asteroidVerts[3 * i + 2].m_uvTexCoords = Vec2( 0, 0 );
	}
}

Asteroid::~Asteroid()
{

}

void Asteroid::Update()
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
	m_orientationDegrees += m_angularVelocity * deltaTime;
	if (IsOffscreen()) {
		if (m_position.x + m_cosmeticRadius < 0.f) {
			m_position.x = WORLD_SIZE_X + m_cosmeticRadius;
		}
		if (m_position.x - m_cosmeticRadius > WORLD_SIZE_X) {
			m_position.x = -m_cosmeticRadius;
		}
		if (m_position.y - m_cosmeticRadius > WORLD_SIZE_Y) {
			m_position.y = -m_cosmeticRadius;
		}
		if (m_position.y + m_cosmeticRadius < 0.f) {
			m_position.y = WORLD_SIZE_Y + m_cosmeticRadius;
		}
	}
}

void Asteroid::Render() const
{
	Vertex_PCU temp_asteroidVerts[NUM_OF_ASTEROID_VERTS];
	for (int i = 0; i < NUM_OF_ASTEROID_VERTS; i++) {
		temp_asteroidVerts[i] = asteroidVerts[i];
	}
	TransformVertexArrayXY3D( NUM_OF_ASTEROID_VERTS, temp_asteroidVerts, 1.f, m_orientationDegrees, m_position );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( NUM_OF_ASTEROID_VERTS, temp_asteroidVerts );
}

void Asteroid::Die()
{
	m_game->SpawnDebris( this );
	m_isDead = true;
	g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::enemyDie ) );
}

