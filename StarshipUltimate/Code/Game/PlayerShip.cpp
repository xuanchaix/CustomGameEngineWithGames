#include "Game/PlayerShip.hpp"
#include "Game/Game.hpp"
#include "Game/Weapon.hpp"
#include "Game/Room.hpp"
#include "Game/Item.hpp"
#include "Game/PlayerController.hpp"
#include "Game/PlayerFaction.hpp"
#include "Game/App.hpp"

constexpr float RAY_MAX_TIME = 1.5f;

PlayerShip::PlayerShip( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity(def, startPos, startOrientation, startVelocity)
{
	m_subWeaponClock = new Clock( *m_clock );
	m_maxHealth = 3.f;
	m_health = 3.f;
	m_invincibleTimer = new Timer( 0.f, m_clock );
	m_dashTimer = new Timer( m_dashingCooldown, m_clock );
	m_revengeBulletTimer = new Timer( REVENGE_BULLET_TIME, m_clock );
	m_revengeBulletShootTimer = new Timer( 0.05f, m_clock );
	m_selfDamageBulletTimer = new Timer( SELF_DAMAGE_BULLET_TIME, m_clock );
	m_selfDamageShootTimer = new Timer( 0.199f, m_clock );
	m_revengeSpeedTimer = new Timer( 2.f, m_clock );
	m_subWeaponCoolDownTimer = new Timer( 1.f, m_subWeaponClock );
	m_timeStopTimer = new Timer( 8.f, m_clock );
	m_subWeaponCoolDownTimer->Start();
	m_mainWeaponDamage = m_def.m_weaponDamage;
	m_maxArmor = 3.f;
	m_curArmor = 3.f;
	m_shield = (PlayerShield*)g_theGame->SpawnEffectToGame( EffectType::PlayerShield, m_position );
	m_shield->m_owner = this;
	m_shield->m_radius = m_cosmeticRadius * 1.3f;
	m_itemList.reserve( 40 );
	m_deathTimer = new Timer( 3.f, GetGameClock() );

	SubscribeEventCallbackFunction( "Command_GetItem", Event_GetItem );
	SubscribeEventCallbackFunction( "Command_LoseItem", Event_LoseItem );
	SubscribeEventCallbackFunction( "Command_RandItem", Event_RandItem );
}

PlayerShip::~PlayerShip()
{
	delete m_invincibleTimer;
	delete m_dashTimer;
	delete m_revengeBulletShootTimer;
	delete m_revengeBulletTimer;
	delete m_selfDamageBulletTimer;
	delete m_selfDamageShootTimer;
	delete m_subWeaponCoolDownTimer;
	delete m_timeStopTimer;
	delete m_deathTimer;
}

void PlayerShip::BeginPlay()
{
	Entity::BeginPlay();
	Vertex_PCU shipVerts[15] = {
		Vertex_PCU( Vec3( 0.f,  2.f, 0.f ), Rgba8::WHITE, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f,  1.f, 0.f ), Rgba8::WHITE, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 2.f,  1.f, 0.f ), Rgba8::WHITE, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f,  1.f, 0.f ),Rgba8::WHITE, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f, -1.f, 0.f ),Rgba8::WHITE, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f,  1.f, 0.f ), Rgba8::WHITE, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f, -1.f, 0.f ),Rgba8::WHITE, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -1.f, 0.f ), Rgba8::WHITE, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f,  1.f, 0.f ), Rgba8::WHITE, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 1.f,  0.f, 0.f ), Rgba8::WHITE, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f,  1.f, 0.f ), Rgba8::WHITE, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -1.f, 0.f ), Rgba8::WHITE, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 2.f, -1.f, 0.f ), Rgba8::WHITE, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f, -1.f, 0.f ),Rgba8::WHITE, Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -2.f, 0.f ), Rgba8::WHITE, Vec2( 0.f, 0.f ) ),
	};
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer( (size_t)15 * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( shipVerts, (size_t)15 * sizeof( Vertex_PCU ), m_vertexBuffer );

	m_asteroid = (PlayerAsteroid*)g_theGame->SpawnEntityToGame( EntityDefinition::GetDefinition( "PlayerAsteriod" ), m_position, 0.f );
	m_asteroid->m_owner = this;
	m_asteroid->BeginPlay();
	m_diagonalRetinue = (DiagonalRetinue*)g_theGame->SpawnEntityToGame( EntityDefinition::GetDefinition( "DiagonalRetinue" ), m_position, 0.f );
	m_diagonalRetinue->m_owner = this;
	m_diagonalRetinue->BeginPlay();
	m_laserWingPlane = (LaserWingPlane*)g_theGame->SpawnEntityToGame( EntityDefinition::GetDefinition( "LaserWingPlane" ), m_position, 0.f );
	m_laserWingPlane->m_owner = this;
	m_laserWingPlane->BeginPlay();
	m_wingPlane = (WingPlane*)g_theGame->SpawnEntityToGame( EntityDefinition::GetDefinition( "WingPlane" ), m_position, 0.f );
	m_wingPlane->m_owner = this;
	m_wingPlane->BeginPlay();

	m_subWeaponRocketPtr = g_theGame->CreateWeaponComponent( WeaponDefinition::GetDefinition( "RocketLauncher" ), this );
	m_subWeaponCoinGunPtr = g_theGame->CreateWeaponComponent( WeaponDefinition::GetDefinition( "CoinGun" ), this );
	m_sprayer = g_theGame->CreateWeaponComponent( WeaponDefinition::GetDefinition( "Spray" ), this );
}

void PlayerShip::Update( float deltaTime )
{
	if (m_deathTimer->HasStartedAndNotPeriodElapsed()) {
		return;
	}
	if (m_deathTimer->HasPeriodElapsed()) {
		g_theApp->GoToAppMode( AppState::ATTRACT_MODE );
	}
	Entity::Update( deltaTime );
	if (m_invincibleTimer->HasPeriodElapsed()) {
		m_invincibleTimer->Stop();
	}

	if (m_revengeBulletTimer->HasStartedAndNotPeriodElapsed()) {
		if (m_revengeBulletShootTimer->DecrementPeriodIfElapsed()) {
			m_revengeBulletShootTimer->Start();
			Projectile* proj = m_mainWeapon->Fire( GetForwardNormal(), m_position );
			SetBulletAttributes( proj );
		}
	}

	if (m_selfDamageBulletTimer->HasStartedAndNotPeriodElapsed()) {
		if (m_selfDamageShootTimer->DecrementPeriodIfElapsed()) {
			constexpr float degreesPerBullet = 360.f / 16.f;
			float startOrientationDegrees = m_orientationDegrees;
			for (int i = 0; i < 16; i++) {
				m_mainWeapon->Fire( Vec2::MakeFromPolarDegrees( startOrientationDegrees + i * degreesPerBullet ), m_position );
			}
		}
	}
	else {
		m_selfDamageShootTimer->Stop();
	}

	if (m_revengeSpeedTimer->HasPeriodElapsed()) {
		m_damageModifier -= 0.2f;
		m_movingSpeedModifier -= 0.5f;
		m_attackSpeedModifier -= 0.5f;
		m_revengeSpeedTimer->Stop();
	}

	if (m_subWeaponShieldDashOn && m_subWeaponCoolDownTimer->GetElapsedTime() > 2.f) {
		m_subWeaponShieldDashOn = false;
		m_physicsRadius /= 3.f;
	}

	if (m_timeStopTimer->HasPeriodElapsed()) {
		m_timeStopTimer->Stop();
		g_theGame->SetTimeScaleOfAllEntity( m_def.m_faction, 1.f );
	}

	if (m_subWeaponShieldDashOn) {
		AddForce( Vec2::MakeFromPolarDegrees( m_orientationDegrees, 500.f ) );
	}
	//m_position.x = GetClamped( m_position.x, g_theGame->m_curRoom->m_bounds.m_mins.x + m_cosmeticRadius, g_theGame->m_curRoom->m_bounds.m_maxs.x - m_cosmeticRadius );
	//m_position.y = GetClamped( m_position.y, g_theGame->m_curRoom->m_bounds.m_mins.y + m_cosmeticRadius, g_theGame->m_curRoom->m_bounds.m_maxs.y - m_cosmeticRadius );
}

