#include "Game/Effects.hpp"
#include "Game/Game.hpp"
#include "Game/Room.hpp"
#include "Game/PlayerController.hpp"
#include "Game/PlayerShip.hpp"

StarshipEffect::StarshipEffect( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:m_position( startPos )
	, m_orientationDegrees( startOrientation )
	, m_velocity( startVelocity )
{

}

StarshipEffect::~StarshipEffect()
{
	delete m_stateTimer;
	m_stateTimer = nullptr;
}

void StarshipEffect::RenderUI()
{

}

Vec2 StarshipEffect::GetForwardNormal() const
{
	return Vec2::MakeFromPolarDegrees( m_orientationDegrees );
}

bool StarshipEffect::IsAlive() const
{
	return !m_isDead;
}

Mat44 StarshipEffect::GetModelMatrix() const
{
	Mat44 translationMat = Mat44::CreateTranslation2D( m_position );
	translationMat.Append( Mat44::CreateZRotationDegrees( m_orientationDegrees ) );
	return translationMat;
}

StarshipReward::StarshipReward( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect( startPos, startOrientation, startVelocity )
{
	m_playerShip = g_theGame->GetPlayerEntity();
	m_physicsRadius = g_theGame->m_randNumGen->RollRandomFloatInRange( 0.4f, 0.6f );
	m_lootPosition = startPos + GetRandomPointOnUnitCircle2D() * g_theGame->m_randNumGen->RollRandomFloatInRange( 5.f, 8.f );
	m_stateTimer = new Timer( 0.2f, GetGameClock() );
	m_stateTimer->Start();
	m_startPosition = startPos;
	m_color = Rgba8( 255, 255, 0 );
	m_type = EffectType::Reward;
	m_renderBeforeEntity = true;
}

StarshipReward::~StarshipReward()
{
}

void StarshipReward::BeginPlay()
{

}

void StarshipReward::Update( float deltaTime )
{
	UNUSED( deltaTime );

	if (m_isDead) {
		m_isGarbage = true;
		return;
	}

	if (m_state == 0 && m_stateTimer->HasPeriodElapsed()) {
		m_stateTimer->Stop();
		m_state = 1;
	}

	if (m_state == 0) {
		m_position = Interpolate( m_startPosition, m_lootPosition, m_stateTimer->GetElapsedFraction() );
		g_theGame->m_curRoom->SetPositionInsideRoom( m_position, m_physicsRadius );
	}
	else if (m_state == 1) {
		if (m_gotoTarget) {
			m_stateTimer->SetPeriodSeconds( 1.f );
			m_stateTimer->Start();
			m_state = 2;
		}
		else {
			if (IsPointInsideDisc2D( m_playerShip->m_position, m_position, m_lootRadius ) || (g_theGame->m_curRoom && !IsPointInsideAABB2D( m_position, g_theGame->m_curRoom->m_bounds ))) {
				m_stateTimer->SetPeriodSeconds( 0.5f );
				m_stateTimer->Start();
				m_state = 2;
			}
		}
	}
	else if (m_state == 2) {
		if (m_gotoTarget) {
			if (m_target && m_target->IsAlive()) {
				m_position = Interpolate( m_lootPosition, m_target->m_position, SmoothStart2( m_stateTimer->GetElapsedFraction() ) );
				if (IsPointInsideDisc2D( m_position, m_target->m_position, m_target->m_physicsRadius * 0.6f )) {
					Die();
				}
			}
			else {
				Die();
			}
		}
		else {
			m_position = Interpolate( m_lootPosition, m_playerShip->m_position, m_stateTimer->GetElapsedFraction() );
			if (IsPointInsideDisc2D( m_position, m_playerShip->m_position, m_playerShip->m_physicsRadius )) {
				Die();
				((PlayerController*)(m_playerShip->m_controller))->GetReward( 1 );
			}
		}
	}

	//ClampIntoCurRoom( m_position, m_physicsRadius );
}

void StarshipReward::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForDisc2D( verts, m_position, m_physicsRadius, m_color );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void StarshipReward::Die()
{
	m_isDead = true;
}

StarshipShield::StarshipShield( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect( startPos, startOrientation, startVelocity )
{
	m_stateTimer = new Timer( 0.1f, GetGameClock() );
	m_stateTimer->Start();
	m_type = EffectType::Shield;
	m_renderBeforeEntity = false;
}

StarshipShield::~StarshipShield()
{

}

void StarshipShield::BeginPlay()
{
	m_orientationDegrees = m_owner->m_orientationDegrees;
	m_verts.reserve( 72 );
	float sectorApertureDegrees = m_sectorApertureDegrees;
	constexpr int NUM_OF_VERTS = 72;
	float startDegrees = -sectorApertureDegrees * 0.5f;
	float stepDegrees = sectorApertureDegrees / NUM_OF_VERTS * 3.f;
	float radius = m_owner->m_cosmeticRadius * 1.5f;
	float distance = CosDegrees( m_sectorApertureDegrees * 0.5f ) * radius;
	Vec2 startPosition = Vec2( distance, 0.f );
	for (int i = 0; i < NUM_OF_VERTS / 3; i++) {
		m_verts.emplace_back( startPosition, Rgba8( 102, 178, 255, 100 ) );
		m_verts.emplace_back( Vec2::MakeFromPolarDegrees( startDegrees + stepDegrees * i, radius ), Rgba8( 102, 178, 255, (unsigned char)(255.f * (1 - 0.5f * SmoothStart3( m_stateTimer->GetElapsedFraction() ))) ) );
		m_verts.emplace_back( Vec2::MakeFromPolarDegrees( startDegrees + stepDegrees * (i + 1), radius ), Rgba8( 102, 178, 255, (unsigned char)(255.f * (1 - 0.5f * SmoothStart3( m_stateTimer->GetElapsedFraction() ))) ) );
	}
}

void StarshipShield::Update( float deltaTime )
{
	UNUSED( deltaTime );

	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_stateTimer->HasPeriodElapsed()) {
		Die();
		return;
	}
	if (m_owner->IsAlive()) {
		m_position = m_owner->m_position;
		m_orientationDegrees = m_owner->m_orientationDegrees;
	}
	else {
		Die();
	}
}

void StarshipShield::Render() const
{
	if (m_isDead) {
		return;
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix() );
	g_theRenderer->DrawVertexArray( m_verts );
}

void StarshipShield::Die()
{
	m_isGarbage = true;
}

RayLaser::RayLaser( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_stateTimer = new Timer( 0.2f, GetGameClock() );
	m_stateTimer->Start();
	m_type = EffectType::Laser;
	m_renderBeforeEntity = true;
}

RayLaser::~RayLaser()
{

}

void RayLaser::BeginPlay()
{

}

void RayLaser::Update( float deltaTime )
{
	UNUSED( deltaTime );

	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_stateTimer->HasPeriodElapsed()) {
		Die();
		return;
	}
	if (m_owner->IsAlive()) {
		//m_position = m_owner->m_position;
		//m_orientationDegrees = m_owner->m_orientationDegrees;
	}
	else {
		Die();
	}
}

void RayLaser::Render() const
{
	if (m_isDead) {
		return;
	}
	
	std::vector<Vertex_PCU> verts;
	AddVertsForLineSegment2D( verts, m_position, m_rayEndPos,
		m_physicsRadius * 0.1f + m_physicsRadius * 0.9f * (Hesitate5( m_stateTimer->GetElapsedFraction() ) > 0.5f ? 1 - Hesitate5( m_stateTimer->GetElapsedFraction() ) : Hesitate5( m_stateTimer->GetElapsedFraction() )), Rgba8( 255, 153, 51, 200 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void RayLaser::Die()
{
	m_isGarbage = true;
}

SlashEffect::SlashEffect( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_stateTimer = new Timer( 0.4f, GetGameClock() );
	m_stateTimer->Start();
	m_type = EffectType::Slash;
	m_renderBeforeEntity = false;
}

SlashEffect::~SlashEffect()
{

}

void SlashEffect::BeginPlay()
{
	m_length = m_curve.GetApproximateLength();
}

void SlashEffect::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_stateTimer->HasPeriodElapsed()) {
		Die();
		return;
	}
	if (m_owner->IsAlive()) {
		m_position = m_owner->m_position;
		m_orientationDegrees = m_owner->m_orientationDegrees;
		m_modelMatrix = GetModelMatrix();
		float fraction = SmoothStop4( m_stateTimer->GetElapsedFraction() );
		m_curDistance = m_length * (fraction + 0.6f > 1.f ? 1.f : fraction + 0.6f);
		m_lastDistance = m_length * fraction;

		Vec2 point = m_curve.EvaluateAtApproximateDistance( m_curDistance );
		point = m_modelMatrix.TransformPosition2D( point );
		for (auto entity : g_theGame->m_entityArray) {
			if (entity && entity->IsAlive() && !entity->IsInvincible() && entity != m_owner 
				&& entity->m_def.m_faction != m_owner->m_def.m_faction
				&& DoDiscsOverlap( point, 5.f, entity->m_position, entity->m_physicsRadius)) {
				entity->BeAttackedOnce( m_owner->GetMainWeaponDamage(), Vec2( 0.f, 0.f ), m_stateTimer->GetPeriodSeconds(), this );
			}
		}
	}
	else {
		Die();
	}
}

void SlashEffect::Render() const
{
	if (m_isDead) {
		return;
	}
	std::vector<Vertex_PCU> verts;
	constexpr int subdivisions = 32;
	float lengthPerSubdivision = (m_curDistance - m_lastDistance) / (float)subdivisions;
	Vec2 lastPos = m_curve.EvaluateAtApproximateDistance( m_lastDistance );
	Vec2 curPos;
	for (int i = 0; i < subdivisions; i++) {
		curPos = m_curve.EvaluateAtApproximateDistance( m_lastDistance + (i + 1) * lengthPerSubdivision );
		AddVertsForLineSegment2D( verts, lastPos, curPos, m_owner->m_cosmeticRadius * 1.2f * (1.f - (m_lastDistance + (i + 1) * lengthPerSubdivision) / m_length), Rgba8::WHITE );
		lastPos = curPos;
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( m_modelMatrix );
	g_theRenderer->DrawVertexArray( verts );
}

void SlashEffect::Die()
{
	m_isGarbage = true;
}

LevelPortal::LevelPortal( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{

}

LevelPortal::~LevelPortal()
{

}

void LevelPortal::BeginPlay()
{
	m_player = g_theGame->GetPlayerEntity();
	if (m_dir == RoomDirection::UP) {
		m_bounds = AABB2( Vec2(), Vec2( 15.f, 4.f ) );
		m_bounds.SetCenter( m_position );
	}
	else if (m_dir == RoomDirection::DOWN) {
		m_bounds = AABB2( Vec2(), Vec2( 15.f, 4.f ) );
		m_bounds.SetCenter( m_position );
	}
	else if (m_dir == RoomDirection::LEFT) {
		m_bounds = AABB2( Vec2(), Vec2( 4.f, 15.f ) );
		m_bounds.SetCenter( m_position );
	}
	else if (m_dir == RoomDirection::RIGHT) {
		m_bounds = AABB2( Vec2(), Vec2( 4.f, 15.f ) );
		m_bounds.SetCenter( m_position );
	}
	else {
		m_bounds = AABB2( Vec2(), Vec2( 10.f, 10.f ) );
		m_bounds.SetCenter( m_position );
	}
}

void LevelPortal::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_owner == g_theGame->m_curRoom && m_resetByEntering && !DoDiscAndAABB2Overlap2D( m_player->m_position, m_player->m_physicsRadius, m_bounds )) {
		m_resetByEntering = false;
	}
	if (g_theGame->CanLeaveCurrentRoom()) {
		m_isOpen = true;
		m_color = Rgba8( 0, 200, 0 );
		if (!m_resetByEntering && DoDiscAndAABB2Overlap2D( m_player->m_position, m_player->m_physicsRadius, m_bounds )) {
			Room* nextRoom = g_theGame->GetRoomInDirectionByCurRoom( m_dir );
			g_theGame->EnterRoom( nextRoom, GetOppositeRoomDir(m_dir) );
		}
	}
	else {
		m_isOpen = false;
		m_color = Rgba8( 200, 0, 0 );
	}

}

void LevelPortal::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, m_bounds, m_color );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void LevelPortal::Die()
{

}

Missle::Missle( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
}

Missle::~Missle()
{
}

void Missle::BeginPlay()
{
	// calculate start velocity
	if (m_flyTime > 0.1f) {
		float vy = (m_targetPos.y - m_startPos.y) / m_flyTime + m_gravity * 0.5f * m_flyTime;
		float vx = (m_targetPos.x - m_startPos.x) / m_flyTime;
		m_velocity = Vec2( vx, vy );
		m_accelerateVelocity = Vec2( 0.f, -m_gravity );
	}
	m_stateTimer = new Timer( m_flyTime, GetGameClock() );
	m_stateTimer->Start();
	m_faction = m_owner->m_def.m_faction;
	m_damage = m_owner->GetMainWeaponDamage();
}

void Missle::Update( float deltaTime )
{
	if (m_stateTimer->HasPeriodElapsed()) {
		if (m_state == 0) {
			m_state = 1;
			m_stateTimer->SetPeriodSeconds( m_blastTime );
			m_stateTimer->Start();
			// add a particle for blast effect
			ParticleSystem2DAddEmitter( 1000, m_blastTime * 0.2f, 
				AABB2( m_targetPos - Vec2( m_maxRadius, m_maxRadius ) * 0.2f, m_targetPos + Vec2( m_maxRadius, m_maxRadius ) * 0.2f ),
				FloatRange( m_maxRadius * 0.1f, m_maxRadius * 0.2f ),
				AABB2( Vec2( -m_maxRadius * 3.f, 0.f ), Vec2( m_maxRadius * 3.f, 50.f ) ),
				FloatRange( m_blastTime * 0.8f, m_blastTime * 1.2f ), Rgba8( 255, 128, 0 ), Particle2DShape::Asteroid, true, FloatRange( 0.f, 360.f ),
				FloatRange( 30.f, 90.f ), nullptr,
				Rgba8( 255, 128, 0, 0 ), 15.f, 0.3f );
		}
		else if (m_state == 1) {
			Die();
		}
	}

	if (m_state == 0) {
		m_velocity += deltaTime * m_accelerateVelocity;
		m_position += m_velocity * deltaTime;
		m_orientationDegrees = m_velocity.GetOrientationDegrees();
	}
	else if (m_state == 1) {
		m_currentRadius = m_maxRadius * SmoothStop2( m_stateTimer->GetElapsedFraction() );
		for (auto entity : g_theGame->m_entityArray) {
			if (entity && entity->IsAlive() && !entity->IsInvincible() && entity != m_owner
				&& entity->m_def.m_faction != m_faction
				&& IsPointInsideDisc2D( entity->m_position, m_targetPos, m_currentRadius )) {
				entity->BeAttackedOnce( m_damage, Vec2( 0.f, 0.f ), m_stateTimer->GetPeriodSeconds(), this );
			}
		}
	}
}

void Missle::Render() const
{
	std::vector<Vertex_PCU> verts;
	if (m_state == 0) {
		AddVertsForCapsule2D( verts, m_position + GetForwardNormal(), m_position - GetForwardNormal(), 0.5f, Rgba8( 255, 128, 0 ) );
	}
	else if (m_state == 1) {
		Vec2 center = m_targetPos;
		float radius = m_currentRadius;
		float thickness = 1.f;
		Rgba8 color1( 153, 76, 0 );
		Rgba8 color2( 153, 76, 0, 100 );
		constexpr int NUM_SIDES = 16;
		constexpr int NUM_TRIANGLES = NUM_SIDES * 2;
		constexpr int NUM_VERTS = 3 * NUM_TRIANGLES;
		constexpr float DEGREES_PER_SIDE = 360.f / (float)NUM_SIDES;
		constexpr float RADIANS_PER_SIDE = DEGREES_PER_SIDE * PI / 180.f;
		verts.resize( NUM_VERTS );
		for (int i = 0; i < NUM_SIDES; i++) {
			verts[6 * i] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * radius, SinRadians( RADIANS_PER_SIDE * i ) * radius, 0 ), color2, Vec2( 0, 0 ) );
			verts[6 * i + 2] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, 0 ), color2, Vec2( 0, 0 ) );
			verts[6 * i + 1] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), 0 ), color1, Vec2( 0, 0 ) );
			verts[6 * i + 3] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, 0 ), color2, Vec2( 0, 0 ) );
			verts[6 * i + 4] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), 0 ), color1, Vec2( 0, 0 ) );
			verts[6 * i + 5] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness), 0 ), color1, Vec2( 0, 0 ) );
		}
		for (int i = 0; i < NUM_VERTS; i++) {
			verts[i].m_position += Vec3( center.x, center.y, 0 );
		}
	}

	// add a cross indicating the target position
	constexpr float crossLength = 3.f;
	AddVertsForLineSegment2D( verts, m_targetPos + Vec2( 0.707f, 0.707f ) * crossLength, m_targetPos + Vec2( -0.707f, -0.707f ) * crossLength, 0.6f, Rgba8( 255, 0, 0 ) );
	AddVertsForLineSegment2D( verts, m_targetPos + Vec2( -0.707f, 0.707f ) * crossLength, m_targetPos + Vec2( 0.707f, -0.707f ) * crossLength, 0.6f, Rgba8( 255, 0, 0 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void Missle::Die()
{
	m_isGarbage = true;
}

PlayerShield::PlayerShield( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_color = Rgba8( 0, 255, 255, 150 );
	m_type = EffectType::PlayerShield;
}

PlayerShield::~PlayerShield()
{

}

void PlayerShield::BeginPlay()
{

}

void PlayerShield::Update( float deltaTime )
{
	UNUSED( deltaTime );
	m_position = m_owner->m_position;
}

void PlayerShield::Render() const
{
	if (!m_isActivated) {
		return;
	}
	constexpr int NUM_OF_VERTS = 90;
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	for (int i = 0; i < NUM_OF_VERTS / 3; i++) {
		verts.emplace_back( m_position, Rgba8( m_color.r, m_color.g, m_color.b, 0 ) );
		verts.emplace_back( m_position + Vec2( CosRadians( 6 * PI / NUM_OF_VERTS * i ) * m_radius, SinRadians( 6 * PI / NUM_OF_VERTS * i ) * m_radius ), m_color );
		verts.emplace_back( m_position + Vec2( CosRadians( 6 * PI / NUM_OF_VERTS * (i + 1) ) * m_radius, SinRadians( 6 * PI / NUM_OF_VERTS * (i + 1) ) * m_radius ), m_color );
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void PlayerShield::Die()
{
	m_isGarbage = true;
}

StarshipLaser::StarshipLaser( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_stateTimer = new Timer( 0.5f, GetGameClock() );
	m_stateTimer->Start();
	m_state = 0;
	m_type = EffectType::SubWeaponLaser;
	m_renderBeforeEntity = true;
}

StarshipLaser::~StarshipLaser()
{

}

void StarshipLaser::BeginPlay()
{

}

void StarshipLaser::Update( float deltaTime )
{
	UNUSED( deltaTime );

	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_target && m_target->m_isDead) {
		m_target = nullptr;
	}
	if (m_state == 0 && m_stateTimer->HasPeriodElapsed()) {
		m_state = 1;
		m_stateTimer->SetPeriodSeconds( 0.3f );
		m_stateTimer->Start();
		g_theGame->SetOneDirCameraShake( -(m_rayEndPos - m_position).GetNormalized(), 0.1f );
		if (m_target && m_target->IsAlive()) {
			m_target->BeAttacked( m_damage, Vec2(), true );
		}
	}
	if (m_state == 1 && m_stateTimer->HasPeriodElapsed()) {
		Die();
		return;
	}
	if (m_owner->IsAlive()) {
		m_position = m_owner->m_position;
		if (m_target) {
			m_rayEndPos = m_target->m_position;
		}
	}
	else {
		Die();
	}
}

void StarshipLaser::Render() const
{
	if (m_isDead) {
		return;
	}

	std::vector<Vertex_PCU> verts;
	if (m_state == 0) {
		AddVertsForLineSegment2D( verts, m_position, m_rayEndPos, 0.5f, Rgba8( 51, 153, 255, 100 ) );
	}
	else if (m_state == 1) {
		AddVertsForLineSegment2D( verts, m_position, m_rayEndPos,
			m_physicsRadius * 0.1f + m_physicsRadius * 0.9f * (Hesitate5( m_stateTimer->GetElapsedFraction() ) > 0.5f ?
				1.f - Hesitate5( m_stateTimer->GetElapsedFraction() ) : Hesitate5( m_stateTimer->GetElapsedFraction() )),
			Rgba8( 51, 153, 255, unsigned char( 200 * (1.f - m_stateTimer->GetElapsedFraction()) ) ) );
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void StarshipLaser::Die()
{
	m_isDead = true;
}

StarshipMine::StarshipMine( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_redTimer = new Timer( 0.08f, GetGameClock() );
	m_color = Rgba8( 255, 153, 51 );
}

StarshipMine::~StarshipMine()
{
	delete m_redTimer;
}

void StarshipMine::BeginPlay()
{
	m_stateTimer = new Timer( m_blastTime, GetGameClock() );
	m_stateTimer->Start();
}

void StarshipMine::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_stateTimer->HasPeriodElapsed()) {
		g_theGame->DealRangeDamage( m_damage, m_position, m_explosionRadius );
		Die();
		return;
	}

	float smoothStart = SmoothStart2( m_stateTimer->GetElapsedFraction() );
	if (m_redTimer->IsStopped()) {
		if (smoothStart > m_count) {
			m_count += m_step;
			m_redTimer->Start();
			m_color = Rgba8( 255, 0, 0 );
		}
	}

	if (m_redTimer->HasPeriodElapsed()) {
		m_redTimer->Stop();
		m_color = Rgba8( 255, 153, 51 );
	}

}

void StarshipMine::Render() const
{
	if (m_isDead) {
		return;
	}

	std::vector<Vertex_PCU> verts;

	Vec2 center = m_position;
	float radius = m_explosionRadius;
	float thickness = 0.5f;
	Rgba8 color1( 153, 76, 0 );
	Rgba8 color2( 153, 76, 0, 100 );
	constexpr int NUM_SIDES = 32;
	constexpr int NUM_TRIANGLES = NUM_SIDES * 2;
	constexpr int NUM_VERTS = 3 * NUM_TRIANGLES;
	constexpr float DEGREES_PER_SIDE = 360.f / (float)NUM_SIDES;
	constexpr float RADIANS_PER_SIDE = DEGREES_PER_SIDE * PI / 180.f;
	verts.resize( NUM_VERTS );
	for (int i = 0; i < NUM_SIDES; i++) {
		verts[6 * i] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * radius, SinRadians( RADIANS_PER_SIDE * i ) * radius, 0 ), color2, Vec2( 0, 0 ) );
		verts[6 * i + 2] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, 0 ), color2, Vec2( 0, 0 ) );
		verts[6 * i + 1] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), 0 ), color1, Vec2( 0, 0 ) );
		verts[6 * i + 3] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, 0 ), color2, Vec2( 0, 0 ) );
		verts[6 * i + 4] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), 0 ), color1, Vec2( 0, 0 ) );
		verts[6 * i + 5] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness), 0 ), color1, Vec2( 0, 0 ) );
	}
	for (int i = 0; i < NUM_VERTS; i++) {
		verts[i].m_position += Vec3( center.x, center.y, 0 );
	}

	AddVertsForDisc2D( verts, m_position, 2.f, m_color );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void StarshipMine::Die()
{
	ParticleSystem2DAddEmitter( 1000, m_blastTime * 0.05f,
		AABB2( m_position - Vec2( m_explosionRadius, m_explosionRadius ) * 0.2f, m_position + Vec2( m_explosionRadius, m_explosionRadius ) * 0.2f ),
		FloatRange( m_explosionRadius * 0.1f, m_explosionRadius * 0.2f ),
		AABB2( Vec2( -m_explosionRadius * 2.f, -15.f ), Vec2( m_explosionRadius * 2.f, 30.f ) ),
		FloatRange( m_blastTime * 0.2f, m_blastTime * 0.4f ), Rgba8( 255, 128, 0 ), Particle2DShape::Asteroid, true, FloatRange( 0.f, 360.f ),
		FloatRange( 30.f, 90.f ), nullptr,
		Rgba8( 255, 128, 0, 0 ), 15.f, 0.3f );
	m_isDead = true;
}

