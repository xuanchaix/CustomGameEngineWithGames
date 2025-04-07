#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Prop.hpp"
#include "Game/Player.hpp"
#include "Game/Map.hpp"
#include "Game/Tile.hpp"
#include "Game/Unit.hpp"

AABB2 const g_menuButton1Trigger = AABB2( Vec2( 330.f, 450.f ), Vec2( 900.f, 500.f ) );
AABB2 const g_menuButton2Trigger = AABB2( Vec2( 330.f, 400.f ), Vec2( 900.f, 450.f ) );
AABB2 const g_menuButton3Trigger = AABB2( Vec2( 330.f, 350.f ), Vec2( 900.f, 400.f ) );

Game::Game()
{
	// load random number generator
	m_randNumGen = new RandomNumberGenerator();
	m_gameClock = new Clock();
}

Game::~Game()
{
	DebugRenderSystemShutdown();
	// delete all pointers managed by Game
	delete m_randNumGen;
	m_randNumGen = nullptr;
	delete m_map;
}

void Game::Startup()
{
	m_timer = new Timer( 1.f, m_gameClock );
	m_timer->Start();
	
	LoadDefinitions();

	SubscribeEventCallbackFunction( "Command_LoadMap", Game::Command_LoadMap );
	SubscribeEventCallbackFunction( "Command_PlayerReady", Command_PlayerReady );
	SubscribeEventCallbackFunction( "Command_StartTurn", Command_StartTurn );
	SubscribeEventCallbackFunction( "Command_SetFocusedHex", Command_SetFocusedHex );
	SubscribeEventCallbackFunction( "Command_SelectFocusedUnit", Command_SelectFocusedUnit );
	SubscribeEventCallbackFunction( "Command_SelectPreviousUnit", Command_SelectPreviousUnit );
	SubscribeEventCallbackFunction( "Command_SelectNextUnit", Command_SelectNextUnit );
	SubscribeEventCallbackFunction( "Command_Move", Command_Move );
	SubscribeEventCallbackFunction( "Command_Stay", Command_Stay );
	SubscribeEventCallbackFunction( "Command_HoldFire", Command_HoldFire );
	SubscribeEventCallbackFunction( "Command_Attack", Command_Attack );
	SubscribeEventCallbackFunction( "Command_Cancel", Command_Cancel );
	SubscribeEventCallbackFunction( "Command_EndTurn", Command_EndTurn );
	SubscribeEventCallbackFunction( "Command_PlayerQuit", Command_PlayerQuit );
	
	if (g_theNetSystem->GetNetMode() == NetMode::NONE) {
		m_menuButtonHoveringID = 2;
	}
	else{
		m_menuButtonHoveringID = 1;
	}

	m_screenCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 0.f, 1.f );
	m_screenCamera.m_mode = CameraMode::Orthographic;
}

void Game::Update()
{
	if (m_returnToMenu) {
		m_state = AppState::InMenu;
		delete m_map;
		m_map = nullptr;
		m_returnToMenu = false;
	}
	UpdateDebugRender();
	HandleKeys();
	if (m_state == AppState::InGame) {
		m_map->Update( m_gameClock->GetDeltaSeconds() );
	}
	else {
		UpdateUI();
	}
}

void Game::Render() const
{
	// Game Camera
	g_theRenderer->BeginCamera( m_playerPOV );
	if (m_state == AppState::InGame) {
		if (m_map) {
			m_map->Render();
		}
	}
	g_theRenderer->EndCamera( m_playerPOV );

	DebugRenderWorld( m_playerPOV );
	// UI Camera
	g_theRenderer->BeginCamera( m_screenCamera );
	if (m_state == AppState::InGame) {
		if (m_map) {
			m_map->RenderUI();
		}
	}
	else {
		RenderUI();
	}
	g_theRenderer->EndCamera( m_screenCamera );
	DebugRenderScreen( m_screenCamera );
}

