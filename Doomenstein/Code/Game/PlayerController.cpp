#include "Game/PlayerController.hpp"
#include "Game/Game.hpp"
#include "Game/Weapon.hpp"
#include "Game/AIController.hpp"

PlayerController::PlayerController()
{
	m_worldCamera.SetRenderBasis( Vec3( 0.f, 0.f, 1.f ), Vec3( -1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ) );
	m_hitTimer = new Timer( 0.5f, g_theGame->m_gameClock );
}

PlayerController::~PlayerController()
{

}

void PlayerController::Update()
{
	if (!m_controlledActorUID.IsValid()) {
		// re-spawn player onto map
		std::vector<ActorUID>& playerStart = g_theGame->m_curMap->m_playerStart;
		if ((int)playerStart.size() > 0) {
			int rnd = g_theGame->m_randNumGen->RollRandomIntLessThan( (int)playerStart.size() );
			ActorUID spawnPointUID = playerStart[rnd];
			ActorUID marineID = g_theGame->m_curMap->SpawnActorToMap( ActorDefinition::GetActorDefinition( "Marine" ), spawnPointUID->m_position, spawnPointUID->m_orientation, spawnPointUID->m_velocity );
			Possess( marineID );
			g_theGame->m_curMap->m_curPlayerActorIndex = marineID.GetIndex();
			if (g_theGame->m_curMap->m_mapDef->m_gameMode == "Survival") {
				((SurvivalGameMode*)g_theGame->m_curMap->m_gameMode)->m_playerDeath++;
			}
		}
		else {
			ERROR_AND_DIE( Stringf( "Error! There is no spawn point in map definition %s", g_theGame->m_curMap->m_mapDef->m_mapName.c_str() ) );
		}
	
		return;
	}
	if (!m_controlledActorUID->IsAlive()) {
		if (m_cameraMode == PlayerControlMode::ACTOR) {
			m_worldCamera.SetTransform( Vec3( m_worldCamera.m_position.x, m_worldCamera.m_position.y, m_controlledActorUID->m_def.m_eyeHeight * (1.f - m_controlledActorUID->m_destroyTimer->GetElapsedFraction()) ), m_worldCamera.m_orientation );
		}
		return;
	}

	UpdateInput();
	UpdateCamera();
}

