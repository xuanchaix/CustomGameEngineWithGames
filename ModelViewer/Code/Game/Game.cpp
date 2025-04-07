

#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ObjLoader.hpp"
#include "Game/Prop.hpp"
#include "Game/Model.hpp"
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

	ExitScene( m_curScene );

	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		delete m_entityArray[i];
		m_entityArray[i] = nullptr;
	}

	delete m_model;
	delete m_model0;
	delete m_model1;
	delete m_model2;
	delete m_model3;
	delete m_model4;
	delete m_prop0;
	delete m_prop1;
	delete m_prop2;
	delete m_prop3;
	delete m_hadrianTank;
	delete m_tutorialBox;
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

	m_dlc.m_sunIntensity = 1.f;
	m_dlc.m_ambientIntensity = 1.f - m_dlc.m_sunIntensity;
	m_sunOrientation.m_yawDegrees = 120.f;
	m_sunOrientation.m_pitchDegrees = 45.f;
	m_dlc.m_sunDirection = m_sunOrientation.GetIFwd();

	//m_model = new Model( this, "Data/Models/Cube_Textured.xml" );
	//AddEntityToEntityArries( m_model );
	//ObjLoader::Load( "Data/Models/Cube.obj", vertexes, indexes, hasNormals, hasUVs, Mat44() );
}

void Game::Update()
{
	UpdateDebugRender();
	HandleKeys();
	UpdateEntityArrays();
	m_dlc.m_ambientIntensity = 1.f - m_dlc.m_sunIntensity;
	m_dlc.m_sunDirection = m_sunOrientation.GetIFwd();
	m_dlc.m_worldEyePosition = m_player->m_position;
}

void Game::Render() const
{
	g_theRenderer->SetDirectionalLightConstants( m_dlc );
	// Game Camera
	g_theRenderer->BeginCamera( m_player->m_camera );
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i]) {
			m_entityArray[i]->Render();
		}
	}

	g_theRenderer->RenderEmissive();

	if (m_debugRender) {
		for (int i = 0; i < (int)m_entityArray.size(); i++) {
			m_entityArray[i]->DebugRender();
		}
	}

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( m_gridVerts );
	g_theRenderer->EndCamera( m_player->m_camera );

	std::vector<Vertex_PCU> sunLightArrow;
	AddVertsForArrow3D( sunLightArrow, Vec3( 0.f, 0.f, 2.5f ), Vec3( 0.f, 0.f, 2.5f ) + m_dlc.m_sunDirection * 1.f, 0.2f, 0.35f, Rgba8( 255, 255, 0 ) );

	g_theRenderer->SetRasterizerMode( RasterizerMode::WIREFRAME_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( sunLightArrow );
	g_theRenderer->EndCamera( m_player->m_camera );


	DebugRenderWorld( m_player->m_camera );
	// UI Camera
	g_theRenderer->BeginCamera( m_screenCamera );
	RenderUI();
	g_theRenderer->EndCamera( m_screenCamera );
	DebugRenderScreen( m_screenCamera );
}

void Game::HandleKeys()
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

	float deltaSeconds = m_gameClock->GetDeltaSeconds();