void Game::HandleKeys()
{

//#ifdef DEBUG_MODE
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

//#endif // DEBUG_MODE

	if (m_state == AppState::InGame && g_theInput->WasKeyJustPressed( KEYCODE_ESC ) && m_map->m_gameState != GameState::EndTurn) {
		m_state = AppState::Pause;
	}

	if (m_map) {
		float deltaSeconds = g_theGame->m_gameClock->GetDeltaSeconds();
		float speed = 10.f;
		Vec3 iBasis, jBasis, kBasis;
		m_playerPOV.m_orientation.GetAsVectors_IFwd_JLeft_KUp( iBasis, jBasis, kBasis );

		iBasis.z = 0;
		iBasis.Normalize();
		jBasis.z = 0;
		jBasis.Normalize();

		if (g_theInput->IsKeyDown( 'W' )) {
			m_playerPOV.m_position += iBasis * speed * deltaSeconds;
		}
		if (g_theInput->IsKeyDown( 'S' )) {
			m_playerPOV.m_position -= iBasis * speed * deltaSeconds;
		}
		if (g_theInput->IsKeyDown( 'A' )) {
			m_playerPOV.m_position += jBasis * speed * deltaSeconds;
		}
		if (g_theInput->IsKeyDown( 'D' )) {
			m_playerPOV.m_position -= jBasis * speed * deltaSeconds;
		}
		if (g_theInput->IsKeyDown( 'Q' )) {
			m_playerPOV.m_position -= Vec3( 0, 0, 1 ) * speed * deltaSeconds;
		}
		if (g_theInput->IsKeyDown( 'E' )) {
			m_playerPOV.m_position += Vec3( 0, 0, 1 ) * speed * deltaSeconds;
		}

		Vec3 forward = m_playerPOV.m_orientation.GetIFwd();
		float fraction = GetFractionWithinRange( 0.f, m_playerPOV.m_position.z, (m_playerPOV.m_position + forward).z );
		Vec3 planePos = m_playerPOV.m_position + fraction * forward;
		planePos.x = GetClamped( planePos.x, m_map->m_cameraBounds.m_mins.x, m_map->m_cameraBounds.m_maxs.x );
		planePos.y = GetClamped( planePos.y, m_map->m_cameraBounds.m_mins.y, m_map->m_cameraBounds.m_maxs.y );
		m_playerPOV.m_position = planePos - fraction * forward;

		m_playerPOV.m_position.z = GetClamped( m_playerPOV.m_position.z, m_map->m_cameraBounds.m_mins.z, m_map->m_cameraBounds.m_maxs.z );
		m_playerPOV.m_position.z = GetClamped( m_playerPOV.m_position.z, 1.f, FLT_MAX );
	}
}

bool Game::Command_LoadMap( EventArgs& args )
{
	std::string name = args.GetValue( "Name", "None" );
	if (name == "None") {
		g_devConsole->AddLine( DevConsole::INFO_ERROR, "Syntax Error!" );
		return true;
	}
	else {
		MapDefinition const& def = MapDefinition::GetDefinition( name );
		if (&def == nullptr) {
			return true;
		}
		delete g_theGame->m_map;
		g_theGame->m_map = new Map( def );
		g_theGame->m_map->StartUp();
		g_theGame->SetUpCameras();
		return false;
	}
}

void Game::LoadDefinitions()
{
	MapDefinition::s_definitions.reserve( 10 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Definitions/MapDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document MapDefinitions.xml error" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "MapDefinitions" ), "Syntax Error! Name of the root of MapDefinitions.xml should be \"MapDefinitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "MapDefinition" ), "Syntax Error! Names of the elements of MapDefinitions.xml should be \"MapDefinition\" " );
		MapDefinition::s_definitions.emplace_back( xmlIter );
		xmlIter = xmlIter->NextSiblingElement();
	}

	TileDefinition::s_definitions.reserve( 10 );
	xmlDocument;
	errorCode = xmlDocument.LoadFile( "Data/Definitions/TileDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document TileDefinitions.xml error" );
	root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "TileDefinitions" ), "Syntax Error! Name of the root of TileDefinitions.xml should be \"TileDefinitions\" " );
	xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "TileDefinition" ), "Syntax Error! Names of the elements of TileDefinition.xml should be \"TileDefinition\" " );
		TileDefinition::s_definitions.emplace_back( xmlIter );
		xmlIter = xmlIter->NextSiblingElement();
	}

	UnitDefinition::s_definitions.reserve( 10 );
	xmlDocument;
	errorCode = xmlDocument.LoadFile( "Data/Definitions/UnitDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document UnitDefinitions.xml error" );
	root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "UnitDefinitions" ), "Syntax Error! Name of the root of UnitDefinitions.xml should be \"UnitDefinitions\" " );
	xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "UnitDefinition" ), "Syntax Error! Names of the elements of UnitDefinition.xml should be \"UnitDefinition\" " );
		UnitDefinition::s_definitions.emplace_back( xmlIter );
		xmlIter = xmlIter->NextSiblingElement();
	}
}

void Game::UpdateDebugRender()
{
	// DebugAddScreenText( Stringf( "Time: %.2f FPS: %.2f Scale: %.1f", Clock::GetSystemClock()->GetTotalSeconds(), 1.f / Clock::GetSystemClock()->GetDeltaSeconds(), Clock::GetSystemClock()->GetTimeScale() ), m_screenCamera.m_cameraBox.m_maxs - Vec2( 400.f, 20.f ), 20.f, Vec2( 0.5f, 0.5f ), 0.f, Rgba8::WHITE, Rgba8::WHITE );
	// DebugAddMessage( Stringf( "Player Position: %.2f %.2f %.2f", m_playerPOV.m_position.x, m_playerPOV.m_position.y, m_playerPOV.m_position.z ), 0.f, Rgba8( 255, 255, 255 ), Rgba8( 255, 255, 255 ) );
}