void PlayerController::RenderUI() const
{
	if (!m_controlledActorUID.IsValid()) {
		return;
	}
	if (m_controlledActorUID->m_def.m_AIEnabled) {
		return;
	}

	/*if ((int)g_theGame->m_AIs.size() > 0) {
		AIController* debugController = g_theGame->m_AIs[0];
		if (debugController && debugController->m_tileHeatMap) {
			std::vector<Vertex_PCU> verts;
			debugController->m_tileHeatMap->AddTextVertsForDebugDraw( verts, AABB2( Vec2( 400.f, 0.f ), Vec2( 1200.f, 800.f ) ) );
			g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
			g_theRenderer->SetDepthMode( DepthMode::DISABLED );
			g_theRenderer->SetBlendMode( BlendMode::ALPHA );
			g_theRenderer->BindShader( nullptr );
			g_theRenderer->BindTexture( nullptr );
			g_theRenderer->SetModelConstants();
			g_theRenderer->DrawVertexArray( verts );
			verts.clear();
		}
	}*/

	if (m_cameraMode == PlayerControlMode::FREE_FLY) {
		return;
	}
	Weapon* curWeapon = m_controlledActorUID->m_curWeapon;
	if (curWeapon) {
		WeaponDefinition const& weaponDef = curWeapon->m_def;
		std::vector<Vertex_PCU> HUDVerts;
		HUDVerts.reserve( 6 );
		AddVertsForAABB2D( HUDVerts, AABB2( Vec2( 0.f, 0.f ), Vec2( 1600.f, 120.f ) ), Rgba8::WHITE, AABB2::IDENTITY );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindShader( weaponDef.m_HUDshader );
		g_theRenderer->BindTexture( weaponDef.m_baseTexture );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( HUDVerts );

		if (m_controlledActorUID->IsAlive()) {
			std::vector<Vertex_PCU> weaponVerts;
			weaponVerts.reserve( 6 );
			SpriteDefinition const& spriteDef = curWeapon->GetCurrentSpriteDef();
			Vec2 pivotTranslation = Vec2( -weaponDef.m_spritePivot.x * weaponDef.m_spriteSize.x, -weaponDef.m_spritePivot.y * weaponDef.m_spriteSize.y );
			AddVertsForAABB2D( weaponVerts, AABB2( Vec2( 800.f, 120.f ) + pivotTranslation, Vec2( 800.f, 120.f ) + weaponDef.m_spriteSize + pivotTranslation ), Rgba8::WHITE, spriteDef.GetUVs() );
			g_theRenderer->BindShader( weaponDef.m_animationShader );
			g_theRenderer->BindTexture( &spriteDef.GetTexture() );
			g_theRenderer->DrawVertexArray( weaponVerts );

			std::vector<Vertex_PCU> reticleVerts;
			reticleVerts.reserve( 6 );
			AddVertsForAABB2D( reticleVerts, AABB2( Vec2( 800.f, 400.f ) - weaponDef.m_reticleSize * 0.5f, Vec2( 800.f, 400.f ) + weaponDef.m_reticleSize * 0.5f ), Rgba8::WHITE, AABB2::IDENTITY );
			g_theRenderer->BindShader( weaponDef.m_HUDshader );
			g_theRenderer->BindTexture( weaponDef.m_reticleTexture );
			g_theRenderer->DrawVertexArray( reticleVerts );
		}
	}

	std::vector<Vertex_PCU> textVerts;
	textVerts.reserve( 100 );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 25.f ), Vec2( 680.f, 100.f ) ), 60.f, Stringf( "%.0f", m_controlledActorUID->m_health ) );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 25.f ), Vec2( 210.f, 100.f ) ), 60.f, Stringf( "%d", m_numOfPlayerKills ) );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1400.f, 25.f ), Vec2( 1600.f, 100.f ) ), 60.f, Stringf( "%d", m_numOfDeaths ) );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );

	if (!m_controlledActorUID->IsAlive()) {
		std::vector<Vertex_PCU> overlayVerts;
		overlayVerts.reserve( 6 );
		AddVertsForAABB2D( overlayVerts, AABB2( Vec2( 0.f, 0.f ), Vec2( 1600.f, 800.f ) ), Rgba8( 32, 32, 32, 128 ), AABB2::IDENTITY );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( overlayVerts );
	}
	else if (m_hitTimer->HasStartedAndNotPeriodElapsed()) {
		std::vector<Vertex_PCU> overlayVerts;
		overlayVerts.reserve( 12 );
		overlayVerts.emplace_back( Vec2( 800.f, 400.f ), Rgba8( 0, 0, 0, 0 ) );
		overlayVerts.emplace_back( Vec2( 1600.f, 800.f ), Rgba8( 200, 0, 0, 128 ) );
		overlayVerts.emplace_back( Vec2( 0.f, 800.f ), Rgba8( 200, 0, 0, 128 ) );
		overlayVerts.emplace_back( Vec2( 800.f, 400.f ), Rgba8( 0, 0, 0, 0 ) );
		overlayVerts.emplace_back( Vec2( 1600.f, 0.f ), Rgba8( 200, 0, 0, 128 ) );
		overlayVerts.emplace_back( Vec2( 1600.f, 800.f ), Rgba8( 200, 0, 0, 128 ) );
		overlayVerts.emplace_back( Vec2( 800.f, 400.f ), Rgba8( 0, 0, 0, 0 ) );
		overlayVerts.emplace_back( Vec2( 0.f, 0.f ), Rgba8( 200, 0, 0, 128 ) );
		overlayVerts.emplace_back( Vec2( 1600.f, 0.f ), Rgba8( 200, 0, 0, 128 ) );
		overlayVerts.emplace_back( Vec2( 800.f, 400.f ), Rgba8( 0, 0, 0, 0 ) );
		overlayVerts.emplace_back( Vec2( 0.f, 800.f ), Rgba8( 200, 0, 0, 128 ) );
		overlayVerts.emplace_back( Vec2( 0.f, 0.f ), Rgba8( 200, 0, 0, 128 ) );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( overlayVerts );
	}

}