HealthPickup::HealthPickup( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect( startPos, startOrientation, startVelocity )
{
	m_playerShip = g_theGame->GetPlayerObject();
	m_physicsRadius = 3.f;
	m_renderBeforeEntity = true;
	m_color = Rgba8( 255, 0, 0 );
	m_room = g_theGame->m_curRoom;
}

HealthPickup::~HealthPickup()
{

}

void HealthPickup::BeginPlay()
{

}

void HealthPickup::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}

	if (DoDiscsOverlap(m_position, m_physicsRadius, m_playerShip->m_position, m_playerShip->m_physicsRadius)) {
		if (m_playerShip->m_health < m_playerShip->m_maxHealth) {
			m_playerShip->GainHealth( 1.f );
			Die();
		}
		else {
			PushDiscsOutOfEachOther2D( m_position, m_physicsRadius, m_playerShip->m_position, m_playerShip->m_physicsRadius );
		}
	}

	ClampIntoRoom( m_position, m_physicsRadius, m_room );
}

void HealthPickup::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForAABB2D( verts, AABB2( Vec2( -m_physicsRadius * 0.8f, -m_physicsRadius * 0.8f ), Vec2( m_physicsRadius * 0.8f, m_physicsRadius * 0.8f ) ), Rgba8::WHITE, g_pickupsSprites->GetSpriteUVs( 0 ) );
	g_theRenderer->BindTexture( &g_pickupsSprites->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix() );
	g_theRenderer->DrawVertexArray( verts );
}

