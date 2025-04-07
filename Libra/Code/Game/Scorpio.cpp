#include "Game/Scorpio.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/App.hpp"

Scorpio::Scorpio( Vec2 const& startPos, Map* map, EntityFaction faction )
	:Entity( startPos, map )
{
	m_velocity = Vec2( 0, 0 );
	m_orientationDegrees = 0.f;
	m_turretOrientationDegrees = g_theGame->m_randNumGen->RollRandomFloatInRange( 0.f, 360.f );
	m_turretAngularVelocity = g_gameConfigBlackboard.GetValue( "scorpioTurnRate", 60.f );

	m_physicsRadius = SCORPIO_PHYSICS_RADIUS;
	m_cosmeticRadius = SCORPIO_COSMETIC_RADIUS;

	m_maxHealth = g_gameConfigBlackboard.GetValue( "scorpioMaxHealth", 5.f );
	m_health = m_maxHealth;

	m_type = EntityType::_SCORPIO;
	m_faction = faction;

	m_target = m_map->GetNearestEnemyActor( this );
	m_curState = 1;

	m_shootTime = g_gameConfigBlackboard.GetValue( "scorpioShootCooldownSeconds", 0.3f );
	m_shootCoolDown = m_shootTime;
	m_damage = g_gameConfigBlackboard.GetValue( "scorpioShootDamage", 1.f );

	m_isPushedByEntities = false;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_isActor = true;
	m_isRanged = true;

	if (m_faction == EntityFaction::FACTION_EVIL) {
		m_baseTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTurretBase.png" );
		m_turretTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyCannon.png" );
	}
	else if (m_faction == EntityFaction::FACTION_GOOD) {
		m_baseTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTurretBase.png" );
		m_turretTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyCannon.png" );
	}

	m_thisFrameRayResult.m_rayStartPos = m_position;
	m_thisFrameRayResult.m_impactDist = 0.f;
	m_thisFrameRayResult.m_rayMaxLength = 10.f;
	m_thisFrameRayResult.m_impactPos = m_position;
	m_thisFrameRayResult.m_didImpact = false;

	m_shootHalfAngleDegrees = g_gameConfigBlackboard.GetValue( "scorpioShootHalfAperture", 5.f );
}

Scorpio::~Scorpio()
{

}

void Scorpio::Update( float deltaTime )
{
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}

	if (m_health <= 0.f) {
		Die();
		return;
	}

	m_target = m_map->GetNearestEnemyActor( this );
	

	if (m_shootTime > 0.f) {
		m_shootTime -= deltaTime;
	}

	RayCastResult2D result;
	bool hasSight;
	if (m_target && m_target->IsAlive()) {
		hasSight = m_map->HasLineOfSight( this, m_target, result, *(m_map->GetAmphibiousTileHeatMap()) );
	}
	else {
		hasSight = false;
	}

	// rotating turret
	if (m_curState == 1) {
		// find target
		if (hasSight) {
			if (m_target->m_type == EntityType::_GOOD_PLAYER) {
				m_map->PlaySound( AudioName::EnemyAlert, m_position, 0.5f );
			}
			m_curState = 2;
			m_turretGoalOrientationDegrees = result.m_rayForwardNormal.GetOrientationDegrees();
			m_turretOrientationDegrees = GetTurnedTowardDegrees( m_turretOrientationDegrees, m_turretGoalOrientationDegrees, m_turretAngularVelocity * deltaTime );
		}
		else {
			m_turretOrientationDegrees += deltaTime * m_turretAngularVelocity;
		}
	}
	else if (m_curState == 2) {
		// if target is dead or lose sight of target goto 1 searching
		if ((m_target && m_target->m_isDead) || !hasSight) {
			m_curState = 1;
			if (!m_target || m_target->m_isDead) {
				m_target = m_map->GetNearestEnemyActor( this );
			}
			m_turretOrientationDegrees += deltaTime * m_turretAngularVelocity;
		}
		else {

			m_turretGoalOrientationDegrees = result.m_rayForwardNormal.GetOrientationDegrees();
			m_turretOrientationDegrees = GetTurnedTowardDegrees( m_turretOrientationDegrees, m_turretGoalOrientationDegrees, m_turretAngularVelocity * deltaTime );
			// shoot
			if (m_shootTime <= 0.f) {
				if (abs( GetShortestAngularDispDegrees( m_turretOrientationDegrees, m_turretGoalOrientationDegrees ) ) <= m_shootHalfAngleDegrees) {
					m_map->SpawnNewEntity( EntityType::_BULLET, m_position + Vec2::MakeFromPolarDegrees( m_turretOrientationDegrees, 0.35f ), m_faction, this, m_turretOrientationDegrees );
					m_shootTime = m_shootCoolDown;
					m_map->PlaySound( AudioName::EnemyShoot, m_position, 0.f, false, 0.5f );
					m_map->SpawnExplosion( m_position + Vec2::MakeFromPolarDegrees( m_turretOrientationDegrees, 0.35f ), 0.2f, 0.2f );
				}
			}
		}
	}

	m_map->RaycastVsTiles( Ray2D( m_position, m_turretOrientationDegrees, g_enemyVisibleRange ), m_thisFrameRayResult, *(m_map->GetAmphibiousTileHeatMap()) );
}

