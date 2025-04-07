#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/GameCommon.hpp"

// follow the instructions on the manual
// All global variables Created and owned by the App
App* g_theApp = nullptr;
Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_window = nullptr;
BitmapFont* g_ASCIIFont = nullptr;
Game* g_theGame = nullptr;

App::App() {
	g_theGame = nullptr;
}

App::~App() {
	
}

void App::Startup() {
	Clock::TickSystemClock();

	EventSystemConfig eConfig;
	g_theEventSystem = new EventSystem( eConfig );
	g_theEventSystem->Startup();

	WindowConfig wConfig;
	wConfig.m_clientAspect = 2.f;
	wConfig.m_isFullScreen = false;
	wConfig.m_windowTitle = std::string( APP_NAME );
	g_window = new Window( wConfig );
	g_window->StartUp();

	RendererConfig rConfig;
	rConfig.m_window = g_window;
	g_theRenderer = new Renderer( rConfig );
	g_theRenderer->StartUp();
	g_ASCIIFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

	DevConsoleConfig dConfig;
	dConfig.m_defaultFont = g_ASCIIFont;
	dConfig.m_defaultRenderer = g_theRenderer;
	dConfig.m_camera = &m_attractModeCamera;
	g_devConsole = new DevConsole( dConfig );
	g_devConsole->Startup();

	InputSystemConfig iConfig;
	g_theInput = new InputSystem( iConfig );
	g_theInput->StartUp();

	AudioSystemConfig aConfig;
	g_theAudio = new AudioSystem( aConfig );
	g_theAudio->Startup();


	SetUpAudio();
	m_attractModeMusic = g_theAudio->StartSound( m_audioDictionary[(int)AudioName::attractMode], true );
	//m_theGame = new Game();
	//m_theGame->Startup();

	SubscribeEventCallbackFunction( "quit", App::SetQuitting );
	SubscribeEventCallbackFunction( "Command_settimescale", Game::Command_SetTimeScale );
	g_devConsole->AddLine( DevConsole::INFO_MINOR, "Initializing..." );

	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Successfully Initialized!" );

	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Input Info:" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "P: Pause O: Run a Frame then Pause T: Time scale = 0.1" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "I: Randomly Generate a Asteroid Esc: Exit Game F8: Restart Game" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "See Other Key Input in Pause Menu" );

	m_attractModeCamera.m_mode = CameraMode::Orthographic;
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );

	IntVec2 dimensions = g_window->GetClientDimensions();
	float xLength = (float)dimensions.x;
	float yLength = (float)dimensions.y;
	m_attractModeCamera.SetViewPort( AABB2( Vec2( 0.f, 0.f ), Vec2( xLength, yLength ) ) );
}

void App::Shutdown() {
	g_theRenderer->Shutdown();
	g_theInput->ShutDown();
	g_theAudio->Shutdown();
	g_window->Shutdown();
	g_theEventSystem->Shutdown();
	g_devConsole->Shutdown();

	if (g_theGame) {
		delete g_theGame;
		g_theGame = nullptr;
	}
	delete g_theRenderer;
	delete g_theInput;
	delete g_theAudio;
	delete g_window;

	g_theInput = nullptr;
	g_theRenderer = nullptr;
	g_theAudio = nullptr;
	g_window = nullptr;
}

void App::Run()
{
	while (!m_isQuitting)
	{
		if (m_isQuitting) {
			break;
		}
		RunFrame();
	}
}

void App::RunFrame() {
	BeginFrame();
 	Update();
	Render();
	EndFrame();
}

/// <summary>
/// Handle key press in app
/// </summary>
/// <returns></returns>
void App::HandleKey() {
	if (g_theInput->WasKeyJustPressed( KEYCODE_ESC ) && m_attractMode) // Escape key quit the game
	{
		m_isQuitting = true;
	}
	if (g_theInput->WasKeyJustPressed( KEYCODE_ESC ) && !m_attractMode)
	{
		ToAttractMode();
	}
#ifndef DUAL_PLAYERS
	// ' ' or N enter game
	if ((g_theInput->WasKeyJustPressed( 0x4E )
		|| g_theInput->WasKeyJustPressed( KEYCODE_SPACE )
		|| g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_START )
		|| g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_A ))
		&& m_attractMode)
	{
		m_attractMode = false;
		g_theAudio->StopSound( m_attractModeMusic );
		m_startButtonA = -STRAT_BUTTON_TIME;
		g_theGame = new Game();
		g_theGame->Startup();
		m_gameModeMusic = g_theAudio->StartSound( m_audioDictionary[(int)AudioName::battle], true );
	}
