#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Entity.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/PlayerController.hpp"
#include "Game/AIController.hpp"
#include "Game/PlayerBullet.hpp"
#include "Game/DiamondFraction.hpp"
#include "Game/Effects.hpp"
#include "Game/Room.hpp"
#include "Game/Item.hpp"
#include "Game/PlayerFaction.hpp"
#include "Game/DemonFaction.hpp"
#include "Game/NeutralFaction.hpp"
#include "Game/BulletFaction.hpp"
#include "Game/BacteriaFaction.hpp"
#include "Game/SettingsScreen.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"

static void* PlayerShieldDashDamageSource = (void*)1;

Game::Game()
{
	// load random number generator
	m_randNumGen = new RandomNumberGenerator( (unsigned int)time( NULL ) );
	m_gameClock = new Clock();
	m_cameraShakeTimer = new Timer( 0.4f, m_gameClock );
	m_cameraPerShakeTimer = new Timer( 0.02f, m_gameClock );
	m_cameraOneShakeTimer = new Timer( 0.f, m_gameClock );

	m_screenCamera.SetViewPort( g_theRenderer->GetSwapChainSize() );
	m_worldCamera.SetViewPort( g_theRenderer->GetSwapChainSize() );
}

Game::~Game()
{
	// stop music
	g_theAudio->StopSound( m_backgroundMusicID );
	// save the coins
	SaveGameBetweenRuns();

	// delete all pointers managed by Game
	delete m_randNumGen;
	m_randNumGen = nullptr;
	delete m_goToNextRoomTimer;
	m_goToNextRoomTimer = nullptr;
	delete m_cameraOneShakeTimer;
	m_cameraOneShakeTimer = nullptr;

	for (auto& entity : m_entityArray) {
		delete entity;
		entity = nullptr;
	}

	for (auto& effect : m_effectArray) {
		delete effect;
		effect = nullptr;
	}

	for (auto& projectile : m_projectileArray) {
		delete projectile;
		projectile = nullptr;
	}

	for (auto& controller : m_controllers) {
		delete controller;
		controller = nullptr;
	}

	for (auto& room : m_roomMap) {
		delete room;
		room = nullptr;
	}
}

void Game::Startup()
{
	// set up camera
	m_screenCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );
	m_screenCamera.m_mode = CameraMode::Orthographic;
	m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 1.f, -1.f );
	m_worldCamera.m_mode = CameraMode::Orthographic;
	m_cameraCenter = m_worldCamera.GetCenter();
	m_goToNextRoomTimer = new Timer( 0.5f, m_gameClock );

	// read game config
	m_levelMusicSequence.reserve( 10 );
	m_levelSequence.reserve( 10 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Definitions/GameDefinition.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document GameDefinition.xml error" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "GameDefinition" ), "Syntax Error! Name of the root of GameDefinition.xml should be \"GameDefinition\" " );
	m_bossMusic = ParseXmlAttribute( *root, "bossMusic", m_bossMusic );
	XmlElement* levelSequence = root->FirstChildElement( "LevelSequence" );
	if (levelSequence != nullptr) {
		XmlElement* levelIter = levelSequence->FirstChildElement( "Level" );
		while (levelIter != nullptr) {
			std::string faction = ParseXmlAttribute( *levelIter, "factionName", "DEFAULT" );
			std::string factionMusic = ParseXmlAttribute( *levelIter, "backgroundMusic", "DEFAULT" );
			m_levelSequence.push_back( faction );
			m_levelMusicSequence.push_back( factionMusic );
			levelIter = levelIter->NextSiblingElement();
		}
	}

	g_theAudio->CreateOrGetSound( m_bossMusic );
	ReadGameSaveData();
	SetUpRooms( m_levelSequence[m_curLevel] );
	SpawnPlayerToGame();
	EnterRoom( m_curRoom, RoomDirection::CENTER );
	BeginGame();
}

void Game::Update()
{
	if (m_goToNextFloorNextFrame) {
		m_goToNextFloorNextFrame = false;
		GoToNextFloor();
		return;
	}
	float deltaSeconds = m_gameClock->GetDeltaSeconds();
	if (m_state == GameState::IN_ROOM) {
		HandleKeys();
		UpdateAllGameObjects( deltaSeconds );
		UpdateCollisions();
		RemoveGarbageGameObjects();

		if (CanLeaveCurrentRoom() && m_clearTheRoomFisrtTime && m_curRoom->m_def.m_type == RoomType::ENEMY) {
			RoomClearCallBack( m_curRoom );
			m_clearTheRoomFisrtTime = false;
			if (m_curRoom->IsBossRoom()) {
				m_portal = (NextFloorPortal*)SpawnEffectToGame( EffectType::FloorPortal, m_curRoom->m_bounds.m_mins + Vec2( 100.f, 75.f ) );
			}
		}

		if (m_cameraShakeTimer->HasStartedAndNotPeriodElapsed()) {
			// update camera shake
			if (m_cameraPerShakeTimer->DecrementPeriodIfElapsed()) {
				m_worldCamera.SetCenter( m_cameraCenter + AABB2( Vec2( -2.f, -2.f ), Vec2( 2.f, 2.f ) ).GetRandomPointInside() );
			}
		}
		else {
			m_cameraPerShakeTimer->Stop();
			if (m_cameraShakeThisFrame) {
				if (m_cameraOneShakeTimer->HasStartedAndNotPeriodElapsed()) {
					if (m_cameraOneShakeTimer->GetElapsedFraction() < 0.5f) {
						m_worldCamera.SetCenter( m_cameraCenter + m_cameraFrameShakeDisplacement * Hesitate5(m_cameraOneShakeTimer->GetElapsedFraction()) * 2.f );
					}
					else {
						m_worldCamera.SetCenter( m_cameraCenter + m_cameraFrameShakeDisplacement * (1.f - Hesitate5( m_cameraOneShakeTimer->GetElapsedFraction() )) * 2.f );
					}
				}
				else {
					m_cameraShakeThisFrame = false;
					m_worldCamera.SetCenter( m_cameraCenter );
					m_cameraOneShakeTimer->Stop();
				}
			}
			else {
				m_worldCamera.SetCenter( m_cameraCenter );
			}
		}
	}
	else if (m_state == GameState::GO_TO_NEXT_ROOM) {
		if (m_goToNextRoomTimer->HasStartedAndNotPeriodElapsed()) {
			m_worldCamera.SetCenter( Interpolate( m_cameraCenter, m_goToNextRoomCameraTargetCenter, SmoothStep5( m_goToNextRoomTimer->GetElapsedFraction() ) ) );
		}
		else {
			m_state = GameState::IN_ROOM;
			m_cameraCenter = m_goToNextRoomCameraTargetCenter;
		}
	}
}

void Game::Render() const
{
	// Game Camera
	g_theRenderer->BeginCamera( m_worldCamera );
	RenderAllGameObjects();
	if (m_debugMode) {
		DebugRenderAllGameObjects();
	}
	g_theRenderer->EndCamera( m_worldCamera );
	ParticleSystem2DRender( m_worldCamera );
	// UI Camera
	g_theRenderer->BeginCamera( m_screenCamera );
	RenderUI();
	g_theRenderer->EndCamera( m_screenCamera );

	for (auto controller : m_controllers) {
		if (controller && controller->IsPlayer()) {
			((PlayerController*)controller)->RenderUI();
		}
	}
}

void Game::HandleKeys()
{
	m_gameClock->SetTimeScale( 1.f );

#ifdef DEBUG_MODE
	if (g_theInput->WasKeyJustPressed( KEYCODE_F1 )) // F1 debug mode
	{
		m_debugMode = !m_debugMode;
	}
	//if (g_theInput->WasKeyJustPressed( KEYCODE_F8 )) { // F8 to restart the game
	//	delete g_theGame;
	//	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Restart Game" );
	//	g_theGame = new Game();
	//	g_theGame->Startup();
	//}

	if (g_theInput->WasKeyJustPressed( 0x4F )) {// O key run a single frame and pauses
		m_gameClock->StepSingleFrame();
	}

	if (g_theInput->IsKeyDown( 0x54 )) // T key slow the game
	{
		m_gameClock->SetTimeScale( 0.1f );
	}

	if (g_theInput->WasKeyJustPressed( 0x50 )) // P key pause the game; handle the pause problem
	{
		if (!m_renderItemScreen && !m_renderMapScreen) {
			m_gameClock->TogglePause();
		}
	}
#endif // DEBUG_MODE

	// show the map
	if (g_theInput->WasKeyJustPressed( PLAYER_MAP_SCREEN_KEYCODE )) {
		if (!m_renderItemScreen) {
			m_renderMapScreen = !m_renderMapScreen;
			m_gameClock->TogglePause();
		}
	}

	// show items
	if (g_theInput->WasKeyJustPressed( PLAYER_ITEM_SCREEN_KEYCODE )) {
		if (!m_renderMapScreen) {
			m_renderItemScreen = !m_renderItemScreen;
			m_gameClock->TogglePause();
		}
	}

	if ((g_theInput->WasKeyJustPressed( KEYCODE_ESC ) || g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_BACK )))
	{
		if (g_theGame) {
			if (m_renderItemScreen) {
				m_renderItemScreen = false;
				m_gameClock->TogglePause();
			}
			else if (m_renderMapScreen) {
				m_renderMapScreen = false;
				m_gameClock->TogglePause();
			}
			else if (m_gameClock->IsPaused() && m_renderOptionsMenu) {
				m_renderOptionsMenu = false;
				m_gameClock->TogglePause();
				//g_theApp->GoToAppMode( AppState::ATTRACT_MODE );
			}
			else {
				m_renderOptionsMenu = true;
				m_gameClock->TogglePause();
			}
		}
	}

	if (m_renderItemScreen) {
		if (g_theInput->WasKeyJustPressed( PLAYER_UP_KEYCODE )) {
			if (m_insepctingItem >= 4) {
				m_insepctingItem -= 4;
			}
		}
		if (g_theInput->WasKeyJustPressed( PLAYER_LEFT_KEYCODE )) {
			if (m_insepctingItem > 0) {
				m_insepctingItem--;
			}
		}
		if (g_theInput->WasKeyJustPressed( PLAYER_DOWN_KEYCODE )) {
			if (m_insepctingItem + 4 < (int)m_playerShip->m_itemList.size()) {
				m_insepctingItem += 4;
			}
		}
		if (g_theInput->WasKeyJustPressed( PLAYER_RIGHT_KEYCODE )) {
			if (m_insepctingItem < (int)m_playerShip->m_itemList.size() - 1) {
				m_insepctingItem++;
			}
		}
	}

	if (g_theInput->WasKeyJustPressed( DEBUG_AUTO_SHOOT_MAIN_KEYCODE )) {
		g_theApp->m_autoShootMainWeapon = !g_theApp->m_autoShootMainWeapon;
		if (g_theApp->m_autoShootMainWeapon) {
			DebugAddMessage( Stringf( "Auto Shoot Main Weapon: On" ), 3.f, Rgba8( 255, 255, 0 ), Rgba8( 255, 255, 0 ) );
		}
		else {
			DebugAddMessage( Stringf( "Auto Shoot Main Weapon: Off" ), 3.f, Rgba8( 255, 255, 0 ), Rgba8( 255, 255, 0 ) );
		}
	}
	if (g_theInput->WasKeyJustPressed( DEBUG_AUTO_SHOOT_SUB_KEYCODE )) {
		g_theApp->m_autoShootSubWeapon = !g_theApp->m_autoShootSubWeapon;
		if (g_theApp->m_autoShootSubWeapon) {
			DebugAddMessage( Stringf( "Auto Shoot Sub-weapon: On" ), 3.f, Rgba8( 255, 255, 0 ), Rgba8( 255, 255, 0 ) );
		}
		else {
			DebugAddMessage( Stringf( "Auto Shoot Sub-weapon: Off" ), 3.f, Rgba8( 255, 255, 0 ), Rgba8( 255, 255, 0 ) );
		}
	}

	if (m_renderMapScreen) {
		if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE )) {
			Vec2 mouseUIPos = g_theGame->m_screenCamera.GetCursorWorldPosition( g_window->GetNormalizedCursorPos() );
			for (int i = 0; i < m_roomDimensions.x; i++) {
				for (int j = 0; j < m_roomDimensions.y; j++) {
					Room* room = GetRoomAtCoords( IntVec2( i, j ) );
					if (room && room->m_boundsOnMap.IsPointInside( mouseUIPos ) && !room->m_isFirstEnter && CanLeaveCurrentRoom()) {
						m_playerShip->m_position = room->m_bounds.GetCenter();
						m_renderMapScreen = false;
						m_gameClock->TogglePause();
						EnterRoom( room, GetOppositeRoomDir( RoomDirection::CENTER ) );
						m_cameraCenter = room->m_bounds.GetCenter();
						m_state = GameState::IN_ROOM;
						m_playerShip->CorrectAllFollowers();
					}
				}
			}
		}
	}

	if (m_renderOptionsMenu) {

		if (g_theInput->WasKeyJustPressed( 'W' ) || g_theInput->WasKeyJustPressed( KEYCODE_UPARROW )) {
			m_curHoveringButtom = (m_curHoveringButtom - 1 + m_numOfButtons) % m_numOfButtons;
		}
		if (g_theInput->WasKeyJustPressed( 'S' ) || g_theInput->WasKeyJustPressed( KEYCODE_DOWNARROW )) {
			m_curHoveringButtom = (m_curHoveringButtom + 1) % m_numOfButtons;
		}
		if (g_theInput->WasKeyJustPressed( KEYCODE_SPACE ) || g_theInput->WasKeyJustPressed( KEYCODE_ENTER )) {
			g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Click.wav" ), false, g_theApp->m_soundVolume * 0.6f );
			if (m_curHoveringButtom == 0) {
				m_renderOptionsMenu = false;
				if (m_gameClock->IsPaused()) {
					m_gameClock->TogglePause();
				}
			}
			else if (m_curHoveringButtom == 1) {
				g_theApp->GoToAppMode( AppState::SETTINGS_MODE );
				g_theApp->m_settingsScreen->m_isFromGame = true;
			}
			else if (m_curHoveringButtom == 2) {
				g_theApp->GoToAppMode( AppState::ATTRACT_MODE );
			}
			else if (m_curHoveringButtom == 3) {
				g_theApp->m_isQuitting = true;
			}
		}
	}
}

