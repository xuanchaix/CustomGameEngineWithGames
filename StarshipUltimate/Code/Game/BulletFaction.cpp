#include "Game/BulletFaction.hpp"
#include "Game/Game.hpp"
#include "Game/Room.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/App.hpp"

constexpr float TARGET_RANGE_TO_KEEP = 40.f;
constexpr float TARGET_RELATIVE_ORIENTATION = 30.f;
constexpr float NORMAL_SHOOT_IMPULSE = 20.f;
constexpr float NORMAL_HIT_IMPULSE = 30.f;
constexpr float GUN_RELATIVE_DEGREES = -45.f;

GunShooter::GunShooter( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity(def, startPos, startOrientation, startVelocity)
	,m_projDef(ProjectileDefinition::GetDefinition("EnemyBullet"))
{
	m_target = g_theGame->GetPlayerEntity();
	m_gunOrientationDegrees = m_gunOrientationDegrees;
	m_deathTimer = new Timer( 2.f, m_clock );
	m_targetOrientation = (m_position - m_target->m_position).GetOrientationDegrees();
	m_targetRelativeOrientation = GetRandGen()->RollRandomFloatInRange( -TARGET_RELATIVE_ORIENTATION, TARGET_RELATIVE_ORIENTATION );
	m_targetOrientation += m_targetRelativeOrientation;
}

GunShooter::~GunShooter()
{
	delete m_stateTimer;
	delete m_deathTimer;
}

void GunShooter::BeginPlay()
{

}

void GunShooter::Update( float deltaTime )
{
	if (m_isDead && (m_deathTimer->HasPeriodElapsed() || m_isShadowed)) {
		m_isGarbage = true;
		return;
	}
	else if (m_isDead) {
		m_clock->SetTimeScale( 1.f );
		AddForce( m_velocity * m_mass * 6.3f );
		UpdatePhysics( deltaTime );
		return;
	}
	Entity::Update( deltaTime );

	Vec2 forwardVector = GetForwardNormal();

	Vec2 targetPosition = m_target->m_position + Vec2::MakeFromPolarDegrees( m_targetOrientation, TARGET_RANGE_TO_KEEP );
	Vec2 displacement = targetPosition - m_position;
	m_gunOrientationDegrees = GetTurnedTowardDegrees( m_gunOrientationDegrees, (m_target->m_position - m_position).GetOrientationDegrees(), 60.f * deltaTime );
	if (GetDistanceSquared2D( m_position, targetPosition ) >= 25.f) {
		AddForce( forwardVector * m_def.m_flySpeed );
	}
	m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, (targetPosition - m_position).GetOrientationDegrees(), m_def.m_turnSpeed * deltaTime );
	if (m_mainWeaponTimer->DecrementPeriodIfElapsed()) {
		GunFire();
	}
}

void GunShooter::Render() const
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> gunVerts;
	verts.reserve( 100 );
	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8(255, 153, 51) );
	AddVertsForOBB2D( gunVerts, OBB2( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ), Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ), Vec2( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 0.2f ) ), Rgba8( 128, 128, 128 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.8f, -m_cosmeticRadius * 0.4f ), Vec2( m_cosmeticRadius * 1.1f, m_cosmeticRadius * 0.4f ) ), Rgba8(255, 102, 102) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), m_color );
	g_theRenderer->DrawVertexArray( gunVerts );
}

void GunShooter::Die()
{
	g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/EnemyDie.wav" ), false, g_theApp->m_soundVolume * 0.1f );
	m_isDead = true;
	m_isInvincible = true;
	m_restrictIntoRoom = false;
	m_deathTimer->Start();
	m_color = Rgba8( 128, 128, 128, 128 );
	if (m_hasReward) {
		SpawnReward();
	}

}

void GunShooter::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false */, Vec2 const& projectileVelocity )
{
	if (m_isDead) {
		return;
	}
	UNUSED( directDamage );
	Vec2 forwardNormal = GetForwardNormal();
	if (hit > 0.f && m_def.m_isShielded && hitNormal != Vec2( 0.f, 0.f ) && DotProduct2D( hitNormal, forwardNormal ) > 0.1f) {
		StarshipEffect* shieldEffect = g_theGame->SpawnEffectToGame( EffectType::Shield, m_position );
		((StarshipShield*)shieldEffect)->m_owner = this;
		shieldEffect->BeginPlay();
		return;
	}
	else {
		m_health -= hit;
		g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/EnemyHit.wav" ), false, g_theApp->m_soundVolume * 0.1f );
		AddImpulse( Starship_GetStrongestPart( projectileVelocity ) * NORMAL_HIT_IMPULSE );
	}
	if (m_health > m_maxHealth) {
		m_health = m_maxHealth;
	}
	if (m_health <= 0.f) {
		Die();
		AddImpulse( Starship_GetStrongestPart( projectileVelocity ) * 60.f );
	}
}

void GunShooter::GunFire()
{
	Vec2 shootForward = Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees );
	m_mainWeapon->Fire( shootForward, m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ) );
	AddImpulse( -shootForward * NORMAL_SHOOT_IMPULSE );
}

MachineGunShooter::MachineGunShooter( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:GunShooter(def, startPos, startOrientation, startVelocity)
{
	m_machineGunCoolDownTimer = new Timer( 4.f, m_clock );
	m_machineGunCoolDownTimer->Start();
	m_machineGunCoolDownTimer->SetElapsedTime( GetRandGen()->RollRandomFloatInRange( 0.f, 2.f ) );
}

MachineGunShooter::~MachineGunShooter()
{
	delete m_machineGunCoolDownTimer;
}

void MachineGunShooter::BeginPlay()
{
	GunShooter::BeginPlay();
}

void MachineGunShooter::Update( float deltaTime )
{
	GunShooter::Update( deltaTime );
	if (m_machineGunCoolDownTimer->DecrementPeriodIfElapsed()) {
		m_isFiring = !m_isFiring;
	}
}

void MachineGunShooter::Render() const
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> gunVerts;
	verts.reserve( 100 );
	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 255, 153, 51 ) );
	if (m_def.m_isShielded) {
		AddVertsForDisc2D( verts, Vec2(), 1.f, Rgba8( 102, 178, 255, 255 ) );
	}
	AddVertsForOBB2D( gunVerts, OBB2( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ), Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ), Vec2( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 0.2f ) ), Rgba8( 128, 128, 128 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.8f, -m_cosmeticRadius * 0.4f ), Vec2( m_cosmeticRadius * 1.1f, m_cosmeticRadius * 0.4f ) ), Rgba8( 255, 255, 0 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), m_color );
	g_theRenderer->DrawVertexArray( gunVerts );
}

void MachineGunShooter::Die()
{
	GunShooter::Die();
}

void MachineGunShooter::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false*/, Vec2 const& projectileVelocity /*= Vec2() */ )
{
	GunShooter::BeAttacked( hit, hitNormal, directDamage, projectileVelocity );
}

void MachineGunShooter::GunFire()
{
	if (m_isFiring) {
		Vec2 shootForward = Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees + GetRandGen()->RollRandomFloatInRange( -30.f, 30.f ) );
		m_mainWeapon->Fire( shootForward, m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ) );
		AddImpulse( -shootForward * NORMAL_SHOOT_IMPULSE );
	}
}

SectorGunShooter::SectorGunShooter( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:GunShooter(def, startPos, startOrientation, startVelocity)
{

}

SectorGunShooter::~SectorGunShooter()
{

}

void SectorGunShooter::BeginPlay()
{
	GunShooter::BeginPlay();
}

void SectorGunShooter::Update( float deltaTime )
{
	GunShooter::Update( deltaTime );
}

void SectorGunShooter::Render() const
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> gunVerts;
	verts.reserve( 100 );
	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 255, 153, 51 ) );
	AddVertsForOBB2D( gunVerts, OBB2( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ), Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ), Vec2( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 0.2f ) ), Rgba8( 128, 128, 128 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.8f, -m_cosmeticRadius * 0.4f ), Vec2( m_cosmeticRadius * 1.1f, m_cosmeticRadius * 0.4f ) ), Rgba8( 153, 255, 51 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), m_color );
	g_theRenderer->DrawVertexArray( gunVerts );
}

void SectorGunShooter::Die()
{
	GunShooter::Die();
}

void SectorGunShooter::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false*/, Vec2 const& projectileVelocity /*= Vec2() */ )
{
	GunShooter::BeAttacked( hit, hitNormal, directDamage, projectileVelocity );
}

void SectorGunShooter::GunFire()
{
	constexpr float startOrientation = -30.f;
	constexpr float stepOrientation = (-2.f * startOrientation) / (float)(7 - 1);
	for (int i = 0; i < 7; i++) {
		Vec2 shootForward = Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees + startOrientation + i * stepOrientation );
		m_mainWeapon->Fire( shootForward, m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ) );
	}
	AddImpulse( -Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ) * NORMAL_SHOOT_IMPULSE * 3.f );
}

