#include "Game/PlayerFaction.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Game.hpp"

PlayerAsteroid::PlayerAsteroid( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity(def, startPos, startOrientation, startVelocity)
{
	m_isInvincible = true;
}

PlayerAsteroid::~PlayerAsteroid()
{

}

void PlayerAsteroid::BeginPlay()
{
	m_disableFriction = true;
	//m_isInvincible = true;
	m_color = Rgba8( 100, 100, 100 );
	constexpr int NUM_OF_DEBRIS_VERTS = 48;
	float randR[NUM_OF_DEBRIS_VERTS / 3];
	for (int i = 0; i < NUM_OF_DEBRIS_VERTS / 3; i++) {
		randR[i] = g_engineRNG->RollRandomFloatInRange( 0.8f * m_cosmeticRadius, 1.f * m_cosmeticRadius );
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
}

void PlayerAsteroid::Update( float deltaTime )
{
	if (!m_isActivated) {
		return;
	}
	Entity::Update( deltaTime );
	m_oritationToOwner += deltaTime * 45.f;
	m_position = m_owner->m_position + Vec2::MakeFromPolarDegrees( m_oritationToOwner, m_owner->m_cosmeticRadius * 6.f );

	for (auto entity : g_theGame->m_entityArray) {
		if (entity && entity->m_def.m_faction != m_def.m_faction && DoDiscsOverlap( m_position, m_physicsRadius, entity->m_position, entity->m_physicsRadius )) {
			entity->BeAttackedOnce( 1.f, Vec2(), 0.04f, this );
		}
	}
}

void PlayerAsteroid::Render() const
{
	if (!m_isActivated) {
		return;
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( m_verts );
}

void PlayerAsteroid::Die()
{
	// m_isGarbage = true;
}

void PlayerAsteroid::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false */ )
{
	UNUSED( hit );
	UNUSED( hitNormal );
	UNUSED( directDamage );
}

BasicFollower::BasicFollower( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
{
	m_isInvincible = true;
}

BasicFollower::~BasicFollower()
{

}

void BasicFollower::BeginPlay()
{

}

void BasicFollower::Update( float deltaTime )
{
	if (!m_isActivated) {
		return;
	}
	Entity::Update( deltaTime );

	m_acclerateToPlayerDist = m_owner->m_cosmeticRadius * 5.f;
	m_idleDist = m_owner->m_cosmeticRadius * 4.f;
	//m_keepOutToPlayerDist = m_owner->m_cosmeticRadius * 3.f;

	float distSquared = GetDistanceSquared2D( m_position, m_owner->m_position );

	if (distSquared >= m_acclerateToPlayerDist * m_acclerateToPlayerDist) {
		float dist = sqrtf( distSquared );
		AddForce( 50.f * ((m_owner->m_position - m_owner->GetForwardNormal() * m_idleDist) - m_position) / dist * (dist - m_acclerateToPlayerDist) * (dist - m_acclerateToPlayerDist) );
	}
	else if (distSquared >= m_idleDist * m_idleDist) {
		// do nothing
	}
	else {
		float dist = sqrtf( distSquared );
		AddForce( 50.f * (m_position - (m_owner->m_position - m_owner->GetForwardNormal() * m_idleDist)) / (dist * dist * dist) );
	}
}

void BasicFollower::Render() const
{
	// do nothing
	return;
}

void BasicFollower::Die()
{
	// no death
}

void BasicFollower::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false */ )
{
	UNUSED( hit );
	UNUSED( hitNormal );
	UNUSED( directDamage );
}

DiagonalRetinue::DiagonalRetinue( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:BasicFollower( def, startPos, startOrientation, startVelocity )
{
	m_attackTimer = new Timer( m_def.m_shootCoolDown, GetGameClock() );
	m_attackTimer->Start();
	m_mass = 0.8f;
}

DiagonalRetinue::~DiagonalRetinue()
{
	delete m_attackTimer;
}

void DiagonalRetinue::BeginPlay()
{
	BasicFollower::BeginPlay();
}

void DiagonalRetinue::Update( float deltaTime )
{
	if (m_attackTimer->DecrementPeriodIfElapsed() && m_isActivated) {
		Vec2 fwdVec = GetForwardNormal();
		Vec2 fwd45 = fwdVec.GetRotatedDegrees( 45.f );
		Vec2 fwd135 = fwd45.GetRotated90Degrees();
		m_mainWeapon->Fire( fwd45, m_position );
		m_mainWeapon->Fire( -fwd45, m_position );
		m_mainWeapon->Fire( fwd135, m_position );
		m_mainWeapon->Fire( -fwd135, m_position );
	}

	if (!m_isActivated) {
		return;
	}

	BasicFollower::Update( deltaTime );
}

void DiagonalRetinue::Render() const
{
	if (!m_isActivated) {
		return;
	}
	std::vector<Vertex_PCU> m_verts;

	AddVertsForOBB2D( m_verts, OBB2( Vec2(), Vec2( 0.707f, 0.707f ), Vec2( m_cosmeticRadius, m_cosmeticRadius ) ), Rgba8( 51, 153, 205 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( m_verts );
}

void DiagonalRetinue::Die()
{

}

void DiagonalRetinue::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false */ )
{
	UNUSED( hit );
	UNUSED( hitNormal );
	UNUSED( directDamage );
}

LaserWingPlane::LaserWingPlane( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:BasicFollower( def, startPos, startOrientation, startVelocity )
{
	m_attackTimer = new Timer( 2.f, GetGameClock() );
	m_attackTimer->Start();
	m_mass = 3.f;
}

LaserWingPlane::~LaserWingPlane()
{
	delete m_attackTimer;
}

void LaserWingPlane::BeginPlay()
{
	BasicFollower::BeginPlay();
}

void LaserWingPlane::Update( float deltaTime )
{
	if (m_attackTimer->DecrementPeriodIfElapsed() && m_isActivated) {
		PersistentRay* ray = (PersistentRay*)g_theGame->SpawnEffectToGame( EffectType::PersistentRay, m_position + GetForwardNormal() * m_cosmeticRadius, m_orientationDegrees );
		ray->m_maxLength = 60.f;
		ray->m_maxWidth = m_cosmeticRadius * 1.5f;
		ray->m_minWidth = m_cosmeticRadius * 1.f;
		ray->m_owner = this;
		ray->m_lifeTimeSeconds = 1.f;
		ray->m_color = Rgba8( 0, 128, 255 );
		ray->m_damage = m_owner->GetMainWeaponDamage() * 0.5f;
		ray->m_damageCoolDown = 0.1f;
		ray->m_updatePositionByOwner = true;
		ray->BeginPlay();
	}

	m_orientationDegrees = m_owner->m_orientationDegrees;

	if (!m_isActivated) {
		return;
	}

	BasicFollower::Update( deltaTime );
}

void LaserWingPlane::Render() const
{
	if (!m_isActivated) {
		return;
	}
	std::vector<Vertex_PCU> m_verts;

	AddVertsForDisc2D( m_verts, Vec2(), m_cosmeticRadius, Rgba8( 51, 153, 205 ) );
	AddVertsForAABB2D( m_verts, AABB2( Vec2( 0.6f * m_cosmeticRadius, -0.4f * m_cosmeticRadius ), Vec2( 1.2f * m_cosmeticRadius, 0.4f * m_cosmeticRadius ) ), Rgba8( 0, 0, 255 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( m_verts );
}

void LaserWingPlane::Die()
{

}

void LaserWingPlane::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false */ )
{
	UNUSED( hit );
	UNUSED( hitNormal );
	UNUSED( directDamage );
}

WingPlane::WingPlane( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:BasicFollower( def, startPos, startOrientation, startVelocity )
{
	m_attackTimer = new Timer( m_def.m_shootCoolDown, GetGameClock() );
	m_attackTimer->Start();
	m_bulletTimer = new Timer( 0.1f, GetGameClock() );
	m_shootTimer = new Timer( 2.f, GetGameClock() );
	m_mass = 2.f;
}

WingPlane::~WingPlane()
{
	delete m_bulletTimer;
	delete m_attackTimer;
	delete m_shootTimer;
}

void WingPlane::BeginPlay()
{
	BasicFollower::BeginPlay();
}

void WingPlane::Update( float deltaTime )
{
	if (m_attackTimer->DecrementPeriodIfElapsed() && m_isActivated) {
		m_shootTimer->Start();
		m_bulletTimer->Start();
	}

	if (m_shootTimer->HasStartedAndNotPeriodElapsed() && m_isActivated) {
		if (m_bulletTimer->DecrementPeriodIfElapsed()) {
			Vec2 shootPos = GetRandomPointInDisc2D( m_cosmeticRadius * 0.5f, m_position );
			Vec2 fwd = GetForwardNormal().GetRotatedDegrees( GetRandGen()->RollRandomFloatInRange( -10.f, 10.f ) );
			m_mainWeapon->Fire( fwd, shootPos );
		}
	}

	m_orientationDegrees = m_owner->m_orientationDegrees;

	if (!m_isActivated) {
		return;
	}

	BasicFollower::Update( deltaTime );
}

void WingPlane::Render() const
{
	if (!m_isActivated) {
		return;
	}
	std::vector<Vertex_PCU> m_verts;

	AddVertsForAABB2D( m_verts, AABB2( Vec2( -m_cosmeticRadius, -m_cosmeticRadius ), Vec2( m_cosmeticRadius, m_cosmeticRadius ) ), Rgba8( 51, 153, 205 ) );
	AddVertsForAABB2D( m_verts, AABB2( Vec2( 0.6f * m_cosmeticRadius, -0.4f * m_cosmeticRadius ), Vec2( 1.2f * m_cosmeticRadius, 0.4f * m_cosmeticRadius ) ), Rgba8( 0, 0, 255 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( m_verts );
}

void WingPlane::Die()
{

}

void WingPlane::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false */ )
{
	UNUSED( hit );
	UNUSED( hitNormal );
	UNUSED( directDamage );
}

float WingPlane::GetMainWeaponDamage() const
{
	return m_owner->GetMainWeaponDamage();
}