Entity* Game::GetPlayerEntity() const
{
	return ((Controller*)m_playerController)->GetControlledEntity();
}

PlayerShip* Game::GetPlayerObject() const
{
	return m_playerShip;
}

void Game::SpawnPlayerToGame()
{
	Entity* player = SpawnEntityToGame( EntityDefinition::GetDefinition( "Playership" ), Vec2( 100.f, 50.f ), 0.f, Vec2() );
	m_playerShip = (PlayerShip*)player;
	m_playerController = new PlayerController();
	m_playerController->Possess( player );
	m_controllers.push_back( m_playerController );
	
	/*
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondStriker" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondStriker" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondStriker" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );

	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondWarrior" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondWarrior" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondWarrior" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondRayShooter" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondRayShooter" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondRayShooter" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondReflector" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondReflector" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondReflector" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondShieldRayShooter" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondShieldRayShooter" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondShieldRayShooter" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondDoubleRayShooter" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondDoubleRayShooter" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	SpawnEntityToGame( EntityDefinition::GetDefinition( "DiamondDoubleRayShooter" ), m_worldCamera.m_cameraBox.GetRandomPointInside(), m_randNumGen->RollRandomFloatInRange( -180.f, 180.f ) );
	*/
}