void PlayerShip::Render() const
{
	if (m_deathTimer->HasStartedAndNotPeriodElapsed()) {
		return;
	}
	Rgba8 color;
	if (m_subWeaponShieldDashOn) {
		color = Rgba8( 192, 192, 192 );
	}
	else if (!m_invincibleTimer->IsStopped()) {
		color = Rgba8( 102, 153, 204, 100 );
	}
	else {
		color = Rgba8( 102, 153, 204 );
	}
	Mat44 modelMatrix = GetModelMatrix();
	if (m_biggerBody) {
		modelMatrix.Append( Mat44::CreateUniformScale2D( 1.5f ) );
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( modelMatrix, color );
	g_theRenderer->DrawVertexBuffer( m_vertexBuffer, (int)(m_vertexBuffer->GetSize() / sizeof( Vertex_PCU )), 0 );
}

void PlayerShip::Die()
{
	if (g_theApp->m_playerNoDie) {
		return;
	}
	//m_isDead = true;
	m_deathTimer->Start();
	ParticleSystem2DAddEmitter( 300, 0.5f, AABB2( m_position, m_position ),
		FloatRange( m_cosmeticRadius * 0.6f, m_cosmeticRadius ),
		AABB2( Vec2( -40.f, -40.f ) + m_velocity * 0.5f, Vec2( 40.f, 40.f ) + m_velocity * 0.5f ),
		FloatRange( 0.6f, 1.5f ), m_def.m_deathParticleColor, Particle2DShape::Asteroid, true, FloatRange( 0.f, 360.f ),
		FloatRange( 40.f, 75.f ), nullptr,
		Rgba8( m_def.m_deathParticleColor.r, m_def.m_deathParticleColor.g, m_def.m_deathParticleColor.b, 0 ) );
}

void PlayerShip::RenderUI() const
{
	std::vector<Vertex_PCU> verts;


	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void PlayerShip::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage, Vec2 const& projectileVelocity )
{
	if (m_deathTimer->HasStartedAndNotPeriodElapsed()) {
		return;
	}
	UNUSED( hitNormal );
	UNUSED( projectileVelocity );
	if (!m_invincibleTimer->IsStopped()) {
		return;
	}
	// lose health
	if (m_hasShield) {
		m_hasShield = false;
		m_shield->m_isActivated = false;
	}
	else if (directDamage || Starship_IsNearlyZero(m_curArmor)) {
		LoseHealth:
		m_health -= hit;
		if (m_loseHealthGainArmor) {
			if (GetRandGen()->RollRandomFloatZeroToOne() + m_luckiness > 0.5f) {
				m_curArmor += 1.f;
			}
		}
		if (m_demonContract && Starship_IsNearlyZero( m_health - 1.f )) {
			g_theGame->SpawnEntityToGame( EntityDefinition::GetDefinition( "LittleDemon" ), m_position + GetRandomPointOnUnitCircle2D() * 15.f, 0.f );
		}
	}
	else {
		if (m_armorHealthRandomLose) {
			float rnd = GetRandGen()->RollRandomFloatZeroToOne();
			if (rnd < 0.5f) {
				goto LoseHealth;
			}
		}
		m_curArmor -= hit;
		if (m_loseArmorGainHealth) {
			if (GetRandGen()->RollRandomFloatZeroToOne() + m_luckiness > 0.5f) {
				m_health += 1.f;
			}
		}
	}
	if (hit >= 1.f) {
		g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Hit.wav" ), false, g_theApp->m_soundVolume * 0.1f );
		g_theGame->StartCameraShake();
		m_invincibleTimer->SetPeriodSeconds( 1.f );
		m_invincibleTimer->Start();
		if (m_revengeBullet) {
			m_revengeBulletTimer->Start();
			m_revengeBulletShootTimer->Start();
		}
		if (m_revengeSpeed) {
			if (m_revengeSpeedTimer->IsStopped() || m_revengeSpeedTimer->HasPeriodElapsed()) {
				m_revengeSpeedTimer->Start();
				m_damageModifier += 0.2f;
				m_movingSpeedModifier += 0.5f;
				m_attackSpeedModifier += 0.5f;
			}
		}
		if (m_medicalInsurance) {
			((PlayerController*)m_controller)->m_reward += 10;
		}
		if (m_moreDamageMoreAttack) {
			m_vengefulAttack += 0.2f;
		}
		if (m_rerandomizeItemsWhenDamaged) {
			RerandomizeAllItems( m_rerandomizeKeepItemID );
		}
	}

	if (m_curArmor < 0.f) {
		m_health += m_curArmor;
		m_curArmor = 0.f;
	}

	if (m_health <= 0.f) {
		if (m_respawn1Health && !m_respawned1HealthBefore) {
			m_respawned1HealthBefore = true;
			m_health = 1.f;
			StartRespawnParticle();
			g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/PlayerReborn.wav" ), false, g_theApp->m_soundVolume * 0.1f );
		}
		else if (m_respawnHalfHealth && !m_respawnedHalfHealthBefore) {
			m_respawnedHalfHealthBefore = true;
			m_health = m_maxHealth / 2.f;
			StartRespawnParticle();
			g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/PlayerReborn.wav" ), false, g_theApp->m_soundVolume * 0.1f );
		}
		if (m_health <= 0.f && !g_theApp->m_playerNoDie) {
			//die
			Die();
			//g_theApp->GoToAppMode( AppState::ATTRACT_MODE );
			return;
		}
	}

	m_health = GetClamped( m_health, 0.f, m_maxHealth );
	m_curArmor = GetClamped( m_curArmor, 0.f, m_maxArmor );
}

bool PlayerShip::IsInvincible()
{
	return !m_invincibleTimer->IsStopped();
}

float PlayerShip::GetMainWeaponDamage() const
{
	float retValue = 0.f;
	if (m_movementDamage) {
		retValue = m_mainWeaponDamage - 1.f + GetMovingSpeed() / 200.f + m_vengefulAttack;
	}
	else {
		retValue = m_mainWeaponDamage + m_vengefulAttack;
	}
	if (m_damageByEmptyHealth) {
		float emptyHealth = m_maxHealth - m_health;
		retValue += (0.2f * emptyHealth);
	}
	if (!m_lowerHealthAdvantage) {
		retValue *= (1.f + m_damageModifier);
	}
	else {
		if (Starship_IsNearlyZero( m_health - 1.f )) {
			retValue *= (1.5f + m_damageModifier);
		}
		else if (Starship_IsNearlyZero( m_health - 2.f )) {
			retValue *= (1.2f + m_damageModifier);
		}
		else if (Starship_IsNearlyZero( m_health - 3.f )) {
			retValue *= (1.1f + m_damageModifier);
		}
		else {
			retValue *= (1.f + m_damageModifier);
		}
	}
	if (m_moreMoneyMakesMeBetter) {
		retValue *= (1.f + (float)(((PlayerController*)m_controller)->m_reward - 50) / 100.f);
	}
	return GetClamped( retValue, 0.1f, 10000.f );
}

bool PlayerShip::Fire( Vec2 const& forwardVec, Vec2 const& startPos, bool forceToFire /*= false */ )
{
	if (m_mainWeaponTimer->DecrementPeriodIfElapsed() || forceToFire) {
		m_mainWeaponTimer->SetPeriodSeconds( GetAttackCoolDown() );
		m_mainWeaponTimer->Start();

		// spray item
		if (m_chanceShootSpray) {
			float rnd = GetRandGen()->RollRandomFloatZeroToOne();
			if (rnd + m_luckiness > 0.95f) {
				FireSprayerWeapon( forwardVec, startPos );
			}
		}

		// ray(laser) > missile > spray
		// first: missile
		if (m_missile && !m_shootLaserInstead) {
			// toilet will override three bullets
			if (m_toilet) {
				int numOfShoots = 10;
				if (m_threeBullets) {
					numOfShoots = 30;
				}
				for (int i = 0; i < numOfShoots; i++) {
					Vec2 position = GetRandomPointInDisc2D( 50.f, m_position );
					Missle* missile = (Missle*)g_theGame->SpawnEffectToGame( EffectType::Missile, m_position );
					missile->m_startPos = m_position;
					missile->m_targetPos = position;
					missile->m_owner = this;
					missile->m_flyTime = 0.8f * 60.f / GetBulletSpeed();
					missile->m_blastTime = 0.5f * 1.5f / GetBulletLifeTime();
					missile->m_maxRadius = 10.f * GetBulletLifeTime();
					if (m_shotGun) {
						missile->m_flyTime /= 2.f;
					}
					missile->BeginPlay();
					missile->m_damage = GetMainWeaponDamage() * 3.f;
				}
			}
			else if (m_threeBullets) {
				Vec2 position = g_theGame->m_worldCamera.GetCursorWorldPosition( g_window->GetNormalizedCursorPos() );
				for (int i = 0; i < 3; i++) {
					Vec2 newPos = position + GetRandomPointInDisc2D( 5.f );
					if (m_bulletBias) {
						newPos += GetRandomPointInDisc2D( 6.f );
					}
					Missle* missile = (Missle*)g_theGame->SpawnEffectToGame( EffectType::Missile, m_position );
					missile->m_startPos = m_position;
					missile->m_targetPos = newPos;
					missile->m_owner = this;
					missile->m_flyTime = 0.8f * 60.f / GetBulletSpeed();
					missile->m_blastTime = 0.5f * 1.5f / GetBulletLifeTime();
					missile->m_maxRadius = 10.f * GetBulletLifeTime();
					if (m_shotGun) {
						missile->m_flyTime /= 2.f;
					}
					missile->BeginPlay();
					missile->m_damage = GetMainWeaponDamage() * 3.f;
				}
			}
			else {
				Vec2 position = g_theGame->m_worldCamera.GetCursorWorldPosition( g_window->GetNormalizedCursorPos() );
				if (m_bulletBias) {
					position += GetRandomPointInDisc2D( 5.f );
				}
				Missle* missile = (Missle*)g_theGame->SpawnEffectToGame( EffectType::Missile, m_position );
				missile->m_startPos = m_position;
				missile->m_targetPos = position;
				missile->m_owner = this;
				missile->m_flyTime = 0.8f * 60.f / GetBulletSpeed();
				missile->m_blastTime = 0.5f * 1.5f / GetBulletLifeTime();
				missile->m_maxRadius = 10.f * GetBulletLifeTime();
				if (m_shotGun) {
					missile->m_flyTime /= 2.f;
				}
				missile->BeginPlay();
				missile->m_damage = GetMainWeaponDamage() * 3.f;
			}
			// audio
		}
		else {
			// normal bullets
			if (!m_threeBullets && !m_toilet && !m_shotGun) {
				Vec2 bulletStartPos = startPos;
				if (m_bulletBias) {
					Vec2 sideVec = forwardVec.GetRotated90Degrees();
					bulletStartPos += sideVec * GetRandGen()->RollRandomFloatInRange( -m_physicsRadius * 0.2f, m_physicsRadius * 0.2f );
				}
				if (m_shootLaserInstead) {
					FireRayWeapon( forwardVec, bulletStartPos );
				}
				else if (m_bacteriaSprayer) {
					FireSprayerWeapon( forwardVec, bulletStartPos );
				}
				else {
					FireBulletWeapon( forwardVec, bulletStartPos );
				}
			}
			// shot gun behavior
			else if (!m_toilet && m_shotGun) {
				if (!m_bacteriaSprayer || m_shootLaserInstead) {
					int numOfBullets = 12;
					if (m_threeBullets) {
						numOfBullets *= 3;
					}
					for (int i = 0; i < numOfBullets; i++) {
						Vec2 bulletStartPos = startPos;
						float orientationDegrees = GetRandGen()->RollRandomFloatInRange( m_orientationDegrees - 60.f, m_orientationDegrees + 60.f );
						Vec2 bulletForwardVec = Vec2::MakeFromPolarDegrees( orientationDegrees );
						if (m_bulletBias) {
							Vec2 sideVec = bulletForwardVec.GetRotated90Degrees();
							bulletStartPos += sideVec * GetRandGen()->RollRandomFloatInRange( -m_physicsRadius * 0.2f, m_physicsRadius * 0.2f );
						}
						if (m_shootLaserInstead) {
							FireRayWeapon( bulletForwardVec, bulletStartPos );
						}
						else {
							FireBulletWeapon( bulletForwardVec, bulletStartPos );
						}
					}
				}
				else if (m_bacteriaSprayer) {
					FireSprayerWeapon( forwardVec, m_position, true );
				}
			}
			// three bullets behavior
			else if (!m_toilet && m_threeBullets) {
				float orientationDegrees = forwardVec.GetOrientationDegrees();
				for (int i = 0; i < 3; i++) {
					Vec2 bulletStartPos = startPos;
					Vec2 bulletForwardVec = Vec2::MakeFromPolarDegrees( orientationDegrees + (i - 1) * 15.f );
					if (m_bulletBias) {
						Vec2 sideVec = bulletForwardVec.GetRotated90Degrees();
						bulletStartPos += sideVec * GetRandGen()->RollRandomFloatInRange( -m_physicsRadius * 0.2f, m_physicsRadius * 0.2f );
					}
					if (m_shootLaserInstead) {
						FireRayWeapon( bulletForwardVec, bulletStartPos );
					}
					else if (m_bacteriaSprayer) {
						FireSprayerWeapon( bulletForwardVec, bulletStartPos );
					}
					else {
						FireBulletWeapon( bulletForwardVec, bulletStartPos );
					}
				}
			}
			// toilet(random bullets) behavior
			else if (m_toilet) {
				int numOfBullets = 6;
				if (m_shotGun) {
					numOfBullets = 9;
				}
				if (m_threeBullets) {
					numOfBullets = 15;
				}
				for (int i = 0; i < numOfBullets; i++) {
					Vec2 bulletStartPos = startPos;
					float orientationDegrees = GetRandGen()->RollRandomFloatInRange( 0.f, 360.f );
					Vec2 bulletForwardVec = Vec2::MakeFromPolarDegrees( orientationDegrees );
					if (m_bulletBias) {
						Vec2 sideVec = bulletForwardVec.GetRotated90Degrees();
						bulletStartPos += sideVec * GetRandGen()->RollRandomFloatInRange( -m_physicsRadius * 0.2f, m_physicsRadius * 0.2f );
					}
					if (m_shootLaserInstead) {
						FireRayWeapon( bulletForwardVec, bulletStartPos );
					}
					else if (m_bacteriaSprayer) {
						FireSprayerWeapon( bulletForwardVec, bulletStartPos );
					}
					else {
						FireBulletWeapon( bulletForwardVec, bulletStartPos );
					}
				}
			}
			if (m_shootLaserInstead) {
				g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Laser.wav" ), false, g_theApp->m_soundVolume * 0.1f );
			}
			else if (m_bacteriaSprayer) {

			}
			else {
				//g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/PlayerShoot.wav" ), false, g_theApp->m_soundVolume * 0.6f );
			}
		}
		
		return true;
	}
	return false;
}

void PlayerShip::DoDash( Vec2 const& direction )
{
	if (!m_dashTimer->IsStopped() && !m_dashTimer->HasPeriodElapsed()) {
		return;
	}
	m_dashTimer->SetPeriodSeconds( GetDashingCoolDown() );
	m_dashTimer->Start();
	AddImpulse( direction * GetDashingDist() );
	if ((m_invincibleTimer->HasStartedAndNotPeriodElapsed() && m_invincibleTimer->GetElapsedTime() >= m_invincibleTimer->GetPeriodSeconds() - 0.35f)
		|| m_invincibleTimer->HasPeriodElapsed() || m_invincibleTimer->IsStopped()) {
		m_invincibleTimer->SetPeriodSeconds( 0.35f );
		m_invincibleTimer->Start();
	}

	/*ParticleSystem2DAddEmitter(300, 0.04f,
		AABB2( m_position - GetForwardNormal() * m_cosmeticRadius, m_position + GetForwardNormal() * m_cosmeticRadius ),
		FloatRange( m_cosmeticRadius * 0.4f, m_cosmeticRadius * 0.6f ),
		AABB2( GetForwardNormal() * m_velocity.GetLength() * 0.5f, GetForwardNormal() * m_velocity.GetLength() * 0.8f ),
		FloatRange( 0.2f, 0.3f ), Rgba8( 255, 255, 255 ), Particle2DShape::Box, true, FloatRange( 0.f, 0.f ),
		FloatRange( 0.f, 0.f ), nullptr, EQUAL_TO_START_COLOR, 0.f, 0.f );*/

}

bool PlayerShip::IsDashing() const
{
	if (!m_dashTimer->IsStopped() && m_dashTimer->GetElapsedTime() < 0.25f) {
		return true;
	}
	return false;
}

void PlayerShip::PerformSkill()
{
	if (m_skillItemID == -1) {
		return;
	}
	ItemDefinition& def = ItemDefinition::GetDefinition( m_skillItemID );
	if (m_skillSelfDamage) {
		if (!m_invincibleTimer->HasStartedAndNotPeriodElapsed()) {
			BeAttacked( 1.f, Vec2( 0.f, 0.f ), true );
			m_selfDamageBulletTimer->Start();
			m_selfDamageShootTimer->Start();
		}
	}
	else if (m_shiningShield) {
		if (def.CanBeUsed()) {
			def.AddCharge( -1 );
			m_hasShield = true;
			m_shield->m_isActivated = true;
		}
	}
	else if (m_itemChooseAdd1Health) {
		g_theGame->TransferItemToMaxHealth( m_position );
	}
	else if (m_selfHurtMachine) {
		if (!m_invincibleTimer->HasStartedAndNotPeriodElapsed()) {
			BeAttacked( 1.f, Vec2( 0.f, 0.f ), false );
			int rndCoin = GetRandGen()->RollRandomIntInRange( 5, 14 );
			for (int i = 0; i < rndCoin; i++) {
				g_theGame->SpawnEffectToGame( EffectType::Reward, m_position );
			}
		}
	}
	else if (m_attackEnhancement) {
		PlayerController* controller = (PlayerController*)m_controller;
		if (controller->m_reward >= 20) {
			m_damageModifier += 0.1f;
			controller->m_reward -= 20;
		}
	}
	else if (m_bribe) {
		if (def.CanBeUsed() && ((PlayerController*)m_controller)->m_reward >= 100) {
			Entity* enemy = g_theGame->GetNearestEnemy( m_position, FLT_MAX );
			if (enemy && enemy->m_def.m_enemyLevel != 5) {
				enemy->Die();
				def.AddCharge( -1 );
			}
		}
	}
	else if (m_armorGenerator) {
		if (def.CanBeUsed() && m_curArmor != m_maxArmor) {
			def.AddCharge( -1 );
			GainArmor( 1.f );
		}
	}
	else if (m_timeStop) {
		if (def.CanBeUsed()) {
			def.AddCharge( -1 );
			g_theGame->SetTimeScaleOfAllEntity( m_def.m_faction, 0.f );
			m_timeStopTimer->Start();
		}
	}
	else if (m_healthToMaxArmor) {
		if (def.CanBeUsed()) {
			if (!m_invincibleTimer->HasStartedAndNotPeriodElapsed()) {
				def.AddCharge( -1 );
				BeAttacked( 1.f, Vec2(), true );
				GainMaxArmor( 1.f );
			}
		}
	}
	else if (m_damageTrap) {
		if (def.CanBeUsed()) {
			def.AddCharge( -1 );
			goto L1;
		}
		else if (((PlayerController*)m_controller)->m_reward >= 10) {
			((PlayerController*)m_controller)->m_reward -= 10;
			L1:
			SunFlame* sf = (SunFlame*)g_theGame->SpawnEffectToGame( EffectType::SunFlame, m_position );
			sf->m_faction = m_def.m_faction;
			sf->m_dist = 300.f;
			sf->m_damage = 3.f;
		}
	}
	else if (m_teleport) {
		if (def.CanBeUsed()) {
			def.AddCharge( -1 );
			m_position = g_theGame->m_worldCamera.GetCursorWorldPosition( g_window->GetNormalizedCursorPos() );
			g_theGame->DealRangeDamage( 5.f * GetMainWeaponDamage(), m_position, m_cosmeticRadius * 5.f, m_def.m_faction );
			StartRespawnParticle();
		}
	}
	else if (m_itemDice) {
		if (def.CanBeUsed()) {
			def.AddCharge( -1 );
			g_theGame->RerandomizeAllItemsInRoom();
		}
	}
	else if (m_generateShop) {
		if (def.CanBeUsed()) {
			def.AddCharge( -1 );
			g_theGame->GenerateNewShopToLevel();
		}
	}

	if (m_healthToCharge && def.m_hasCharge && def.m_charge == 0 && m_invincibleTimer->IsStopped()) {
		def.m_charge += 1;
		BeAttacked( 2.f, Vec2() );
	}
}

bool PlayerShip::FireMainWeapon()
{
	return Fire( GetForwardNormal(), m_position );
}

void PlayerShip::FireSubWeapon()
{
	if (m_subWeaponCoolDownTimer->HasPeriodElapsed()) {
		if (m_subWeaponShieldDash) {
			AddImpulse( Vec2::MakeFromPolarDegrees( m_orientationDegrees, 50.f ), false );
			m_subWeaponShieldDashOn = true;
			m_physicsRadius *= 3.f;
			m_subWeaponCoolDownTimer->Start();
		}
		else if (m_subWeaponLaser) {
			Entity* target = g_theGame->GetNearestEnemy( m_position, 50.f );
			if (target) {
				StarshipLaser* rayEffect = (StarshipLaser*)g_theGame->SpawnEffectToGame( EffectType::SubWeaponLaser, m_position, m_orientationDegrees );
				rayEffect->m_owner = this;
				rayEffect->m_rayEndPos = target->m_position;
				rayEffect->m_physicsRadius = 2.f * m_physicsRadius;
				rayEffect->m_target = target;
				rayEffect->m_damage = GetMainWeaponDamage();
				rayEffect->BeginPlay();
				m_subWeaponCoolDownTimer->Start();
			}
		}
		else if (m_subWeaponRocket) {
			m_subWeaponRocketPtr->Fire( GetForwardNormal(), m_position );
			m_subWeaponCoolDownTimer->Start();
			AddImpulse( -GetForwardNormal() * 100.f );
			g_theGame->SetOneDirCameraShake( -GetForwardNormal() * 0.6f, 0.05f );
		}
		else if (m_subWeaponExplosive) {
			StarshipMine* mine = (StarshipMine*)g_theGame->SpawnEffectToGame( EffectType::StarshipMine, m_position );
			m_subWeaponCoolDownTimer->Start();
			mine->m_damage = GetMainWeaponDamage() * 5.f;
			mine->m_blastTime = 1.f;
			mine->m_explosionRadius = 13.f;
			mine->BeginPlay();
		}
		else if (m_subWeaponCoinGun && ((PlayerController*)m_controller)->m_reward >= 1) {
			CoinBullet* coin = (CoinBullet*)m_subWeaponCoinGunPtr->Fire( GetForwardNormal(), m_position );
			coin->m_damage = GetMainWeaponDamage() * 5.f;
			m_subWeaponCoolDownTimer->Start();
			((PlayerController*)m_controller)->m_reward -= 1;
		}
		else if (m_subWeaponElectricChain) {
			m_subWeaponCoolDownTimer->Start();
			ElectricChain* chain = (ElectricChain*)g_theGame->SpawnEffectToGame( EffectType::ElectricChain, m_position );
			chain->m_damage = GetMainWeaponDamage() * 0.5f;
			chain->m_maxDist = GetBulletSpeed() * GetBulletLifeTime() * 0.3f;
			chain->m_owner = this;
			chain->BeginPlay();
		}
	}
}

void PlayerShip::Interact()
{
	g_theGame->PickUpItem( m_position );

}

void PlayerShip::GainMaxHealth( float maxHealth )
{
	m_maxHealth += maxHealth;
	m_maxHealthModifier += maxHealth;
	maxHealth = maxHealth >= 1.f ? maxHealth : 1.f;
	m_maxHealthModifier = m_maxHealthModifier >= 0.f ? m_maxHealthModifier : 0.f;
	if (maxHealth > 0.f) {
		m_health += maxHealth;
	}
	m_health = GetClamped( m_health, 0.f, m_maxHealth );
	if (m_health == 0.f) {
		Die();
	}
}

void PlayerShip::GainMaxArmor( float maxArmor )
{
	m_maxArmor += maxArmor;
	m_maxArmorModifier += maxArmor;
	m_maxArmor = m_maxArmor >= 0.f ? m_maxArmor : 0.f;
	m_maxArmorModifier = m_maxArmorModifier >= 0.f ? m_maxArmorModifier : 0.f;
	if (maxArmor > 0.f) {
		m_curArmor += maxArmor;
	}
	m_curArmor = GetClamped( m_curArmor, 0.f, m_maxArmor );
}

void PlayerShip::GainHealth( float health )
{
	m_health += health;
	m_health = GetClamped( m_health, 0.f, m_maxHealth );
}

void PlayerShip::GainArmor( float armor )
{
	m_curArmor += armor;
	m_curArmor = GetClamped( m_curArmor, 0.f, m_maxArmor );
}

void PlayerShip::GainItem( int itemID )
{
	if (HasItem( itemID )) {
		return;
		//ERROR_AND_DIE( "Cannot have two items at the same time!" );
	}

	ItemDefinition& itemDef = ItemDefinition::GetDefinition( itemID );
	if (itemDef.m_category == "Skill" && m_skillItemID != -1) {
		// exchange skill
		int itemToDrop = m_skillItemID;
		LoseItem( itemToDrop );
		ItemDefinition& itemToDropDef = ItemDefinition::GetDefinition( itemToDrop );
		itemToDropDef.m_status = ItemStatus::In_Room;
		itemToDropDef.m_isThrowAwayItem = true;
		itemToDropDef.m_position = m_position;
		g_theGame->m_isChoosingItems = true;
		g_theGame->m_showingItems.push_back( itemToDrop );
	}
	else if (itemDef.m_category == "SubWeapon" && m_subWeaponItemID != -1) {
		// exchange sub-weapon
		int itemToDrop = m_subWeaponItemID;
		LoseItem( itemToDrop );
		ItemDefinition& itemToDropDef = ItemDefinition::GetDefinition( itemToDrop );
		itemToDropDef.m_status = ItemStatus::In_Room;
		itemToDropDef.m_isThrowAwayItem = true;
		itemToDropDef.m_position = m_position;
		g_theGame->m_isChoosingItems = true;
		g_theGame->m_showingItems.push_back( itemToDrop );
	}
	GainMaxHealth( itemDef.m_maxHealthModifier );
	GainMaxArmor( itemDef.m_maxArmorModifier );
	GainHealth( itemDef.m_recoverHealth );
	m_damageModifier += itemDef.m_damageModifier;
	m_attackSpeedModifier += itemDef.m_attackSpeedModifier;
	m_movingSpeedModifier += itemDef.m_movingSpeedModifier;
	m_bulletSpeedModifier += itemDef.m_bulletSpeedModifier;
	m_bulletLifeTimeModifier += itemDef.m_bulletLifeTimeModifier;
	m_dashingCoolDownModifier += itemDef.m_dashingCoolDownModifier;
	m_dashingDistanceModifier += itemDef.m_dashingDistanceModifier;
	ItemDefinition::SetItemAvailability( itemID, false );
	itemDef.m_status = ItemStatus::In_Player_Hands;
	SetItemEffect( itemDef.m_type, true );
	AddItemToItemList( itemID );
	if (itemDef.m_category == "Skill") {
		m_skillItemID = itemID;
	}
	else if (itemDef.m_category == "SubWeapon") {
		m_subWeaponItemID = itemID;
	}
	if (itemDef.m_type == "RerandomizeItemsWhenDamaged") {
		m_rerandomizeKeepItemID = itemID;
	}
}

void PlayerShip::LoseItem( int itemID )
{
	if (HasItem( itemID )) {
		ItemDefinition const& itemDef = ItemDefinition::GetDefinition( itemID );
		GainMaxHealth( -itemDef.m_maxHealthModifier );
		GainMaxArmor( -itemDef.m_maxArmorModifier );
		m_damageModifier -= itemDef.m_damageModifier;
		m_attackSpeedModifier -= itemDef.m_attackSpeedModifier;
		m_movingSpeedModifier -= itemDef.m_movingSpeedModifier;
		m_bulletSpeedModifier -= itemDef.m_bulletSpeedModifier;
		m_bulletLifeTimeModifier -= itemDef.m_bulletLifeTimeModifier;
		m_dashingCoolDownModifier -= itemDef.m_dashingCoolDownModifier;
		m_dashingDistanceModifier -= itemDef.m_dashingDistanceModifier;
		SetItemEffect( itemDef.m_type, false );
		RemoveItemFromItemList( itemID );
		ItemDefinition::SetItemAvailability( itemID, true );
		if (itemDef.m_category == "Skill") {
			m_skillItemID = -1;
		}
		else if (itemDef.m_category == "SubWeapon") {
			m_subWeaponItemID = -1;
		}
	}
	else {
		return;
		//ERROR_RECOVERABLE( "Trying to remove unowned item!" );
	}
}

bool PlayerShip::HasItem( int itemID ) const
{
	if ((int)m_itemList.size() == 0) {
		return false;
	}
	int index = (int)m_itemList.size() / 2;
	int leftBound = 0;
	int rightBound = (int)m_itemList.size() - 1;
	do {
		if (m_itemList[index] == itemID) {
			return true;
		}
		else if (m_itemList[index] < itemID) {
			leftBound = index + 1;
		}
		else {
			rightBound = index - 1;
		}
		index = (leftBound + rightBound) / 2;
	} while (leftBound <= rightBound);
	return false;
}

float PlayerShip::GetAttackCoolDown() const
{
	float retValue = 0.f;
	if (!m_lowerHealthAdvantage) {
		retValue = GetClamped( (1.f - m_attackSpeedModifier), 0.1f, 10.f ) * m_def.m_shootCoolDown;
	}
	else {
		if (Starship_IsNearlyZero( m_health - 1.f )) {
			retValue = GetClamped( (0.8f - m_attackSpeedModifier), 0.1f, 10.f ) * m_def.m_shootCoolDown;
		}
		else if (Starship_IsNearlyZero( m_health - 2.f )) {
			retValue = GetClamped( (0.9f - m_attackSpeedModifier), 0.1f, 10.f ) * m_def.m_shootCoolDown;
		}
		else if (Starship_IsNearlyZero( m_health - 3.f )) {
			retValue = GetClamped( (0.95f - m_attackSpeedModifier), 0.1f, 10.f ) * m_def.m_shootCoolDown;
		}
		else {
			retValue = GetClamped( (1.f - m_attackSpeedModifier), 0.1f, 10.f ) * m_def.m_shootCoolDown;
		}
	}
	if (m_shootLaserInstead) {
		retValue = retValue < RAY_MAX_TIME ? RAY_MAX_TIME : retValue;
	}
	return GetClamped( retValue, 0.01f, 100.f );
}

float PlayerShip::GetBulletPerSecond() const
{
	return 1.f / GetAttackCoolDown();
}

float PlayerShip::GetMovingSpeed() const
{
	if (!m_lowerHealthAdvantage) {
		return PLAYER_BASIC_SPEED * (1.f + m_movingSpeedModifier);
	}
	else {
		if (Starship_IsNearlyZero( m_health - 1.f )) {
			return PLAYER_BASIC_SPEED * (1.15f + m_movingSpeedModifier);
		}
		else if (Starship_IsNearlyZero( m_health - 2.f )) {
			return PLAYER_BASIC_SPEED * (1.1f + m_movingSpeedModifier);
		}
		else if (Starship_IsNearlyZero( m_health - 3.f )) {
			return PLAYER_BASIC_SPEED * (1.05f + m_movingSpeedModifier);
		}
		else {
			return PLAYER_BASIC_SPEED * (1.f + m_movingSpeedModifier);
		}
	}
}

float PlayerShip::GetBulletSpeed() const
{
	float speed = m_mainWeapon->GetProjectileDef().m_speed * (1 + m_bulletSpeedModifier);
	return GetClamped( speed, 1.f, 10000.f );
}

float PlayerShip::GetBulletLifeTime() const
{
	return GetClamped(m_mainWeapon->GetProjectileDef().m_lifeSeconds * (1 + m_bulletLifeTimeModifier), 0.1f, 10.f);
}

float PlayerShip::GetDashingCoolDown() const
{
	return m_dashingCooldown * (1.f - m_dashingCoolDownModifier);
}

float PlayerShip::GetDashingDist() const
{
	return 150.f * (1.f + m_dashingDistanceModifier);
}

void PlayerShip::GoToNextLevel()
{
	if (m_skillItemID != -1) {
		ItemDefinition& def = ItemDefinition::GetDefinition( m_skillItemID );
		def.AddCharge( def.m_chargePerLevel );
	}
	m_vengefulAttack = 0.f;
	CorrectAllFollowers();
}

void PlayerShip::CorrectAllFollowers()
{
	m_diagonalRetinue->m_position = m_position - GetForwardNormal() * m_cosmeticRadius * 4.f;
	m_laserWingPlane->m_position = m_position - GetForwardNormal() * m_cosmeticRadius * 4.f;
	m_wingPlane->m_position = m_position - GetForwardNormal() * m_cosmeticRadius * 4.f;
}

void PlayerShip::SetBulletAttributes( Projectile* proj )
{
	if (proj) {
		if (m_spearBullet) {
			proj->m_isPuncturing = true;
		}
		if (m_rangeDamage) {
			proj->m_hasRangeDamage = true;
			proj->m_rangeDamageRadius = proj->m_physicsRadius * 5.f;
			proj->m_rangeDamagePercentage = 1.f;
		}
		if (m_poisonousBullet) {
			proj->m_isPoisonous = true;
			proj->m_poisonTime = 2.f;
		}
		if (m_iceBullet) {
			proj->m_isIced = true;
			proj->m_slowTime = 2.f;
		}
		if (m_biggerBullet) {
			proj->m_physicsRadius *= 2.f;
			proj->m_scale *= 2.f;
		}
		if (m_bulletAddDamageByLifeTime) {
			proj->m_addDamageByLifeTime = true;
		}
		proj->m_velocity *= (1.f + m_bulletSpeedModifier);
		proj->m_lifeTimer->SetPeriodSeconds( proj->m_lifeTimer->GetPeriodSeconds() * GetClamped( (1.f + m_bulletLifeTimeModifier), 0.1f, 100.f ) );
	}
}

void PlayerShip::AddItemToItemList( int id )
{
	for (int i = 0; i < (int)m_itemList.size(); i++) {
		if (m_itemList[i] > id) {
			m_itemList.insert( m_itemList.begin() + i, id );
			return;
		}
	}
	m_itemList.push_back( id );
}

void PlayerShip::RemoveItemFromItemList( int id )
{
	for (int i = 0; i < (int)m_itemList.size(); i++) {
		if (m_itemList[i] == id) {
			m_itemList.erase( m_itemList.begin() + i );
			return;
		}
	}
}

void PlayerShip::SetItemEffect( std::string const& type, bool isGetItem )
{
	if (type == "HealthToCharge") {
		m_healthToCharge = isGetItem;
	}
	else if (type == "ArmorHealthRandomLose") {
		m_armorHealthRandomLose = isGetItem;
	}
	else if (type == "Retinue") {
		m_diagonalRetinue->m_isActivated = isGetItem;
		m_diagonalRetinue->m_isInvincible = !isGetItem;
		if (isGetItem) {
			m_diagonalRetinue->m_position = m_position - GetForwardNormal() * m_cosmeticRadius * 4.f;
		}
	}
	else if (type == "WingPlane") {
		m_wingPlane->m_isActivated = isGetItem;
		m_wingPlane->m_isInvincible = !isGetItem;
		if (isGetItem) {
			m_wingPlane->m_position = m_position - GetForwardNormal() * m_cosmeticRadius * 4.f;
		}
	}
	else if (type == "LaserWingPlane") {
		m_laserWingPlane->m_isActivated = isGetItem;
		m_laserWingPlane->m_isInvincible = !isGetItem;
		if (isGetItem) {
			m_laserWingPlane->m_position = m_position - GetForwardNormal() * m_cosmeticRadius * 4.f;
		}
	}
	else if (type == "Random Stats") {
		if (isGetItem) {
			float rnd = GetRandGen()->RollRandomFloatZeroToOne();
			if (rnd < 0.5f) {
				m_damageModifier += 0.3f;
			}
			else if (rnd < 0.7f) {
				m_damageModifier -= 0.3f;
			}
			rnd = GetRandGen()->RollRandomFloatZeroToOne();
			if (rnd < 0.5f) {
				m_attackSpeedModifier += 0.1f;
			}
			else if (rnd < 0.7f) {
				m_attackSpeedModifier -= 0.1f;
			}
			rnd = GetRandGen()->RollRandomFloatZeroToOne();
			if (rnd < 0.5f) {
				m_movingSpeedModifier += 0.05f;
			}
			else if (rnd < 0.7f) {
				m_movingSpeedModifier -= 0.05f;
			}
			rnd = GetRandGen()->RollRandomFloatZeroToOne();
			if (rnd < 0.5f) {
				m_bulletSpeedModifier += 0.05f;
			}
			else if (rnd < 0.7f) {
				m_bulletSpeedModifier -= 0.05f;
			}
			rnd = GetRandGen()->RollRandomFloatZeroToOne();
			if (rnd < 0.5f) {
				m_bulletLifeTimeModifier += 0.1f;
			}
			else if (rnd < 0.7f) {
				m_bulletLifeTimeModifier -= 0.1f;
			}
			rnd = GetRandGen()->RollRandomFloatZeroToOne();
			if (rnd < 0.5f) {
				GainMaxHealth( 1.f );
			}
			else if (rnd < 0.7f) {
				GainMaxHealth( -1.f );
			}
			rnd = GetRandGen()->RollRandomFloatZeroToOne();
			if (rnd < 0.5f) {
				GainMaxArmor( 1.f );
			}
			else if (rnd < 0.7f) {
				if (m_maxArmor >= 1.f) {
					GainMaxArmor( -1.f );
				}
			}
		}
	}
	else if (type == "ItemAutoRefresh") {
		m_itemAutoRefresh = isGetItem;
	}
	else if (type == "GenerateShop") {
		m_generateShop = isGetItem;
	}
	else if (type == "RerandomizeItemsWhenDamaged") {
		m_rerandomizeItemsWhenDamaged = isGetItem;
	}
	else if (type == "ChanceShootSpray") {
		m_chanceShootSpray = isGetItem;
	}
	else if (type == "DamageByEmptyHealth") {
		m_damageByEmptyHealth = isGetItem;
	}
	else if (type == "SubWeaponElectricChain") {
		m_subWeaponElectricChain = isGetItem;
		if (isGetItem) {
			m_subWeaponCoolDownTimer->SetPeriodSeconds( 2.f );
			m_subWeaponCoolDownTimer->Start();
			m_subWeaponCoolDownTimer->SetElapsedTime( 2.f );
		}
	}
	else if (type == "Teleport") {
		m_teleport = isGetItem;
	}
	else if (type == "ItemDice") {
		m_itemDice = isGetItem;
	}
	else if (type == "IceBullet") {
		m_iceBullet = isGetItem;
	}
	else if (type == "DamageTrap") {
		m_damageTrap = isGetItem;
	}
	else if (type == "BulletAddDamageLifeTime") {
		m_bulletAddDamageByLifeTime = isGetItem;
	}
	else if (type == "Shotgun") {
		m_shotGun = isGetItem;
	}
	else if (type == "Missile") {
		m_missile = isGetItem;
	}
	else if (type == "RichCanDoAnything") {
		m_richCanDoAnything = isGetItem;
	}
	else if (type == "5pickup") {
		if (isGetItem) {
			for (int i = 0; i < 5; i++) {
				float rnd = GetRandGen()->RollRandomFloatZeroToOne();
				Vec2 pos = GetRandomPointOnUnitCircle2D() * m_cosmeticRadius * 3.f + m_position;
				if (rnd < 0.5f) {
					g_theGame->SpawnEffectToGame( EffectType::HealthPickup, pos );
				}
				else {
					g_theGame->SpawnEffectToGame( EffectType::ArmorPickup, pos );
				}
			}
		}
	}
	else if (type == "HealthToMaxArmor") {
		m_healthToMaxArmor = isGetItem;
	}
	else if (type == "ThreeBullets") {
		m_threeBullets = isGetItem;
	}
	else if (type == "BulletBias") {
		m_bulletBias = isGetItem;
	}
	else if (type == "SpearBullet") {
		m_spearBullet = isGetItem;
	}
	else if (type == "RangeBullet") {
		m_rangeDamage = isGetItem;
	}
	else if (type == "RevengeBullet") {
		m_revengeBullet = isGetItem;
	}
	else if (type == "SelfDamage") {
		m_skillSelfDamage = isGetItem;
	}
	else if (type == "Respawn1Health") {
		m_respawn1Health = isGetItem;
	}
	else if (type == "RespawnHalfHealth") {
		m_respawnHalfHealth = isGetItem;
	}
	else if (type == "LoseHealthGainArmor") {
		m_loseHealthGainArmor = isGetItem;
	}
	else if (type == "LoseArmorGainHealth") {
		m_loseArmorGainHealth = isGetItem;
	}
	else if (type == "RevengeSpeed") {
		m_revengeSpeed = isGetItem;
	}
	else if (type == "ChooseLevel1") {
		if (isGetItem) {
			std::vector<int> allItems;
			ItemDefinition::GetAllItemsInPool( 1, allItems );
			if ((int)allItems.size() > 0) {
				g_theGame->EnterChooseItemMode();
				for (auto i : allItems) {
					g_theGame->AddItemToChoose( i );
				}
			}
		}
	}
	else if (type == "ChooseLevel2") {
		if (isGetItem) {
			std::vector<int> allItems;
			ItemDefinition::GetAllItemsInPool( 2, allItems );
			if ((int)allItems.size() > 0) {
				g_theGame->EnterChooseItemMode();
				for (auto i : allItems) {
					g_theGame->AddItemToChoose( i );
				}
			}
		}
	}
	else if (type == "ChooseLevel3") {
		if (isGetItem) {
			std::vector<int> allItems;
			ItemDefinition::GetAllItemsInPool( 3, allItems );
			if ((int)allItems.size() > 0) {
				g_theGame->EnterChooseItemMode();
				for (auto i : allItems) {
					g_theGame->AddItemToChoose( i );
				}
			}
		}
	}
	else if (type == "ChooseSkill") {
		if (isGetItem) {
			std::vector<int> allItems;
			ItemDefinition::GetAvailableSkillItem( allItems );
			if ((int)allItems.size() > 0) {
				g_theGame->EnterChooseItemMode();
				for (auto i : allItems) {
					g_theGame->AddItemToChoose( i );
				}
			}
		}
	}
	else if (type == "ChooseSubWeapon") {
		if (isGetItem) {
			std::vector<int> allItems;
			ItemDefinition::GetAvailableSubWeaponItem( allItems );
			if ((int)allItems.size() > 0) {
				g_theGame->EnterChooseItemMode();
				for (auto i : allItems) {
					g_theGame->AddItemToChoose( i );
				}
			}
		}
	}
	else if (type == "ShiningShield") {
		m_shiningShield = isGetItem;
	}
	else if (type == "LowerHealthAdvantage") {
		m_lowerHealthAdvantage = isGetItem;
	}
	else if (type == "RandomAllItems") {
		if (isGetItem) {
			RerandomizeAllItems( m_rerandomizeKeepItemID );
		}
	}
	else if (type == "UpgradeItem") {
		m_upgradeItem = isGetItem;
		m_luckiness += 0.01f;
	}
	else if (type == "ItemChooseAdd1Health") {
		m_itemChooseAdd1Health = isGetItem;
	}
	else if (type == "SubWeaponShieldDash") {
		m_subWeaponShieldDash = isGetItem;
		if (isGetItem) {
			m_subWeaponCoolDownTimer->SetPeriodSeconds( 15.f );
			m_subWeaponCoolDownTimer->Start();
			m_subWeaponCoolDownTimer->SetElapsedTime( 15.f );
		}
	}
	else if (type == "SubWeaponLaser") {
		m_subWeaponLaser = isGetItem;
		if (isGetItem) {
			m_subWeaponCoolDownTimer->SetPeriodSeconds( 2.5f );
			m_subWeaponCoolDownTimer->Start();
			m_subWeaponCoolDownTimer->SetElapsedTime( 2.5f );
		}
	}
	else if (type == "BlindWithItem") {
		m_blindWithItem = isGetItem;
		ItemDefinition* def = ItemDefinition::GetRandomDefinition( 4 );
		if (def) {
			GainItem( def->m_id );
		}
	}
	else if (type == "Coins") {
		if (isGetItem) {
			for (int i = 0; i < 50; i++) {
				g_theGame->SpawnEffectToGame( EffectType::Reward, m_position );
			}
		}
	}
	else if (type == "BaseAttack") {
		if (isGetItem) {
			m_mainWeaponDamage += 0.2f;
		}
		else {
			m_mainWeaponDamage -= 0.2f;
		}
	}
	else if (type == "BloodThirst") {
		if (isGetItem) {
			m_mainWeaponDamage *= 0.5f;
		}
		m_bloodThirst = isGetItem;
	}
	else if (type == "CustomerCard") {
		m_customerCard = isGetItem;
	}
	else if (type == "DiscountCard") {
		m_discountCard = isGetItem;
	}
	else if (type == "SelfHurtMachine") {
		m_selfHurtMachine = isGetItem;
	}
	else if (type == "MoreMoneyShopBonus") {
		m_moreMoneyShopBonus = isGetItem;
	}
	else if (type == "AttackEnhancement") {
		m_attackEnhancement = isGetItem;
	}
	else if (type == "Asteroid") {
		m_asteroid->m_isInvincible = !isGetItem;
		m_asteroid->m_isActivated = isGetItem;
	}
	else if (type == "SubWeaponRocket") {
		m_subWeaponRocket = isGetItem;
		if (isGetItem) {
			m_subWeaponCoolDownTimer->SetPeriodSeconds( 2.5f );
			m_subWeaponCoolDownTimer->Start();
			m_subWeaponCoolDownTimer->SetElapsedTime( 2.5f );
		}
	}
	else if (type == "Bribe") {
		m_bribe = isGetItem;
	}
	else if (type == "PoisonousBullet") {
		m_poisonousBullet = isGetItem;
	}
	else if (type == "BlindWithRoom") {
		m_blindWithRoom = isGetItem;
	}
	else if (type == "SubWeaponExplosive") {
		m_subWeaponExplosive = isGetItem;
		if (isGetItem) {
			m_subWeaponCoolDownTimer->SetPeriodSeconds( 5.f );
			m_subWeaponCoolDownTimer->Start();
			m_subWeaponCoolDownTimer->SetElapsedTime( 5.f );
		}
	}
	else if (type == "DemonContract") {
		m_demonContract = isGetItem;
	}
	else if (type == "MedicalInsurance") {
		m_medicalInsurance = isGetItem;
	}
	else if (type == "Toilet") {
		m_toilet = isGetItem;
	}
	else if (type == "BiggerBullet") {
		m_biggerBullet = isGetItem;
	}
	else if (type == "MoreMoneyMakesMeBetter") {
		m_moreMoneyMakesMeBetter = isGetItem;
	}
	else if (type == "MoreDamageMoreAttack") {
		if (isGetItem) {
			m_moreDamageMoreAttack = true;
			m_vengefulAttack = 0.f;
		}
		else {
			m_moreDamageMoreAttack = false;
			m_vengefulAttack = 0.f;
		}
	}
	else if (type == "MainWeaponLaser") {
		m_shootLaserInstead = isGetItem;
	}
	else if (type == "MovementDamage") {
		m_movementDamage = isGetItem;
	}
	else if (type == "ArmorToHealth") {
		if (isGetItem) {
			float maxArmor = m_maxArmor;
			GainMaxArmor( -m_maxArmor );
			GainMaxHealth( maxArmor );
		}
	}
	else if (type == "ArmorGenerator") {
		m_armorGenerator = isGetItem;
	}
	else if (type == "BiggerBody") {
		m_biggerBody = isGetItem;
		if (isGetItem) {
			m_physicsRadius *= 1.5f;
			m_cosmeticRadius *= 1.5f;
		}
		else {
			m_physicsRadius /= 1.5f;
			m_cosmeticRadius /= 1.5f;
		}
	}
	else if (type == "CoinGun") {
		m_subWeaponCoinGun = isGetItem;
		if (isGetItem) {
			m_subWeaponCoolDownTimer->SetPeriodSeconds( 0.8f );
			m_subWeaponCoolDownTimer->Start();
			m_subWeaponCoolDownTimer->SetElapsedTime( 0.8f );
		}
	}
	else if (type == "BecomeLighter") {
		m_becomeLighter = isGetItem;
	}
	else if (type == "BacteriaSprayer") {
		m_bacteriaSprayer = isGetItem;
	}
	else if (type == "CollisionDamage") {
		m_collisionDamage = isGetItem;
	}
	else if (type == "RecoverHealth") {
		if (isGetItem) {
			GainHealth( m_maxHealth - m_health );
		}
	}
	else if (type == "TimeStop") {
		m_timeStop = isGetItem;
	}
	else if (type == "HealthBlind") {
		m_healthBlind = isGetItem;
	}
	else if (type == "SubweaponCooldown") {
		if (isGetItem) {
			m_subWeaponClock->SetTimeScale( 2.f );
		}
		else {
			m_subWeaponClock->SetTimeScale( 1.f );
		}
	}
}

void PlayerShip::RerandomizeAllItems( int excludeItem )
{
	int numOfLevel[5] = { 0, 0, 0, 0, 0 }; // 0-4 levels
	std::vector<int> tempItemList; // duplicated item list
	tempItemList.resize( m_itemList.size() );
	memcpy( tempItemList.data(), m_itemList.data(), m_itemList.size() * sizeof(int) );
	// lose all items
	for (int i = 0; i < (int)tempItemList.size(); i++) {
		if (excludeItem != tempItemList[i]) {
			ItemDefinition& def = ItemDefinition::GetDefinition( tempItemList[i] );
			numOfLevel[def.m_pool]++;
			LoseItem( tempItemList[i] );
			def.m_charge = def.m_startCharge;
		}
	}
	// regain items
	for (int i = 0; i < 5; i++) {
		for (int k = 0; k < numOfLevel[i]; k++) {
			ItemDefinition* def;
			int count = 0;
			do {
				def = ItemDefinition::GetRandomDefinition( i );
				++count;
				if (count == 100) {
					break;
				}
				// can only have 1 skill or sub-weapon
			} while (def && ((m_skillItemID != -1 && def->m_category == "Skill") || (m_subWeaponItemID != -1 && def->m_category == "SubWeapon") || (def->m_category == "ChooseItem") || (def->m_type == "RandomAllItems")));
			if (count != 100 && def) {
				GainItem( def->m_id );
			}
		}
	}
}

void PlayerShip::StartRespawnParticle()
{
	ParticleSystem2DAddEmitter( 1000, 0.05f,
		AABB2( m_position - Vec2( m_cosmeticRadius * 5.f, m_cosmeticRadius * 5.f ), m_position + Vec2( m_cosmeticRadius * 5.f, m_cosmeticRadius * 5.f ) ),
		FloatRange( m_cosmeticRadius * 0.4f, m_cosmeticRadius * 0.8f ),
		AABB2(Vec2( m_cosmeticRadius * 5.f, m_cosmeticRadius * 5.f ), Vec2( m_cosmeticRadius * 5.f, m_cosmeticRadius * 5.f ) ),
		FloatRange( 0.3f, 0.6f ), Rgba8( 102, 153, 204, 100 ), Particle2DShape::Asteroid, true, FloatRange( 0.f, 0.f ),
		FloatRange( 0.f, 0.f ), nullptr, Rgba8( 102, 153, 204, 255 ), 0.f, 0.f );
}

void PlayerShip::FireBulletWeapon( Vec2 const& fwdVec, Vec2 const& bulletPos )
{
	Projectile* proj = m_mainWeapon->Fire( fwdVec, bulletPos );
	SetBulletAttributes( proj );
}

void PlayerShip::FireRayWeapon( Vec2 const& fwdVec, Vec2 const& startPos )
{
	PersistentRay* ray = (PersistentRay*)g_theGame->SpawnEffectToGame( EffectType::PersistentRay, startPos, fwdVec.GetOrientationDegrees() );
	ray->m_maxLength = GetBulletSpeed() * GetBulletLifeTime();
	ray->m_maxWidth = m_physicsRadius * 3.f;
	ray->m_minWidth = m_physicsRadius * 2.f;
	ray->m_owner = this;
	ray->m_lifeTimeSeconds = RAY_MAX_TIME;
	ray->m_color = Rgba8( 0, 128, 255 );
	ray->m_damage = GetMainWeaponDamage() * 0.6f;
	ray->m_damageCoolDown = 0.1f;
	ray->m_updatePositionByOwner = true;
	ray->BeginPlay();
}

void PlayerShip::FireSprayerWeapon( Vec2 const& fwdVec, Vec2 const& startPos, bool sectorAttack )
{
	if (sectorAttack) {
		((Spray*)m_sprayer)->m_isSector = true;
		((Spray*)m_sprayer)->m_sectorRangeDegrees = 120.f;
		((Spray*)m_sprayer)->m_length = GetBulletSpeed() * GetBulletLifeTime();
	}
	else {
		((Spray*)m_sprayer)->m_isSector = false;
		((Spray*)m_sprayer)->m_length = GetBulletSpeed() * GetBulletLifeTime() * 0.5f;
	}
	((Spray*)m_sprayer)->m_isPoisonous = m_poisonousBullet;
	m_sprayer->Fire( fwdVec, startPos + fwdVec * m_cosmeticRadius );
}

bool PlayerShip::Event_GetItem( EventArgs& args )
{
	if (g_theGame && g_theGame->GetPlayerEntity()) {
		PlayerShip* player = (PlayerShip*)g_theGame->GetPlayerEntity();
		int id = args.GetValue( "id", 1 );
		if (!player->HasItem( id )) {
			player->GainItem( id );
		}
	}
	return true;
}

bool PlayerShip::Event_LoseItem( EventArgs& args )
{
	if (g_theGame && g_theGame->GetPlayerEntity()) {
		PlayerShip* player = (PlayerShip*)g_theGame->GetPlayerEntity();
		int id = args.GetValue( "id", 1 );
		if (player->HasItem( id )) {
			player->LoseItem( id );
		}
	}
	return true;
}

bool PlayerShip::Event_RandItem( EventArgs& args )
{
	if (g_theGame && g_theGame->GetPlayerEntity()) {
		PlayerShip* player = (PlayerShip*)g_theGame->GetPlayerEntity();
		int pool = args.GetValue( "pool", -1 );
		ItemDefinition* def = ItemDefinition::GetRandomDefinition( pool );
		if (def) {
			player->GainItem( def->m_id );
		}
	}
	return true;
}
