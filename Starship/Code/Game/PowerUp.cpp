#include "Game/PowerUp.hpp"
#include "Game/Game.hpp"

PowerUp::PowerUp( Vec2 startPos, Game* game, PowerUpType type, Entity const* entity )
	:Entity(startPos, game)
{
	m_type = EntityType::powerUp;
	m_powerUpType = type;
	m_orientationDegrees = entity->m_orientationDegrees;

	m_velocity = Vec2( 0, 0 );
	if (m_velocity.GetLengthSquared() <= 9.f) {
		m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, 3.f );
	}
	m_angularVelocity = 120.f;
	m_accelerateVelocity = Vec2( 0.f, 0.f );
	m_physicsRadius = POWER_UP_PHYSICS_RADIUS;
	m_cosmeticRadius = POWER_UP_COSMETIC_RADIUS;
	m_health = 1;
	m_maxHealth = 1;

	if (m_powerUpType == PowerUpType::enhance) {
		m_color = Rgba8( 102, 255, 255, 128 ); 
		m_powerUpVerts[0] = Vertex_PCU( Vec3( 0.f, 2.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[1] = Vertex_PCU( Vec3( 2.f, 1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[2] = Vertex_PCU( Vec3( -2.f, 1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[3] = Vertex_PCU( Vec3( -2.f, 1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[4] = Vertex_PCU( Vec3( -2.f, -1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[5] = Vertex_PCU( Vec3( 0.f, 1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[6] = Vertex_PCU( Vec3( -2.f, -1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[7] = Vertex_PCU( Vec3( 0.f, -1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[8] = Vertex_PCU( Vec3( 0.f, 1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[9] = Vertex_PCU( Vec3( 0.f, 1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[10] = Vertex_PCU( Vec3( 1.f, 0.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[11] = Vertex_PCU( Vec3( 0.f, -1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[12] = Vertex_PCU( Vec3( -2.f, -1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[13] = Vertex_PCU( Vec3( 2.f, -1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[14] = Vertex_PCU( Vec3( 0.f, -2.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) );

		m_powerUpVerts[15] = Vertex_PCU( Vec3( 1.f, -1.f, 0.f ), Rgba8( 192, 192, 192, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[16] = Vertex_PCU( Vec3( -1.5f, -1.f, 0.f ), Rgba8( 192, 192, 192, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[17] = Vertex_PCU( Vec3( -5.f, -3.f, 0.f ), Rgba8( 192, 192, 192, 0 ), Vec2( 0.f, 0.f ) );

		m_powerUpVerts[18] = Vertex_PCU( Vec3( 1.f, 1.f, 0.f ), Rgba8( 192, 192, 192, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[19] = Vertex_PCU( Vec3( -1.5f, 1.f, 0.f ), Rgba8( 192, 192, 192, 255 ), Vec2( 0.f, 0.f ) );
		m_powerUpVerts[20] = Vertex_PCU( Vec3( -5.f, 3.f, 0.f ), Rgba8( 192, 192, 192, 0 ), Vec2( 0.f, 0.f ) );
		TransformVertexArrayXY3D( NUM_OF_POWER_UP_VERTS[(int)m_powerUpType], m_powerUpVerts, 0.8f, 0.f, Vec2( 0.f, 0.f ) );
	}
	else if (m_powerUpType == PowerUpType::health) {
		m_color = Rgba8( 51, 255, 51, 255 );
		m_powerUpVerts[0] = Vertex_PCU( Vec3( -POWER_UP_COSMETIC_RADIUS, POWER_UP_COSMETIC_RADIUS / 4.f, 0 ), m_color, Vec2( 0, 0 ) );
		m_powerUpVerts[1] = Vertex_PCU( Vec3( -POWER_UP_COSMETIC_RADIUS, -POWER_UP_COSMETIC_RADIUS / 4.f, 0 ), m_color, Vec2( 0, 0 ) );
		m_powerUpVerts[2] = Vertex_PCU( Vec3( POWER_UP_COSMETIC_RADIUS, POWER_UP_COSMETIC_RADIUS / 4.f, 0 ), m_color, Vec2( 0, 0 ) );
		m_powerUpVerts[3] = Vertex_PCU( Vec3( -POWER_UP_COSMETIC_RADIUS, -POWER_UP_COSMETIC_RADIUS / 4.f, 0 ), m_color, Vec2( 0, 0 ) );
		m_powerUpVerts[4] = Vertex_PCU( Vec3( POWER_UP_COSMETIC_RADIUS, POWER_UP_COSMETIC_RADIUS / 4.f, 0 ), m_color, Vec2( 0, 0 ) );
		m_powerUpVerts[5] = Vertex_PCU( Vec3( POWER_UP_COSMETIC_RADIUS, -POWER_UP_COSMETIC_RADIUS / 4.f, 0 ), m_color, Vec2( 0, 0 ) );
		m_powerUpVerts[6] = Vertex_PCU( Vec3( -POWER_UP_COSMETIC_RADIUS / 4.f, POWER_UP_COSMETIC_RADIUS, 0 ), m_color, Vec2( 0, 0 ) );
		m_powerUpVerts[7] = Vertex_PCU( Vec3( -POWER_UP_COSMETIC_RADIUS / 4.f, -POWER_UP_COSMETIC_RADIUS, 0 ), m_color, Vec2( 0, 0 ) );
		m_powerUpVerts[8] = Vertex_PCU( Vec3( POWER_UP_COSMETIC_RADIUS / 4.f, POWER_UP_COSMETIC_RADIUS, 0 ), m_color, Vec2( 0, 0 ) );
		m_powerUpVerts[9] = Vertex_PCU( Vec3( -POWER_UP_COSMETIC_RADIUS / 4.f, -POWER_UP_COSMETIC_RADIUS, 0 ), m_color, Vec2( 0, 0 ) );
		m_powerUpVerts[10] = Vertex_PCU( Vec3( POWER_UP_COSMETIC_RADIUS / 4.f, POWER_UP_COSMETIC_RADIUS, 0 ), m_color, Vec2( 0, 0 ) );
		m_powerUpVerts[11] = Vertex_PCU( Vec3( POWER_UP_COSMETIC_RADIUS / 4.f, -POWER_UP_COSMETIC_RADIUS, 0 ), m_color, Vec2( 0, 0 ) );
	}
	else if (m_powerUpType == PowerUpType::ammo) {
		m_color = Rgba8( 204, 255, 255, 128 );
		Rgba8 color1 = Rgba8( 255, 255, 0, 255 );
		Rgba8 color2 = Rgba8( 255, 0, 0, 255 );
		Rgba8 color3 = Rgba8( 255, 0, 0, 0 );
		m_powerUpVerts[0] = Vertex_PCU( Vec3( 0.f, 0.5f, 0.f ), color1, Vec2( 0.f, 0.f ) );
		m_powerUpVerts[1] = Vertex_PCU( Vec3( 0.5f, 0.f, 0.f ), color1, Vec2( 0.f, 0.f ) );
		m_powerUpVerts[2] = Vertex_PCU( Vec3( 0.f, -0.5f, 0.f ), color1, Vec2( 0.f, 0.f ) );
		m_powerUpVerts[3] = Vertex_PCU( Vec3( 0.f, 0.5f, 0.f ), color2, Vec2( 0.f, 0.f ) );
		m_powerUpVerts[4] = Vertex_PCU( Vec3( 0.f, -0.5f, 0.f ), color2, Vec2( 0.f, 0.f ) );
		m_powerUpVerts[5] = Vertex_PCU( Vec3( -2.f, 0.f, 0.f ), color3, Vec2( 0.f, 0.f ) );
		TransformVertexArrayXY3D( 6, m_powerUpVerts, 1.f, 90, Vec2( 0, 0 ) );
	}
}

PowerUp::~PowerUp()
{

}

void PowerUp::Update()
{
	float deltaTime = g_theGame->m_gameClock->GetDeltaSeconds();
	m_lifespan -= deltaTime;
	if (m_health <= 0) {
		Die();
		return;
	}
	if (IsOffscreen() || m_lifespan <= 0.f) {
		m_isGarbage = true;
		m_isDead = true;
		return;
	}
	m_orientationDegrees += m_angularVelocity * deltaTime;
}

void PowerUp::Render() const
{
	Vertex_PCU temp_powerUpVerts[48];
	for (int i = 0; i < NUM_OF_POWER_UP_VERTS[(int)m_powerUpType]; i++) {
		temp_powerUpVerts[i] = m_powerUpVerts[i];
	}
	DebugDrawRing( m_position, m_physicsRadius, 0.4f, m_color );
	TransformVertexArrayXY3D( NUM_OF_POWER_UP_VERTS[(int)m_powerUpType], temp_powerUpVerts, 1.f, m_orientationDegrees, m_position );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( NUM_OF_POWER_UP_VERTS[(int)m_powerUpType], temp_powerUpVerts );
}

void PowerUp::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}
