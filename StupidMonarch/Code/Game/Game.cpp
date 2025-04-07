#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Entity.hpp"
#include "Game/Map.hpp"
#include "Game/Force.hpp"
#include "Game/Province.hpp"
#include "Game/Army.hpp"
#include "Game/BattleReport.hpp"
#include "Game/TranslationUtils.hpp"
#include "Game/SM_BitMapFont.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"

GameMode g_gameMode = GameMode::CHOOSE_FORCES;
AABB2 const g_developButtonAABB2 = AABB2( Vec2( 50.f, 200.f ), Vec2( 150.f, 250.f ) );
AABB2 const g_attractButtonAABB2 = AABB2( Vec2( 50.f, 125.f ), Vec2( 150.f, 175.f ) );
AABB2 const g_defenseButtonAABB2 = AABB2( Vec2( 50.f, 50.f ), Vec2( 150.f, 100.f ) );
AABB2 const g_buildArmyButtonAABB2 = AABB2( Vec2( 75.f, 350.f ), Vec2( 175.f, 400.f ) );
AABB2 const g_addLEgalProgressButtonAABB2 = AABB2( Vec2( 225.f, 350.f ), Vec2( 325.f, 400.f ) );
AABB2 const g_saveButtonAABB2 = AABB2( Vec2( 1450.f, 250.f ), Vec2( 1550.f, 300.f ) );

AABB2 const g_firstYearButtonAABB2 = AABB2( Vec2( 30.f, 680.f ), Vec2( 260.f, 750.f ) );
AABB2 const g_startGameButtonAABB2 = AABB2( Vec2( 1350.f, 50.f ), Vec2( 1550.f, 130.f ) );
AABB2 const g_addPlayerButtonAABB2 = AABB2( Vec2( 1520.f, 710.f ), Vec2( 1550.f, 740.f ) );
AABB2 const g_subPlayerButtonAABB2 = AABB2( Vec2( 1400.f, 710.f ), Vec2( 1430.f, 740.f ) );
AABB2 const g_gotoNextPlayerButtonAABB2 = AABB2( Vec2( 1520.f, 660.f ), Vec2( 1550.f, 690.f ) );
AABB2 const g_gotoLastPlayerButtonAABB2 = AABB2( Vec2( 1400.f, 660.f ), Vec2( 1430.f, 690.f ) );


AABB2 const g_forceInfoAABB2 = AABB2( Vec2( 1300.f, 190.f ), Vec2( 1600.f, 610.f ) );
AABB2 const g_descriptionAABB2 = AABB2( Vec2( 300.f, 20.f ), Vec2( 1300.f, 200.f ) );

Game::Game()
{
	// load random number generator
	m_randNumGen = new RandomNumberGenerator();
}

Game::~Game()
{
	// delete all pointers managed by Game
	delete m_randNumGen;
	m_randNumGen = nullptr;
	delete m_map;
	m_map = nullptr;
}