// 	if (g_theInput->WasKeyJustPressed( '1' )) {
// 		DebugAddWorldLine( m_player->m_position + m_player->GetForwardNormal() * 0.5f, m_player->m_position + m_player->GetForwardNormal() * 20.5f, 0.01f, 10.f, Rgba8( 255, 255, 0 ), Rgba8( 255, 255, 0 ), DebugRenderMode::X_RAY );
// 	}
// 	if (g_theInput->IsKeyDown( '2' )) {
// 		DebugAddWorldWireSphere( Vec3( m_player->m_position.x, m_player->m_position.y, 0.f ), 1.f, 60.f, Rgba8( 150, 75, 0 ), Rgba8( 150, 75, 0 ), false, DebugRenderMode::USE_DEPTH );
// 	}
// 	if (g_theInput->WasKeyJustPressed( '3' )) {
// 		DebugAddWorldWireSphere( m_player->m_position + m_player->GetForwardNormal() * 2.f, 1.f, 5.f, Rgba8( 0, 255, 0 ), Rgba8( 255, 0, 0 ), true, DebugRenderMode::USE_DEPTH );
// 	}
// 	if (g_theInput->WasKeyJustPressed( '4' )) {
// 		Mat44 modelMat = m_player->GetModelMatrix();
// 		DebugAddWorldArrow( m_player->m_position, m_player->m_position + modelMat.GetIBasis3D(), 0.05f, 20.f, Rgba8( 255, 0, 0 ), Rgba8( 255, 0, 0 ), DebugRenderMode::USE_DEPTH );
// 		DebugAddWorldArrow( m_player->m_position, m_player->m_position + modelMat.GetJBasis3D(), 0.05f, 20.f, Rgba8( 0, 255, 0 ), Rgba8( 0, 255, 0 ), DebugRenderMode::USE_DEPTH );
// 		DebugAddWorldArrow( m_player->m_position, m_player->m_position + modelMat.GetKBasis3D(), 0.05f, 20.f, Rgba8( 0, 0, 255 ), Rgba8( 0, 0, 255 ), DebugRenderMode::USE_DEPTH );
// 	}
// 	if (g_theInput->WasKeyJustPressed( '5' )) {
// 		DebugAddWorldBillboardText( Stringf( "Position: %.2f %.2f %.2f Orientation: %.2f %.2f %.2f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z,
// 			m_player->m_orientation.m_yawDegrees, m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_rollDegrees ), 
// 			Vec3( m_player->m_position + m_player->GetForwardNormal() * 3.f ), 0.1f, Vec2( 0.5f, 0.5f ), 10.f, Rgba8::WHITE, Rgba8( 255, 0, 0 ) );
// 	}
// 	if (g_theInput->WasKeyJustPressed( '6' )) {
// 		Mat44 mat = m_player->m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
// 		Vec3 iBasis = mat.GetIBasis3D();
// 		Vec3 kBasis = mat.GetKBasis3D();
// 		DebugAddWorldWireCylinder( m_player->m_position + iBasis - kBasis, m_player->m_position + iBasis + kBasis, 0.3f, 10.f, Rgba8::WHITE, Rgba8( 255, 0, 0 ), true, DebugRenderMode::USE_DEPTH );
// 	}
// 	if (g_theInput->WasKeyJustPressed( '7' )) {
// 		DebugAddMessage( Stringf( "Camera Orientation: %.2f %.2f %.2f", m_player->m_orientation.m_yawDegrees, m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_rollDegrees ), 5.f, Rgba8( 255, 0, 0 ), Rgba8( 0, 0, 255 ) );
// 	}
	if (g_theInput->WasKeyJustPressed( 0xbc )) {
		m_dlc.m_sunIntensity -= 0.1f;
		m_dlc.m_sunIntensity = GetClamped( m_dlc.m_sunIntensity, 0.f, 1.f );
		m_dlc.m_ambientIntensity = 1.f - m_dlc.m_sunIntensity;
	}
	if (g_theInput->WasKeyJustPressed( 0xbe )) {
		m_dlc.m_sunIntensity += 0.1f;
		m_dlc.m_sunIntensity = GetClamped( m_dlc.m_sunIntensity, 0.f, 1.f );
		m_dlc.m_ambientIntensity = 1.f - m_dlc.m_sunIntensity;
	}
	if (g_theInput->WasKeyJustPressed( 0xDB )) {
		ExitScene( m_curScene );
		m_curScene = (m_curScene + 4 - 1) % 4;
		GoToScene( m_curScene );
	}
	if (g_theInput->WasKeyJustPressed( 0xDD )) {
		ExitScene( m_curScene );
		m_curScene = (m_curScene + 1) % 4;
		GoToScene( m_curScene );
	}
	if (g_theInput->IsKeyDown( KEYCODE_DOWNARROW )) {
		m_sunOrientation.m_pitchDegrees -= 45.f * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( KEYCODE_UPARROW )) {
		m_sunOrientation.m_pitchDegrees += 45.f * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( KEYCODE_LEFTARROW )) {
		m_sunOrientation.m_yawDegrees -= 45.f * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( KEYCODE_RIGHTARROW )) {
		m_sunOrientation.m_yawDegrees += 45.f * deltaSeconds;
	}

	if (g_theInput->WasKeyJustPressed( '1' )) {
		m_dlc.m_lightingDebug.m_renderAmbient = 1 - m_dlc.m_lightingDebug.m_renderAmbient;
	}
	if (g_theInput->WasKeyJustPressed( '2' )) {
		m_dlc.m_lightingDebug.m_renderDiffuse = 1 - m_dlc.m_lightingDebug.m_renderDiffuse;
	}
	if (g_theInput->WasKeyJustPressed( '3' )) {
		m_dlc.m_lightingDebug.m_renderSpecular = 1 - m_dlc.m_lightingDebug.m_renderSpecular;
	}
	if (g_theInput->WasKeyJustPressed( '4' )) {
		m_dlc.m_lightingDebug.m_renderEmissive = 1 - m_dlc.m_lightingDebug.m_renderEmissive;
	}
	if (g_theInput->WasKeyJustPressed( '5' )) {
		m_dlc.m_lightingDebug.m_useDiffuseMap = 1 - m_dlc.m_lightingDebug.m_useDiffuseMap;
	}
	if (g_theInput->WasKeyJustPressed( '6' )) {
		m_dlc.m_lightingDebug.m_useNormalMap = 1 - m_dlc.m_lightingDebug.m_useNormalMap;
	}
	if (g_theInput->WasKeyJustPressed( '7' )) {
		m_dlc.m_lightingDebug.m_useSpecularMap = 1 - m_dlc.m_lightingDebug.m_useSpecularMap;
	}
	if (g_theInput->WasKeyJustPressed( '8' )) {
		m_dlc.m_lightingDebug.m_useGlossinessMap = 1 - m_dlc.m_lightingDebug.m_useGlossinessMap;
	}
	if (g_theInput->WasKeyJustPressed( '9' )) {
		m_dlc.m_lightingDebug.m_useEmissiveMap = 1 - m_dlc.m_lightingDebug.m_useEmissiveMap;
	}
	if (g_theInput->WasKeyJustPressed( 'N' )) {
		m_debugRender = !m_debugRender;
	}

	if (g_theInput->WasKeyJustPressed( 'R' )) {
		m_debugRotation = !m_debugRotation;
	}

	if (g_theInput->WasKeyJustPressed( 'L' )) {
		ExitScene( m_curScene );
		// load models
		std::string fileName;
		g_window->ChooseFileFromBroser( fileName );
		if (!m_model) {
			m_model = new Model(this, fileName);
		}
		else {
			m_model->Load( fileName );
		}
		EventArgs args3;
		args3.SetValue( "KeyCode", Stringf( "%d", (unsigned char)'L' ) );
		FireEvent( "KeyReleased", args3 );
		AddEntityToEntityArrays( m_model );
	}
