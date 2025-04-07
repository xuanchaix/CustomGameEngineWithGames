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

bool APP_TestEventSystemFunc1(EventArgs& args) {
	g_devConsole->AddLine( DevConsole::INFO_ERROR, Stringf( "triggered by event system1! First=%i Second=%i", args.GetValue( "First", 0 ), args.GetValue( "Second", 0 ) ) );
	// adjust this to test consume
	return false;
}

bool APP_TestEventSystemFunc2( EventArgs& args ) {
	g_devConsole->AddLine( DevConsole::INFO_ERROR, Stringf( "triggered by event system2! First=%i Second=%i", args.GetValue( "First", 0 ), args.GetValue( "Second", 0 ) ) );
	return false;
}

App::App() {
	m_timeEnd = 0;
	m_timeStart = 0;
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

	DevConsoleConfig dConfig;
	dConfig.m_defaultFont = g_ASCIIFont;
	dConfig.m_defaultRenderer = g_theRenderer;
	dConfig.m_camera = &m_attractModeCamera;
	g_devConsole = new DevConsole( dConfig );
	g_devConsole->Startup();

	InputSystemConfig iConfig;
	g_theInput = new InputSystem( iConfig );
	g_theInput->StartUp();

	AudioSystemConfig aConfig;
	g_theAudio = new AudioSystem( aConfig );
	g_theAudio->Startup();

	SetUpAudio();
	m_attractModeMusic = g_theAudio->StartSound( m_audioDictionary[(int)AudioName::AttractMode], true );

	SubscribeEventCallbackFunction( "quit", App::SetQuitting );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Libra Version 0.1" );
	g_devConsole->AddLine( DevConsole::INFO_MINOR, "Initializing..." );

	attractTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/AttractScreen.png" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Successfully Initialized!" );

	m_attractModeCamera.m_mode = CameraMode::Orthographic;
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
	m_attractModeCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );
	IntVec2 dimensions = g_window->GetClientDimensions();
	float xLength = (float)dimensions.x;
	float yLength = (float)dimensions.y;
	m_attractModeCamera.SetViewPort( AABB2( Vec2( 0.f, 0.f ), Vec2( xLength, yLength ) ) );
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
		if (m_isQuitting) {
			break;
		}
		RunFrame();
	}
}

void App::RunFrame() {
	BeginFrame();
	m_timeEnd = GetCurrentTimeSeconds();
	float deltaTime = (float)(m_timeEnd - m_timeStart);
	static float s_frameTimeSum = 0.f;
	static int s_frameCount = 0;
	s_frameTimeSum += deltaTime;
	deltaTime = GetClamped( deltaTime, 0.f, 0.1f );
	m_timeStart = m_timeEnd;
	++s_frameCount;
	if (s_frameTimeSum >= 0.3f) {
		m_framePerSecond = RoundDownToInt( (float)s_frameCount / s_frameTimeSum );
		s_frameTimeSum = 0.f;
		s_frameCount = 0;
	}
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
	if (g_theInput->WasKeyJustPressed( 0x54 )) // T key slow the game
	{
		m_isSlowMo = true;
	}
	if (g_theInput->WasKeyJustPressed( 'Y' )) // Y key speed up the game
	{
		m_isFastMo = true;
	}
	if (g_theInput->WasKeyJustReleased( 'Y' )) // Y key speed up the game
	{
		m_isFastMo = false;
	}
	if (!m_toAttractMode && !m_attractMode && (g_theInput->WasKeyJustPressed( 0x50 ) || g_theInput->GetController(0).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))) // P key pause the game; handle the pause problem
	{
		m_isPaused = !m_isPaused;
		if (m_isPaused) {
			PlaySound( AudioName::Pause );
		}
		else {
			PlaySound( AudioName::UnPause );
		}
	}
	if (g_theInput->WasKeyJustPressed( KEYCODE_F1 )) // F1 debug mode
	{
		m_debugMode = !m_debugMode;
	}
	if (g_theInput->WasKeyJustReleased( 0x54 )) { // T key slow the game
		m_isSlowMo = false;
	}
	if (g_theInput->WasKeyJustPressed( KEYCODE_F8 )) { // F8 to restart the game
		delete g_theGame;
		g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Restart Game" );
		g_theGame = new Game();
		g_theGame->Startup();
	}
