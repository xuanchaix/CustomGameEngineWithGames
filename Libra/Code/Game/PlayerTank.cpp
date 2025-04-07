#include "Game/PlayerTank.hpp"
#include "Game/Map.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"

PlayerTank::PlayerTank( Vec2 const& startPos, Map* map )
	:Entity(startPos, map)
{
	m_speed = g_gameConfigBlackboard.GetValue( "playerDriveSpeed", 1.f );
	m_velocity = Vec2( 0, 0 );
	m_orientationDegrees = PLAYER_START_ROTATION;
	m_turretRelativeOrientationDegrees = 0.f;
	m_turretOrientationDegrees = m_orientationDegrees + m_turretRelativeOrientationDegrees;
	m_angularVelocity = g_gameConfigBlackboard.GetValue( "playerTurnRate", 180.f );
	m_turretAngularVelocity = g_gameConfigBlackboard.GetValue( "playerGunTurnRate", 360.f );

	m_physicsRadius = PLAYER_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_COSMETIC_RADIUS;

	m_shootTime = g_gameConfigBlackboard.GetValue( "playerShootCooldownSeconds", 0.1f );
	m_shootCoolDown = g_gameConfigBlackboard.GetValue( "playerShootCooldownSeconds", 0.1f );
	m_damage = g_gameConfigBlackboard.GetValue( "playerShootDamage", 1.f );

	m_maxHealth = g_gameConfigBlackboard.GetValue( "playerMaxHealth", 10.f );;
	m_health = m_maxHealth;

	m_type = EntityType::_GOOD_PLAYER;
	m_faction = EntityFaction::FACTION_GOOD;

	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_isActor = true;

	m_tankBaseTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankBase.png" );
	m_tankTurretTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankTop.png" );
}

PlayerTank::~PlayerTank()
{

}