void Game::Startup()
{
	// set up map
	LoadScenarios();
	m_map = new Map( this );
	m_map->StartUp( "Data/Map/map.png" );

	// set up camera
	m_screenCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );
	m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 1.f, -1.f );
	m_screenCamera.m_mode = CameraMode::Orthographic;
	m_worldCamera.m_mode = CameraMode::Orthographic;
	m_worldCamera.Scale2D( 3.f, 3.f );
	m_worldCameraScale = 3.f;
	//m_cameraCenter = Vec2( GetClamped( m_cameraCenter.x, WORLD_SIZE_X * 0.5f, m_map->GetDimensions().x - WORLD_SIZE_X * 0.5f ), GetClamped( m_cameraCenter.y, WORLD_SIZE_Y * 0.5f, m_map->GetDimensions().y - WORLD_SIZE_Y * 0.5f ) );
	m_worldCamera.SetCenter( m_cameraCenter );

	// set up buttons
	SubscribeEventCallbackFunction( "RankButtonClicked", OnRankButtonClicked );
	m_buttons.push_back( RectButton( AABB2( Vec2( 1450.f, 170.f ), Vec2( 1550.f, 220.f ) ), "RANK", Rgba8( 255, 255, 153 ), "None", "RankButtonClicked", "$(rank_button)" ) );
	SetButtonVisable( "RANK", false );
	SubscribeEventCallbackFunction( "AttractButtonClicked", OnAttractButtonClicked );
	m_buttons.push_back( RectButton( g_attractButtonAABB2, "ATR", Rgba8( 255, 255, 153 ), "None", "AttractButtonClicked", "$(ATR_button)" ) );
	SetButtonVisable( "ATR", false );
	SubscribeEventCallbackFunction( "DefenseButtonClicked", OnDefenseButtonClicked );
	m_buttons.push_back( RectButton( g_defenseButtonAABB2, "DEF", Rgba8( 255, 255, 153 ), "None", "DefenseButtonClicked", "$(DEF_button)" ) );
	SetButtonVisable( "DEF", false );
	SubscribeEventCallbackFunction( "DevelopButtonClicked", OnDevelopmentButtonClicked );
	m_buttons.push_back( RectButton( g_developButtonAABB2, "DEV", Rgba8( 255, 255, 153 ), "None", "DevelopButtonClicked", "$(DEV_button)" ) );
	SetButtonVisable( "DEV", false );
	SubscribeEventCallbackFunction( "ValidButtonClicked", OnValidButtonClicked );
	m_buttons.push_back( RectButton( g_addLEgalProgressButtonAABB2, "VLD", Rgba8( 255, 255, 153 ), "None", "ValidButtonClicked", "$(VLD_button)" ) );
	SetButtonVisable( "VLD", false );
	SubscribeEventCallbackFunction( "ArmyButtonClicked", OnBuildArmyButtonClicked );
	m_buttons.push_back( RectButton( g_buildArmyButtonAABB2, "ARM", Rgba8( 255, 255, 153 ), "None", "ArmyButtonClicked", "$(ARM_button)" ) );
	SetButtonVisable( "ARM", false );
	SubscribeEventCallbackFunction( "RecruitButtonClicked", OnRecruitButtonClicked );
	m_buttons.push_back( RectButton( g_developButtonAABB2, "RCT", Rgba8( 255, 255, 153 ), "None", "RecruitButtonClicked", "$(RCT_button)" ) );
	SetButtonVisable( "RCT", false );
	SubscribeEventCallbackFunction( "Recruit5ButtonClicked", OnRecruit5ButtonClicked );
	m_buttons.push_back( RectButton( g_attractButtonAABB2, "RCT5", Rgba8( 255, 255, 153 ), "None", "Recruit5ButtonClicked", "$(RCT5_button)" ) );
	SetButtonVisable( "RCT5", false );
	SubscribeEventCallbackFunction( "SaveButtonClicked", OnSaveButtonClicked );
	m_buttons.push_back( RectButton( g_saveButtonAABB2, "SAVE", Rgba8( 255, 255, 153 ), "None", "SaveButtonClicked", "$(SAVE_button)" ) );
	SetButtonVisable( "SAVE", false );

	if (!m_loadFromSave) {
		for (int i = 0; i < (int)m_scenarios.size(); i++) {
			SubscribeEventCallbackFunction( "YearButtonClicked", OnYearButtonClicked );
			m_buttons.push_back( RectButton( AABB2( g_firstYearButtonAABB2.m_mins - Vec2( 0.f, 100.f * i ), g_firstYearButtonAABB2.m_maxs - Vec2( 0.f, 100.f * i ) ),
				std::string( "YEAR" ) + std::to_string( i ), Rgba8( 255, 255, 153 ), "None", "YearButtonClicked", m_scenarios[i].m_name ) );
			SetButtonVisable( std::string( "YEAR" ) + std::to_string( i ), true );
		}
	}

	SubscribeEventCallbackFunction( "StartButtonClicked", OnPlayGameButtonClicked );
	m_buttons.push_back( RectButton( g_startGameButtonAABB2, "START", Rgba8( 255, 255, 153 ), "None", "StartButtonClicked", "$(START_button)" ) );
	SetButtonVisable( "START", true );
	SubscribeEventCallbackFunction( "AddPlayerButtonClicked", OnNumOfPlayerButtonClicked );
	m_buttons.push_back( RectButton( g_addPlayerButtonAABB2, "ADDPLY", Rgba8( 255, 255, 153 ), "None", "AddPlayerButtonClicked", "$(ADDPLY_button)" ) );
	SetButtonVisable( "ADDPLY", true );
	SubscribeEventCallbackFunction( "SubPlayerButtonClicked", OnNumOfPlayerButtonClicked );
	m_buttons.push_back( RectButton( g_subPlayerButtonAABB2, "SUBPLY", Rgba8( 255, 255, 153 ), "None", "SubPlayerButtonClicked", "$(SUBPLY_button)" ) );
	SetButtonVisable( "SUBPLY", true );
	SubscribeEventCallbackFunction( "NextPlayerButtonClicked", OnGotoNextPlayerButtonClicked );
	m_buttons.push_back( RectButton( g_gotoNextPlayerButtonAABB2, "NXTPLY", Rgba8( 255, 255, 153 ), "None", "NextPlayerButtonClicked", "$(NXTPLY_button)" ) );
	SetButtonVisable( "NXTPLY", true );
	SubscribeEventCallbackFunction( "PrevPlayerButtonClicked", OnGotoNextPlayerButtonClicked );
	m_buttons.push_back( RectButton( g_gotoLastPlayerButtonAABB2, "PRVPLY", Rgba8( 255, 255, 153 ), "None", "PrevPlayerButtonClicked", "$(PRVPLY_button)" ) );
	SetButtonVisable( "PRVPLY", true );

	m_playerChooseForceVector.push_back( -1 );

	/*for (auto force : m_map->m_forcesAsOrder) {
		if (!force->isAI()) {
			m_curForce = force;
			m_cameraCenter = m_curForce->GetCapitalProv()->GetCenter();
			break;
		}
	}*/
}