Entity* Game::SpawnEntityToGame( EntityDefinition const& def, Vec2 const& position, float orientationDegrees, Vec2 const& initialVelocity )
{
	Entity* newEntity = nullptr;
	if (def.m_isAIEnabled) {
		if (def.m_aiBehavior == "DiamondWarrior"){
			newEntity = new DiamondWarrior( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "DiamondRayShooter"){
			newEntity = new DiamondRayShooter( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "DiamondStriker") {
			newEntity = new DiamondStriker( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "DiamondReflector") {
			newEntity = new DiamondReflector( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "DiamondDoubleRayShooter") {
			newEntity = new DiamondDoubleRayShooter( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "DiamondMiner") {
			newEntity = new DiamondMiner( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "DiamondBossAssassin") {
			newEntity = new DiamondBossAssassin( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "PlayerAsteriod") {
			newEntity = new PlayerAsteroid( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "LittleDemon") {
			newEntity = new LittleDemon( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "DiamondBossSuperMiner") {
			newEntity = new DiamondBossSuperMiner( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "DiamondBossRayChannel") {
			newEntity = new DiamondBossRayChannel( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "Chest") {
			newEntity = new Chest( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "GunShooter") {
			newEntity = new GunShooter( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "MachineGunShooter") {
			newEntity = new MachineGunShooter( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "SectorGunShooter") {
			newEntity = new SectorGunShooter( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "ShotGunShooter") {
			newEntity = new ShotGunShooter( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "Sniper") {
			newEntity = new Sniper( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "MissileShooter") {
			newEntity = new MissileShooter( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "GunTwinElder") {
			newEntity = new GunTwinElder( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "GunTwinYoung") {
			newEntity = new GunTwinYoung( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "SmallBacteria") {
			newEntity = new SmallBacteria( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "TinyBacteria") {
			newEntity = new TinyBacteria( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "MediumBacteria") {
			newEntity = new MediumBacteria( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "LargeBacteria") {
			newEntity = new LargeBacteria( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "BacteriaMothership") {
			newEntity = new BacteriaMothership( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "BacteriaSpawn") {
			newEntity = new BacteriaSpawn( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "BacteriaBreeder") {
			newEntity = new BacteriaBreeder( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "BacteriaSprayer") {
			newEntity = new BacteriaSprayer( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "BacteriaFusion") {
			newEntity = new BacteriaFusion( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "BacteriaBossTheGreatFusion") {
			newEntity = new BacteriaBossTheGreatFusion( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "DiagonalRetinue") {
			newEntity = new DiagonalRetinue( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "LaserWingPlane") {
			newEntity = new LaserWingPlane( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "WingPlane") {
			newEntity = new WingPlane( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "MimicChest") {
			newEntity = new MimicChest( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "GunAudience") {
			newEntity = new GunAudience( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "BossDoubleGun") {
			newEntity = new BossDoubleGun( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "BacteriaBossMarshKing") {
			newEntity = new BacteriaBossMarshKing( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "InteractableMachine") {
			newEntity = new InteractableMachine( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "FinalBossMercyKiller") {
			newEntity = new BossMercyKiller( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "BossArmsMaster") {
			newEntity = new BossArmsMaster( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "BacteriaBossThreeSisters") {
			newEntity = new BacteriaBossThreeSisters( def, position, orientationDegrees, initialVelocity );
		}
		else if (def.m_aiBehavior == "BacteriaBossSister") {
			newEntity = new BacteriaBossSister( def, position, orientationDegrees, initialVelocity );
		}
		else {
			ERROR_AND_DIE( Stringf( "Cannot find an entity AI behavior called %s", def.m_aiBehavior.c_str() ) );
		}
	}
	else {
		newEntity = new PlayerShip( def, position, orientationDegrees, initialVelocity );
	}
	AddEntityToGame( newEntity );
	newEntity->BeginPlay();
	return newEntity;
}

Projectile* Game::SpawnProjectileToGame( ProjectileDefinition const& def, Vec2 const& position, float orientationDegrees, Vec2 const& initialVelocity /*= Vec2( 0.f, 0.f ) */ )
{
	Projectile* newProjectile = nullptr;
	if (def.m_name == "Bullet") {
		newProjectile = new PlayerBullet( def, position, orientationDegrees, initialVelocity );
	}
	else if (def.m_name == "Shuriken") {
		newProjectile = new Shuriken( def, position, orientationDegrees, initialVelocity );
	}
	else if (def.m_name == "Rocket") {
		newProjectile = new Rocket( def, position, orientationDegrees, initialVelocity );
	}
	else if (def.m_name == "DemonBullet") {
		newProjectile = new DemonBullet( def, position, orientationDegrees, initialVelocity );
	}
	else if (def.m_name == "EnemyBullet") {
		newProjectile = new EnemyBullet( def, position, orientationDegrees, initialVelocity );
	}
	else if (def.m_name == "CurveMissile") {
		newProjectile = new CurveMissile( def, position, orientationDegrees, initialVelocity );
	}
	else if (def.m_name == "CoinBullet") {
		newProjectile = new CoinBullet( def, position, orientationDegrees, initialVelocity );
	}
	else if (def.m_name == "SharpenedObsidian") {
		newProjectile = new SharpenedObsidian( def, position, orientationDegrees, initialVelocity );
	}
	else if (def.m_name == "Arrow") {
		newProjectile = new Arrow( def, position, orientationDegrees, initialVelocity );
	}
	else {
		ERROR_AND_DIE( Stringf( "No such projectile name %s", def.m_name.c_str() ) );
	}

	AddProjectileToGame( newProjectile );
	return newProjectile;
}

StarshipEffect* Game::SpawnEffectToGame( EffectType type, Vec2 const& position, float orientationDegrees /*= 0.f*/, Vec2 const& initialVelocity /*= Vec2( 0.f, 0.f ) */, bool pushBack )
{
	StarshipEffect* effect = nullptr;
	if (type == EffectType::Reward) {
		effect = new StarshipReward( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::Shield) {
		effect = new StarshipShield( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::Laser) {
		effect = new RayLaser( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::Slash) {
		effect = new SlashEffect( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::Door) {
		effect = new LevelPortal( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::Missile) {
		effect = new Missle( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::PlayerShield) {
		effect = new PlayerShield( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::SubWeaponLaser) {
		effect = new StarshipLaser( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::StarshipMine) {
		effect = new StarshipMine( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::HealthPickup) {
		effect = new HealthPickup( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::ArmorPickup) {
		effect = new ArmorPickup( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::PersistentRay) {
		effect = new PersistentRay( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::FloorPortal) {
		effect = new NextFloorPortal( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::BacteriaDrop) {
		effect = new BacteriaDrop( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::BacteriaSap) {
		effect = new BacteriaSap( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::SprayAttack) {
		effect = new SprayAttack( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::SectorSprayAttack) {
		effect = new SectorSprayAttack( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::SunFlame) {
		effect = new SunFlame( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::ElectricChain) {
		effect = new ElectricChain( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::BacteriaLick) {
		effect = new BacteriaLick( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::MercyKillerHarmer) {
		effect = new MercyKillerHarmer( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::MeteorShower) {
		effect = new MeteorShower( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::Meteor) {
		effect = new Meteor( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::MercyKillerCage) {
		effect = new MercyKillerCage( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::MercyKillerCageChain) {
		effect = new MercyKillerCageChain( position, orientationDegrees, initialVelocity );
	}
	else if (type == EffectType::MercyKillerRespawn) {
		effect = new MercyKillerRespawn( position, orientationDegrees, initialVelocity );
	}

	if (!pushBack) {
		for (int i = 0; i < (int)m_effectArray.size(); i++) {
			if (m_effectArray[i] == nullptr) {
				m_effectArray[i] = effect;
				return effect;
			}
		}
	}
	m_effectArray.push_back( effect );
	return effect;
}

void Game::AddEntityToGame( Entity* entityToAdd )
{
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i] == nullptr) {
			m_entityArray[i] = entityToAdd;
			return;
		}
	}
	m_entityArray.push_back( entityToAdd );
}

void Game::RemoveEntityFromGame( Entity* entityToRemove )
{
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i] == entityToRemove) {
			m_entityArray[i] = nullptr;
			return;
		}
	}
}

StarshipRayCastResult Game::RayCastVsEntities( Vec2 position, Vec2 direction, float maxDist, Entity* ignoreEntity, std::string const& targetFaction, bool targetReflector /*= false */ )
{
	RayCastResult2D res;
	StarshipRayCastResult finalRes;
	for (auto entity : m_entityArray) {
		if(entity && entity->IsAlive() && entity != ignoreEntity && (entity->m_def.m_faction == targetFaction || (targetReflector && entity->m_def.m_isReflector)) 
			&& RayCastVsDisc2D( res, position, direction, maxDist, entity->m_position, entity->m_physicsRadius )) {
			if (res.m_impactDist < finalRes.m_impactDist || !finalRes.m_didImpact) {
				finalRes = StarshipRayCastResult( res );
				finalRes.m_entityHit = entity;
				finalRes.m_isEntityHit = true;
			}
		}
	}
	return finalRes;
}

std::vector<DiamondReflector*> Game::GetAllDiamondReflectors() const
{
	std::vector<DiamondReflector*> retVector;
	for (auto entity : m_entityArray) {
		if (entity && entity->IsAlive() && entity->m_def.m_aiBehavior == "DiamondReflector") {
			retVector.push_back( (DiamondReflector*)entity );
		}
	}
	return retVector;
}

void Game::StartCameraShake()
{
	m_cameraShakeTimer->Start();
	m_cameraPerShakeTimer->Start();
}

void Game::SetOneDirCameraShake( Vec2 const& displacement, float seconds )
{
	m_cameraShakeThisFrame = true;
	m_cameraFrameShakeDisplacement = displacement;
	m_cameraOneShakeTimer->SetPeriodSeconds( seconds );
	m_cameraOneShakeTimer->Start();
}

Weapon* Game::CreateWeaponComponent( WeaponDefinition const& def, Entity* owner ) const
{
	if (def.m_behavior == "BulletGun") {
		return new BulletGun( def, owner );
	}
	else if (def.m_behavior == "RayShooter") {
		return new RayShooter( def, owner );
	}
	else if (def.m_behavior == "Machete") {
		return new Machete( def, owner );
	}
	else if (def.m_behavior == "RocketLauncher") {
		return new RocketShooter( def, owner );
	}
	else if (def.m_behavior == "EnemyBulletGun") {
		return new EnemyBulletGun( def, owner );
	}
	else if (def.m_behavior == "MissileGun") {
		return new MissileGun( def, owner );
	}
	else if (def.m_behavior == "Spray") {
		return new Spray( def, owner );
	}
	else if (def.m_behavior == "CoinGun") {
		return new CoinGun( def, owner );
	}
	return nullptr;
}

Room* Game::GetRoomInDirectionByCurRoom( RoomDirection dir ) const
{
	IntVec2 curCoords = m_curRoom->m_coords;
	if (dir == RoomDirection::UP) {
		return GetRoomAtCoords( curCoords + IntVec2( 0, 1 ) );
	}
	else if (dir == RoomDirection::DOWN) {
		return GetRoomAtCoords( curCoords + IntVec2( 0, -1 ) );
	}
	else if (dir == RoomDirection::LEFT) {
		return GetRoomAtCoords( curCoords + IntVec2( -1, 0 ) );
	}
	else if (dir == RoomDirection::RIGHT) {
		return GetRoomAtCoords( curCoords + IntVec2( 1, 0 ) );
	}

	return nullptr;
}

Room* Game::GetRoomAtCoords( IntVec2 const& coords ) const
{
	if (!IsRoomCoordsInBounds(coords)) {
		return nullptr;
	}
	return m_roomMap[coords.x + coords.y * m_roomDimensions.x];
}

bool Game::CanLeaveCurrentRoom() const
{
	if (m_curRoom->m_itemChooseMode) {
		return false;
	}
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i] && m_entityArray[i]->m_def.m_isEnemy) {
			return false;
		}
	}
	for (int i = 0; i < (int)m_effectArray.size(); i++) {
		if (m_effectArray[i] && m_effectArray[i]->m_type == EffectType::BacteriaDrop) {
			return false;
		}
	}
	return true;
}

void Game::BeginGame()
{
	//for (int i = 0; i < (int)m_effectArray.size(); i++) {
	//	if (m_effectArray[i]) {
	//		m_effectArray[i]->BeginPlay();
	//	}
	//}

	//for (int i = 0; i < (int)m_entityArray.size(); i++) {
	//	if (m_entityArray[i]) {
	//		m_entityArray[i]->BeginPlay();
	//	}
	//}
}

void Game::SetUpRooms( std::string const& faction, int level )
{
	if (m_backgroundMusicID != MISSING_SOUND_ID) {
		g_theAudio->StopSound( m_backgroundMusicID );
	}
	m_backgroundMusicID = g_theAudio->StartSound( g_theAudio->CreateOrGetSound( m_levelMusicSequence[level] ), true, g_theApp->m_musicVolume * 0.6f );
	
	if (g_theApp->m_bossRushFlag) {
		m_roomMap.resize( m_roomDimensions.x * m_roomDimensions.y, nullptr );

		m_roomMap[4 + 4 * m_roomDimensions.x] = new Room( RoomDefinition::GetDefinition( "startRoom" ), IntVec2( 4, 4 ) );
		m_curRoom = GetRoomAtCoords( IntVec2( 4, 4 ) );

		Vec2 margin( 10.f, 10.f );
		Vec2 size( 80.f, 60.f );
		Vec2 startPos( 400.f, 90.f );
		Vec2 leftBottomPoint = Vec2( (margin + size).x * 4, (margin + size).y * 4 ) + startPos;
		m_curRoom->m_boundsOnMap = AABB2( leftBottomPoint, leftBottomPoint + size );

		if (faction == "Final") {
			RandomSetRoom( "Shop", 1, level + 1 );
			RandomSetRoom( "FinalBoss", 1, 5 );
			return;
		}
		RandomSetRoom( "Chest", 1, level + 1 );
		RandomSetRoom( "Shop", 1, level + 1 );
		if (level == 0) {
			RandomSetRoom( "Chest", 1, 2 );
			RandomSetRoom( faction, 3, 5 );
		}
		else {
			RandomSetRoom( faction, 3, 5 );
		}
	}
	else {
		if (faction == "Final") {
			SetUpLastFloor( level );
			return;
		}
		m_roomMap.resize( m_roomDimensions.x * m_roomDimensions.y, nullptr );

		m_roomMap[4 + 4 * m_roomDimensions.x] = new Room( RoomDefinition::GetDefinition( "startRoom" ), IntVec2( 4, 4 ) );
		m_curRoom = GetRoomAtCoords( IntVec2( 4, 4 ) );

		Vec2 margin( 10.f, 10.f );
		Vec2 size( 80.f, 60.f );
		Vec2 startPos( 400.f, 90.f );
		Vec2 leftBottomPoint = Vec2( (margin + size).x * 4, (margin + size).y * 4 ) + startPos;
		m_curRoom->m_boundsOnMap = AABB2( leftBottomPoint, leftBottomPoint + size );

		//RandomSetRoom( "DangerousChest", 1, level + 1 );
		if (level != 0) {
			float rnd = GetRandGen()->RollRandomFloatZeroToOne();
			if (rnd < 0.1f) {
				RandomSetRoom( "DangerousChest", 1, level + 1 );
			}
			else {
				RandomSetRoom( "Chest", 1, level + 1 );
			}
		}
		else {
			RandomSetRoom( "Chest", 1, level + 1 );
		}

		RandomSetRoom( faction, 5, 1 );
		RandomSetRoom( "Shop", 1, level + 1 );
		RandomSetRoom( "Event", 1, 1 );
		RandomSetRoom( faction, 2 + level, 2 );
		RandomSetRoom( faction, 1 + level, 3 );
		RandomSetRoom( faction, 1, 5 );

		// blind map
		if (m_playerShip && m_playerShip->m_blindWithRoom) {
			RandomSetRoom( faction, 5, -1 );
		}
	}

}

void Game::SetUpLastFloor( int level /*= 0 */ )
{
	m_roomMap.resize( m_roomDimensions.x * m_roomDimensions.y, nullptr );

	m_roomMap[4 + 4 * m_roomDimensions.x] = new Room( RoomDefinition::GetDefinition( "startRoom" ), IntVec2( 4, 4 ) );
	m_curRoom = GetRoomAtCoords( IntVec2( 4, 4 ) );

	Vec2 margin( 10.f, 10.f );
	Vec2 size( 80.f, 60.f );
	Vec2 startPos( 400.f, 90.f );
	Vec2 leftBottomPoint = Vec2( (margin + size).x * 4, (margin + size).y * 4 ) + startPos;
	m_curRoom->m_boundsOnMap = AABB2( leftBottomPoint, leftBottomPoint + size );

	//RandomSetRoom( "DangerousChest", 1, level + 1 );
	if (level != 0) {
		float rnd = GetRandGen()->RollRandomFloatZeroToOne();
		if (rnd < 0.1f) {
			RandomSetRoom( "DangerousChest", 1, level + 1 );
		}
		else {
			RandomSetRoom( "Chest", 1, level + 1 );
		}
	}
	else {
		RandomSetRoom( "Chest", 1, level + 1 );
	}
	RandomSetRoom( "Event", 1, 1 );
	RandomSetRoom( "Shop", 1, level + 1 );
	for (int i = 0; i < level; i++) {
		RandomSetRoom( GetRandomFaction(), 1, 2 );
	}
	for (int i = 0; i < level + 3; i++) {
		RandomSetRoom( GetRandomFaction(), 1, 3 );
	}
	RandomSetRoom( "FinalBoss", 1, 5 );

	// blind map
	if (m_playerShip && m_playerShip->m_blindWithRoom) {
		RandomSetRoom( GetRandomFaction(), 5, -1 );
	}
}

void Game::UpdateAllGameObjects( float deltaSeconds )
{
	for (int i = 0; i < (int)m_effectArray.size(); i++) {
		if (m_effectArray[i]) {
			m_effectArray[i]->Update( deltaSeconds );
		}
	}
	for (int i = 0; i < (int)m_projectileArray.size(); i++) {
		if (m_projectileArray[i]) {
			m_projectileArray[i]->Update( deltaSeconds );
		}
	}
	for (int i = 0; i < (int)m_controllers.size(); i++) {
		if (m_controllers[i]) {
			m_controllers[i]->Update( deltaSeconds );
		}
	}
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i]) {
			m_entityArray[i]->Update( m_entityArray[i]->m_clock->GetDeltaSeconds() );
		}
	}
}

void Game::RemoveGarbageGameObjects()
{
	for (int i = 0; i < (int)m_effectArray.size(); i++) {
		if (m_effectArray[i] && m_effectArray[i]->m_isGarbage) {
			delete m_effectArray[i];
			m_effectArray[i] = nullptr;
		}
	}
	for (int i = 0; i < (int)m_projectileArray.size(); i++) {
		if (m_projectileArray[i] && m_projectileArray[i]->m_isGarbage) {
			delete m_projectileArray[i];
			m_projectileArray[i] = nullptr;
		}
	}
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i] && m_entityArray[i]->m_isGarbage) {
			if (m_playerShip->m_bloodThirst && m_entityArray[i]->m_hasReward && m_entityArray[i]->m_def.m_faction != "Player") {
				if (m_entityArray[i]->m_def.m_enemyLevel == 5) {
					m_playerShip->m_mainWeaponDamage += 0.1f;
				}
				else {
					m_playerShip->m_mainWeaponDamage += 0.01f;
				}
			}
			if (m_entityArray[i]->m_hasReward && m_entityArray[i]->m_def.m_faction != "Player") {
				GenerateHealthOrArmorPickup( m_entityArray[i]->m_position);
			}
			delete m_entityArray[i];
			m_entityArray[i] = nullptr;
		}
	}
}

void Game::RenderAllGameObjects() const
{
	RenderItemsInRoom();

	for (int i = 0; i < (int)m_effectArray.size(); i++) {
		if (m_effectArray[i] && m_effectArray[i]->m_renderBeforeEntity) {
			m_effectArray[i]->Render();
		}
	}
	for (int i = 0; i < (int)m_projectileArray.size(); i++) {
		if (m_projectileArray[i]) {
			m_projectileArray[i]->Render();
		}
	}
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i]) {
			m_entityArray[i]->Render();
		}
	}
	for (int i = 0; i < (int)m_effectArray.size(); i++) {
		if (m_effectArray[i] && !m_effectArray[i]->m_renderBeforeEntity) {
			m_effectArray[i]->Render();
		}
	}
	for (int i = 0; i < (int)m_controllers.size(); i++) {
		if (m_controllers[i]) {
			m_controllers[i]->Render();
		}
	}
}

void Game::RenderItemsInRoom() const
{
	// render items in room
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;
	verts.reserve( 1000 );
	textVerts.reserve( 1000 );
	if (m_isChoosingItems) {
		for (int i = 0; i < (int)m_showingItems.size(); i++) {
			//ItemDefinition::GetDefinition( m_showingItems[i] ).AddVertsForItem( verts );
			ItemDefinition const& def = ItemDefinition::GetDefinition( m_showingItems[i] );

			if (m_playerShip->m_blindWithItem) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( def.m_position - Vec2( 4.f, 4.f ), def.m_position + Vec2( 4.f, 4.f ) ), 3.f, Stringf( "?" ), Rgba8( 255, 0, 0 ) );
			}
			else {
				ItemDefinition::GetDefinition( m_showingItems[i] ).RenderItem( AABB2( def.m_position - Vec2( 3.f, 3.f ), def.m_position + Vec2( 3.f, 3.f ) ) );
			}
			//else {
			//	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( def.m_position - Vec2( 3.f, 3.f ), def.m_position + Vec2( 3.f, 3.f ) ), 3.f, Stringf( "%d", def.m_id ), Rgba8( 0, 0, 0 ) );
			//}
			//AddVertsForAABB2D( verts, AABB2( def.m_position - Vec2( 3.f, 3.f ), def.m_position + Vec2( 3.f, 3.f ) ), Rgba8::WHITE );
			if (IsInShop() && def.m_isSold) {
				int price = def.GetPrice();
				Rgba8 textColor;
				if (price <= m_playerController->m_reward) {
					if (def.m_isDiscount) {
						textColor = Rgba8( 255, 255, 0 );
					}
					else {
						textColor = Rgba8::WHITE;
					}
				}
				else {
					textColor = Rgba8( 255, 0, 0 );
				}

				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( def.m_position + Vec2( -1.f, -8.f ), def.m_position + Vec2( 6.f, -4.f ) ), 4.f, Stringf( "%d", price ), textColor );
				AddVertsForDisc2D( verts, def.m_position + Vec2( -3.f, -6.f ), 1.f, Rgba8( 255, 255, 0 ) );
			}
		}
	}
	if (IsInShop()) {
		std::vector<Vertex_PCU> healthVerts;
		healthVerts.reserve( 100 );
		float modifier = m_playerShip->m_customerCard ? 0.5f : 1.f;
		constexpr float healthHeight = 7.f;
		constexpr float healthWidth = 7.f;
		constexpr float marginPercentage = 0.1f;
		if (m_curRoom->m_hasHealth) {
			AddVertsForAABB2D( healthVerts, AABB2( m_curRoom->m_healthLBPos + Vec2( healthWidth, healthHeight ) * marginPercentage * 0.5f
				, m_curRoom->m_healthLBPos + Vec2( healthWidth, healthHeight ) * (1.f - marginPercentage) ), Rgba8::WHITE, g_pickupsSprites->GetSpriteUVs( 0 ) );
			int price = int(m_curRoom->m_healthPrice * modifier);
			Rgba8 textColor;
			if (price <= m_playerController->m_reward) {
				textColor = Rgba8::WHITE;
			}
			else {
				textColor = Rgba8( 255, 0, 0 );
			}
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( m_curRoom->m_healthLBPos + Vec2( 1.f, -4.f ), m_curRoom->m_healthLBPos + Vec2( 6.f, -1.f ) ), 3.f, Stringf( "%d", price ), textColor );
			AddVertsForDisc2D( verts, m_curRoom->m_healthLBPos + Vec2( 0.f, -2.5f ), 0.8f, Rgba8( 255, 255, 0 ) );
		}
		if (m_curRoom->m_hasArmor) {
			AddVertsForAABB2D( healthVerts, AABB2( m_curRoom->m_armorLBPos + Vec2( healthWidth, healthHeight ) * marginPercentage * 0.5f
				, m_curRoom->m_armorLBPos + Vec2( healthWidth, healthHeight ) * (1.f - marginPercentage) ), Rgba8::WHITE, g_pickupsSprites->GetSpriteUVs( 1 ) );
			int price = int(m_curRoom->m_armorPrice * modifier);
			Rgba8 textColor;
			if (price <= m_playerController->m_reward) {
				textColor = Rgba8::WHITE;
			}
			else {
				textColor = Rgba8( 255, 0, 0 );
			}
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( m_curRoom->m_armorLBPos + Vec2( 1.f, -4.f ), m_curRoom->m_armorLBPos + Vec2( 6.f, -1.f ) ), 3.f, Stringf( "%d", price ), textColor );
			AddVertsForDisc2D( verts, m_curRoom->m_armorLBPos + Vec2( 0.f, -2.5f ), 0.8f, Rgba8( 255, 255, 0 ) );
		}
		if (m_curRoom->m_hasMaxHealth) {
			AddVertsForAABB2D( healthVerts, AABB2( m_curRoom->m_maxHealthLBPos + Vec2( healthWidth, healthHeight ) * marginPercentage * 0.5f
				, m_curRoom->m_maxHealthLBPos + Vec2( healthWidth, healthHeight ) * (1.f - marginPercentage) ), Rgba8::WHITE, g_pickupsSprites->GetSpriteUVs( 2 ) );
			int price = int(m_curRoom->m_maxHealthPrice * modifier);
			Rgba8 textColor;
			if (price <= m_playerController->m_reward) {
				textColor = Rgba8::WHITE;
			}
			else {
				textColor = Rgba8( 255, 0, 0 );
			}
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( m_curRoom->m_maxHealthLBPos + Vec2( 1.f, -4.f ), m_curRoom->m_maxHealthLBPos + Vec2( 6.f, -1.f ) ), 3.f, Stringf( "%d", price ), textColor );
			AddVertsForDisc2D( verts, m_curRoom->m_maxHealthLBPos + Vec2( 0.f, -2.5f ), 0.8f, Rgba8( 255, 255, 0 ) );
		}
		if (m_curRoom->m_hasMaxArmor) {
			AddVertsForAABB2D( healthVerts, AABB2( m_curRoom->m_maxArmorLBPos + Vec2( healthWidth, healthHeight ) * marginPercentage * 0.5f
				, m_curRoom->m_maxArmorLBPos + Vec2( healthWidth, healthHeight ) * (1.f - marginPercentage) ), Rgba8::WHITE, g_pickupsSprites->GetSpriteUVs( 3 ) );
			int price = int(m_curRoom->m_maxArmorPrice * modifier);
			Rgba8 textColor;
			if (price <= m_playerController->m_reward) {
				textColor = Rgba8::WHITE;
			}
			else {
				textColor = Rgba8( 255, 0, 0 );
			}
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( m_curRoom->m_maxArmorLBPos + Vec2( 1.f, -4.f ), m_curRoom->m_maxArmorLBPos + Vec2( 6.f, -1.f ) ), 3.f, Stringf( "%d", price ), textColor );
			AddVertsForDisc2D( verts, m_curRoom->m_maxArmorLBPos + Vec2( 0.f, -2.5f ), 0.8f, Rgba8( 255, 255, 0 ) );
		}
		g_theRenderer->BindTexture( &g_pickupsSprites->GetTexture() );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( healthVerts );
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

}

void Game::DebugRenderAllGameObjects() const
{
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i]) {
			m_entityArray[i]->DebugRender();
		}
	}
	for (int i = 0; i < (int)m_projectileArray.size(); i++) {
		if (m_projectileArray[i]) {
			m_projectileArray[i]->DebugRender();
		}
	}
}

void Game::AddProjectileToGame( Projectile* projectile )
{
	for (int i = 0; i < (int)m_projectileArray.size(); i++) {
		if (m_projectileArray[i] == nullptr) {
			m_projectileArray[i] = projectile;
			return;
		}
	}
	m_projectileArray.push_back( projectile );
}

void Game::RenderUI() const
{
	if (m_renderOptionsMenu) {
		RenderOptionsScreen();
	}
	else if (m_renderMapScreen) {
		RenderMapScreen();
	}
	else if (m_renderItemScreen) {
		RenderItemScreen();
		for (int i = 0; i < (int)m_entityArray.size(); i++) {
			if (m_entityArray[i] && m_entityArray[i]->m_def.m_name == "InteractableMachine") {
				m_entityArray[i]->RenderUI();
			}
		}
	}
	else {
		for (int i = 0; i < (int)m_effectArray.size(); i++) {
			if (m_effectArray[i]) {
				m_effectArray[i]->RenderUI();
			}
		}
		for (int i = 0; i < (int)m_entityArray.size(); i++) {
			if (m_entityArray[i]) {
				m_entityArray[i]->RenderUI();
			}
		}
		std::vector<Vertex_PCU> verts;
		std::vector<Vertex_PCU> textVerts;

		if (IsInShop()) {
			NonItemMerchandise nonItem = m_curRoom->GetNearestInRangeNonItemMerchandise( GetPlayerEntity()->m_position );
			if (nonItem != NonItemMerchandise::None) {
				PlayerShip* player = (PlayerShip*)GetPlayerEntity();
				// render non-item texts
				if (nonItem == NonItemMerchandise::Armor && m_curRoom->m_hasArmor) {
					g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 690.f ), Vec2( 1600.f, 730.f ) ), 40.f, Stringf( "Armor Refresh" ) );
					g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 665.f ), Vec2( 1600.f, 685.f ) ), 20.f, Stringf( "+1 Armor" ) );
					int price = m_curRoom->m_armorPrice;
					if (price <= m_playerController->m_reward && !Starship_IsNearlyZero( player->m_curArmor - player->m_maxArmor )) {
						g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 120.f ), Vec2( 1300.f, 160.f ) ), 40.f, Stringf( "%c to Buy", PLAYER_INTERACT_KEYCODE ) );
					}
				}
				else if (nonItem == NonItemMerchandise::Health && m_curRoom->m_hasHealth) {
					g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 690.f ), Vec2( 1600.f, 730.f ) ), 40.f, Stringf( "Health Recovery" ) );
					g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 665.f ), Vec2( 1600.f, 685.f ) ), 20.f, Stringf( "+1 Health" ) );
					int price = m_curRoom->m_healthPrice;
					if (price <= m_playerController->m_reward && !Starship_IsNearlyZero( player->m_health - player->m_maxHealth )) {
						g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 120.f ), Vec2( 1300.f, 160.f ) ), 40.f, Stringf( "%c to Buy", PLAYER_INTERACT_KEYCODE ) );
					}
				}
				else if (nonItem == NonItemMerchandise::MaxArmor && m_curRoom->m_hasMaxArmor) {
					g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 690.f ), Vec2( 1600.f, 730.f ) ), 40.f, Stringf( "Extra Armor" ) );
					g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 665.f ), Vec2( 1600.f, 685.f ) ), 20.f, Stringf( "+1 Max Armor" ) );
					int price = m_curRoom->m_maxArmorPrice;
					if (price <= m_playerController->m_reward) {
						g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 120.f ), Vec2( 1300.f, 160.f ) ), 40.f, Stringf( "%c to Buy", PLAYER_INTERACT_KEYCODE ) );
					}
				}
				else if (nonItem == NonItemMerchandise::MaxHealth && m_curRoom->m_hasMaxHealth) {
					g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 690.f ), Vec2( 1600.f, 730.f ) ), 40.f, Stringf( "Extra Health" ) );
					g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 665.f ), Vec2( 1600.f, 685.f ) ), 20.f, Stringf( "+1 Max Health" ) );
					int price = m_curRoom->m_maxHealthPrice;
					if (price <= m_playerController->m_reward) {
						g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 120.f ), Vec2( 1300.f, 160.f ) ), 40.f, Stringf( "%c to Buy", PLAYER_INTERACT_KEYCODE ) );
					}
				}
			}
			else {
				// do not near non-item
				goto L1;
			}
		}
		else {
			// render item texts
		L1:
			int id = -1;
			float dist = FLT_MAX;
			GetTheNearestItemIDAndDist( id, dist, GetPlayerEntity()->m_position );
			if (dist <= 5.f) {
				ItemDefinition& def = ItemDefinition::GetDefinition( id );
				if (!m_playerShip->m_blindWithItem) {
					g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 690.f ), Vec2( 1600.f, 730.f ) ), 40.f, Stringf( "%s", ItemDefinition::GetDefinition( id ).m_name.c_str() ) );
					g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 665.f ), Vec2( 1600.f, 685.f ) ), 20.f, Stringf( "%s", ItemDefinition::GetDefinition( id ).m_description.c_str() ) );
				}
				if (IsInShop()) {
					int price = def.GetPrice();
					if (price <= m_playerController->m_reward && def.m_isSold) {
						g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 120.f ), Vec2( 1300.f, 160.f ) ), 40.f, Stringf( "%c to Buy Item", PLAYER_INTERACT_KEYCODE ) );
					}
					else if (!def.m_isSold) {
						g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 120.f ), Vec2( 1300.f, 160.f ) ), 40.f, Stringf( "%c to Collect Item", PLAYER_INTERACT_KEYCODE ) );
					}
				}
				else {
					g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 120.f ), Vec2( 1300.f, 160.f ) ), 40.f, Stringf( "%c to Collect Item", PLAYER_INTERACT_KEYCODE ) );
				}
			}
		}


		if (m_playerShip->m_timeStopTimer->HasStartedAndNotPeriodElapsed()) {
			AddVertsForAABB2D( verts, AABB2( Vec2( 0.f, 0.f ), Vec2( 1600.f, 800.f ) ), Rgba8( 128, 128, 128, 128 ) );
		}

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
	std::vector<Vertex_PCU> cursorVerts;
	Vec2 cursorPos = m_screenCamera.GetCursorWorldPosition( g_window->GetNormalizedCursorPos() );
	AddVertsForAABB2D( cursorVerts, AABB2( Vec2( cursorPos - Vec2( 15.f, 15.f ) ), Vec2( cursorPos + Vec2( 15.f, 15.f ) ) ), Rgba8::WHITE );

	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Reticle.png" ) );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( cursorVerts );
}