void PlayerTank::Update( float deltaTime )
{
	if (m_isDead) {
		return;
	}

	if (m_health <= 0.f) {
		Die();
		return;
	}

#ifdef DEBUG_MODE
	// debug invincible
	if (g_theInput->WasKeyJustPressed( KEYCODE_F2 )) {
		m_isImmortal = !m_isImmortal;
	}
#endif // DEBUG_MODE

	XboxController controller = g_theInput->GetController( 0 );

	if (m_map->m_doControllerShake) {
		controller.SetVibration( 10000, 10000 );
	}
	else {
		controller.SetVibration( 0, 0 );
	}

	// shoot
	if (m_shootTime > 0.f) {
		m_shootTime -= deltaTime;
	}
	if (g_theInput->IsKeyDown( ' ' ) || (controller.GetRightTrigger() > 0.2f)) {
		if (m_shootTime <= 0.f) {
			m_map->SpawnNewEntity( EntityType::_BULLET, m_position + Vec2::MakeFromPolarDegrees( m_turretOrientationDegrees, 0.2f ), m_faction, this, m_turretOrientationDegrees );
			m_map->SpawnNewEntity( EntityType::_EXPLOSION, m_position + Vec2::MakeFromPolarDegrees( m_turretOrientationDegrees, 0.32f ), m_faction, this, m_turretOrientationDegrees );
			m_shootTime = m_shootCoolDown;
			m_map->PlaySound( AudioName::PlayerShootNormal, m_position, 0.f, false, 0.6f );
		}
	}

	if (m_fuel > 0.f && (g_theInput->IsKeyDown( 'V' ) || (controller.GetLeftTrigger() > 0.2f))) {
		m_fuel -= deltaTime;
		float rndDegrees = g_theGame->m_randNumGen->RollRandomFloatInRange( -5.f, 5.f );
		m_map->SpawnNewEntity( EntityType::_FLAME_BULLET, m_position + Vec2::MakeFromPolarDegrees( m_turretOrientationDegrees, 0.32f ), m_faction, this, m_turretOrientationDegrees + rndDegrees );
		m_shootTime = m_shootCoolDown - 0.09f;
	}
	else if(!(g_theInput->IsKeyDown( 'V' ) || (controller.GetLeftTrigger() > 0.2f))) {
		m_fuel = m_fuel + deltaTime > MAX_FUEL ? MAX_FUEL : m_fuel + deltaTime;
	}

	// turn tank body
	// define a vector to represent where is the destination
	Vec2 turnTarget = Vec2( 0, 0 );
	if (g_theInput->IsKeyDown( 'W' )) {
		turnTarget += Vec2( 0, 1 );
	}
	if (g_theInput->IsKeyDown( 'A' )) {
		turnTarget += Vec2( -1, 0 );
	}
	if (g_theInput->IsKeyDown( 'S' )) {
		turnTarget += Vec2( 0, -1 );
	}
	if (g_theInput->IsKeyDown( 'D' )) {
		turnTarget += Vec2( 1, 0 );
	}
	// XBox controller has speed requirement
	float speedFactor = 1.f;
	// Get controller information
	if (controller.GetLeftStick().GetMagnitude() > 0.f) {
		turnTarget = controller.GetLeftStick().GetPosition();
		speedFactor = controller.GetLeftStick().GetMagnitude();
	}

	if (turnTarget == Vec2( 0, 0 )) {
		m_isBodyTurning = false;
	}
	else {
		m_isBodyTurning = true;
		m_goalOrientationDegrees = turnTarget.GetOrientationDegrees();
	}

	// rotate tank turret
	turnTarget = Vec2( 0, 0 );
	if (g_theInput->IsKeyDown( 'I' )) {
		turnTarget += Vec2( 0, 1 );
	}
	if (g_theInput->IsKeyDown( 'J' )) {
		turnTarget += Vec2( -1, 0 );
	}
	if (g_theInput->IsKeyDown( 'K' )) {
		turnTarget += Vec2( 0, -1 );
	}
	if (g_theInput->IsKeyDown( 'L' )) {
		turnTarget += Vec2( 1, 0 );
	}
	if (controller.GetRightStick().GetMagnitude() > 0.f) {
		turnTarget = controller.GetRightStick().GetPosition();
	}

	if (turnTarget == Vec2( 0, 0 )) {
		m_isTurretTurning = false;
	}
	else {
		m_isTurretTurning = true;
		m_turretGoalOrientationDegrees = turnTarget.GetOrientationDegrees();
	}

	// update position
	if (m_isTurretTurning && !m_isBodyTurning) {
		m_turretRelativeOrientationDegrees = GetTurnedTowardDegrees( m_turretOrientationDegrees, m_turretGoalOrientationDegrees, m_angularVelocity * deltaTime ) - m_orientationDegrees;
		m_turretOrientationDegrees = m_turretRelativeOrientationDegrees + m_orientationDegrees;
	}
	else if (m_isBodyTurning) {
		m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, m_goalOrientationDegrees, m_angularVelocity * deltaTime );
		if (!m_isTurretTurning) {
			m_turretOrientationDegrees = m_turretRelativeOrientationDegrees + m_orientationDegrees;
		}
		else {
			m_turretOrientationDegrees = GetTurnedTowardDegrees( m_turretOrientationDegrees, m_turretGoalOrientationDegrees, m_angularVelocity * deltaTime );
			m_turretRelativeOrientationDegrees = m_turretOrientationDegrees - m_orientationDegrees;
		}
		m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_speed * speedFactor );
		m_position += m_velocity * deltaTime;
	}
	
	// check if can call reinforcements
	IntVec2 curMapPos = m_map->GetMapPosFromWorldPos( m_position );
	IntVec2 mapDimension = m_map->GetDimensions();
	if (curMapPos.x == 1 || curMapPos.x == mapDimension.x - 2 || curMapPos.y == 1 || curMapPos.y == mapDimension.y - 2) {
		m_canCallReinforcements = true;
	}
	else {
		m_canCallReinforcements = false;
	}
	if (m_canCallReinforcements && (g_theInput->WasKeyJustPressed( 'B' ) || controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_B))) {
		m_map->CallReinforcemets( this );
	}

	m_health = m_health + m_playerSelfRepairRate * deltaTime < m_maxHealth ? m_health + m_playerSelfRepairRate * deltaTime : m_maxHealth;
}

