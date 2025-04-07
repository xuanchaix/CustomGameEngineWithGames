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

	JobSystemConfig jConfig;
	jConfig.m_numOfWorkers = -1;
	g_theJobSystem = new JobSystem( jConfig );
	g_theJobSystem->StartUp();

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

	m_attractModeCamera = new Camera();
	DevConsoleConfig dConfig;
	dConfig.m_defaultFont = g_ASCIIFont;
	dConfig.m_defaultRenderer = g_theRenderer;
	dConfig.m_camera = m_attractModeCamera;
	g_devConsole = new DevConsole( dConfig );
	g_devConsole->Startup();

	InputSystemConfig iConfig;
	g_theInput = new InputSystem( iConfig );
	g_theInput->StartUp();

	AudioSystemConfig aConfig;
	g_theAudio = new AudioSystem( aConfig );
	g_theAudio->Startup();


	SetUpAudio();
	g_theGame = new Game();
	g_theGame->Startup();

	SubscribeEventCallbackFunction( "quit", App::SetQuitting );
	g_devConsole->AddLine( DevConsole::INFO_MINOR, "Initializing..." );

	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Successfully Initialized!" );

	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Input Info:" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "P: Pause O: Run a Frame then Pause T: Time scale = 0.1" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Esc: Exit Game F8: Restart Game" );

	m_attractModeCamera->SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );
	m_attractModeCamera->m_mode = CameraMode::Orthographic;
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
}

void App::Shutdown() {
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
	delete g_theJobSystem;

	g_theGame = nullptr;
	g_theInput = nullptr;
	g_theRenderer = nullptr;
	g_theAudio = nullptr;
	g_window = nullptr;
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
	if (g_theInput->WasKeyJustPressed( KEYCODE_ESC )) {
		m_isQuitting = true;
	}

#ifdef DEBUG_MODE

#endif
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
}

void App::Update() {
	//float deltaSeconds = Clock::GetSystemClock()->GetDeltaSeconds();

	g_theGame->Update();

	HandleKey();
}

void App::Render() const
{
	g_theRenderer->ClearScreen( Rgba8( 255, 255, 255, 255 ) );

	g_theGame->Render();

	if (g_devConsole->GetMode() == DevConsoleMode::EMERGE) {
		g_devConsole->Render( m_attractModeCamera->m_cameraBox );
	}
}

void App::EndFrame() {
	g_window->EndFrame();
	g_theRenderer->EndFrame();
	g_theInput->EndFrame();
	g_theAudio->EndFrame();
	g_devConsole->EndFrame();
	g_theEventSystem->EndFrame();
	g_theJobSystem->EndFrame();
}

void App::SetUpAudio()
{
	m_audioDictionary[(int)AudioName::AttractMode] = g_theAudio->CreateOrGetSound( "Data/Music/MainMenu.mp3" );
}

void App::SetUpTexture()
{
	g_ASCIIFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
}

void App::SetUpBlackBoard()
{
	/*XmlDocument gameConfig;
	XmlError errorCode = gameConfig.LoadFile( "Data/GameConfig.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document GameConfig.xml error" );
	XmlElement* root = gameConfig.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "GameConfig" ), "Syntax Error! Name of the root of GameConfig.xml should be \"GameConfig\" " );

	g_gameConfigBlackboard.PopulateFromXmlElementAttributes( *root );*/
}

bool App::SetQuitting( EventArgs& args )
{
	UNUSED( args );
	g_theApp->m_isQuitting = true;
	return true;
}

