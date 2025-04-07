#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <filesystem>

// follow the instructions on the manual
// All global variables Created and owned by the App
App* g_theApp = nullptr;
Game* g_theGame = nullptr;
Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_window = nullptr;
BitmapFont* g_ASCIIFont = nullptr;

App::App() {
}

App::~App() {

}

void App::Startup() {
	std::filesystem::create_directory( "Saves" );

	SetUpBlackBoard();
	Clock::TickSystemClock();

	JobSystemConfig jConfig;
	jConfig.m_numOfWorkers = -1;
	g_theJobSystem = new JobSystem( jConfig );
	g_theJobSystem->StartUp();
	if (g_theJobSystem->GetWorkersCount() > 5) {
		g_theJobSystem->SetWorkerThreadType( 0, saveLoadWorkerType );
	}

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
	SetUpTexture();
	DebugRenderConfig drConfig;
	drConfig.m_renderer = g_theRenderer;
	//drConfig.m_fontName = 
	DebugRenderSystemStartup( drConfig );

	m_attractModeCamera = new Camera();
	DevConsoleConfig dConfig;
	dConfig.m_defaultFont = g_ASCIIFont;
	dConfig.m_defaultRenderer = g_theRenderer;
	dConfig.m_camera = m_attractModeCamera;
	g_devConsole = new DevConsole( dConfig );
	g_devConsole->Startup();

	InputSystemConfig iConfig;
	//iConfig.m_curWindow = g_window;
	g_theInput = new InputSystem( iConfig );
	g_theInput->StartUp();
	g_theInput->SetCursorMode( true, true );

	AudioSystemConfig aConfig;
	g_theAudio = new AudioSystem( aConfig );
	g_theAudio->Startup();

	SetUpAudio();
	//m_theGame = new Game();
	//m_theGame->Startup();

	SubscribeEventCallbackFunction( "quit", App::SetQuitting );
	g_devConsole->AddLine( DevConsole::INFO_MINOR, "Initializing..." );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Successfully Initialized!" );

	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Input Info:" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "W: Forward S: Backward A: Left D: Right Q&E: Roll" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Mouse move: Camera Rotation H: Return to world position (0,0,0)" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Z/C: Move world up/down Shift: Speed * 10" );

	m_attractModeCamera->SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );
	m_attractModeCamera->m_mode = CameraMode::Orthographic;

	g_hiddenSurfaceRemovalEnable = g_gameConfigBlackboard.GetValue( "hiddenSurfaceRemovalEnable", g_hiddenSurfaceRemovalEnable );
	g_chunkActivationDist = g_gameConfigBlackboard.GetValue( "chunkActivationDistance", g_chunkActivationDist );
	g_saveModifiedChunks = g_gameConfigBlackboard.GetValue( "saveModifiedChunks", g_saveModifiedChunks );
	g_autoCreateChunks = g_gameConfigBlackboard.GetValue( "autoCreateChunks", g_autoCreateChunks );
}

void App::Shutdown() {
	delete m_attractModeCamera;
	g_theRenderer->Shutdown();
	g_theInput->ShutDown();
	g_theAudio->Shutdown();
	g_window->Shutdown();
	g_devConsole->Shutdown();
	g_theEventSystem->Shutdown();
	g_theJobSystem->ShutDown();

	delete g_theGame;
	delete g_theRenderer;
	delete g_theInput;
	delete g_theAudio;
	delete g_window;
	delete g_theEventSystem;
	delete g_sprite;
	delete g_theJobSystem;

	g_theGame = nullptr;
	g_theInput = nullptr;
	g_theRenderer = nullptr;
	g_theAudio = nullptr;
	g_window = nullptr;
	g_sprite = nullptr;
}

void App::Run()
{
	while (!m_isQuitting)
	{
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
//#ifdef DEBUG_MODE
	if (g_theInput->WasKeyJustPressed( KEYCODE_F1 )) // F1 debug mode
	{
		m_debugMode = !m_debugMode;
	}
	if (g_theInput->WasKeyJustPressed( KEYCODE_F8 )) { // F8 to restart the game
		delete g_theGame;
		g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Restart Game" );
		g_theGame = new Game();
		g_theGame->Startup();
	}
//#endif

	if ((g_theInput->WasKeyJustPressed( KEYCODE_ESC ) || g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_BACK )) && !m_attractMode)
	{
		if (g_theGame) {
			if (m_isPaused) {
				m_isPaused = false;
				ToAttractMode();
			}
			else {
				m_isPaused = true;
			}
		}
	}

	// ' ' or P enter game
	if ((g_theInput->WasKeyJustPressed( 'P' )
		|| g_theInput->WasKeyJustPressed( KEYCODE_SPACE )
		|| g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_START )
		|| g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_A ))
		&& m_attractMode)
	{
		m_attractMode = false;
		m_startButtonA = -STRAT_BUTTON_TIME;
		g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Enter Game" );
		g_theGame = new Game();
		g_theGame->Startup();
	}

	if (g_theInput->WasKeyJustPressed( KEYCODE_ESC ) && m_attractMode) {
		m_isQuitting = true;
	}
}

void App::ToAttractMode()
{
	m_toAttractMode = true;
}