#else
	// ' ' or N enter game
	if ((g_theInput->WasKeyJustPressed( 0x4E )
		|| g_theInput->WasKeyJustPressed( KEYCODE_SPACE )
		|| g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_START )
		|| g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_A )
		|| g_theInput->GetController( 1 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_START )
		|| g_theInput->GetController( 1 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_A ))
		&& m_attractMode)
	{
		m_attractMode = false;
		m_startButtonA = -STRAT_BUTTON_TIME;
		m_theGame = new Game();
		m_theGame->Startup();
	}
#endif // DUAL_PLAYERS

#ifdef DEBUG_MODE
	if (g_theInput->WasKeyJustPressed( KEYCODE_F1 )) // F1 debug mode
	{
		m_debugMode = !m_debugMode;
	}
#endif
	if (g_theInput->WasKeyJustPressed( KEYCODE_F8 )) {
		delete g_theGame;
		g_theGame = new Game();
		g_theGame->Startup();
	}
}

void App::ToAttractMode()
{
	m_toAttractMode = true;
}

SoundID App::GetSoundId( AudioName name )
{
	return m_audioDictionary[(int)name];
}

void App::BeginFrame() {
	Clock::TickSystemClock();
	g_window->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theInput->BeginFrame();
	g_theAudio->BeginFrame();
	g_devConsole->BeginFrame();
	g_theEventSystem->BeginFrame();
}

void App::Update() {
	float deltaSeconds = Clock::GetSystemClock()->GetDeltaSeconds();
	if (m_toAttractMode) {
		m_toAttractMode = false;
		g_theAudio->StopSound( m_gameModeMusic );
		delete g_theGame;
		g_theGame = nullptr;
		m_attractMode = true;
		m_attractModeMusic = g_theAudio->StartSound( m_audioDictionary[(int)AudioName::attractMode], true );
	}

	m_attractModeCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );

	if (m_attractMode) {
		m_startButtonA += deltaSeconds;
		if (m_startButtonA > STRAT_BUTTON_TIME) {
			m_startButtonA = -STRAT_BUTTON_TIME;
		}
	}

	if (!m_attractMode) {
		g_theGame->Update();
	}

	HandleKey();
}

void App::Render() const 
{
	// Clear all screen (back buffer) pixels to medium-blue
	g_theRenderer->ClearScreen( Rgba8( 0, 0, 0, 255 ) );

	if (!m_attractMode) {
		g_theGame->Render();
	}
	else {
		g_theRenderer->BeginCamera( m_attractModeCamera );
		RenderAttractMode();
		g_theRenderer->EndCamera( m_attractModeCamera );
	}

	if (g_devConsole->GetMode() == DevConsoleMode::EMERGE) {
		g_devConsole->Render( m_attractModeCamera.m_cameraBox );
	}
}

void App::EndFrame() {
	g_window->EndFrame();
	g_theRenderer->EndFrame();
	g_theInput->EndFrame();
	g_theAudio->EndFrame();
	g_devConsole->EndFrame();
	g_theEventSystem->EndFrame();
}

