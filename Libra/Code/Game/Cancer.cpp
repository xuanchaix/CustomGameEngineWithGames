#include "Game/Cancer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/App.hpp"

Cancer::Cancer( Vec2 const& startPos, Map* map, EntityFaction faction )
	:Entity( startPos, map )
{
	m_speed = g_gameConfigBlackboard.GetValue( "cancerDriveSpeed", 0.7f );
	m_velocity = Vec2( 0, 0 );
	m_orientationDegrees = g_theGame->m_randNumGen->RollRandomFloatInRange( 0.f, 360.f );
	m_angularVelocity = g_gameConfigBlackboard.GetValue( "cancerTurnRate", 120.f );

	m_physicsRadius = CANCER_PHYSICS_RADIUS;
	m_cosmeticRadius = CANCER_COSMETIC_RADIUS;

	m_maxHealth = g_gameConfigBlackboard.GetValue( "cancerMaxHealth", 5.f );
	m_health = m_maxHealth;

	m_type = EntityType::_CANCER;
	m_faction = faction;

	m_target = m_map->GetNearestEnemyActor( this );
	m_curState = 1;

	m_wonderingCoolDown = g_gameConfigBlackboard.GetValue( "cancerWanderRandomizePeriod", 1.f );
	m_wonderingTime = m_wonderingCoolDown;

	m_shootCoolDown = g_gameConfigBlackboard.GetValue( "cancerShootCooldownSeconds", 1.2f );
	m_shootTime = m_shootCoolDown;
	m_damage = g_gameConfigBlackboard.GetValue( "cancerShootDamage", 1.f );

	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_isActor = true;
	m_isRanged = false;

	if (m_faction == EntityFaction::FACTION_EVIL) {
		m_tankTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank1.png" );
	}
	else if (m_faction == EntityFaction::FACTION_GOOD) {
		m_tankTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTank1.png" );
	}

	m_driveHalfAngleDegrees = g_gameConfigBlackboard.GetValue( "cancerDriveHalfAperture", 45.f );
	m_shootHalfAngleDegrees = g_gameConfigBlackboard.GetValue( "cancerShootHalfAperture", 5.f );

	Entity::InitializeMovingWarrior();
}

Cancer::~Cancer()
{

}

void Cancer::Update( float deltaTime )
{
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}

	if (m_health <= 0.f) {
		Die();
		return;
	}

	if (m_shootTime > 0.f) {
		m_shootTime -= deltaTime;
	}

	Entity::ConductAIMovingWarrior( deltaTime );
	// shoot
	if (m_curState == 2 && m_shootTime <= 0.f) {
		if (GetDistanceSquared2D( m_target->m_position, m_position ) < 9.f 
			&& abs( GetShortestAngularDispDegrees( m_orientationDegrees, (m_target->m_position - m_position).GetOrientationDegrees() ) ) <= m_shootHalfAngleDegrees) {
			float rndDegrees = g_theGame->m_randNumGen->RollRandomFloatInRange( -5.f, 5.f );
			m_map->SpawnNewEntity( EntityType::_FLAME_BULLET, m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees, 0.4f ), m_faction, this, m_orientationDegrees + rndDegrees );
			m_shootTime = m_shootCoolDown;
		}
	}
}

void Cancer::Render() const
{
	if (m_isDead) {
		return;
	}

	std::vector<Vertex_PCU> tankVerts;
	tankVerts.reserve( 6 );
	AddVertsForOBB2D( tankVerts, OBB2( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees ), Vec2( m_cosmeticRadius, m_cosmeticRadius ) ), Rgba8( 255, 255, 255, 255 ) );
	g_theRenderer->BindTexture( m_tankTexture );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( tankVerts );
}

void Cancer::Die()
{
	m_map->PlaySound( AudioName::EnemyDied, m_position );
	m_isDead = true;
	m_map->SpawnNewEntity( EntityType::_EXPLOSION, m_position, m_faction, this );
}

void Cancer::DebugRender() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 400 );
	AddVertsForDebugDrawLine( verts, m_position, m_orientationDegrees, m_cosmeticRadius, 0.02f, Rgba8( 255, 0, 0, 255 ) );
	AddVertsForDebugDrawLine( verts, m_position, m_orientationDegrees + 90.f, m_cosmeticRadius, 0.02f, Rgba8( 0, 255, 0, 255 ) );
	AddVertsForDebugDrawRing( verts, m_position, m_cosmeticRadius, 0.04f, Rgba8( 255, 0, 255, 255 ) );
	AddVertsForDebugDrawRing( verts, m_position, m_physicsRadius, 0.04f, Rgba8( 0, 255, 255, 255 ) );
	AddVertsForDebugDrawLine( verts, m_position, Atan2Degrees( m_velocity.y, m_velocity.x ), m_velocity.GetLength(), 0.02f, Rgba8( 255, 255, 0, 255 ) );

	AddVertsForDebugDrawLine( verts, m_position, m_lastSeenPosition, 0.1f, Rgba8( 0, 0, 0 ) );
	AddVertsForDebugDrawLine( verts, m_position, m_nextWayPointPosition, 0.1f, Rgba8( 255, 0, 0, 255 ) );


	AddVertsForDebugDrawLine( verts, m_position + (m_nextWayPointPosition - m_position).GetNormalized().GetRotated90Degrees() * m_physicsRadius, (m_nextWayPointPosition - m_position).GetOrientationDegrees(), (m_nextWayPointPosition - m_position).GetLength(), 0.04f, Rgba8( 255, 0, 0, 255 ) );
	AddVertsForDebugDrawLine( verts, m_position + (m_nextWayPointPosition - m_position).GetNormalized().GetRotatedMinus90Degrees() * m_physicsRadius, (m_nextWayPointPosition - m_position).GetOrientationDegrees(), (m_nextWayPointPosition - m_position).GetLength(), 0.04f, Rgba8( 255, 0, 0, 255 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void Cancer::BeAttacked( float hit )
{
	m_map->PlaySound( AudioName::EnemyHit, m_position );
	m_health -= hit;
}
