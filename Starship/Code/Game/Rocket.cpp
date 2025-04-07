#include "Game/Rocket.hpp"
#include "Game/Game.hpp"

Rocket::Rocket( Vec2 startPos, Game* game, Vec2 velocity, float orientationDegree, float range, bool isEnemyBullet )
	:Entity(startPos, game)
{
	m_velocity = velocity;
	m_orientationDegrees = orientationDegree;
	m_accelerateVelocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, 20.f );
	m_physicsRadius = ROCKET_PHYSICS_RADIUS;
	m_cosmeticRadius = ROCKET_COSMETIC_RADIUS;
	m_health = 1;
	m_color = Rgba8( 150, 100, 0, 255 );
	if (isEnemyBullet) {
		m_type = EntityType::enemyBullet;
		m_color = Rgba8( 0, 100, 150, 255 );
	}
	else {
		m_type = EntityType::bullet;
		m_color = Rgba8( 255, 255, 102, 255 );
	}
	m_range = range;
	RefreshTarget();
	m_orientationDegrees = (m_rndMiddlePos - m_startPos).GetOrientationDegrees();
}

Rocket::~Rocket()
{

}

void Rocket::Update()
{
	float deltaTime = g_theGame->m_gameClock->GetDeltaSeconds();
	if (m_health <= 0 || m_lifespan < 0.f || IsOffscreen()) {
		Die();
		return;
	}
	m_lifespan -= deltaTime;
	if (m_target && !m_target->IsAlive()) {
		RefreshTarget();
	}
	else if (!m_target){
		float radius = 10.f;
		Vec2 forwardNormal = Vec2::MakeFromPolarDegrees( m_orientationDegrees, 1.f );
		Vec2 sideNormal = forwardNormal.GetRotatedMinus90Degrees();
		Vec2 center = m_position + radius * sideNormal;
		float orientationDegreesByCenter = (-sideNormal).GetOrientationDegrees();
		orientationDegreesByCenter -= deltaTime * 180.f;
		m_orientationDegrees -= deltaTime * 180.f;
		m_position = center + Vec2::MakeFromPolarDegrees( orientationDegreesByCenter, radius );
		RefreshTarget();
	}
	else {
		if (m_target) {
			m_targetPos = m_target->m_position;
			float t = m_timer / m_timeToTarget;
			Vec2 startLerp = m_startPos * (1 - t) + m_rndMiddlePos * t;
			Vec2 endLerp = m_rndMiddlePos * (1 - t) + m_targetPos * t;
			Vec2 forwardVector = endLerp - startLerp;
			float targetOrientationDegrees = Atan2Degrees( forwardVector.y, forwardVector.x );
			m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, targetOrientationDegrees, 300.f * deltaTime );
			if (abs( GetShortestAngularDispDegrees( m_orientationDegrees, targetOrientationDegrees ) ) <= 8.f) {
				m_timer += deltaTime;
				m_position = startLerp * (1 - t) + endLerp * t;
			}
		}
		//m_orientationDegrees = forwardVector.GetOrientationDegrees();
	}
}

void Rocket::Render() const
{
	Vertex_PCU localRocketVerts[15];
	localRocketVerts[0] = Vertex_PCU( Vec2( -0.5f, 1.f ), m_color );
	localRocketVerts[1] = Vertex_PCU( Vec2( 0.f, -0.5f ), m_color );
	localRocketVerts[2] = Vertex_PCU( Vec2( -0.5f, -1.f ), m_color );
	localRocketVerts[3] = Vertex_PCU( Vec2( -0.5f, 1.f ), m_color );
	localRocketVerts[4] = Vertex_PCU( Vec2( 0.f, -0.5f ), m_color );
	localRocketVerts[5] = Vertex_PCU( Vec2( 0.f, 0.5f ), m_color );

	localRocketVerts[6] = Vertex_PCU( Vec2( 0.f, 0.5f ), m_color );
	localRocketVerts[7] = Vertex_PCU( Vec2( 0.f, -0.5f ), m_color );
	localRocketVerts[8] = Vertex_PCU( Vec2( 2.f, -0.5f ), m_color );
	localRocketVerts[9] = Vertex_PCU( Vec2( 0.f, 0.5f ), m_color );
	localRocketVerts[10] = Vertex_PCU( Vec2( 2.f, -0.5f ), m_color );
	localRocketVerts[11] = Vertex_PCU( Vec2( 2.f, 0.5f ), m_color );

	localRocketVerts[12] = Vertex_PCU( Vec2( 2.f, 0.5f ), m_color );
	localRocketVerts[13] = Vertex_PCU( Vec2( 2.f, -0.5f ), m_color );
	localRocketVerts[14] = Vertex_PCU( Vec2( 2.5f, 0.f ), m_color );

	TransformVertexArrayXY3D( 15, localRocketVerts, 1.f, m_orientationDegrees, m_position );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 15, localRocketVerts );
}

void Rocket::Die()
{
	m_game->DealRocketDamage( m_position, m_range );
	m_game->SpawnRocketDebris( this );
	m_isDead = true;
	m_isGarbage = true;
}

void Rocket::RefreshTarget()
{
	m_target = m_game->GetRandomEnemyActor();
	if (m_target) {
		m_targetPos = m_target->m_position;
	}
	else {
		m_targetPos = m_position + Vec2( m_game->m_randNumGen->RollRandomFloatInRange( -10.f, 10.f ), m_game->m_randNumGen->RollRandomFloatInRange( -10.f, 10.f ));
	}
	m_startPos = m_position;
	m_timer = 0.f;
	float distance = GetDistance2D( m_startPos, m_targetPos );
	m_timeToTarget = distance / 40.f;
	m_rndMiddlePos = m_startPos + ((m_targetPos - m_startPos) / distance).GetRotatedDegrees( m_game->m_randNumGen->RollRandomPositiveNegative() * m_game->m_randNumGen->RollRandomFloatInRange( 20.f, 60.f ) ) * m_game->m_randNumGen->RollRandomFloatInRange( distance * 0.3f, distance * 0.7f );
}
