#include "Game/PlayerShip.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Shield.hpp"

PlayerShip::PlayerShip( Vec2 startPos, Game* game ):Entity(startPos, game)
{
	m_type = EntityType::playerShip;
	m_velocity = Vec2( 0.f, 0.f );
	m_orientationDegrees = 0.f;
	m_accelerateVelocity = Vec2( 0.f, 0.f );
	m_physicsRadius = PLAYER_SHIP_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_SHIP_COSMETIC_RADIUS;
	m_health = PLAYER_SHIP_HEALTH;
	m_maxHealth = PLAYER_SHIP_HEALTH;
	m_color = Rgba8( 102, 153, 204, 255 );
}

PlayerShip::~PlayerShip()
{

}

void PlayerShip::Update()
{
	float deltaTime = g_theGame->m_gameClock->GetDeltaSeconds();
	if (m_isDead) {
		return;
	}
	// upgrades
	if (m_game->IsUpgraded( UpgradeType::fasterAmmoGrow )) {
		m_ammoGrowSpeed = 25.f;
	}
	if (m_game->IsUpgraded( UpgradeType::lessAmmoCost )) {
		m_ammoCostModifier = 0.8f;
	}
	if (m_game->IsUpgraded( UpgradeType::fasterSpeed )) {
		m_maxSpeed = 40.f;
	}
	if (m_game->IsUpgraded( UpgradeType::shield )) {
		m_isShieldOn = true;
	}
	if (m_game->IsUpgraded( UpgradeType::shieldLessTime )) {
		m_shieldCoolDownTime = 15.f;
	}
	if (m_game->IsUpgraded( UpgradeType::superBulletWeapon )) {
		m_superBulletWeapon = true;
	}
	if (m_game->IsUpgraded( UpgradeType::coneAttackLargerRange )) {
		m_coneAttackRange = 35.f;
	}
	if (m_game->IsUpgraded( UpgradeType::moreLightSaber )) {
		m_lightSaberAmmoCost = 100.f;
	}
	if (m_game->IsUpgraded( UpgradeType::rocketLargerRange )) {
		m_rocketAmmoCost = 40.f;
	}

	XboxController& xboxController = g_theInput->GetController( 0 );
	if (xboxController.IsConnected()) {
		m_isControllerPlugIn = true;
	}
	else {
		m_isControllerPlugIn = false;
	}
	if (m_health <= 0) {
		Die();
		return;
	}

	if (m_laserCoolDown > 0.f) {
		m_laserCoolDown -= deltaTime;
	}
	else {
		m_laserCoolDown = 0.f;
	}

	if (m_enhanceTimeLeft < 0.f) {
		m_enhanceTimeLeft = 0.f;
	}
	else if (m_enhanceTimeLeft > 0.f) {
		m_enhanceTimeLeft -= deltaTime;
	}

	m_bulletTimer -= deltaTime;

	float lightSaberShootCost = m_lightSaberAmmoCost * m_ammoCostModifier;
	float rocketShootCost = m_rocketAmmoCost * m_ammoCostModifier;
	float bulletShootCost = 6.f * m_ammoCostModifier;
	float coneShootCost = m_coneAttackAmmoCost * m_ammoCostModifier;
	m_ammoAmount += deltaTime * m_ammoGrowSpeed;
	if (m_ammoAmount > m_maxAmmo) {
		m_ammoAmount = m_maxAmmo;
	}
	// light saber start when W or X pressed
	if (m_allowLighrSaber && !m_isLightSaberStart && m_ammoAmount >= 0.5f * lightSaberShootCost && (xboxController.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_X ) || g_theInput->WasKeyJustPressed( 'W' ))) {
		if (m_enhanceTimeLeft > 0.f) {
			m_game->CreateSaber( m_position, this, 30.f, deltaTime * lightSaberShootCost );
		}
		else {
			m_game->CreateSaber( m_position, this, 20.f, deltaTime * lightSaberShootCost );
		}
		m_isLightSaberStart = true;
	}

	// shoot rocket when R or Y pressed
	else if (m_game->IsUpgraded(UpgradeType::rocketAttack) && m_ammoAmount >= rocketShootCost && (m_laserCoolDown == 0.f && (xboxController.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_Y) || g_theInput->WasKeyJustPressed( 'R' )))) {
		/*m_laserCoolDown = LASER_COOLDOWN_SECONDS;
		if (m_enhanceTimeLeft > 0.f) {
			m_game->CreateLaser( m_position, m_orientationDegrees, false, this, 2.f );
		}
		else {
			m_game->CreateLaser( m_position, m_orientationDegrees, false, this, 1.f );
		}*/
		if (m_enhanceTimeLeft > 0.f) {
			m_game->CreateRocket( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees, 30.f + m_velocity.GetLength() ), m_orientationDegrees, false, m_rocketRange + 12.f );
			m_game->CreateRocket( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees, 30.f + m_velocity.GetLength() ), m_orientationDegrees, false, m_rocketRange + 12.f );
			m_game->CreateRocket( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees, 30.f + m_velocity.GetLength() ), m_orientationDegrees, false, m_rocketRange + 12.f );
		}
		else {
			m_game->CreateRocket( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees, 5.f + m_velocity.GetLength() ), m_orientationDegrees, false, m_rocketRange );
			m_game->CreateRocket( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees, 5.f + m_velocity.GetLength() ), m_orientationDegrees, false, m_rocketRange );
		}
		m_ammoAmount -= rocketShootCost;
		g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::rocket ) );
	}

	// cone attack when E or B pressed
	else if (m_game->IsUpgraded( UpgradeType::coneAttack ) && m_ammoAmount >= coneShootCost && (xboxController.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_B ) || g_theInput->WasKeyJustPressed( 'E' ))) {
		if (m_enhanceTimeLeft > 0.f) {
			m_game->CreateCone( m_position, m_orientationDegrees, false, this, m_coneAttackRange + 10.f, CONE_APERTURE_DEGREES + 20.f );
		}
		else {
			m_game->CreateCone( m_position, m_orientationDegrees, false, this, m_coneAttackRange, CONE_APERTURE_DEGREES );
		}
		g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::coneAttack ) );
		m_ammoAmount -= coneShootCost;
	}

	// shoot bullet when A or Q pressed
	else if (m_ammoAmount >= bulletShootCost && (xboxController.IsButtonDown( XboxButtonID::XBOX_BUTTON_A ) || g_theInput->IsKeyDown( 'Q' )) && m_bulletTimer <= 0.f) {
		if (m_superBulletWeapon && m_superWeaponTimer <= 0.f) {
			m_superWeaponTimer = m_superWeaponCoolDownTime;
			// super weapon
			constexpr int bulletAngle = 10;
			for (int i = 0; i < 360 / bulletAngle; i++) {
				m_game->CreateBullet( m_position,
					Vec2::MakeFromPolarDegrees( m_orientationDegrees + i * bulletAngle, BULLET_SPEED + m_velocity.GetLength() / 2 ),
					m_orientationDegrees + i * bulletAngle, false );
			}
		}
		else if (m_enhanceTimeLeft > 0.f) {
			m_bulletTimer = 0.2f;
			constexpr float bulletAngle = 10.f;
			m_game->CreateBullet( GetNosePosition(),
				Vec2::MakeFromPolarDegrees( m_orientationDegrees, BULLET_SPEED + m_velocity.GetLength() / 2 ),
				m_orientationDegrees, false );
			/*float bulletOrientationDegrees = m_game->m_randNumGen->RollRandomFloatInRange(0.f, 360.f);
			m_game->CreateBullet( GetNosePosition(),
				Vec2::MakeFromPolarDegrees( bulletOrientationDegrees, BULLET_SPEED + m_velocity.GetLength() / 2 ),
				bulletOrientationDegrees, false );*/

			m_game->CreateBullet( GetNosePosition(),
				Vec2::MakeFromPolarDegrees( m_orientationDegrees + bulletAngle, BULLET_SPEED + m_velocity.GetLength() / 2 ),
				m_orientationDegrees + bulletAngle, false );
			m_game->CreateBullet(GetNosePosition(),
				Vec2::MakeFromPolarDegrees( m_orientationDegrees - bulletAngle, BULLET_SPEED + m_velocity.GetLength() / 2 ),
				m_orientationDegrees - bulletAngle, false );
			m_game->CreateBullet( GetNosePosition(),
				Vec2::MakeFromPolarDegrees( m_orientationDegrees - 2 * bulletAngle, BULLET_SPEED + m_velocity.GetLength() / 2 ),
				m_orientationDegrees - 2 * bulletAngle, false );
			m_game->CreateBullet( GetNosePosition(),
				Vec2::MakeFromPolarDegrees( m_orientationDegrees + 2 * bulletAngle, BULLET_SPEED + m_velocity.GetLength() / 2 ),
				m_orientationDegrees + 2 * bulletAngle, false );
		}
		else {
			m_bulletTimer = 0.2f;
			constexpr float bulletAngle = 15.f;
			m_game->CreateBullet( GetNosePosition(),
				Vec2::MakeFromPolarDegrees( m_orientationDegrees, BULLET_SPEED + m_velocity.GetLength() / 2 ),
				m_orientationDegrees, false );
			m_game->CreateBullet(GetNosePosition(),
				Vec2::MakeFromPolarDegrees( m_orientationDegrees + bulletAngle, BULLET_SPEED + m_velocity.GetLength() / 2 ),
				m_orientationDegrees + bulletAngle, false );
			m_game->CreateBullet( GetNosePosition(),
				Vec2::MakeFromPolarDegrees( m_orientationDegrees - bulletAngle, BULLET_SPEED + m_velocity.GetLength() / 2 ),
				m_orientationDegrees - bulletAngle, false );

		}
		g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::shoot ), false, 0.1f );
		m_ammoAmount -= bulletShootCost;
	}

	if (m_allowLighrSaber && m_isLightSaberStart && ((m_ammoAmount < deltaTime * lightSaberShootCost && (g_theInput->IsKeyDown( 'W' ) || xboxController.IsButtonDown( XboxButtonID::XBOX_BUTTON_X ))) || xboxController.WasButtonJustReleased( XboxButtonID::XBOX_BUTTON_X ) || g_theInput->WasKeyJustReleased( 'W' ))) {
		m_game->CreateSaber( m_position, this, 0.f, deltaTime* lightSaberShootCost );
		m_isLightSaberStart = false;
	}

	if (m_ammoAmount >= deltaTime * lightSaberShootCost && m_isLightSaberStart) {
		m_ammoAmount -= deltaTime * lightSaberShootCost;
	}

	// orientation
	if (xboxController.IsConnected()) {
		if (xboxController.GetLeftStick().GetMagnitude() != 0.f) {
			m_orientationDegrees = xboxController.GetLeftStick().GetOrientationDegrees();
		}
	}
	else {
		Vec2 mouseWorldPos = m_game->m_worldCamera.GetCursorWorldPosition( g_window->GetNormalizedCursorPos() );
		m_orientationDegrees = (mouseWorldPos - m_position).GetOrientationDegrees();
	}

	if (g_theInput->IsKeyDown( KEYCODE_LEFTMOUSE ) || g_theInput->IsKeyDown( KEYCODE_RIGHTMOUSE ) || g_theInput->IsKeyDown( 'A' )) {
		// mouse key accelerates
		m_accelerateVelocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, PLAYER_SHIP_ACCELERATION );
	}
	else {
		// joystick orientation and accelerate
		Vec2 leftJoystickPos = xboxController.GetLeftStick().GetPosition();
		m_accelerateVelocity = leftJoystickPos * PLAYER_SHIP_ACCELERATION;
	}


	/*
	if (g_theInput->IsKeyDown( 'W' )) { // W key accelerates
		m_accelerateVelocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, PLAYER_SHIP_ACCELERATION );
	}
	if (g_theInput->WasKeyJustReleased( 0x45 )) { // stop accelerates
		m_accelerateVelocity = Vec2( 0.f, 0.f );
	}
	 
	if (g_theInput->IsKeyDown( 'D' )) { // D key turn right
		m_orientationDegrees -= PLAYER_SHIP_TURN_SPEED * deltaTime;
		while (m_orientationDegrees < 0.f) {
			m_orientationDegrees += 360.f;
		}
	}
	if (g_theInput->IsKeyDown( 'A' )) { // A key turn left
		m_orientationDegrees += PLAYER_SHIP_TURN_SPEED * deltaTime;
		while (m_orientationDegrees > 360.f) {
			m_orientationDegrees -= 360.f;
		}
	}*/
	
	if (m_invincibleTimeLeft > 0.f) {
		m_color = Rgba8( 102, 153, 204, 100 );
		m_invincibleTimeLeft -= deltaTime;
	}
	else {
		m_color = Rgba8( 102, 153, 204, 255 );
		m_invincibleTimeLeft = 0.f;
	}

	// shield cool down
	if (m_isShieldOn && !m_hasShield) {
		m_shieldTimer -= deltaTime;
		if (m_shieldTimer <= 0.f) {
			m_hasShield = true;
			m_shieldEntity = new Shield( m_position, m_game, this, m_physicsRadius * 1.8f, Rgba8( 0, 255, 255, 150 ) );
			m_game->SpawnNewDebris( m_shieldEntity );
			g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::playerReborn ) );
		}
	}

	// super bullet weapon cool down
	if (m_superBulletWeapon && m_superWeaponTimer > 0.f) {
		m_superWeaponTimer -= deltaTime;
		if (m_superWeaponTimer < 0.f) {
			m_superWeaponTimer = 0.f;
		}
	}

	// light saber sound effect cool down
	if (m_isLightSaberStart && m_lightSaberRotateTime <= 0.f) {
		m_lightSaberRotateTime = m_lightSaberRotateCooldown;
		g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::lightSaberRotate ), false, 1.3f );
	}
	else if (m_isLightSaberStart) {
		m_lightSaberRotateTime -= deltaTime;
	}

	m_velocity += deltaTime * m_accelerateVelocity;
	m_velocity -= m_velocity.GetNormalized() * 100.f * deltaTime;
	m_velocity.ClampLength( m_maxSpeed );
	m_position += deltaTime * m_velocity;
	DealCollidingEdge();
}