void Game::UpdateCollisions()
{
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i] && m_entityArray[i]->IsAlive()) {
			for (int j = i + 1; j < (int)m_entityArray.size(); j++) {
				if (m_entityArray[j] && m_entityArray[j]->IsAlive()) {
					CollideTwoEntities( m_entityArray[i], m_entityArray[j] );
				}
			}
		}
	}

	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i] && m_entityArray[i]->IsAlive()) {
			for (int j = 0; j < (int)m_projectileArray.size(); j++) {
				if (m_projectileArray[j] && m_projectileArray[j]->IsAlive()) {
					CollideEntityAndProjectile( m_entityArray[i], m_projectileArray[j] );
				}
			}
		}
	}

	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i] && m_entityArray[i]->IsAlive() && m_entityArray[i]->m_restrictIntoRoom) {
			if (m_entityArray[i]->m_position.x < m_entityArray[i]->m_cosmeticRadius + m_curRoom->m_bounds.m_mins.x) {
				m_entityArray[i]->m_position.x = m_entityArray[i]->m_cosmeticRadius + m_curRoom->m_bounds.m_mins.x;
			}
			if (m_entityArray[i]->m_position.x > m_curRoom->m_bounds.m_maxs.x - m_entityArray[i]->m_cosmeticRadius) {
				m_entityArray[i]->m_position.x = m_curRoom->m_bounds.m_maxs.x - m_entityArray[i]->m_cosmeticRadius;
			}
			if (m_entityArray[i]->m_position.y < m_entityArray[i]->m_cosmeticRadius + m_curRoom->m_bounds.m_mins.y) {
				m_entityArray[i]->m_position.y = m_entityArray[i]->m_cosmeticRadius + m_curRoom->m_bounds.m_mins.y;
			}
			if (m_entityArray[i]->m_position.y > m_curRoom->m_bounds.m_maxs.y - m_entityArray[i]->m_cosmeticRadius) {
				m_entityArray[i]->m_position.y = m_curRoom->m_bounds.m_maxs.y - m_entityArray[i]->m_cosmeticRadius;
			}
		}
	}
}

