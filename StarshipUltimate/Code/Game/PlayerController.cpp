#include "Game/PlayerController.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Game.hpp"
#include "Game/Room.hpp"
#include "Game/Item.hpp"
#include "Game/App.hpp"

char PLAYER_UP_KEYCODE = 'W';
char PLAYER_DOWN_KEYCODE = 'S';
char PLAYER_LEFT_KEYCODE = 'A';
char PLAYER_RIGHT_KEYCODE = 'D';
char PLAYER_MAIN_WEAPON_KEYCODE = KEYCODE_LEFTMOUSE;
char PLAYER_SUB_WEAPON_KEYCODE = KEYCODE_RIGHTMOUSE;
char PLAYER_DASH_KEYCODE = ' ';
char PLAYER_RECOVER_KEYCODE = 'R';
char PLAYER_ULTIMATE_KEYCODE = 'E';
char PLAYER_INTERACT_KEYCODE = 'F';
char PLAYER_ITEM_SCREEN_KEYCODE = 'I';
char PLAYER_MAP_SCREEN_KEYCODE = 'M';

char DEBUG_KILL_ALL_KEYCODE = 'K';
char DEBUG_AUTO_SHOOT_MAIN_KEYCODE = 'Y';
char DEBUG_AUTO_SHOOT_SUB_KEYCODE = 'U';


PlayerController::PlayerController()
{
	m_UICamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );
	m_UICamera.m_mode = CameraMode::Orthographic;
}

PlayerController::~PlayerController()
{

}

bool PlayerController::IsPlayer() const
{
	return true;
}

bool PlayerController::IsAI() const
{
	return false;
}

void PlayerController::Possess( Entity* entity )
{
	Controller::Possess( entity );
	m_playerShip = (PlayerShip*)entity;
}

