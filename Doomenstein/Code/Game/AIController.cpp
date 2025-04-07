#include "Game/AIController.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Weapon.hpp"

AIController::~AIController()
{
	delete m_tileHeatMap;
	m_tileHeatMap = nullptr;
}

bool AIController::IsPlayer() const
{
	return false;
}

bool AIController::IsAI() const
{
	return true;
}

void AIController::Damagedby( ActorUID uid, bool isLethal )
{
	m_targetActorUID = uid;
	UNUSED( isLethal );
}

void AIController::Update()
{
	// do nothing
}

bool AIController::IsLoseControl() const
{
	return m_isGarbage;
}

void AIController::Refresh()
{
	m_targetActorUID = ActorUID();
}

bool AIController::HasLineOfSight( Vec3 const& targetPos ) const
{
	RayCastResult3D res1;
	RayCastResult3D res2;
	Vec2 forwardVector = targetPos - m_controlledActorUID->m_position;
	float length = forwardVector.GetLength();
	Vec2 forwardNormal = forwardVector / length;
	Vec2 sideVector = forwardNormal.GetRotated90Degrees() * m_controlledActorUID->m_physicsRadius;
	res1 = g_theGame->m_curMap->RayCastWorldXY( m_controlledActorUID->m_position + sideVector + Vec3( 0.f, 0.f, 0.5f ), forwardNormal, length );
	res2 = g_theGame->m_curMap->RayCastWorldXY( m_controlledActorUID->m_position - sideVector + Vec3( 0.f, 0.f, 0.5f ), forwardNormal, length );
	if (!res1.m_didImpact && !res2.m_didImpact) {
		return true;
	}
	return false;
}

void AIController::UpdateTileHeatMap( Map* curMap )
{
	// assume target actor is valid
	IntVec2 targetMapPos = curMap->GetMapPosFromWorldPos( m_targetActorUID->m_position );
	IntVec2 curMapPos = curMap->GetMapPosFromWorldPos( m_controlledActorUID->m_position );
	curMap->PopulateHeatMapDistanceField( *m_tileHeatMap, targetMapPos );
	m_routeToTarget.clear();
	std::vector<IntVec2> tempRouteToTarget;
	do {
		float curPosHeatValue = m_tileHeatMap->GetTileValue( curMapPos );
		if (m_tileHeatMap->GetTileValue( curMapPos + IntVec2( 1, 0 ), FLT_MAX ) < curPosHeatValue) {
			tempRouteToTarget.push_back( curMapPos + IntVec2( 1, 0 ) );
			curMapPos = curMapPos + IntVec2( 1, 0 );
		}
		else if (m_tileHeatMap->GetTileValue( curMapPos + IntVec2( 0, 1 ), FLT_MAX ) < curPosHeatValue) {
			tempRouteToTarget.push_back( curMapPos + IntVec2( 0, 1 ) );
			curMapPos = curMapPos + IntVec2( 0, 1 );
		}
		else if (m_tileHeatMap->GetTileValue( curMapPos + IntVec2( -1, 0 ), FLT_MAX ) < curPosHeatValue) {
			tempRouteToTarget.push_back( curMapPos + IntVec2( -1, 0 ) );
			curMapPos = curMapPos + IntVec2( -1, 0 );
		}
		else if (m_tileHeatMap->GetTileValue( curMapPos + IntVec2( 0, -1 ), FLT_MAX ) < curPosHeatValue) {
			tempRouteToTarget.push_back( curMapPos + IntVec2( 0, -1 ) );
			curMapPos = curMapPos + IntVec2( 0, -1 );
		}
		else {
			// do nothing?
			break;
		}
	} while (targetMapPos != curMapPos);

	// reverse
	for (int i = (int)tempRouteToTarget.size() - 1; i >= 0; i--) {
		m_routeToTarget.push_back( tempRouteToTarget[i] );
	}
}

void AIController::OptimizeRoute( Map* curMap )
{
	if (!m_routeToTarget.empty()) {
		if (IsPointInsideDisc2D( m_controlledActorUID->m_position, Vec2(m_routeToTarget[(int)m_routeToTarget.size() - 1]) + Vec2(0.5f, 0.5f), m_controlledActorUID->m_physicsRadius * 1.5f )) {
			m_routeToTarget.pop_back();
		}
		// from last to first can reach point
		int pathPointsInitialSize = (int)m_routeToTarget.size();
		for (int i = 0; i < pathPointsInitialSize - 1; i++) {
			RayCastResult3D res1;
			RayCastResult3D res2;
			Vec2 forwardVector = Vec2( m_routeToTarget[i] ) + Vec2( 0.5f, 0.5f ) - Vec2( m_controlledActorUID->m_position );
			float length = forwardVector.GetLength();
			Vec2 forwardNormal = forwardVector / length;
			Vec2 sideVector = forwardNormal.GetRotated90Degrees() * m_controlledActorUID->m_physicsRadius;
			res1 = curMap->RayCastWorldXY( m_controlledActorUID->m_position + sideVector + Vec3( 0.f, 0.f, 0.5f ), forwardNormal, length );
			res2 = curMap->RayCastWorldXY( m_controlledActorUID->m_position - sideVector + Vec3( 0.f, 0.f, 0.5f ), forwardNormal, length );
			if (!res1.m_didImpact && !res2.m_didImpact) {
				m_optimizedRouteIndex = i;
				return;
			}
		}
		if ((int)m_routeToTarget.size() >= 2) {
			m_optimizedRouteIndex = (int)m_routeToTarget.size() - 1;
		}
		else {
			m_optimizedRouteIndex = INT_MAX;
		}
	}
}