void Game::CollideTwoEntities( Entity* a, Entity* b )
{
	if (!a->m_def.m_enableCollision || !b->m_def.m_enableCollision) {
		return;
	}
	if (!a->IsInvincible() && !b->IsInvincible() && DoDiscsOverlap( a->m_position, a->m_physicsRadius, b->m_position, b->m_physicsRadius )
		&& a->m_def.m_faction != b->m_def.m_faction) {
		if (a == m_playerShip) {
			if (m_playerShip->m_collisionDamage) {
				b->BeAttackedOnce( 2.f, Vec2( 0.f, 0.f ), 0.3f, a );
			}
			if (m_playerShip->m_subWeaponShieldDashOn) {
				b->BeAttackedOnce( m_playerShip->m_curArmor, Vec2( 0.f, 0.f ), 2.f, PlayerShieldDashDamageSource );
				if (b->m_isDead && b->m_hasReward) {
					m_playerShip->GainArmor( 1.f );
				}
				return;
			}
		}
		else if (b == m_playerShip) {
			if (m_playerShip->m_collisionDamage) {
				a->BeAttackedOnce( 2.f, Vec2( 0.f, 0.f ), 0.3f, b );
			}
			if (m_playerShip->m_subWeaponShieldDashOn) {
				a->BeAttackedOnce( m_playerShip->m_curArmor, Vec2( 0.f, 0.f ), 2.f, PlayerShieldDashDamageSource );
				if (a->m_isDead && a->m_hasReward) {
					m_playerShip->GainArmor( 1.f );
				}
				return;
			}
		}
		if (a->m_def.m_dealDamageOnCollide) {
			b->BeAttacked( 1.f, Vec2( 0.f, 0.f ) );
		}
		if (b->m_def.m_dealDamageOnCollide) {
			a->BeAttacked( 1.f, Vec2( 0.f, 0.f ) );
		}
		PushDiscsOutOfEachOther2D( a->m_position, a->m_physicsRadius, b->m_position, b->m_physicsRadius );
	}
	else if(!a->IsInvincible() && !b->IsInvincible()) {
		PushDiscsOutOfEachOther2D( a->m_position, a->m_physicsRadius, b->m_position, b->m_physicsRadius );
	}
}

void Game::CollideEntityAndProjectile( Entity* entity, Projectile* projectile )
{
	if (entity->m_def.m_faction == projectile->m_faction) {
		return;
	}
	if (entity->IsInvincible()) {
		return;
	}
	if (entity == m_playerShip && m_playerShip->m_subWeaponShieldDashOn) {
		return;
	}
	if (DoDiscsOverlap( entity->m_position, entity->m_physicsRadius, projectile->m_position, projectile->m_physicsRadius )) {
		if (projectile->m_isPuncturing) {
			projectile->SpawnCollisionEffect();
			if (projectile->m_hasRangeDamage) {
				DealRangeDamage( projectile, true, 100.f );
			}
			else {
				entity->BeAttackedOnce( projectile->m_damage, (projectile->m_position - entity->m_position).GetNormalized(), 100.f, projectile, false, projectile->m_velocity );
			}
		}
		else {
			PushDiscOutOfFixedDisc2D( projectile->m_position, projectile->m_physicsRadius, entity->m_position, entity->m_physicsRadius );
			projectile->SpawnCollisionEffect();
			projectile->Die();
			if (projectile->m_hasRangeDamage) {
				DealRangeDamage( projectile );
			}
			else {
				entity->BeAttacked( projectile->m_damage, (projectile->m_position - entity->m_position).GetNormalized(), false, projectile->m_velocity );
			}
		}
		if (projectile->m_isPoisonous) {
			entity->m_poisonTimer += projectile->m_poisonTime;
			entity->m_isPoisoned = true;
		}
		if (projectile->m_isIced) {
			entity->m_slowTimer += projectile->m_slowTime;
			entity->m_isSlowed = true;
		}
	}
}

bool Game::IsRoomCoordsInBounds( IntVec2 const& coords ) const
{
	return !(coords.x < 0 || coords.y < 0 || coords.x > m_roomDimensions.x - 1 || coords.y > m_roomDimensions.y - 1);
}

