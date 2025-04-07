#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/PlayerController.hpp"
#include "Game/Weapon.hpp"
#include "Game/Actor.hpp"
#include "Game/AIController.hpp"

Game::Game()
{
	// load random number generator
	m_randNumGen = new RandomNumberGenerator();
	m_gameClock = new Clock();
}

Game::~Game()
{
	for (int i = 0; i < (int)m_players.size(); i++) {
		delete m_players[i];
		m_players[i] = nullptr;
	}
	for (int i = 0; i < (int)m_maps.size(); i++) {
		delete m_maps[i];
		m_maps[i] = nullptr;
	}
	DebugRenderSystemShutdown();
	// delete all pointers managed by Game
	delete m_randNumGen;
	m_randNumGen = nullptr;
}

void Game::Startup()
{
	m_timer = new Timer( 1.f );
	m_timer->Start();
	m_mapChooseButtonJoystickCooldownTimer = new Timer( 0.3f );
	m_mapChooseButtonJoystickCooldownTimer->Start();
	m_mapChooseButtonJoystickCooldownTimer->SetElapsedTime( 0.3f );
	// set up camera
	//m_screenCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ) );
	m_gameScreenCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 0.f, 1.f );
	m_gameScreenCamera.m_mode = CameraMode::Orthographic;

	LoadDefinitions();
	SetUpMaps();

	m_players.resize( 2, nullptr );
}

void Game::Update()
{
	UpdateState();
	if (m_currentState == GameState::ATTRACT) {
		UpdateAttractMode();
	}
	else if (m_currentState == GameState::PLAYING) {
		UpdatePlayingMode();
	}
	else if (m_currentState == GameState::LOBBY) {
		UpdateLobbyState();
	}
	else if (m_currentState == GameState::VICTORY) {
		UpdateVictoryMode();
	}
	HandleKeys();
}

void Game::Render() const
{
	if (m_currentState == GameState::ATTRACT) {
		g_theRenderer->ClearScreen( Rgba8( 64, 64, 64 ) );
		RenderAttractMode();
	}
	else if (m_currentState == GameState::PLAYING) {
		g_theRenderer->ClearScreen( Rgba8( 153, 255, 255 ) );
		RenderPlayingMode();
	}
	else if (m_currentState == GameState::LOBBY) {
		g_theRenderer->ClearScreen( Rgba8( 64, 64, 64 ) );
		RenderLobbyState();
	}
	else if (m_currentState == GameState::VICTORY) {
		g_theRenderer->ClearScreen( Rgba8( 153, 255, 255 ) );
		RenderVictoryMode();
	}
}

void Game::EnterState( GameState state )
{
	m_nextFrameState = state;
}

GameState Game::GetState() const
{
	return m_currentState;
}

PlayerController* Game::GetPlayerController( int playerId ) const
{
	return m_players[playerId];
}

AIController* Game::CreateNewAIController( std::string const& aiBehavior )
{
	AIController* newController = nullptr;
	if (aiBehavior == "MeleeAI") {
		newController = new MeleeAIController();
	}
	else if (aiBehavior == "RangedAI") {
		newController = new RangedAIController();
	}
	else {
		newController = new AIController();
	}

	for (int i = 0; i < (int)m_AIs.size(); i++) {
		if (m_AIs[i] == nullptr) {
			m_AIs[i] = newController;
			return m_AIs[i];
		}
	}
	m_AIs.push_back( newController );
	return newController;
}

SoundPlaybackID Game::PlaySound3D( std::string const& name, Vec3 const& position, ActorUID callingActor, bool addToUpdateList, float intervalTimeSeconds /*= 0.f*/, bool isLooped /*= false*/, float volume /*= 1.f*/, float balance /*= 0.f*/, float speed /*= 1.f */ )
{
	double curTimeSeconds = GetCurrentTimeSeconds();
	if (float( curTimeSeconds - g_theApp->m_lastSoundPlaySecondsByID[name] ) >= intervalTimeSeconds) {
		g_theApp->m_lastSoundPlaySecondsByID[name] = curTimeSeconds;
		SoundPlaybackID retID = g_theAudio->StartSoundAt( g_theApp->GetSoundId( name ), position, isLooped, volume, balance, speed );
		if (addToUpdateList) {
			m_3DSounds[retID] = callingActor;
		}
		return retID;
	}
	return (SoundPlaybackID)(-1);
}