void Game::Update( float deltaTime )
{
	for (auto& button : m_buttons) {
		button.BeginFrame();
	}

	if (g_theAudio->IsSoundEnd( m_backgroundMusic )) {
		// #ToDo: hard code now
		int i = m_randNumGen->RollRandomIntInRange( 0, 8 );
		std::string musicPath = "Data/Music/" + std::to_string( i ) + ".mp3";
		m_backgroundMusic = g_theAudio->StartSound( g_theAudio->CreateOrGetSound( musicPath ) );
	}

	Vec2 mouseWorldPos = g_window->GetNormalizedCursorPos();
	int deltaValue = g_theInput->GetMouseWheelInput();
	if (deltaValue < 0 && m_worldCameraScale < 10.f) {
		//move up
		m_worldCameraScale *= GetClamped( 1.f + 10.f * deltaTime, 1.1f, 1.5f );
		//if (m_worldCameraScale <= 5.f) {
		m_worldCamera.Scale2D( GetClamped( 1.f + 10.f * deltaTime, 1.01f, 1.5f ), GetClamped( 1.f + 10.f * deltaTime, 1.01f, 1.5f ) );
		//}
		g_theInput->ConsumeMouseWheelInput();
	}
	else if (deltaValue > 0 && m_worldCameraScale > 0.5f) {
		m_cameraCenter += (mouseWorldPos - Vec2( 0.5f, 0.5f )) * deltaTime * 1500.f;
		m_worldCameraScale *= GetClamped( (1.f - 10.f * deltaTime), 0.5f, 0.9f );
		//if (m_worldCameraScale >= 0.5f) {
		m_worldCamera.Scale2D( GetClamped( (1.f - 10.f * deltaTime), 0.5f, 0.99f ), GetClamped( (1.f - 10.f * deltaTime), 0.5f, 0.99f ) );
		//}
		g_theInput->ConsumeMouseWheelInput();
	}
	constexpr float cameraMoveSpeed = 50.f;
	Vec2 movingDirection = Vec2( 0.f, 0.f );
	if (mouseWorldPos.x <= 0.f) {
		movingDirection -= Vec2( 1.f, 0.f );
	}
	if (mouseWorldPos.x >= 1.f) {
		movingDirection += Vec2( 1.f, 0.f );
	}
	if (mouseWorldPos.y <= 0.f) {
		movingDirection -= Vec2( 0.f, 1.f );
	}
	if (mouseWorldPos.y >= 1.f) {
		movingDirection += Vec2( 0.f, 1.f );
	}
	m_cameraCenter += movingDirection.GetNormalized() * deltaTime * m_scrollSpeed * cameraMoveSpeed;

	m_cameraCenter = Vec2( GetClamped( m_cameraCenter.x, WORLD_SIZE_X * 0.5f - 50.f, m_map->GetDimensions().x - WORLD_SIZE_X * 0.5f ), GetClamped( m_cameraCenter.y, WORLD_SIZE_Y * 0.5f, m_map->GetDimensions().y - WORLD_SIZE_Y * 0.5f ) );
	m_worldCamera.SetCenter( m_cameraCenter );

	if (g_gameMode == GameMode::ARMY_FIGHT_INSPECT) {
		m_cameraCenter = m_battleReports[0]->GetPosition();
	}

	if (g_theInput->WasKeyJustPressed( KEYCODE_ENTER )) {
		NextTurn();
	}

	if (g_theInput->WasKeyJustPressed( KEYCODE_F6 ) && g_gameMode == GameMode::VIEW_MAP) {
		if (m_map->m_viewMode == MapViewMode::VIEW_ECONOMY_MAP) {
			m_map->m_viewMode = MapViewMode::VIEW_FORCE_MAP;
		}
		else if (m_map->m_viewMode == MapViewMode::VIEW_FORCE_MAP) {
			m_map->m_viewMode = MapViewMode::VIEW_ECONOMY_MAP;
		}
	}
	Vec2 cursorScreenPosition = m_screenCamera.GetCursorWorldPosition( mouseWorldPos );
	for (auto& button : m_buttons) {
		if (button.IsPointInside( cursorScreenPosition )) {
			button.OnCursorHover();
		}
	}

	if (g_gameMode == GameMode::VIEW_MAP) {
		if (m_onInspectProv) {
			if (m_curForce->m_commandPointAmount >= developmentCost && m_curForce == m_onInspectProv->m_owner && !m_onInspectProv->IsSiegedByEnemy()) {
				SetButtonDisable( "DEV", false );
			}
			else {
				SetButtonDisable( "DEV", true );
			}
			if (m_curForce->m_commandPointAmount >= attractCost && !m_onInspectProv->m_isAttractingPopulation && m_curForce == m_onInspectProv->m_owner && !m_onInspectProv->IsSiegedByEnemy()) {
				SetButtonDisable( "ATR", false );
			}
			else {
				SetButtonDisable( "ATR", true );
			}
			if (m_curForce->m_commandPointAmount >= defenseCost && m_onInspectProv->m_defenseRate < m_onInspectProv->m_maxDefenseRate && m_curForce == m_onInspectProv->m_owner && !m_onInspectProv->IsSiegedByEnemy()) {
				SetButtonDisable( "DEF", false );
			}
			else {
				SetButtonDisable( "DEF", true );
			}

			if (!m_onInspectProv->m_armyOn && ((m_curForce->GetArmyAmount() < m_curForce->m_maxArmyAmount && m_curForce->m_commandPointAmount >= 1) || m_curForce->m_commandPointAmount >= buildArmyCost) && m_curForce->isProvOwnedAndValid( m_onInspectProv ) && !m_onInspectProv->IsSiegedByEnemy()) {
				SetButtonDisable( "ARM", false );
			}
			else {
				SetButtonDisable( "ARM", true );
			}

			if (m_curForce->m_commandPointAmount >= addLegalProgressCost && !m_onInspectProv->m_legalIsAddedThisTurn && !m_onInspectProv->IsLegal() && m_curForce == m_onInspectProv->m_owner && !m_onInspectProv->IsSiegedByEnemy()) {
				SetButtonDisable( "VLD", false );
			}
			else {
				SetButtonDisable( "VLD", true );
			}
		}
	}
	else if (g_gameMode == GameMode::CLICK_ARMY) {
		if (m_curForce->m_commandPointAmount >= 1 && m_choosingArmy->GetSize() < m_choosingArmy->GetMaxSize() && m_curForce->isProvOwnedAndValid( m_choosingArmy->GetProvinceIn() )) {
			SetButtonDisable( "RCT", false );
		}
		else {
			SetButtonDisable( "RCT", true );
		}
		if (m_curForce->m_commandPointAmount >= 5 && m_choosingArmy->GetSize() <= m_choosingArmy->GetMaxSize() - 5 && m_curForce->isProvOwnedAndValid( m_choosingArmy->GetProvinceIn() )) {
			SetButtonDisable( "RCT5", false );
		}
		else {
			SetButtonDisable( "RCT5", true );
		}
	}
	else if (g_gameMode == GameMode::CHOOSE_FORCES) {
		if (m_numOfPlayers == 1) {
			SetButtonDisable( "SUBPLY", true );
		}
		else {
			SetButtonDisable( "SUBPLY", false );
		}
		if (CanGameStart()) {
			SetButtonDisable( "START", false );
		}
		else {
			SetButtonDisable( "START", true );
		}
	}

	if (g_theInput->WasKeyJustReleased( KEYCODE_LEFTMOUSE )) {
		Vec2 clickPos = m_worldCamera.GetCursorWorldPosition( mouseWorldPos );
		Vec2 clickPosInUI = cursorScreenPosition;
		if (g_gameMode == GameMode::VIEW_MAP) {
			Army* clickArmy = m_curForce->GetArmyAtWorldPos( clickPos );
			if (m_isShowingRank) {
				bool skipOther = false;
				for (auto& button : m_buttons) {
					if (button.IsPointInside( cursorScreenPosition )) {
						EventArgs args;
						button.OnButtonReleased( args );
						skipOther = true;
					}
				}
				if (IsPointInsideDisc2D( clickPosInUI, Vec2( 1100.f, 720.f ), 15.f ) && !skipOther) {
					g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Click.wav" ) );
					m_isShowingRank = false;
				}
			}
			else if (IsPointInsideDisc2D( clickPosInUI, Vec2( 1500.f, 100.f ), 45.f )) {
				g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Click.wav" ) );
				// next turn
				NextTurn();
			}
			// choose an army
			else if (clickArmy) {
				GoToGameMode( GameMode::CLICK_ARMY );
				g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Click.wav" ) );
				m_choosingArmy = clickArmy;
			}
			else if (m_onInspectProv) {
				if (IsPointInsideDisc2D( clickPosInUI, Vec2( 390.f, 590.f ), 15.f )) {
					g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Click.wav" ) );
					// close the tab
					m_onInspectProv = nullptr;
					for (auto& button : m_buttons) {
						button.SetVisible( false );
					}
					SetButtonVisable( "RANK", true );
					SetButtonVisable( "SAVE", true );
				}
				else if (IsPointInsideAABB2D( clickPosInUI, AABB2( Vec2( 0.f, 0.f ), Vec2( 400.f, 600.f ) ) )) {
					for (auto& button : m_buttons) {
						if (button.IsPointInside( cursorScreenPosition )) {
							EventArgs args;
							args.SetValue( "ID", std::to_string( m_onInspectProv->m_id ) );
							button.OnButtonReleased( args );
						}
					}
				}
				else {
					// inspect a new province
					g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Click.wav" ) );
					m_onInspectProv = m_map->GetProvinceByWorldPos( clickPos );
					if (m_onInspectProv == nullptr) {
						for (auto& button : m_buttons) {
							button.SetVisible( false );
						}
						SetButtonVisable( "RANK", true );
						SetButtonVisable( "SAVE", true );
					}
				}
			}
			else {
				bool skipOther = false;
				// click on rank
				for (auto& button : m_buttons) {
					if (button.IsPointInside( cursorScreenPosition )) {
						EventArgs args;
						button.OnButtonReleased( args );
						skipOther = true;
					}
				}
				// inspect a province
				if (!skipOther) {
					g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Click.wav" ) );
					m_onInspectProv = m_map->GetProvinceByWorldPos( clickPos );
					if (m_onInspectProv != nullptr) {
						for (auto& button : m_buttons) {
							button.SetVisible( false );
						}
						SetButtonVisable( "ATR", true );
						SetButtonVisable( "DEV", true );
						SetButtonVisable( "DEF", true );
						SetButtonVisable( "VLD", true );
						SetButtonVisable( "ARM", true );
						m_isShowingRank = false;
					}
				}
			}
		}
		else if (g_gameMode == GameMode::CLICK_ARMY) {
			if (IsPointInsideAABB2D( clickPosInUI, AABB2( Vec2( 0.f, 0.f ), Vec2( 400.f, 600.f ) ) )) {
				for (auto& button : m_buttons) {
					if (button.IsPointInside( cursorScreenPosition )) {
						EventArgs args;
						args.SetValue( "ID", std::to_string( m_curForce->GetArmyId( m_choosingArmy ) ) );
						button.OnButtonReleased( args );
					}
				}
			}
			else {
				g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Click.wav" ) );
				GoToGameMode( GameMode::VIEW_MAP );
			}
		}
		else if (g_gameMode == GameMode::ARMY_FIGHT_INSPECT) {
			if (IsPointInsideDisc2D( clickPosInUI, Vec2( 1300.f, 600.f ), 15.f )) {
				g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Click.wav" ) );
				delete m_battleReports[0];
				m_battleReports.erase( m_battleReports.begin() );
				if (m_battleReports.size() == 0) {
					GoToGameMode( GameMode::VIEW_MAP );
					//m_map->NextTurn();
				}
			}
		}
		else if (g_gameMode == GameMode::CHOOSE_FORCES) {
			bool passOther = false;
			for (auto& button : m_buttons) {
				if (button.IsPointInside( cursorScreenPosition )) {
					EventArgs args;
					if (button.m_name == "ADDPLY") {
						args.SetValue( "ADD", "true" );
					}
					else if (button.m_name == "SUBPLY") {
						args.SetValue( "ADD", "false" );
					}
					else if (button.m_name == "NXTPLY") {
						args.SetValue( "NXT", "true" );
					}
					else if (button.m_name == "PRVPLY") {
						args.SetValue( "NXT", "false" );
					}
					else if (button.m_name.find( "YEAR" ) != std::string::npos) {
						size_t res = button.m_name.find( "YEAR" );
						std::string num = button.m_name.substr( res + 4 );
						args.SetValue( "index", num );
					}
					button.OnButtonReleased( args );
					passOther = true;
				}
			}
			if (!passOther) {
				g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Click.wav" ) );
				Province* prov = m_map->GetProvinceByWorldPos( clickPos );
				if (IsPointInsideAABB2D(clickPosInUI, AABB2( Vec2( 1360.f, 630.f ), Vec2( 1600.f, 800.f ) ) ) || IsPointInsideAABB2D( clickPosInUI, g_descriptionAABB2) || (m_onInspectForce && IsPointInsideAABB2D(clickPosInUI, g_forceInfoAABB2)) ) {
					// do nothing
				}
				else if (prov && !IsForceChosenByPlayer( prov->m_owner )) {
					m_onInspectForce = prov->m_owner;
					m_playerChooseForceVector[m_curChoosingPlayer] = m_onInspectForce->m_id;
				}
				else if (!prov) {
					m_onInspectForce = nullptr;
					m_playerChooseForceVector[m_curChoosingPlayer] = -1;
				}
			}
		}
	}
	if (g_theInput->WasKeyJustPressed( KEYCODE_RIGHTMOUSE )) {
		Vec2 clickPos = m_worldCamera.GetCursorWorldPosition( mouseWorldPos );
		Province* prov = m_map->GetProvinceByWorldPos( clickPos );
		if (g_gameMode == GameMode::CLICK_ARMY) {
			if (m_choosingArmy->IsProvValidToGo( prov )) {
				g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/Click.wav" ) );
				if (prov != m_choosingArmy->GetProvinceIn()) {
					m_choosingArmy->SetProvToGo( prov );
				}
				else {
					m_choosingArmy->SetProvToGo( nullptr );
				}
				GoToGameMode( GameMode::VIEW_MAP );
			}
		}
	}

	m_map->Update( deltaTime );
	
}