ShotGunShooter::ShotGunShooter( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:GunShooter(def, startPos, startOrientation, startVelocity)
{
	m_stateTimer = new Timer( m_def.m_shootCoolDown, m_clock );
	//m_stateTimer->SetElapsedTime( g_theGame->m_randNumGen->RollRandomFloatInRange( 0.5f, 1.f ) );
	m_stateTimer->Start();
}

ShotGunShooter::~ShotGunShooter()
{
	//delete m_stateTimer;
}

void ShotGunShooter::BeginPlay()
{
	GunShooter::BeginPlay();
}

void ShotGunShooter::Update( float deltaTime )
{
	if (m_isDead && m_deathTimer->HasPeriodElapsed()) {
		m_isGarbage = true;
		return;
	}
	else if (m_isDead) {
		m_clock->SetTimeScale( 1.f );
		AddForce( m_velocity * m_mass * 6.3f );
		UpdatePhysics( deltaTime );
		return;
	}
	Entity::Update( deltaTime );

	m_gunOrientationDegrees = GetTurnedTowardDegrees( m_gunOrientationDegrees, (m_target->m_position - m_position).GetOrientationDegrees(), 60.f * deltaTime );

	if (m_stateTimer->DecrementPeriodIfElapsed()) {
		if (m_state == 0) {
			// Dash
			m_state = 1;
			AddImpulse( GetForwardNormal() * 200.f );
			m_stateTimer->SetPeriodSeconds( 0.25f );
			m_stateTimer->Start();
		}
		else {
			// Turn to player
			m_state = 0;
			m_stateTimer->SetPeriodSeconds( m_def.m_shootCoolDown );
			m_stateTimer->Start();
			GunFire();
		}
	}

	if (m_state == 0) {
		float targetDegrees = (m_target->m_position - m_position).GetOrientationDegrees();
		SetOrientationDegrees( GetTurnedTowardDegrees( m_orientationDegrees, targetDegrees, m_def.m_turnSpeed * deltaTime ) );
	}
	else if (m_state == 1) {
		AddForce( GetForwardNormal() * m_def.m_flySpeed );
	}
}

void ShotGunShooter::Render() const
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> gunVerts;
	verts.reserve( 100 );
	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 255, 153, 51 ) );
	AddVertsForOBB2D( gunVerts, OBB2( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ), Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ), Vec2( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 0.2f ) ), Rgba8( 128, 128, 128 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.8f, -m_cosmeticRadius * 0.4f ), Vec2( m_cosmeticRadius * 1.1f, m_cosmeticRadius * 0.4f ) ), Rgba8( 255, 102, 255 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), m_color );
	g_theRenderer->DrawVertexArray( gunVerts );
}

void ShotGunShooter::Die()
{
	GunShooter::Die();
}

void ShotGunShooter::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false*/, Vec2 const& projectileVelocity /*= Vec2() */ )
{
	GunShooter::BeAttacked( hit, hitNormal, directDamage, projectileVelocity );
}

void ShotGunShooter::GunFire()
{
	constexpr float startOrientation = -30.f;
	constexpr float stepOrientation = (-2.f * startOrientation) / (float)(7 - 1);
	for (int i = 0; i < 7; i++) {
		Vec2 shootForward = Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees + i * stepOrientation );
		Projectile* proj = m_mainWeapon->Fire( shootForward, m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ) );
		proj->m_lifeTimer->SetPeriodSeconds( 0.26f );
		proj->m_velocity *= 3.5f;
	}
	AddImpulse( -Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ) * NORMAL_SHOOT_IMPULSE * 2.f );
}

Sniper::Sniper( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:GunShooter(def, startPos, startOrientation, startVelocity)
{
	m_rayColor = Rgba8( 255, 0, 0, 0 );
	m_redTimer = new Timer( 0.04f, m_clock );

}

Sniper::~Sniper()
{

}

void Sniper::BeginPlay()
{
	GunShooter::BeginPlay();
}

void Sniper::Update( float deltaTime )
{
	GunShooter::Update( deltaTime );

	if (m_isDead) {
		return;
	}

	if (m_mainWeaponTimer->GetElapsedFraction() > 0.4f) {
		float smoothStart = SmoothStart2( RangeMapClamped( m_mainWeaponTimer->GetElapsedFraction(), 0.4f, 1.f, 0.f, 1.f ) );
		if (m_redTimer->IsStopped()) {
			if (smoothStart > m_count) {
				m_count += m_step;
				m_redTimer->Start();
				m_rayColor = Rgba8( 255, 0, 0 );
			}
		}

		if (m_redTimer->HasPeriodElapsed()) {
			m_redTimer->Stop();
			m_rayColor = Rgba8( 255, 0, 0, 0 );
		}
	}
	else {
		m_count = 0.f;
		m_redTimer->Stop();
		m_rayColor = Rgba8( 255, 0, 0, 0 );
	}
}

void Sniper::Render() const
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> gunVerts;
	verts.reserve( 100 );
	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 255, 153, 51 ) );
	Vec2 gunPos = m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius );
	Vec2 gunForward = Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees );
	AddVertsForLineSegment2D( gunVerts, gunPos, gunPos + 300.f * gunForward, 0.2f, m_rayColor );
	AddVertsForOBB2D( gunVerts, OBB2( gunPos, gunForward, Vec2( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 0.2f ) ), Rgba8( 128, 128, 128 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.8f, -m_cosmeticRadius * 0.4f ), Vec2( m_cosmeticRadius * 1.1f, m_cosmeticRadius * 0.4f ) ), Rgba8( 64, 64, 64 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), m_color );
	g_theRenderer->DrawVertexArray( gunVerts );
}

void Sniper::Die()
{
	GunShooter::Die();
}

void Sniper::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false*/, Vec2 const& projectileVelocity /*= Vec2() */ )
{
	GunShooter::BeAttacked( hit, hitNormal, directDamage, projectileVelocity );
}

void Sniper::GunFire()
{
	Vec2 shootForward = Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees );
	Projectile* proj = m_mainWeapon->Fire( shootForward, m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ) );
	proj->m_velocity *= 5.f;
	AddImpulse( -shootForward * NORMAL_SHOOT_IMPULSE );
}

MissileShooter::MissileShooter( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:GunShooter(def, startPos, startOrientation, startVelocity)
{
	m_stateTimer = new Timer( 6.f, m_clock );
	m_stateTimer->Start();
}

MissileShooter::~MissileShooter()
{

}

void MissileShooter::BeginPlay()
{
	GunShooter::BeginPlay();
}

void MissileShooter::Update( float deltaTime )
{
	if (m_isDead && m_deathTimer->HasPeriodElapsed()) {
		m_isGarbage = true;
		return;
	}
	else if (m_isDead) {
		m_clock->SetTimeScale( 1.f );
		AddForce( m_velocity * m_mass * 6.3f );
		UpdatePhysics( deltaTime );
		return;
	}

	Entity::Update( deltaTime );

	if (m_stateTimer->DecrementPeriodIfElapsed()) {
		if (m_state == 0) {
			// to shoot state
			m_state = 1;
			m_stateTimer->SetPeriodSeconds( 4.f );
			m_stateTimer->Start();
			m_mainWeaponTimer->Start();
		}
		else {
			// to wonder state
			m_state = 0;
			m_stateTimer->SetPeriodSeconds( 4.f );
			m_stateTimer->Start();
		}
	}


	if (m_state == 0) {
		Vec2 forwardVector = GetForwardNormal();

		Vec2 targetPosition = m_target->m_position + Vec2::MakeFromPolarDegrees( m_targetOrientation, TARGET_RANGE_TO_KEEP * 1.5f );
		Vec2 displacement = targetPosition - m_position;
		// m_gunOrientationDegrees = GetTurnedTowardDegrees( m_gunOrientationDegrees, (m_target->m_position - m_position).GetOrientationDegrees(), 75.f * deltaTime );
		if (GetDistanceSquared2D( m_position, targetPosition ) >= 25.f) {
			AddForce( forwardVector * m_def.m_flySpeed );
		}
		m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, (targetPosition - m_position).GetOrientationDegrees(), m_def.m_turnSpeed * deltaTime );
	}
	else if (m_state == 1) {
		if (m_mainWeaponTimer->DecrementPeriodIfElapsed()) {
			GunFire();
		}
	}

}

void MissileShooter::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 255, 153, 51 ) );
	//AddVertsForOBB2D( gunVerts, OBB2( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ), Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ), Vec2( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 0.2f ) ), Rgba8( 128, 128, 128 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.8f, -m_cosmeticRadius * 0.4f ), Vec2( m_cosmeticRadius * 1.1f, m_cosmeticRadius * 0.4f ) ), Rgba8( 0, 153, 153 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void MissileShooter::Die()
{
	GunShooter::Die();
}

void MissileShooter::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false*/, Vec2 const& projectileVelocity /*= Vec2() */ )
{
	GunShooter::BeAttacked( hit, hitNormal, directDamage, projectileVelocity );
}

void MissileShooter::GunFire()
{
	Vec2 forward( 0.f, 0.f );
	CurveMissile* proj = (CurveMissile*)m_mainWeapon->Fire( forward, m_position );
	proj->m_goUp = m_goUp;
	proj->m_targetPosition = GetRandomPointInDisc2D( 20.f, m_target->m_position );
	proj->BeginPlay();
	m_goUp = !m_goUp;
}