MapDefinition const& Game::GetMapDefByName( std::string name ) const
{
	for (int i = 0; i < (int)m_mapDefs.size(); i++) {
		if (name == m_mapDefs[i].m_mapName) {
			return m_mapDefs[i];
		}
	}
	ERROR_AND_DIE( Stringf( "Cannot get map definition by %s", name.c_str() ) );
}

void Game::HandleKeys()
{

	m_gameClock->SetTimeScale( 1.f );

	if (g_theInput->WasKeyJustPressed( 0x4F )) {// O key run a single frame and pauses
		m_gameClock->StepSingleFrame();
	}

	if (g_theInput->IsKeyDown( 0x54 )) // T key slow the game
	{
		m_gameClock->SetTimeScale( 0.1f );
	}

	if (g_theInput->WasKeyJustPressed( 0x50 )) // P key pause the game; handle the pause problem
	{
		m_gameClock->TogglePause();
	}

}

bool Game::IsMultiplayerPlaying() const
{
	return m_numOfPlayers > 1;
}

void Game::LoadDefinitions()
{
	WeaponDefinition::SetUpWeaponDefinitions();
	ActorDefinition::SetUpActorDefinitions();
	LoadMapDefs();
}

void Game::LoadMapDefs()
{
	g_devConsole->AddLine( DevConsole::INFO_MINOR, "Reading map definitions..." );
	m_mapDefs.reserve( 50 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Definitions/MapDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document MapDefinitions.xml error" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "Definitions" ), "Syntax Error! Name of the root of MapDefinitions.xml should be \"Definitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	int i = 0;
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "MapDefinition" ), "Syntax Error! Names of the elements of MapDefinitions.xml should be \"MapDefinition\" " );
		m_mapDefs.push_back( MapDefinition() );
		m_mapDefs[i].m_mapName = ParseXmlAttribute( *xmlIter, "name", "Default" );
		m_mapDefs[i].m_imagePath = ParseXmlAttribute( *xmlIter, "image", "Default" );
		m_mapDefs[i].m_shaderName = ParseXmlAttribute( *xmlIter, "shader", "Default" );
		m_mapDefs[i].m_spriteTexturePath = ParseXmlAttribute( *xmlIter, "spriteSheetTexture", "Default" );
		m_mapDefs[i].m_spriteSheetCellCount = ParseXmlAttribute( *xmlIter, "spriteSheetCellCount", IntVec2( -1, -1 ) );
		m_mapDefs[i].m_gameMode = ParseXmlAttribute( *xmlIter, "gameMode", m_mapDefs[i].m_gameMode );
		m_mapDefs[i].m_description = ParseXmlAttribute( *xmlIter, "description", "No Description!" );
		m_mapDefs[i].m_hasPointLight = ParseXmlAttribute( *xmlIter, "hasPointLight", m_mapDefs[i].m_hasPointLight );
		m_mapDefs[i].m_pointLightPosition = ParseXmlAttribute( *xmlIter, "pointLightPosition", m_mapDefs[i].m_pointLightPosition );
		m_mapDefs[i].m_pointLightOrientation = ParseXmlAttribute( *xmlIter, "pointLightOrientation", m_mapDefs[i].m_pointLightOrientation );
		m_mapDefs[i].m_hasDirectionalLight = ParseXmlAttribute( *xmlIter, "hasDirectionalLight", m_mapDefs[i].m_hasDirectionalLight );
		m_mapDefs[i].m_directionalLightOrientation = ParseXmlAttribute( *xmlIter, "directionLightOrientation", m_mapDefs[i].m_directionalLightOrientation );
		m_mapDefs[i].m_directionalLightPosition = ParseXmlAttribute( *xmlIter, "directionalLightPosition", m_mapDefs[i].m_directionalLightPosition );
		m_mapDefs[i].m_lightAmbient = ParseXmlAttribute( *xmlIter, "ambient", m_mapDefs[i].m_lightAmbient );
		XmlElement* spawnInfos = xmlIter->FirstChildElement( "SpawnInfos" );
		if (spawnInfos) {
			XmlElement* spawnInfoIter = spawnInfos->FirstChildElement();
			GUARANTEE_OR_DIE( !strcmp( spawnInfoIter->Name(), "SpawnInfo" ), Stringf( "Syntax Error! Cannot find SpawnInfo! %s is found!", spawnInfoIter->Name() ) );
			while (spawnInfoIter != nullptr) {
				SpawnInfo spawnInfo;
				spawnInfo.m_actorName = ParseXmlAttribute( *spawnInfoIter, "actor", "Default" );
				spawnInfo.m_orientation = ParseXmlAttribute( *spawnInfoIter, "orientation", EulerAngles() );
				spawnInfo.m_position = ParseXmlAttribute( *spawnInfoIter, "position", Vec3() );
				spawnInfo.m_velocity = ParseXmlAttribute( *spawnInfoIter, "velocity", Vec3() );
				m_mapDefs[i].m_spawnInfos.push_back( spawnInfo );
				spawnInfoIter = spawnInfoIter->NextSiblingElement();
			}
		}
		xmlIter = xmlIter->NextSiblingElement();
		i++;
	}
	m_numOfMaps = (int)m_mapDefs.size();
}

