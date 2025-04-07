#include "Game/Debris.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

Debris::Debris( Vec2 startPos, Game* game, Vec2 baseVelocity, float baseRadius, Rgba8 color ):Entity(startPos, game)
{
	m_type = EntityType::debris;
	m_orientationDegrees = game->m_randNumGen->RollRandomFloatZeroToOne() * 360.f;
	
	float speed = game->m_randNumGen->RollRandomFloatInRange( 8.f, 15.f );
	m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, speed ) + baseVelocity;
	m_accelerateVelocity = Vec2( 0.f, 0.f );
	m_angularVelocity = 0.f;

	float radius = game->m_randNumGen->RollRandomFloatInRange( 0.1f * baseRadius, 0.5f * baseRadius );
	m_physicsRadius = radius;
	m_cosmeticRadius = radius * 1.2f;

	color.a = 127;
	m_color = color;
	float randR[NUM_OF_DEBRIS_VERTS / 3];
	for (int i = 0; i < NUM_OF_DEBRIS_VERTS / 3; i++) {
		randR[i] = game->m_randNumGen->RollRandomFloatInRange( 0.5f * m_physicsRadius, m_cosmeticRadius );
	}
	for (int i = 0; i < NUM_OF_DEBRIS_VERTS / 3; i++) {
		debrisVerts[3 * i].m_position = Vec3( 0, 0, 0 );
		debrisVerts[3 * i].m_color = color;
		debrisVerts[3 * i].m_uvTexCoords = Vec2( 0, 0 );
		debrisVerts[3 * i + 1].m_position = Vec3( CosRadians( 6 * PI / NUM_OF_DEBRIS_VERTS * i ) * randR[i], SinRadians( 6 * PI / NUM_OF_DEBRIS_VERTS * i ) * randR[i], 0 );
		debrisVerts[3 * i + 1].m_color = color;
		debrisVerts[3 * i + 1].m_uvTexCoords = Vec2( 0, 0 );
		debrisVerts[3 * i + 2].m_position = Vec3( CosRadians( 6 * PI / NUM_OF_DEBRIS_VERTS * (i + 1) ) * randR[(i + 1) % (NUM_OF_DEBRIS_VERTS / 3)], SinRadians( 6 * PI / NUM_OF_DEBRIS_VERTS * (i + 1) ) * randR[(i + 1) % (NUM_OF_DEBRIS_VERTS / 3)], 0 );
		debrisVerts[3 * i + 2].m_color = color;
		debrisVerts[3 * i + 2].m_uvTexCoords = Vec2( 0, 0 );
	}
}

Debris::~Debris()
{

}

void Debris::Update()
{
	float deltaTime = g_theGame->m_gameClock->GetDeltaSeconds();
	if (m_lifeSpan <= 0.f || 
		(m_position.x - m_cosmeticRadius > WORLD_SIZE_X + 5.f || m_position.x + m_cosmeticRadius < -5.f
			|| m_position.y - m_cosmeticRadius > WORLD_SIZE_Y + 5.f || m_position.y + m_cosmeticRadius < -5.f)
		) {
		Die();
		return;
	}
	m_lifeSpan -= deltaTime;
	m_position += m_velocity * deltaTime;
	if (m_lifeSpan < 0.f) {
		m_lifeSpan = 0.f;
	}
}

void Debris::Render() const
{
	Vertex_PCU temp_debrisVerts[NUM_OF_DEBRIS_VERTS];
	for (int i = 0; i < NUM_OF_DEBRIS_VERTS; i++) {
		temp_debrisVerts[i] = debrisVerts[i];
		temp_debrisVerts[i].m_color.a = (unsigned char)((float)temp_debrisVerts[i].m_color.a * (m_lifeSpan / DEBRIS_LIFETIME_SECONDS));
	}
	TransformVertexArrayXY3D( NUM_OF_DEBRIS_VERTS, temp_debrisVerts, 1.f, m_orientationDegrees, m_position );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( NUM_OF_DEBRIS_VERTS, temp_debrisVerts );
}

void Debris::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}
