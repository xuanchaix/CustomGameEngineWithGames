#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Entity.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"

class DummyJob :public Job {
public:
	DummyJob( int sleepMilliseconds ) { m_sleepMilliseconds = sleepMilliseconds; };
	virtual ~DummyJob() {};
	virtual void Execute() override;

	int m_sleepMilliseconds = 0;
};

void DummyJob::Execute()
{
	std::this_thread::sleep_for( std::chrono::milliseconds( m_sleepMilliseconds ) );
}

Game::Game()
{
	// load random number generator
	m_randNumGen = new RandomNumberGenerator();
	m_gameClock = new Clock();
}

Game::~Game()
{
	// delete all pointers managed by Game
	delete m_randNumGen;
	m_randNumGen = nullptr;

}

void Game::Startup()
{
	// set up camera
	m_screenCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );
	m_screenCamera.m_mode = CameraMode::Orthographic;
	m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 1.f, -1.f );
	m_worldCamera.m_mode = CameraMode::Orthographic;

	m_map = new Map();
	m_map->Startup();
}

void Game::Update()
{
	HandleKeys();
	m_map->Update();
}

void Game::Render() const
{
	// Game Camera
	g_theRenderer->BeginCamera( m_worldCamera );
	m_map->Render();
	g_theRenderer->EndCamera( m_worldCamera );
	// UI Camera
	g_theRenderer->BeginCamera( m_screenCamera );

	g_theRenderer->EndCamera( m_screenCamera );
}

void Game::HandleKeys()
{

#ifdef DEBUG_MODE
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
#endif // DEBUG_MODE
}
