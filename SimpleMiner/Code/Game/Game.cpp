#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Prop.hpp"
#include "Game/Player.hpp"
#include "Game/World.hpp"
#include "Game/Block.hpp"
#include "Game/Chunk.hpp"
#include "Game/GameCommon.hpp"
#include "Game/BlockTemplates.hpp"

World* g_theWorld = nullptr;

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
	delete m_world;
	m_world = nullptr;

	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		delete m_entityArray[i];
		m_entityArray[i] = nullptr;
	}

}

void Game::Startup()
{
	// set up camera
	//m_screenCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ) );
	m_screenCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 0.f, 1.f );
	SetupDefinitions();

	m_world = new World();
	m_world->StartUp();
	g_theWorld = m_world;

	DebugAddWorldArrow( Vec3( 0, 0, 0 ), Vec3( 1, 0, 0 ), 0.05f, -1.f, Rgba8( 255, 0, 0 ), Rgba8( 255, 0, 0 ), DebugRenderMode::USE_DEPTH );
	DebugAddWorldArrow( Vec3( 0, 0, 0 ), Vec3( 0, 1, 0 ), 0.05f, -1.f, Rgba8( 0, 255, 0 ), Rgba8( 0, 255, 0 ), DebugRenderMode::USE_DEPTH );
	DebugAddWorldArrow( Vec3( 0, 0, 0 ), Vec3( 0, 0, 1 ), 0.05f, -1.f, Rgba8( 0, 0, 255 ), Rgba8( 0, 0, 255 ), DebugRenderMode::USE_DEPTH );

}

void Game::Update()
{
	//double begin = GetCurrentTimeSeconds();
	UpdateDebugRender();
	HandleKeys();
	m_world->Update();
	UpdateEntityArrays();
	//double end = GetCurrentTimeSeconds();
	//g_devConsole->AddLine( Rgba8::WHITE, Stringf( "%f", end - begin ) );
}

void Game::Render() const
{
	//double begin = GetCurrentTimeSeconds();
	// Game Camera
	g_theRenderer->ClearScreen( m_world->m_skyColor );

	g_theRenderer->BeginCamera( m_world->GetPlayer()->m_camera );
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		m_entityArray[i]->Render();
	}
	m_world->Render();
	g_theRenderer->EndCamera( m_world->GetPlayer()->m_camera );

	DebugRenderWorld( m_world->GetPlayer()->m_camera );
	// UI Camera
	g_theRenderer->BeginCamera( m_screenCamera );
	m_world->RenderUI();
	g_theRenderer->EndCamera( m_screenCamera );
	DebugRenderScreen( m_screenCamera );
	//double end = GetCurrentTimeSeconds();
	//g_devConsole->AddLine( Rgba8::WHITE, Stringf( "%f", end - begin ) );
}

