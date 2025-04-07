#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Entity.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/UISystem.hpp"
#include "Game/Resource.hpp"
#include "Game/Projectile.hpp"
#include "Game/Enemy.hpp"

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
	delete m_map;
	delete g_uiSystem;
	g_uiSystem = nullptr;
}

void Game::Startup()
{
	ProductDefinition::SetUpProductDefinitions();
	ProjectileDefinition::SetUpProjectileDefinitions();
	EnemyDefinition::SetUpEnemyDefinitions();
	// set up camera
	m_screenCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );
	m_screenCamera.m_mode = CameraMode::Orthographic;
	m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 1.f, -1.f );
	m_worldCamera.m_mode = CameraMode::Orthographic;

	g_uiSystem = new SFUISystem();

	m_map = new Map();
	m_map->StartUp();
	g_theGame->m_worldCamera.SetCenter( Vec2( (float)TILE_MAP_SIZE_X * 0.5f, (float)TILE_MAP_SIZE_Y * 0.5f ) );
}

void Game::Update()
{
	HandleKeys();
	m_map->Update();
}

void Game::Render() const
{
	float deltaSeconds = m_gameClock->GetDeltaSeconds();
	// Game Camera
	g_theRenderer->BeginCamera( m_worldCamera );
	m_map->Render();
	g_theRenderer->EndCamera( m_worldCamera );
	// UI Camera
	g_theRenderer->BeginCamera( m_screenCamera );
	std::vector<Vertex_PCU> textVerts;
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 1500.f, 750.f ), Vec2( 1600.f, 800.f ) ), 50.f, Stringf( "FPS%.2f", 1.f / deltaSeconds ) );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
	g_theRenderer->DrawVertexArray( textVerts );

	m_map->RenderUI();
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

	if (g_theInput->IsKeyDown( 'Y' )) // T key slow the game
	{
		m_gameClock->SetTimeScale( 4.f );
	}

	if (g_theInput->WasKeyJustPressed( 0x50 )) // P key pause the game; handle the pause problem
	{
		m_gameClock->TogglePause();
	}
#endif // DEBUG_MODE

	float deltaSeconds = m_gameClock->GetDeltaSeconds();
	Vec2 translation = Vec2( 0.f, 0.f );
	if (g_theInput->IsKeyDown( 'W' )) {
		translation += Vec2( 0.f, deltaSeconds * 5.f );
	}
	if (g_theInput->IsKeyDown( 'A' )) {
		translation += Vec2( -deltaSeconds * 5.f, 0.f );
	}
	if (g_theInput->IsKeyDown( 'S' )) {
		translation += Vec2( 0.f, -deltaSeconds * 5.f );
	}
	if (g_theInput->IsKeyDown( 'D' )) {
		translation += Vec2( deltaSeconds * 5.f, 0.f );
	}
	if (g_theInput->IsKeyDown( 'B' )) {
		m_worldCamera.SetCenter( m_map->GetMapCenter() );
	}

	if (g_theInput->WasKeyJustPressed( KEYCODE_F4 )) {
		m_wholeMapMode = !m_wholeMapMode;
		Vec2 center = m_worldCamera.GetCenter();
		if (m_wholeMapMode) {
// 			if (TILE_MAP_SIZE_Y < TILE_MAP_SIZE_X / 2) {
// 				m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( TILE_MAP_SIZE_Y * 2, TILE_MAP_SIZE_Y ), 1.f, -1.f );
// 			}
// 			else {
				m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( TILE_MAP_SIZE_X, TILE_MAP_SIZE_X / 2 ), 1.f, -1.f );
				//}
				m_worldCamera.SetCenter( center );
		}
		else {
			if (m_smallMapMode) {
				m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X * 2, WORLD_SIZE_Y * 2 ), 1.f, -1.f );
				m_worldCamera.SetCenter( center );
			}
			else {
				m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 1.f, -1.f );
				m_worldCamera.SetCenter( center );
			}
		}
	}

	if (g_theInput->WasKeyJustPressed( KEYCODE_F3 ) && !m_wholeMapMode) {
		m_smallMapMode = !m_smallMapMode;
		Vec2 center = m_worldCamera.GetCenter();
		if (m_smallMapMode) {
			m_wholeMapMode = false;
			m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X * 2, WORLD_SIZE_Y * 2 ), 1.f, -1.f );
			m_worldCamera.SetCenter( center );
		}
		else {
			m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 1.f, -1.f );
			m_worldCamera.SetCenter( center );
		}
	}
	if (m_wholeMapMode) {
		m_worldCamera.Translate2D( translation * TILE_MAP_SIZE_X / WORLD_SIZE_X );
	}
	else if (m_smallMapMode) {
		m_worldCamera.Translate2D( translation * 2.f );
	}
	else {
		m_worldCamera.Translate2D( translation );
	}
	
	Vec2 center = m_worldCamera.GetCenter();
	float halfX = (m_worldCamera.m_cameraBox.m_maxs.x - m_worldCamera.m_cameraBox.m_mins.x) * 0.5f;
	float halfY = (m_worldCamera.m_cameraBox.m_maxs.y - m_worldCamera.m_cameraBox.m_mins.y) * 0.5f;
	center.x = GetClamped( center.x, halfX, TILE_MAP_SIZE_X - halfX );
	center.y = GetClamped( center.y, halfY, TILE_MAP_SIZE_Y - halfY );
	m_worldCamera.SetCenter( center );


}