bool Game::IsRoomAdjToOther( IntVec2 const& coords ) const
{
	if (GetRoomAtCoords( coords + IntVec2( 1, 0 ) )) {
		return true;
	}
	if (GetRoomAtCoords( coords + IntVec2( -1, 0 ) )) {
		return true;
	}
	if (GetRoomAtCoords( coords + IntVec2( 0, 1 ) )) {
		return true;
	}
	if (GetRoomAtCoords( coords + IntVec2( 0, -1 ) )) {
		return true;
	}
	return false;
}

void Game::RandomSetRoom( std::string const& faction, int num, int level )
{
	int rndX, rndY;
	IntVec2 coords;
	Vec2 margin( 10.f, 10.f );
	Vec2 size( 80.f, 60.f );
	Vec2 startPos( 400.f, 90.f );
	if (num + m_numOfRooms >= m_roomDimensions.x * m_roomDimensions.y) {
		return;
	}
	while (num > 0) {
		do {
			rndX = m_randNumGen->RollRandomIntLessThan( m_roomDimensions.x );
			rndY = m_randNumGen->RollRandomIntLessThan( m_roomDimensions.y );
			coords = IntVec2( rndX, rndY );
		} while (!IsRoomCoordsInBounds( coords ) || GetRoomAtCoords( coords ) || !IsRoomAdjToOther( coords ));
		num--;
		Room* room = new Room( RoomDefinition::GetRandomDefinition( faction, level ), IntVec2( rndX, rndY ) );
		m_roomMap[rndX + rndY * m_roomDimensions.x] = room;
		Vec2 leftBottomPoint = Vec2( (margin + size).x * rndX, (margin + size).y * rndY ) + startPos;
		room->m_boundsOnMap = AABB2( leftBottomPoint, leftBottomPoint + size );
		m_numOfRooms++;
	}
}

void Game::RenderMapScreen() const
{
	if (m_playerShip->m_blindWithRoom) {
		return;
	}
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;

	for (int y = 0; y < m_roomDimensions.y; y++) {
		for (int x = 0; x < m_roomDimensions.x; x++) {
			Room* room = m_roomMap[x + m_roomDimensions.x * y];
			if (room) {
				Rgba8 color;
				if (room == m_curRoom) {
					color = Rgba8( 224, 224, 224, 160 );
				}
				else if (!room->m_isFirstEnter) {
					color = Rgba8( 128, 128, 128, 140 );
				}
				else {
					color = Rgba8( 102, 51, 0, 100 );
				}
				AddVertsForAABB2D( verts, room->m_boundsOnMap, color );
				if (room->IsBossRoom()) {
					g_ASCIIFont->AddVertsForTextInBox2D( textVerts, room->m_boundsOnMap, 30.f, "BOSS" );
				}
				else if (room->m_def.m_type == RoomType::SHOP) {
					g_ASCIIFont->AddVertsForTextInBox2D( textVerts, room->m_boundsOnMap, 30.f, "SHOP", Rgba8( 255, 255, 0 ) );
				}
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
}

void Game::RenderItemScreen() const
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;

	PlayerShip* playerShip = (PlayerShip*)GetPlayerEntity();

	AddVertsForAABB2D( verts, AABB2( Vec2(), Vec2( 1600.f, 800.f ) ), Rgba8( 0, 0, 0, 100 ) );

	AABB2 playerStatStartBound( Vec2( 50.f, 410.f ), Vec2( 350.f, 450.f ) );
	Vec2 dropBounds( 0.f, -40.f );
	// render player stats
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, playerStatStartBound, 30.f, Stringf( "Damage: %.2f", playerShip->GetMainWeaponDamage() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2(playerStatStartBound.m_mins + dropBounds, playerStatStartBound.m_maxs + dropBounds),
		30.f, Stringf( "AttackSpeed: %.2f", playerShip->GetBulletPerSecond() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( playerStatStartBound.m_mins + 2.f * dropBounds, playerStatStartBound.m_maxs + 2.f * dropBounds ),
		30.f, Stringf( "MoveSpeed: %.2f", playerShip->GetMovingSpeed() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( playerStatStartBound.m_mins + 3.f * dropBounds, playerStatStartBound.m_maxs + 3.f * dropBounds ),
		30.f, Stringf( "BulletSpeed: %.2f", playerShip->GetBulletSpeed() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( playerStatStartBound.m_mins + 4.f * dropBounds, playerStatStartBound.m_maxs + 4.f * dropBounds ),
		30.f, Stringf( "BulletLifeTime: %.2f", playerShip->GetBulletLifeTime() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( playerStatStartBound.m_mins + 5.f * dropBounds, playerStatStartBound.m_maxs + 5.f * dropBounds ),
		30.f, Stringf( "DashCoolDown: %.2f", playerShip->GetDashingCoolDown() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( playerStatStartBound.m_mins + 6.f * dropBounds, playerStatStartBound.m_maxs + 6.f * dropBounds ),
		30.f, Stringf( "DashDistance: %.2f", playerShip->GetDashingDist() ), Rgba8::WHITE, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );

	// render items
	AABB2 itemBounds( Vec2( 1400.f, 60.f ), Vec2( 1560.f, 600.f ) );
	constexpr float itemHeight = 40.f;
	constexpr float itemWidth = 40.f;
	constexpr float itemMarginPercentage = 0.2f;
	Vec2 topLeft = Vec2( itemBounds.m_mins.x, itemBounds.m_maxs.y );
	float line = 1.f;
	float numOfItemEachLine = (itemBounds.m_maxs.x - itemBounds.m_mins.x) / itemWidth;
	std::vector<int> const& itemList = playerShip->m_itemList;
	for (int i = 0; i < (int)itemList.size(); i++) {
		Vec2 thisBottomLeft = topLeft + Vec2( (i - (line - 1.f) * numOfItemEachLine) * itemWidth, -itemHeight * line );
		//AddVertsForAABB2D( verts, AABB2( thisBottomLeft + Vec2( itemWidth, itemHeight ) * itemMarginPercentage * 0.5f
		//	, thisBottomLeft + Vec2( itemWidth, itemHeight ) * (1.f - itemMarginPercentage) ), Rgba8( 255, 255, 255 ) );
		//g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( thisBottomLeft + Vec2( itemWidth, itemHeight ) * itemMarginPercentage * 0.5f
		//	, thisBottomLeft + Vec2( itemWidth, itemHeight ) * (1.f - itemMarginPercentage) ), 30.f, Stringf( "%d", itemList[i] ), Rgba8( 0, 0, 0 ) );
		ItemDefinition::GetDefinition( itemList[i] ).RenderItem( AABB2( thisBottomLeft + Vec2( itemWidth, itemHeight ) * itemMarginPercentage * 0.5f, thisBottomLeft + Vec2( itemWidth, itemHeight ) * (1.f - itemMarginPercentage) ) );

		if (m_insepctingItem == i) {
			AddVertsForLineSegment2D( verts, thisBottomLeft, thisBottomLeft + Vec2( 0.f, 6.f ), 2.f, Rgba8::WHITE );
			AddVertsForLineSegment2D( verts, thisBottomLeft, thisBottomLeft + Vec2( 6.f, 0.f ), 2.f, Rgba8::WHITE );
			AddVertsForLineSegment2D( verts, thisBottomLeft + Vec2( itemWidth, itemHeight ), thisBottomLeft + Vec2( itemWidth, itemHeight ) + Vec2( 0.f, -6.f ), 2.f, Rgba8::WHITE );
			AddVertsForLineSegment2D( verts, thisBottomLeft + Vec2( itemWidth, itemHeight ), thisBottomLeft + Vec2( itemWidth, itemHeight ) + Vec2( -6.f, 0.f ), 2.f, Rgba8::WHITE );
			AddVertsForLineSegment2D( verts, thisBottomLeft + Vec2( itemWidth, 0.f ), thisBottomLeft + Vec2( itemWidth, 0.f ) + Vec2( 0.f, 6.f ), 2.f, Rgba8::WHITE );
			AddVertsForLineSegment2D( verts, thisBottomLeft + Vec2( itemWidth, 0.f ), thisBottomLeft + Vec2( itemWidth, 0.f ) + Vec2( -6.f, 0.f ), 2.f, Rgba8::WHITE );
			AddVertsForLineSegment2D( verts, thisBottomLeft + Vec2( 0.f, itemHeight ), thisBottomLeft + Vec2( 0.f, itemHeight ) + Vec2( 0.f, -6.f ), 2.f, Rgba8::WHITE );
			AddVertsForLineSegment2D( verts, thisBottomLeft + Vec2( 0.f, itemHeight ), thisBottomLeft + Vec2( 0.f, itemHeight ) + Vec2( 6.f, 0.f ), 2.f, Rgba8::WHITE );

			ItemDefinition const& def = ItemDefinition::GetDefinition( itemList[i] );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 660.f ), Vec2( 1400.f, 700.f ) ), 40.f, def.m_name );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 620.f ), Vec2( 1400.f, 650.f ) ), 30.f, def.m_description );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 480.f, 200.f ), Vec2( 1220.f, 580.f ) ), 25.f, def.m_detail, Rgba8::WHITE, 0.618f, Vec2( 0.f, 1.f ), TextBoxMode::AUTO_NEW_LINE );
		}

		if (i + 1.f >= numOfItemEachLine * line) {
			line += 1.f;
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
}

void Game::RenderOptionsScreen() const
{
	// buttons
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;

	AddVertsForAABB2D( verts, AABB2( Vec2(), Vec2( 1600.f, 800.f ) ), Rgba8( 0, 0, 0, 100 ) );

	Vec2 buttonLB[4] = { Vec2( 675.f, 500.f ),  Vec2( 675.f, 380.f ), Vec2( 675.f, 260.f ), Vec2( 675.f, 140.f ) };
	std::string buttonDesc[4] = { "Back to Game", "Settings", "Back to Menu", "Quit" };
	float buttonWidth = 250.f;
	float buttonHeight = 75.f;

	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 630.f ), Vec2( 1600.f, 700.f ) ), 70.f, "Options", Rgba8( 255, 255, 0 ) );
	for (int i = 0; i < m_numOfButtons; i++) {
		AddVertsForAABB2D( verts, AABB2( buttonLB[i], buttonLB[i] + Vec2( buttonWidth, buttonHeight ) ), Rgba8( 255, 255, 153 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( buttonLB[i], buttonLB[i] + Vec2( buttonWidth, buttonHeight ) ), 25.f, buttonDesc[i], Rgba8( 0, 0, 0 ) );
		if (m_curHoveringButtom == i) {
			Rgba8 color = Rgba8( 51, 51, 255 );
			AddVertsForLineSegment2D( verts, buttonLB[i], buttonLB[i] + Vec2( 0.f, 6.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i], buttonLB[i] + Vec2( 6.f, 0.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( buttonWidth, buttonHeight ), buttonLB[i] + Vec2( buttonWidth, buttonHeight ) + Vec2( 0.f, -6.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( buttonWidth, buttonHeight ), buttonLB[i] + Vec2( buttonWidth, buttonHeight ) + Vec2( -6.f, 0.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( buttonWidth, 0.f ), buttonLB[i] + Vec2( buttonWidth, 0.f ) + Vec2( 0.f, 6.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( buttonWidth, 0.f ), buttonLB[i] + Vec2( buttonWidth, 0.f ) + Vec2( -6.f, 0.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( 0.f, buttonHeight ), buttonLB[i] + Vec2( 0.f, buttonHeight ) + Vec2( 0.f, -6.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( 0.f, buttonHeight ), buttonLB[i] + Vec2( 0.f, buttonHeight ) + Vec2( 6.f, 0.f ), 5.f, color );
		}
	}

	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 20.f ), Vec2( 1600.f, 60.f ) ), 35.f, "WASD Navigate   Space/Enter Confirm   Esc Back to Game", Rgba8::WHITE );

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
}

void Game::EnterRoom( Room* roomToEnter, RoomDirection dirFrom )
{
	m_cameraCenter = m_curRoom->m_bounds.GetCenter();
	m_curRoom->m_hasItems = m_isChoosingItems;
	m_curRoom->m_numOfItemsCanChoose = m_numOfItemsCanChoose;
	m_curRoom->m_items = m_showingItems;
	m_curRoom->ExitRoom();
	m_curRoom = roomToEnter;
	Entity* player = GetPlayerEntity();
	player->m_position = roomToEnter->GetDoorAtDir( dirFrom );
	if (m_curRoom->m_isFirstEnter) {
		m_clearTheRoomFisrtTime = true;
	}
	else {
		m_clearTheRoomFisrtTime = false;
	}
	m_curRoom->EnterRoom();
	m_isChoosingItems = m_curRoom->m_hasItems;
	m_numOfItemsCanChoose = m_curRoom->m_numOfItemsCanChoose;
	m_showingItems.clear();
	for (int i = 0; i < (int)m_curRoom->m_items.size(); i++) {
		if (!m_playerShip->HasItem( m_curRoom->m_items[i] )) {
			m_showingItems.push_back( m_curRoom->m_items[i] );
		}
	}

	m_goToNextRoomTimer->Start();
	m_goToNextRoomCameraTargetCenter = roomToEnter->m_bounds.GetCenter();
	m_state = GameState::GO_TO_NEXT_ROOM;

}

void Game::DebugKillEverything()
{
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i] && m_entityArray[i] != m_playerController->GetControlledEntity()) {
			m_entityArray[i]->Die();
		}
	}
}