void MeleeAIController::Update()
{
	if (!m_controlledActorUID.IsValid()) {
		return;
	}
	if (!m_controlledActorUID->IsAlive()) {
		m_isGarbage = true;
		return;
	}

	float deltaSeconds = g_theGame->m_gameClock->GetDeltaSeconds();
	if (!m_targetActorUID.IsValid() || !m_targetActorUID->IsAlive()) {
		// target not valid: change target
		ActorFaction faction = ActorFaction::NEUTRAL;
		if (m_controlledActorUID->m_def.m_faction == ActorFaction::ALLY) {
			faction = ActorFaction::ENEMY;
		}
		else if (m_controlledActorUID->m_def.m_faction == ActorFaction::ENEMY) {
			faction = ActorFaction::ALLY;
		}
		Actor* thisFrameTarget;
		if (g_theGame->m_curMap->m_mapDef->m_gameMode == "Survival") {
			thisFrameTarget = m_controlledActorUID->m_map->GetNearestEnemy( faction, m_controlledActorUID.GetActor() );
		}
		else {
			thisFrameTarget = m_controlledActorUID->m_map->GetNearestVisibleEnemy( faction, m_controlledActorUID.GetActor() );
		}
		if (thisFrameTarget) {
			// successfully find a target
			m_targetActorUID = thisFrameTarget->m_uid;
		}
	}
	if (m_targetActorUID.IsValid()) {
		// target is valid
		Map* curMap = m_targetActorUID->m_map;
		if (m_tileHeatMap == nullptr || curMap->GetMapPosFromWorldPos( m_targetLastFramePosition ) != curMap->GetMapPosFromWorldPos( m_targetActorUID->m_position )) {
			if (m_tileHeatMap == nullptr) {
				m_tileHeatMap = new TileHeatMap( curMap->m_dimensions );
			}
			UpdateTileHeatMap( curMap );
		}
		OptimizeRoute( curMap );
		if (m_optimizedRouteIndex == 0 || m_optimizedRouteIndex >= (int)m_routeToTarget.size()) {
			m_nextPositionToGo = m_targetActorUID->m_position;
		}
		else {
			m_nextPositionToGo = Vec2( m_routeToTarget[m_optimizedRouteIndex] ) + Vec2( 0.5f, 0.5f );
		}
		//Actor* m_actor = m_controlledActorUID.GetActor();
		Vec2 nextPosForwardVector = m_nextPositionToGo - Vec2( m_controlledActorUID->m_position );
		m_controlledActorUID->SetYawDegrees( GetTurnedTowardDegrees( m_controlledActorUID->m_orientation.m_yawDegrees, nextPosForwardVector.GetOrientationDegrees(), m_controlledActorUID->m_def.m_turnSpeedDegrees * deltaSeconds ) );
		if (abs( GetShortestAngularDispDegrees( nextPosForwardVector.GetOrientationDegrees(), m_controlledActorUID->m_orientation.m_yawDegrees ) ) < 10.f
			&& GetDistanceSquared2D( m_controlledActorUID->m_position, m_targetActorUID->m_position ) > (m_controlledActorUID->m_physicsRadius + m_targetActorUID->m_physicsRadius + 0.05f) * (m_controlledActorUID->m_physicsRadius + m_targetActorUID->m_physicsRadius + 0.05f)) {
			if (g_theGame->m_curMap->m_mapDef->m_gameMode == "Survival") {
				m_controlledActorUID->AddForce( Vec2::MakeFromPolarDegrees( m_controlledActorUID->m_orientation.m_yawDegrees ) * m_controlledActorUID->m_def.m_walkSpeed );
			}
			else {
				m_controlledActorUID->AddForce( Vec2::MakeFromPolarDegrees( m_controlledActorUID->m_orientation.m_yawDegrees ) * m_controlledActorUID->m_def.m_runSpeed );
			}
			if (m_controlledActorUID->m_animationState == "Default") {
				m_controlledActorUID->SetAnimationState( "Walk" );
				m_controlledActorUID->m_animationTimer->Start();
			}
		}
		if (DoDiscOverlapDirectedSector2D( m_targetActorUID->m_position, m_targetActorUID->m_physicsRadius, m_controlledActorUID->m_position, m_controlledActorUID->GetForwardNormal(), m_controlledActorUID->m_curWeapon->m_def.m_meleeArc, m_controlledActorUID->m_curWeapon->m_def.m_meleeRange )) {
			m_controlledActorUID->Attack();
		}
		m_targetLastFramePosition = m_targetActorUID->m_position;
	}
	else {
		// no target: idle
	}
}