void Game::SetUpMaps()
{

}

GameMode* Game::CreateGameMode( Map* map, std::string const& gameModeName )
{
	if (gameModeName == "KillAllEnemies") {
		return new KillAllGameMode( map );
	}
	else if (gameModeName == "KillOtherPlayer") {
		return new KillOpponentGameMode( map );
	}
	else if (gameModeName == "Survival") {
		return new SurvivalGameMode( map );
	}
	else {
		return new GameMode( map );
	}
}

void Game::UpdateState()
{
	if (m_currentState != m_nextFrameState) {
		if (m_nextFrameState == GameState::ATTRACT) {
			if (m_currentState == GameState::VICTORY || m_currentState == GameState::PLAYING) {
				for (int i = 0; i < (int)m_players.size(); i++) {
					delete m_players[i];
					m_players[i] = nullptr;
				}
				for (int i = 0; i < (int)m_AIs.size(); i++) {
					delete m_AIs[i];
					m_AIs[i] = nullptr;
				}
				m_attractModeMusic = g_theApp->PlaySound( g_gameConfigBlackboard.GetValue( "mainMenuMusic", "" ), 0.f, true, g_gameConfigBlackboard.GetValue( "musicVolume", 0.1f ) );
				g_theAudio->StopSound( m_gameMusic );
			}
			m_maps.clear();
			delete m_curMap;
			m_3DSounds.clear();
			m_curMap = nullptr;
			m_numOfPlayers = 0;
		}
		else if (m_nextFrameState == GameState::PLAYING) {
			std::string defaultMapName = g_gameConfigBlackboard.GetValue( "defaultMap", "Default" );
			MapDefinition const& defaultMapDef = m_mapDefs[m_mapChoose];
			m_curMap = new Map( defaultMapDef );
			m_maps.push_back( m_curMap );
			m_players[0]->m_worldCamera.m_perspectiveAspect = g_window->GetAspect();
			if (m_numOfPlayers == 2) {
				m_players[1]->m_worldCamera.m_perspectiveAspect = g_window->GetAspect();
			}
			m_curMap->Startup();
			m_curMap->m_gameMode = CreateGameMode( m_curMap, defaultMapDef.m_gameMode );
			m_curMap->AddActorToMap( m_curMap->m_gameMode );
			m_curMap->m_gameMode->BeginPlay();
			
			m_gameMusic = g_theApp->PlaySound( g_gameConfigBlackboard.GetValue( "gameMusic", "" ), 0.f, true, g_gameConfigBlackboard.GetValue( "musicVolume", 0.1f ) );
			g_theAudio->StopSound( m_attractModeMusic );
			g_theAudio->SetNumListeners( m_numOfPlayers );
			if (m_numOfPlayers == 2) {
				// split screen
				IntVec2 dimensions = g_window->GetClientDimensions();
				float xLength = (float)dimensions.x;
				float yLength = (float)dimensions.y;
				m_players[0]->m_worldCamera.SetViewPort( AABB2( Vec2( 0.f, 0.f ), Vec2( xLength, yLength * 0.5f ) ) );
				m_players[0]->m_uiCamera.SetViewPort( AABB2( Vec2( 0.f, 0.f ), Vec2( xLength, yLength * 0.5f ) ) );
				m_players[1]->m_worldCamera.SetViewPort( AABB2( Vec2( 0.f, yLength * 0.5f ), Vec2( xLength, yLength ) ) );
				m_players[1]->m_uiCamera.SetViewPort( AABB2( Vec2( 0.f, yLength * 0.5f ), Vec2( xLength, yLength ) ) );
			}
		}
		else if (m_nextFrameState == GameState::VICTORY) {
			m_victoryTimer = 3.f;
		}
		m_currentState = m_nextFrameState;
	}
}