void PlayerTank::Render() const
{
	std::vector<Vertex_PCU> tankBaseVerts;
	tankBaseVerts.reserve( 6 );
	if (m_map->m_curMapState == MapState::PLAYING) {
		AddVertsForOBB2D( tankBaseVerts, OBB2( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees ), Vec2( m_cosmeticRadius, m_cosmeticRadius ) ), Rgba8( 255, 255, 255, 255 ) );
	}
	else if (m_map->m_curMapState == MapState::EXIT_MAP) {
		AddVertsForOBB2D( tankBaseVerts, OBB2( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees + 720 * m_map->m_enterExitTimer * m_map->m_enterExitTimer ), Vec2( m_cosmeticRadius, m_cosmeticRadius ) * (1.f - m_map->m_enterExitTimer) ), Rgba8( 255, 255, 255, 255 ) );
	}
	else if (m_map->m_curMapState == MapState::ENTER_MAP) {
		AddVertsForOBB2D( tankBaseVerts, OBB2( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees + 720 * (1.f - m_map->m_enterExitTimer) * (1.f - m_map->m_enterExitTimer) ), Vec2( m_cosmeticRadius, m_cosmeticRadius ) * m_map->m_enterExitTimer ), Rgba8( 255, 255, 255, 255 ) );
	}
	g_theRenderer->BindTexture( m_tankBaseTexture );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( (int)tankBaseVerts.size(), tankBaseVerts.data() );

	std::vector<Vertex_PCU> tankTurretVerts;
	tankTurretVerts.reserve( 6 );
	if (m_map->m_curMapState == MapState::PLAYING) {
		AddVertsForOBB2D( tankTurretVerts, OBB2( m_position, Vec2::MakeFromPolarDegrees( m_turretOrientationDegrees ), Vec2( m_cosmeticRadius, m_cosmeticRadius ) ), Rgba8( 255, 255, 255, 255 ) );
	}
	else if (m_map->m_curMapState == MapState::EXIT_MAP) {
		AddVertsForOBB2D( tankTurretVerts, OBB2( m_position, Vec2::MakeFromPolarDegrees( m_turretOrientationDegrees + 720.f * m_map->m_enterExitTimer * m_map->m_enterExitTimer ), Vec2( m_cosmeticRadius, m_cosmeticRadius ) * (1.f - m_map->m_enterExitTimer) ), Rgba8( 255, 255, 255, 255 ) );
	}
	else if (m_map->m_curMapState == MapState::ENTER_MAP) {
		AddVertsForOBB2D( tankTurretVerts, OBB2( m_position, Vec2::MakeFromPolarDegrees( m_turretOrientationDegrees + 720.f * (1.f - m_map->m_enterExitTimer) * (1.f - m_map->m_enterExitTimer) ), Vec2( m_cosmeticRadius, m_cosmeticRadius ) * m_map->m_enterExitTimer ), Rgba8( 255, 255, 255, 255 ) );
	}
	g_theRenderer->BindTexture( m_tankTurretTexture );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( (int)tankTurretVerts.size(), tankTurretVerts.data() );

	if (m_isImmortal) {
		std::vector<Vertex_PCU> verts;
		verts.reserve( 100 );
		AddVertsForDebugDrawRing( verts, m_position, m_physicsRadius + 0.05f, 0.04f, Rgba8( 0, 0, 255, 255 ) );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( verts );
	}
}

void PlayerTank::Die()
{
	m_isDead = true;
	m_map->SpawnNewEntity( EntityType::_EXPLOSION, m_position, m_faction, this );
}

