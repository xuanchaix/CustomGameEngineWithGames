#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/Item.hpp"
#include "Game/Room.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/SettingsScreen.hpp"
#include <time.h>
#include "Game/PlayerController.hpp"
#include <filesystem>

// follow the instructions on the manual
// All global variables Created and owned by the App
App* g_theApp = nullptr;
Game* g_theGame = nullptr;
Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_window = nullptr;
BitmapFont* g_ASCIIFont = nullptr;
SpriteSheet* g_pickupsSprites = nullptr;
Texture* g_floorPortalTexture = nullptr;

App::App() {
}

App::~App() {

}

void App::Startup() {
	std::filesystem::create_directory( "Saves" );

	srand( (unsigned int)time( NULL ) );
	SetUpBlackBoard();
	Clock::TickSystemClock();

	EventSystemConfig eConfig;
	g_theEventSystem = new EventSystem( eConfig );
	g_theEventSystem->Startup();

	LoadGameSettings();
	m_settingsScreen = new SettingsScreen();

	WindowConfig wConfig;
	wConfig.m_clientAspect = 2.f;
	wConfig.m_isFullScreen = false;
	wConfig.m_windowTitle = std::string( APP_NAME );
	g_window = new Window( wConfig );
	g_window->StartUp();
	g_window->SetFullScreen( m_fullScreen );

	RendererConfig rConfig;
	rConfig.m_window = g_window;
	g_theRenderer = new Renderer( rConfig );
	g_theRenderer->StartUp();
	SetUpTexture();
	DebugRenderConfig drConfig;
	drConfig.m_renderer = g_theRenderer;
	//drConfig.m_fontName = 
	DebugRenderSystemStartup( drConfig );

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

	ParticleSystem2DConfig psConfig;
	psConfig.m_renderer = g_theRenderer;
	psConfig.m_clock = Clock::GetSystemClock();
	ParticleSystem2DStartup( psConfig );

	SetUpAudio();
	m_attractModeMusic = g_theAudio->StartSound( m_audioDictionary[(int)AudioName::AttractMode], true, g_theApp->m_musicVolume * 0.6f );
	//m_theGame = new Game();
	//m_theGame->Startup();

	SubscribeEventCallbackFunction( "quit", App::SetQuitting );
	g_devConsole->AddLine( DevConsole::INFO_MINOR, "Initializing..." );

	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Successfully Initialized!" );

	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Input Info:" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "P: Pause O: Run a Frame then Pause T: Time scale = 0.1" );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Esc: Exit Game F8: Restart Game" );

	m_attractModeCamera->SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );
	m_attractModeCamera->m_mode = CameraMode::Orthographic;
	LoadDefinitions();
	g_theInput->SetCursorMode( true, false );
}

void App::Shutdown() {
	SaveGameSettings();
	ParticleSystem2DShutdown();
	DebugRenderSystemShutdown();
	g_theRenderer->Shutdown();
	g_theInput->ShutDown();
	g_theAudio->Shutdown();
	g_window->Shutdown();
	g_devConsole->Shutdown();
	g_theEventSystem->Shutdown();

	delete m_settingsScreen;
	delete g_pickupsSprites;
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
		RunFrame();
	}
}

void App::RunFrame() {
	BeginFrame();
	Update();
	Render();
	EndFrame();
}

void App::UpdateAppState()
{
	if (m_appState != m_appStateNextFrame) {
		if (m_appState == AppState::PLAY_MODE && m_appStateNextFrame == AppState::ATTRACT_MODE) {
			m_attractModeMusic = g_theAudio->StartSound( m_audioDictionary[(int)AudioName::AttractMode], true, g_theApp->m_musicVolume * 0.6f );
			ItemDefinition::ResetAllItems();
			RoomDefinition::ResetAllDefinitionsForEachGame();
			delete g_theGame;
			g_theGame = nullptr;
			if (m_bossRushFlag) {
				m_bossRushFlag = false;
			}
		}
		else if (m_appState == AppState::ATTRACT_MODE && m_appStateNextFrame == AppState::PLAY_MODE) {
			g_theAudio->StopSound( m_attractModeMusic );
			g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Enter Game" );
			g_theGame = new Game();
			g_theGame->Startup();
		}
		else if (m_appStateNextFrame == AppState::SETTINGS_MODE) {
			m_settingsScreen->GoIntoScreen();
		}
		else if (m_appState == AppState::SETTINGS_MODE) {
			m_settingsScreen->ExitScreen();
		}
		m_appState = m_appStateNextFrame;
	}
}