//#endif // DEBUG_MODE
}

void Game::AddEntityToEntityArrays( Entity* entityToAdd )
{
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i] == nullptr) {
			m_entityArray[i] = entityToAdd;
			return;
		}
	}
	m_entityArray.push_back( entityToAdd );
}

void Game::RemoveEntityFromEntityArrays( Entity* entityToRemove )
{
	for (int i = 0; i < (int)m_entityArray.size(); i++) {
		if (m_entityArray[i] == entityToRemove) {
			m_entityArray[i] = nullptr;
			break;
		}
	}
}

void Game::UpdateEntityArrays()
{
	UpdateEntityArray( m_entityArray );
}

void Game::UpdateEntityArray( EntityList& entityArray )
{
	for (int i = 0; i < (int)entityArray.size(); i++) {
		if (entityArray[i]) {
			entityArray[i]->Update();
		}
	}
}

void Game::UpdateDebugRender()
{
	DebugAddScreenText( Stringf( "Time: %.2f FPS: %.2f Scale: %.1f", Clock::GetSystemClock()->GetTotalSeconds(), 1.f / Clock::GetSystemClock()->GetDeltaSeconds(), Clock::GetSystemClock()->GetTimeScale() ), m_screenCamera.m_cameraBox.m_maxs - Vec2( 410.f, 20.f ), 20.f, Vec2( 0.5f, 0.5f ), 0.f, Rgba8::WHITE, Rgba8::WHITE );
	//DebugAddScreenText( Stringf( "Time: %.2f FPS: %.2f Scale: %.1f", Clock::GetSystemClock()->GetTotalSeconds(), 1.f / Clock::GetSystemClock()->GetDeltaSeconds(), Clock::GetSystemClock()->GetTimeScale() ), m_screenCamera.m_cameraBox.m_maxs - Vec2( 380.f, 40.f ), 20.f, Vec2( 0.5f, 0.5f ), 0.f, Rgba8::WHITE, Rgba8::WHITE );

	DebugAddMessage( Stringf( "Player Position: %.2f %.2f %.2f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z ), 0.f, Rgba8( 255, 255, 255 ), Rgba8( 255, 255, 255 ) );
}

void Game::RenderUI() const
{
	std::vector<Vertex_PCU> textVerts;
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 750.f ), Vec2( 1600.f, 780.f ) ), 25.f, Stringf( "Sun Orientation(Arrows): (%.1f, %.1f, %.1f)", m_sunOrientation.m_yawDegrees, m_sunOrientation.m_pitchDegrees, m_sunOrientation.m_rollDegrees ), Rgba8::WHITE, 0.618f, Vec2( 1.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 720.f ), Vec2( 1600.f, 750.f ) ), 25.f, Stringf( "Scene ([/]):" ), Rgba8::WHITE, 0.618f, Vec2( 1.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 690.f ), Vec2( 1600.f, 720.f ) ), 25.f, Stringf( "Sun Direction: (%.1f, %.1f, %.1f)", m_dlc.m_sunDirection.x, m_dlc.m_sunDirection.y, m_dlc.m_sunDirection.z ), Rgba8::WHITE, 0.618f, Vec2( 1.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 660.f ), Vec2( 1600.f, 690.f ) ), 25.f, Stringf( "Sun Intensity (</>): %.1f", m_dlc.m_sunIntensity ), Rgba8::WHITE, 0.618f, Vec2( 1.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 630.f ), Vec2( 1600.f, 660.f ) ), 25.f, Stringf( "Ambient Intensity (</>): %.1f", m_dlc.m_ambientIntensity ), Rgba8::WHITE, 0.618f, Vec2( 1.f, 0.5f ), TextBoxMode::OVERRUN );
	
	std::string onOffStrs[2];
	onOffStrs[0] = "Off";
	onOffStrs[1] = " On";
	
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 600.f ), Vec2( 1600.f, 630.f ) ), 25.f, Stringf( "Ambient [1]:  %s", onOffStrs[(int)m_dlc.m_lightingDebug.m_renderAmbient].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 1.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 570.f ), Vec2( 1600.f, 600.f ) ), 25.f, Stringf( "Diffuse [2]:  %s", onOffStrs[(int)m_dlc.m_lightingDebug.m_renderDiffuse].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 1.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 540.f ), Vec2( 1600.f, 570.f ) ), 25.f, Stringf( "Specular [3]:  %s", onOffStrs[(int)m_dlc.m_lightingDebug.m_renderSpecular].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 1.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 510.f ), Vec2( 1600.f, 540.f ) ), 25.f, Stringf( "Emissive [4]:  %s", onOffStrs[(int)m_dlc.m_lightingDebug.m_renderEmissive].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 1.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 480.f ), Vec2( 1600.f, 510.f ) ), 25.f, Stringf( "Diffuse Map [5]:  %s", onOffStrs[(int)m_dlc.m_lightingDebug.m_useDiffuseMap].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 1.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 450.f ), Vec2( 1600.f, 480.f ) ), 25.f, Stringf( "Normal Map [6]:  %s", onOffStrs[(int)m_dlc.m_lightingDebug.m_useNormalMap].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 1.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 420.f ), Vec2( 1600.f, 450.f ) ), 25.f, Stringf( "Specular Map [7]:  %s", onOffStrs[(int)m_dlc.m_lightingDebug.m_useSpecularMap].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 1.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 390.f ), Vec2( 1600.f, 420.f ) ), 25.f, Stringf( "Glossiness Map [8]:  %s", onOffStrs[(int)m_dlc.m_lightingDebug.m_useGlossinessMap].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 1.f, 0.5f ), TextBoxMode::OVERRUN );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 360.f ), Vec2( 1600.f, 390.f ) ), 25.f, Stringf( "Emissive Map [9]:  %s", onOffStrs[(int)m_dlc.m_lightingDebug.m_useEmissiveMap].c_str() ), Rgba8::WHITE, 0.618f, Vec2( 1.f, 0.5f ), TextBoxMode::OVERRUN );

	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );
}