void HealthPickup::Die()
{
	m_isDead = true;
}

ArmorPickup::ArmorPickup( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect( startPos, startOrientation, startVelocity )
{
	m_playerShip = g_theGame->GetPlayerObject();
	m_physicsRadius = 3.f;
	m_renderBeforeEntity = true;
	m_color = Rgba8( 0, 128, 155 );
	m_room = g_theGame->m_curRoom;
}

ArmorPickup::~ArmorPickup()
{

}

void ArmorPickup::BeginPlay()
{

}

void ArmorPickup::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}

	if (DoDiscsOverlap( m_position, m_physicsRadius, m_playerShip->m_position, m_playerShip->m_physicsRadius )) {
		if (m_playerShip->m_curArmor < m_playerShip->m_maxArmor) {
			m_playerShip->GainArmor( 1.f );
			Die();
		}
		else {
			PushDiscsOutOfEachOther2D( m_position, m_physicsRadius, m_playerShip->m_position, m_playerShip->m_physicsRadius );
		}
	}
	ClampIntoRoom( m_position, m_physicsRadius, m_room );
}

void ArmorPickup::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForAABB2D( verts, AABB2( Vec2( -m_physicsRadius * 0.8f, -m_physicsRadius * 0.8f ), Vec2( m_physicsRadius * 0.8f, m_physicsRadius * 0.8f ) ), Rgba8::WHITE, g_pickupsSprites->GetSpriteUVs( 1 ) );
	g_theRenderer->BindTexture( &g_pickupsSprites->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix() );
	g_theRenderer->DrawVertexArray( verts );
}