void App::UpdateAttractMode( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	if (g_theInput->WasKeyJustPressed( KEYCODE_ESC )) {
		m_isQuitting = true;
	}

	if (g_theInput->WasKeyJustPressed( KEYCODE_SPACE ) || g_theInput->WasKeyJustPressed( KEYCODE_ENTER )
		|| g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_START )
		|| g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_A ))
	{
		g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Click.wav" ), false, g_theApp->m_soundVolume * 0.6f );
		if (m_curHoveringButtom == 0) {
			GoToAppMode( AppState::PLAY_MODE );
		}
		else if (m_curHoveringButtom == 1) {
			m_bossRushFlag = true;
			GoToAppMode( AppState::PLAY_MODE );
		}
		else if (m_curHoveringButtom == 2) {
			GoToAppMode( AppState::SETTINGS_MODE );
		}
		else if (m_curHoveringButtom == 3) {
			m_isQuitting = true;
		}
	}

	if (g_theInput->WasKeyJustPressed( 'W' ) || g_theInput->WasKeyJustPressed( KEYCODE_UPARROW )) {
		m_curHoveringButtom = (m_curHoveringButtom - 1 + m_numOfAttractButtons) % m_numOfAttractButtons;
	}
	if (g_theInput->WasKeyJustPressed( 'S' ) || g_theInput->WasKeyJustPressed( KEYCODE_DOWNARROW )) {
		m_curHoveringButtom = (m_curHoveringButtom + 1) % m_numOfAttractButtons;
	}
}

void App::UpdateGameMode()
{
	g_theGame->Update();
	ParticleSystem2DUpdate();
}

void App::GoToAppMode( AppState modeToGo )
{
	m_appStateNextFrame = modeToGo;
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
	ParticleSystem2DBeginFrame();
	DebugRenderBeginFrame();
}

void App::Update() {
	float deltaSeconds = Clock::GetSystemClock()->GetDeltaSeconds();

	UpdateAppState();


	if (m_appState == AppState::ATTRACT_MODE) {
		UpdateAttractMode( deltaSeconds );
	}
	else if (m_appState == AppState::PLAY_MODE) {
		UpdateGameMode();
	}
	else if (m_appState == AppState::SETTINGS_MODE) {
		m_settingsScreen->Update();
	}

}

void App::Render() const
{
	g_theRenderer->ClearScreen( Rgba8( 40, 54, 83 ) );

	if (m_appState == AppState::PLAY_MODE) {
		RenderGameMode();
	}
	else if(m_appState == AppState::ATTRACT_MODE) {
		RenderAttractMode();
	}
	else if (m_appState == AppState::SETTINGS_MODE) {
		m_settingsScreen->Render();
	}

	if (g_devConsole->GetMode() == DevConsoleMode::EMERGE) {
		g_devConsole->Render( m_attractModeCamera->m_cameraBox );
	}

	DebugRenderScreen( *m_attractModeCamera );
}

void App::EndFrame() {
	g_window->EndFrame();
	g_theRenderer->EndFrame();
	g_theInput->EndFrame();
	g_theAudio->EndFrame();
	g_devConsole->EndFrame();
	g_theEventSystem->EndFrame();
	ParticleSystem2DEndFrame();
}

void App::LoadDefinitions()
{
	ProjectileDefinition::SetUpProjectileDefinitions();
	WeaponDefinition::SetUpWeaponDefinitions();
	ItemDefinition::SetUpItemDefinitions();
	EntityDefinition::SetUpEntityDefinitions();
	RoomDefinition::SetUpRoomDefinitions();
}

