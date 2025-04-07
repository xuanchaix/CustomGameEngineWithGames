#include "Game/PlayerBullet.hpp"
#include "Game/Game.hpp"

PlayerBullet::PlayerBullet( ProjectileDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Projectile(def, startPos, startOrientation, startVelocity)
{
	m_physicsRadius = 0.5f;
	m_cosmeticRadius = 0.5f;
}

PlayerBullet::~PlayerBullet()
{

}

void PlayerBullet::BeginPlay()
{

}

void PlayerBullet::Update( float deltaTime )
{
	Projectile::Update( deltaTime );
	m_position += m_velocity * deltaTime;
	if (!g_theGame->m_worldCamera.m_cameraBox.IsPointInside( m_position )) {
		Die( false );
	}
	if (m_isPoisonous && m_isIced) {
		m_color = Rgba8( 128, 128, 128 );
	}
	else if (m_isPoisonous) {
		m_color = Rgba8( 128, 255, 0 );
	}
}

void PlayerBullet::Render() const
{
	Rgba8 color1;
	Rgba8 color2;
	Rgba8 color3;
	if (m_isIced && !m_isPoisonous) {
		color1 = Rgba8( 255, 255, 255, 255 );
		color2 = Rgba8( 192, 192, 192, 255 );
		color3 = Rgba8( 192, 192, 192, 0 );
	}
	else {
		color1 = Rgba8( 0, 255, 255, 255 );
		color2 = Rgba8( 0, 0, 255, 255 );
		color3 = Rgba8( 0, 0, 255, 0 );
	}

	Vertex_PCU bulletVerts[6] = {
	Vertex_PCU( Vec3( 1.f, 0.f, 0.f ), color1, Vec2( 0.f, 0.f ) ),
	Vertex_PCU( Vec3( 0.f, 0.5f, 0.f ), color1, Vec2( 0.f, 0.f ) ),
	Vertex_PCU( Vec3( 0.f, -0.5f, 0.f ), color1, Vec2( 0.f, 0.f ) ),
	Vertex_PCU( Vec3( 0.f, -0.5f, 0.f ), color2, Vec2( 0.f, 0.f ) ),
	Vertex_PCU( Vec3( 0.f, 0.5f, 0.f ), color2, Vec2( 0.f, 0.f ) ),
	Vertex_PCU( Vec3( -2.f, 0.f, 0.f ), color3, Vec2( 0.f, 0.f ) ),
	};
	Mat44 modelMatrix = GetModelMatrix();
	modelMatrix.AppendScaleUniform2D( m_scale );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( modelMatrix, m_color );
	g_theRenderer->DrawVertexArray( 6, bulletVerts );
}

void PlayerBullet::Die( bool dieByCollision )
{
	UNUSED( dieByCollision );
	m_isDead = true;
	//if (dieByCollision) {
	//	SpawnCollisionEffect();
	//}
}

void PlayerBullet::SpawnCollisionEffect()
{
	Rgba8 color1, color2;
	if (m_isPoisonous) {
		color1 = Rgba8( 0, 255, 0, 150 );
		color2 = Rgba8( 0, 255, 0, 0 );
	}
	else {
		color1 = Rgba8( 0, 0, 255, 150 );
		color2 = Rgba8( 0, 0, 255, 0 );
	}
	if (m_hasRangeDamage) {
		ParticleSystem2DAddEmitter( 300, 0.05f, AABB2( m_position, m_position ),
			FloatRange( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 1.2f ),
			AABB2( Vec2( -30.f, -30.f ), Vec2( 30.f, 30.f ) ),
			FloatRange( 0.2f, 0.4f ), color1, Particle2DShape::Asteroid, true, FloatRange( 0.f, 360.f ),
			FloatRange( 40.f, 75.f ), nullptr,
			color2 );
	}
	else {
		ParticleSystem2DAddEmitter( 300, 0.05f, AABB2( m_position, m_position ),
			FloatRange( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 1.2f ),
			AABB2( Vec2( -20.f, -20.f ) - m_velocity, Vec2( 20.f, 20.f ) - m_velocity ),
			FloatRange( 0.2f, 0.4f ), color1, Particle2DShape::Asteroid, true, FloatRange( 0.f, 360.f ),
			FloatRange( 40.f, 75.f ), nullptr,
			color2 );
	}
}