void Game::SetupTestProps()
{
	m_prop1 = new Prop(this);
	m_prop1->CreateCube( "Data/Materials/Grass.xml" );
	m_prop1->m_position = Vec3( -2.f, -2.f, 0.f );

	m_prop2 = new Prop( this );
	m_prop2->CreateSphere( "Data/Materials/Grass.xml" );
	m_prop2->m_position = Vec3( 2.f, -2.f, 0.f );

	m_prop3 = new Prop( this );
	m_prop3->CreateCube( "Data/Materials/Brick.xml" );
	m_prop3->m_position = Vec3( 2.f, 2.f, 0.f );

	m_prop0 = new Prop( this );
	m_prop0->CreateSphere( "Data/Materials/Brick.xml" );
	m_prop0->m_position = Vec3( -2.f, 2.f, 0.f );

	m_model0 = new Model( this, "Data/Models/Cube_Textured.xml" );
	m_model0->m_position = Vec3( 0.f, 0.f, 0.5f );
	AddEntityToEntityArrays( m_model0 );

	m_model1 = new Model( this, "Data/Models/Cube_Emissive.xml" );
	m_model1->m_position = Vec3( -1.f, -1.f, 0.5f );
	AddEntityToEntityArrays( m_model1 );

	m_model2 = new Model( this, "Data/Models/Cube_Emissive.xml" );
	m_model2->m_position = Vec3( 1.f, -1.f, 0.5f );
	m_model2->m_color = Rgba8( 255, 0, 0 );
	AddEntityToEntityArrays( m_model2 );

	m_model3 = new Model( this, "Data/Models/Cube_Emissive.xml" );
	m_model3->m_position = Vec3( 1.f, 1.f, 0.5f );
	m_model3->m_color = Rgba8( 0, 255, 0 );
	AddEntityToEntityArrays( m_model3 );

	m_model4 = new Model( this, "Data/Models/Cube_Emissive.xml" );
	m_model4->m_position = Vec3( -1.f, 1.f, 0.5f );
	m_model4->m_color = Rgba8( 0, 0, 255 );
	AddEntityToEntityArrays( m_model4 );

	m_tutorialBox = new Model( this, "Data/Models/Tutorial_Box.xml" );
	m_hadrianTank = new Model( this, "Data/Models/Hadrian.xml" );

	m_player = new Player( this );
	AddEntityToEntityArrays( m_player );
	m_player->m_camera.SetRenderBasis( Vec3( 0.f, 0.f, 1.f ), Vec3( -1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ) );
	m_player->m_camera.SetOrthoView( Vec2( -1.f, -1.f ), Vec2( 1.f, 1.f ), 0.f, 1.f );
	m_player->m_camera.SetPerspectiveView( g_window->GetAspect(), 60.f, 0.1f, 100.f );
	m_player->m_camera.m_mode = CameraMode::Perspective;
	m_player->m_position = Vec3( 3.f, 0.f, 3.f );
	m_player->m_orientation.m_yawDegrees = 180.f;
	m_player->m_orientation.m_pitchDegrees = 45.f;

	//Prop* test1 = new Prop( this );
	//AddVertsForCylinder3D( test1->m_vertexes, Vec3( -2, -2, -2 ), Vec3( 2, 2, 2 ), 1.f, Rgba8::WHITE, AABB2::IDENTITY, 16 );
	//AddVertsForCone3D( test1->m_vertexes, Vec3( -2, -2, -2 ), Vec3( 2, 2, 2 ), 1.f, Rgba8::WHITE, AABB2::IDENTITY, 16 );
	//test1->m_texture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/TestUV.png" );
	//AddEntityToEntityArries( test1 );
}