void Game::Render() const
{
	// Game Camera
	g_theRenderer->BeginCamera( m_worldCamera );
	m_map->Render();
	g_theRenderer->EndCamera( m_worldCamera );

	// UI Camera
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> shapeVerts;
	textVerts.reserve( 10000 );
	g_theRenderer->BeginCamera( m_screenCamera );
	AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 1470.f, 760.f ), Vec2( 1600.f, 790.f ) ), 30.f, Stringf( "FPS:%i", g_theApp->m_framePerSecond ), Rgba8( 255, 0, 0 ), 0.6f, Vec2( 0.f, 0.5f ) );

	if (g_gameMode == GameMode::VIEW_MAP) {
		AddVertsForDisc2D( shapeVerts, Vec2( 1500.f, 100.f ), 45.f, Rgba8( 192, 192, 192 ) );
		AddVertsForArrow2D( shapeVerts, Vec2( 1470.f, 100.f ), Vec2( 1530.f, 100.f ), 20.f, 8.f, Rgba8( 255, 0, 0 ) );

		if (m_isShowingRank) {
			AddVertsForAABB2D( shapeVerts, AABB2( Vec2( 500.f, 50.f ), Vec2( 1100.f, 720.f ) ), Rgba8( 160, 160, 160, 128 ) );
			std::vector<Force*> forceRank;
			m_map->GetForceRank( forceRank );
			//g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2(), 15.f );
			for (int i = 0; i < (int)forceRank.size(); i++) {
				AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 600.f, 625.f - (float)i * 80.f ), Vec2( 1000.f, 705.f - (float)i * 80.f ) ), 40.f,
					Stringf( "%i $(force%d)  %.0f", i + 1, forceRank[i]->m_id, forceRank[i]->GetCorrectedEconomyPoint() * 0.001f + (float)forceRank[i]->GetTotalArmySize() ), Rgba8( 0, 0, 0 ), 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
			}
			AddVertsForExitButton( shapeVerts, Vec2( 1100.f, 720.f ), 15.f );
		}
		else if (m_onInspectProv) {
			AddVertsForAABB2D( shapeVerts, AABB2( Vec2( 0.f, 0.f ), Vec2( 400.f, 600.f ) ), Rgba8( 224, 224, 224 ), AABB2::IDENTITY );
			std::string titles = "\n";
			if (m_onInspectProv == m_onInspectProv->m_owner->GetCapitalProv()) {
				titles += "$(prov_info_capital) ";
			}
			if (m_onInspectProv->m_isMajorProv) {
				titles += "$(prov_info_major_prov) ";
			}
			if (m_onInspectProv->m_isPlain) {
				titles += "$(prov_info_plain) ";
			}
			if (m_onInspectProv->m_isMountain) {
				titles += "$(prov_info_mountain) ";
			}
			if (titles == "\n") {
				titles = "";
			}
			AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 20.f, 0.f ), Vec2( 400.f, 600.f ) ), 20.f,
				Stringf( "$(prov%d) id:%d %s\n$(prov_owner):$(force%d)\n$(prov_capital):$(provcap%d)\n$(prov_economy):%d\n$(prov_population):%.0f\n$(prov_uncaltivated):%.0f", m_onInspectProv->m_id, m_onInspectProv->m_id, titles.c_str(), m_onInspectProv->m_owner->m_id, m_onInspectProv->m_id, m_onInspectProv->GetEconomy(), m_onInspectProv->GetPopulation(), m_onInspectProv->m_huhuaness * 100.f ),
				Rgba8( 0, 0, 0 ), 0.618f, Vec2( 0.f, 0.95f ), TextBoxMode::OVERRUN );
			AddVertsForExitButton( shapeVerts, Vec2( 390.f, 590.f ), 15.f );

			// development
			AddVertsForAABB2D( shapeVerts, AABB2( Vec2( 200.f, 225.f ), Vec2( 350.f, 250.f ) ), Rgba8( 128, 128, 128 ), AABB2::IDENTITY );
			AddVertsForAABB2D( shapeVerts, AABB2( Vec2( 200.f, 225.f ), Vec2( 200.f + 150.f * GetClampedZeroToOne( (float)m_onInspectProv->m_developmentRate / m_onInspectProv->m_maxDevelopmentRate ), 250.f ) ), Rgba8( 51, 255, 51 ), AABB2::IDENTITY );
			AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 200.f, 190.f ), Vec2( 350.f, 220.f ) ), 30.f, Stringf( "%d/%d", m_onInspectProv->m_developmentRate, m_onInspectProv->m_maxDevelopmentRate ), Rgba8( 0, 0, 0 ) );

			// defense
			AddVertsForAABB2D( shapeVerts, AABB2( Vec2( 200.f, 75.f ), Vec2( 350.f, 100.f ) ), Rgba8( 128, 128, 128 ), AABB2::IDENTITY );
			AddVertsForAABB2D( shapeVerts, AABB2( Vec2( 200.f, 75.f ), Vec2( 200.f + 150.f * GetClampedZeroToOne( (float)m_onInspectProv->m_defenseRate / m_onInspectProv->m_maxDefenseRate ), 100.f ) ), Rgba8( 51, 255, 51 ), AABB2::IDENTITY );
			AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 200.f, 40.f ), Vec2( 350.f, 70.f ) ), 30.f, Stringf( "%.0f/%.0f", m_onInspectProv->m_defenseRate, m_onInspectProv->m_maxDefenseRate ), Rgba8( 0, 0, 0 ) );

			// population growth
			AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 200.f, 125.f ), Vec2( 350.f, 175.f ) ), 30.f, Stringf( "%.1f%%", m_onInspectProv->m_populationGrowthRate * 100.f ), Rgba8( 0, 0, 0 ) );

			// legal progress
			AddVertsForAABB2D( shapeVerts, AABB2( Vec2( 50.f, 300.f ), Vec2( 350.f, 320.f ) ), Rgba8( 128, 128, 128 ), AABB2::IDENTITY );
			AddVertsForAABB2D( shapeVerts, AABB2( Vec2( 50.f, 300.f ), Vec2( 50.f + 300.f * m_onInspectProv->GetLegalProgress(), 320.f ) ), Rgba8( 51, 255, 51 ), AABB2::IDENTITY );
			if (!m_onInspectProv->IsLegal()) {
				AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 75.f, 425.f ), Vec2( 325.f, 450.f ) ), 20.f, "$(illegally_occupied)", Rgba8( 255, 0, 0 ) );
			}//ILLEGALLY OCCUPIED
			AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 50.f, 320.f ), Vec2( 350.f, 350.f ) ), 20.f, "$(legal_progress)", Rgba8( 0, 0, 0 ) );
		}
		AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 1410.f, 0.f ), Vec2( 1600.f, 50.f ) ), 30.f, Stringf( "$(round)%d", m_roundCount ), Rgba8( 0, 0, 0 ) );
	}
	else if (g_gameMode == GameMode::ARMY_FIGHT_INSPECT) {
		m_battleReports[0]->Render();
		AddVertsForExitButton( shapeVerts, Vec2( 1300.f, 600.f ), 15.f );
	}
	else if (g_gameMode == GameMode::CLICK_ARMY) {
		AddVertsForAABB2D( shapeVerts, AABB2( Vec2( 0.f, 0.f ), Vec2( 400.f, 600.f ) ), Rgba8( 224, 224, 224 ), AABB2::IDENTITY );
		AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 20.f, 0.f ), Vec2( 400.f, 600.f ) ), 20.f,
			Stringf( "$(army_owner):$(force%d)\n$(army_size):%d/%d\n$(army_uncultivated):%d", m_choosingArmy->GetOwner()->m_id, m_choosingArmy->GetSize(), m_choosingArmy->GetMaxSize(), m_choosingArmy->GetOwner()->GetHuhuaness() ),
			Rgba8( 0, 0, 0 ), 0.618f, Vec2( 0.f, 0.95f ), TextBoxMode::OVERRUN );
	}
	else if (g_gameMode == GameMode::CHOOSE_FORCES) {
		AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 1430.f, 700.f ), Vec2( 1520.f, 750.f ) ), 20.f, Stringf( "$(num_of_player) %d", m_numOfPlayers ), Rgba8( 0, 0, 0 ) );
		AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 1430.f, 650.f ), Vec2( 1520.f, 700.f ) ), 20.f, Stringf( "$(name_of_player)%d", m_curChoosingPlayer + 1 ), Rgba8( 0, 0, 0 ) );
		AddVertsForAABB2D( shapeVerts, AABB2( Vec2( 1360.f, 630.f ), Vec2( 1600.f, 800.f ) ), Rgba8( 224, 224, 224 ) );
		if (!m_loadFromSave) {
			AddVertsForAABB2D( shapeVerts, g_descriptionAABB2, Rgba8( 224, 224, 224 ) );
			AddVertsForTextPlaceHolder( textVerts, g_descriptionAABB2, 20.f, m_scenarios[m_curScenario].m_description, Rgba8( 0, 0, 0 ), 0.618f, Vec2( 0.f, 1.f ), TextBoxMode::AUTO_NEW_LINE );
		}
		if (m_onInspectForce) {
			AddVertsForAABB2D( shapeVerts, g_forceInfoAABB2, Rgba8( 224, 224, 224 ) );
			if (m_loadFromSave) {
				AddVertsForTextPlaceHolder( textVerts, g_forceInfoAABB2, 20.f,
					Stringf( "$(force%d)\n$(force_info_economy):%.0f\n$(force_info_military):%d\n$(force_info_population):%.0f\n$(force_info_action_points):%d\n$(force_info_uncultivated):%d\n",/*$(force_desc%d)*/
						m_onInspectForce->m_id, m_onInspectForce->GetCorrectedEconomyPoint() * 0.001f, m_onInspectForce->GetTotalArmySize(), m_onInspectForce->GetTotalPopulation(), m_onInspectForce->CalculateCommandPoint(), m_onInspectForce->GetHuhuaness(), m_onInspectForce->m_id ),
					Rgba8( 0, 0, 0 ), 0.618f, Vec2( 0.03f, 0.97f ), TextBoxMode::AUTO_NEW_LINE );
			}
			else {
				AddVertsForTextPlaceHolder( textVerts, g_forceInfoAABB2, 20.f,
					Stringf( "$(force%d)\n$(force_info_economy):%.0f\n$(force_info_military):%.0f\n$(force_info_population):%.0f\n$(force_info_action_points):%d\n$(force_info_uncultivated):%d\n",
						m_onInspectForce->m_id, m_onInspectForce->GetCorrectedEconomyPoint() * 0.001f, m_onInspectForce->GetTotalPopulation() / 8000.f, m_onInspectForce->GetTotalPopulation(), m_onInspectForce->CalculateCommandPoint(), m_onInspectForce->GetHuhuaness(), m_onInspectForce->m_id ),
					Rgba8( 0, 0, 0 ), 0.618f, Vec2( 0.03f, 0.97f ), TextBoxMode::AUTO_NEW_LINE );
			}
		}
	}

	if (m_curForce) {
		AddVertsForAABB2D( shapeVerts, AABB2( Vec2( 0.f, 765.f ), Vec2( 1600.f, 800.f ) ), Rgba8( 224, 224, 224 ), AABB2::IDENTITY );
		AddVertsForTextPlaceHolder( textVerts, AABB2( Vec2( 0.f, 765.f ), Vec2( 1600.f, 800.f ) ), 30.f,
			Stringf( "$(force):$(force%d)   $(force_action_point):%d/%d   $(force_army_num):%d/%d   $(force_uncultivated):%d/100", m_curForce->m_id,
				m_curForce->GetCommandPoint(), m_curForce->GetMaxCommandPoint(), m_curForce->GetArmyAmount(), m_curForce->GetMaxArmyAmount(),
				m_curForce->GetHuhuaness() ),
			Rgba8( 0, 0, 0 ), 0.618f, Vec2( 0.01f, 0.5f ) );
	}

	
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( shapeVerts );
	if (g_gameLanguage == SM_GameLanguage::ENGLISH) {
		g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	}
	else if (g_gameLanguage == SM_GameLanguage::ZH) {
		g_theRenderer->BindTexture( &g_chineseFont->GetTexture() );
	}		
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );

	for (auto& button : m_buttons) {
		button.Render();
	}
	g_theRenderer->EndCamera( m_screenCamera );

	DebugRenderScreen( m_screenCamera );
}