void Scorpio::Render() const
{
	if (m_isDead) {
		return;
	}

	std::vector<Vertex_PCU> verts;
	Rgba8 color1;
	Rgba8 color2;
	if (m_faction == EntityFaction::FACTION_EVIL) {
		color1 = Rgba8( 255, 0, 0, 200 );
		color2 = Rgba8( 255, 0, 0, (unsigned char)(200 * (1.f - (m_thisFrameRayResult.m_impactDist / m_thisFrameRayResult.m_rayMaxLength))) );
	}
	else if (m_faction == EntityFaction::FACTION_GOOD) {
		color1 = Rgba8( 0, 0, 255, 200 );
		color2 = Rgba8( 0, 0, 255, (unsigned char)(200 * (1.f - (m_thisFrameRayResult.m_impactDist / m_thisFrameRayResult.m_rayMaxLength))) );
	}
	
	verts.reserve( 6 );
	float thickness = 0.08f;
	Vec2 startPoint = m_position + Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees, 0.5f);
	Vec2 endPoint = m_thisFrameRayResult.m_impactPos;
	Vec2 dForward = (endPoint - startPoint).GetNormalized();
	Vec2 forward = dForward * thickness * 0.5f;
	Vec2 left = dForward.GetRotated90Degrees() * thickness * 0.5f;
	Vec2 p1 = Vec2( (startPoint - forward + left).x, (startPoint - forward + left).y );
	Vec2 p2 = Vec2( (startPoint - forward - left).x, (startPoint - forward - left).y );
	Vec2 p3 = Vec2( (endPoint + left).x, (endPoint + left).y );
	Vec2 p4 = Vec2( (endPoint - left).x, (endPoint - left).y );
	verts.emplace_back( p1, color1 );
	verts.emplace_back( p2, color1 );
	verts.emplace_back( p3, color2 );
	verts.emplace_back( p2, color1 );
	verts.emplace_back( p3, color2 );
	verts.emplace_back( p4, color2 );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );

	std::vector<Vertex_PCU> tankBaseVerts;
	tankBaseVerts.reserve( 6 );
	AddVertsForAABB2D( tankBaseVerts, AABB2( m_position - Vec2(m_cosmeticRadius, m_cosmeticRadius), m_position + Vec2( m_cosmeticRadius, m_cosmeticRadius ) ), Rgba8( 255, 255, 255, 255 ) );
	g_theRenderer->BindTexture( m_baseTexture );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( tankBaseVerts );

	std::vector<Vertex_PCU> tankTurretVerts;
	tankTurretVerts.reserve( 6 );
	AddVertsForOBB2D( tankTurretVerts, OBB2( m_position, Vec2::MakeFromPolarDegrees( m_turretOrientationDegrees ), Vec2( m_cosmeticRadius, m_cosmeticRadius ) ), Rgba8( 255, 255, 255, 255 ) );
	g_theRenderer->BindTexture( m_turretTexture );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( tankTurretVerts );


}

void Scorpio::Die()
{
	m_map->PlaySound( AudioName::EnemyDied, m_position );
	m_isDead = true;
	m_map->SpawnNewEntity( EntityType::_EXPLOSION, m_position, m_faction, this );
}

void Scorpio::DebugRender() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 400 );
	AddVertsForDebugDrawLine( verts, m_position, m_orientationDegrees, m_cosmeticRadius, 0.02f, Rgba8( 255, 0, 0, 255 ) );
	AddVertsForDebugDrawLine( verts, m_position, m_orientationDegrees + 90.f, m_cosmeticRadius, 0.02f, Rgba8( 0, 255, 0, 255 ) );
	AddVertsForDebugDrawRing( verts, m_position, m_cosmeticRadius, 0.04f, Rgba8( 255, 0, 255, 255 ) );
	AddVertsForDebugDrawRing( verts, m_position, m_physicsRadius, 0.04f, Rgba8( 0, 255, 255, 255 ) );
	AddVertsForDebugDrawLine( verts, m_position, Atan2Degrees( m_velocity.y, m_velocity.x ), m_velocity.GetLength(), 0.02f, Rgba8( 255, 255, 0, 255 ) );
	if (m_curState == 2) {
		AddVertsForDebugDrawLine( verts, m_position, m_target->m_position, 0.03f, Rgba8( 64, 64, 64, 255 ) );
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void Scorpio::BeAttacked( float hit )
{
	m_map->PlaySound( AudioName::EnemyHit, m_position );
	m_health -= hit;
}

