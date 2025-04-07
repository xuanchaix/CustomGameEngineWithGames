#include "Game/App.hpp"
#include "Game/MP1A5.hpp"
#include "Game/MP1A6.hpp"
#include "Game/MP2A3.hpp"
#include "Game/MP2A4.hpp"
#include "Game/MP2A5.hpp"
#include "Game/MP2A6.hpp"

// follow the instructions on the manual
// All global variables Created and owned by the App
App* g_theApp = nullptr;
Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
Window* g_window = nullptr;
BitmapFont* g_ASCIIFont = nullptr;

App::App() {
	m_curVisualTest = nullptr;
	m_timeEnd = 0;
	m_timeStart = 0;
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
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
	DebugRenderConfig dConfig;
	dConfig.m_renderer = g_theRenderer;
	dConfig.m_fontName = "SquirrelFixedFont";
	DebugRenderSystemStartup( dConfig );

	InputSystemConfig iConfig;
	g_theInput = new InputSystem( iConfig );
	g_theInput->StartUp();

	SubscribeEventCallbackFunction( "quit", App::SetQuitting );

	//g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	m_timeStart = GetCurrentTimeSeconds();
	StartNewTest();
	m_curVisualTest->StartUp();
}

void App::Shutdown() {
	delete m_curVisualTest;
	m_curVisualTest = nullptr;

	g_theRenderer->Shutdown();
	g_theInput->ShutDown();
	g_window->Shutdown();
	g_theEventSystem->Shutdown();

	delete g_theRenderer;
	delete g_theInput;
	delete g_window;

	g_theInput = nullptr;
	g_theRenderer = nullptr;
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
	m_timeEnd = GetCurrentTimeSeconds();
	float deltaTime = (float)(m_timeEnd - m_timeStart);
	deltaTime = GetClamped( deltaTime, 0.f, 0.1f );
	m_timeStart = m_timeEnd;
	Update( deltaTime );
	Render();
	EndFrame();
}

/// <summary>
/// Handle key press in app
/// </summary>
/// <returns></returns>
void App::HandleKey() {
#ifdef DEBUG_MODE
	if (g_theInput->WasKeyJustPressed( 0x4F )) {// O key run a single frame and pauses
		m_isPaused = false;
		m_pauseAfterUpdate = true;
	}
#endif
	if (g_theInput->WasKeyJustPressed( KEYCODE_ESC )) // Escape key quit the game
	{
		m_isQuitting = true;
	}
#ifdef DEBUG_MODE
	if (g_theInput->WasKeyJustPressed( 0x54 )) // T key slow the game
	{
		m_isSlowMo = true;
	}
#endif

#ifdef DEBUG_MODE
	if (g_theInput->WasKeyJustPressed( 0x50 )) // P key pause the game; handle the pause problem
	{
		m_isPaused = !m_isPaused;
	}
	if (g_theInput->WasKeyJustPressed( KEYCODE_F1 )) // F1 debug mode
	{
		m_debugMode = !m_debugMode;
	}
	if (g_theInput->WasKeyJustReleased( 0x54 )) { // T key slow the game
		m_isSlowMo = false;
	}
#endif
	if (g_theInput->WasKeyJustPressed( KEYCODE_F8 )) {
		m_curVisualTest->RandomizeTest();
	}
	if (g_theInput->WasKeyJustPressed( KEYCODE_F6 )) {
		GoToPreviousTest();
		m_curVisualTest->StartUp();
	}
	if (g_theInput->WasKeyJustPressed( KEYCODE_F7 )) {
		GoToNextTest();
		m_curVisualTest->StartUp();
	}
}

void App::GoToNextTest()
{
	if (m_curVisualTest) {
		delete m_curVisualTest;
	}
	m_curTestName = (MathVisualTestName)(((int)m_curTestName + 1) % (int)MathVisualTestName::NUM);
	StartNewTest();
}

void App::GoToPreviousTest()
{
	if (m_curVisualTest) {
		delete m_curVisualTest;
	}
	m_curTestName = (MathVisualTestName)(((int)m_curTestName - 1 + (int)MathVisualTestName::NUM) % (int)MathVisualTestName::NUM);
	StartNewTest();
}

void App::StartNewTest()
{
	switch (m_curTestName)
	{
	case MathVisualTestName::POINT_VS_SHAPES:
		m_curVisualTest = new MP1A5();
		break;
	case MathVisualTestName::RAY_CAST_VS_DISCS:
		m_curVisualTest = new MP1A6();
		break;
	case MathVisualTestName::RAY_CAST_VS_LINE_SEGS:
		m_curVisualTest = new RayVsLineSeg2DTest();
		break;
	case MathVisualTestName::RAY_CAST_VS_AABB2S:
		m_curVisualTest = new RayVsAABB2DTest();
		break;
	case MathVisualTestName::RAY_CAST_VS_3D:
		m_curVisualTest = new RayVs3DTest();
		break;
	case MathVisualTestName::CURVE_2D:
		m_curVisualTest = new CurveTest2D();
		break; 
	case MathVisualTestName::Pachinko_Machine_2D:
		m_curVisualTest = new PachinkoMachine2DTest();
		break;
	case MathVisualTestName::NUM:
		m_curVisualTest = nullptr;
		break;
	default:
		m_curVisualTest = nullptr;
		break;
	}
}

void App::BeginFrame() {
	Clock::TickSystemClock();
	DebugRenderBeginFrame();
	g_window->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theInput->BeginFrame();
	g_theEventSystem->BeginFrame();
}

void App::Update( float deltaSeconds ) {
	if (m_isSlowMo) {
		deltaSeconds = deltaSeconds / 10;
	}
	if (m_isPaused) {
		deltaSeconds = 0;
	}
	m_curVisualTest->Update( deltaSeconds );
	if (m_pauseAfterUpdate) {
		m_pauseAfterUpdate = false;
		m_isPaused = true;
	}

	HandleKey();
}

void App::Render() const 
{
	// Clear all screen (back buffer) pixels to medium-blue
	g_theRenderer->ClearScreen( Rgba8( 0, 0, 0, 255 ) );

	m_curVisualTest->Render();
}

void App::EndFrame() {
	DebugRenderEndFrame();
	g_window->EndFrame();
	g_theRenderer->EndFrame();
	g_theInput->EndFrame();
	g_theEventSystem->EndFrame();
}

bool App::SetQuitting( EventArgs& args )
{
	UNUSED( args );
	g_theApp->m_isQuitting = true;
	return true;
}