bool Game::IsPlayerBulletInRange( Vec2 const& pos, float rangeRadius ) const
{
	for (int i = 0; i < (int)m_projectileArray.size(); i++) {
		if (m_projectileArray[i] && m_projectileArray[i]->m_faction == "Player") {
			if (IsPointInsideDisc2D( m_projectileArray[i]->m_position, pos, rangeRadius )) {
				return true;
			}
		}
	}
	return false;
}

void Game::DealRangeDamage( Projectile* proj, bool dealOnce /*= false*/, float coolDownTime /*= 0.f */ )
{
	for (auto entity : m_entityArray) {
		if (!entity) {
			continue;
		}
		if (entity->m_def.m_faction == proj->m_faction) {
			continue;
		}
		if (entity->IsInvincible()) {
			continue;
		}
		if (DoDiscsOverlap( proj->m_position, proj->m_rangeDamageRadius, entity->m_position, entity->m_physicsRadius )) {
			if (dealOnce) {
				entity->BeAttackedOnce( proj->m_rangeDamagePercentage * proj->m_damage, Vec2( 0.f, 0.f ), coolDownTime, proj );
			}
			else {
				entity->BeAttacked( proj->m_rangeDamagePercentage * proj->m_damage, Vec2( 0.f, 0.f ) );
			}
		}
	}
}

void Game::DealRangeDamage( float damage, Vec2 const& position, float range, std::string const& sourceFaction, bool dealOnce /*= false*/, float coolDownTime /*= 0.f */, void* source )
{
	for (auto entity : m_entityArray) {
		if (!entity) {
			continue;
		}
		if (entity->IsInvincible()) {
			continue;
		}
		if (DoDiscsOverlap( position, range, entity->m_position, entity->m_physicsRadius ) && entity->m_def.m_faction != sourceFaction) {
			if (dealOnce) {
				entity->BeAttackedOnce( damage, Vec2( 0.f, 0.f ), coolDownTime, source );
			}
			else {
				entity->BeAttacked( damage, Vec2( 0.f, 0.f ) );
			}
		}
	}
}

void Game::PickUpItem( Vec2 const& pos )
{
	if (m_isQuitting) {
		return;
	}
	PlayerShip* player = ((PlayerShip*)GetPlayerEntity());
	// item or non-item?
	if (IsInShop()) {
		NonItemMerchandise nonItem = m_curRoom->GetNearestInRangeNonItemMerchandise( pos );
		if (nonItem != NonItemMerchandise::None) {
			float modifier = m_playerShip->m_customerCard ? 0.5f : 1.f;
			if (nonItem == NonItemMerchandise::Armor && m_curRoom->m_hasArmor) {
				int price = int(m_curRoom->m_armorPrice * modifier);
				if (price <= m_playerController->m_reward && !Starship_IsNearlyZero( player->m_curArmor - player->m_maxArmor )) {
					m_playerController->m_reward -= price;
					player->GainArmor( 1.f );
					m_curRoom->BuyNonItemMerchandise( nonItem );
				}
			}
			else if (nonItem == NonItemMerchandise::Health && m_curRoom->m_hasHealth) {
				int price = int(m_curRoom->m_healthPrice * modifier);
				if (price <= m_playerController->m_reward && !Starship_IsNearlyZero( player->m_health - player->m_maxHealth )) {
					m_playerController->m_reward -= price;
					player->GainHealth( 1.f );
					m_curRoom->BuyNonItemMerchandise( nonItem );
				}
			}
			else if (nonItem == NonItemMerchandise::MaxArmor && m_curRoom->m_hasMaxArmor) {
				int price = int(m_curRoom->m_maxArmorPrice * modifier);
				if (price <= m_playerController->m_reward) {
					m_playerController->m_reward -= price;
					player->GainMaxArmor( 1.f );
					m_curRoom->BuyNonItemMerchandise( nonItem );
				}
			}
			else if (nonItem == NonItemMerchandise::MaxHealth && m_curRoom->m_hasMaxHealth) {
				int price = int(m_curRoom->m_maxHealthPrice * modifier);
				if (price <= m_playerController->m_reward) {
					m_playerController->m_reward -= price;
					player->GainMaxHealth( 1.f );
					m_curRoom->BuyNonItemMerchandise( nonItem );
				}
			}
		}
		else {
			// do not near non-item
			goto L1;
		}
	}
	else {
		// pick up item
	L1:
		if (g_theGame->m_isChoosingItems) {
			int id = -1;
			float dist = FLT_MAX;
			GetTheNearestItemIDAndDist( id, dist, pos );
			bool canGetItem = false;

			// can pick up? cost?
			if (id != -1 && dist <= 5.f) {
				ItemDefinition& def = ItemDefinition::GetDefinition( id );
				if (IsInShop() && def.m_isSold) {
					if (m_playerController->m_reward >= def.GetPrice()) {
						m_playerController->m_reward -= def.GetPrice();
						if (m_playerShip->m_becomeLighter && m_playerController->m_reward >= 100) {
							m_playerShip->m_movingSpeedModifier += 0.1f;
						}
						canGetItem = true;
						m_numOfItemsCanChoose--;
						def.m_isDiscount = false;
						def.m_isSold = false;
						for (int i = 0; i < (int)m_showingItems.size(); i++) {
							if (m_showingItems[i] == id) {
								if (m_playerShip->m_itemAutoRefresh) {
									ItemDefinition* newDef = ItemDefinition::GetRandomDefinition( -1 );
									if (newDef) {
										m_numOfItemsCanChoose++;
										ItemDefinition::SetItemAvailability( newDef->m_id, false );
										ItemDefinition::SetStatusAndPosition( newDef->m_id, ItemStatus::In_Room, def.m_position, true );
										m_showingItems[i] = newDef->m_id;
									}
									else {
										m_showingItems.erase( m_showingItems.begin() + i );
										break;
									}
								}
								else {
									m_showingItems.erase( m_showingItems.begin() + i );
									break;
								}
							}
						}
					}
				}
				else {
					canGetItem = true;
					if (!def.m_isThrowAwayItem) {
						m_numOfItemsCanChoose--;
					}
					for (int i = 0; i < (int)m_showingItems.size(); i++) {
						if (m_showingItems[i] == id) {
							m_showingItems.erase( m_showingItems.begin() + i );
							break;
						}
					}
				}
			}

			// if reach the limit of picking up, go to normal mode
			if (m_isChoosingItems) {
				if (m_numOfItemsCanChoose <= 0) {
					//m_isChoosingItems = false;
					if (m_curRoom->m_itemChooseMode) {
						ExitChooseItemMode();
					}
					// return the unchosen items
					for (int i = 0; i < (int)m_showingItems.size(); i++) {
						if (!ItemDefinition::GetDefinition( m_showingItems[i] ).m_isThrowAwayItem && !ItemDefinition::GetDefinition( m_showingItems[i] ).m_isSold) {
							ItemDefinition::SetItemAvailability( m_showingItems[i], true );
							ItemDefinition::SetStatusAndPosition( m_showingItems[i], ItemStatus::In_Pool, Vec2() );
							m_showingItems.erase( i + m_showingItems.begin() );
							i--;
						}
					}
				}
			}

			// do item effect
			if (canGetItem) {
				player->GainItem( id );
				g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Powerup2.wav" ), false, g_theApp->m_soundVolume * 0.1f );
			}
		}
	}
}

void Game::TransferItemToMaxHealth( Vec2 const& pos )
{
	int id = -1;
	float dist = FLT_MAX;
	GetTheNearestItemIDAndDist( id, dist, pos );
	bool canGetItem = false;

	// can pick up? cost?
	if (id != -1 && dist <= 5.f) {
		ItemDefinition& def = ItemDefinition::GetDefinition( id );
		if (!(IsInShop() && def.m_isSold)) {
			canGetItem = true;
			if (!def.m_isThrowAwayItem) {
				m_numOfItemsCanChoose--;
			}
			for (int i = 0; i < (int)m_showingItems.size(); i++) {
				if (m_showingItems[i] == id) {
					m_showingItems.erase( m_showingItems.begin() + i );
					break;
				}
			}
		}
	}

	// if reach the limit of picking up, go to normal mode
	if (m_isChoosingItems) {
		if (m_numOfItemsCanChoose <= 0) {
			m_isChoosingItems = false;
			if (m_curRoom->m_itemChooseMode) {
				ExitChooseItemMode();
			}
			// return the unchosen items
			for (int i = 0; i < (int)m_showingItems.size(); i++) {
				if (!ItemDefinition::GetDefinition( m_showingItems[i] ).m_isThrowAwayItem) {
					ItemDefinition::SetItemAvailability( m_showingItems[i], true );
					ItemDefinition::SetStatusAndPosition( m_showingItems[i], ItemStatus::In_Pool, Vec2() );
				}
			}
			m_showingItems.clear();
		}
	}

	// do item effect
	if (canGetItem) {
		m_playerShip->GainMaxHealth( 1.f );
	}
}

bool Game::IsInShop() const
{
	if (m_isQuitting) {
		return false;
	}
	return m_curRoom->m_def.m_type == RoomType::SHOP;
}

void Game::EnterChooseItemMode()
{
	m_curRoom->m_itemChooseMode = true;
	m_isChoosingItems = true;
	m_numOfItemsCanChoose = 1;
}