void Game::UpdateDebugRender()
{
	DebugAddScreenText( Stringf( "Time: %.2f FPS: %.2f Scale: %.1f", m_gameClock->GetTotalSeconds(), 1.f / Clock::GetSystemClock()->GetDeltaSeconds(), Clock::GetSystemClock()->GetTimeScale() ), m_gameScreenCamera.m_cameraBox.m_maxs - Vec2( 400.f, 20.f ), 20.f, Vec2( 0.f, 0.f ), 0.f, Rgba8::WHITE, Rgba8::WHITE );
}

void Game::UpdateAttractMode()
{
	//float deltaSeconds = Clock::GetSystemClock()->GetDeltaSeconds();
	// space enter game

	if (g_theInput->WasKeyJustPressed( KEYCODE_UPARROW ) || g_theInput->WasKeyJustPressed( 'W' )
		|| g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_UP )
		|| g_theInput->GetController( 1 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_UP )) {
		m_mapChoose = (m_mapChoose - 1 + m_numOfMaps) % m_numOfMaps;
	}

	if (g_theInput->WasKeyJustPressed( KEYCODE_DOWNARROW ) || g_theInput->WasKeyJustPressed( 'S' )
		|| g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_DOWN )
		|| g_theInput->GetController( 1 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_DOWN )) {
		m_mapChoose = (m_mapChoose + 1) % m_numOfMaps;
	}

	if ((g_theInput->GetController( 0 ).GetLeftStick().GetPosition().y > 0.5f
		|| g_theInput->GetController( 1 ).GetLeftStick().GetPosition().y > 0.5f)
		&& m_mapChooseButtonJoystickCooldownTimer->HasPeriodElapsed()) {
		m_mapChoose = (m_mapChoose - 1 + m_numOfMaps) % m_numOfMaps;
		m_mapChooseButtonJoystickCooldownTimer->Start();
	}

	if ((g_theInput->GetController( 0 ).GetLeftStick().GetPosition().y < -0.5f
		|| g_theInput->GetController( 1 ).GetLeftStick().GetPosition().y < -0.5f)
		&& m_mapChooseButtonJoystickCooldownTimer->HasPeriodElapsed()) {
		m_mapChoose = (m_mapChoose + 1) % m_numOfMaps;
		m_mapChooseButtonJoystickCooldownTimer->Start();
	}

	if (g_theInput->WasKeyJustPressed( KEYCODE_SPACE ))
	{
		EnterState( GameState::LOBBY );
		g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Enter Lobby" );
		g_theApp->PlaySound( g_gameConfigBlackboard.GetValue( "buttonClickSound", "" ), 0.f, false );
		PlayerController* controller = new PlayerController();
		controller->m_controllerIndex = -1;
		m_players[0] = controller;
		m_numOfPlayers = 1;
		//g_theGame->Startup();
	}
	else if (g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_START )) {
		EnterState( GameState::LOBBY );
		g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Enter Lobby" );
		g_theApp->PlaySound( g_gameConfigBlackboard.GetValue( "buttonClickSound", "" ), 0.f, false );
		PlayerController* controller = new PlayerController();
		controller->m_controllerIndex = 0;
		m_players[0] = controller;
		m_numOfPlayers = 1;
	}
	else if (g_theInput->GetController( 1 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_START )) {
		EnterState( GameState::LOBBY );
		g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Enter Lobby" );
		g_theApp->PlaySound( g_gameConfigBlackboard.GetValue( "buttonClickSound", "" ), 0.f, false );
		PlayerController* controller = new PlayerController();
		controller->m_controllerIndex = 1;
		m_players[0] = controller;
		m_numOfPlayers = 1;
	}
}

