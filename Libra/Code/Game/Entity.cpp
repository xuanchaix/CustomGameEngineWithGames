#include "Game/Entity.hpp"
#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Bullet.hpp"
#include "Game/Ray.hpp"
#include "Game/App.hpp"

Entity::Entity( Vec2 const& startPos, Map* map):
	m_position(startPos),
	m_map(map)
{
	m_speed = 0.f;
	m_angularVelocity = 0.f;
	m_cosmeticRadius = 0.f;
	m_physicsRadius = 0.f;
	m_orientationDegrees = 0.f;
	m_health = 1;
	//m_color = Rgba8();
	m_maxHealth = 1;
	m_pathPoints.reserve( (size_t)m_map->GetDimensions().x + m_map->GetDimensions().y );
}

Entity::~Entity() {
  	delete m_targetDistanceTileHeatMap;
}

void Entity::DebugRender() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 300 );
	AddVertsForDebugDrawLine( verts, m_position, m_orientationDegrees, m_cosmeticRadius, 0.02f, Rgba8( 255, 0, 0, 255 ) );
	AddVertsForDebugDrawLine( verts, m_position, m_orientationDegrees + 90.f, m_cosmeticRadius, 0.02f, Rgba8( 0, 255, 0, 255 ) );
	AddVertsForDebugDrawRing( verts, m_position, m_cosmeticRadius, 0.04f, Rgba8( 255, 0, 255, 255 ) );
	AddVertsForDebugDrawRing( verts, m_position, m_physicsRadius, 0.04f, Rgba8( 0, 255, 255, 255 ) );
	AddVertsForDebugDrawLine( verts, m_position, Atan2Degrees( m_velocity.y, m_velocity.x ), m_velocity.GetLength(), 0.02f, Rgba8( 255, 255, 0, 255 ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( (int)verts.size(), verts.data() );
}


void Entity::RenderUI() const
{
	if (m_isDead || m_isGarbage || m_health <= 0.f) {
		return;
	}
	if (m_health == m_maxHealth) {
		return;
	}
	float remainHealthRatio = (float)m_health / (float)m_maxHealth;
	if (remainHealthRatio < 0.f) {
		remainHealthRatio = 0.f;
	}
	std::vector<Vertex_PCU> healthBarVerts;
	healthBarVerts.reserve( 12 );
	AddVertsForAABB2D( healthBarVerts, AABB2( m_position + Vec2( -0.3f, m_cosmeticRadius - 0.05f ), m_position + Vec2( 0.3f, m_cosmeticRadius + 0.05f ) ), Rgba8( 255, 0, 0, 255 ), AABB2::IDENTITY );
	AddVertsForAABB2D( healthBarVerts, AABB2( m_position + Vec2( -0.3f, m_cosmeticRadius - 0.05f ), m_position + Vec2( 0.3f - 0.6f * (1 - remainHealthRatio), m_cosmeticRadius + 0.05f ) ), Rgba8( 0, 255, 0, 255 ), AABB2::IDENTITY );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( healthBarVerts );
}

Vec2 const Entity::GetForwardNormal() const
{
	return m_velocity.GetNormalized();
}

bool Entity::IsAlive() const
{
	return !m_isDead;
}

bool Entity::IsBullet() const
{
	return m_type == EntityType::_BULLET || m_type == EntityType::_BOLT || m_type == EntityType::_GUIDED_BULLET || m_type == EntityType::_FLAME_BULLET;
}

bool Entity::IsActor() const
{
	return m_isActor;
}

TileHeatMap const* Entity::GetTargetDistanceTileHeatMap() const
{
	return m_targetDistanceTileHeatMap;
}

void Entity::BeAttacked( float hit )
{
	m_health -= hit;
	if (m_health > m_maxHealth) {
		m_health = m_maxHealth;
	}
}

void Entity::GetAttackedByBullet( Bullet* b )
{
	Vec2 normalVector = b->m_position - m_position;
	float theta = abs( GetShortestAngularDispDegrees( normalVector.GetOrientationDegrees(), m_orientationDegrees ) );
	if (theta <= 45.f) {
		BeAttacked( b->m_damage * 0.5f );
	}
	else if (theta > 45.f && theta <= 135.f) {
		BeAttacked( b->m_damage );
	}
	else {
		BeAttacked( b->m_damage * 2.f );
	}

	if (b->m_type == EntityType::_FLAME_BULLET) {
		b->m_isDead = true;
	}
	else {
		b->Die();
	}
}

void Entity::InitializeMovingWarrior()
{
	m_targetDistanceTileHeatMap = new TileHeatMap( m_map->GetDimensions() );
	// wondering
	m_map->GetRandomWonderPosAndDistanceTileHeatMap( this, m_lastSeenPosition, *m_targetDistanceTileHeatMap );
	m_map->GenerateEntityPathToGoal( m_pathPoints, *m_targetDistanceTileHeatMap, m_position, m_lastSeenPosition );
	OptimizeRoute();
	m_nextWayPointPosition = (int)m_pathPoints.size() >= 1 && m_optimizedPathPointsIndex != 0 ? m_pathPoints[m_optimizedPathPointsIndex] : m_lastSeenPosition;
	m_goalOrientationDegrees = (m_nextWayPointPosition - m_position).GetOrientationDegrees();
}

void Entity::ConductAIMovingWarrior( float deltaTime )
{
	if (m_target && m_target->m_isDead) {
		m_curState = 1;
		m_map->GetRandomWonderPosAndDistanceTileHeatMap( this, m_lastSeenPosition, *m_targetDistanceTileHeatMap );
		m_map->GenerateEntityPathToGoal( m_pathPoints, *m_targetDistanceTileHeatMap, m_position, m_lastSeenPosition );
	}
	m_target = m_map->GetNearestEnemyActor( this );

	// if next point to go is reached pop it
	/*if ((int)m_pathPoints.size() > 0 && IsPointInsideDisc2D(m_pathPoints[m_optimizedPathPointsIndex], m_position, m_physicsRadius)) {
		if (!m_target || !((int)m_pathPoints.size() == 1 && GetDistanceSquared2D(m_target->m_position, m_position) > 1.f)) {
			for (int i = m_optimizedPathPointsIndex; i < (int)m_pathPoints.size(); i++) {
				m_pathPoints.pop_back();
			}
		}
	}*/

	RayCastResult2D result;
	bool hasSight;
	if (m_target && m_target->IsAlive()) {
		hasSight = m_map->HasLineOfSight( this, m_target, result, *(m_map->GetAmphibiousTileHeatMap()) );
	}
	else {
		hasSight = false;
	}

	// wondering
	if (m_curState == 1) {
		// if find target
		if (hasSight) {
			m_curState = 2;
			// prepare for state 2
			// update heat map
			if (m_target->m_type == EntityType::_GOOD_PLAYER) {
				m_map->PlaySound( AudioName::EnemyAlert, m_position, 0.5f );
			}
			m_map->GetDistanceTileHeatMapForTargetPos( this, m_target->m_position, *m_targetDistanceTileHeatMap );
			m_map->GenerateEntityPathToGoal( m_pathPoints, *m_targetDistanceTileHeatMap, m_position, m_target->m_position );
			OptimizeRoute();
			m_nextWayPointPosition = (int)m_pathPoints.size() >= 1 && m_optimizedPathPointsIndex != 0 ? m_pathPoints[m_optimizedPathPointsIndex] : m_target->m_position;
			m_goalOrientationDegrees = (m_nextWayPointPosition - m_position).GetOrientationDegrees();
			// if target in drive to range
			if (GetShortestAngularDispDegrees( m_orientationDegrees, m_goalOrientationDegrees ) <= m_driveHalfAngleDegrees
				&& GetShortestAngularDispDegrees( m_orientationDegrees, m_goalOrientationDegrees ) >= -m_driveHalfAngleDegrees) {
				m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_speed );
			}
			else {
				m_velocity = Vec2( 0, 0 );
			}
		}
		else {
			// wondering
			// is entity reach the destination
			if (IsPointInsideDisc2D( m_lastSeenPosition, m_position, m_physicsRadius )) {
				m_map->GetRandomWonderPosAndDistanceTileHeatMap( this, m_lastSeenPosition, *m_targetDistanceTileHeatMap );
				m_map->GenerateEntityPathToGoal( m_pathPoints, *m_targetDistanceTileHeatMap, m_position, m_lastSeenPosition );
				OptimizeRoute();
				m_nextWayPointPosition = (int)m_pathPoints.size() >= 1 && m_optimizedPathPointsIndex != 0 ? m_pathPoints[m_optimizedPathPointsIndex] : m_lastSeenPosition;
				m_goalOrientationDegrees = (m_nextWayPointPosition - m_position).GetOrientationDegrees();
			}
			else {
				OptimizeRoute();
				m_nextWayPointPosition = (int)m_pathPoints.size() >= 1 && m_optimizedPathPointsIndex != 0 ? m_pathPoints[m_optimizedPathPointsIndex] : m_lastSeenPosition;
				m_goalOrientationDegrees = (m_nextWayPointPosition - m_position).GetOrientationDegrees();
			}
			// if target in drive to range
			if (GetShortestAngularDispDegrees( m_orientationDegrees, m_goalOrientationDegrees ) <= m_driveHalfAngleDegrees
				&& GetShortestAngularDispDegrees( m_orientationDegrees, m_goalOrientationDegrees ) >= -m_driveHalfAngleDegrees) {
				m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_speed );
			}
			else {
				m_velocity = Vec2( 0, 0 );
			}
		}
	}
	// if find target and go for it
	else if (m_curState == 2) {
		// if target is dead goto 1 wondering
		if (!m_target || m_target->m_isDead) {
			m_curState = 1;
			m_target = m_map->GetNearestEnemyActor( this );
			m_map->GetRandomWonderPosAndDistanceTileHeatMap( this, m_lastSeenPosition, *m_targetDistanceTileHeatMap );
			m_map->GenerateEntityPathToGoal( m_pathPoints, *m_targetDistanceTileHeatMap, m_position, m_lastSeenPosition );
			OptimizeRoute();
			m_nextWayPointPosition = (int)m_pathPoints.size() >= 1 && m_optimizedPathPointsIndex != 0 ? m_pathPoints[m_optimizedPathPointsIndex] : m_lastSeenPosition;
			m_goalOrientationDegrees = (m_nextWayPointPosition - m_position).GetOrientationDegrees();
			// if target in drive to range
			if (GetShortestAngularDispDegrees( m_orientationDegrees, m_goalOrientationDegrees ) <= m_driveHalfAngleDegrees
				&& GetShortestAngularDispDegrees( m_orientationDegrees, m_goalOrientationDegrees ) >= -m_driveHalfAngleDegrees) {
				m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_speed );
			}
			else {
				m_velocity = Vec2( 0, 0 );
			}
		}
		// if target disappear
		else if (!hasSight) {
			m_curState = 3;
			// update heat map
			m_map->GetDistanceTileHeatMapForTargetPos( this, m_lastSeenPosition, *m_targetDistanceTileHeatMap );
			m_map->GenerateEntityPathToGoal( m_pathPoints, *m_targetDistanceTileHeatMap, m_position, m_lastSeenPosition );
			OptimizeRoute();
			m_nextWayPointPosition = (int)m_pathPoints.size() >= 1 && m_optimizedPathPointsIndex != 0 ? m_pathPoints[m_optimizedPathPointsIndex] : m_lastSeenPosition;
			// prepare goto state 3
			m_goalOrientationDegrees = (m_nextWayPointPosition - m_position).GetOrientationDegrees();
			// if target in drive to range
			if (GetShortestAngularDispDegrees( m_orientationDegrees, m_goalOrientationDegrees ) <= m_driveHalfAngleDegrees
				&& GetShortestAngularDispDegrees( m_orientationDegrees, m_goalOrientationDegrees ) >= -m_driveHalfAngleDegrees) {
				m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_speed );
			}
			else {
				m_velocity = Vec2( 0, 0 );
			}
		}
		// if is in normal hunting mode
		else {
			// if target moves to another tile
			if (m_map->GetMapPosFromWorldPos( m_lastSeenPosition ) != m_map->GetMapPosFromWorldPos( m_target->m_position )) {
				// update heat map
				m_map->GetDistanceTileHeatMapForTargetPos( this, m_target->m_position, *m_targetDistanceTileHeatMap );
				m_map->GenerateEntityPathToGoal( m_pathPoints, *m_targetDistanceTileHeatMap, m_position, m_target->m_position );
				OptimizeRoute();
				m_nextWayPointPosition = (int)m_pathPoints.size() >= 1 && m_optimizedPathPointsIndex != 0 ? m_pathPoints[m_optimizedPathPointsIndex] : m_target->m_position;
			}
			else {
				OptimizeRoute();
				m_nextWayPointPosition = (int)m_pathPoints.size() >= 1 && m_optimizedPathPointsIndex != 0 ? m_pathPoints[m_optimizedPathPointsIndex] : m_target->m_position;
			}
			// update last seen position
			m_lastSeenPosition = m_target->m_position;
			// if target in drive to range
			if (m_isRanged && GetDistanceSquared2D( m_position, m_target->m_position ) <= 16.f) {
				RayCastResult2D res1;
				RayCastResult2D res2;
				Vec2 forwardVector = m_target->m_position - m_position;
				float length = forwardVector.GetLength();
				Vec2 forwardNormal = forwardVector / length;
				Vec2 sideVector = forwardNormal.GetRotated90Degrees() * (BULLET_PHYSICS_RADIUS + 0.01f);
				if (!m_targetDistanceTileHeatMap->RayCastVsGrid2D( res1, Ray2D( m_position + sideVector, forwardNormal, length ), FLT_MAX )
					&& !m_targetDistanceTileHeatMap->RayCastVsGrid2D( res2, Ray2D( m_position - sideVector, forwardNormal, length ), FLT_MAX )) {
					m_goalOrientationDegrees = (m_target->m_position - m_position).GetOrientationDegrees();;
					m_velocity = Vec2( 0, 0 );
				}
				else {
					goto C2;
				}
			}
			else {
				C2:
				// update velocity value
				m_goalOrientationDegrees = (m_nextWayPointPosition - m_position).GetOrientationDegrees();;
				if (GetShortestAngularDispDegrees( m_orientationDegrees, m_goalOrientationDegrees ) <= m_driveHalfAngleDegrees
					&& GetShortestAngularDispDegrees( m_orientationDegrees, m_goalOrientationDegrees ) >= -m_driveHalfAngleDegrees
					) {
					m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_speed );
				}
				else {
					m_velocity = Vec2( 0, 0 );
				}
			}
		}
	}
	// go to last seen
	else if (m_curState == 3) {
		// if see the player
		if (hasSight) {
			m_curState = 2;
			// update heat map
			m_map->GetDistanceTileHeatMapForTargetPos( this, m_target->m_position, *m_targetDistanceTileHeatMap );
			m_map->GenerateEntityPathToGoal( m_pathPoints, *m_targetDistanceTileHeatMap, m_position, m_target->m_position );
			OptimizeRoute();
			m_nextWayPointPosition = (int)m_pathPoints.size() >= 1 && m_optimizedPathPointsIndex != 0 ? m_pathPoints[m_optimizedPathPointsIndex] : m_target->m_position;
			// prepare goto stage 2
			m_goalOrientationDegrees = (m_nextWayPointPosition - m_position).GetOrientationDegrees();;
			// if target in drive to range
			if (GetShortestAngularDispDegrees( m_orientationDegrees, m_goalOrientationDegrees ) <= m_driveHalfAngleDegrees
				&& GetShortestAngularDispDegrees( m_orientationDegrees, m_goalOrientationDegrees ) >= -m_driveHalfAngleDegrees) {
				m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_speed );
			}
			else {
				m_velocity = Vec2( 0, 0 );
			}
		}
		// if in last seen spot, goto state 1
		else if (IsPointInsideDisc2D( m_lastSeenPosition, m_position, m_physicsRadius )) {
			m_curState = 1;
			m_map->GetRandomWonderPosAndDistanceTileHeatMap( this, m_lastSeenPosition, *m_targetDistanceTileHeatMap );
			m_map->GenerateEntityPathToGoal( m_pathPoints, *m_targetDistanceTileHeatMap, m_position, m_lastSeenPosition );
			OptimizeRoute();
			m_nextWayPointPosition = (int)m_pathPoints.size() >= 1 && m_optimizedPathPointsIndex != 0 ? m_pathPoints[m_optimizedPathPointsIndex] : m_lastSeenPosition;
			m_goalOrientationDegrees = (m_nextWayPointPosition - m_position).GetOrientationDegrees();
			// if target in drive to range
			if (GetShortestAngularDispDegrees( m_orientationDegrees, m_goalOrientationDegrees ) <= m_driveHalfAngleDegrees
				&& GetShortestAngularDispDegrees( m_orientationDegrees, m_goalOrientationDegrees ) >= -m_driveHalfAngleDegrees) {
				m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_speed );
			}
			else {
				m_velocity = Vec2( 0, 0 );
			}
		}
		else {
			// continue go to last seen spot
			OptimizeRoute();
			m_nextWayPointPosition = (int)m_pathPoints.size() >= 1 && m_optimizedPathPointsIndex != 0 ? m_pathPoints[m_optimizedPathPointsIndex] : m_lastSeenPosition;
			m_goalOrientationDegrees = (m_nextWayPointPosition - m_position).GetOrientationDegrees();
			// if target in drive to range
			if (GetShortestAngularDispDegrees( m_orientationDegrees, m_goalOrientationDegrees ) <= m_driveHalfAngleDegrees
				&& GetShortestAngularDispDegrees( m_orientationDegrees, m_goalOrientationDegrees ) >= -m_driveHalfAngleDegrees) {
				m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_speed );
			}
			else {
				m_velocity = Vec2( 0, 0 );
			}
		}
	}

	m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, m_goalOrientationDegrees, deltaTime * m_angularVelocity );
	m_position += m_velocity * deltaTime;
}

