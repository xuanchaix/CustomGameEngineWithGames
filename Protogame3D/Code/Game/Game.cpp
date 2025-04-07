#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Prop.hpp"
#include "Game/Player.hpp"

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

	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		delete m_entityArray[i];
		m_entityArray[i] = nullptr;
	}

}

void Game::Startup()
{
	m_timer = new Timer( 1.f, m_gameClock );
	m_timer->Start();
	// set up camera
	//m_screenCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ) );
	m_screenCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 0.f, 1.f );
	m_screenCamera.m_mode = CameraMode::Orthographic;

	SetupTestProps();
	SetupGrids();
	//m_screenCamera;
}

void Game::Update()
{
	UpdateDebugRender();
	HandleKeys();
	UpdateEntityArrays();
	//Entity* cube1 = m_entityArray[0];
	Entity* cube2 = m_entityArray[1];
	static bool s_isGoingBlack = true;
	if (m_timer->DecrementPeriodIfElapsed()) {
		s_isGoingBlack = !s_isGoingBlack;
	}
	if (s_isGoingBlack) {
		cube2->m_color = Rgba8::Interpolate( Rgba8::WHITE, Rgba8( 0, 0, 0 ), m_timer->GetElapsedFraction() );
	}
	else {
		cube2->m_color = Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8::WHITE, m_timer->GetElapsedFraction() );
	}
}