void PlayerController::Update( float deltaSeconds )
{
	if (m_playerShip->m_deathTimer->HasStartedAndNotPeriodElapsed()) {
		return;
	}
	UNUSED( deltaSeconds );
	if (m_playerShip->IsDashing()) {
		return;
	}
	if (deltaSeconds == 0.f) {
		return;
	}
	Vec2 mouseWorldPos = g_theGame->m_worldCamera.GetCursorWorldPosition( g_window->GetNormalizedCursorPos() );
	float orientationDegrees = (mouseWorldPos - m_controlledEntity->m_position).GetOrientationDegrees();
	m_controlledEntity->SetOrientationDegrees( orientationDegrees );
	Vec2 forwardVector = m_controlledEntity->GetForwardNormal();
	Vec2 directionToGo = Vec2( 0.f, 0.f );
	if (g_theInput->IsKeyDown( PLAYER_UP_KEYCODE )) {
		directionToGo += Vec2( 0.f, 1.f );
	}
	if (g_theInput->IsKeyDown( PLAYER_DOWN_KEYCODE )) {
		directionToGo += Vec2( 0.f, -1.f );
	}
	if (g_theInput->IsKeyDown( PLAYER_LEFT_KEYCODE )) {
		directionToGo += Vec2( -1.f, 0.f );
	}
	if (g_theInput->IsKeyDown( PLAYER_RIGHT_KEYCODE )) {
		directionToGo += Vec2( 1.f, 0.f );
	}
	if (g_theApp->m_autoShootMainWeapon) {
		if (m_playerShip->FireMainWeapon()) {
			//g_theGame->SetOneDirCameraShake( -forwardVector * 0.4f, 0.1f );
		}
	}
	else {
		if (g_theInput->IsKeyDown( PLAYER_MAIN_WEAPON_KEYCODE )) {
			if (m_playerShip->FireMainWeapon()) {
				//g_theGame->SetOneDirCameraShake( -forwardVector * 0.4f, 0.1f );
			}
		}
	}
	if (g_theApp->m_autoShootSubWeapon) {
		m_playerShip->FireSubWeapon();
	}
	else {
		if (g_theInput->WasKeyJustPressed( PLAYER_SUB_WEAPON_KEYCODE )) {
			m_playerShip->FireSubWeapon();
		}
	}
	directionToGo.Normalize();
	if (g_theInput->WasKeyJustPressed( PLAYER_DASH_KEYCODE )) {
		if (Starship_IsVec2NearlyZero( directionToGo )) {
		//	Vec2 direction = (mouseWorldPos - m_controlledEntity->m_position).GetNormalized();
		//	m_playerShip->DoDash( direction );
		}
		else {
		// only dash when player ship has speed
			m_playerShip->DoDash( directionToGo );
		}
	}
	if (g_theInput->IsKeyDown( PLAYER_RECOVER_KEYCODE )) {

	}
	if (g_theInput->WasKeyJustPressed( PLAYER_ULTIMATE_KEYCODE )) {
		m_playerShip->PerformSkill();
	}
	if (g_theInput->WasKeyJustPressed( PLAYER_INTERACT_KEYCODE )) {
		m_playerShip->Interact();
	}
#ifdef DEBUG_MODE
	if (g_theInput->WasKeyJustPressed( DEBUG_KILL_ALL_KEYCODE )) {
		g_theGame->DebugKillEverything();
	}
#endif

	if (directionToGo != Vec2()) {
		m_controlledEntity->AddForce( directionToGo * m_playerShip->GetMovingSpeed() );
		Vec2 velocityRef = -m_controlledEntity->GetForwardNormal() * m_controlledEntity->m_velocity.GetLength() * 0.8f;
		OBB2 velocityRange( velocityRef, -m_controlledEntity->GetForwardNormal(), Vec2( m_controlledEntity->m_velocity.GetLength() * 0.2f, m_controlledEntity->m_velocity.GetLength() * 0.5f ) );
		ParticleSystem2DAddEmitter( 100, 0.02f, AABB2( m_controlledEntity->m_position - m_controlledEntity->GetForwardNormal() * m_controlledEntity->m_cosmeticRadius,
			m_controlledEntity->m_position - m_controlledEntity->GetForwardNormal() * m_controlledEntity->m_cosmeticRadius ),
			FloatRange( m_controlledEntity->m_cosmeticRadius * 0.2f, m_controlledEntity->m_cosmeticRadius * 0.3f ),
			AABB2( velocityRange.GetRandomPointInside(), velocityRange.GetRandomPointInside() ),
			FloatRange( 0.05f, 0.1f ), Rgba8( 153, 204, 255 ), Particle2DShape::Disc, true, FloatRange( 0.f, 0.f ),
			FloatRange( 0.f, 0.f ), nullptr,
			Rgba8( 153, 204, 255, 0 ) );
	}
}

void PlayerController::Render()
{

}