SoundID App::GetSoundId( AudioName name ) const
{
	return m_audioDictionary[(int)name];
}

bool App::PlaySound( AudioName name, float intervalTimeSeconds, bool isLooped, float volume, float balance, float speed )
{
	double curTimeSeconds = GetCurrentTimeSeconds();
	if (float( curTimeSeconds - m_lastSoundPlaySecondsByID[(int)name] ) >= intervalTimeSeconds) {
		m_lastSoundPlaySecondsByID[(int)name] = curTimeSeconds;
		g_theAudio->StartSound( GetSoundId( name ), isLooped, volume, balance, speed );
		return true;
	}
	return false;
}

SpriteAnimDefinition* App::GetAnimation( AnimationName name )
{
	return m_animDictionary[(int)name];
}

void App::BeginFrame() {
	Clock::TickSystemClock();
	g_theJobSystem->BeginFrame();
	g_window->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theInput->BeginFrame();
	g_theAudio->BeginFrame();
	g_devConsole->BeginFrame();
	g_theEventSystem->BeginFrame();
	DebugRenderBeginFrame();
}

void App::Update() {
	float deltaSeconds = Clock::GetSystemClock()->GetDeltaSeconds();

	if (!g_window->IsFocus() || g_devConsole->GetMode() == DevConsoleMode::EMERGE || m_attractMode) {
		g_theInput->SetCursorMode( false, false );
	}
	else {
		g_theInput->SetCursorMode( true, true );
	}

	if (m_toAttractMode) {
		m_toAttractMode = false;
		delete g_theGame;
		g_theGame = nullptr;
		m_attractMode = true;
	}

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
	if (!m_attractMode) {
		g_theGame->Render();
	}
	else {
		g_theRenderer->ClearScreen( Rgba8( 64, 64, 64 ) );
		g_theRenderer->BeginCamera( *m_attractModeCamera );
		RenderAttractMode();
		g_theRenderer->EndCamera( *m_attractModeCamera );
	}

	if (g_devConsole->GetMode() == DevConsoleMode::EMERGE) {
		g_devConsole->Render( m_attractModeCamera->m_cameraBox );
	}
}

void App::EndFrame() {
	g_theJobSystem->EndFrame();
	g_window->EndFrame();
	g_theRenderer->EndFrame();
	g_theInput->EndFrame();
	g_theAudio->EndFrame();
	g_devConsole->EndFrame();
	g_theEventSystem->EndFrame();
	DebugRenderEndFrame();
}

void App::RenderAttractMode() const
{
	
	/*float a;
	if (m_startButtonA < 0) {
		a = -m_startButtonA;
	}
	else {
		a = m_startButtonA;
	}

	a = RangeMap( a, 0.f, 1.f, 0.2f, 1.f );
	Vertex_PCU startIconVerts[3] = {
		Vertex_PCU( Vec3( 750, 200, 0 ), Rgba8( 0, 255, 0, (unsigned char)(255 * a) ), Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 750, 600, 0 ), Rgba8( 0, 255, 0, (unsigned char)(255 * a) ), Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 1150, 400, 0 ), Rgba8( 0, 255, 0, (unsigned char)(255 * a) ), Vec2( 0, 0 ) ),
	};

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( 3, startIconVerts );
	*/

	//SpriteAnimDefinition* anim = m_animDictionary[(int)AnimationName::Explosion];
	//std::vector<Vertex_PCU> textVerts;
	//textVerts.reserve( 1000 );
	//AddVertsForAABB2D( textVerts, AABB2( Vec2( 0, 0 ), Vec2( 500, 500 ) ), Rgba8::WHITE, anim->GetSpriteDefAtTime( m_timePassed ).GetUVs() );
	//g_theRenderer->BindTexture( anim->GetTexture() );
	//g_theRenderer->DrawVertexArray( textVerts );
	/*
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2(0.f, 0.f), Vec2(300.f, 300.f) ), 50.f, std::string( "jdscnsjc\nnewcnewkcnkewnckjewnckjewn\nkew" ), Rgba8::WHITE, 0.6f, Vec2(1.f, 1.f), TextBoxMode::SHRINK_TO_FIT, 10 );
	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 250.f, 400.f ), 40.f, "It's nice to have options!", Rgba8::RED, 0.6f );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->DrawVertexArray( textVerts );*/

}

void App::SetUpAudio()
{
	m_audioDictionary[(int)AudioName::AttractMode] = g_theAudio->CreateOrGetSound( "Data/Music/MainMenu.mp3" );
}

void App::SetUpTexture()
{
	g_ASCIIFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
	g_sprite = new SpriteSheet( *(g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/BasicSprites_64x64.png" )), IntVec2( 64, 64 ) );
}

void App::SetUpBlackBoard()
{
	XmlDocument gameConfig;
	XmlError errorCode = gameConfig.LoadFile( "Data/GameConfig.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document GameConfig.xml error" );
	XmlElement* root = gameConfig.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "GameConfig" ), "Syntax Error! Name of the root of GameConfig.xml should be \"GameConfig\" " );

	g_gameConfigBlackboard.PopulateFromXmlElementAttributes( *root );
}

bool App::SetQuitting( EventArgs& args )
{
	UNUSED( args );
	g_theApp->m_isQuitting = true;
	return true;
}