void Game::Render() const
{
	// Game Camera
	g_theRenderer->BeginCamera( m_player->m_camera );
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		m_entityArray[i]->Render();
	}

	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( m_gridVerts );
	g_theRenderer->EndCamera( m_player->m_camera );

	DebugRenderWorld( m_player->m_camera );
	// UI Camera
	g_theRenderer->BeginCamera( m_screenCamera );

	g_theRenderer->EndCamera( m_screenCamera );
	DebugRenderScreen( m_screenCamera );
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
	}
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
	DebugAddScreenText( Stringf( "Time: %.2f FPS: %.2f Scale: %.1f", Clock::GetSystemClock()->GetTotalSeconds(), 1.f / Clock::GetSystemClock()->GetDeltaSeconds(), Clock::GetSystemClock()->GetTimeScale() ), m_screenCamera.m_cameraBox.m_maxs - Vec2( 220.f, 20.f ), 20.f, Vec2( 0.5f, 0.5f ), 0.f, Rgba8::WHITE, Rgba8::WHITE );
	DebugAddMessage( Stringf( "Player Position: %.2f %.2f %.2f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z ), 0.f, Rgba8( 255, 255, 255 ), Rgba8( 255, 255, 255 ) );
}

void Game::SetupTestProps()
{
	Prop* cube1 = new Prop( this );
	cube1->m_position = Vec3( 2, 2, 0 );
	cube1->m_angularVelocity = EulerAngles( 0.f, 30.f, 30.f );
	AddVertsForQuad3D( cube1->m_vertexes, Vec3( -0.5f, -0.5f, 0.5f ), Vec3( 0.5f, -0.5f, 0.5f ), Vec3( 0.5f, 0.5f, 0.5f ), Vec3( -0.5f, 0.5f, 0.5f ), Rgba8( 0, 0, 255 ) );
	AddVertsForQuad3D( cube1->m_vertexes, Vec3( -0.5f, 0.5f, -0.5f ), Vec3( 0.5f, 0.5f, -0.5f ), Vec3( 0.5f, -0.5f, -0.5f ), Vec3( -0.5f, -0.5f, -0.5f ), Rgba8( 255, 255, 0 ) );
	AddVertsForQuad3D( cube1->m_vertexes, Vec3( 0.5f, -0.5f, -0.5f ), Vec3( 0.5f, 0.5f, -0.5f ), Vec3( 0.5f, 0.5f, 0.5f ), Vec3( 0.5f, -0.5f, 0.5f ), Rgba8( 255, 0, 0 ) );
	AddVertsForQuad3D( cube1->m_vertexes, Vec3( -0.5f, 0.5f, -0.5f ), Vec3( -0.5f, -0.5f, -0.5f ), Vec3( -0.5f, -0.5f, 0.5f ), Vec3( -0.5f, 0.5f, 0.5f ), Rgba8( 0, 255, 255 ) );
	AddVertsForQuad3D( cube1->m_vertexes, Vec3( 0.5f, 0.5f, -0.5f ), Vec3( -0.5f, 0.5f, -0.5f ), Vec3( -0.5f, 0.5f, 0.5f ), Vec3( 0.5f, 0.5f, 0.5f ), Rgba8( 0, 255, 0 ) );
	AddVertsForQuad3D( cube1->m_vertexes, Vec3( -0.5f, -0.5f, -0.5f ), Vec3( 0.5f, -0.5f, -0.5f ), Vec3( 0.5f, -0.5f, 0.5f ), Vec3( -0.5f, -0.5f, 0.5f ), Rgba8( 255, 0, 255 ) );
	AddEntityToEntityArries( cube1 );

	Prop* cube2 = new Prop( this );
	cube2->m_position = Vec3( -2, -2, 0 );
	AddVertsForQuad3D( cube2->m_vertexes, Vec3( -0.5f, -0.5f, 0.5f ), Vec3( 0.5f, -0.5f, 0.5f ), Vec3( 0.5f, 0.5f, 0.5f ), Vec3( -0.5f, 0.5f, 0.5f ), Rgba8( 0, 0, 255 ) );
	AddVertsForQuad3D( cube2->m_vertexes, Vec3( -0.5f, 0.5f, -0.5f ), Vec3( 0.5f, 0.5f, -0.5f ), Vec3( 0.5f, -0.5f, -0.5f ), Vec3( -0.5f, -0.5f, -0.5f ), Rgba8( 255, 255, 0 ) );
	AddVertsForQuad3D( cube2->m_vertexes, Vec3( 0.5f, -0.5f, -0.5f ), Vec3( 0.5f, 0.5f, -0.5f ), Vec3( 0.5f, 0.5f, 0.5f ), Vec3( 0.5f, -0.5f, 0.5f ), Rgba8( 255, 0, 0 ) );
	AddVertsForQuad3D( cube2->m_vertexes, Vec3( -0.5f, 0.5f, -0.5f ), Vec3( -0.5f, -0.5f, -0.5f ), Vec3( -0.5f, -0.5f, 0.5f ), Vec3( -0.5f, 0.5f, 0.5f ), Rgba8( 0, 255, 255 ) );
	AddVertsForQuad3D( cube2->m_vertexes, Vec3( 0.5f, 0.5f, -0.5f ), Vec3( -0.5f, 0.5f, -0.5f ), Vec3( -0.5f, 0.5f, 0.5f ), Vec3( 0.5f, 0.5f, 0.5f ), Rgba8( 0, 255, 0 ) );
	AddVertsForQuad3D( cube2->m_vertexes, Vec3( -0.5f, -0.5f, -0.5f ), Vec3( 0.5f, -0.5f, -0.5f ), Vec3( 0.5f, -0.5f, 0.5f ), Vec3( -0.5f, -0.5f, 0.5f ), Rgba8( 255, 0, 255 ) );
	AddEntityToEntityArries( cube2 );

	Prop* sphere1 = new Prop( this );
	sphere1->m_position = Vec3( 10.f, -5.f, 1.f );
	sphere1->m_angularVelocity = EulerAngles( 45.f, 0.f, 0.f );
	AddVertsForSphere3D( sphere1->m_vertexes, Vec3( 0.f, 0.f, 0.f ), 1.f );
	sphere1->m_texture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/TestUV.png" );
	AddEntityToEntityArries( sphere1 );

	Prop* sphere2 = new Prop( this );
	sphere2->m_position = Vec3( -10.f, 5.f, 2.f );
	sphere2->m_angularVelocity = EulerAngles( -45.f, 0.f, 0.f );
	AddVertsForAABB3D( sphere2->m_vertexes, sphere2->m_indexes, AABB3( Vec3( -1.f, -1.f, -1.f ), Vec3( 1.f, 1.f, 1.f ) ) );
	sphere2->m_texture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/TestUV.png" );
	AddEntityToEntityArries( sphere2 );

	m_player = new Player( this );
	AddEntityToEntityArries( m_player );
	m_player->m_camera.SetRenderBasis( Vec3( 0.f, 0.f, 1.f ), Vec3( -1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ) );
	m_player->m_camera.SetOrthoView( Vec2( -1.f, -1.f ), Vec2( 1.f, 1.f ), 0.f, 1.f );
	m_player->m_camera.SetPerspectiveView( g_window->GetAspect(), 60.f, 0.1f, 100.f );
	m_player->m_camera.m_mode = CameraMode::Perspective;
	m_player->m_position = Vec3( 0.f, -10.f, 10.f );
	m_player->m_orientation.m_yawDegrees = 90.f;
	m_player->m_orientation.m_pitchDegrees = 45.f;

	//Prop* test1 = new Prop( this );
	//AddVertsForCylinder3D( test1->m_vertexes, Vec3( -2, -2, -2 ), Vec3( 2, 2, 2 ), 1.f, Rgba8::WHITE, AABB2::IDENTITY, 16 );
	//AddVertsForCone3D( test1->m_vertexes, Vec3( -2, -2, -2 ), Vec3( 2, 2, 2 ), 1.f, Rgba8::WHITE, AABB2::IDENTITY, 16 );
	//test1->m_texture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/TestUV.png" );
	//AddEntityToEntityArries( test1 );
}

void Game::SetupGrids( int minX /*= -50*/, int maxX /*= 50*/, int minY /*= -50*/, int maxY /*= 50 */ )
{
	DebugAddWorldArrow( Vec3( 0, 0, 0 ), Vec3( 1, 0, 0 ), 0.1f, -1.f, Rgba8( 255, 0, 0 ), Rgba8( 255, 0, 0 ), DebugRenderMode::USE_DEPTH );
	DebugAddWorldArrow( Vec3( 0, 0, 0 ), Vec3( 0, 1, 0 ), 0.1f, -1.f, Rgba8( 0, 255, 0 ), Rgba8( 0, 255, 0 ), DebugRenderMode::USE_DEPTH );
	DebugAddWorldArrow( Vec3( 0, 0, 0 ), Vec3( 0, 0, 1 ), 0.1f, -1.f, Rgba8( 0, 0, 255 ), Rgba8( 0, 0, 255 ), DebugRenderMode::USE_DEPTH );

	Mat44 xMat = Mat44::CreateZRotationDegrees( -90.f );
	xMat.SetTranslation3D( Vec3( 1.f, 0.f, 0.4f ) );
	Mat44 yMat = Mat44::CreateTranslation3D( Vec3( 0.f, 1.f, 0.4f ) );
	Mat44 zMat = Mat44::CreateXRotationDegrees( 90.f );
	zMat.SetTranslation3D( Vec3( 0.f, -0.4f, 1.f ) );
	DebugAddWorldText( "x-forward", xMat, 0.3f, Vec2( 0.5f, 0.5f ), -1.f, Rgba8( 255, 0, 0 ), Rgba8( 255, 0, 0 ), DebugRenderMode::USE_DEPTH );
	DebugAddWorldText( "y-left", yMat, 0.3f, Vec2( 0.5f, 0.5f ), -1.f, Rgba8( 0, 255, 0 ), Rgba8( 0, 255, 0 ), DebugRenderMode::USE_DEPTH );
	DebugAddWorldText( "z-up", zMat, 0.3f, Vec2( 0.5f, 0.5f ), -1.f, Rgba8( 0, 0, 255 ), Rgba8( 0, 0, 255 ), DebugRenderMode::USE_DEPTH );

	m_gridVerts.reserve( 10000 );
	for (int x = minX; x <= maxX; x++) {
		if (x == 0) {
			AddVertsForAABB3D( m_gridVerts, AABB3( Vec3( x - 0.05f, (float)minY, -0.05f ), Vec3( x + 0.05f, (float)maxY, 0.05f ) ), Rgba8( 0, 255, 0 ) );
		}
		else if (x % 5 == 0) {
			AddVertsForAABB3D( m_gridVerts, AABB3( Vec3( x - 0.03f, (float)minY, -0.03f ), Vec3( x + 0.03f, (float)maxY, 0.03f ) ), Rgba8( 0, 255, 0, 200 ) );
		}
		else {
			AddVertsForAABB3D( m_gridVerts, AABB3( Vec3( x - 0.02f, (float)minY, -0.02f ), Vec3( x + 0.02f, (float)maxY, 0.02f ) ), Rgba8( 192, 192, 192, 200 ) );
		}
	}
	for (int y = minY; y <= maxY; y++) {
		if (y == 0) {
			AddVertsForAABB3D( m_gridVerts, AABB3( Vec3( (float)minX, y - 0.05f, -0.05f ), Vec3( (float)maxX, y + 0.05f, 0.05f ) ), Rgba8( 255, 0, 0 ) );
		}
		else if (y % 5 == 0) {
			AddVertsForAABB3D( m_gridVerts, AABB3( Vec3( (float)minX, y - 0.03f, -0.03f ), Vec3( (float)maxX, y + 0.03f, 0.03f ) ), Rgba8( 255, 0, 0, 200 ) );
		}
		else {
			AddVertsForAABB3D( m_gridVerts, AABB3( Vec3( (float)minX, y - 0.02f, -0.02f ), Vec3( (float)maxX, y + 0.02f, 0.02f ) ), Rgba8( 192, 192, 192, 200 ) );
		}
	}
}