GunTwinElder::GunTwinElder( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:GunShooter(def, startPos, startOrientation, startVelocity)
	,m_missileDef(ProjectileDefinition::GetDefinition("CurveMissile"))
{
	m_selfClock = new Clock( *m_clock );
	m_state = 0;
	m_stateTimer = new Timer( m_state0Time, m_selfClock );
	m_stateTimer->Start();
	m_brother = (GunTwinYoung*)g_theGame->SpawnEntityToGame( EntityDefinition::GetDefinition( "GunTwinYoung" ), m_position - Vec2( 50.f, 0.f ), m_orientationDegrees );
	m_brother->m_brother = this;
	m_grenadeTimer = new Timer( 0.8f, m_selfClock );
}

GunTwinElder::~GunTwinElder()
{
	delete m_selfClock;
	delete m_grenadeTimer;
}

void GunTwinElder::BeginPlay()
{
	GunShooter::BeginPlay();
}

void GunTwinElder::Update( float deltaTime )
{
	if (m_isDead && m_deathTimer->HasPeriodElapsed()) {
		m_isGarbage = true;
		return;
	}
	else if (m_isDead) {
		m_clock->SetTimeScale( 1.f );
		AddForce( m_velocity * m_mass * 6.3f );
		UpdatePhysics( deltaTime );
		return;
	}

	if (m_brother && m_brother->m_isDead) {
		m_brother = nullptr;
		m_isAngry = true;
		m_curStateIndex = 1;
		m_health += 20.f;
		m_selfClock->SetTimeScale( m_selfClock->GetTimeScale() * 1.2f );
	}

	Entity::Update( deltaTime );

	if (m_stateTimer->HasPeriodElapsed()) {
		GoToNextState();
	}

	if (m_state == 0 || m_state == 1) {
		Vec2 forwardVector = GetForwardNormal();

		Vec2 targetPosition = m_target->m_position + Vec2::MakeFromPolarDegrees( m_targetOrientation, TARGET_RANGE_TO_KEEP );
		Vec2 displacement = targetPosition - m_position;
		m_gunOrientationDegrees = GetTurnedTowardDegrees( m_gunOrientationDegrees, (m_target->m_position - m_position).GetOrientationDegrees(), 60.f * deltaTime );
		if (GetDistanceSquared2D( m_position, targetPosition ) >= 25.f) {
			AddForce( forwardVector * m_def.m_flySpeed );
		}
		m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, (targetPosition - m_position).GetOrientationDegrees(), m_def.m_turnSpeed * deltaTime );
	}
	else if (m_state == 2) {
		AddForce( GetForwardNormal() * 200.f );
		AddForce( m_velocity * m_mass * 3.4f );
	}
	else if (m_state == 3) {
		AddForce( Vec2( 0.f, -120.f ) );
	}
	else if (m_state == 4) {
		if (m_grenadeTimer->DecrementPeriodIfElapsed()) {
			Vec2 position = GetRandomPointInDisc2D( 25.f, m_target->m_position );
			Missle* missile = (Missle*)g_theGame->SpawnEffectToGame( EffectType::Missile, m_position );
			missile->m_startPos = m_position;
			missile->m_targetPos = position;
			missile->m_owner = this;
			missile->m_flyTime = 1.3f;
			missile->BeginPlay();
		}
	}

	if (m_mainWeaponTimer->DecrementPeriodIfElapsed()) {
		GunFire();
	}
}

void GunTwinElder::Render() const
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> gunVerts;
	verts.reserve( 100 );
	if (m_isAngry) {
		AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 255, 0, 0 ) );
	}
	else {
		AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 255, 153, 51 ) );
	}
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 0.3f ), Vec2( m_cosmeticRadius * 0.75f, m_cosmeticRadius * 0.7f ) ), Rgba8( 0, 0, 0 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.6f, -m_cosmeticRadius * 0.7f ), Vec2( m_cosmeticRadius * 0.75f, -m_cosmeticRadius * 0.3f ) ), Rgba8( 0, 0, 0 ) );
	AddVertsForOBB2D( gunVerts, OBB2( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ), Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ), Vec2( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 0.2f ) ), Rgba8( 128, 128, 128 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.8f, -m_cosmeticRadius * 0.4f ), Vec2( m_cosmeticRadius * 1.1f, m_cosmeticRadius * 0.4f ) ), Rgba8( 0, 0, 255 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), m_color );
	g_theRenderer->DrawVertexArray( gunVerts );
}

void GunTwinElder::Die()
{
	GunShooter::Die();
}

void GunTwinElder::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false*/, Vec2 const& projectileVelocity /*= Vec2() */ )
{
	if (m_isDead) {
		return;
	}
	UNUSED( hitNormal );
	UNUSED( directDamage );
	UNUSED( projectileVelocity );
	m_health -= hit;
	g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/EnemyHit.wav" ), false, g_theApp->m_soundVolume * 0.1f );
	if (m_health > m_maxHealth) {
		m_health = m_maxHealth;
	}
	if (m_health <= 0.f) {
		Die();
		AddImpulse( Starship_GetStrongestPart( projectileVelocity ) * 60.f );
	}
}

void GunTwinElder::GunFire()
{
	if (m_state == 1) {
		Vec2 shootForward = Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees + GetRandGen()->RollRandomFloatInRange( -30.f, 30.f ) );
		m_mainWeapon->Fire( shootForward, m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ) );
		// AddImpulse( -shootForward * NORMAL_SHOOT_IMPULSE );
	}
	else if (m_state == 3) {
		CurveMissile* proj = (CurveMissile*)g_theGame->SpawnProjectileToGame( m_missileDef, m_position, 0.f );
		proj->m_faction = m_def.m_faction;
		proj->m_damage = GetMainWeaponDamage();
		proj->m_goUp = m_goUp;
		proj->m_targetPosition = GetRandomPointInDisc2D( 20.f, m_target->m_position );
		proj->BeginPlay();
		m_goUp = !m_goUp;
	}
}

void GunTwinElder::GoToNextState()
{
	if (m_state == 2) {
		constexpr float startOrientation = -30.f;
		constexpr float stepOrientation = (-2.f * startOrientation) / (float)(7 - 1);
		for (int i = 0; i < 7; i++) {
			Vec2 shootForward = Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees + startOrientation + i * stepOrientation );
			m_mainWeapon->Fire( shootForward, m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ) );
		}
		AddImpulse( -Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ) * NORMAL_SHOOT_IMPULSE * 2.f );
	}
	else if (m_state == 3) {
		g_theGame->DealRangeDamage( 1.f, m_position, m_physicsRadius * 1.5f );
	}

	if (m_isAngry) {
		m_curStateIndex = (m_curStateIndex + 1) % m_angryStateCount;
		m_state = m_stateSequenceAngry[m_curStateIndex];
	}
	else {
		m_curStateIndex = (m_curStateIndex + 1) % m_normalStateCount;
		m_state = m_stateSequenceNormal[m_curStateIndex];
	}

	if (m_state == 0) {
		m_targetOrientation = (m_position - m_target->m_position).GetOrientationDegrees();
		m_targetRelativeOrientation = GetRandGen()->RollRandomFloatInRange( -TARGET_RELATIVE_ORIENTATION, TARGET_RELATIVE_ORIENTATION );
		m_targetOrientation += m_targetRelativeOrientation;
		m_stateTimer->SetPeriodSeconds( m_state0Time );
		m_disableFriction = false;
		m_isInvincible = false;
		m_restrictIntoRoom = true;
	}
	else if (m_state == 1) {
		m_stateTimer->SetPeriodSeconds( m_state1Time );
		m_disableFriction = false;
		m_isInvincible = false;
		m_restrictIntoRoom = true;
	}
	else if (m_state == 2) {
		m_stateTimer->SetPeriodSeconds( m_state2Time );
		AddImpulse( GetForwardNormal() * m_def.m_flySpeed * 2.f );
		m_disableFriction = false;
		m_isInvincible = false;
		m_restrictIntoRoom = true;
	}
	else if (m_state == 3) {
		m_stateTimer->SetPeriodSeconds( m_state3Time );
		do {
			m_jumpTargetPos = GetRandomPointInAABB2D( g_theGame->m_curRoom->m_bounds );
		} while (abs( m_jumpTargetPos.x - m_position.x ) < 60.f);
		m_disableFriction = true;
		m_isInvincible = true;
		m_restrictIntoRoom = false;
		float vx = (m_jumpTargetPos.x - m_position.x) / m_state3Time;
		float vy = (m_jumpTargetPos.y - m_position.y) / m_state3Time + 60.f * m_state3Time;
		AddImpulse( Vec2( vx, vy ) );
	}
	else if (m_state == 4) {
		m_grenadeTimer->Start();
		m_stateTimer->SetPeriodSeconds( m_state4Time );
		m_disableFriction = false;
		m_isInvincible = false;
		m_restrictIntoRoom = true;
	}
	m_stateTimer->Start();
}

void GunTwinElder::RenderUI() const
{
	float remainHealthRatio = (float)m_health / (float)m_maxHealth;
	remainHealthRatio = GetClamped( remainHealthRatio, 0.f, 1.f );
	Rgba8 healthColor = Rgba8( 255, 215, 0 );
	Rgba8 maxHealthColor = Rgba8( 255, 0, 0 );
	float startX = 50.f;
	float endX = 550.f;
	float Y = 30.f;
	float radius = 8.f;
	std::vector<Vertex_PCU> verts;
	AddVertsForCapsule2D( verts, Vec2( startX, Y ), Vec2( endX, Y ), radius, maxHealthColor );
	AddVertsForCapsule2D( verts, Vec2( endX - (endX - startX) * remainHealthRatio, Y ), Vec2( endX, Y ), radius, healthColor );

	std::vector<Vertex_PCU> textVerts;
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( startX, Y + radius * 2.f ), Vec2( endX, Y + radius * 2.f + 35.f ) ), 35.f, Stringf( "Gun Twins" ), Rgba8::WHITE, 0.618f, Vec2( 0.05f, 0.5f ) );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