void PlayerController::RenderUI()
{
	g_theRenderer->BeginCamera( m_UICamera );

	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;

	// if in start room, render input key bindings
	if (!g_theGame->m_isQuitting && g_theGame->m_curRoom->m_def.m_name == "startRoom" && !g_theGame->m_renderItemScreen && !g_theGame->m_renderMapScreen && !g_theGame->m_renderOptionsMenu) {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 200.f, 600.f ), Vec2( 1600.f, 630.f ) ), 30.f, Stringf( "Move Up: %s", g_starshipKeyStringMapping[PLAYER_UP_KEYCODE].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 200.f, 550.f ), Vec2( 1600.f, 580.f ) ), 30.f, Stringf( "Move Left: %s", g_starshipKeyStringMapping[PLAYER_LEFT_KEYCODE].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 200.f, 500.f ), Vec2( 1600.f, 530.f ) ), 30.f, Stringf( "Move Down: %s", g_starshipKeyStringMapping[PLAYER_DOWN_KEYCODE].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 200.f, 450.f ), Vec2( 1600.f, 480.f ) ), 30.f, Stringf( "Move Right: %s", g_starshipKeyStringMapping[PLAYER_RIGHT_KEYCODE].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 200.f, 400.f ), Vec2( 1600.f, 430.f ) ), 30.f, Stringf( "Interact: %s", g_starshipKeyStringMapping[PLAYER_INTERACT_KEYCODE].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 200.f, 350.f ), Vec2( 1600.f, 380.f ) ), 30.f, Stringf( "Options Menu: %s", g_starshipKeyStringMapping[KEYCODE_ESC].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
	
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 800.f, 600.f ), Vec2( 1600.f, 630.f ) ), 30.f, Stringf( "Dash: %s", g_starshipKeyStringMapping[PLAYER_DASH_KEYCODE].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 800.f, 550.f ), Vec2( 1600.f, 580.f ) ), 30.f, Stringf( "Skill: %s", g_starshipKeyStringMapping[PLAYER_ULTIMATE_KEYCODE].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 800.f, 500.f ), Vec2( 1600.f, 530.f ) ), 30.f, Stringf( "Shoot Main Weapon: %s", g_starshipKeyStringMapping[PLAYER_MAIN_WEAPON_KEYCODE].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 800.f, 450.f ), Vec2( 1600.f, 480.f ) ), 30.f, Stringf( "Shoot Sub-Weapon: %s", g_starshipKeyStringMapping[PLAYER_SUB_WEAPON_KEYCODE].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 800.f, 400.f ), Vec2( 1600.f, 430.f ) ), 30.f, Stringf( "Map Screen: %s", g_starshipKeyStringMapping[PLAYER_MAP_SCREEN_KEYCODE].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 800.f, 330.f ), Vec2( 1600.f, 380.f ) ), 30.f, Stringf( "Item Screen: %s", g_starshipKeyStringMapping[PLAYER_ITEM_SCREEN_KEYCODE].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
	
	}

	// coins
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 60.f, 740.f ), Vec2( 120.f, 770.f ) ), 30.f, Stringf( "%d", m_reward ) );
	//g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1450.f, 740.f ), Vec2( 1600.f, 770.f ) ), 30.f, Stringf( "%d %d", g_theGame->m_curRoom->m_coords.x, g_theGame->m_curRoom->m_coords.y ) );
	AddVertsForDisc2D( verts, Vec2( 36.f, 757.f ), 15.f, Rgba8( 255, 255, 0 ) );

	// skill item remain skills
	if (m_playerShip->m_skillItemID != -1) {
		ItemDefinition const& def = ItemDefinition::GetDefinition( m_playerShip->m_skillItemID );
		AABB2 itemUIPos( Vec2( 50.f, 670.f ), Vec2( 100.f, 720.f ) );

		//g_ASCIIFont->AddVertsForTextInBox2D( textVerts, itemUIPos, 25.f, Stringf( "%d", def.m_id ), Rgba8( 0, 0, 0 ) );
		//AddVertsForAABB2D( verts, itemUIPos, Rgba8::WHITE );
		ItemDefinition::GetDefinition( m_playerShip->m_skillItemID ).RenderItem( itemUIPos );

		if (def.m_hasCharge) {
			AABB2 firstBlockAABB( Vec2( 45.f, 640.f ), Vec2( 53.f, 660.f ) );
			for (int i = 0; i < def.m_maxCharge; i++) {
				Rgba8 color;
				if (i < def.m_charge) {
					color = Rgba8( 0, 255, 0 );
				}
				else {
					color = Rgba8( 255, 0, 0 );
				}
				AddVertsForAABB2D( verts, AABB2( Vec2( firstBlockAABB.m_mins.x + i * 13.f, firstBlockAABB.m_mins.y ), Vec2( firstBlockAABB.m_maxs.x + i * 13.f, firstBlockAABB.m_maxs.y ) ), color );
			}
		}
	}


	if (!m_playerShip->m_healthBlind) {
		std::vector<Vertex_PCU> healthVerts;
		// render health and armor
		AABB2 bounds( Vec2( 150.f, 705.f ), Vec2( 510.f, 780.f ) );
		constexpr float healthHeight = 30.f;
		constexpr float healthWidth = 30.f;
		constexpr float marginPercentage = 0.1f;
		Vec2 topLeft = Vec2( bounds.m_mins.x, bounds.m_maxs.y );
		float line = 1.f;
		float numOfHealthEachLine = (bounds.m_maxs.x - bounds.m_mins.x) / healthWidth;
		for (int i = 0; i < (int)m_playerShip->m_health; i++) {
			Vec2 thisBottomLeft = topLeft + Vec2( (i - (line - 1.f) * numOfHealthEachLine) * healthWidth, -healthHeight * line );
			AddVertsForAABB2D( healthVerts, AABB2( thisBottomLeft + Vec2( healthWidth, healthHeight ) * marginPercentage * 0.5f
				, thisBottomLeft + Vec2( healthWidth, healthHeight ) * (1.f - marginPercentage) ), Rgba8::WHITE, g_pickupsSprites->GetSpriteUVs( 0 ) );
			if (i + 1.f >= numOfHealthEachLine * line) {
				line += 1.f;
			}
		}
		for (int i = (int)m_playerShip->m_health; i < (int)m_playerShip->m_maxHealth; i++) {
			Vec2 thisBottomLeft = topLeft + Vec2( (i - (line - 1.f) * numOfHealthEachLine) * healthWidth, -healthHeight * line );
			AddVertsForAABB2D( healthVerts, AABB2( thisBottomLeft + Vec2( healthWidth, healthHeight ) * marginPercentage * 0.5f
				, thisBottomLeft + Vec2( healthWidth, healthHeight ) * (1.f - marginPercentage) ), Rgba8::WHITE, g_pickupsSprites->GetSpriteUVs( 2 ) );
			if (i + 1.f >= numOfHealthEachLine * line) {
				line += 1.f;
			}
		}
		for (int i = (int)m_playerShip->m_maxHealth; i < (int)(m_playerShip->m_curArmor + m_playerShip->m_maxHealth); i++) {
			Vec2 thisBottomLeft = topLeft + Vec2( (i - (line - 1.f) * numOfHealthEachLine) * healthWidth, -healthHeight * line );
			AddVertsForAABB2D( healthVerts, AABB2( thisBottomLeft + Vec2( healthWidth, healthHeight ) * marginPercentage * 0.5f
				, thisBottomLeft + Vec2( healthWidth, healthHeight ) * (1.f - marginPercentage) ), Rgba8::WHITE, g_pickupsSprites->GetSpriteUVs( 1 ) );
			if (i + 1.f >= numOfHealthEachLine * line) {
				line += 1.f;
			}
		}
		for (int i = (int)(m_playerShip->m_curArmor + m_playerShip->m_maxHealth); i < (int)(m_playerShip->m_maxArmor + m_playerShip->m_maxHealth); i++) {
			Vec2 thisBottomLeft = topLeft + Vec2( (i - (line - 1.f) * numOfHealthEachLine) * healthWidth, -healthHeight * line );
			AddVertsForAABB2D( healthVerts, AABB2( thisBottomLeft + Vec2( healthWidth, healthHeight ) * marginPercentage * 0.5f
				, thisBottomLeft + Vec2( healthWidth, healthHeight ) * (1.f - marginPercentage) ), Rgba8::WHITE, g_pickupsSprites->GetSpriteUVs( 3 ) );
			if (i + 1.f >= numOfHealthEachLine * line) {
				line += 1.f;
			}
		}
		g_theRenderer->BindTexture( &g_pickupsSprites->GetTexture() );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( healthVerts );

	}

	// sub-weapon and cool down
	if (m_playerShip->m_subWeaponItemID != -1) {
		//ItemDefinition const& def = ItemDefinition::GetDefinition( m_playerShip->m_subWeaponItemID );
		AABB2 itemUIPos( Vec2( 50.f, 580.f ), Vec2( 100.f, 630.f ) );
		//g_ASCIIFont->AddVertsForTextInBox2D( textVerts, itemUIPos, 25.f, Stringf( "%d", def.m_id ), Rgba8( 0, 0, 0 ) );
		//AddVertsForAABB2D( verts, itemUIPos, Rgba8::WHITE );
		ItemDefinition::GetDefinition( m_playerShip->m_subWeaponItemID ).RenderItem( itemUIPos );

		AddVertsForAABB2D( verts, AABB2( Vec2( 45.f, 550.f ), Vec2( 105.f, 570.f ) ), Rgba8( 255, 0, 0 ) );
		AddVertsForAABB2D( verts, AABB2( Vec2( 45.f, 550.f ), Vec2( 45.f + 60.f * m_playerShip->m_subWeaponCoolDownTimer->GetElapsedFraction(), 570.f ) ), Rgba8( 0, 255, 0 ) );
	}

	if (!m_playerShip->m_blindWithRoom && !g_theGame->m_isQuitting) {
		// room map (top right corner)
		Room* curRoom = g_theGame->m_curRoom;
		Vec2 startPos = Vec2( 1450.f, 680.f );
		Vec2 size = Vec2( 50.f, 40.f );
		Vec2 margin = Vec2( 10.f, 8.f );
		AddVertsForAABB2D( verts, AABB2( startPos, startPos + size ), Rgba8( 224, 224, 224, 160 ) );
		if (curRoom->IsBossRoom()) {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( startPos, startPos + size ), 30.f, "BOSS" );
		}
		if (curRoom->m_def.m_type == RoomType::SHOP) {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( startPos, startPos + size ), 30.f, "SHOP", Rgba8( 255, 255, 0 ) );
		}

		Rgba8 adjColor;
		Rgba8 undiscoveredColor = Rgba8( 102, 51, 0, 100 );
		Rgba8 discoveredColor = Rgba8( 128, 128, 128, 140 );

		Room* checkRoom = g_theGame->GetRoomAtCoords( curRoom->m_coords + IntVec2( 1, 0 ) );
		if (checkRoom) {
			Vec2 thisStartPos = startPos + Vec2( margin.x + size.x, 0.f );
			if (checkRoom->m_isFirstEnter) {
				adjColor = undiscoveredColor;
			}
			else {
				adjColor = discoveredColor;
			}
			AddVertsForAABB2D( verts, AABB2( thisStartPos, thisStartPos + size ), adjColor );
			if (checkRoom->IsBossRoom()) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisStartPos, thisStartPos + size ), 30.f, "BOSS" );
			}
			if (checkRoom->m_def.m_type == RoomType::SHOP) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisStartPos, thisStartPos + size ), 30.f, "SHOP", Rgba8( 255, 255, 0 ) );
			}
		}
		checkRoom = g_theGame->GetRoomAtCoords( curRoom->m_coords + IntVec2( 1, 1 ) );
		if (checkRoom) {
			Vec2 thisStartPos = startPos + margin + size;
			if (checkRoom->m_isFirstEnter) {
				adjColor = undiscoveredColor;
			}
			else {
				adjColor = discoveredColor;
			}
			AddVertsForAABB2D( verts, AABB2( thisStartPos, thisStartPos + size ), adjColor );
			if (checkRoom->IsBossRoom()) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisStartPos, thisStartPos + size ), 30.f, "BOSS" );
			}
			if (checkRoom->m_def.m_type == RoomType::SHOP) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisStartPos, thisStartPos + size ), 30.f, "SHOP", Rgba8( 255, 255, 0 ) );
			}
		}
		checkRoom = g_theGame->GetRoomAtCoords( curRoom->m_coords + IntVec2( 0, 1 ) );
		if (checkRoom) {
			Vec2 thisStartPos = startPos + Vec2( 0.f, margin.y + size.y );
			if (checkRoom->m_isFirstEnter) {
				adjColor = undiscoveredColor;
			}
			else {
				adjColor = discoveredColor;
			}
			AddVertsForAABB2D( verts, AABB2( thisStartPos, thisStartPos + size ), adjColor );
			if (checkRoom->IsBossRoom()) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisStartPos, thisStartPos + size ), 30.f, "BOSS" );
			}
			if (checkRoom->m_def.m_type == RoomType::SHOP) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisStartPos, thisStartPos + size ), 30.f, "SHOP", Rgba8( 255, 255, 0 ) );
			}
		}
		checkRoom = g_theGame->GetRoomAtCoords( curRoom->m_coords + IntVec2( -1, 1 ) );
		if (checkRoom) {
			Vec2 thisStartPos = startPos + Vec2( -margin.x - size.x, margin.y + size.y );
			if (checkRoom->m_isFirstEnter) {
				adjColor = undiscoveredColor;
			}
			else {
				adjColor = discoveredColor;
			}
			AddVertsForAABB2D( verts, AABB2( thisStartPos, thisStartPos + size ), adjColor );
			if (checkRoom->IsBossRoom()) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisStartPos, thisStartPos + size ), 30.f, "BOSS" );
			}
			if (checkRoom->m_def.m_type == RoomType::SHOP) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisStartPos, thisStartPos + size ), 30.f, "SHOP", Rgba8( 255, 255, 0 ) );
			}
		}
		checkRoom = g_theGame->GetRoomAtCoords( curRoom->m_coords + IntVec2( -1, 0 ) );
		if (checkRoom) {
			Vec2 thisStartPos = startPos + Vec2( -margin.x - size.x, 0.f );
			if (checkRoom->m_isFirstEnter) {
				adjColor = undiscoveredColor;
			}
			else {
				adjColor = discoveredColor;
			}
			AddVertsForAABB2D( verts, AABB2( thisStartPos, thisStartPos + size ), adjColor );
			if (checkRoom->IsBossRoom()) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisStartPos, thisStartPos + size ), 30.f, "BOSS" );
			}
			if (checkRoom->m_def.m_type == RoomType::SHOP) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisStartPos, thisStartPos + size ), 30.f, "SHOP", Rgba8( 255, 255, 0 ) );
			}
		}
		checkRoom = g_theGame->GetRoomAtCoords( curRoom->m_coords + IntVec2( -1, -1 ) );
		if (checkRoom) {
			Vec2 thisStartPos = startPos - margin - size;
			if (checkRoom->m_isFirstEnter) {
				adjColor = undiscoveredColor;
			}
			else {
				adjColor = discoveredColor;
			}
			AddVertsForAABB2D( verts, AABB2( thisStartPos, thisStartPos + size ), adjColor );
			if (checkRoom->IsBossRoom()) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisStartPos, thisStartPos + size ), 30.f, "BOSS" );
			}
			if (checkRoom->m_def.m_type == RoomType::SHOP) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisStartPos, thisStartPos + size ), 30.f, "SHOP", Rgba8( 255, 255, 0 ) );
			}
		}
		checkRoom = g_theGame->GetRoomAtCoords( curRoom->m_coords + IntVec2( 0, -1 ) );
		if (checkRoom) {
			Vec2 thisStartPos = startPos + Vec2( 0.f, -margin.y - size.y );
			if (checkRoom->m_isFirstEnter) {
				adjColor = undiscoveredColor;
			}
			else {
				adjColor = discoveredColor;
			}
			AddVertsForAABB2D( verts, AABB2( thisStartPos, thisStartPos + size ), adjColor );
			if (checkRoom->IsBossRoom()) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisStartPos, thisStartPos + size ), 30.f, "BOSS" );
			}
			if (checkRoom->m_def.m_type == RoomType::SHOP) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisStartPos, thisStartPos + size ), 30.f, "SHOP", Rgba8( 255, 255, 0 ) );
			}
		}
		checkRoom = g_theGame->GetRoomAtCoords( curRoom->m_coords + IntVec2( 1, -1 ) );
		if (checkRoom) {
			Vec2 thisStartPos = startPos + Vec2( margin.x + size.x, -margin.y - size.y );
			if (checkRoom->m_isFirstEnter) {
				adjColor = undiscoveredColor;
			}
			else {
				adjColor = discoveredColor;
			}
			AddVertsForAABB2D( verts, AABB2( thisStartPos, thisStartPos + size ), adjColor );
			if (checkRoom->IsBossRoom()) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisStartPos, thisStartPos + size ), 30.f, "BOSS" );
			}
			if (checkRoom->m_def.m_type == RoomType::SHOP) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisStartPos, thisStartPos + size ), 30.f, "SHOP", Rgba8( 255, 255, 0 ) );
			}
		}
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );

	g_theRenderer->EndCamera( m_UICamera );
}

void PlayerController::GetReward( int numOfReward/*=1*/ )
{
	m_reward += numOfReward;
}