void App::RenderAttractMode() const
{
	g_theRenderer->BeginCamera( *m_attractModeCamera );
	// buttons
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;

	Vec2 buttonLB[4] = { Vec2( 675.f, 480.f ),  Vec2( 675.f, 360.f ), Vec2( 675.f, 240.f ), Vec2( 675.f, 120.f ) };
	std::string buttonDesc[4] = { "Start Game", "Boss Rush", "Settings", "Quit" };
	float buttonWidth = 250.f;
	float buttonHeight = 75.f;

	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 630.f ), Vec2( 1600.f, 700.f ) ), 70.f, "Starship Ultimate", Rgba8( 255, 255, 0 ) );
	for (int i = 0; i < m_numOfAttractButtons; i++) {
		AddVertsForAABB2D( verts, AABB2( buttonLB[i], buttonLB[i] + Vec2( buttonWidth, buttonHeight ) ), Rgba8( 255, 255, 153 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( buttonLB[i], buttonLB[i] + Vec2( buttonWidth, buttonHeight ) ), 25.f, buttonDesc[i], Rgba8( 0, 0, 0 ) );
		if (m_curHoveringButtom == i) {
			Rgba8 color = Rgba8( 51, 51, 255 );
			AddVertsForLineSegment2D( verts, buttonLB[i], buttonLB[i] + Vec2( 0.f, 6.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i], buttonLB[i] + Vec2( 6.f, 0.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( buttonWidth, buttonHeight ), buttonLB[i] + Vec2( buttonWidth, buttonHeight ) + Vec2( 0.f, -6.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( buttonWidth, buttonHeight ), buttonLB[i] + Vec2( buttonWidth, buttonHeight ) + Vec2( -6.f, 0.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( buttonWidth, 0.f ), buttonLB[i] + Vec2( buttonWidth, 0.f ) + Vec2( 0.f, 6.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( buttonWidth, 0.f ), buttonLB[i] + Vec2( buttonWidth, 0.f ) + Vec2( -6.f, 0.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( 0.f, buttonHeight ), buttonLB[i] + Vec2( 0.f, buttonHeight ) + Vec2( 0.f, -6.f ), 5.f, color );
			AddVertsForLineSegment2D( verts, buttonLB[i] + Vec2( 0.f, buttonHeight ), buttonLB[i] + Vec2( 0.f, buttonHeight ) + Vec2( 6.f, 0.f ), 5.f, color );
		}
	}

	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 20.f ), Vec2( 1600.f, 60.f ) ), 35.f, "WASD Navigate   Space/Enter Confirm   Esc Quit", Rgba8::WHITE );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );

	g_theRenderer->EndCamera( *m_attractModeCamera );
}

void App::RenderGameMode() const
{
	g_theGame->Render();
}

void App::SetUpAudio()
{
	m_audioDictionary[(int)AudioName::AttractMode] = g_theAudio->CreateOrGetSound( "Data/Music/MainMenu.mp3" );
}

void App::SetUpTexture()
{
	g_ASCIIFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
	Texture* pickupTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/HealthPickup.png" );
	g_pickupsSprites = new SpriteSheet( *pickupTexture, IntVec2( 4, 1 ) );
	g_floorPortalTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/LevelPortal.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Reticle.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/MercyKiller.png" );
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

void App::SaveGameSettings()
{
	std::vector<uint8_t> buffer;
	buffer.reserve( 100 );
	buffer.push_back( 'S' );
	buffer.push_back( 'S' );
	buffer.push_back( 'U' );
	buffer.push_back( 'T' );

	uint8_t* musicVolume = (uint8_t*)&m_musicVolume;
	buffer.push_back( musicVolume[0] );
	buffer.push_back( musicVolume[1] );
	buffer.push_back( musicVolume[2] );
	buffer.push_back( musicVolume[3] );

	uint8_t* soundVolume = (uint8_t*)&m_soundVolume;
	buffer.push_back( soundVolume[0] );
	buffer.push_back( soundVolume[1] );
	buffer.push_back( soundVolume[2] );
	buffer.push_back( soundVolume[3] );

	buffer.push_back( uint8_t( m_fullScreen ) );
	buffer.push_back( uint8_t( m_autoShootMainWeapon ) );
	buffer.push_back( uint8_t( m_autoShootSubWeapon ) );
	buffer.push_back( uint8_t( PLAYER_UP_KEYCODE ) );
	buffer.push_back( uint8_t( PLAYER_DOWN_KEYCODE ) );
	buffer.push_back( uint8_t( PLAYER_LEFT_KEYCODE ) );
	buffer.push_back( uint8_t( PLAYER_RIGHT_KEYCODE ) );
	buffer.push_back( uint8_t( PLAYER_MAIN_WEAPON_KEYCODE ) );
	buffer.push_back( uint8_t( PLAYER_SUB_WEAPON_KEYCODE ) );
	buffer.push_back( uint8_t( PLAYER_DASH_KEYCODE ) );
	buffer.push_back( uint8_t( PLAYER_ULTIMATE_KEYCODE ) );
	buffer.push_back( uint8_t( PLAYER_INTERACT_KEYCODE ) );
	buffer.push_back( uint8_t( PLAYER_ITEM_SCREEN_KEYCODE ) );
	buffer.push_back( uint8_t( PLAYER_MAP_SCREEN_KEYCODE ) );
	buffer.push_back( uint8_t( m_playerNoDie ) );

	BufferWriteToFile( buffer, Stringf( "Saves/Settings.ssut" ) );
}

void App::LoadGameSettings()
{
	std::vector<uint8_t> buffer;
	int res = FileReadToBuffer( buffer, Stringf( "Saves/Settings.ssut" ) );
	if (res == -1 || (int)buffer.size() == 0) {
		return;
	}
	if (!(buffer[0] == 'S' && buffer[1] == 'S' && buffer[2] == 'U' && buffer[3] == 'T')) {
		return;
	}

	m_musicVolume = *(float*)&buffer[4];
	m_soundVolume = *(float*)&buffer[8];
	m_fullScreen = (bool)buffer[12];
	m_autoShootMainWeapon = (bool)buffer[13];
	m_autoShootSubWeapon = (bool)buffer[14];

	PLAYER_UP_KEYCODE = (char)buffer[15];
	PLAYER_DOWN_KEYCODE = (char)buffer[16];
	PLAYER_LEFT_KEYCODE = (char)buffer[17];
	PLAYER_RIGHT_KEYCODE = (char)buffer[18];
	PLAYER_MAIN_WEAPON_KEYCODE = (char)buffer[19];
	PLAYER_SUB_WEAPON_KEYCODE = (char)buffer[20];
	PLAYER_DASH_KEYCODE = (char)buffer[21];
	PLAYER_ULTIMATE_KEYCODE = (char)buffer[22];
	PLAYER_INTERACT_KEYCODE = (char)buffer[23];
	PLAYER_ITEM_SCREEN_KEYCODE = (char)buffer[24];
	PLAYER_MAP_SCREEN_KEYCODE = (char)buffer[25];
	m_playerNoDie = (bool)buffer[26];
}