void ArmorPickup::Die()
{
	m_isDead = true;
}

PersistentRay::PersistentRay( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_renderBeforeEntity = true;
}

PersistentRay::~PersistentRay()
{

}

void PersistentRay::BeginPlay()
{
	m_faction = m_owner->m_def.m_faction;
	if (m_updatePositionByOwner && m_owner) {
		m_relativeOrientation = m_orientationDegrees - m_owner->m_orientationDegrees;
	}
}

void PersistentRay::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_owner->m_isDead) {
		Die();
		return;
	}
	if (m_lifeTimeSeconds < 0.f) {
		Die();
		return;
	}

	m_lifeTimeSeconds -= deltaTime;
	if (m_curLength < m_maxLength) {
		m_curLength += 150.f * deltaTime;
		if (m_curLength > m_maxLength) {
			m_curLength = m_maxLength;
		}
	}
	m_width = GetRandGen()->RollRandomFloatInRange( m_minWidth, m_maxWidth );

	if (m_updatePositionByOwner && m_owner) {
		m_position = m_owner->m_position;
		m_orientationDegrees = m_owner->m_orientationDegrees + m_relativeOrientation;
	}

	for (auto entity : g_theGame->m_entityArray) {
		if (entity && entity->m_def.m_faction != m_faction && entity->IsAlive() && !entity->IsInvincible() && entity != m_owner) {
			Vec2 forwardVec = Vec2::MakeFromPolarDegrees( m_orientationDegrees );
			Vec2 center = m_position + forwardVec * m_curLength * 0.5f;
			OBB2 obbCollider( center, forwardVec, Vec2( m_curLength * 0.5f, m_width * 0.5f ) );
			if (DoDiscAndOBB2Overlap2D( entity->m_position, entity->m_physicsRadius, obbCollider )) {
				entity->BeAttackedOnce( m_damage, -forwardVec, m_damageCoolDown, this, false );
				m_curLength = GetDistance2D( m_position, entity->m_position );
			}
		}
	}

}

void PersistentRay::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForAABB2D( verts, AABB2( Vec2( 0.f, -m_minWidth * 0.5f ), Vec2( m_curLength, m_minWidth * 0.5f ) ), Rgba8::WHITE );
	verts.emplace_back( Vec2( 0.f, -m_minWidth * 0.5f ), Rgba8( 255, 255, 255, 255 ) );
	verts.emplace_back( Vec2( 0.f, -m_width * 0.5f ), Rgba8( 255, 255, 255, 60 ) );
	verts.emplace_back( Vec2( m_curLength, -m_width * 0.5f ), Rgba8( 255, 255, 255, 60 ) );
	verts.emplace_back( Vec2( 0.f, -m_minWidth * 0.5f ), Rgba8( 255, 255, 255, 255 ) );
	verts.emplace_back( Vec2( m_curLength, -m_width * 0.5f ), Rgba8( 255, 255, 255, 60 ) );
	verts.emplace_back( Vec2( m_curLength, -m_minWidth * 0.5f ), Rgba8( 255, 255, 255, 255 ) );
	verts.emplace_back( Vec2( 0.f, m_width * 0.5f ), Rgba8( 255, 255, 255, 60 ) );
	verts.emplace_back( Vec2( 0.f, m_minWidth * 0.5f ), Rgba8( 255, 255, 255, 255 ) );
	verts.emplace_back( Vec2( m_curLength, m_minWidth * 0.5f ), Rgba8( 255, 255, 255, 255 ) );
	verts.emplace_back( Vec2( 0.f, m_width * 0.5f ), Rgba8( 255, 255, 255, 60 ) );
	verts.emplace_back( Vec2( m_curLength, m_minWidth * 0.5f ), Rgba8( 255, 255, 255, 255 ) );
	verts.emplace_back( Vec2( m_curLength, m_width * 0.5f ), Rgba8( 255, 255, 255, 60 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void PersistentRay::Die()
{
	m_isDead = true;
}

NextFloorPortal::NextFloorPortal( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_bounds = AABB2( m_position - Vec2( 6.f, 5.f ), m_position + Vec2( 6.f, 5.f ) );
	m_player = g_theGame->GetPlayerObject();
	m_type = EffectType::FloorPortal;
	m_renderBeforeEntity = true;
}

NextFloorPortal::~NextFloorPortal()
{

}

void NextFloorPortal::BeginPlay()
{

}

void NextFloorPortal::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	m_isOverlap = false;
	if (g_theGame->m_curRoom->m_itemChooseMode) {
		return;
	}
	if (!g_theGame->CanLeaveCurrentRoom()) {
		return;
	}
	if (m_isDead) {
		return;
	}
	if (DoDiscAndAABB2Overlap2D( m_player->m_position, m_player->m_physicsRadius, m_bounds )) {
		m_isOverlap = true;
		if (g_theInput->WasKeyJustPressed( PLAYER_INTERACT_KEYCODE )) {
			g_theGame->m_goToNextFloorNextFrame = true;
			//g_theGame->GoToNextFloor();
			//Die();
		}
	}
}

void NextFloorPortal::Render() const
{
	if (g_theGame->m_curRoom->m_itemChooseMode) {
		return;
	}
	if (m_isDead) {
		return;
	}
	if (!g_theGame->CanLeaveCurrentRoom()) {
		return;
	}
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForAABB2D( verts, m_bounds, Rgba8::WHITE );

	g_theRenderer->BindTexture( g_floorPortalTexture );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void NextFloorPortal::Die()
{
	m_isDead = true;
}

void NextFloorPortal::RenderUI()
{
	if (m_isOverlap) {
		std::vector<Vertex_PCU> textVerts;
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 120.f ), Vec2( 1300.f, 160.f ) ), 40.f, Stringf( "%c to Go to Next Floor", PLAYER_INTERACT_KEYCODE ) );
		g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( textVerts );
	}
}

BacteriaDrop::BacteriaDrop( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_type = EffectType::BacteriaDrop;
}

BacteriaDrop::~BacteriaDrop()
{

}

void BacteriaDrop::BeginPlay()
{
	m_lifeTimer = new Timer( m_travelTime, GetGameClock() );
	m_lifeTimer->Start();
	m_startPos = m_position;
}

void BacteriaDrop::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	m_position = Interpolate( m_startPos, m_targetPos, m_lifeTimer->GetElapsedFraction() );
	if (m_lifeTimer->HasPeriodElapsed()) {
		Die();
	}
}

void BacteriaDrop::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForCapsule2D( verts, Vec2( -0.5f, 0.f ), Vec2( 0.5f, 0.f ), 0.3f, Rgba8( 204, 255, 153 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix() );
	g_theRenderer->DrawVertexArray( verts );
}

void BacteriaDrop::Die()
{
	m_isDead = true;
	Entity* e = g_theGame->SpawnEntityToGame( *m_entityToSpawn, m_position, GetRandGen()->RollRandomFloatInRange( 0.f, 360.f ) );
	e->m_hasReward = false;
}

BacteriaSap::BacteriaSap( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_lifeTimer = new Timer( m_lifeTime, GetGameClock() );
	m_renderBeforeEntity = true;
}

BacteriaSap::~BacteriaSap()
{
	delete m_lifeTimer;
}

void BacteriaSap::BeginPlay()
{
	m_lifeTimer->SetPeriodSeconds( m_lifeTime );
	m_lifeTimer->Start();
	for (int i = 0; i < m_numOfDisc; i++) {
		m_centerPos.push_back( Starship_GetRandomPosInCapsule( m_startPos, m_endPos, m_capsuleRadius ) );
		m_discRadius.push_back( GetRandGen()->RollRandomFloatInRange( m_maxRadius * 0.75f, m_maxRadius ) );
		m_a.push_back( (unsigned char)GetRandGen()->RollRandomIntInRange( 50, 150 ) );
	}
}

void BacteriaSap::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead || m_isGarbage) {
		return;
	}
	if (m_lifeTimer->HasPeriodElapsed()) {
		Die();
	}
}

