#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/SM_BitMapFont.hpp"
#include "Game/TranslationUtils.hpp"
#include "Game/Force.hpp"
#include "Game/Province.hpp"
#include "Game/Army.hpp"
#include "Game/Map.hpp"
#include "Game/AIUtils.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <time.h>

// follow the instructions on the manual
// All global variables Created and owned by the App
App* g_theApp = nullptr;
Game* g_theGame = nullptr;
Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_window = nullptr;
BitmapFont* g_ASCIIFont = nullptr;
SM_BitmapFont* g_chineseFont = nullptr;
SM_GameLanguage g_gameLanguage = SM_GameLanguage::ZH;

App::App() {
	m_timeEnd = 0;
	m_timeStart = 0;
}

App::~App() {

}

void App::Startup() {
	SetUpBlackBoard();
	Clock::TickSystemClock();
	srand( (unsigned int)time( NULL ) );

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
	SetUpLocalisations();
	DebugRenderConfig drConfig;
	drConfig.m_renderer = g_theRenderer;
	//drConfig.m_fontName = 
	DebugRenderSystemStartup( drConfig );

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
	m_attractModeMusic = g_theAudio->StartSound( m_audioDictionary[(int)AudioName::AttractMode], false );
	m_timeStart = GetCurrentTimeSeconds();
	//m_theGame = new Game();
	//m_theGame->Startup();

	SubscribeEventCallbackFunction( "quit", App::SetQuitting );
	g_devConsole->AddLine( DevConsole::INFO_MINOR, "Initializing..." );

	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Successfully Initialized!" );

	m_attractModeCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );
	m_attractModeCamera.m_mode = CameraMode::Orthographic;
	//g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );

	SubscribeEventCallbackFunction( "StartGameButtonClicked", OnUIStartGameButtonClicked );
	m_attractModeButtons.push_back( RectButton( AABB2( Vec2( 1250.f, 620.f ), Vec2( 1450.f, 700.f ) ), "ATTRACTSTARTGAME", Rgba8( 50, 152, 204 ), "None", "StartGameButtonClicked", "$(attract_start_game_button)" ) );

	SubscribeEventCallbackFunction( "LoadSavingsButtonClicked", OnUILoadSavingsButtonsClicked );
	m_attractModeButtons.push_back( RectButton( AABB2( Vec2( 1250.f, 490.f ), Vec2( 1450.f, 570.f ) ), "ATTRACTLOADGAME", Rgba8( 50, 152, 204 ), "None", "LoadSavingsButtonClicked", "$(attract_load_game_button)" ) );

	// credit?

	SubscribeEventCallbackFunction( "ExitGameButtonClicked", SetQuitting );
	m_attractModeButtons.push_back( RectButton( AABB2( Vec2( 1250.f, 360.f ), Vec2( 1450.f, 440.f ) ), "ATTRACTENDGAME", Rgba8( 50, 152, 204 ), "None", "ExitGameButtonClicked", "$(attract_exit_game_button)" ) );
	
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
	DebugRenderSystemShutdown();

	delete g_theGame;
	delete g_theRenderer;
	delete g_theInput;
	delete g_theAudio;
	delete g_window;
	delete g_theEventSystem;
	delete g_chineseFont;

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
	if (!m_toAttractMode && !m_attractMode && (g_theInput->WasKeyJustPressed( 0x50 ) || g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::XBOX_BUTTON_START ))) // P key pause the game; handle the pause problem
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
		if (g_theGame) {
			if (m_isPaused) {
				m_isPaused = false;
				ToAttractMode();
			}
			else {
				m_isPaused = true;
			}
		}
	}

/*
	if (g_theInput->WasKeyJustPressed( KEYCODE_SPACE )
		&& m_attractMode)
	{
		m_attractMode = false;
		g_theAudio->StopSound( m_attractModeMusic );
		g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Enter Game" );
		g_theGame = new Game();
		g_theGame->Startup();
		g_gameMode = GameMode::CHOOSE_FORCES;
	}
	*/
	if (g_theInput->WasKeyJustPressed( KEYCODE_ESC ) && m_attractMode) {
		m_isQuitting = true;
	}
}