void Game::SetUpCameras()
{
	// set up camera
	//m_screenCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ) );


	m_playerPOV.SetRenderBasis( Vec3( 0.f, 0.f, 1.f ), Vec3( -1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ) );
	m_playerPOV.SetOrthoView( Vec2( -1.f, -1.f ), Vec2( 1.f, 1.f ), 0.f, 1.f );
	m_playerPOV.SetPerspectiveView( g_window->GetAspect(), 60.f, 0.01f, 100.f );
	m_playerPOV.m_mode = CameraMode::Perspective;
	m_playerPOV.m_position = Vec3( m_map->m_mapBounds.GetCenter().x, m_map->m_mapBounds.GetCenter().y * 0.5f, 5.f);
	m_playerPOV.m_orientation.m_yawDegrees = 90.f;
	m_playerPOV.m_orientation.m_pitchDegrees = 60.f;
}

void Game::RenderUI() const
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> screenBackground;
	textVerts.reserve( 100 );
	verts.reserve( 100 );
	screenBackground.reserve( 6 );

	AddVertsForAABB2D( screenBackground, AABB2( Vec2( 600.f, 200.f ), Vec2( 1000.f, 600.f ) ), Rgba8::WHITE, AABB2::IDENTITY );
	if (m_state == AppState::AttractScreen) {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 80.f ), Vec2( 1600.f, 130.f ) ), 50.f, "Press ENTER or click to start", Rgba8::WHITE, 0.5f );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 650.f ), Vec2( 1600.f, 750.f ) ), 100.f, "Vaporum", Rgba8::WHITE, 0.5f );
	}
	else if (m_state == AppState::InMenu) {
		AddVertsForAABB2D( verts, AABB2( Vec2( 315.f, 0.f ), Vec2( 330.f, 800.f ) ), Rgba8::WHITE );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( -800.f, -40.f ), Vec2( 800.f, 40.f ) ), 80.f, "Main Menu", Rgba8::WHITE, 0.5f );
		TransformVertexArrayXY3D( textVerts, Vec2( 0.f, 1.f ), Vec2( -1.f, 0.f ), Vec2( 280.f, 400.f ) );

		if (g_theNetSystem->GetNetMode() == NetMode::NONE) {
			if (m_menuButtonHoveringID == 2) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, g_menuButton2Trigger, 60.f, "Local Game", Rgba8::BLACK, 0.5f, Vec2( 0.05f, 0.5f ) );
			}
			else {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, g_menuButton2Trigger, 60.f, "Local Game", Rgba8::WHITE, 0.5f, Vec2( 0.05f, 0.5f ) );
			}
		}
		else {
			if (m_menuButtonHoveringID == 1) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, g_menuButton1Trigger, 60.f, "Local Game", Rgba8::BLACK, 0.5f, Vec2( 0.05f, 0.5f ) );
			}
			else {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, g_menuButton1Trigger, 60.f, "Local Game", Rgba8::WHITE, 0.5f, Vec2( 0.05f, 0.5f ) );
			}
			if (m_menuButtonHoveringID == 2) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, g_menuButton2Trigger, 60.f, "Network Game", Rgba8::BLACK, 0.5f, Vec2( 0.05f, 0.5f ) );
			}
			else {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, g_menuButton2Trigger, 60.f, "Network Game", Rgba8::WHITE, 0.5f, Vec2( 0.05f, 0.5f ) );
			}
		}
		if (m_menuButtonHoveringID == 3) {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, g_menuButton3Trigger, 60.f, "Quit", Rgba8::BLACK, 0.5f, Vec2( 0.05f, 0.5f ) );
		}
		else {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, g_menuButton3Trigger, 60.f, "Quit", Rgba8::WHITE, 0.5f, Vec2( 0.05f, 0.5f ) );
		}

		if (m_menuButtonHoveringID == 1) {
			AddVertsForUIPanelWhite( verts, g_menuButton1Trigger, 2.f );
		}
		else if (m_menuButtonHoveringID == 2) {
			AddVertsForUIPanelWhite( verts, g_menuButton2Trigger, 2.f );
		}
		else if (m_menuButtonHoveringID == 3) {
			AddVertsForUIPanelWhite( verts, g_menuButton3Trigger, 2.f );
		}
	}
	else if (m_state == AppState::Pause) {
		AddVertsForAABB2D( verts, AABB2( Vec2( 315.f, 0.f ), Vec2( 330.f, 800.f ) ), Rgba8::WHITE );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( -800.f, -40.f ), Vec2( 800.f, 40.f ) ), 80.f, "Pause Menu", Rgba8::WHITE, 0.5f );
		TransformVertexArrayXY3D( textVerts, Vec2( 0.f, 1.f ), Vec2( -1.f, 0.f ), Vec2( 280.f, 400.f ) );

		if (m_menuButtonHoveringID == 2) {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, g_menuButton2Trigger, 60.f, "Resume Game", Rgba8::BLACK, 0.5f, Vec2( 0.05f, 0.5f ) );
		}
		else {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, g_menuButton2Trigger, 60.f, "Resume Game", Rgba8::WHITE, 0.5f, Vec2( 0.05f, 0.5f ) );
		}
		if (m_menuButtonHoveringID == 3) {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, g_menuButton3Trigger, 60.f, "Main Menu", Rgba8::BLACK, 0.5f, Vec2( 0.05f, 0.5f ) );
		}
		else {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, g_menuButton3Trigger, 60.f, "Main Menu", Rgba8::WHITE, 0.5f, Vec2( 0.05f, 0.5f ) );
		}

		if (m_menuButtonHoveringID == 2) {
			AddVertsForUIPanelWhite( verts, g_menuButton2Trigger, 2.f );
		}
		else if (m_menuButtonHoveringID == 3) {
			AddVertsForUIPanelWhite( verts, g_menuButton3Trigger, 2.f );
		}
	}

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Logo.png" ) );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( screenBackground );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );
}