void PlayerShip::Render() const
{
	Vertex_PCU localShipVerts[24];
	int numOfVerts;
	if (m_enhanceTimeLeft > 0.f) {
		numOfVerts = 24;
	}
	else {
		numOfVerts = 18;
	}
	GetShipVerts( localShipVerts );

	// flame of the ship
	float lenOfFlame = m_accelerateVelocity.GetLength() / 100.f;
	localShipVerts[15] = Vertex_PCU( Vec3( -2.f, 0.7f, 0.f ), Rgba8( 255, 100, 0, 255 ), Vec2( 0.f, 0.f ) );
	localShipVerts[16] = Vertex_PCU( Vec3( -2.f, -0.7f, 0.f ), Rgba8( 255, 100, 0, 255 ), Vec2( 0.f, 0.f ) );
	localShipVerts[17] = Vertex_PCU( Vec3( -2.f - lenOfFlame, 0.f, 0.f ), Rgba8( 255, 100, 0, (unsigned char) (m_game->m_randNumGen->RollRandomIntInRange(0, 100))), Vec2( 0.f, 0.f ) );
	if (m_enhanceTimeLeft > 0.f) {
		localShipVerts[18] = Vertex_PCU( Vec3( 1.f, -1.f, 0.f ), Rgba8( 192, 192, 192, 255 ), Vec2( 0.f, 0.f ) );
		localShipVerts[19] = Vertex_PCU( Vec3( -1.5f, -1.f, 0.f ), Rgba8( 192, 192, 192, 255 ), Vec2( 0.f, 0.f ) );
		localShipVerts[20] = Vertex_PCU( Vec3( -5.f, -3.f, 0.f ), Rgba8( 192, 192, 192, 0 ), Vec2( 0.f, 0.f ) );

		localShipVerts[21] = Vertex_PCU( Vec3( 1.f, 1.f, 0.f ), Rgba8( 192, 192, 192, 255 ), Vec2( 0.f, 0.f ) );
		localShipVerts[22] = Vertex_PCU( Vec3( -1.5f, 1.f, 0.f ), Rgba8( 192, 192, 192, 255 ), Vec2( 0.f, 0.f ) );
		localShipVerts[23] = Vertex_PCU( Vec3( -5.f, 3.f, 0.f ), Rgba8( 192, 192, 192, 0 ), Vec2( 0.f, 0.f ) );
	}

	for (int i = 0; i < 15; i++) {
		localShipVerts[i].m_color = m_color;
	}
	TransformVertexArrayXY3D( numOfVerts, localShipVerts, 1.f, m_orientationDegrees, m_position );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( numOfVerts, localShipVerts );

	//if (m_hasShield) {
	//	DebugDrawRing( m_position, m_cosmeticRadius, 0.4f, Rgba8( 100, 100, 255, 128 ) );
	//}
}