void App::UpdateAttractMode( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	for (auto& button : m_attractModeButtons) {
		button.BeginFrame();
	}

	Vec2 mouseWorldPos = g_window->GetNormalizedCursorPos();
	Vec2 cursorScreenPosition = m_attractModeCamera.GetCursorWorldPosition( mouseWorldPos );
	for (auto& button : m_attractModeButtons) {
		if (button.IsPointInside( cursorScreenPosition )) {
			button.OnCursorHover();
		}
	}

	if (g_theInput->WasKeyJustReleased( KEYCODE_LEFTMOUSE )) {
		for (auto& button : m_attractModeButtons) {
			if (button.IsPointInside( cursorScreenPosition )) {
				EventArgs args;
				button.OnButtonReleased( args );
			}
		}
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
	DebugRenderBeginFrame();
}

void App::Update( float deltaSeconds ) {
	//m_timePassed += deltaSeconds;

	if (m_toAttractMode) {
		g_theAudio->StopSound( g_theGame->m_backgroundMusic );
		m_toAttractMode = false;
		delete g_theGame;
		g_theGame = nullptr;
		m_attractMode = true;
		m_attractModeMusic = g_theAudio->StartSound( m_audioDictionary[(int)AudioName::AttractMode], false );
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

	if (m_attractMode) {
		UpdateAttractMode( deltaSeconds );
	}

	HandleKey();
}

void App::Render() const
{
	g_theRenderer->ClearScreen( Rgba8( 255, 255, 255 ) );

	if (!m_attractMode) {
		g_theGame->Render();
	}
	else {
		g_theRenderer->BeginCamera( m_attractModeCamera );
		RenderAttractMode();
		g_theRenderer->EndCamera( m_attractModeCamera );
	}

	if (g_devConsole->GetMode() == DevConsoleMode::EMERGE) {
		g_theRenderer->BeginCamera( m_attractModeCamera );
		g_devConsole->Render( m_attractModeCamera.m_cameraBox );
		g_theRenderer->EndCamera( m_attractModeCamera );
	}
}

void App::EndFrame() {
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
	//g_theRenderer->BindTexture( nullptr );
	//g_theRenderer->DrawVertexArray( 3, startIconVerts );

	for (auto& button : m_attractModeButtons) {
		button.Render();
	}

	//std::vector<Vertex_PCU> cnVerts;
	//g_chineseFont->AddVertsForText2D( cnVerts, Vec2( 100.f, 100.f ), 100.f, std::wstring( L"你好世界Abc" ), Rgba8( 0, 0, 0 ), 1.f );

	//g_theRenderer->BindTexture( &g_chineseFont->GetTexture() );
	//g_theRenderer->DrawVertexArray( cnVerts );

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
	m_audioDictionary[(int)AudioName::AttractMode] = g_theAudio->CreateOrGetSound( "Data/Music/0.mp3" );
	g_theAudio->CreateOrGetSound( "Data/Music/1.mp3" );
	g_theAudio->CreateOrGetSound( "Data/Music/2.mp3" );
	g_theAudio->CreateOrGetSound( "Data/Music/3.mp3" );
}

void App::SetUpTexture()
{
	g_ASCIIFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

	std::string filePath = "Data/Fonts/Alvin'sStupidChineseFont.png";
	Texture* bitMapTexture = g_theRenderer->CreateOrGetTextureFromFile( filePath.c_str() );
	filePath = filePath.substr( 0, filePath.size() - 4 );
	filePath += ".fnt";
	g_chineseFont = new SM_BitmapFont( filePath.c_str(), *bitMapTexture );
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

void App::SetUpLocalisations()
{
	std::string language = g_gameConfigBlackboard.GetValue( "language", "DEFAULT" );
	if (language == "Zh") {
		StartUpTranslation( SM_GameLanguage::ZH );
	}
	else if (language == "Zh-CN") {
		StartUpTranslation( SM_GameLanguage::ZH );
	}
	else if (language == "En") {
		StartUpTranslation( SM_GameLanguage::ENGLISH );
	}
}

void App::LoadSaveToCurGame( std::string const& path )
{
	// assume we have a game with loaded province history and in force choosing state
	if (!g_theGame) {
		return;
	}
	std::string fileStr;
	FileReadToString( fileStr, path );
	Strings fileStrs;
	int numOfLines = SplitStringOnDelimiter( fileStrs, fileStr, '\n' );
	int curReadingState = 0;
	Force* curReadingForce = nullptr;
	// 0 game 1 province 2 force 3 army 4 ai
	for (int i = 0; i < numOfLines; i++) {
		std::string& curLine = fileStrs[i];
		if (curReadingState == 0) {
			// read game information
			if (curLine == "Game") {
				continue;
			}
			else if (curLine == "Provinces") {
				curReadingState = 1;
				continue;
			}
			else {
				Strings elements = SplitStringOnDelimiter( curLine );
				g_theGame->m_roundCount = atoi( elements[1].c_str() );
				Force* lastPlayForce = g_theGame->m_map->GetForceByNickName( elements[0] );
				g_theGame->m_playerChooseForceVector[0] = lastPlayForce->m_id;
				g_theGame->m_onInspectForce = lastPlayForce;
				g_theGame->m_cameraCenter = lastPlayForce->GetCapitalProv()->GetCenter();
			}
		}
		else if (curReadingState == 1) {
			// read provinces
			if (curLine == "Forces") {
				curReadingState = 2;
				continue;
			}
			else {
				Strings elements = SplitStringOnDelimiter( curLine );
				// save the provinces
				/*
				int m_id = -1;
				int m_economy = 1;
				float m_population = 1000;
				Force* m_owner = nullptr;
				Army* m_armyOn = nullptr;
				std::vector<Force*> m_legalForces;

				float m_defenseRate = 1000.f;
				int m_developmentRate = 0;
				float m_populationGrowthRate = 0.001f;
				bool m_isAttractingPopulation = false;
				float m_huhuaness = 0.f;

				float m_legalProgress;
				float m_maxLegalProgress;
				bool m_legalIsAddedThisTurn = false;
				*/
				int id = atoi( elements[0].c_str() );
				Province* prov = g_theGame->m_map->m_provinces[id];
				prov->m_economy = atoi( elements[1].c_str() );
				prov->m_population = (float)atof( elements[2].c_str() );
				prov->m_owner->LoseProvince( prov );
				prov->m_owner = g_theGame->m_map->GetForceByNickName( elements[3] );
				prov->m_owner->GainProvince( prov );
				std::string legalForcesStr = elements[4].substr( 1, elements[4].length() - 2 );
				Strings legalForceStrs = SplitStringOnDelimiter( legalForcesStr, '|' );
				for (auto str : legalForceStrs) {
					prov->AddLegalForce( g_theGame->m_map->GetForceByNickName( str ) );
				}
				prov->m_defenseRate = (float)atof( elements[5].c_str() );
				prov->m_developmentRate = atoi( elements[6].c_str() );
				prov->m_populationGrowthRate = (float)atof( elements[7].c_str() );
				prov->m_isAttractingPopulation = atoi( elements[8].c_str() );
				prov->m_huhuaness = (float)atof( elements[9].c_str() );
				prov->m_legalProgress = (float)atof( elements[10].c_str() );
				prov->m_maxLegalProgress = (float)atof( elements[11].c_str() );
				prov->m_legalIsAddedThisTurn = atoi( elements[12].c_str() );
			}
		}
		else if (curReadingState == 2) {
			// save the forces
			/*
			int m_id = -1;
			std::vector<Province*> m_ownedProvs;
			//std::vector<Army*> m_armies;
			Province* m_capitalProv = nullptr;
			
			int m_commandPointAmount = 0;
			
			StupidMonarchAI* m_ai = nullptr;
			*/
			if (curLine._Starts_with( "force:" )) {
				curLine = curLine.substr( 6 );
				Strings elements = SplitStringOnDelimiter( curLine );
				curReadingForce = g_theGame->m_map->GetForceByNickName( elements[0] );

				curReadingForce->m_capitalProv = g_theGame->m_map->m_provinces[atoi( elements[1].c_str() )];
				curReadingForce->m_commandPointAmount = atoi( elements[2].c_str() );
				curReadingForce->m_maxArmyAmount = atoi( elements[3].c_str() );

				for (auto army : curReadingForce->m_armies) {
					delete army;
				}
				curReadingForce->m_armies.clear();
			}
			else {
				curReadingState = 3;
				i--;
			}
		}
		else if (curReadingState == 3) {
			if (curLine._Starts_with( "army:" )) {
				// save the armies
				/*
				Force* m_owner;
				Province* m_inProvince;
				int m_maxSize = 100;
				int m_size = 1;
				*/
				curLine = curLine.substr( 5 );
				Strings elements = SplitStringOnDelimiter( curLine );
				Army* army = new Army( curReadingForce, g_theGame->m_map->m_provinces[atoi( elements[0].c_str() )], atoi( elements[1].c_str() ), atoi( elements[2].c_str() ) );
				curReadingForce->m_armies.push_back( army );
			}
			else if (curLine._Starts_with( "ai:" )) {
				curReadingState = 4;
				i--;
			}
		}
		else if (curReadingState == 4) {
			if (curLine._Starts_with( "ai:" )) {
				/*
				AIPersonality m_personality = AIPersonality::COUNT;
				float m_difficulty = 1.f;
				Force* m_targetEnemy;
				*/
				curLine = curLine.substr( 3 );
				Strings elements = SplitStringOnDelimiter( curLine );
				if (curReadingForce->m_ai) {
					curReadingForce->m_ai->m_personality = (AIPersonality)atoi( elements[0].c_str() );
					curReadingForce->m_ai->m_difficulty = (float)atof( elements[1].c_str() );
					curReadingForce->m_ai->m_targetEnemy = g_theGame->m_map->GetForceByNickName( elements[2] );
				}
			}
			else if (curLine._Starts_with( "force:" )) {
				curReadingState = 2;
				i--;
			}
		}
	}
}

void App::CreateSave( std::string const& folder, std::string const& name )
{
	// save g_theGame
	FILE* file = nullptr;
	errno_t errNo = fopen_s( &file, (folder + name).c_str(), "wb" );
	if (errNo != 0 || file == nullptr) {
		ERROR_AND_DIE( Stringf( "Cannot open file %s", (folder + name).c_str() ) );
	}

	fprintf( file, "Game\n" );

	// save game variables
	/*
	Force* m_curForce = nullptr; // continue game from this
	// curForce to be choosing force
	int m_numOfPlayers = 1;
	int m_curChoosingPlayer = 0;
	std::vector<int> m_playerChooseForceVector;
	Force* m_onInspectForce = nullptr;
	// round count
	int m_roundCount = 1;
	*/
	fprintf( file, "%s,%d\n", g_theGame->m_curForce->m_nickName.c_str(), g_theGame->m_roundCount );

	// save the map
	/*
	// save the provinces
	std::vector<Province*> m_provinces;
	// save the forces
	std::vector<Force*> m_forcesAsOrder;
	*/

	// save the provinces
	/*
	int m_id = -1;
	int m_economy = 1;
	float m_population = 1000;
	Force* m_owner = nullptr;
	std::vector<Force*> m_legalForces;

	float m_defenseRate = 1000.f;
	int m_developmentRate = 0;
	float m_populationGrowthRate = 0.001f;
	bool m_isAttractingPopulation = false;
	float m_huhuaness = 0.f;

	float m_legalProgress;
	float m_maxLegalProgress;
	bool m_legalIsAddedThisTurn = false;
	*/
	fprintf( file, "Provinces\n" );
	for (auto prov : g_theGame->m_map->m_provinces) {
		if (!prov->m_owner) {
			continue;
		}
		fprintf( file, "%d,%d,%f,%s,[", prov->m_id, prov->m_economy, prov->m_population, prov->m_owner->m_nickName.c_str() );
		for (int i = 0; i < (int)prov->m_legalForces.size(); i++) {
			if (i != (int)prov->m_legalForces.size() - 1) {
				fprintf( file, "%s|", prov->m_legalForces[i]->m_nickName.c_str() );
			}
			else {
				fprintf( file, "%s", prov->m_legalForces[i]->m_nickName.c_str() );
			}
		}
		fprintf( file, "],%f,%d,%f,%d,%f,%f,%f,%d\n", prov->m_defenseRate, prov->m_developmentRate, prov->m_populationGrowthRate, prov->m_isAttractingPopulation,
			prov->m_huhuaness, prov->m_legalProgress, prov->m_maxLegalProgress, prov->m_legalIsAddedThisTurn );
	}

	// save the forces
	/*
	int m_id = -1;
	std::vector<Province*> m_ownedProvs;
	//std::vector<Army*> m_armies;
	Province* m_capitalProv = nullptr;

	int m_commandPointAmount = 0;

	StupidMonarchAI* m_ai = nullptr;
	*/
	fprintf( file, "Forces\n" );
	for (auto force : g_theGame->m_map->m_forcesAsOrder) {
		if (!force) {
			continue;
		}
		fprintf( file, "force:%s,%d,%d,%d\n", force->m_nickName.c_str(), force->m_capitalProv->m_id, force->m_commandPointAmount, force->m_maxArmyAmount );
		// save the armies
		/*
		Force* m_owner;
		Province* m_inProvince;
		int m_maxSize = 100;
		int m_size = 1;
		*/
		for (auto army : force->m_armies) {
			fprintf( file, "army:%d,%d,%d\n", army->GetProvinceIn()->m_id, army->GetMaxSize(), army->GetSize() );
		}
		// save the AI
		/*
		AIPersonality m_personality = AIPersonality::COUNT;
		float m_difficulty = 1.f;
		Force* m_targetEnemy;
		*/
		if (force->m_ai) {
			fprintf( file, "ai:%d,%f,", (int)force->m_ai->m_personality, force->m_ai->m_difficulty );
			if (force->m_ai->m_targetEnemy) {
				fprintf( file, "%s\n", force->m_ai->m_targetEnemy->m_nickName.c_str() );
			}
			else {
				fprintf( file, "\n" );
			}
		}
	}
	fclose( file );
}

bool App::SetQuitting( EventArgs& args )
{
	UNUSED( args );
	g_theApp->m_isQuitting = true;
	return true;
}

bool App::OnUIStartGameButtonClicked( EventArgs& args )
{
	UNUSED( args );
	g_theApp->m_attractMode = false;
	//g_theAudio->StopSound( g_theApp->m_attractModeMusic );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Enter Game" );
	g_theGame = new Game();
	g_theGame->Startup();
	g_theGame->GoToGameMode( GameMode::CHOOSE_FORCES );
	g_theGame->m_backgroundMusic = g_theApp->m_attractModeMusic;
	return true;
}

bool App::OnUILoadSavingsButtonsClicked( EventArgs& args )
{
	UNUSED( args );
	g_theApp->m_attractMode = false;
	//g_theAudio->StopSound( g_theApp->m_attractModeMusic );
	g_devConsole->AddLine( DevConsole::INFO_MAJOR, "Enter Game" );
	g_theGame = new Game();
	g_theGame->m_loadFromSave = true;
	g_theGame->Startup();
	g_theGame->GoToGameMode( GameMode::CHOOSE_FORCES );
	g_theApp->LoadSaveToCurGame( "./save.sm" );
	g_theGame->m_startForceFromSave = g_theGame->m_onInspectForce;
	g_theGame->m_backgroundMusic = g_theApp->m_attractModeMusic;
	return true;
}