GunTwinYoung::GunTwinYoung( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:GunShooter( def, startPos, startOrientation, startVelocity )
	, m_missileDef( ProjectileDefinition::GetDefinition( "CurveMissile" ) )
{
	m_selfClock = new Clock( *m_clock );
	m_state = 0;
	m_stateTimer = new Timer( m_state0Time, m_selfClock );
	m_stateTimer->Start();
	m_grenadeTimer = new Timer( 0.8f, m_selfClock );
}

GunTwinYoung::~GunTwinYoung()
{
	delete m_grenadeTimer;
	delete m_selfClock;
}

void GunTwinYoung::BeginPlay()
{
	GunShooter::BeginPlay();
}

void GunTwinYoung::Update( float deltaTime )
{
	if (m_isDead && m_deathTimer->HasPeriodElapsed()) {
		m_isGarbage = true;
		return;
	}
	else if (m_isDead) {
		m_clock->SetTimeScale( 1.f );
		AddForce( m_velocity * m_mass * 6.3f );
		UpdatePhysics( deltaTime );
		return;
	}
	Entity::Update( deltaTime );

	if (m_brother && m_brother->m_isDead) {
		m_brother = nullptr;
		m_isAngry = true;
		m_curStateIndex = 1;
		m_health += 20.f;
		m_selfClock->SetTimeScale( m_selfClock->GetTimeScale() * 1.2f );
	}

	if (m_stateTimer->HasPeriodElapsed()) {
		GoToNextState();
	}

	if (m_state == 0 || m_state == 1) {
		Vec2 forwardVector = GetForwardNormal();

		Vec2 targetPosition = m_target->m_position + Vec2::MakeFromPolarDegrees( m_targetOrientation, TARGET_RANGE_TO_KEEP );
		Vec2 displacement = targetPosition - m_position;
		m_gunOrientationDegrees = GetTurnedTowardDegrees( m_gunOrientationDegrees, (m_target->m_position - m_position).GetOrientationDegrees(), 60.f * deltaTime );
		if (GetDistanceSquared2D( m_position, targetPosition ) >= 25.f) {
			AddForce( forwardVector * m_def.m_flySpeed );
		}
		m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, (targetPosition - m_position).GetOrientationDegrees(), m_def.m_turnSpeed * deltaTime );
	}
	else if (m_state == 2) {
		if (m_grenadeTimer->DecrementPeriodIfElapsed()) {
			Vec2 position = GetRandomPointInDisc2D( 25.f, m_target->m_position );
			Missle* missile = (Missle*)g_theGame->SpawnEffectToGame( EffectType::Missile, m_position );
			missile->m_startPos = m_position;
			missile->m_targetPos = position;
			missile->m_owner = this;
			missile->m_flyTime = 1.3f;
			missile->BeginPlay();
		}
	}
	else if (m_state == 3) {
		AddForce( Vec2( 0.f, -120.f ) );
	}
	else if (m_state == 4) {
		AddForce( GetForwardNormal() * 200.f );
		AddForce( m_velocity * m_mass * 3.4f );
	}

	if (m_mainWeaponTimer->DecrementPeriodIfElapsed()) {
		GunFire();
	}
}

void GunTwinYoung::Render() const
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> gunVerts;
	verts.reserve( 100 );
	if (m_isAngry) {
		AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 255, 0, 0 ) );
	}
	else {
		AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 255, 153, 51 ) );
	}
	AddVertsForOBB2D( gunVerts, OBB2( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ), Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ), Vec2( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 0.2f ) ), Rgba8( 128, 128, 128 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.8f, -m_cosmeticRadius * 0.4f ), Vec2( m_cosmeticRadius * 1.1f, m_cosmeticRadius * 0.4f ) ), Rgba8( 127, 0, 255 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), m_color );
	g_theRenderer->DrawVertexArray( gunVerts );
}

void GunTwinYoung::Die()
{
	GunShooter::Die();
}

void GunTwinYoung::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false*/, Vec2 const& projectileVelocity /*= Vec2() */ )
{
	if (m_isDead) {
		return;
	}
	UNUSED( hitNormal );
	UNUSED( directDamage );
	UNUSED( projectileVelocity );
	m_health -= hit;
	g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/EnemyHit.wav" ), false, g_theApp->m_soundVolume * 0.1f );
	if (m_health > m_maxHealth) {
		m_health = m_maxHealth;
	}
	if (m_health <= 0.f) {
		Die();
		AddImpulse( Starship_GetStrongestPart( projectileVelocity ) * 60.f );
	}
}

void GunTwinYoung::GunFire()
{
	if (m_state == 1) {
		Vec2 shootForward = Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees + GetRandGen()->RollRandomFloatInRange( -30.f, 30.f ) );
		m_mainWeapon->Fire( shootForward, m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ) );
		// AddImpulse( -shootForward * NORMAL_SHOOT_IMPULSE );
	}
	else if (m_state == 3) {
		CurveMissile* proj = (CurveMissile*)g_theGame->SpawnProjectileToGame( m_missileDef, m_position, 0.f );
		proj->m_faction = m_def.m_faction;
		proj->m_damage = GetMainWeaponDamage();
		proj->m_goUp = m_goUp;
		proj->m_targetPosition = GetRandomPointInDisc2D( 20.f, m_target->m_position );
		proj->BeginPlay();
		m_goUp = !m_goUp;
	}
}

void GunTwinYoung::GoToNextState()
{
	if (m_state == 4) {
		constexpr float startOrientation = -30.f;
		constexpr float stepOrientation = (-2.f * startOrientation) / (float)(7 - 1);
		for (int i = 0; i < 7; i++) {
			Vec2 shootForward = Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees + startOrientation + i * stepOrientation );
			m_mainWeapon->Fire( shootForward, m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ) );
		}
		AddImpulse( -Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ) * NORMAL_SHOOT_IMPULSE * 2.f );
	}
	else if (m_state == 3) {
		g_theGame->DealRangeDamage( 1.f, m_position, m_physicsRadius * 1.5f );
	}

	if (m_isAngry) {
		m_curStateIndex = (m_curStateIndex + 1) % m_angryStateCount;
		m_state = m_stateSequenceAngry[m_curStateIndex];
	}
	else {
		m_curStateIndex = (m_curStateIndex + 1) % m_normalStateCount;
		m_state = m_stateSequenceNormal[m_curStateIndex];
	}

	if (m_state == 0) {
		m_targetOrientation = (m_position - m_target->m_position).GetOrientationDegrees();
		m_targetRelativeOrientation = GetRandGen()->RollRandomFloatInRange( -TARGET_RELATIVE_ORIENTATION, TARGET_RELATIVE_ORIENTATION );
		m_targetOrientation += m_targetRelativeOrientation;
		m_stateTimer->SetPeriodSeconds( m_state0Time );
		m_disableFriction = false;
		m_isInvincible = false;
		m_restrictIntoRoom = true;
	}
	else if (m_state == 1) {
		m_stateTimer->SetPeriodSeconds( m_state1Time );
		m_disableFriction = false;
		m_isInvincible = false;
		m_restrictIntoRoom = true;
	}
	else if (m_state == 2) {
		m_grenadeTimer->Start();
		m_stateTimer->SetPeriodSeconds( m_state2Time );
		m_disableFriction = false;
		m_isInvincible = false;
		m_restrictIntoRoom = true;
	}
	else if (m_state == 3) {
		m_stateTimer->SetPeriodSeconds( m_state3Time );
		do {
			m_jumpTargetPos = GetRandomPointInAABB2D( g_theGame->m_curRoom->m_bounds );
		} while (abs( m_jumpTargetPos.x - m_position.x ) < 60.f);
		m_disableFriction = true;
		m_isInvincible = true;
		m_restrictIntoRoom = false;
		float vx = (m_jumpTargetPos.x - m_position.x) / m_state3Time;
		float vy = (m_jumpTargetPos.y - m_position.y) / m_state3Time + 60.f * m_state3Time;
		AddImpulse( Vec2( vx, vy ) );
	}
	else if (m_state == 4) {
		m_stateTimer->SetPeriodSeconds( m_state4Time );
		AddImpulse( GetForwardNormal() * m_def.m_flySpeed * 2.f );
		m_disableFriction = false;
		m_isInvincible = false;
		m_restrictIntoRoom = true;
	}
	m_stateTimer->Start();
}

void GunTwinYoung::RenderUI() const
{
	float remainHealthRatio = (float)m_health / (float)m_maxHealth;
	remainHealthRatio = GetClamped( remainHealthRatio, 0.f, 1.f );
	Rgba8 healthColor = Rgba8( 255, 215, 0 );
	Rgba8 maxHealthColor = Rgba8( 255, 0, 0 );
	float startX = 1050.f;
	float endX = 1550.f;
	float Y = 30.f;
	float radius = 8.f;
	std::vector<Vertex_PCU> verts;
	AddVertsForCapsule2D( verts, Vec2( startX, Y ), Vec2( endX, Y ), radius, maxHealthColor );
	AddVertsForCapsule2D( verts, Vec2( endX - (endX - startX) * remainHealthRatio, Y ), Vec2( endX, Y ), radius, healthColor );

	std::vector<Vertex_PCU> textVerts;
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( startX, Y + radius * 2.f ), Vec2( endX, Y + radius * 2.f + 35.f ) ), 35.f, Stringf( "Gun Twins" ), Rgba8::WHITE, 0.618f, Vec2( 0.95f, 0.5f ) );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

