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

	// initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init( g_window->GetWindowHandle() );
	ImGui_ImplDX11_Init( g_theRenderer->m_dx11Renderer->m_device, g_theRenderer->m_dx11Renderer->m_deviceContext );

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
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	delete g_theGame;
	g_theRenderer->Shutdown();
	g_theInput->ShutDown();
	g_theAudio->Shutdown();
	g_window->Shutdown();
	g_devConsole->Shutdown();
	g_theEventSystem->Shutdown();
	g_theJobSystem->ShutDown();

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
#ifdef DEBUG_MODE
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
#endif

	if (g_theInput->WasKeyJustPressed( KEYCODE_ESC )) {
		m_isQuitting = true;
	}
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
	// (Your code process and dispatch Win32 messages)
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//ImGui::ShowDemoWindow(); // Show demo window! :)
}

void App::Render() const
{
	g_theRenderer->ClearScreen( Rgba8( 255, 255, 255, 255 ) );

	g_theGame->Render();


	if (g_devConsole->GetMode() == DevConsoleMode::EMERGE) {
		g_devConsole->Render( m_attractModeCamera->m_cameraBox );
	}

	g_theRenderer->BeginCamera( *m_attractModeCamera );
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, AABB2( Vec2( 0.f, 0.f ), Vec2( 0.f, 0.f ) ), Rgba8::WHITE );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
	g_theRenderer->EndCamera( *m_attractModeCamera );

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
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