void Entity::OptimizeRoute()
{

	//if ( !(m_orientationDegrees != m_goalOrientationDegrees && m_velocity != Vec2( 0, 0 )) ) {

		// from next to first cannot reach point
		// if next next point can be reached directly pop next point
		/*for (int i = (int)m_pathPoints.size() - 2; i >= 0; i--) {
			RayCastResult2D res1;
			RayCastResult2D res2;
			Vec2 sideVector = Vec2::MakeFromPolarDegrees( m_orientationDegrees + 90, m_physicsRadius );
			if (m_map->HasLineOfSight( m_position + sideVector, m_pathPoints[i] + sideVector, res1, *m_targetDistanceTileHeatMap )
				&& m_map->HasLineOfSight( m_position - sideVector, m_pathPoints[i] - sideVector, res2, *m_targetDistanceTileHeatMap )) {
				m_pathPoints.pop_back();
			}
			else {
				return;
			}
		}*/
		/*
		// if next next point can be reached directly pop next point
		if ((int)m_pathPoints.size() >= 2) {
			RayCastResult2D res1;
			RayCastResult2D res2;
			Vec2 sideVector = (m_pathPoints[(int)m_pathPoints.size() - 2] - m_position).GetNormalized().GetRotated90Degrees() * m_physicsRadius;
			if (m_map->HasLineOfSight( m_position + sideVector, m_pathPoints[(int)m_pathPoints.size() - 2] + sideVector, res1, *m_targetDistanceTileHeatMap )
				&& m_map->HasLineOfSight( m_position - sideVector, m_pathPoints[(int)m_pathPoints.size() - 2] - sideVector, res2, *m_targetDistanceTileHeatMap )) {
				m_pathPoints.pop_back();
			}
		}*/
		
		// from last to first can reach point
		int pathPointsInitialSize = (int)m_pathPoints.size();
		for (int i = 0; i < pathPointsInitialSize - 1; i++) {
			RayCastResult2D res1;
			RayCastResult2D res2;
			Vec2 forwardVector = m_pathPoints[i] - m_position;
			float length = forwardVector.GetLength();
			Vec2 forwardNormal = forwardVector / length;
			Vec2 sideVector = forwardNormal.GetRotated90Degrees() * m_physicsRadius;
			if (!m_targetDistanceTileHeatMap->RayCastVsGrid2D( res1, Ray2D( m_position + sideVector, forwardNormal, length ), FLT_MAX )
				&& !m_targetDistanceTileHeatMap->RayCastVsGrid2D( res2, Ray2D( m_position - sideVector, forwardNormal, length ), FLT_MAX )) {
				//for (int j = 0; j < pathPointsInitialSize - i - 1; j++) {
				//	m_pathPoints.pop_back();
				//}
				m_optimizedPathPointsIndex = i;
				return;
			}
		}
		m_optimizedPathPointsIndex = (int)m_pathPoints.size() - 1;
	//}
}