GunAudience::GunAudience( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:GunShooter(def, startPos, startOrientation, startVelocity)
	,m_bounds(g_theGame->m_curRoom->m_bounds)
{
	m_hasReward = false;
	m_restrictIntoRoom = false;
}

GunAudience::~GunAudience()
{

}

void GunAudience::Update( float deltaTime )
{
	if (m_isAngry) {
		GunShooter::Update( deltaTime );
	}
	if (m_isDead) {
		m_isAngry = true;
	}
	if (!m_isDead) {
		m_position = m_bounds.GetNearestPoint( m_position );
	}
}

void GunAudience::Render() const
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> gunVerts;
	verts.reserve( 100 );
	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 255, 153, 51 ) );
	if (m_isAngry) {
		AddVertsForOBB2D( gunVerts, OBB2( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ), Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ), Vec2( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 0.2f ) ), Rgba8( 128, 128, 128 ) );
	}
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.8f, -m_cosmeticRadius * 0.4f ), Vec2( m_cosmeticRadius * 1.1f, m_cosmeticRadius * 0.4f ) ), Rgba8( 255, 102, 102 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );

	if (m_isAngry) {
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->SetModelConstants( Mat44(), m_color );
		g_theRenderer->DrawVertexArray( gunVerts );
	}
}

void GunAudience::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false*/, Vec2 const& projectileVelocity /*= Vec2() */ )
{
	GunShooter::BeAttacked( hit, hitNormal, directDamage, projectileVelocity );
	if (!m_isAngry) {
		m_isAngry = true;
		m_mainWeaponTimer->Start();
		m_color = Rgba8( 200, 0, 0 );
	}
}

BossDoubleGun::BossDoubleGun( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:GunShooter(def, startPos, startOrientation, startVelocity)
{
	m_state = 0;
	m_stateTimer = new Timer( m_stateTime[m_state], m_clock);
	m_stateTimer->Start();
	m_machineGunTimer = new Timer( 1.f, m_clock );
	m_renderPopularityTimer = new Timer( 1.f, GetGameClock() );

	EntityDefinition const& audienceDef = EntityDefinition::GetDefinition( "GunAudience" );

	constexpr int numOfAudiencePerWidth = 38;
	constexpr float stepDistForAudience = 200.f / (float)(numOfAudiencePerWidth + 2);
	constexpr int numOfAudiencePerHeight = numOfAudiencePerWidth / 2;
	Vec2 const& LBPos = g_theGame->m_curRoom->m_bounds.m_mins;
	for (int i = 0; i < numOfAudiencePerWidth; i++) {
		Vec2 spawnPos1 = LBPos + Vec2( (float)(i + 1) * stepDistForAudience, 2.5f );
		if (GetDistanceSquared2D( spawnPos1, m_target->m_position ) > 64.f) {
			Entity* e1 = g_theGame->SpawnEntityToGame( audienceDef, spawnPos1, 90.f );
			m_audience.push_back( e1 );
		}
		Vec2 spawnPos2 = LBPos + Vec2( (float)(i + 1) * stepDistForAudience, 97.5f );
		if (GetDistanceSquared2D( spawnPos2, m_target->m_position ) > 64.f) {
			Entity* e2 = g_theGame->SpawnEntityToGame( audienceDef, spawnPos2, 270.f );
			m_audience.push_back( e2 );
		}
	}
	for (int i = 0; i < numOfAudiencePerHeight; i++) {
		Vec2 spawnPos1 = LBPos + Vec2( 2.5f, (float)(i + 1) * stepDistForAudience );
		if (GetDistanceSquared2D( spawnPos1, m_target->m_position ) > 64.f) {
			Entity* e1 = g_theGame->SpawnEntityToGame( audienceDef, spawnPos1, 0.f );
			m_audience.push_back( e1 );
		}
		Vec2 spawnPos2 = LBPos + Vec2( 197.5f, (float)(i + 1) * stepDistForAudience );
		if (GetDistanceSquared2D( spawnPos2, m_target->m_position ) > 64.f) {
			Entity* e2 = g_theGame->SpawnEntityToGame( audienceDef, spawnPos2, 180.f );
			m_audience.push_back( e2 );
		}
	}

	m_lastFramePlayerHealth = m_target->m_health + ((PlayerShip*)m_target)->m_curArmor;
}

BossDoubleGun::~BossDoubleGun()
{
	delete m_renderPopularityTimer;
	delete m_machineGunTimer;
}

void BossDoubleGun::BeginPlay()
{

}

void BossDoubleGun::Update( float deltaTime )
{
	if (m_isDead && m_deathTimer->HasPeriodElapsed()) {
		m_isGarbage = true;
		return;
	}
	else if (m_isDead) {
		m_clock->SetTimeScale( 1.f );
		AddForce( m_velocity * m_mass * 6.3f );
		UpdatePhysics( deltaTime );
		return;
	}
	if (m_popularity <= 0) {
		Die();
		return;
	}
	Entity::Update( deltaTime );

	for (int i = 0; i < (int)m_audience.size(); i++) {
		if (m_audience[i] && !m_audience[i]->IsAlive()) {
			m_audience[i] = nullptr;
			ReducePopularity( 10.f );
		}
	}

	if (m_stateTimer->HasPeriodElapsed()) {
		GoToNextState();
	}
	Vec2 forwardVector = GetForwardNormal();

	Vec2 targetPosition = m_target->m_position + Vec2::MakeFromPolarDegrees( m_targetOrientation, TARGET_RANGE_TO_KEEP );
	Vec2 displacement = targetPosition - m_position;
	m_gunOrientationDegrees = GetTurnedTowardDegrees( m_gunOrientationDegrees, (m_target->m_position - m_position).GetOrientationDegrees(), 60.f * deltaTime );
	if (m_state == 0) {
		if (GetDistanceSquared2D( m_position, targetPosition ) >= 64.f) {
			AddForce( forwardVector * m_def.m_flySpeed );
			m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, (targetPosition - m_position).GetOrientationDegrees(), m_def.m_turnSpeed * deltaTime );
		}
		else {
			m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, (m_target->m_position - m_position).GetOrientationDegrees(), m_def.m_turnSpeed * deltaTime );
		}
	}
	else if (m_state == 2) {
		AddForce( GetForwardNormal() * 120.f );
		AddForce( m_velocity * m_mass * 3.4f );
	}
	else if (m_state == 3) {
		AddForce( m_velocity * m_mass * 3.4f );
	}

	if (m_state == 0 || m_state == 1) {
		if (m_mainWeaponTimer->HasPeriodElapsed()) {
			m_mainWeaponTimer->Start();
			GunFire( 0 );
		}
	}

	if (m_lastFramePlayerHealth > m_target->m_health + ((PlayerShip*)m_target)->m_curArmor) {
		m_lastFramePlayerHealth = m_target->m_health + ((PlayerShip*)m_target)->m_curArmor;
		m_doNotHitPlayer = false;
		ReducePopularity( -5.f );
	}
}

void BossDoubleGun::Render() const
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> gunVerts;
	verts.reserve( 100 );
	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 255, 153, 51 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 0.3f ), Vec2( m_cosmeticRadius * 0.75f, m_cosmeticRadius * 0.7f ) ), Rgba8( 0, 0, 0 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.6f, -m_cosmeticRadius * 0.7f ), Vec2( m_cosmeticRadius * 0.75f, -m_cosmeticRadius * 0.3f ) ), Rgba8( 0, 0, 0 ) );
	AddVertsForOBB2D( gunVerts, OBB2( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ), Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ), Vec2( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 0.2f ) ), Rgba8( 128, 128, 128 ) );
	AddVertsForOBB2D( gunVerts, OBB2( m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees - GUN_RELATIVE_DEGREES, m_cosmeticRadius ), Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ), Vec2( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 0.2f ) ), Rgba8( 128, 128, 128 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.8f, -m_cosmeticRadius * 0.4f ), Vec2( m_cosmeticRadius * 1.1f, m_cosmeticRadius * 0.4f ) ), Rgba8( 0, 0, 255 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), m_color );
	g_theRenderer->DrawVertexArray( gunVerts );
}

void BossDoubleGun::Die()
{
	GunShooter::Die();
}

void BossDoubleGun::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false*/, Vec2 const& projectileVelocity /*= Vec2() */ )
{
	if (m_isDead) {
		return;
	}
	UNUSED( hitNormal );
	UNUSED( directDamage );
	UNUSED( projectileVelocity );
	m_health -= hit;
	if (m_health > m_maxHealth) {
		m_health = m_maxHealth;
	}
	if (hit > 0.f) {
		g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/EnemyHit.wav" ), false, g_theApp->m_soundVolume * 0.1f );
		m_popularity -= hit;
	}
	if (m_health <= 0.f) {
		Die();
		AddImpulse( Starship_GetStrongestPart( projectileVelocity ) * 60.f );
	}
}