void RangedAIController::Update()
{
	if (!m_controlledActorUID.IsValid()) {
		return;
	}
	if (!m_controlledActorUID->IsAlive()) {
		m_isGarbage = true;
		return;
	}

	float deltaSeconds = g_theGame->m_gameClock->GetDeltaSeconds();
	if (!m_targetActorUID.IsValid() || !m_targetActorUID->IsAlive()) {
		// target not valid: change target
		ActorFaction faction = ActorFaction::NEUTRAL;
		if (m_controlledActorUID->m_def.m_faction == ActorFaction::ALLY) {
			faction = ActorFaction::ENEMY;
		}
		else if (m_controlledActorUID->m_def.m_faction == ActorFaction::ENEMY) {
			faction = ActorFaction::ALLY;
		}
		Actor* thisFrameTarget;
		if (g_theGame->m_curMap->m_mapDef->m_gameMode == "Survival") {
			thisFrameTarget = m_controlledActorUID->m_map->GetNearestEnemy( faction, m_controlledActorUID.GetActor() );
		}
		else {
			thisFrameTarget = m_controlledActorUID->m_map->GetNearestVisibleEnemy( faction, m_controlledActorUID.GetActor() );
		}
		if (thisFrameTarget) {
			// successfully find a target
			m_targetActorUID = thisFrameTarget->m_uid;
		}
	}
	if (m_targetActorUID.IsValid()) {
		// target is valid
		Map* curMap = m_targetActorUID->m_map;
		if (m_tileHeatMap == nullptr || curMap->GetMapPosFromWorldPos( m_targetLastFramePosition ) != curMap->GetMapPosFromWorldPos( m_targetActorUID->m_position )) {
			if (m_tileHeatMap == nullptr) {
				m_tileHeatMap = new TileHeatMap( curMap->m_dimensions );
			}
			UpdateTileHeatMap( curMap );
		}
		OptimizeRoute( curMap );
		if (m_optimizedRouteIndex == 0 || m_optimizedRouteIndex >= (int)m_routeToTarget.size()) {
			m_nextPositionToGo = m_targetActorUID->m_position;
		}
		else {
			m_nextPositionToGo = Vec2( m_routeToTarget[m_optimizedRouteIndex] ) + Vec2( 0.5f, 0.5f );
		}
		//Actor* m_actor = m_controlledActorUID.GetActor();
		Vec2 nextPosForwardVector = m_nextPositionToGo - Vec2( m_controlledActorUID->m_position );
		m_controlledActorUID->SetYawDegrees( GetTurnedTowardDegrees( m_controlledActorUID->m_orientation.m_yawDegrees, nextPosForwardVector.GetOrientationDegrees(), m_controlledActorUID->m_def.m_turnSpeedDegrees * deltaSeconds ) );
		if (abs( GetShortestAngularDispDegrees( nextPosForwardVector.GetOrientationDegrees(), m_controlledActorUID->m_orientation.m_yawDegrees ) ) < 10.f
			&& (GetDistanceSquared2D( m_controlledActorUID->m_position, m_targetActorUID->m_position ) > 144.f || !HasLineOfSight( m_targetActorUID->m_position ))) {
			if (g_theGame->m_curMap->m_mapDef->m_gameMode == "Survival") {
				m_controlledActorUID->AddForce( Vec2::MakeFromPolarDegrees( m_controlledActorUID->m_orientation.m_yawDegrees ) * m_controlledActorUID->m_def.m_walkSpeed );
			}
			else {
				m_controlledActorUID->AddForce( Vec2::MakeFromPolarDegrees( m_controlledActorUID->m_orientation.m_yawDegrees ) * m_controlledActorUID->m_def.m_walkSpeed );
			}
			if (m_controlledActorUID->m_animationState == "Default") {
				m_controlledActorUID->SetAnimationState( "Walk" );
				m_controlledActorUID->m_animationTimer->Start();
			}
		}
		if (abs( GetShortestAngularDispDegrees( nextPosForwardVector.GetOrientationDegrees(), m_controlledActorUID->m_orientation.m_yawDegrees ) ) < 5.f
			&& HasLineOfSight(m_targetActorUID->m_position)) {
			m_controlledActorUID->Attack();
		}
		m_targetLastFramePosition = m_targetActorUID->m_position;
	}
	else {
		// no target: idle
	}
}