void Game::SetCurrentForce( Force* playerForce )
{
	m_curForce = playerForce;
	if (playerForce->m_nickName != g_gameConfigBlackboard.GetValue( "observerForce", "OB" )) {
		m_cameraCenter = m_curForce->GetCapitalProv()->GetCenter();
	}
}

Force* Game::GetCurrentForce() const
{
	return m_curForce;
}

AABB2 const& Game::GetCameraRangeOnMap() const
{
	return m_worldCamera.m_cameraBox;
}

Army const* Game::GetChoosingArmy() const
{
	return m_choosingArmy;
}

void Game::NextRound()
{
	m_roundCount++;
}

void Game::AddBattleReport( BattleReport* battleReport )
{
	GoToGameMode( GameMode::ARMY_FIGHT_INSPECT );
	m_battleReports.push_back( battleReport );
	m_cameraCenter = m_battleReports[m_battleReports.size() - 1]->GetPosition();
}

void Game::SetButtonVisable( std::string const& buttonName, bool isVisible )
{
	for (auto& button : m_buttons) {
		if (button.m_name == buttonName) {
			button.SetVisible( isVisible );
			return;
		}
	}
}

void Game::SetButtonDisable( std::string const& buttonName, bool isDisable )
{
	for (auto& button : m_buttons) {
		if (button.m_name == buttonName) {
			button.SetDisable( isDisable );
			return;
		}
	}
}