void BossDoubleGun::GunFire( int skill )
{
	if (skill == 0) {
		Vec2 shootForward = Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees + GetRandGen()->RollRandomFloatInRange( -30.f, 30.f ) );
		m_mainWeapon->Fire( shootForward, m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ) );
		shootForward = Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees + GetRandGen()->RollRandomFloatInRange( -30.f, 30.f ) );
		m_mainWeapon->Fire( shootForward, m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees - GUN_RELATIVE_DEGREES, m_cosmeticRadius ) );
	}
	else if (skill == 1) {
		constexpr float startOrientation = -30.f;
		constexpr float stepOrientation = (-2.f * startOrientation) / (float)(7 - 1);
		for (int i = 0; i < 7; i++) {
			Vec2 shootForward = Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees + startOrientation + i * stepOrientation );
			m_mainWeapon->Fire( shootForward, m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ) );
		}
		for (int i = 0; i < 7; i++) {
			Vec2 shootForward = Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees + startOrientation + i * stepOrientation );
			m_mainWeapon->Fire( shootForward, m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees - GUN_RELATIVE_DEGREES, m_cosmeticRadius ) );
		}
		AddImpulse( -Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ) * NORMAL_SHOOT_IMPULSE * 2.f );
	}
	else if (skill == 2) {
		Projectile* proj = m_mainWeapon->Fire( Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ), m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + GUN_RELATIVE_DEGREES, m_cosmeticRadius ) );
		proj->m_velocity *= 4.f;
		proj = m_mainWeapon->Fire( Vec2::MakeFromPolarDegrees( m_gunOrientationDegrees ), m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees - GUN_RELATIVE_DEGREES, m_cosmeticRadius ) );
		proj->m_velocity *= 4.f;
	}
}

void BossDoubleGun::GoToNextState()
{
	if (m_state == 2) {
		GunFire( 1 );
	}
	else if (m_state == 3) {
		GunFire( 2 );
	}

	m_curStateIndex = (m_curStateIndex + 1) % m_stateCount;
	m_state = m_stateSequence[m_curStateIndex];
	m_stateTimer->SetPeriodSeconds( m_stateTime[m_state] );

	if (m_curStateIndex == 0) {
		if (m_doNotHitPlayer) {
			ReducePopularity( 100.f );
		}
		m_doNotHitPlayer = true;
	}

	// wonder
	if (m_state == 0) {
		m_targetOrientation = (m_position - m_target->m_position).GetOrientationDegrees();
		m_targetRelativeOrientation = GetRandGen()->RollRandomFloatInRange( -TARGET_RELATIVE_ORIENTATION * 0.5f, TARGET_RELATIVE_ORIENTATION * 0.5f );
		m_targetOrientation += m_targetRelativeOrientation;
		m_machineGunTimer->SetPeriodSeconds( m_stateTime[m_state] );
		m_machineGunTimer->Start();
	}
	// teleport
	else if (m_state == 1) {
		AABB2 bounds = AABB2( g_theGame->m_curRoom->m_bounds.m_mins + Vec2( 30.f, 15.f ), g_theGame->m_curRoom->m_bounds.m_mins + Vec2( 170.f, 85.f ) );
		do {
			m_position = bounds.GetRandomPointInside();
		} while (GetDistanceSquared2D( m_position, m_target->m_position ) < 400.f);
		m_orientationDegrees = (m_target->m_position - m_position).GetOrientationDegrees();
		m_gunOrientationDegrees = m_orientationDegrees;
		m_machineGunTimer->SetPeriodSeconds( m_stateTime[m_state] );
		m_machineGunTimer->Start();
	}
	// dash
	else if (m_state == 2) {
		AddImpulse( GetForwardNormal() * m_def.m_flySpeed );
	}
	// dodge
	else if (m_state == 3) {
		int rnd = GetRandGen()->RollRandomIntInRange( 0, 1 );
		if (rnd == 1) {
			AddImpulse( GetForwardNormal().GetRotated90Degrees() * m_def.m_flySpeed );
		}
		else {
			AddImpulse( GetForwardNormal().GetRotatedMinus90Degrees() * m_def.m_flySpeed );
		}
	}
	m_stateTimer->Start();
}

void BossDoubleGun::RenderUI() const
{
	std::vector<Vertex_PCU> textVerts;
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 710.f ), Vec2( 1600.f, 750.f ) ), 20.f, Stringf( "Arena Champion Double Gun\nPopularity %d", RoundDownToInt( m_popularity ) ) );
	if (m_renderPopularityTimer->HasStartedAndNotPeriodElapsed()) {
		if (m_popularityToRender > 0.f) {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 200.f, 690.f ), Vec2( 1600.f, 710.f ) ), 20.f, Stringf( "-%d", abs( m_popularityToRender ) ), Rgba8( 0, 255, 0 ) );
		}
		else {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 200.f, 690.f ), Vec2( 1600.f, 710.f ) ), 20.f, Stringf( "+%d", abs( m_popularityToRender ) ), Rgba8( 255, 0, 0 ) );
		}
	}

	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );
}

void BossDoubleGun::ReducePopularity( float count )
{
	m_popularity -= count;
	if (abs(count) >= 4.f) {
		m_renderPopularityTimer->Start();
		m_popularityToRender = RoundDownToInt( count );
	}
}