void BacteriaSap::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( (size_t)100 * (size_t)m_numOfDisc );

	for (int i = 0; i < m_numOfDisc; i++) {
		AddVertsForDisc2D( verts, m_centerPos[i], Interpolate( m_discRadius[i] * 0.9f, m_discRadius[i], SmoothStart2( m_lifeTimer->GetElapsedFraction() ) ), 
			Rgba8( 255, 255, 255, m_a[i] ) );
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void BacteriaSap::Die()
{
	m_isGarbage = true;
}

SprayAttack::SprayAttack( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_attackTimer = new Timer( 1.f, GetGameClock() );
	m_sapTimer = new Timer( 0.099f, GetGameClock() );
}

SprayAttack::~SprayAttack()
{
	delete m_attackTimer;
	delete m_sapTimer;
}

void SprayAttack::BeginPlay()
{
	m_sapTimer->Start();
	m_attackTimer->Start();
	m_boundingBox.m_halfDimensions = Vec2( 0.f, m_width * 0.5f );
	m_startPos = m_boundingBox.m_center;
	Vec2 const& forwardVec = m_boundingBox.m_iBasisNormal;
	Vec2 const& leftVec = forwardVec.GetRotated90Degrees();
	ParticleSystem2DAddEmitter( 200, 0.8f, AABB2( m_startPos + m_width * 0.5f * leftVec, m_startPos - m_width * 0.5f * leftVec ),
		FloatRange( 1.f, 3.f ),
		AABB2( forwardVec * m_dist, forwardVec * m_dist ),
		FloatRange( 1.f, 1.f ), Rgba8( 0, 200, 0 ), Particle2DShape::Asteroid, true, FloatRange( 0.f, 360.f ),
		FloatRange( 0.f, 0.f ), nullptr,
		Rgba8( 0, 200, 0, 0 ), 0.f, 0.f );
}

void SprayAttack::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}

	if (m_attackTimer->HasPeriodElapsed()) {
		Die();
		return;
	}

	m_boundingBox.m_halfDimensions.x = m_dist * m_attackTimer->GetElapsedFraction() * 0.5f;
	m_boundingBox.m_center = m_startPos + m_boundingBox.m_iBasisNormal * m_boundingBox.m_halfDimensions.x;

	if (m_sapTimer->DecrementPeriodIfElapsed()) {
		BacteriaSap* sap = (BacteriaSap*)g_theGame->SpawnEffectToGame( EffectType::BacteriaSap, m_boundingBox.m_center );
		sap->m_startPos = m_boundingBox.m_center;
		sap->m_endPos = m_boundingBox.m_center;
		sap->m_capsuleRadius = m_width;
		sap->m_maxRadius = m_width * 1.1f;
		sap->m_numOfDisc = 5;
		sap->m_lifeTime = 3.f;
		sap->m_color = Rgba8( 0, 153, 0 );
		sap->BeginPlay();
	}

	for (auto entity : g_theGame->m_entityArray) {
		if (entity && entity->m_def.m_faction != m_faction && DoDiscAndOBB2Overlap2D( entity->m_position, entity->m_physicsRadius, m_boundingBox )) {
			entity->BeAttackedOnce( m_damage, -m_boundingBox.m_iBasisNormal, 1.f, this, false, m_boundingBox.m_iBasisNormal );
			if (m_isPoisonous) {
				entity->m_poisonTimer += 0.1f;
				entity->m_isPoisoned = true;
			}
		}
	}
}

void SprayAttack::Render() const
{
	return;
}

void SprayAttack::Die()
{
	m_isDead = true;
}

SectorSprayAttack::SectorSprayAttack( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect( startPos, startOrientation, startVelocity )
{
	m_attackTimer = new Timer( 1.f, GetGameClock() );
	m_sapTimer = new Timer( 0.099f, GetGameClock() );
}

SectorSprayAttack::~SectorSprayAttack()
{
	delete m_attackTimer;
	delete m_sapTimer;
}

void SectorSprayAttack::BeginPlay()
{
	m_sapTimer->Start();
	m_attackTimer->Start();
	m_startPos = m_position;
	ParticleSystem2DAddEmitter(1000, 0.8f, AABB2(m_startPos, m_startPos),
		FloatRange( 1.f, 3.f ),
		AABB2( Vec2( -m_dist, -m_dist ), Vec2( m_dist, m_dist ) ),
		FloatRange( 1.f, 1.f ), Rgba8( 0, 200, 0 ), Particle2DShape::Asteroid, true, FloatRange( 0.f, 360.f ),
		FloatRange( 0.f, 0.f ), nullptr,
		Rgba8( 0, 200, 0, 0 ), 0.f, 0.f );
}

void SectorSprayAttack::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}

	if (m_attackTimer->HasPeriodElapsed()) {
		Die();
	}

	m_radius = m_dist * SmoothStop2( m_attackTimer->GetElapsedFraction() );

	if (m_sapTimer->DecrementPeriodIfElapsed()) {
		float startDegrees = m_forwardDegrees - m_rangeDegrees * 0.5f;
		float stepDegrees = m_rangeDegrees / 6.f;
		for (int i = 0; i < 6; i++) {
			Vec2 forwardVec = Vec2::MakeFromPolarDegrees( startDegrees + (float)i * stepDegrees, m_radius );
			BacteriaSap* sap = (BacteriaSap*)g_theGame->SpawnEffectToGame( EffectType::BacteriaSap, m_startPos );
			sap->m_startPos = m_startPos + forwardVec;
			sap->m_endPos = m_startPos + forwardVec;
			sap->m_capsuleRadius = m_dist * 0.2f;
			sap->m_maxRadius = m_dist * 0.2f;
			sap->m_numOfDisc = 5;
			sap->m_lifeTime = 3.f;
			sap->m_color = Rgba8( 0, 153, 0 );
			sap->BeginPlay();
		}
	}

	for (auto entity : g_theGame->m_entityArray) {
		if (entity && entity->m_def.m_faction != m_faction && DoDiscAndSectorOverlap2D( entity->m_position, entity->m_physicsRadius, m_startPos, m_forwardDegrees, m_rangeDegrees, m_radius )) {
			entity->BeAttackedOnce( m_damage, Vec2(), 1.f, this );
			if (m_isPoisonous) {
				entity->m_poisonTimer += 0.1f;
				entity->m_isPoisoned = true;
			}
		}
	}
}

void SectorSprayAttack::Render() const
{
	return;
}

void SectorSprayAttack::Die()
{
	m_isDead = true;
}

SunFlame::SunFlame( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect( startPos, startOrientation, startVelocity )
{
	m_attackTimer = new Timer( 4.f, GetGameClock() );
	m_attackTimer->Start();
}

SunFlame::~SunFlame()
{
	delete m_attackTimer;
}

void SunFlame::BeginPlay()
{

}

void SunFlame::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}

	if (m_attackTimer->HasPeriodElapsed()) {
		Die();
	}

	m_radius = m_dist * SmoothStop2( m_attackTimer->GetElapsedFraction() );

	for (auto entity : g_theGame->m_entityArray) {
		if (entity && entity->m_def.m_faction != m_faction && 
			DoDiscsOverlap( entity->m_position, entity->m_physicsRadius, m_position, m_radius ) 
			&& g_theGame->m_curRoom->m_bounds.IsPointInside(entity->m_position)) {
			entity->BeAttackedOnce( m_damage, Vec2(), 10000.f, this );
		}
	}
}

void SunFlame::Render() const
{
	std::vector<Vertex_PCU> verts;
	Vec2 center = m_position;
	float radius = m_radius;
	float thickness = 2.f;
	Rgba8 color1( 153, 76, 0 );
	Rgba8 color2( 153, 76, 0, 100 );
	constexpr int NUM_SIDES = 16;
	constexpr int NUM_TRIANGLES = NUM_SIDES * 2;
	constexpr int NUM_VERTS = 3 * NUM_TRIANGLES;
	constexpr float DEGREES_PER_SIDE = 360.f / (float)NUM_SIDES;
	constexpr float RADIANS_PER_SIDE = DEGREES_PER_SIDE * PI / 180.f;
	verts.resize( NUM_VERTS );
	for (int i = 0; i < NUM_SIDES; i++) {
		verts[6 * i] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * radius, SinRadians( RADIANS_PER_SIDE * i ) * radius, 0 ), color2, Vec2( 0, 0 ) );
		verts[6 * i + 2] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, 0 ), color2, Vec2( 0, 0 ) );
		verts[6 * i + 1] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), 0 ), color1, Vec2( 0, 0 ) );
		verts[6 * i + 3] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, 0 ), color2, Vec2( 0, 0 ) );
		verts[6 * i + 4] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), 0 ), color1, Vec2( 0, 0 ) );
		verts[6 * i + 5] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness), 0 ), color1, Vec2( 0, 0 ) );
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void SunFlame::Die()
{
	m_isDead = true;
}

ElectricChain::ElectricChain( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect( startPos, startOrientation, startVelocity )
{
	m_lifeTimer = new Timer( 0.8f, GetGameClock() );
	m_lifeTimer->Start();
}

ElectricChain::~ElectricChain()
{
	delete m_lifeTimer;
}

void ElectricChain::BeginPlay()
{
	Entity* enemy = nullptr;
	Vec2 refPos = m_position;
	do {
		enemy = g_theGame->GetNearestEnemy( refPos, m_maxDist, enemy );
		if (enemy) {
			refPos = enemy->m_position;
			enemy->BeAttacked( m_damage, Vec2() );
			m_damageTargets.push_back( enemy );
		}
	} while (enemy != nullptr && (int)m_damageTargets.size() < m_maxTargets);
}

void ElectricChain::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_lifeTimer->HasPeriodElapsed()) {
		Die();
		return;
	}

	if (!m_owner->IsAlive()) {
		m_owner = nullptr;
	}
	if (m_owner) {
		m_position = m_owner->m_position;
	}

	for (int i = 0; i < (int)m_damageTargets.size(); i++) {
		if (!m_damageTargets[i]->IsAlive()) {
			m_damageTargets.erase( m_damageTargets.begin() + i );
			i--;
		}
	}
}