void Game::UpdatePlayingMode()
{
	UpdateDebugRender();
	for (int i = 0; i < (int)m_players.size(); i++) {
		if (m_players[i] && !m_curMap->m_blockUpdate) {
			m_players[i]->Update();
		}
	}
	for (int i = 0; i < (int)m_AIs.size(); i++) {
		if (m_AIs[i] && !m_curMap->m_blockUpdate) {
			m_AIs[i]->Update();
			if (m_AIs[i]->IsLoseControl()) {
				m_AIs[i]->m_controlledActorUID->m_controller = nullptr;
				delete m_AIs[i];
				m_AIs[i] = nullptr;
			}
		}
	}
	m_curMap->Update();
	UpdateSounds();

	// escape exit the game
	if (g_theInput->WasKeyJustPressed( KEYCODE_ESC )
		|| g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_BACK )
		|| g_theInput->GetController( 1 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_BACK ))
	{
		EnterState( GameState::ATTRACT );
		g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Exit Game" );
		g_theApp->PlaySound( g_gameConfigBlackboard.GetValue( "buttonClickSound", "" ), 0.f, false );
	}
}

void Game::UpdateLobbyState()
{
	for (int i = 0; i < m_numOfPlayers; i++) {
		if (m_players[i]->m_controllerIndex == -1) {
			if (g_theInput->WasKeyJustPressed( ' ' )) {
				if (m_numOfPlayers == 2 || m_mapDefs[m_mapChoose].m_gameMode != "KillOtherPlayer") {
					EnterState( GameState::PLAYING );
					g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Enter Game" );
					g_theApp->PlaySound( g_gameConfigBlackboard.GetValue( "buttonClickSound", "" ), 0.f, false );
				}
				return;
			}
			if (g_theInput->WasKeyJustPressed( KEYCODE_ESC )) {
				g_theApp->PlaySound( g_gameConfigBlackboard.GetValue( "buttonClickSound", "" ), 0.f, false );
				delete m_players[i];
				if (m_numOfPlayers == 1) {
					m_players[0] = nullptr;
					m_numOfPlayers = 0;
					EnterState( GameState::ATTRACT );
				}
				else if (m_numOfPlayers == 2) {
					m_numOfPlayers = 1;
					if (i == 0) {
						m_players[0] = m_players[1];
						m_players[1] = nullptr;
					}
					else if (i == 1) {
						m_players[i] = nullptr;
					}
				}
				return;
			}
		}
		else {
			int controllerID = m_players[i]->m_controllerIndex;
			if (g_theInput->GetController( controllerID ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_START )) {
				if (m_numOfPlayers == 2 || m_mapDefs[m_mapChoose].m_gameMode != "KillOtherPlayer") {
					EnterState( GameState::PLAYING );
					g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Enter Game" );
					g_theApp->PlaySound( g_gameConfigBlackboard.GetValue( "buttonClickSound", "" ), 0.f, false );
				}
				return;
			}
			if (g_theInput->GetController( controllerID ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_BACK )) {
				g_theApp->PlaySound( g_gameConfigBlackboard.GetValue( "buttonClickSound", "" ), 0.f, false );
				delete m_players[i];
				if (m_numOfPlayers == 1) {
					m_players[0] = nullptr;
					m_numOfPlayers = 0;
					EnterState( GameState::ATTRACT );
				}
				else if (m_numOfPlayers == 2) {
					m_numOfPlayers = 1;
					if (i == 0) {
						m_players[0] = m_players[1];
						m_players[1] = nullptr;
					}
					else if (i == 1) {
						m_players[i] = nullptr;
					}
				}
				return;
			}
		}
	}

	// add player
	if (g_theInput->WasKeyJustPressed( ' ' ) && m_numOfPlayers == 1) {
		m_numOfPlayers = 2;
		PlayerController* controller = new PlayerController();
		controller->m_controllerIndex = -1;
		m_players[1] = controller;
	}
	if (g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_START ) && m_numOfPlayers == 1) {
		m_numOfPlayers = 2;
		PlayerController* controller = new PlayerController();
		controller->m_controllerIndex = 0;
		m_players[1] = controller;
	}
	if (g_theInput->GetController( 1 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_START ) && m_numOfPlayers == 1) {
		m_numOfPlayers = 2;
		PlayerController* controller = new PlayerController();
		controller->m_controllerIndex = 1;
		m_players[1] = controller;
	}
	
}