BossArmsMaster::BossArmsMaster( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
	, m_weaponSprites(*g_theRenderer->CreateOrGetTextureFromFile("Data/Images/ArmsMasterWeapons.png"), IntVec2(4, 2))
{
	m_weaponTimer = new Timer( 2.f, m_clock );
	m_weaponTimer->Start();
	m_deathTimer = new Timer( 2.f, m_clock );
	m_shootTimer = new Timer( 1.f, m_clock );
	m_target = g_theGame->GetPlayerEntity();
	m_targetOrientation = (m_position - m_target->m_position).GetOrientationDegrees();
	m_targetRelativeOrientation = GetRandGen()->RollRandomFloatInRange( -TARGET_RELATIVE_ORIENTATION, TARGET_RELATIVE_ORIENTATION );
	m_targetOrientation += m_targetRelativeOrientation;

	m_playerBulletGun = (BulletGun*)g_theGame->CreateWeaponComponent( WeaponDefinition::GetDefinition( "BulletGun" ), this );
	m_rayShooter = (RayShooter*)g_theGame->CreateWeaponComponent( WeaponDefinition::GetDefinition( "RayShooter" ), this );
	m_machete = (Machete*)g_theGame->CreateWeaponComponent( WeaponDefinition::GetDefinition( "Machete" ), this );
	m_rocketShooter = (RocketShooter*)g_theGame->CreateWeaponComponent( WeaponDefinition::GetDefinition( "RocketLauncher" ), this );
	m_enemyBulletGun = (EnemyBulletGun*)g_theGame->CreateWeaponComponent( WeaponDefinition::GetDefinition( "EnemyBulletGun" ), this );
	m_missileGun = (MissileGun*)g_theGame->CreateWeaponComponent( WeaponDefinition::GetDefinition( "MissileGun" ), this );
	m_sprayWeapon = (Spray*)g_theGame->CreateWeaponComponent( WeaponDefinition::GetDefinition( "Spray" ), this );
	m_coinGun = (CoinGun*)g_theGame->CreateWeaponComponent( WeaponDefinition::GetDefinition( "CoinGun" ), this );

	m_rndNextNum = GetRandGen()->RollRandomIntInRange( 1, 7 );
}

BossArmsMaster::~BossArmsMaster()
{

}

void BossArmsMaster::BeginPlay()
{

}

void BossArmsMaster::Update( float deltaTime )
{
	if (m_isDead && m_deathTimer->HasPeriodElapsed()) {
		m_isGarbage = true;
		return;
	}
	else if (m_isDead) {
		m_clock->SetTimeScale( 1.f );
		AddForce( m_velocity * m_mass * 6.3f );
		UpdatePhysics( deltaTime );
		return;
	}
	Entity::Update( deltaTime );

	if (m_isWondering) {
		if (m_weaponTimer->HasPeriodElapsed()) {
			m_isWondering = false;
			GoToNextWeaponMode();
		}
		else {
			PerformWonder( deltaTime );
		}
	}
	else {
		if (m_weaponTimer->HasPeriodElapsed()) {
			m_rndNextNum = GetRandGen()->RollRandomIntInRange( 1, 7 );
			//m_rndNextNum = 1;
			m_isWondering = true;
			m_weaponTimer->SetPeriodSeconds( 3.5f );
			m_weaponTimer->Start();
			m_targetOrientation = (m_position - m_target->m_position).GetOrientationDegrees();
			m_targetRelativeOrientation = GetRandGen()->RollRandomFloatInRange( -TARGET_RELATIVE_ORIENTATION * 0.5f, TARGET_RELATIVE_ORIENTATION * 0.5f );
			m_targetOrientation += m_targetRelativeOrientation;
		}
		else {
			// use weapon
			// player bullet
			if (m_curWeaponMode == 0) {
				// chase player and shoot
				Vec2 disp = m_target->m_position - m_position;
				Vec2 dispNormalized = disp.GetNormalized();
				AddForce( dispNormalized * m_def.m_flySpeed );
				m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, dispNormalized.GetOrientationDegrees(), deltaTime * m_def.m_turnSpeed );
				if (m_shootTimer->DecrementPeriodIfElapsed()) {
					Vec2 fwdVec = GetForwardNormal();
					Vec2 leftVec = fwdVec.GetRotated90Degrees();
					Projectile* proj = m_playerBulletGun->Fire( fwdVec, m_position + fwdVec * m_cosmeticRadius * 0.5f );
					proj->m_velocity *= 1.3f;
					proj = m_playerBulletGun->Fire( fwdVec, m_position + leftVec * m_cosmeticRadius * 0.3f );
					proj->m_velocity *= 1.3f;
					proj = m_playerBulletGun->Fire( fwdVec, m_position - leftVec * m_cosmeticRadius * 0.3f );
					proj->m_velocity *= 1.3f;
				}
			}
			// ray
			else if (m_curWeaponMode == 1) {
				Vec2 disp = m_target->m_position - m_position;
				m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, disp.GetOrientationDegrees(), deltaTime * m_def.m_turnSpeed );
				Vec2 fwdVec = GetForwardNormal();
				AddForce( fwdVec * m_def.m_flySpeed * 0.5f );
				if (m_shootTimer->DecrementPeriodIfElapsed()) {
					if (m_rayShooterCounter == 0) {
						m_rayShooter->Fire( fwdVec, m_position );
					}
					else if (m_rayShooterCounter == 1) {
						for (int i = 0; i < 3; i++) {
							float startOrientation = m_orientationDegrees - 60.f;
							Vec2 rayFwd = Vec2::MakeFromPolarDegrees( startOrientation + i * 60.f );
							m_rayShooter->Fire( rayFwd, m_position );
						}
					}
					else if (m_rayShooterCounter == 2) {
						for (int i = 0; i < 5; i++) {
							float startOrientation = m_orientationDegrees - 120.f;
							Vec2 rayFwd = Vec2::MakeFromPolarDegrees( startOrientation + i * 60.f );
							m_rayShooter->Fire( rayFwd, m_position );
						}
					}
					else if (m_rayShooterCounter >= 3) {
						for (int i = 0; i < 6; i++) {
							float startOrientation = m_orientationDegrees - m_rayShooterCounter * 20.f;
							Vec2 rayFwd = Vec2::MakeFromPolarDegrees( startOrientation + i * 60.f );
							m_rayShooter->Fire( rayFwd, m_position );
						}
					}
					m_rayShooterCounter++;
				}
			}
			// machete
			else if (m_curWeaponMode == 2) {
				if (m_macheteState == 0) {
					if (m_macheteGoLeft) {
						m_position = Interpolate( m_macheteStartPos, Vec2( g_theGame->m_curRoom->m_bounds.m_mins.x - 20.f, m_macheteStartPos.y ), SmoothStart2( m_shootTimer->GetElapsedFraction() ) );
					}
					else {
						m_position = Interpolate( m_macheteStartPos, Vec2( g_theGame->m_curRoom->m_bounds.m_maxs.x + 20.f, m_macheteStartPos.y ), SmoothStart2( m_shootTimer->GetElapsedFraction() ) );
					}
				}
				else if (m_macheteState == 2) {
					if (m_macheteFromTop) {
						m_position = Interpolate( Vec2( m_target->m_position.x, g_theGame->m_curRoom->m_bounds.m_maxs.y + 10.f ), m_target->m_position + Vec2( 0.f, m_physicsRadius + 2.f + m_target->m_physicsRadius ), SmoothStop2( m_shootTimer->GetElapsedFraction() ) );
					}
					else {
						m_position = Interpolate( Vec2( m_target->m_position.x, g_theGame->m_curRoom->m_bounds.m_mins.y - 10.f ), m_target->m_position + Vec2( 0.f, -m_physicsRadius - 2.f - m_target->m_physicsRadius ), SmoothStop2( m_shootTimer->GetElapsedFraction() ) );
					}
				}

				if (m_shootTimer->DecrementPeriodIfElapsed()) {
					m_macheteState++;
					if (m_macheteState == 1) {
						m_shootTimer->SetPeriodSeconds( 0.5f );
					}
					else if (m_macheteState == 2) {
						if (m_target->m_position.y < g_theGame->m_curRoom->m_bounds.GetCenter().y) {
							m_macheteFromTop = true;
						}
						else {
							m_macheteFromTop = false;
						}
						m_shootTimer->SetPeriodSeconds( 1.f );
					}
					else if (m_macheteState == 3) {
						m_orientationDegrees = (m_target->m_position - m_position).GetOrientationDegrees();
						if (m_macheteFromTop) {
							m_machete->Fire( GetForwardNormal(), m_position );
						}
						else {
							m_machete->Fire( GetForwardNormal(), m_position );
						}
						m_restrictIntoRoom = true;
						m_disableFriction = false;
						m_shootTimer->SetPeriodSeconds( 0.5f );
					}
					else if (m_macheteState == 4) {
						AddImpulse( (m_position - m_target->m_position).GetNormalized() * 250.f );
					}
				}
			}
			// rocket
			else if (m_curWeaponMode == 3) {
				Vec2 disp = m_target->m_position - m_position;
				Vec2 fwdVec = GetForwardNormal();
				m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, disp.GetOrientationDegrees(), deltaTime * m_def.m_turnSpeed );
				if (m_shootTimer->DecrementPeriodIfElapsed()) {
					if (m_rocketDodge) {
						Vec2 leftVec = fwdVec.GetRotated90Degrees();
						if (m_dodgeLeft) {
							AddImpulse( leftVec * 100.f );
						}
						else {
							AddImpulse( -leftVec * 100.f );
						}
						m_dodgeLeft = !m_dodgeLeft;
						m_rocketDodge = false;
					}
					else {
						m_rocketShooter->Fire( fwdVec, m_position );
						m_rocketDodge = true;
					}
				}
			}
			// enemy bullet
			else if (m_curWeaponMode == 4) {
				// teleport to one place and shoot in 16 directions
				if (m_shootTimer->DecrementPeriodIfElapsed()) {
					Vec2 newPos;
					AABB2 bounds = AABB2( g_theGame->m_curRoom->m_bounds.m_mins + Vec2( m_cosmeticRadius, m_cosmeticRadius ), g_theGame->m_curRoom->m_bounds.m_maxs - Vec2( m_cosmeticRadius, m_cosmeticRadius ) );
					do {
						newPos = bounds.GetRandomPointInside();
					} while (GetDistanceSquared2D(newPos, m_target->m_position) < 4900.f);
					m_position = newPos;
					constexpr float stepPerI = 360.f / 30.f;
					for (int i = 0; i < 30; i++) {
						EnemyBullet* bullet = (EnemyBullet*)m_enemyBulletGun->Fire( Vec2::MakeFromPolarDegrees( m_orientationDegrees + i * stepPerI ), m_position );
						bullet->m_lifeTimer->SetPeriodSeconds( 6.f );
					}
				}
			}
			// missile
			else if (m_curWeaponMode == 5) {
				m_targetOrientation += deltaTime * 60.f;
				PerformWonder( deltaTime );
				if (m_shootTimer->DecrementPeriodIfElapsed()) {
					Vec2 forward( 0.f, 0.f );
					CurveMissile* proj = (CurveMissile*)m_missileGun->Fire( forward, m_position );
					proj->m_goUp = m_missileGoUp;
					Vec2 taegetVelFwd = m_target->m_velocity.GetNormalized();
					proj->m_targetPosition = m_target->m_position + taegetVelFwd * GetRandGen()->RollRandomFloatInRange( 0.f, 50.f );
					proj->BeginPlay();
					m_missileGoUp = !m_missileGoUp;
				}
			}
			// spray
			else if (m_curWeaponMode == 6) {
				Vec2 disp = m_target->m_position - m_position;
				m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, disp.GetOrientationDegrees(), deltaTime * m_def.m_turnSpeed );
				if (m_shootTimer->DecrementPeriodIfElapsed()) {
					if (m_isSprayDash) {
						AddImpulse( GetForwardNormal() * 200.f );
					}
					else {
						m_sprayWeapon->m_isSector = true;
						m_sprayWeapon->m_sectorRangeDegrees = 360.f;
						m_sprayWeapon->m_length = 40.f;
						m_sprayWeapon->Fire( Vec2(), m_position );
					}
					m_isSprayDash = !m_isSprayDash;
				}
			}
			// coin
			else if (m_curWeaponMode == 7) {
				m_orientationDegrees += deltaTime * 720.f;
				if (m_shootTimer->DecrementPeriodIfElapsed()) {
					m_coinGun->Fire( GetForwardNormal(), m_position );
				}
			}
		}
	}

}