void ElectricChain::Render() const
{
	std::vector<Vertex_PCU> verts;
	if ((int)m_damageTargets.size() >= 1) {
		AddVertsForLineSegment2D( verts, m_position, m_damageTargets[0]->m_position, 0.6f, Rgba8( 255, 255, 255, 200 ) );
	}
	for (int i = 0; i < (int)m_damageTargets.size() - 1; i++) {
		AddVertsForLineSegment2D( verts, m_damageTargets[i]->m_position, m_damageTargets[i + 1]->m_position, 0.6f, Rgba8( 255, 255, 255, 200 ) );
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void ElectricChain::Die()
{
	m_isDead = true;
}

BacteriaLick::BacteriaLick( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_lifeTimer = new Timer( 0.5f, GetGameClock() );
	m_lifeTimer->Start();
	m_target = g_theGame->GetPlayerEntity();
}

BacteriaLick::~BacteriaLick()
{
	delete m_lifeTimer;
}

void BacteriaLick::BeginPlay()
{

}

void BacteriaLick::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_lifeTimer->HasPeriodElapsed()) {
		Die();
		return;
	}

	m_length = SmoothStop4( m_lifeTimer->GetElapsedFraction() ) * 20.f;
	Vec2 fwdVec = Vec2::MakeFromPolarDegrees( m_orientationDegrees );
	OBB2 boundingBox = OBB2( m_position + fwdVec * m_length * 0.5f, fwdVec, Vec2( m_length * 0.5f, 1.f ) );
	if (DoDiscAndOBB2Overlap2D( m_target->m_position, m_target->m_physicsRadius, boundingBox )) {
		m_target->BeAttackedOnce( 1.f, Vec2(), 1.f, this );
	}
}

void BacteriaLick::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, AABB2( Vec2( 0.f, -1.f ), Vec2( m_length, 1.f ) ), Rgba8( 102, 102, 0 ) );
	AddVertsForDisc2D( verts, Vec2( m_length, 0.f ), 1.2f, Rgba8( 204, 204, 0 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );

}

void BacteriaLick::Die()
{

	m_isDead = true;
}

MercyKillerHarmer::MercyKillerHarmer( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_target = g_theGame->GetPlayerEntity();
	m_length = (m_position - m_target->m_position).GetLength();
	m_targetOrientationDegrees = (m_target->m_position - m_position).GetOrientationDegrees();
	m_orientationDegrees = m_targetOrientationDegrees - 90.f;
	m_targetOrientationDegrees += 60.f;
	if (!g_theGame->m_curRoom->m_bounds.IsPointInside( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_length ) )) {
		m_targetOrientationDegrees -= 120.f;
		m_orientationDegrees = m_targetOrientationDegrees + 150.f;
	}
	m_startOrientationDegrees = m_orientationDegrees;
	m_lifeTimer = new Timer( 1.7f, GetGameClock() );
	m_lifeTimer->Start();
}

MercyKillerHarmer::~MercyKillerHarmer()
{
	delete m_lifeTimer;
}

void MercyKillerHarmer::BeginPlay()
{

}

void MercyKillerHarmer::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_lifeTimer->HasPeriodElapsed()) {
		Die();
		return;
	}

	float fraction = 0.f;
	if (m_lifeTimer->GetElapsedFraction() >= 0.2f) {
		fraction = SmoothStart3( RangeMapClamped( m_lifeTimer->GetElapsedFraction(), 0.2f, 1.f, 0.f, 1.f ) );
	}
	m_orientationDegrees = Interpolate( m_startOrientationDegrees, m_targetOrientationDegrees, fraction );

	Vec2 colliderIFwd = Vec2::MakeFromPolarDegrees( m_orientationDegrees );
	Vec2 colliderCenter = colliderIFwd * (m_length + 15.f) + m_position;
	Vec2 IJWidth = Vec2( 7.5f, 15.f );
	if (DoDiscAndOBB2Overlap2D( m_target->m_position, m_target->m_physicsRadius, OBB2( colliderCenter, colliderIFwd, IJWidth ) )) {
		m_target->BeAttackedOnce( 1.f, Vec2(), 3.f, this );
	}
}

void MercyKillerHarmer::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, AABB2( Vec2( m_length, -15.f ), Vec2( m_length + 15.f, 15.f ) ), Rgba8( 63, 62, 60 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( -6.f, -2.f ), Vec2( m_length + 4.f, 2.f ) ), Rgba8( 0, 0, 0 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void MercyKillerHarmer::Die()
{
	m_isDead = true;
}

MeteorShower::MeteorShower( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_renderBeforeEntity = true;
	m_lifeTimer = new Timer( 5.f, GetGameClock() );
	m_spawnMeteorTimer = new Timer( 0.04f, GetGameClock() );
	m_lifeTimer->Start();
	m_spawnMeteorTimer->Start();
}

MeteorShower::~MeteorShower()
{
	delete m_lifeTimer;
	delete m_spawnMeteorTimer;
}

void MeteorShower::BeginPlay()
{

}

void MeteorShower::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_lifeTimer->HasPeriodElapsed()) {
		Die();
		return;
	}

	if (m_lifeTimer->GetElapsedFraction() > 0.2f && m_lifeTimer->GetElapsedFraction() < 0.5f) {
		// spawn meteor
		if (m_spawnMeteorTimer->DecrementPeriodIfElapsed()) {
			g_theGame->SpawnEffectToGame( EffectType::Meteor, m_position, m_orientationDegrees + GetRandGen()->RollRandomFloatInRange( -0.5f * m_apertureDegrees, 0.5f * m_apertureDegrees ) );
		}
	}
}

void MeteorShower::Render() const
{
	std::vector<Vertex_PCU> verts;
	if (m_lifeTimer->GetElapsedFraction() < 0.3f) {
		float fraction = RangeMapClamped( m_lifeTimer->GetElapsedFraction(), 0.f, 0.3f, m_apertureDegrees * 0.2f, m_apertureDegrees );
		AddVertsForSector2D( verts, Vec2(), Vec2( 1.f, 0.f ), fraction, m_radius, Rgba8( 100, 100, 100, 100 ) );
	}
	else {
		AddVertsForSector2D( verts, Vec2(), Vec2( 1.f, 0.f ), m_apertureDegrees, m_radius, Rgba8( 200, 200, 200, 100 ) );
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void MeteorShower::Die()
{
	m_isDead = true;
}

Meteor::Meteor( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_lifeTimer = new Timer( 2.5f, GetGameClock() );
	m_lifeTimer->Start();
	m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, GetRandGen()->RollRandomFloatInRange( 65.f, 75.f ) );

	m_color = Rgba8( 60, 60, 60 );
	constexpr int NUM_OF_DEBRIS_VERTS = 48;
	float randR[NUM_OF_DEBRIS_VERTS / 3];
	for (int i = 0; i < NUM_OF_DEBRIS_VERTS / 3; i++) {
		randR[i] = g_engineRNG->RollRandomFloatInRange( 0.6f, 1.f );
	}
	for (int i = 0; i < NUM_OF_DEBRIS_VERTS / 3; i++) {
		Vertex_PCU vert1, vert2, vert3;
		vert1.m_position = Vec3( 0, 0, 0 );
		vert1.m_color = Rgba8::WHITE;
		vert1.m_uvTexCoords = Vec2( 0, 0 );
		vert2.m_position = Vec3( CosRadians( 6 * PI / NUM_OF_DEBRIS_VERTS * i ) * randR[i], SinRadians( 6 * PI / NUM_OF_DEBRIS_VERTS * i ) * randR[i], 0 );
		vert2.m_color = Rgba8::WHITE;
		vert2.m_uvTexCoords = Vec2( 0, 0 );
		vert3.m_position = Vec3( CosRadians( 6 * PI / NUM_OF_DEBRIS_VERTS * (i + 1) ) * randR[(i + 1) % (NUM_OF_DEBRIS_VERTS / 3)], SinRadians( 6 * PI / NUM_OF_DEBRIS_VERTS * (i + 1) ) * randR[(i + 1) % (NUM_OF_DEBRIS_VERTS / 3)], 0 );
		vert3.m_color = Rgba8::WHITE;
		vert3.m_uvTexCoords = Vec2( 0, 0 );
		m_verts.push_back( vert1 );
		m_verts.push_back( vert2 );
		m_verts.push_back( vert3 );
	}

	m_player = g_theGame->GetPlayerObject();
}

Meteor::~Meteor()
{
	delete m_lifeTimer;
}

void Meteor::BeginPlay()
{

}

void Meteor::Update( float deltaTime )
{
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_lifeTimer->HasPeriodElapsed()) {
		Die();
		return;
	}

	m_position += deltaTime * m_velocity;
	m_orientationDegrees += deltaTime * 360.f;
	if (DoDiscsOverlap( m_position, 0.4f, m_player->m_position, m_player->m_physicsRadius )) {
		m_player->BeAttackedOnce( 1.f, Vec2(), 3.f, this );
	}
}