void Game::NextTurn()
{
	if (m_onInspectProv) {
		for (auto& button : m_buttons) {
			button.SetVisible( false );
		}
		SetButtonVisable( "RANK", true );
		SetButtonVisable( "SAVE", true );
	}
	m_isShowingRank = false;
	m_onInspectProv = nullptr;
	m_curForce->NextTurn();
	m_map->NextTurn();
}

bool Game::CanGameStart()
{
	for (auto i : m_playerChooseForceVector) {
		if (i == -1) {
			return false;
		}
	}
	return true;
}

void Game::LoadScenarios()
{
	// Load forces
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Map/ScenarioDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Loading Xml Document ScenarioDefinitions.xml failed!" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "ScenarioDefinitions" ), "Syntax Error! Name of the root of ScenarioDefinitions.xml should be \"ScenarioDefinitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "ScenarioDefinition" ), "Syntax Error! Names of the elements of ScenarioDefinitions.xml should be \"ScenarioDefinition\" " );
		Scenario s;
		s.m_year = ParseXmlAttribute( *xmlIter, "year", -1 );
		s.m_name = ParseXmlAttribute( *xmlIter, "name", "DEFAULT");
		s.m_description = ParseXmlAttribute( *xmlIter, "description", "DEFAULT" );
		m_scenarios.push_back( s );
		xmlIter = xmlIter->NextSiblingElement();
	}
}