void BossArmsMaster::Render() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100 );
	// machete
	if (m_curWeaponMode == 2) {
		std::vector<Vertex_PCU> worldVerts;
		worldVerts.reserve( 6 );
		if (m_macheteState == 0) {
			if (m_macheteGoLeft) {
				AddVertsForAABB2D( worldVerts, AABB2( Vec2( g_theGame->m_curRoom->m_bounds.m_mins.x - 20.f, m_macheteStartPos.y - 0.5f ), Vec2( m_position.x, m_macheteStartPos.y + 0.5f ) ), Rgba8( 192, 192, 192 ) );
			}
			else {
				AddVertsForAABB2D( worldVerts, AABB2( Vec2( m_position.x, m_macheteStartPos.y - 0.5f ), Vec2( g_theGame->m_curRoom->m_bounds.m_maxs.x + 20.f, m_macheteStartPos.y + 0.5f ) ), Rgba8( 192, 192, 192 ) );
			}
		}
		else if (m_macheteState == 2) {
			if (m_macheteFromTop) {
				AddVertsForAABB2D( worldVerts, AABB2( Vec2( m_target->m_position.x - 0.5f, m_position.y ), Vec2( m_target->m_position.x + 0.5f, g_theGame->m_curRoom->m_bounds.m_maxs.y + 10.f ) ), Rgba8( 192, 192, 192 ) );
			}
			else {
				AddVertsForAABB2D( worldVerts, AABB2( Vec2( m_target->m_position.x - 0.5f, g_theGame->m_curRoom->m_bounds.m_mins.y - 10.f ), Vec2( m_target->m_position.x + 0.5f, m_position.y ) ), Rgba8( 192, 192, 192 ) );
			}
		}

		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( worldVerts );
	}

	// render switching weapon
	std::vector<Vertex_PCU> weaponVerts;
	weaponVerts.reserve( 6 );
	if (m_isWondering) {
		float fraction = m_weaponTimer->GetElapsedFraction() * 1.5f;
		fraction = GetClamped( fraction, 0.f, 1.f );
		//fraction = SmoothStop3( fraction );
		int weaponIllustration = (m_curWeaponMode + RoundDownToInt( (float)m_rndNextNum * fraction )) % 8;
		AddVertsForAABB2D( weaponVerts, AABB2( m_position + Vec2( -m_cosmeticRadius * 0.45f, m_cosmeticRadius * 1.1f ), m_position + Vec2( m_cosmeticRadius * 0.45f, m_cosmeticRadius * 2.f ) ), Rgba8::WHITE, m_weaponSprites.GetSpriteUVs( weaponIllustration ) );
	}
	else {
		AddVertsForAABB2D( weaponVerts, AABB2( m_position + Vec2( -m_cosmeticRadius * 0.45f, m_cosmeticRadius * 1.1f ), m_position + Vec2( m_cosmeticRadius * 0.45f, m_cosmeticRadius * 2.f ) ), Rgba8::WHITE, m_weaponSprites.GetSpriteUVs( m_curWeaponMode ) );
	}
	g_theRenderer->BindTexture( &m_weaponSprites.GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( weaponVerts );

	AddVertsForDisc2D( verts, Vec2(), m_cosmeticRadius, Rgba8( 255, 153, 51 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.6f, m_cosmeticRadius * 0.3f ), Vec2( m_cosmeticRadius * 0.75f, m_cosmeticRadius * 0.7f ) ), Rgba8( 0, 0, 0 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.6f, -m_cosmeticRadius * 0.7f ), Vec2( m_cosmeticRadius * 0.75f, -m_cosmeticRadius * 0.3f ) ), Rgba8( 0, 0, 0 ) );
	AddVertsForAABB2D( verts, AABB2( Vec2( m_cosmeticRadius * 0.7f, -m_cosmeticRadius * 0.45f ), Vec2( m_cosmeticRadius * 1.1f, m_cosmeticRadius * 0.45f ) ), m_faceColor );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void BossArmsMaster::Die()
{
	g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/EnemyDie.wav" ), false, g_theApp->m_soundVolume * 0.1f );
	m_isDead = true;
	m_isInvincible = true;
	m_restrictIntoRoom = false;
	m_deathTimer->Start();
	m_color = Rgba8( 128, 128, 128, 128 );
	if (m_hasReward) {
		SpawnReward();
	}
}

void BossArmsMaster::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false*/, Vec2 const& projectileVelocity /*= Vec2() */ )
{
	if (m_isDead) {
		return;
	}
	UNUSED( directDamage );
	Vec2 forwardNormal = GetForwardNormal();
	if (hit > 0.f && m_def.m_isShielded && hitNormal != Vec2( 0.f, 0.f ) && DotProduct2D( hitNormal, forwardNormal ) > 0.1f) {
		StarshipEffect* shieldEffect = g_theGame->SpawnEffectToGame( EffectType::Shield, m_position );
		((StarshipShield*)shieldEffect)->m_owner = this;
		shieldEffect->BeginPlay();
		return;
	}
	else {
		m_health -= hit;
		g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/EnemyHit.wav" ), false, g_theApp->m_soundVolume * 0.1f );
	}
	if (m_health > m_maxHealth) {
		m_health = m_maxHealth;
	}
	if (m_health <= 0.f) {
		Die();
		AddImpulse( Starship_GetStrongestPart( projectileVelocity ) * 60.f );
	}
}

void BossArmsMaster::PerformWonder( float deltaTime )
{
	// perform wondering
	Vec2 forwardVector = GetForwardNormal();

	Vec2 targetPosition = m_target->m_position + Vec2::MakeFromPolarDegrees( m_targetOrientation, TARGET_RANGE_TO_KEEP );
	Vec2 displacement = targetPosition - m_position;
	if (GetDistanceSquared2D( m_position, targetPosition ) >= 64.f) {
		AddForce( forwardVector * m_def.m_flySpeed );
		m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, (targetPosition - m_position).GetOrientationDegrees(), m_def.m_turnSpeed * deltaTime );
	}
	else {
		m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, (m_target->m_position - m_position).GetOrientationDegrees(), m_def.m_turnSpeed * deltaTime );
	}
}

void BossArmsMaster::GoToNextWeaponMode()
{
	m_curWeaponMode = (m_curWeaponMode + m_rndNextNum) % 8;
	// player bullet
	if (m_curWeaponMode == 0) {
		m_faceColor = Rgba8( 51, 153, 255 );
		m_weaponTimer->SetPeriodSeconds( 4.f );
		m_shootTimer->SetPeriodSeconds( 0.18f );
	}
	// ray
	else if (m_curWeaponMode == 1) {
		m_faceColor = Rgba8( 204, 102, 0 );
		m_weaponTimer->SetPeriodSeconds( 4.f );
		m_shootTimer->SetPeriodSeconds( 0.49f );
		m_shootTimer->SetElapsedTime( 0.5f );
	}
	// machete
	else if (m_curWeaponMode == 2) {
		m_faceColor = Rgba8( 224, 224, 224 );
		m_weaponTimer->SetPeriodSeconds( 4.f );
		Vec2 disp = m_target->m_position - m_position;
		if (disp.x < 0.f) {
			m_macheteGoLeft = false;
		}
		else {
			m_macheteGoLeft = true;
		}
		m_macheteState = 0;
		m_shootTimer->SetPeriodSeconds( 1.5f );
		m_restrictIntoRoom = false;
		m_disableFriction = true;
		m_macheteStartPos = m_position;
	}
	// rocket
	else if (m_curWeaponMode == 3) {
		m_faceColor = Rgba8( 0, 102, 204 );
		m_weaponTimer->SetPeriodSeconds( 5.f );
		m_shootTimer->SetPeriodSeconds( 0.249f );
		m_rocketDodge = true;
		m_dodgeLeft = true;
	}
	// enemy bullet
	else if (m_curWeaponMode == 4) {
		m_faceColor = Rgba8( 255, 204, 153 );
		m_weaponTimer->SetPeriodSeconds( 6.f );
		m_shootTimer->SetPeriodSeconds( 0.49f );
	}
	// missile
	else if (m_curWeaponMode == 5) {
		m_faceColor = Rgba8( 255, 255, 153 );
		m_weaponTimer->SetPeriodSeconds( 5.f );
		m_shootTimer->SetPeriodSeconds( 0.15f );
	}
	// spray
	else if (m_curWeaponMode == 6) {
		m_faceColor = Rgba8( 153, 255, 51 );
		m_weaponTimer->SetPeriodSeconds( 4.f );
		m_shootTimer->SetPeriodSeconds( 0.49f );
		m_isSprayDash = true;
	}
	// coin
	else if (m_curWeaponMode == 7) {
		m_faceColor = Rgba8( 255, 255, 0 );
		m_weaponTimer->SetPeriodSeconds( 5.f );
		m_shootTimer->SetPeriodSeconds( 0.02f );
	}
	m_weaponTimer->Start();
	m_shootTimer->Start();
}

void BossArmsMaster::RenderUI() const
{
	float remainHealthRatio = (float)m_health / (float)m_maxHealth;
	remainHealthRatio = GetClamped( remainHealthRatio, 0.f, 1.f );
	Rgba8 healthColor = Rgba8( 255, 215, 0 );
	Rgba8 maxHealthColor = Rgba8( 255, 0, 0 );
	float startX, endX;
	startX = 1050.f;
	endX = 1550.f;
	
	float Y = 30.f;
	float radius = 8.f;
	std::vector<Vertex_PCU> verts;
	AddVertsForCapsule2D( verts, Vec2( startX, Y ), Vec2( endX, Y ), radius, maxHealthColor );
	AddVertsForCapsule2D( verts, Vec2( endX - (endX - startX) * remainHealthRatio, Y ), Vec2( endX, Y ), radius, healthColor );

	std::vector<Vertex_PCU> textVerts;
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( startX, Y + radius * 2.f ), Vec2( endX, Y + radius * 2.f + 35.f ) ), 35.f, Stringf( "Arms Master" ), Rgba8::WHITE, 0.618f, Vec2( 0.95f, 0.5f ) );
	
	
	
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}