void Game::AddItemToChoose( int itemID )
{	
	static AABB2 const bounds = AABB2( Vec2( 20.f, 10.f ), Vec2( 170.f, 75.f ) );
	constexpr float itemSize = 6.f;
	constexpr float margin = 4.f;
	constexpr int numOfItemsInARow = 15;
	int numOfItemsNow = 0;
	for (int i = 0; i < (int)m_showingItems.size(); i++) {
		if (!ItemDefinition::GetDefinition( m_showingItems[i] ).m_isThrowAwayItem && !ItemDefinition::GetDefinition( m_showingItems[i] ).m_isSold) {
			numOfItemsNow++;
		}
	}
	int row = numOfItemsNow / numOfItemsInARow;
	int col = numOfItemsNow % numOfItemsInARow;
	m_showingItems.push_back( itemID );
	ItemDefinition::SetItemAvailability( itemID, false );
	ItemDefinition::SetStatusAndPosition( itemID, ItemStatus::In_Room, m_curRoom->m_bounds.m_mins + Vec2( bounds.m_mins.x, bounds.m_maxs.y ) + Vec2( margin * 0.5f + col * (itemSize + margin), -10.f * row ) );
}

void Game::ExitChooseItemMode()
{
	if (!IsInShop()) {
		m_isChoosingItems = false;
	}
	m_curRoom->m_itemChooseMode = false;
}

Entity* Game::GetNearestEnemy( Vec2 const& position, float maxDist, Entity* excludeEnemy ) const
{
	float minDist = maxDist * maxDist;
	Entity* entity = nullptr;
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i] && m_entityArray[i] != excludeEnemy && m_entityArray[i]->IsAlive() 
			&& m_entityArray[i]->m_def.m_faction != "Player" && m_entityArray[i]->m_def.m_faction != "Neutral" 
			&& GetDistanceSquared2D(position, m_entityArray[i]->m_position) < minDist) {
			minDist = GetDistanceSquared2D( position, m_entityArray[i]->m_position );
			entity = m_entityArray[i];
		}
	}
	return entity;
}

void Game::GoToNextFloor()
{
	CleanUpCurLevel();
	m_curLevel++;
	if (m_curLevel == (int)m_levelSequence.size()) {
		g_theApp->GoToAppMode( AppState::ATTRACT_MODE );
		m_isQuitting = true;
		return;
	}
	m_numOfHealthPickupGenerated = 0;
	m_maxNumOfHealthPickupThisLevel = 6 + m_curLevel;
	SetUpRooms( m_levelSequence[m_curLevel], m_curLevel );
	EnterRoom( m_curRoom, RoomDirection::CENTER );
	m_playerShip->GoToNextLevel();
	m_portal = nullptr;
	//BeginGame();
}

void Game::SetTimeScaleOfAllEntity( std::string const& friendlyFaction, float newTimeScale )
{
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i] && m_entityArray[i]->IsAlive() && m_entityArray[i]->m_def.m_faction != friendlyFaction) {
			m_entityArray[i]->m_clock->SetTimeScale( newTimeScale );
		}
	}
}

void Game::RerandomizeAllItemsInRoom( bool onlyShop )
{
	std::vector<int> pendingItems;
	pendingItems.reserve( m_showingItems.size() );
	for (int i = 0; i < (int)m_showingItems.size(); i++) {
		ItemDefinition& oldDef = ItemDefinition::GetDefinition( m_showingItems[i] );
		if (!onlyShop || (onlyShop && oldDef.m_isSold)) {
			ItemDefinition* newDef = ItemDefinition::GetRandomDefinition( oldDef.m_pool );
			if (newDef) {
				pendingItems.push_back( newDef->m_id );

				newDef->m_isAvailable = oldDef.m_isAvailable;
				newDef->m_isSold = oldDef.m_isSold;
				newDef->m_isDiscount = oldDef.m_isDiscount;
				newDef->m_isThrowAwayItem = oldDef.m_isThrowAwayItem;
				newDef->m_status = oldDef.m_status;
				newDef->m_position = oldDef.m_position;
			}
		}
	}

	for (int i = 0; i < (int)m_showingItems.size(); i++) {
		ItemDefinition& oldDef = ItemDefinition::GetDefinition( m_showingItems[i] );
		if (!onlyShop || (onlyShop && oldDef.m_isSold)) {
			oldDef.m_isAvailable = true;
			oldDef.m_isSold = false;
			oldDef.m_isDiscount = false;
			oldDef.m_isThrowAwayItem = false;
			oldDef.m_status = ItemStatus::In_Pool;
			oldDef.m_position = Vec2();
			oldDef.m_charge = oldDef.m_startCharge;
			if (onlyShop) {
				m_showingItems.erase( m_showingItems.begin() + i );
				i--;
			}
		}
	}
	if (!onlyShop) {
		m_showingItems.clear();
	}
	for (int i = 0; i < (int)pendingItems.size(); i++) {
		m_showingItems.push_back( pendingItems[i] );
	}
}

void Game::GenerateNewShopToLevel( int levelOfShop /*= 1 */ )
{
	RandomSetRoom( "Shop", 1, levelOfShop );
}

void Game::GetAllEntityByDef( std::vector<Entity*>& out_entityArray, EntityDefinition const& def ) const
{
	out_entityArray.clear();
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i] && m_entityArray[i]->IsAlive() && m_entityArray[i]->m_def.m_aiBehavior == def.m_aiBehavior) {
			out_entityArray.push_back( m_entityArray[i] );
		}
	}
}

void Game::RoomClearCallBack( Room* room )
{
	if (room->IsBossRoom()) {
		m_bossKilledThisFloor++;
		if (g_theApp->m_bossRushFlag) {
			SpawnEffectToGame( EffectType::ArmorPickup, m_curRoom->m_bounds.m_mins + Vec2( 110.f, 65.f ) );
			SpawnEffectToGame( EffectType::HealthPickup, m_curRoom->m_bounds.m_mins + Vec2( 90.f, 65.f ) );
		}
		g_theAudio->StopSound( m_backgroundMusicID );
		m_backgroundMusicID = g_theAudio->StartSound( g_theAudio->CreateOrGetSound( m_levelMusicSequence[m_curLevel] ), true, g_theApp->m_musicVolume * 0.6f );
	}
	if (room->m_def.m_itemInfo.m_draftingPool != -1) {
		m_isChoosingItems = true;
		m_numOfItemsCanChoose = 1;
		constexpr float itemSize = 6.f;
		constexpr float halfItemSize = itemSize * 0.5f;
		constexpr float margin = 3.f;
		float startX = room->m_def.m_itemInfo.m_position.x - (room->m_def.m_numOfItemsToChoose % 2) * halfItemSize - (room->m_def.m_numOfItemsToChoose / 2) * (itemSize + margin) - ((room->m_def.m_numOfItemsToChoose % 2) * 0.5f + 0.5f) * margin;
		for (int i = 0; i < room->m_def.m_numOfItemsToChoose; i++) {
			ItemDefinition* def = nullptr;
			if (g_theApp->m_bossRushFlag) {
				def = ItemDefinition::GetRandomDefinition( GetClamped( m_bossKilledThisFloor + 1, 1, 4 ) );
			}
			else {
				def = ItemDefinition::GetRandomDefinition( room->m_def.m_itemInfo.m_draftingPool );
			}
			
			if (def) {
				int id = def->m_id;
				m_showingItems.push_back( id );
				ItemDefinition::SetItemAvailability( id, false );
				ItemDefinition::SetStatusAndPosition( id, ItemStatus::In_Room, room->m_bounds.m_mins + Vec2( startX + i * (itemSize + margin) + margin, room->m_def.m_itemInfo.m_position.y ) );
			}
		}
	}
}

void Game::GetTheNearestItemIDAndDist( int& id, float& dist, Vec2 const& refPos ) const
{
	int nearestID = -1;
	float nearestDistSquared = FLT_MAX;
	for (int i = 0; i < (int)m_showingItems.size(); i++) {
		float distSquared = GetDistanceSquared2D( refPos, ItemDefinition::GetDefinition( m_showingItems[i] ).m_position );
		if (distSquared < nearestDistSquared) {
			nearestDistSquared = distSquared;
			nearestID = m_showingItems[i];
		}
	}
	id = nearestID;
	dist = sqrtf( nearestDistSquared );
}

void Game::GenerateHealthOrArmorPickup( Vec2 const& pos )
{
	if (m_numOfHealthPickupGenerated >= m_maxNumOfHealthPickupThisLevel) {
		return;
	}
	float rnd = GetRandGen()->RollRandomFloatZeroToOne();
	if (rnd < 0.1f) {
		SpawnEffectToGame( EffectType::HealthPickup, pos );
		m_numOfHealthPickupGenerated++;
	}
	else if (rnd < 0.2f) {
		SpawnEffectToGame( EffectType::ArmorPickup, pos );
		m_numOfHealthPickupGenerated++;
	}
}

void Game::CleanUpCurLevel()
{
	ItemDefinition::ResetAllInRoomItems();
	RoomDefinition::ResetAllDefinitionsForEachFloor();
	for (auto& entity : m_entityArray) {
		if (entity && entity->m_def.m_faction != "Player") {
			delete entity;
			entity = nullptr;
		}
	}

	for (auto& effect : m_effectArray) {
		if (effect && effect->m_type != EffectType::PlayerShield) {
			delete effect;
			effect = nullptr;
		}
	}

	for (auto& projectile : m_projectileArray) {
		delete projectile;
		projectile = nullptr;
	}
	for (auto room : m_roomMap) {
		delete room;
	}
	m_roomMap.clear();
	m_numOfRooms = 0;
	m_bossKilledThisFloor = 0;
}

std::string Game::GetRandomFaction() const
{
	int rnd = GetRandGen()->RollRandomIntInRange( 0, 2 );
	return m_levelSequence[rnd];
}

void Game::SaveGameBetweenRuns() const
{
	std::vector<uint8_t> buffer;
	buffer.reserve( 100 );
	buffer.push_back( 'S' );
	buffer.push_back( 'S' );
	buffer.push_back( 'U' );
	buffer.push_back( 'T' );

	uint8_t* coinsArray = (uint8_t*)&m_savedCoins;

	buffer.push_back( coinsArray[0] );
	buffer.push_back( coinsArray[1] );
	buffer.push_back( coinsArray[2] );
	buffer.push_back( coinsArray[3] );

	BufferWriteToFile( buffer, Stringf( "Saves/GameSaves.ssut" ) );
}

void Game::ReadGameSaveData()
{
	std::vector<uint8_t> buffer;
	int res = FileReadToBuffer( buffer, Stringf( "Saves/GameSaves.ssut" ) );
	if (res == -1 || (int)buffer.size() == 0) {
		return;
	}
	if (!(buffer[0] == 'S' && buffer[1] == 'S' && buffer[2] == 'U' && buffer[3] == 'T')) {
		return;
	}
	int* savedCoins = (int*)&buffer[4];

	m_savedCoins = *savedCoins;
}

StarshipRayCastResult::StarshipRayCastResult( RayCastResult2D const& res )
{
	m_didImpact = res.m_didImpact;
	m_impactDist = res.m_impactDist;
	m_impactPos = res.m_impactPos;
	m_impactNormal = res.m_impactNormal;
	m_rayForwardNormal = res.m_rayForwardNormal;
	m_rayStartPos = res.m_rayStartPos;
	m_rayMaxLength = res.m_rayMaxLength;
}

StarshipRayCastResult::StarshipRayCastResult()
{

}