void Game::UpdateVictoryMode()
{
	m_victoryTimer -= m_gameClock->GetDeltaSeconds();
	if (m_victoryTimer <= 0.f) {
		EnterState( GameState::ATTRACT );
	}
}

void Game::RenderAttractMode() const
{
	g_theRenderer->BeginCamera( m_gameScreenCamera );

	// hard code: add verts for buttons

	AABB2 buttonBoxBounds = AABB2( Vec2( 600.f, 150.f ), Vec2( 1000.f, 600.f ) );

	std::vector<Vertex_PCU> shapeVerts;
	Rgba8 color1 = Rgba8( 192, 192, 192 );
	Rgba8 color2 = Rgba8( 0, 0, 0 );
	std::vector<Vertex_PCU> textVerts;
	Rgba8 textColor1 = Rgba8( 0, 0, 0 );
	Rgba8 textColor2 = Rgba8( 255, 255, 51 );

	float startOfY = buttonBoxBounds.m_maxs.y;
	float stepOfY = (buttonBoxBounds.m_maxs.y - buttonBoxBounds.m_mins.y) / m_numOfMaps;
	float ratioOfButton = 0.4f;
	for (int i = 0; i < m_numOfMaps; i++) {
		AABB2 buttonBounds = AABB2( Vec2( buttonBoxBounds.m_mins.x, startOfY - stepOfY * ratioOfButton ), Vec2( buttonBoxBounds.m_maxs.x, startOfY ) );
		AABB2 textBounds = AABB2( buttonBounds.m_mins, buttonBounds.m_maxs - Vec2( 0.f, 6.f ) );
		if (m_mapChoose == i) {
			AddVertsForAABB2D( shapeVerts, buttonBounds, color2 );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, buttonBounds, 30.f, m_mapDefs[i].m_mapName, textColor2 );
			Rgba8 lineColor = Rgba8();
			float lineThickness = 4.f;
			AddVertsForLineSegment2D( shapeVerts, buttonBounds.m_maxs + Vec2( 5.f, 5.f ), buttonBounds.m_maxs + Vec2( -35.f , 5.f ), lineThickness, lineColor );
			AddVertsForLineSegment2D( shapeVerts, buttonBounds.m_maxs + Vec2( 5.f, 5.f ), buttonBounds.m_maxs + Vec2( 5.f, 5.f - stepOfY * 0.08f ), lineThickness, lineColor );
			AddVertsForLineSegment2D( shapeVerts, buttonBounds.m_mins - Vec2( 5.f, 5.f ), buttonBounds.m_mins + Vec2( 35.f, -5.f ), lineThickness, lineColor );
			AddVertsForLineSegment2D( shapeVerts, buttonBounds.m_mins - Vec2( 5.f, 5.f ), buttonBounds.m_mins + Vec2( -5.f, -5.f + stepOfY * 0.08f ), lineThickness, lineColor );
			AddVertsForLineSegment2D( shapeVerts, Vec2( buttonBounds.m_mins.x, buttonBounds.m_maxs.y ) + Vec2( -5.f, 5.f ),
				Vec2( buttonBounds.m_mins.x, buttonBounds.m_maxs.y ) + Vec2( 35.f, 5.f ), lineThickness, lineColor );
			AddVertsForLineSegment2D( shapeVerts, Vec2( buttonBounds.m_mins.x, buttonBounds.m_maxs.y ) + Vec2( -5.f, 5.f ),
				Vec2( buttonBounds.m_mins.x, buttonBounds.m_maxs.y ) + Vec2( -5.f, 5.f - stepOfY * 0.08f ), lineThickness, lineColor );
			AddVertsForLineSegment2D( shapeVerts, Vec2( buttonBounds.m_maxs.x, buttonBounds.m_mins.y ) + Vec2( 5.f, -5.f ),
				Vec2( buttonBounds.m_maxs.x, buttonBounds.m_mins.y ) + Vec2( -35.f, -5.f ), lineThickness, lineColor );
			AddVertsForLineSegment2D( shapeVerts, Vec2( buttonBounds.m_maxs.x, buttonBounds.m_mins.y ) + Vec2( 5.f, -5.f ),
				Vec2( buttonBounds.m_maxs.x, buttonBounds.m_mins.y ) + Vec2( 5.f, -5.f + stepOfY * 0.08f ), lineThickness, lineColor );
		}
		else {
			AddVertsForAABB2D( shapeVerts, buttonBounds, color1 );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, buttonBounds, 30.f, m_mapDefs[i].m_mapName, textColor1 );
		}
		startOfY -= stepOfY;
	}

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( shapeVerts );
	g_theRenderer->EndCamera( m_gameScreenCamera );


	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 0.f ), Vec2( 1600.f, 200.f ) ), 30.f,
		"Press SPACE to join with mouse and keyboard\nPress START to join with controller\nPress ESCAPE or BACK to exit" );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 670.f ), Vec2( 1600.f, 750.f ) ), 80.f, "DOOMENSTEIN GOLD", Rgba8( 255, 255, 0 ) );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );
	g_theRenderer->EndCamera( m_gameScreenCamera );

}