void Game::HandleKeys() const
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
	/*
	if (g_theInput->WasKeyJustPressed( '1' )) {
		DebugAddWorldLine( m_player->m_position + m_player->GetForwardNormal() * 0.5f, m_player->m_position + m_player->GetForwardNormal() * 20.5f, 0.01f, 10.f, Rgba8( 255, 255, 0 ), Rgba8( 255, 255, 0 ), DebugRenderMode::X_RAY );
	}
	if (g_theInput->IsKeyDown( '2' )) {
		DebugAddWorldWireSphere( Vec3( m_player->m_position.x, m_player->m_position.y, 0.f ), 1.f, 60.f, Rgba8( 150, 75, 0 ), Rgba8( 150, 75, 0 ), false, DebugRenderMode::USE_DEPTH );
	}
	if (g_theInput->WasKeyJustPressed( '3' )) {
		DebugAddWorldWireSphere( m_player->m_position + m_player->GetForwardNormal() * 2.f, 1.f, 5.f, Rgba8( 0, 255, 0 ), Rgba8( 255, 0, 0 ), true, DebugRenderMode::USE_DEPTH );
	}
	if (g_theInput->WasKeyJustPressed( '4' )) {
		Mat44 modelMat = m_player->GetModelMatrix();
		DebugAddWorldArrow( m_player->m_position, m_player->m_position + modelMat.GetIBasis3D(), 0.05f, 20.f, Rgba8( 255, 0, 0 ), Rgba8( 255, 0, 0 ), DebugRenderMode::USE_DEPTH );
		DebugAddWorldArrow( m_player->m_position, m_player->m_position + modelMat.GetJBasis3D(), 0.05f, 20.f, Rgba8( 0, 255, 0 ), Rgba8( 0, 255, 0 ), DebugRenderMode::USE_DEPTH );
		DebugAddWorldArrow( m_player->m_position, m_player->m_position + modelMat.GetKBasis3D(), 0.05f, 20.f, Rgba8( 0, 0, 255 ), Rgba8( 0, 0, 255 ), DebugRenderMode::USE_DEPTH );
	}
	if (g_theInput->WasKeyJustPressed( '5' )) {
		DebugAddWorldBillboardText( Stringf( "Position: %.2f %.2f %.2f Orientation: %.2f %.2f %.2f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z,
			m_player->m_orientation.m_yawDegrees, m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_rollDegrees ), 
			Vec3( m_player->m_position + m_player->GetForwardNormal() * 3.f ), 0.1f, Vec2( 0.5f, 0.5f ), 10.f, Rgba8::WHITE, Rgba8( 255, 0, 0 ) );
	}
	if (g_theInput->WasKeyJustPressed( '6' )) {
		Mat44 mat = m_player->m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
		Vec3 iBasis = mat.GetIBasis3D();
		Vec3 kBasis = mat.GetKBasis3D();
		DebugAddWorldWireCylinder( m_player->m_position + iBasis - kBasis, m_player->m_position + iBasis + kBasis, 0.3f, 10.f, Rgba8::WHITE, Rgba8( 255, 0, 0 ), true, DebugRenderMode::USE_DEPTH );
	}
	if (g_theInput->WasKeyJustPressed( '7' )) {
		DebugAddMessage( Stringf( "Camera Orientation: %.2f %.2f %.2f", m_player->m_orientation.m_yawDegrees, m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_rollDegrees ), 5.f, Rgba8( 255, 0, 0 ), Rgba8( 0, 0, 255 ) );
	}*/
//#endif // DEBUG_MODE
}

void Game::AddEntityToEntityArries( Entity* entityToAdd )
{
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i] == nullptr) {
			m_entityArray[i] = entityToAdd;
		}
	}
	m_entityArray.push_back( entityToAdd );
}

void Game::UpdateEntityArrays()
{
	UpdateEntityArray( m_entityArray );
}

void Game::UpdateEntityArray( EntityList& entityArray )
{
	for (int i = 0; i < (int)entityArray.size(); i++) {
		entityArray[i]->Update();
	}
}

void Game::UpdateDebugRender()
{
	DebugAddScreenText( Stringf( "Time: %.2f FPS: %.2f Scale: %.1f", Clock::GetSystemClock()->GetTotalSeconds(), 1.f / Clock::GetSystemClock()->GetDeltaSeconds(), Clock::GetSystemClock()->GetTimeScale() ), m_screenCamera.m_cameraBox.m_maxs - Vec2( 400.f, 20.f ), 20.f, Vec2( 0.5f, 0.5f ), 0.f, Rgba8::WHITE, Rgba8::WHITE );
	DebugAddMessage( Stringf( "Player Position: %.2f %.2f %.2f", m_world->GetPlayer()->m_position.x, m_world->GetPlayer()->m_position.y, m_world->GetPlayer()->m_position.z ), 0.f, Rgba8( 255, 255, 255 ), Rgba8( 255, 255, 255 ) );
	DebugAddMessage( Stringf( "Chunks: %d/%d Blocks: %d Verts: %d", m_world->m_activeChunks.size(), m_world->m_maxChunks, m_world->m_activeChunks.size() * BLOCK_COUNT_EACH_CHUNK, m_world->GetVertsCount() ), 0.f, Rgba8( 255, 255, 255 ), Rgba8( 255, 255, 255 ) );
	DebugAddMessage( m_world->GetCurDayTimeText(), 0.f, Rgba8( 255, 255, 255 ), Rgba8( 255, 255, 255 ) );
}

void Game::SetupDefinitions()
{
	BlockDefinition::SetUpDefinitions();
	BlockTemplate::SetUpBlockTemplates();
}