void PlayerShip::Die()
{
	m_game->SpawnDebris( this );
	g_theAudio->StartSound( g_theApp->GetSoundId( AudioName::playerDie ) );
	m_isDead = true;
	if (m_isLightSaberStart) {
		m_game->CreateSaber( m_position, this, 8.f, 100000.f );
		m_isLightSaberStart = false;
	}
}

void PlayerShip::BeAttacked( int hit )
{
	if (m_invincibleTimeLeft <= 0.f) {
		m_game->m_screenShakeCountSeconds = SCREEN_SHAKE_SECONDS;
		if (m_hasShield) {
			m_hasShield = false;
			m_shieldTimer = m_shieldCoolDownTime;
			m_shieldEntity->Die();
			m_shieldEntity = nullptr;
		}
		else {
			m_health -= hit;
		}
		m_invincibleTimeLeft = 2.f;
	}
}

Vec2 PlayerShip::GetNosePosition() const
{
	return Vec2::MakeFromPolarDegrees( m_orientationDegrees ) + m_position;
}

void PlayerShip::GetShipVerts( Vertex_PCU* shipVertsOut, int numOfVerts )
{
	
	Vertex_PCU shipVerts[NUM_OF_PLAYER_SHIP_VERTS] = {
		Vertex_PCU( Vec3( 0.f,  2.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 2.f,  1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f,  1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f,  1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f, -1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f,  1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f, -1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f,  1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f,  1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 1.f,  0.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -2.f, -1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 2.f, -1.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( 0.f, -2.f, 0.f ), Rgba8( 102, 153, 204, 255 ), Vec2( 0.f, 0.f ) ),
	};
	for (int i = 0; i < numOfVerts; i++) {
		shipVertsOut[i] = shipVerts[i];
	}
}

void PlayerShip::DealCollidingEdge()
{
	if (m_position.x + m_physicsRadius > WORLD_SIZE_X) {
		m_position.x = WORLD_SIZE_X - m_physicsRadius;
		m_velocity.x = -m_velocity.x;
	}
	if (m_position.x - m_physicsRadius < 0) {
		m_position.x = m_physicsRadius;
		m_velocity.x = -m_velocity.x;
	}
	if (m_position.y + m_physicsRadius > WORLD_SIZE_Y) {
		m_position.y = WORLD_SIZE_Y - m_physicsRadius;
		m_velocity.y = -m_velocity.y;
	}
	if (m_position.y - m_physicsRadius < 0) {
		m_position.y = m_physicsRadius;
		m_velocity.y = -m_velocity.y;
	}
}