void Game::RenderPlayingMode() const
{
	// Game Camera
	g_theRenderer->BeginCamera( m_players[0]->m_worldCamera );
	m_curMap->RenderWorld( m_players[0]->m_worldCamera );
	g_theRenderer->EndCamera( m_players[0]->m_worldCamera );
	DebugRenderWorld( m_players[0]->m_worldCamera );

	// UI Camera
	g_theRenderer->BeginCamera( m_players[0]->m_uiCamera );
	m_players[0]->RenderUI();
	g_theRenderer->EndCamera( m_players[0]->m_uiCamera );

	if (m_numOfPlayers == 2) {
		// Game Camera
		g_theRenderer->BeginCamera( m_players[1]->m_worldCamera );
		m_curMap->RenderWorld( m_players[1]->m_worldCamera );
		g_theRenderer->EndCamera( m_players[1]->m_worldCamera );
		DebugRenderWorld( m_players[1]->m_worldCamera );

		// UI Camera
		g_theRenderer->BeginCamera( m_players[1]->m_uiCamera );
		m_players[1]->RenderUI();
		g_theRenderer->EndCamera( m_players[1]->m_uiCamera );
	}


	g_theRenderer->BeginCamera( m_gameScreenCamera );
	m_curMap->RenderUI();
	g_theRenderer->EndCamera( m_gameScreenCamera );

	DebugRenderScreen( m_gameScreenCamera );
}

