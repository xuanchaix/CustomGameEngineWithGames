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

void App::Startup( char const* commandLine ) {
	SetUpBlackBoard();
	Clock::TickSystemClock();

	EventSystemConfig eConfig;
	g_theEventSystem = new EventSystem( eConfig );
	g_theEventSystem->Startup();

	std::string commandLineStr = std::string( commandLine );
	Strings commandLineSplit;
	int numOfStrs = SplitStringOnDelimiter( commandLineSplit, commandLineStr, ' ' );
	if (numOfStrs == 2) {
		EventArgs args;
		Strings keyValuePair;
		int lengthOfPair = SplitStringOnDelimiter( keyValuePair, commandLineSplit[1], '=' );
		if (lengthOfPair == 2) {
			args.SetValue( keyValuePair[0], keyValuePair[1] );
		}
 		FireEvent( commandLineSplit[0], args );
	}

	WindowConfig wConfig;
	wConfig.m_clientAspect = g_gameConfigBlackboard.GetValue( "windowAspect", 2.f );
	wConfig.m_isFullScreen = g_gameConfigBlackboard.GetValue( "windowFullscreen", false );
	wConfig.m_windowTitle = g_gameConfigBlackboard.GetValue( "windowTitle", "ProtoGame" );
	wConfig.m_windowSize = g_gameConfigBlackboard.GetValue( "windowSize", IntVec2( -1, -1 ) );
	wConfig.m_windowPosition = g_gameConfigBlackboard.GetValue( "windowPosition", IntVec2( -1, -1 ) );
	g_window = new Window( wConfig );
	g_window->StartUp();

	m_attractModeCamera = new Camera();

	RendererConfig rConfig;
	rConfig.m_window = g_window;
	g_theRenderer = new Renderer( rConfig );
	g_theRenderer->StartUp();
	SetUpTexture();
	DebugRenderConfig drConfig;
	drConfig.m_renderer = g_theRenderer;
	drConfig.m_fontName = "RobotoMonoSemiBold128";
	DebugRenderSystemStartup( drConfig );
	
	DevConsoleConfig dConfig;
	dConfig.m_defaultFont = g_ASCIIFont;
	dConfig.m_defaultRenderer = g_theRenderer;
	dConfig.m_camera = m_attractModeCamera;
	dConfig.m_fontAspect = dConfig.m_fontAspect / g_window->GetAspect() * 2.f;
	g_devConsole = new DevConsole( dConfig );
	g_devConsole->Startup();

	InputSystemConfig iConfig;
	//iConfig.m_curWindow = g_window;
	g_theInput = new InputSystem( iConfig );
	g_theInput->StartUp();
	g_theInput->SetCursorMode( false, false );

	AudioSystemConfig aConfig;
	g_theAudio = new AudioSystem( aConfig );
	g_theAudio->Startup();

	SetUpAudio();

	NetSystemConfig nConfig;
	nConfig.m_hostAddressString = g_gameConfigBlackboard.GetValue( "netHostAddress", nConfig.m_hostAddressString );
	nConfig.m_modeString = g_gameConfigBlackboard.GetValue( "netMode", nConfig.m_modeString );
	nConfig.m_recvBufferSize = g_gameConfigBlackboard.GetValue( "netRecvBufferSize", nConfig.m_recvBufferSize );
	nConfig.m_sendBufferSize = g_gameConfigBlackboard.GetValue( "netSendBufferSize", nConfig.m_sendBufferSize );

	g_theNetSystem = new NetSystem( nConfig );
	g_theNetSystem->StartUp();
	//m_theGame = new Game();
	//m_theGame->Startup();

	SubscribeEventCallbackFunction( "quit", App::SetQuitting );
	g_devConsole->AddLine( DevConsole::INFO_MINOR, "Initializing..." );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Successfully Initialized!" );

	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Input Info:" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "~: Development Console" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Type help to get all commands" );

	m_attractModeCamera->SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );
	m_attractModeCamera->m_mode = CameraMode::Orthographic;

	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Start Game!" );
	g_theGame = new Game();
	g_theGame->Startup();
	g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Music/MainMenu.mp3" ) );
}

void App::Shutdown() {
	delete g_theGame;

	g_theNetSystem->ShutDown();
	g_theRenderer->Shutdown();
	g_theInput->ShutDown();
	g_theAudio->Shutdown();
	g_window->Shutdown();
	g_devConsole->Shutdown();
	g_theEventSystem->Shutdown();

	delete g_theNetSystem;
	delete g_theRenderer;
	delete g_theInput;
	delete g_theAudio;
	delete g_window;
	delete g_theEventSystem;

	g_theGame = nullptr;
	g_theNetSystem = nullptr;
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
#ifdef DEBUG_MODE
	if (g_theInput->WasKeyJustPressed( KEYCODE_F1 )) // F1 debug mode
	{
		m_debugMode = !m_debugMode;
	}
	/*if (g_theInput->WasKeyJustPressed(KEYCODE_F8)) { // F8 to restart the game
		delete g_theGame;
		g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Restart Game" );
		g_theGame = new Game();
		g_theGame->Startup();
	}*/
#endif

	/*if ((g_theInput->WasKeyJustPressed(KEYCODE_ESC) || g_theInput->GetController(0).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_BACK)) && !m_attractMode)
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
	}*/

	// ' ' or P enter game
	/*if ((g_theInput->WasKeyJustPressed('P')
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
	}*/

	//if (g_theInput->WasKeyJustPressed( KEYCODE_ESC )) {
	//	m_isQuitting = true;
	//}
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
	g_theNetSystem->BeginFrame();
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
		g_theInput->SetCursorMode( false, false );
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

	if (m_isQuitting2) {
		m_isQuitting = true;
	}
	else if (m_isQuitting3) {
		m_isQuitting2 = true;
	}
	

	if (!m_attractMode) {
		g_theGame->Update();
	}

	HandleKey();
}

void App::Render() const
{
	g_theRenderer->ClearScreen( Rgba8( 0, 0, 0 ) );

	if (!m_attractMode) {
		g_theGame->Render();
	}
	else {
		g_theRenderer->BeginCamera( *m_attractModeCamera );
		RenderAttractMode();
		g_theRenderer->EndCamera( *m_attractModeCamera );
	}

	if (g_devConsole->GetMode() == DevConsoleMode::EMERGE) {
		g_devConsole->Render( m_attractModeCamera->m_cameraBox );
	}
}

void App::EndFrame() {
	g_theNetSystem->EndFrame();
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

}

void App::SetUpAudio()
{
	m_audioDictionary[(int)AudioName::AttractMode] = g_theAudio->CreateOrGetSound( "Data/Music/MainMenu.mp3" );
}

void App::SetUpTexture()
{
	g_ASCIIFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/RobotoMonoSemiBold128" );
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
	g_theApp->m_isQuitting = false;
	g_theApp->m_isQuitting2 = false;
	g_theApp->m_isQuitting3 = true;
	return true;
}