#endif
	if (g_theInput->WasKeyJustPressed( KEYCODE_TILDE )) {
		g_devConsole->ToggleMode( DevConsoleMode::EMERGE );
	}

	if ((g_theInput->WasKeyJustPressed( KEYCODE_ESC ) || g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_BACK )) && !m_attractMode)
	{
		if (g_theGame && g_theGame->GetCurrentMap()->m_curMapState != MapState::WIN) {
			if (m_isPaused) {
				m_isPaused = false;
				ToAttractMode();
			}
			else {
				m_isPaused = true;
			}
		}
	}
	if (g_theInput->WasKeyJustPressed( KEYCODE_ESC ) && m_attractMode) {
		m_isQuitting = true;
	}

	// ' ' or P enter game
	if ((g_theInput->WasKeyJustPressed( 'P' )
		|| g_theInput->WasKeyJustPressed( KEYCODE_SPACE )
		|| g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_START )
		|| g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_A ))
		&& m_attractMode)
	{
		m_attractMode = false;
		g_theAudio->StopSound( m_attractModeMusic );
		m_startButtonA = -STRAT_BUTTON_TIME;
		PlaySound( AudioName::StartGame );
		g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Enter Game" );
		g_theGame = new Game();
		g_theGame->Startup();
	}
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
	g_window->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theInput->BeginFrame();
	g_theAudio->BeginFrame();
	g_devConsole->BeginFrame();
	g_theEventSystem->BeginFrame();
}

void App::Update( float deltaSeconds ) {
	//m_timePassed += deltaSeconds;

	if (m_toAttractMode) {
		m_toAttractMode = false;
		delete g_theGame;
		g_theGame = nullptr;
		m_attractMode = true;
		m_attractModeMusic = g_theAudio->StartSound( m_audioDictionary[(int)AudioName::AttractMode], true );
	}

	if (m_attractMode) {
		m_startButtonA += deltaSeconds;
		if (m_startButtonA > STRAT_BUTTON_TIME) {
			m_startButtonA = -STRAT_BUTTON_TIME;
		}
	}

	if (!m_attractMode) {
		if (m_isSlowMo && !m_isFastMo) {
			deltaSeconds = deltaSeconds * 0.1f;
		}
		if (m_isPaused) {
			deltaSeconds = 0;
		}
		if (m_isFastMo && !m_isSlowMo) {
			deltaSeconds = deltaSeconds * 4.f;
		}
		if (m_isSlowMo && m_isFastMo) {
			deltaSeconds = deltaSeconds * 8.f;
		}

		g_theGame->Update( deltaSeconds );

		if (m_pauseAfterUpdate) {
			m_pauseAfterUpdate = false;
			m_isPaused = true;
		}
	}

	HandleKey();
}

void App::Render() const 
{
	// Clear all screen (back buffer) pixels to medium-blue
	g_theRenderer->ClearScreen( Rgba8( 0, 0, 0, 255 ) );


	if (!m_attractMode) {
		g_theGame->Render();
	}
	else {
		g_theRenderer->BeginCamera( m_attractModeCamera );
		RenderAttractMode();
		g_theRenderer->EndCamera( m_attractModeCamera );
	}

	if (g_devConsole->GetMode() == DevConsoleMode::EMERGE) {
		g_devConsole->Render( m_attractModeCamera.m_cameraBox );
	}
}

void App::EndFrame() {
	g_window->EndFrame();
	g_theRenderer->EndFrame();
	g_theInput->EndFrame();
	g_theAudio->EndFrame();
	g_devConsole->EndFrame();
	g_theEventSystem->EndFrame();
}

void App::RenderAttractMode() const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 6 );
	AddVertsForAABB2D( verts, m_attractModeCamera.m_cameraBox, Rgba8( 255, 255, 255, 255 ) );
	g_theRenderer->BindTexture( attractTexture );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( (int)verts.size(), verts.data() );

	float a;
	if (m_startButtonA < 0) {
		a = -m_startButtonA;
	}
	else {
		a = m_startButtonA;
	}

	a = RangeMap( a, 0.f, 1.f, 0.2f, 1.f );
	Vertex_PCU startIconVerts[3] = {
		Vertex_PCU( Vec3( 750, 200, 0 ), Rgba8( 0, 255, 0, (unsigned char)(255 * a) ), Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 750, 600, 0 ), Rgba8( 0, 255, 0, (unsigned char)(255 * a) ), Vec2( 0, 0 ) ),
		Vertex_PCU( Vec3( 1150, 400, 0 ), Rgba8( 0, 255, 0, (unsigned char)(255 * a) ), Vec2( 0, 0 ) ),
	};
	
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 3, startIconVerts );

	
	//SpriteAnimDefinition* anim = m_animDictionary[(int)AnimationName::Explosion];
	//std::vector<Vertex_PCU> textVerts;
	//textVerts.reserve( 1000 );
	//AddVertsForAABB2D( textVerts, AABB2( Vec2( 0, 0 ), Vec2( 500, 500 ) ), Rgba8::WHITE, anim->GetSpriteDefAtTime( m_timePassed ).GetUVs() );
	//g_theRenderer->BindTexture( anim->GetTexture() );
	//g_theRenderer->DrawVertexArray( textVerts );
	/*
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2(0.f, 0.f), Vec2(300.f, 300.f) ), 50.f, std::string( "jdscnsjc\nnewcnewkcnkewnckjewnckjewn\nkew" ), Rgba8::WHITE, 0.6f, Vec2(1.f, 1.f), TextBoxMode::SHRINK_TO_FIT, 10 );
	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 250.f, 400.f ), 40.f, "It's nice to have options!", Rgba8::RED, 0.6f );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->DrawVertexArray( textVerts );*/

}