void Game::UpdateUI()
{
	if (m_state == AppState::AttractScreen) {
		if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE ) || g_theInput->WasKeyJustPressed( KEYCODE_ENTER )) {
			m_state = AppState::InMenu;
		}
	}
	else if (m_state == AppState::InMenu) {
		Vec2 cursorUIPos =  m_screenCamera.GetCursorWorldPosition( g_window->GetNormalizedCursorPos() );
		if (g_theNetSystem->GetNetMode() != NetMode::NONE && g_menuButton1Trigger.IsPointInside( cursorUIPos )) {
			m_menuButtonHoveringID = 1;
		}
		else if (g_menuButton2Trigger.IsPointInside( cursorUIPos )) {
			m_menuButtonHoveringID = 2;
		}
		else if (g_menuButton3Trigger.IsPointInside( cursorUIPos )) {
			m_menuButtonHoveringID = 3;
		}
		if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE ) || g_theInput->WasKeyJustPressed( KEYCODE_ENTER )) {
			if (g_theNetSystem->GetNetMode() != NetMode::NONE) {
				if (m_menuButtonHoveringID == 1) {
					delete m_map;
					m_state = AppState::InGame;
					m_map = new Map( MapDefinition::GetDefinition( "Grid12x12" ) );
					//m_map = new Map( MapDefinition::GetDefinition( "Grid4x4" ) );
					m_map->m_isOnlineNetworkingGame = false;
					m_map->StartUp();
					SetUpCameras();
				}
				else if (m_menuButtonHoveringID == 2) {
					delete m_map;
					m_state = AppState::InGame;
					m_map = new Map( MapDefinition::GetDefinition( "Grid12x12" ) );
					//m_map = new Map( MapDefinition::GetDefinition( "Grid4x4" ) );
					m_map->m_isOnlineNetworkingGame = true;
					m_map->StartUp();
					SetUpCameras();
				}
				else if (m_menuButtonHoveringID == 3) {
					FireEvent( "quit" );
				}
			}
			else {
				if (m_menuButtonHoveringID == 2) {
					delete m_map;
					m_state = AppState::InGame;
					m_map = new Map( MapDefinition::GetDefinition( "Grid12x12" ) );
					//m_map = new Map( MapDefinition::GetDefinition( "Grid4x4" ) );
					m_map->m_isOnlineNetworkingGame = false;
					m_map->StartUp();
					SetUpCameras();
				}
				else if (m_menuButtonHoveringID == 3) {
					FireEvent( "quit" );
				}
			}
		}
	}
	else if (m_state == AppState::Pause) {
		Vec2 cursorUIPos = m_screenCamera.GetCursorWorldPosition( g_window->GetNormalizedCursorPos() );
		if (g_menuButton2Trigger.IsPointInside( cursorUIPos )) {
			m_menuButtonHoveringID = 2;
		}
		else if (g_menuButton3Trigger.IsPointInside( cursorUIPos )) {
			m_menuButtonHoveringID = 3;
		}

		if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE ) || g_theInput->WasKeyJustPressed( KEYCODE_ENTER )) {
			if (m_menuButtonHoveringID == 2) {
				m_state = AppState::InGame;
			}
			else if (m_menuButtonHoveringID == 3) {
				m_state = AppState::InMenu;
				delete m_map;
				m_map = nullptr;
			}
		}
	}
}