void PlayerTank::DebugRender() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 500 );
	AddVertsForDebugDrawLine( verts, m_position, m_orientationDegrees, m_cosmeticRadius, 0.02f, Rgba8( 255, 0, 0, 255 ) );
	AddVertsForDebugDrawLine( verts, m_position, m_orientationDegrees + 90.f, m_cosmeticRadius, 0.02f, Rgba8( 0, 255, 0, 255 ) );
	AddVertsForDebugDrawRing( verts, m_position, m_cosmeticRadius, 0.04f, Rgba8( 255, 0, 255, 255 ) );
	AddVertsForDebugDrawRing( verts, m_position, m_physicsRadius, 0.04f, Rgba8( 0, 255, 255, 255 ) );
	AddVertsForDebugDrawLine( verts, m_position, Atan2Degrees( m_velocity.y, m_velocity.x ), m_velocity.GetLength(), 0.02f, Rgba8( 255, 255, 0, 255 ) );
	AddVertsForDebugDrawLine( verts, m_position, m_turretRelativeOrientationDegrees + m_orientationDegrees, 0.7f, 0.2f, Rgba8( 0, 0, 255, 255 ) );
	if (m_isBodyTurning) {
		AddVertsForDebugDrawLine( verts, m_position + Vec2::MakeFromPolarDegrees( m_goalOrientationDegrees, m_cosmeticRadius ), m_goalOrientationDegrees, 0.2f, 0.1f, Rgba8( 255, 0, 0, 255 ) );
	}
	if (m_isTurretTurning) {
		AddVertsForDebugDrawLine( verts, m_position + Vec2::MakeFromPolarDegrees( m_turretGoalOrientationDegrees, m_cosmeticRadius - 0.2f ), m_turretGoalOrientationDegrees, 0.4f, 0.2f, Rgba8( 0, 0, 255, 255 ) );
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( (int)verts.size(), verts.data() );
}

void PlayerTank::RenderUI() const
{
	if (m_isDead || m_isGarbage || m_health <= 0.f) {
		return;
	}
	float remainHealthRatio = (float)m_health / (float)m_maxHealth;
	if (remainHealthRatio < 0.f) {
		remainHealthRatio = 0.f;
	}
	std::vector<Vertex_PCU> healthBarVerts;
	healthBarVerts.reserve( 24 );
	AddVertsForAABB2D( healthBarVerts, AABB2( m_position + Vec2( -0.3f, m_cosmeticRadius - 0.05f ), m_position + Vec2( 0.3f, m_cosmeticRadius + 0.05f ) ), Rgba8( 255, 0, 0, 255 ), AABB2::IDENTITY );
	AddVertsForAABB2D( healthBarVerts, AABB2( m_position + Vec2( -0.3f, m_cosmeticRadius - 0.05f ), m_position + Vec2( 0.3f - 0.6f * (1 - remainHealthRatio), m_cosmeticRadius + 0.05f ) ), Rgba8( 0, 255, 0, 255 ), AABB2::IDENTITY );
	
	float remainfuelRatio = m_fuel / MAX_FUEL;
	if (remainfuelRatio < 0.f) {
		remainfuelRatio = 0.f;
	}
	AddVertsForAABB2D( healthBarVerts, AABB2( m_position + Vec2( -0.3f, m_cosmeticRadius + 0.05f ), m_position + Vec2( 0.3f, m_cosmeticRadius + 0.15f ) ), Rgba8( 0, 0, 255, 255 ), AABB2::IDENTITY );
	AddVertsForAABB2D( healthBarVerts, AABB2( m_position + Vec2( -0.3f, m_cosmeticRadius + 0.05f ), m_position + Vec2( 0.3f - 0.6f * (1 - remainfuelRatio), m_cosmeticRadius + 0.15f ) ), Rgba8( 255, 255, 0, 255 ), AABB2::IDENTITY );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( healthBarVerts );
}

void PlayerTank::BeAttacked( float hit )
{
	if (!m_isImmortal) {
		m_health -= hit;
	}
	m_map->PlaySound( AudioName::PlayerHit, m_position );
}

void PlayerTank::Reborn()
{
	if (m_isDead) {
		m_isDead = false;
		m_health = m_maxHealth;
	}
}

void PlayerTank::TransferToMap( Map* map )
{
	m_map = map;
	m_position = map->GetEnterPoint();
	m_orientationDegrees = PLAYER_START_ROTATION;
	m_turretRelativeOrientationDegrees = 0.f;
	m_turretOrientationDegrees = m_orientationDegrees + m_turretRelativeOrientationDegrees;
}