void App::SetUpAudio()
{
	m_audioDictionary[(int)AudioName::AttractMode] = g_theAudio->CreateOrGetSound( "Data/Audio/AttractMusic.mp3" );
	m_audioDictionary[(int)AudioName::Click] = g_theAudio->CreateOrGetSound( "Data/Audio/Click.mp3" );
	m_audioDictionary[(int)AudioName::GameMode] = g_theAudio->CreateOrGetSound( "Data/Audio/GameplayMusic.mp3" );
	m_audioDictionary[(int)AudioName::Pause] = g_theAudio->CreateOrGetSound( "Data/Audio/Pause.mp3" );
	m_audioDictionary[(int)AudioName::UnPause] = g_theAudio->CreateOrGetSound( "Data/Audio/Unpause.mp3" );
	m_audioDictionary[(int)AudioName::StartGame] = g_theAudio->CreateOrGetSound( "Data/Audio/Welcome.mp3" );
	m_audioDictionary[(int)AudioName::BulletBounce] = g_theAudio->CreateOrGetSound( "Data/Audio/BulletBounce.wav" );
	m_audioDictionary[(int)AudioName::BulletRicochet] = g_theAudio->CreateOrGetSound( "Data/Audio/BulletRicochet.wav" );
	m_audioDictionary[(int)AudioName::BulletRicochet2] = g_theAudio->CreateOrGetSound( "Data/Audio/BulletRicochet2.wav" );
	m_audioDictionary[(int)AudioName::EnemyDied] = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyDied.wav" );
	m_audioDictionary[(int)AudioName::EnemyHit] = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyHit.wav" );
	m_audioDictionary[(int)AudioName::EnemyShoot] = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyShoot.wav" );
	m_audioDictionary[(int)AudioName::ExitMap] = g_theAudio->CreateOrGetSound( "Data/Audio/ExitMap.wav" );
	m_audioDictionary[(int)AudioName::GameOver] = g_theAudio->CreateOrGetSound( "Data/Audio/GameOver.mp3" );
	m_audioDictionary[(int)AudioName::PlayerHit] = g_theAudio->CreateOrGetSound( "Data/Audio/PlayerHit.wav" );
	m_audioDictionary[(int)AudioName::PlayerShootNormal] = g_theAudio->CreateOrGetSound( "Data/Audio/PlayerShootNormal.ogg" );
	m_audioDictionary[(int)AudioName::Victory] = g_theAudio->CreateOrGetSound( "Data/Audio/Victory.mp3" );
	m_audioDictionary[(int)AudioName::EnemyAlert] = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyAlertSFX.wav" );
}

void App::SetUpTexture()
{
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/AttractScreen.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyAries.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyBolt.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyBullet.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyCannon.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyGatling.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyShell.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank0.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank1.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank2.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank3.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank4.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTurretBase.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Extras_4x4.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyBolt.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyBullet.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyCannon.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyGatling.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyShell.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTank0.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTank1.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTank2.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTank3.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTank4.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTurretBase.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankBase.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankTop.png" );

	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_8x8.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/VictoryScreen.jpg" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/YouDiedScreen.png" );

	g_ASCIIFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
	SpriteSheet* sprites = new SpriteSheet( *(g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Explosion_5x5.png" )), IntVec2( 5, 5 ) );
	m_animDictionary[(int)AnimationName::EntityExplosion] = new SpriteAnimDefinition( *sprites, 0, 24, 0.6f, SpriteAnimPlaybackType::ONCE );
	m_animDictionary[(int)AnimationName::BulletExplosion] = new SpriteAnimDefinition( *sprites, 0, 24, 0.4f, SpriteAnimPlaybackType::ONCE );
	m_animDictionary[(int)AnimationName::FlameBullet] = new SpriteAnimDefinition( *sprites, 0, 24, 1.f, SpriteAnimPlaybackType::ONCE );
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