void Game::LoadScenarioHistory( Scenario const& scenario )
{
	m_map->LoadHistory( scenario.m_year );
	if (m_onInspectForce && !m_onInspectForce->isAlive()) {
		m_onInspectForce = nullptr;
	}
	for (auto& i : m_playerChooseForceVector) {
		if (i != -1 && !m_map->m_forcesAsOrder[i]->isAlive()) {
			i = -1;
		}
	}
}

void Game::GoToGameMode( GameMode gameModeToGo )
{
	//GameMode curGameMode = g_gameMode;
	if (gameModeToGo == GameMode::ARMY_FIGHT_INSPECT) {
		m_onInspectProv = nullptr;
		for (auto& button : m_buttons) {
			button.SetVisible( false );
		}
	}
	else if (gameModeToGo == GameMode::CHOOSE_FORCES) {
		m_cameraCenter = m_map->GetForceByNickName( g_gameConfigBlackboard.GetValue( "centerForce", "XW" ) )->GetCapitalProv()->GetCenter();
	}
	else if (gameModeToGo == GameMode::CLICK_ARMY) {
		for (auto& button : m_buttons) {
			button.SetVisible( false );
		}
		SetButtonVisable( "RCT", true );
		SetButtonVisable( "RCT5", true );
		m_onInspectProv = nullptr;
		m_isShowingRank = false;
	}
	else if (gameModeToGo == GameMode::VIEW_MAP) {
		for (auto& button : m_buttons) {
			button.SetVisible( false );
		}
		SetButtonVisable( "RANK", true );
		SetButtonVisable( "SAVE", true );
	}
	g_gameMode = gameModeToGo;
}

bool Game::IsForceChosenByPlayer( Force* force ) const
{
	for (auto i : m_playerChooseForceVector) {
		if (force->m_id == i) {
			return true;
		}
	}
	return false;
}

bool OnAttractButtonClicked( EventArgs& args )
{
	int id = args.GetValue( "ID", -1 );
	Map* map = g_theGame->m_map;
	Force* force = g_theGame->m_curForce;
	if (id == -1) {
		return false;
	}
	if (force->m_commandPointAmount >= attractCost && !map->m_provinces[id]->m_isAttractingPopulation && force == map->m_provinces[id]->m_owner && !map->m_provinces[id]->IsSiegedByEnemy()) {
		force->m_commandPointAmount -= attractCost;
		map->m_provinces[id]->Attract();
		return true;
	}
	return false;
}

bool OnDefenseButtonClicked( EventArgs& args )
{
	int id = args.GetValue( "ID", -1 );
	Map* map = g_theGame->m_map;
	Force* force = g_theGame->m_curForce;
	if (id == -1) {
		return false;
	}
	if (force->m_commandPointAmount >= defenseCost && map->m_provinces[id]->m_defenseRate < map->m_provinces[id]->m_maxDefenseRate && force == map->m_provinces[id]->m_owner && !map->m_provinces[id]->IsSiegedByEnemy()) {
		force->m_commandPointAmount -= defenseCost;
		map->m_provinces[id]->Defense();
		return true;
	}
	return false;
}

bool OnDevelopmentButtonClicked( EventArgs& args )
{
	int id = args.GetValue( "ID", -1 );
	Map* map = g_theGame->m_map;
	Force* force = g_theGame->m_curForce;
	if (id == -1) {
		return false;
	}
	if (force->m_commandPointAmount >= developmentCost && force == map->m_provinces[id]->m_owner && !map->m_provinces[id]->IsSiegedByEnemy()){
		force->m_commandPointAmount -= developmentCost;
		map->m_provinces[id]->Develop();
		return true;
	}
	return false;
}