void Meteor::Render() const
{
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( m_verts );
}

void Meteor::Die()
{
	m_isDead = true;
}

constexpr float edgeWidth = 2.f;
constexpr float maxEdgeDist = 6.f;

MercyKillerCage::MercyKillerCage( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_lifeTimer = new Timer( 8.f, GetGameClock() );
	m_lifeTimer->Start();
	m_chainTimer = new Timer( 0.6f, GetGameClock() );
	m_player = g_theGame->GetPlayerObject();
}

MercyKillerCage::~MercyKillerCage()
{
	delete m_lifeTimer;
	delete m_chainTimer;
}

void MercyKillerCage::BeginPlay()
{

}

void MercyKillerCage::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_lifeTimer->HasPeriodElapsed()) {
		Die();
		return;
	}

	float lifeFraction = m_lifeTimer->GetElapsedFraction();
	if (lifeFraction < 0.2f) {
		float thisFraction = RangeMapClamped( lifeFraction, 0.f, 0.2f, 1.f, 0.f );
		m_player->m_position.x = GetClamped( m_player->m_position.x, m_bounds.m_mins.x - maxEdgeDist * thisFraction + edgeWidth + m_player->m_physicsRadius, m_bounds.m_maxs.x + maxEdgeDist * thisFraction - edgeWidth - m_player->m_physicsRadius );
		m_player->m_position.y = GetClamped( m_player->m_position.y, m_bounds.m_mins.y - maxEdgeDist * thisFraction + edgeWidth + m_player->m_physicsRadius, m_bounds.m_maxs.y + maxEdgeDist * thisFraction - edgeWidth - m_player->m_physicsRadius );

	}
	else {
		m_player->m_position.x = GetClamped( m_player->m_position.x, m_bounds.m_mins.x + edgeWidth + m_player->m_physicsRadius, m_bounds.m_maxs.x - edgeWidth - m_player->m_physicsRadius );
		m_player->m_position.y = GetClamped( m_player->m_position.y, m_bounds.m_mins.y + edgeWidth + m_player->m_physicsRadius, m_bounds.m_maxs.y - edgeWidth - m_player->m_physicsRadius );
	}

	if (lifeFraction > 0.2f) {

		if (!m_hasStartChain) {
			m_chainTimer->Start();
			m_hasStartChain = true;
		}

		if (m_chainTimer->DecrementPeriodIfElapsed()) {
			Vec2 chainStartPosition;
			int direction = GetRandGen()->RollRandomIntInRange( 0, 3 );
			float height = 0.f;
			// go to east
			if (direction == 0) {
				chainStartPosition.x = m_bounds.m_mins.x - 40.f;
				height = 40.f;
				chainStartPosition.y = GetRandGen()->RollRandomFloatInRange( m_bounds.m_mins.y, m_bounds.m_maxs.y );
			}
			// go to west
			else if (direction == 1) {
				chainStartPosition.x = m_bounds.m_maxs.x + 40.f;
				height = 40.f;
				chainStartPosition.y = GetRandGen()->RollRandomFloatInRange( m_bounds.m_mins.y, m_bounds.m_maxs.y );
			}
			// go to north
			else if (direction == 2) {
				chainStartPosition.y = m_bounds.m_mins.y - 30.f;
				height = 30.f;
				chainStartPosition.x = GetRandGen()->RollRandomFloatInRange( m_bounds.m_mins.x, m_bounds.m_maxs.x );
			}
			// go to south
			else if (direction == 3) {
				chainStartPosition.y = m_bounds.m_maxs.y + 30.f;
				height = 30.f;
				chainStartPosition.x = GetRandGen()->RollRandomFloatInRange( m_bounds.m_mins.x, m_bounds.m_maxs.x );
			}
			MercyKillerCageChain* chain = (MercyKillerCageChain*)g_theGame->SpawnEffectToGame( EffectType::MercyKillerCageChain, chainStartPosition, 0.f, Vec2(), true );
			chain->m_width = 2.f;
			chain->m_height = height;
			chain->m_direction = direction;
		}
	}
}

void MercyKillerCage::Render() const
{
	std::vector<Vertex_PCU> verts;
	Rgba8 edgeColor = Rgba8( 50, 50, 50 );
	float lifeFraction = m_lifeTimer->GetElapsedFraction();

	float height = m_bounds.m_maxs.y - m_bounds.m_mins.y;
	// render locks
	if (lifeFraction > 0.2f) {
		// 9.5 10.5
		if (lifeFraction < 0.3f) {
			float thisFraction = RangeMapClamped( lifeFraction, 0.2f, 0.3f, 1.f, 0.f );
			AddVertsForAABB2D( verts, AABB2( Vec2( m_bounds.m_mins.x + 11.5f, m_bounds.m_mins.y + height * thisFraction ), Vec2( m_bounds.m_mins.x + 12.5f, m_bounds.m_maxs.y ) ), Rgba8( 30, 30, 30, 100 ) );
			AddVertsForAABB2D( verts, AABB2( Vec2( m_bounds.m_mins.x + 21.5f, m_bounds.m_mins.y + height * thisFraction ), Vec2( m_bounds.m_mins.x + 22.5f, m_bounds.m_maxs.y ) ), Rgba8( 30, 30, 30, 100 ) );
			AddVertsForAABB2D( verts, AABB2( Vec2( m_bounds.m_mins.x + 31.5f, m_bounds.m_mins.y + height * thisFraction ), Vec2( m_bounds.m_mins.x + 32.5f, m_bounds.m_maxs.y ) ), Rgba8( 30, 30, 30, 100 ) );
		}
		else {
			AddVertsForAABB2D( verts, AABB2( Vec2( m_bounds.m_mins.x + 11.5f, m_bounds.m_mins.y ), Vec2( m_bounds.m_mins.x + 12.5f, m_bounds.m_maxs.y ) ), Rgba8( 30, 30, 30, 100 ) );
			AddVertsForAABB2D( verts, AABB2( Vec2( m_bounds.m_mins.x + 21.5f, m_bounds.m_mins.y ), Vec2( m_bounds.m_mins.x + 22.5f, m_bounds.m_maxs.y ) ), Rgba8( 30, 30, 30, 100 ) );
			AddVertsForAABB2D( verts, AABB2( Vec2( m_bounds.m_mins.x + 31.5f, m_bounds.m_mins.y ), Vec2( m_bounds.m_mins.x + 32.5f, m_bounds.m_maxs.y ) ), Rgba8( 30, 30, 30, 100 ) );
		}
	}

	// render top down edge
	if (lifeFraction < 0.2f) {
		float thisFraction = RangeMapClamped( lifeFraction, 0.f, 0.2f, 1.f, 0.f );
		// top
		AddVertsForAABB2D( verts, AABB2( Vec2( m_bounds.m_mins.x, m_bounds.m_maxs.y + maxEdgeDist * thisFraction - edgeWidth ), Vec2( m_bounds.m_maxs.x, m_bounds.m_maxs.y + maxEdgeDist * thisFraction ) ), edgeColor );
		// down
		AddVertsForAABB2D( verts, AABB2( Vec2( m_bounds.m_mins.x, m_bounds.m_mins.y - maxEdgeDist * thisFraction ), Vec2( m_bounds.m_maxs.x, m_bounds.m_mins.y - maxEdgeDist * thisFraction + edgeWidth ) ), edgeColor );
	}
	else {
		AddVertsForAABB2D( verts, AABB2( Vec2( m_bounds.m_mins.x, m_bounds.m_maxs.y - edgeWidth ), Vec2( m_bounds.m_maxs.x, m_bounds.m_maxs.y ) ), edgeColor );
		AddVertsForAABB2D( verts, AABB2( Vec2( m_bounds.m_mins.x, m_bounds.m_mins.y ), Vec2( m_bounds.m_maxs.x, m_bounds.m_mins.y + edgeWidth ) ), edgeColor );
	}

	// render left right edge
	if (lifeFraction < 0.2f) {
		float thisFraction = RangeMapClamped( lifeFraction, 0.f, 0.2f, 1.f, 0.f );
		AddVertsForAABB2D( verts, AABB2( Vec2( m_bounds.m_mins.x - maxEdgeDist * thisFraction, m_bounds.m_mins.y ), Vec2( m_bounds.m_mins.x - maxEdgeDist * thisFraction + edgeWidth, m_bounds.m_maxs.y ) ), edgeColor );
		AddVertsForAABB2D( verts, AABB2( Vec2( m_bounds.m_maxs.x + maxEdgeDist * thisFraction - edgeWidth, m_bounds.m_mins.y ), Vec2( m_bounds.m_maxs.x + maxEdgeDist * thisFraction, m_bounds.m_maxs.y ) ), edgeColor );
	}
	else {
		AddVertsForAABB2D( verts, AABB2( Vec2( m_bounds.m_mins.x, m_bounds.m_mins.y ), Vec2( m_bounds.m_mins.x + edgeWidth, m_bounds.m_maxs.y ) ), edgeColor );
		AddVertsForAABB2D( verts, AABB2( Vec2( m_bounds.m_maxs.x - edgeWidth, m_bounds.m_mins.y ), Vec2( m_bounds.m_maxs.x, m_bounds.m_maxs.y ) ), edgeColor );
	}

	AABB2 const& roomBounds = g_theGame->m_curRoom->m_bounds;
	// hide player sight
	if (lifeFraction >= 0.2f) {
		AddVertsForAABB2D( verts, AABB2( Vec2( roomBounds.m_mins.x - 10.f, roomBounds.m_mins.y - 10.f ), Vec2( m_bounds.m_mins.x, roomBounds.m_maxs.y + 10.f ) ), Rgba8( 0, 0, 0 ) );
		AddVertsForAABB2D( verts, AABB2( Vec2( m_bounds.m_maxs.x, roomBounds.m_mins.y - 10.f ), Vec2( roomBounds.m_maxs.x + 10.f, roomBounds.m_maxs.y + 10.f ) ), Rgba8( 0, 0, 0 ) );
		AddVertsForAABB2D( verts, AABB2( Vec2( roomBounds.m_mins.x - 10.f, m_bounds.m_maxs.y ), Vec2( roomBounds.m_maxs.x + 10.f, roomBounds.m_maxs.y + 10.f ) ), Rgba8( 0, 0, 0 ) );
		AddVertsForAABB2D( verts, AABB2( Vec2( roomBounds.m_mins.x - 10.f, roomBounds.m_mins.y - 10.f ), Vec2( roomBounds.m_maxs.x + 10.f, m_bounds.m_mins.y ) ), Rgba8( 0, 0, 0 ) );
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), m_color);
	g_theRenderer->DrawVertexArray( verts );
}