void Game::RenderLobbyState() const
{
	g_theRenderer->BeginCamera( m_gameScreenCamera );

	std::vector<Vertex_PCU> textVerts;
	for (int i = 0; i < m_numOfPlayers; i++) {
		if (m_players[i]->m_controllerIndex == -1) {
			if (m_numOfPlayers == 1) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 600.f, 200.f ), Vec2( 1600.f, 600.f ) ), 25.f,
					Stringf( "Player%d\nMouse and Keyboard\nPress SPACE to start game\nPress START to join a player with controller\nPress ESCAPE to leave game", (i + 1) ) );
			}
			else if (m_numOfPlayers == 2) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 600.f, 800.f - 400.f * (float)(i + 1) ), Vec2( 1600.f, 800.f - 400.f * (float)i ) ), 25.f,
					Stringf( "Player%d\nMouse and Keyboard\nPress SPACE to start game\nPress ESCAPE to leave game", (i + 1) ) );
			}
		}
		else {
			if (m_numOfPlayers == 1) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 600.f, 200.f ), Vec2( 1600.f, 600.f ) ), 25.f,
					Stringf( "Player%d\nController\nPress START to start game\nPress SPACE or START to join a player\nPress BACK to leave game", (i + 1) ) );
			}
			else if (m_numOfPlayers == 2) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 600.f, 800.f - 400.f * (float)(i + 1) ), Vec2( 1600.f, 800.f - 400.f * (float)i ) ), 25.f,
					Stringf( "Player%d\nController\nPress START to start game\nPress BACK to leave game", (i + 1) ) );
			}

		}
	}
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 50.f, 200.f ), Vec2( 600.f, 600.f ) ), 30.f, Stringf( (std::string( "Task:\n" ) + m_mapDefs[m_mapChoose].m_description).c_str() ) );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );
	g_theRenderer->EndCamera( m_gameScreenCamera );
}

void Game::RenderVictoryMode() const
{
	for (int i = 0; i < m_numOfPlayers; i++) {
		if (m_players[i]->m_isVictory) {
			g_theRenderer->BeginCamera( m_players[i]->m_uiCamera );
			std::vector<Vertex_PCU> verts;
			AddVertsForAABB2D( verts, AABB2( Vec2( 0.f, 0.f ), Vec2( 1600.f, 800.f ) ), Rgba8::WHITE, AABB2::IDENTITY );

			g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
			g_theRenderer->SetDepthMode( DepthMode::DISABLED );
			g_theRenderer->SetBlendMode( BlendMode::ALPHA );
			g_theRenderer->BindShader( nullptr );
			g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( g_gameConfigBlackboard.GetValue( "victoryScreen", "Data/Images/VictoryScreen.jpg" ).c_str() ) );
			g_theRenderer->SetModelConstants();
			g_theRenderer->DrawVertexArray( verts );

			g_theRenderer->EndCamera( m_players[i]->m_uiCamera );
		}
	}
}

void Game::UpdateSounds()
{
	std::vector<SoundPlaybackID> soundsToErase;
	for (auto soundPair : m_3DSounds) {
		if (soundPair.first != MISSING_SOUND_ID && soundPair.second.IsValid() && g_theAudio->IsSoundPlaying( soundPair.first )) {
			g_theAudio->SetSoundPosition( soundPair.first, soundPair.second->m_position );
		}
		else {
			soundsToErase.push_back( soundPair.first );
		}
	}

	for (auto sound : soundsToErase) {
		m_3DSounds.erase( sound );
	}

	for (int i = 0; i < m_numOfPlayers; i++) {
		PlayerController* controller = m_players[i];
		Vec3 iBasis, jBasis, kBasis;
		controller->m_worldCamera.m_orientation.GetAsVectors_IFwd_JLeft_KUp( iBasis, jBasis, kBasis );
		g_theAudio->UpdateListener( i, controller->m_worldCamera.m_position, iBasis, kBasis );
	}
}