bool OnValidButtonClicked( EventArgs& args )
{
	int id = args.GetValue( "ID", -1 );
	Map* map = g_theGame->m_map;
	Force* force = g_theGame->m_curForce;
	if (id == -1) {
		return false;
	}
	if (force->m_commandPointAmount >= addLegalProgressCost && !map->m_provinces[id]->m_legalIsAddedThisTurn && !map->m_provinces[id]->IsLegal() && force == map->m_provinces[id]->m_owner && !map->m_provinces[id]->IsSiegedByEnemy()) {
		force->m_commandPointAmount -= addLegalProgressCost;
		map->m_provinces[id]->AddLegalProgress( addLegalProgressCost );
		return true;
	}
	return false;
}

bool OnBuildArmyButtonClicked( EventArgs& args )
{
	int id = args.GetValue( "ID", -1 );
	Map* map = g_theGame->m_map;
	Force* force = g_theGame->m_curForce;
	if (id == -1) {
		return false;
	}
	if (!map->m_provinces[id]->m_armyOn && force->isProvOwnedAndValid( map->m_provinces[id] ) && !map->m_provinces[id]->IsSiegedByEnemy()) {
		// build an army
		if (force->GetArmyAmount() < force->m_maxArmyAmount) {
			force->m_commandPointAmount -= 1;
			force->BuildAnNewArmy( map->m_provinces[id] );
		}
		else if (force->m_commandPointAmount >= buildArmyCost) {
			force->m_commandPointAmount -= (buildArmyCost + 1);
			force->BuildAnNewArmy( map->m_provinces[id] );
		}
		return true;
	}
	return false;
}

bool OnRankButtonClicked( EventArgs& args )
{
	UNUSED( args );
	g_theGame->m_isShowingRank = !g_theGame->m_isShowingRank;
	return true;
}

bool OnSaveButtonClicked( EventArgs& args )
{
	UNUSED( args );
	g_theApp->CreateSave( "./", "save.sm" );
	return true;
}

bool OnRecruitButtonClicked( EventArgs& args )
{
	int id = args.GetValue( "ID", -1 );
	Force* force = g_theGame->m_curForce;
	Army* army = force->m_armies[id];
	if (id == -1) {
		return false;
	}
	if (force->m_commandPointAmount >= 1 && army->GetSize() < army->GetMaxSize() && force->isProvOwnedAndValid( army->GetProvinceIn() )) {
		force->m_commandPointAmount -= 1;
		army->SetSize( army->GetSize() + 1 );
		return true;
	}
	return false;
}

bool OnRecruit5ButtonClicked( EventArgs& args )
{
	int id = args.GetValue( "ID", -1 );
	Force* force = g_theGame->m_curForce;
	Army* army = force->m_armies[id];
	if (id == -1) {
		return false;
	}
	if (force->m_commandPointAmount >= 5 && army->GetSize() <= army->GetMaxSize() - 5 && force->isProvOwnedAndValid( army->GetProvinceIn() )) {
		force->m_commandPointAmount -= 5;
		army->SetSize( army->GetSize() + 5 );
		return true;
	}
	return false;
}

bool OnYearButtonClicked( EventArgs& args )
{
	int index = args.GetValue( "index", -1 );
	g_theGame->m_curScenario = index;
	g_theGame->LoadScenarioHistory( g_theGame->m_scenarios[index] );
	return true;
}

bool OnNumOfPlayerButtonClicked( EventArgs& args )
{
	if (args.GetValue( "ADD", true )) {
		g_theGame->m_numOfPlayers += 1;
		g_theGame->m_playerChooseForceVector.push_back( -1 );
	}
	else {
		if (g_theGame->m_curChoosingPlayer == g_theGame->m_numOfPlayers - 1) {
			g_theGame->m_curChoosingPlayer -= 1;
		}
		g_theGame->m_numOfPlayers -= 1;
		g_theGame->m_playerChooseForceVector.pop_back();
	}
	return true;
}

bool OnGotoNextPlayerButtonClicked( EventArgs& args )
{
	if (args.GetValue( "NXT", true )) {
		g_theGame->m_curChoosingPlayer += 1;
		g_theGame->m_curChoosingPlayer = g_theGame->m_curChoosingPlayer % g_theGame->m_numOfPlayers;
	}
	else {
		g_theGame->m_curChoosingPlayer += (g_theGame->m_numOfPlayers - 1);
		g_theGame->m_curChoosingPlayer = g_theGame->m_curChoosingPlayer % g_theGame->m_numOfPlayers;
	}
	if (g_theGame->m_playerChooseForceVector[g_theGame->m_curChoosingPlayer] != -1) {
		g_theGame->m_onInspectForce = g_theGame->m_map->m_forcesAsOrder[g_theGame->m_playerChooseForceVector[g_theGame->m_curChoosingPlayer]];
		g_theGame->m_cameraCenter = g_theGame->m_map->m_forcesAsOrder[g_theGame->m_playerChooseForceVector[g_theGame->m_curChoosingPlayer]]->GetCapitalProv()->GetCenter();
	}
	else {
		g_theGame->m_onInspectForce = nullptr;
	}
	return true;
}

bool OnPlayGameButtonClicked( EventArgs& args )
{
	UNUSED( args );
	for (auto i : g_theGame->m_playerChooseForceVector) {
		g_theGame->m_map->m_forcesAsOrder[i]->SetAsPlayer( true );
	}
	for (auto force : g_theGame->m_map->m_forcesAsOrder) {
		if (force) {
			force->StartUp( g_theGame->m_loadFromSave );
		}
	}
	int startForceIndex = 0;
	if (g_theGame->m_loadFromSave) {
		startForceIndex = g_theGame->m_startForceFromSave->m_id;
	}
	if (!g_theGame->m_map->m_forcesAsOrder[startForceIndex]->isAI() && g_theGame->m_map->m_forcesAsOrder[startForceIndex]->isAlive()) {
		g_theGame->m_curForce = g_theGame->m_map->m_forcesAsOrder[startForceIndex];
		g_theGame->m_map->m_curForceIndex = startForceIndex;
		g_theGame->m_cameraCenter = g_theGame->m_curForce->GetCapitalProv()->GetCenter();
	}
	else {
		g_theGame->m_map->m_curForceIndex = startForceIndex - 1;
		g_theGame->m_map->NextTurn();
	}
	if ((int)g_theGame->m_battleReports.size() > 0) {
		g_theGame->GoToGameMode( GameMode::ARMY_FIGHT_INSPECT );
	}
	else {
		g_theGame->GoToGameMode( GameMode::VIEW_MAP );
	}
	return true;
}
