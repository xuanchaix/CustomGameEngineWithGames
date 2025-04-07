#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

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
	SetUpBlackBoard();
	Clock::TickSystemClock();

	EventSystemConfig eConfig;
	g_theEventSystem = new EventSystem( eConfig );
	g_theEventSystem->Startup();

	WindowConfig wConfig;
	wConfig.m_clientAspect = g_gameConfigBlackboard.GetValue( "windowAspect", 1.f );
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
	
	DevConsoleConfig dConfig;
	m_devConsoleCamera = new Camera();
	dConfig.m_defaultFont = g_ASCIIFont;
	dConfig.m_defaultRenderer = g_theRenderer;
	dConfig.m_camera = m_devConsoleCamera;
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

	SubscribeEventCallbackFunction( "quit", App::SetQuitting );
	g_devConsole->AddLine( DevConsole::INFO_MINOR, "Initializing..." );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Successfully Initialized!" );

	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Controls" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Mouse - Aim" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "W / S - Move Forward Back" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "A / D - Move Left Right" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Z / C - Elevate" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Shift - Sprint" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "1 - Pistol" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "2 - Plasma Rifle" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "P - Pause" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "O - Step Frame" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "F - Toggle Free Camera" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "N - Possess Next Actor" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "~ - Open Dev Console" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Escape - Exit Game" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Space - Start Game" );

	m_devConsoleCamera->SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 0.f, 1.f );
	m_devConsoleCamera->m_mode = CameraMode::Orthographic;

	g_theGame = new Game();
	g_theGame->Startup();
	g_theGame->m_attractModeMusic = PlaySound( g_gameConfigBlackboard.GetValue( "mainMenuMusic", "" ), 0.f, true, g_gameConfigBlackboard.GetValue( "musicVolume", 0.1f ) );
}

void App::Shutdown() {
	g_theRenderer->Shutdown();
	g_theInput->ShutDown();
	g_theAudio->Shutdown();
	g_window->Shutdown();
	g_devConsole->Shutdown();
	g_theEventSystem->Shutdown();

	delete g_theGame;
	delete g_theRenderer;
	delete g_theInput;
	delete g_theAudio;
	delete g_window;
	delete g_theEventSystem;
	delete m_devConsoleCamera;

	m_devConsoleCamera = nullptr;
	g_theGame = nullptr;
	g_theInput = nullptr;
	g_theRenderer = nullptr;
	g_theAudio = nullptr;
	g_window = nullptr;
}
/*
	m_audioDict["mainMenuMusic"] = ;
	m_audioDict["gameMusic"] = g_gameConfigBlackboard.GetValue( "gameMusic" );
	m_audioDict["buttonClickSound"] = g_gameConfigBlackboard.GetValue( "buttonClickSound" );
*/

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
	//if (g_theInput->WasKeyJustPressed( KEYCODE_F8 )) { // F8 to restart the game
	//	delete g_theGame;
	//	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Restart Game" );
	//	g_theGame = new Game();
	//	g_theGame->Startup();
	//}
//#endif

	if (g_theInput->WasKeyJustPressed( KEYCODE_ESC ) && g_theGame->GetState() == GameState::ATTRACT) {
		m_isQuitting = true;
	}
}


SoundID App::GetSoundId( std::string const& name ) const
{
	auto iter = m_audioDict.find( name );
	if (iter != m_audioDict.end()) {
		return iter->second;
	}
	return g_theAudio->CreateOrGetSound( name );
}

SoundPlaybackID App::PlaySound( std::string const& name, float intervalTimeSeconds, bool isLooped, float volume, float balance, float speed )
{
	double curTimeSeconds = GetCurrentTimeSeconds();
	if (float( curTimeSeconds - m_lastSoundPlaySecondsByID[name] ) >= intervalTimeSeconds) {
		m_lastSoundPlaySecondsByID[name] = curTimeSeconds;
		return g_theAudio->StartSound( GetSoundId( name ), isLooped, volume, balance, speed );
	}
	return (SoundPlaybackID)(-1);
}

SpriteAnimDefinition* App::GetAnimation( AnimationName name )
{
	return m_animDictionary[(int)name];
}

void App::BeginFrame() {
	Clock::TickSystemClock();
	g_window->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theInput->BeginFrame();
	g_theAudio->BeginFrame();
	g_devConsole->BeginFrame();
	g_theEventSystem->BeginFrame();
	DebugRenderBeginFrame();
}

void App::Update() {
	//float deltaSeconds = Clock::GetSystemClock()->GetDeltaSeconds();

	if (!g_window->IsFocus() || g_devConsole->GetMode() == DevConsoleMode::EMERGE || g_theGame->GetState() == GameState::ATTRACT) {
		g_theInput->SetCursorMode( false, false );
	}
	else {
		g_theInput->SetCursorMode( true, true );
	}

	g_theGame->Update();

	HandleKey();
}

void App::Render() const
{
	g_theGame->Render();

	if (g_devConsole->GetMode() == DevConsoleMode::EMERGE) {
		g_devConsole->Render( m_devConsoleCamera->m_cameraBox );
	}
}

void App::EndFrame() {
	g_window->EndFrame();
	g_theRenderer->EndFrame();
	g_theInput->EndFrame();
	g_theAudio->EndFrame();
	g_devConsole->EndFrame();
	g_theEventSystem->EndFrame();
	DebugRenderEndFrame();
}


void App::SetUpTexture()
{
	g_ASCIIFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/TestUV.png" );
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