void MercyKillerCage::Die()
{
	m_isDead = true;
}

MercyKillerCageChain::MercyKillerCageChain( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_startPos = m_position;
	m_player = g_theGame->GetPlayerObject();
	m_lifeTimer = new Timer( 1.f, GetGameClock() );
	m_lifeTimer->Start();
}

MercyKillerCageChain::~MercyKillerCageChain()
{
	delete m_lifeTimer;
}

void MercyKillerCageChain::BeginPlay()
{

}

void MercyKillerCageChain::Update( float deltaTime )
{
	UNUSED( deltaTime );
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_lifeTimer->HasPeriodElapsed()) {
		Die();
		return;
	}
	// go to east
	if (m_direction == 0) {
		m_position.x = Interpolate( m_startPos.x, m_startPos.x + m_height, SmoothStart6( m_lifeTimer->GetElapsedFraction() ) );
		AABB2 bounds( Vec2( m_position.x, m_position.y - m_width * 0.5f ), Vec2( m_position.x + m_height, m_position.y + m_width * 0.5f ) );
		if (DoDiscAndAABB2Overlap2D( m_player->m_position, m_player->m_physicsRadius, bounds )) {
			m_player->BeAttackedOnce( 1.f, Vec2(), 3.f, this );
		}
	}
	// go to west
	else if (m_direction == 1) {
		m_position.x = Interpolate( m_startPos.x, m_startPos.x - m_height, SmoothStart6( m_lifeTimer->GetElapsedFraction() ) );
		AABB2 bounds( Vec2( m_position.x - m_height, m_position.y - m_width * 0.5f ), Vec2( m_position.x, m_position.y + m_width * 0.5f ) );
		if (DoDiscAndAABB2Overlap2D( m_player->m_position, m_player->m_physicsRadius, bounds )) {
			m_player->BeAttackedOnce( 1.f, Vec2(), 3.f, this );
		}
	}
	// go to north
	else if (m_direction == 2) {
		m_position.y = Interpolate( m_startPos.y, m_startPos.y + m_height, SmoothStart6( m_lifeTimer->GetElapsedFraction() ) );
		AABB2 bounds( Vec2( m_position.x - m_width * 0.5f, m_position.y ), Vec2( m_position.x + m_width * 0.5f, m_position.y + m_height ) );
		if (DoDiscAndAABB2Overlap2D( m_player->m_position, m_player->m_physicsRadius, bounds )) {
			m_player->BeAttackedOnce( 1.f, Vec2(), 3.f, this );
		}
	}
	// go to south
	else if (m_direction == 3) {
		m_position.y = Interpolate( m_startPos.y, m_startPos.y - m_height, SmoothStart6( m_lifeTimer->GetElapsedFraction() ) );
		AABB2 bounds( Vec2( m_position.x - m_width * 0.5f, m_position.y - m_height ), Vec2( m_position.x + m_width * 0.5f, m_position.y ) );
		if (DoDiscAndAABB2Overlap2D( m_player->m_position, m_player->m_physicsRadius, bounds )) {
			m_player->BeAttackedOnce( 1.f, Vec2(), 3.f, this );
		}
	}
}

void MercyKillerCageChain::Render() const
{
	std::vector<Vertex_PCU> verts;
	// go to east
	if (m_direction == 0) {
		verts.emplace_back( Vec2( m_position.x, m_position.y + m_width * 0.5f ), Rgba8( 255, 0, 0 ) );
		verts.emplace_back( Vec2( m_position.x, m_position.y - m_width * 0.5f ), Rgba8( 255, 0, 0 ) );
		verts.emplace_back( Vec2( m_position.x + m_height, m_position.y ), Rgba8( 255, 0, 0 ) );
	}
	// go to west
	else if (m_direction == 1) {
		verts.emplace_back( Vec2( m_position.x, m_position.y + m_width * 0.5f ), Rgba8( 255, 0, 0 ) );
		verts.emplace_back( Vec2( m_position.x - m_height, m_position.y ), Rgba8( 255, 0, 0 ) );
		verts.emplace_back( Vec2( m_position.x, m_position.y - m_width * 0.5f ), Rgba8( 255, 0, 0 ) );

	}
	// go to north
	else if (m_direction == 2) {
		verts.emplace_back( Vec2( m_position.x - m_width * 0.5f, m_position.y ), Rgba8( 255, 0, 0 ) );
		verts.emplace_back( Vec2( m_position.x + m_width * 0.5f, m_position.y ), Rgba8( 255, 0, 0 ) );
		verts.emplace_back( Vec2( m_position.x, m_position.y + m_height ), Rgba8( 255, 0, 0 ) );
	}
	// go to south
	else if (m_direction == 3) {
		verts.emplace_back( Vec2( m_position.x - m_width * 0.5f, m_position.y ), Rgba8( 255, 0, 0 ) );
		verts.emplace_back( Vec2( m_position.x, m_position.y - m_height ), Rgba8( 255, 0, 0 ) );
		verts.emplace_back( Vec2( m_position.x + m_width * 0.5f, m_position.y ), Rgba8( 255, 0, 0 ) );
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void MercyKillerCageChain::Die()
{
	m_isDead = true;
}

MercyKillerRespawn::MercyKillerRespawn( Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:StarshipEffect(startPos, startOrientation, startVelocity)
{
	m_lifeTimer = new Timer( 3.f, GetGameClock() );
	m_lifeTimer->Start();
	m_particleTimer = new Timer( 0.04f, GetGameClock() );
	m_particleTimer->Start();
}

MercyKillerRespawn::~MercyKillerRespawn()
{
	delete m_lifeTimer;
	delete m_particleTimer;
}

void MercyKillerRespawn::BeginPlay()
{

}

void MercyKillerRespawn::Update( float deltaTime )
{
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_lifeTimer->HasPeriodElapsed()) {
		Die();
		return;
	}

	if (m_particleTimer->DecrementPeriodIfElapsed()) {
		for (int i = 0; i < 5; i++) {
			Vec2 pos = GetRandomPointOnUnitCircle2D() * 15.f + m_position;
			m_particlePos.push_back( pos );
			m_particleRadius.push_back( GetRandGen()->RollRandomFloatInRange( 0.6f, 1.2f ) );
		}
	}

	for (int i = 0; i < (int)m_particlePos.size(); i++) {
		if (IsPointInsideDisc2D( m_position, m_particlePos[i], m_particleRadius[i] * 0.5f )) {
			m_particlePos.erase( m_particlePos.begin() + i );
			m_particleRadius.erase( m_particleRadius.begin() + i );
			i--;
		}
		else {
			Vec2 disp = (m_position - m_particlePos[i]) * deltaTime * 2.f;
			m_particlePos[i] += disp;
		}
	}
}

void MercyKillerRespawn::Render() const
{
	std::vector<Vertex_PCU> verts;
	for (int i = 0; i < (int)m_particlePos.size(); i++) {
		AddVertsForDisc2D( verts, m_particlePos[i], m_particleRadius[i], Rgba8( 255, 153, 51 ) );
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void MercyKillerRespawn::Die()
{
	m_isDead = true;
}