void PlayerController::UpdateInput()
{
	if (g_theInput->WasKeyJustPressed( 'F' ) && g_theGame->m_numOfPlayers == 1) {
		if (m_cameraMode == PlayerControlMode::ACTOR) {
			m_cameraMode = PlayerControlMode::FREE_FLY;
			m_worldCamera.SetPerspectiveView( m_worldCamera.m_perspectiveAspect, 60.f, 0.1f, 100.f );
			m_worldCamera.SetTransform( Vec3( 15.5f, 15.5f, 28.f ), EulerAngles( 90.f, 90.f, 0.f ) );

		}
		else if (m_cameraMode == PlayerControlMode::FREE_FLY) {
			m_cameraMode = PlayerControlMode::ACTOR;
			m_worldCamera.SetPerspectiveView( m_worldCamera.m_perspectiveAspect, m_controlledActorUID->m_def.m_cameraFOVDegrees, 0.1f, 100.f );
		}
	}
	float deltaSeconds = g_theGame->m_gameClock->GetDeltaSeconds();
	float speed;
	Vec3 iBasis, jBasis, kBasis;
	if (m_cameraMode == PlayerControlMode::ACTOR) {
		speed = m_controlledActorUID->m_def.m_walkSpeed;
		m_controlledActorUID->m_orientation.GetAsVectors_IFwd_JLeft_KUp( iBasis, jBasis, kBasis );
		Vec3 force = Vec3( 0.f, 0.f, 0.f );

		if (m_controllerIndex == -1) {
			if (g_theInput->IsKeyDown( 0x10/*Shift Key*/ )) {
				speed = m_controlledActorUID->m_def.m_runSpeed;
			}
			if (g_theInput->IsKeyDown( 'W' )) {
				force += iBasis;
			}
			if (g_theInput->IsKeyDown( 'S' )) {
				force -= iBasis;
			}
			if (g_theInput->IsKeyDown( 'A' )) {
				force += jBasis;
			}
			if (g_theInput->IsKeyDown( 'D' )) {
				force -= jBasis;
			}
			if (g_theInput->IsKeyDown( KEYCODE_LEFTMOUSE )) {
				m_controlledActorUID->Attack();
			}
			if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTARROW )) {
				m_controlledActorUID->EquipPrevWeapon();
			}
			if (g_theInput->WasKeyJustPressed( KEYCODE_RIGHTARROW )) {
				m_controlledActorUID->EquipNextWeapon();
			}
			if (g_theInput->WasKeyJustPressed( '1' )) {
				m_controlledActorUID->EquipWeapon( 0 );
			}
			if (g_theInput->WasKeyJustPressed( '2' )) {
				m_controlledActorUID->EquipWeapon( 1 );
			}
			if (g_theInput->WasKeyJustPressed( 'N' )) {
				m_controlledActorUID->m_map->DebugPossessNext();
			}
			if (g_theInput->WasKeyJustPressed( 'K' )) {
				m_controlledActorUID->m_map->DebugKillAllExceptSelf( m_controlledActorUID );
			}

			if (deltaSeconds != 0.f) {
				Vec2 cursorDisp = g_theInput->GetCursorClientDelta();
				m_controlledActorUID->TurnInDirection( EulerAngles( -0.075f * cursorDisp.x * g_window->GetClientDimensions().x,
					-0.075f * cursorDisp.y * g_window->GetClientDimensions().y, 0.f ) );
			}
			force.Normalize();
		}
		else if (m_controllerIndex >= 0) {
			XboxController& controller = g_theInput->GetController( m_controllerIndex );
			if (controller.IsConnected()) {
				Vec2 normalizedLeftStick = controller.GetLeftStick().GetPosition();
				Vec2 normalizedRightStick = controller.GetRightStick().GetPosition();
				force += iBasis * normalizedLeftStick.y;
				force -= jBasis * normalizedLeftStick.x;
				m_controlledActorUID->TurnInDirection(
					EulerAngles( -m_controlledActorUID->m_def.m_turnSpeedDegrees * deltaSeconds * normalizedRightStick.x,
						-m_controlledActorUID->m_def.m_turnSpeedDegrees * deltaSeconds * normalizedRightStick.y, 0.f ) );

				if (controller.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_A )) {
					speed = m_controlledActorUID->m_def.m_runSpeed;
				}
				if (controller.GetRightTrigger() > 0.2f) {
					m_controlledActorUID->Attack();
				}
				if (controller.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_UP )) {
					m_controlledActorUID->EquipPrevWeapon();
				}
				if (controller.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_DOWN )) {
					m_controlledActorUID->EquipNextWeapon();
				}
				if (controller.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_X )) {
					m_controlledActorUID->EquipWeapon( 0 );
				}
				if (controller.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_Y )) {
					m_controlledActorUID->EquipWeapon( 1 );
				}
			}
		}
		if (force != Vec3()) {
			m_controlledActorUID->AddForce( force * speed );
			if (m_controlledActorUID->m_animationState == "Default") {
				m_controlledActorUID->SetAnimationState( "Walk" );
				m_controlledActorUID->m_animationTimer->Start();
			}
		}
	}
	else if (m_cameraMode == PlayerControlMode::FREE_FLY) {
		deltaSeconds = Clock::GetSystemClock()->GetDeltaSeconds();
		m_worldCamera.m_orientation.GetAsVectors_IFwd_JLeft_KUp( iBasis, jBasis, kBasis );
		speed = 1.f;
		if (m_controllerIndex >= 0) {
			XboxController& controller = g_theInput->GetController( m_controllerIndex );
			if (controller.IsConnected()) {
				Vec2 normalizedLeftStick = controller.GetLeftStick().GetPosition();
				Vec2 normalizedRightStick = controller.GetRightStick().GetPosition();
				m_worldCamera.m_position += iBasis * speed * normalizedLeftStick.y * deltaSeconds;
				m_worldCamera.m_position -= jBasis * speed * normalizedLeftStick.x * deltaSeconds;
				m_worldCamera.m_orientation.m_yawDegrees -= 180.f * deltaSeconds * normalizedRightStick.x;
				m_worldCamera.m_orientation.m_pitchDegrees -= 180.f * deltaSeconds * normalizedRightStick.y;
				if (controller.WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_A )) {
					speed = 15.f;
				}
				if (controller.IsButtonDown( XboxButtonID::XBOX_BUTTON_LEFTSHOULDER )) {
					m_worldCamera.m_position += Vec3( 0.f, 0.f, 1.f ) * speed * deltaSeconds;
				}
				if (controller.IsButtonDown( XboxButtonID::XBOX_BUTTON_RIGHTSHOULDER )) {
					m_worldCamera.m_position += Vec3( 0.f, 0.f, -1.f ) * speed * deltaSeconds;
				}
			}
		}
		else {
			if (g_theInput->IsKeyDown( 0x10/*Shift Key*/ )) {
				speed = 15.f;
			}
			if (g_theInput->IsKeyDown( 'W' )) {
				m_worldCamera.m_position += iBasis * speed * deltaSeconds;
			}
			if (g_theInput->IsKeyDown( 'S' )) {
				m_worldCamera.m_position -= iBasis * speed * deltaSeconds;
			}
			if (g_theInput->IsKeyDown( 'A' )) {
				m_worldCamera.m_position += jBasis * speed * deltaSeconds;
			}
			if (g_theInput->IsKeyDown( 'D' )) {
				m_worldCamera.m_position -= jBasis * speed * deltaSeconds;
			}
			if (g_theInput->IsKeyDown( 'Z' )) {
				m_worldCamera.m_position += Vec3( 0.f, 0.f, 1.f ) * speed * deltaSeconds;
			}
			if (g_theInput->IsKeyDown( 'C' )) {
				m_worldCamera.m_position += Vec3( 0.f, 0.f, -1.f ) * speed * deltaSeconds;
			}
			if (g_theInput->WasKeyJustPressed( 'K' )) {
				m_controlledActorUID->m_map->DebugKillAllExceptSelf( m_controlledActorUID );
			}
			if (deltaSeconds != 0.f) {
				Vec2 cursorDisp = g_theInput->GetCursorClientDelta();
				m_worldCamera.m_orientation.m_yawDegrees -= 0.075f * cursorDisp.x * g_window->GetClientDimensions().x;
				m_worldCamera.m_orientation.m_pitchDegrees -= 0.075f * cursorDisp.y * g_window->GetClientDimensions().y;
			}
		}
	}
	m_controlledActorUID->m_orientation.m_pitchDegrees = GetClamped( m_controlledActorUID->m_orientation.m_pitchDegrees, -85.f, 85.f );
}

void PlayerController::UpdateCamera()
{
	if (m_cameraMode == PlayerControlMode::ACTOR) {
		m_worldCamera.SetTransform( m_controlledActorUID->m_position + Vec3( 0.f, 0.f, m_controlledActorUID->m_def.m_eyeHeight ), m_controlledActorUID->m_orientation );
	}
}

bool PlayerController::IsPlayer() const
{
	return true;
}

bool PlayerController::IsAI() const
{
	return false;
}

void PlayerController::Possess( ActorUID uid )
{
	Controller::Possess( uid );
	m_worldCamera.SetPerspectiveView( m_worldCamera.m_perspectiveAspect, uid->m_def.m_cameraFOVDegrees, 0.1f, 100.f );
	m_uiCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( UI_SIZE_X, UI_SIZE_Y ) );
}

void PlayerController::Damagedby( ActorUID uid, bool isLethal )
{
	if (isLethal) {
		Controller* damageSourceController = uid->m_controller;
		if (damageSourceController && damageSourceController->IsPlayer()) {
			m_numOfDeaths++;
			((PlayerController*)damageSourceController)->m_numOfPlayerKills++;
		}
	}
	m_hitTimer->Start();
}