void App::RenderAttractMode() const
{
	float a;
	if (m_startButtonA < 0) {
		a = -m_startButtonA;
	}
	else {
		a = m_startButtonA;
	}

	std::vector<Vertex_PCU> spaceVerts;
	spaceVerts.reserve( 6 );
	AddVertsForAABB2D( spaceVerts, AABB2( Vec2( 615.f, 78.f ), Vec2( 751.f, 118.f ) ), Rgba8::WHITE, AABB2::IDENTITY );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( spaceVerts );
	DrawXBoxButton( Vec2( 838.f, 97.f ), 28.f, XboxButtonID::XBOX_BUTTON_A, Rgba8( 0, 0, 0 ) );

	std::vector<Vertex_PCU> textVerts;
	textVerts.reserve( 100 );
	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 395.f, 580.f ), 120.f, "STAR  SHIP", Rgba8( 200, 150, 50, 255 ) );
	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 503.f, 80.f ), 30.f, "PRESS", Rgba8( 192, 192, 192, 255 ) );
	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 638.f, 80.f ), 30.f, "SPACE", Rgba8( 0, 0, 0, 255 ) );
	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 765.f, 80.f ), 30.f, "OR    TO START", Rgba8( 192, 192, 192, 255 ) );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( (int)textVerts.size(), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );

	a = RangeMap( a, 0.f, 1.f, 0.2f, 1.f );
	Vertex_PCU startIconVerts[3] = {
		Vertex_PCU( Vec3( 736, 286, 0 ), Rgba8( 0, 255, 0, (unsigned char)(255 * a) ), Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 736, 414, 0 ), Rgba8( 0, 255, 0, (unsigned char)(255 * a) ), Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 864, 350, 0 ), Rgba8( 0, 255, 0, (unsigned char)(255 * a) ), Vec2( 0, 0 ) ),
	};
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 3, startIconVerts );
	Vertex_PCU localShipVerts1[15];
	PlayerShip::GetShipVerts( localShipVerts1 );
	TransformVertexArrayXY3D( 15, localShipVerts1, 80.f, 0.f, Vec2( 300.f + a * 200, 350.f ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 15, localShipVerts1 );
	Vertex_PCU localShipVerts2[15];
	PlayerShip::GetShipVerts( localShipVerts2 );
	TransformVertexArrayXY3D( 15, localShipVerts2, 80.f, 180.f, Vec2( 1300.f - a * 200, 350.f ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 15, localShipVerts2 );

	//DrawJoyStick( Vec2( 100.f, 100.f ), 30.f, Rgba8::WHITE );
	//DrawKeyBoardButtom( AABB2( Vec2( 300.f, 300.f ), Vec2( 330.f, 330.f ) ), 'A', Rgba8::WHITE );
	//DrawKeyBoardButtom( AABB2( Vec2( 360.f, 300.f ), Vec2( 390.f, 330.f ) ), 'B', Rgba8( 0, 0, 0 ) );
	//DrawKeyBoardButtom( AABB2( Vec2( 420.f, 300.f ), Vec2( 450.f, 330.f ) ), 'C', Rgba8::WHITE );
	//DrawKeyBoardButtom( AABB2( Vec2( 480.f, 300.f ), Vec2( 510.f, 330.f ) ), 'D', Rgba8( 0, 0, 0 ) );
}

void App::SetUpAudio()
{
	//shoot, playerDie, playerReborn, enemyDie, newWave, victory, defeat, attractMode, battle, boss
	m_audioDictionary[(int)AudioName::shoot] = g_theAudio->CreateOrGetSound( "Data/Sound/PlayerShoot.wav" );
	m_audioDictionary[(int)AudioName::playerDie] = g_theAudio->CreateOrGetSound( "Data/Sound/Explosion.wav" );
	m_audioDictionary[(int)AudioName::playerReborn] = g_theAudio->CreateOrGetSound( "Data/Sound/PlayerReborn.wav" );
	m_audioDictionary[(int)AudioName::enemyDie] = g_theAudio->CreateOrGetSound( "Data/Sound/EnemyDie.wav" );
	m_audioDictionary[(int)AudioName::newLevel] = g_theAudio->CreateOrGetSound( "Data/Sound/NewLevel.mp3" );
	m_audioDictionary[(int)AudioName::victory] = g_theAudio->CreateOrGetSound( "Data/Sound/YouWin.mp3" );
	m_audioDictionary[(int)AudioName::defeat] = g_theAudio->CreateOrGetSound( "Data/Sound/YouLose.mp3" );
	m_audioDictionary[(int)AudioName::laser] = g_theAudio->CreateOrGetSound( "Data/Sound/Laser.wav" );
	m_audioDictionary[(int)AudioName::powerUp] = g_theAudio->CreateOrGetSound( "Data/Sound/Powerup.wav" );
	m_audioDictionary[(int)AudioName::health] = g_theAudio->CreateOrGetSound( "Data/Sound/Powerup2.wav" );
	m_audioDictionary[(int)AudioName::coneAttack] = g_theAudio->CreateOrGetSound( "Data/Sound/coneAttack.wav" );
	m_audioDictionary[(int)AudioName::rocket] = g_theAudio->CreateOrGetSound( "Data/Sound/rocket.wav" );
	m_audioDictionary[(int)AudioName::lightSaberOn] = g_theAudio->CreateOrGetSound( "Data/Sound/lightsaberStart.mp3" );
	m_audioDictionary[(int)AudioName::lightSaberRotate] = g_theAudio->CreateOrGetSound( "Data/Sound/lightsaberRotate.mp3" );
	m_audioDictionary[(int)AudioName::attractMode] = g_theAudio->CreateOrGetSound( "Data/Music/MainMenu.mp3" );
	m_audioDictionary[(int)AudioName::battle] = g_theAudio->CreateOrGetSound( "Data/Music/BattleBasic.mp3" );
	m_audioDictionary[(int)AudioName::boss] = g_theAudio->CreateOrGetSound( "Data/Music/Battle2.mp3" );
}

bool App::SetQuitting( EventArgs& args )
{
	UNUSED( args );
	g_theApp->m_isQuitting = true;
	return true;
}