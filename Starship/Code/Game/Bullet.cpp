#include "Game/Bullet.hpp"
#include "Game/Game.hpp"


Bullet::Bullet( Vec2 startPos, Game* game, Vec2 velocity, float orientationDegree, bool isEnemyBullet ) :
	Entity( startPos, game )
{
	m_velocity = velocity;
	m_orientationDegrees = orientationDegree;
	m_accelerateVelocity = Vec2( 0, 0 );
	m_physicsRadius = BULLET_PHYSICS_RADIUS;
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
	m_health = 1;
	//m_color = Rgba8( 150, 100, 0, 255 );
	if (isEnemyBullet) {
		m_type = EntityType::enemyBullet;
		m_color = Rgba8( 0, 100, 150, 255 );
	}
	else {
		m_type = EntityType::bullet;
		m_color = Rgba8( 150, 100, 0, 255 );
	}
}




Bullet::~Bullet()
{

}

void Bullet::Update()
{
	float deltaTime = g_theGame->m_gameClock->GetDeltaSeconds();
	if (m_health <= 0) {
		Die();
		return;
	}
	if (IsOffscreen() || m_lifespan < 0.f) {
		m_isGarbage = true;
		m_isDead = true;
		return;
	}
	m_lifespan -= deltaTime;
	m_position += deltaTime * m_velocity;
}

void Bullet::Render() const
{
	Rgba8 color1 = Rgba8( 255, 255, 0, 255 );
	Rgba8 color2 = Rgba8( 255, 0, 0, 255 );
	Rgba8 color3 = Rgba8( 255, 0, 0, 0 );

	if (m_type == EntityType::enemyBullet) {
		color1 = Rgba8( 0, 255, 255, 255 );
		color2 = Rgba8( 0, 0, 255, 255 );
		color3 = Rgba8( 0, 0, 255, 0 );
	}
	

	Vertex_PCU bulletVerts[6] = {
	Vertex_PCU( Vec3( 0.f, 0.5f, 0.f ), color1, Vec2( 0.f, 0.f ) ),
	Vertex_PCU( Vec3( 0.5f, 0.f, 0.f ), color1, Vec2( 0.f, 0.f ) ),
	Vertex_PCU( Vec3( 0.f, -0.5f, 0.f ), color1, Vec2( 0.f, 0.f ) ),
	Vertex_PCU( Vec3( 0.f, 0.5f, 0.f ), color2, Vec2( 0.f, 0.f ) ),
	Vertex_PCU( Vec3( 0.f, -0.5f, 0.f ), color2, Vec2( 0.f, 0.f ) ),
	Vertex_PCU( Vec3( -2.f, 0.f, 0.f ), color3, Vec2( 0.f, 0.f ) ),
	};
	TransformVertexArrayXY3D( 6, bulletVerts, 1.f, m_orientationDegrees, m_position );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 6, bulletVerts );
}

void Bullet::Die()
{
	m_game->SpawnDebris( this );
	m_isDead = true;
	m_isGarbage = true;
}