void Game::SetupGrids( int minX /*= -50*/, int maxX /*= 50*/, int minY /*= -50*/, int maxY /*= 50 */ )
{
	//DebugAddWorldArrow( Vec3( 0, 0, 0 ), Vec3( 1, 0, 0 ), 0.1f, -1.f, Rgba8( 255, 0, 0 ), Rgba8( 255, 0, 0 ), DebugRenderMode::USE_DEPTH );
	//DebugAddWorldArrow( Vec3( 0, 0, 0 ), Vec3( 0, 1, 0 ), 0.1f, -1.f, Rgba8( 0, 255, 0 ), Rgba8( 0, 255, 0 ), DebugRenderMode::USE_DEPTH );
	//DebugAddWorldArrow( Vec3( 0, 0, 0 ), Vec3( 0, 0, 1 ), 0.1f, -1.f, Rgba8( 0, 0, 255 ), Rgba8( 0, 0, 255 ), DebugRenderMode::USE_DEPTH );

	Mat44 xMat = Mat44::CreateZRotationDegrees( -90.f );
	xMat.SetTranslation3D( Vec3( 1.f, 0.f, 0.4f ) );
	Mat44 yMat = Mat44::CreateTranslation3D( Vec3( 0.f, 1.f, 0.4f ) );
	Mat44 zMat = Mat44::CreateXRotationDegrees( 90.f );
	zMat.SetTranslation3D( Vec3( 0.f, -0.4f, 1.f ) );
	//DebugAddWorldText( "x-forward", xMat, 0.3f, Vec2( 0.5f, 0.5f ), -1.f, Rgba8( 255, 0, 0 ), Rgba8( 255, 0, 0 ), DebugRenderMode::USE_DEPTH );
	//DebugAddWorldText( "y-left", yMat, 0.3f, Vec2( 0.5f, 0.5f ), -1.f, Rgba8( 0, 255, 0 ), Rgba8( 0, 255, 0 ), DebugRenderMode::USE_DEPTH );
	//DebugAddWorldText( "z-up", zMat, 0.3f, Vec2( 0.5f, 0.5f ), -1.f, Rgba8( 0, 0, 255 ), Rgba8( 0, 0, 255 ), DebugRenderMode::USE_DEPTH );

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

void Game::ExitScene( int scene )
{
	if (scene == 0) {
		RemoveEntityFromEntityArrays( m_model0 );
		RemoveEntityFromEntityArrays( m_model1 );
		RemoveEntityFromEntityArrays( m_model2 );
		RemoveEntityFromEntityArrays( m_model3 );
		RemoveEntityFromEntityArrays( m_model4 );
		RemoveEntityFromEntityArrays( m_model );
		for (auto& entity : m_entityArray) {
			if (entity != m_player) {
				delete entity;
				entity = nullptr;
			}
		}
	}
	else if (scene == 1) {
		RemoveEntityFromEntityArrays( m_prop0 );
		RemoveEntityFromEntityArrays( m_prop1 );
		RemoveEntityFromEntityArrays( m_prop2 );
		RemoveEntityFromEntityArrays( m_prop3 );
		RemoveEntityFromEntityArrays( m_model );
		for (auto& entity : m_entityArray) {
			if (entity != m_player) {
				delete entity;
				entity = nullptr;
			}
		}
	}
	else if (scene == 2) {
		RemoveEntityFromEntityArrays( m_tutorialBox );
		RemoveEntityFromEntityArrays( m_model );
		for (auto& entity : m_entityArray) {
			if (entity != m_player) {
				delete entity;
				entity = nullptr;
			}
		}
	}
	else {
		RemoveEntityFromEntityArrays( m_hadrianTank );
		RemoveEntityFromEntityArrays( m_model );
		for (auto& entity : m_entityArray) {
			if (entity != m_player) {
				delete entity;
				entity = nullptr;
			}
		}
	}
}

void Game::GoToScene( int scene )
{
	if (scene == 0) {
		AddEntityToEntityArrays( m_model0 );
		AddEntityToEntityArrays( m_model1 );
		AddEntityToEntityArrays( m_model2 );
		AddEntityToEntityArrays( m_model3 );
		AddEntityToEntityArrays( m_model4 );
	}
	else if (scene == 1) {
		AddEntityToEntityArrays( m_prop0 );
		AddEntityToEntityArrays( m_prop1 );
		AddEntityToEntityArrays( m_prop2 );
		AddEntityToEntityArrays( m_prop3 );
	}
	else if (scene == 2) {
		AddEntityToEntityArrays( m_tutorialBox );
	}
	else {
		AddEntityToEntityArrays( m_hadrianTank );
	}
}
