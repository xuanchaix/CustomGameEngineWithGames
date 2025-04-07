#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Culture.hpp"
#include "Game/Religion.hpp"
#include "Game/City.hpp"
#include "Game/Country.hpp"
#include "Game/Region.hpp"
#include "Game/Army.hpp"
#include <filesystem>

Game::Game()
{
	// load random number generator
	m_randNumGen = new RandomNumberGenerator();
	m_gameClock = new Clock();
	m_loadingTimer = new Timer( 2.f );
}

Game::~Game()
{
	// delete all pointers managed by Game
	delete m_randNumGen;
	m_randNumGen = nullptr;
	delete m_map;
}

void Game::Startup()
{
	// set up camera
	m_screenCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );
	m_screenCamera.m_mode = CameraMode::Orthographic;
	m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 1.f, -1.f );
	m_worldCamera.m_mode = CameraMode::Orthographic;
	m_cameraCenter = m_worldCamera.GetCenter();

	g_productNameMap.reserve( (size_t)ProductType::Num );
	g_productNameMap.push_back( "Fish" );
	g_productNameMap.push_back( "Grain" );
	g_productNameMap.push_back( "Fruit" );
	g_productNameMap.push_back( "Sugar" );
	g_productNameMap.push_back( "Salt" );
	g_productNameMap.push_back( "Livestock" );
	g_productNameMap.push_back( "Wax" );
	g_productNameMap.push_back( "Fur" );
	g_productNameMap.push_back( "Wood" );
	g_productNameMap.push_back( "Ivory" );
	g_productNameMap.push_back( "Cotton" );
	g_productNameMap.push_back( "Wool" );
	g_productNameMap.push_back( "Iron" );
	g_productNameMap.push_back( "Gold" );
	g_productNameMap.push_back( "Copper" );
	g_productNameMap.push_back( "Silver" );
	g_productNameMap.push_back( "Wine" );
	g_productNameMap.push_back( "Spice" );
	g_productNameMap.push_back( "Tea" );
	g_productNameMap.push_back( "Coffee" );
	g_productNameMap.push_back( "Tobacco" );
	g_productNameMap.push_back( "Glass" );
	g_productNameMap.push_back( "Silk" );
	g_productNameMap.push_back( "Jade" );
	g_productNameMap.push_back( "Porcelain" );
	g_productNameMap.push_back( "Cloth" );
	g_productNameMap.push_back( "Gem" );
	g_productNameMap.push_back( "WarHorse" );
	g_productNameMap.push_back( "Sword" );
	g_productNameMap.push_back( "Num" );
	g_productNameMap.push_back( "Nothing" );

	g_climateNameMap.reserve( (size_t)ClimateType::Num );
	g_climateNameMap.push_back( "Tropical Rainforest" );
	g_climateNameMap.push_back( "Tropical Monsoon" );
	g_climateNameMap.push_back( "Tropical Savanna" );
	g_climateNameMap.push_back( "Hot Desert" );
	g_climateNameMap.push_back( "Cold Desert" );
	g_climateNameMap.push_back( "Mediterranean Climate" );
	g_climateNameMap.push_back( "Humid Subtropical" );
	g_climateNameMap.push_back( "Oceanic" );
	g_climateNameMap.push_back( "Humid Monsoon" );
	g_climateNameMap.push_back( "Humid Continental" );
	g_climateNameMap.push_back( "Subarctic" );
	g_climateNameMap.push_back( "Tundra" );
	g_climateNameMap.push_back( "Ice Cap" );
	g_climateNameMap.push_back( "Water" );
	g_climateNameMap.push_back( "None" );
	g_climateNameMap.push_back( "Num" );

	g_landformNameMap.reserve( (size_t)LandformType::NUM );
	g_landformNameMap.push_back( "Ocean" );
	g_landformNameMap.push_back( "Land" );
	g_landformNameMap.push_back( "Lake" );
	g_landformNameMap.push_back( "Island" );
	g_landformNameMap.push_back( "Lowland Plain" );
	g_landformNameMap.push_back( "Highland Plain" );
	g_landformNameMap.push_back( "Grassland" );
	g_landformNameMap.push_back( "Savanna" );
	g_landformNameMap.push_back( "Marsh" );
	g_landformNameMap.push_back( "Rainforest" );
	g_landformNameMap.push_back( "Temperate Forest" );
	g_landformNameMap.push_back( "Subarctic Forest" );
	g_landformNameMap.push_back( "Subtropical Forest" );
	g_landformNameMap.push_back( "Lowland Hill" );
	g_landformNameMap.push_back( "Highland Hill" );
	g_landformNameMap.push_back( "Mountain" );
	g_landformNameMap.push_back( "Icefield" );
	g_landformNameMap.push_back( "Tundra" );
	g_landformNameMap.push_back( "Desert" );
	g_landformNameMap.push_back( "Town" );

	g_cultureOriginNameMap.reserve( (size_t)CultureGeoOrigin::Num );
	/*RiverOrigin, MountainOrigin, OceanOrigin, GrasslandOrigin, ForestOrigin, Num,*/
	g_cultureOriginNameMap.push_back( "RiverOrigin" );
	g_cultureOriginNameMap.push_back( "MountainOrigin" );
	g_cultureOriginNameMap.push_back( "OceanOrigin" );
	g_cultureOriginNameMap.push_back( "GrasslandOrigin" );
	g_cultureOriginNameMap.push_back( "ForestOrigin" );
	g_cultureOriginNameMap.push_back( "None" );

	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Manila Paper.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Photograph Frame.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Ornate Endpaper.png" );
	
}

void Game::Update()
{
	if (m_map && m_loadingTimer->HasStartedAndNotPeriodElapsed()) {
		float cameraScale = Maxf( m_map->m_dimensions.x / WORLD_SIZE_X, m_map->m_dimensions.y / WORLD_SIZE_Y ) * 1.1f;
		m_worldCamera.m_position.z = RangeMapClamped( SmoothStop2( m_loadingTimer->GetElapsedFraction() ), 0.f, 1.f, 10.f, 60.f ) * cameraScale;
		g_theGame->m_worldCameraScale = g_theGame->m_worldCamera.m_position.z;
		m_map->Update();
		if (m_isLabelDirty) {
			m_map->RefreshAllLabels();
		}
	}
	else {
		HandleKeys();
		UpdateMouseInput();
		if (m_map) {
			m_map->Update();
		}
		UpdateUI();
		if (m_isLabelDirty && m_map) {
			m_map->RefreshAllLabels();
		}
	}
}

void Game::Render() const
{
	// Game Camera
	if (m_viewMode == MapViewMode::ViewMode2D) {
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	}
	else {
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	}
	g_theRenderer->BeginCamera( m_worldCamera );
	if (m_map) {
		m_map->Render();
	}
	g_theRenderer->EndCamera( m_worldCamera );
	// UI Camera
	g_theRenderer->BeginCamera( m_screenCamera );
	RenderAttractScreen();

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

void Game::UpdateMouseInput()
{
	if (!m_map) {
		return;
	}
	if (m_viewMode == MapViewMode::ViewMode2D) {
		float deltaTime = Clock::GetSystemClock()->GetDeltaSeconds();
		Vec2 mouseWorldPos = g_window->GetNormalizedCursorPos();
		int deltaValue = g_theInput->GetMouseWheelInput();
		if (deltaValue < 0 && m_worldCameraScale < m_2DMaxWorldCameraHeight) {
			//move up
			m_worldCameraScale += m_2DScalingSpeed * deltaTime;
			//if (m_worldCameraScale <= 5.f) {
			m_worldCamera.m_position.z += m_2DScalingSpeed * deltaTime;
			//}
			g_theInput->ConsumeMouseWheelInput();
		}
		else if (deltaValue > 0 && m_worldCameraScale > m_2DMinWorldCameraHeight) {
			m_worldCameraScale -= m_2DScalingSpeed * deltaTime;
			//if (m_worldCameraScale >= 0.5f) {
			m_worldCamera.m_position.z -= m_2DScalingSpeed * deltaTime;
			//}
			g_theInput->ConsumeMouseWheelInput();
		}
		if (g_window && g_window->IsFocus()) {
			constexpr float cameraMoveSpeed = 40.f;
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
			m_worldCamera.m_position += movingDirection.GetNormalized() * deltaTime * m_scrollSpeed * cameraMoveSpeed * SmoothStop2( GetFractionWithinRange( m_worldCameraScale, 0.f, m_2DMaxWorldCameraHeight ) ) / 8.f;
		}
	}
	else if (m_viewMode == MapViewMode::ViewMode3D) { // 3D mode
		if (g_theInput->IsKeyDown( KEYCODE_RIGHTMOUSE )) {
			g_theInput->SetCursorMode( true, true );
			float deltaSeconds = g_theGame->m_gameClock->GetDeltaSeconds();
			float speed = 15.f;
			Vec3 iBasis, jBasis, kBasis;
			m_worldCamera.m_orientation.GetAsVectors_IFwd_JLeft_KUp( iBasis, jBasis, kBasis );

			if (g_theInput->IsKeyDown( 0x10/*Shift Key*/ )) {
				speed *= 5.f;
			}
			if (g_theInput->IsKeyDown( 'W' )) {
				m_worldCamera.m_position += iBasis * speed * deltaSeconds;
			}
			if (g_theInput->IsKeyDown( 'S' )) {
				m_worldCamera.m_position -= iBasis * speed * deltaSeconds;
			}
			if (g_theInput->IsKeyDown( 'A' )) {
				m_worldCamera.m_position += jBasis * speed * deltaSeconds;
			}
			if (g_theInput->IsKeyDown( 'D' )) {
				m_worldCamera.m_position -= jBasis * speed * deltaSeconds;
			}
			if (g_theInput->IsKeyDown( 'Z' )) {
				m_worldCamera.m_position -= Vec3( 0.f, 0.f, 15.f ) * speed * deltaSeconds;
			}
			if (g_theInput->IsKeyDown( 'C' )) {
				m_worldCamera.m_position += Vec3( 0.f, 0.f, 15.f ) * speed * deltaSeconds;
			}

			if (deltaSeconds != 0.f) {
				Vec2 cursorDisp = g_theInput->GetCursorClientDelta();
				m_worldCamera.m_orientation.m_yawDegrees -= 0.075f * cursorDisp.x * g_window->GetClientDimensions().x;
				m_worldCamera.m_orientation.m_pitchDegrees -= 0.075f * cursorDisp.y * g_window->GetClientDimensions().y;
			}

			m_worldCamera.m_orientation.m_pitchDegrees = GetClamped( m_worldCamera.m_orientation.m_pitchDegrees, -85.f, 85.f );
			m_worldCamera.m_orientation.m_rollDegrees = GetClamped( m_worldCamera.m_orientation.m_rollDegrees, -45.f, 45.f );
		}
		else {
			g_theInput->SetCursorMode( false, false );
		}
	}
	else if (m_viewMode == MapViewMode::ViewModeSphere) {
		float deltaSeconds = g_theGame->m_gameClock->GetDeltaSeconds();
		Vec2 mouseWorldPos = g_window->GetNormalizedCursorPos();
		int deltaValue = g_theInput->GetMouseWheelInput();
		if (deltaValue < 0 && m_worldCameraScale < m_2DMaxWorldCameraHeight) {
			//move up
			m_worldCameraScale += m_2DScalingSpeed * deltaSeconds * 1.5f;
			//if (m_worldCameraScale <= 5.f) {
			m_worldCamera.m_position.z += m_2DScalingSpeed * deltaSeconds * 1.5f;
			//}
			g_theInput->ConsumeMouseWheelInput();
		}
		else if (deltaValue > 0 && m_worldCameraScale > m_2DMinWorldCameraHeight) {
			m_worldCameraScale -= m_2DScalingSpeed * deltaSeconds * 1.5f;
			//if (m_worldCameraScale >= 0.5f) {
			m_worldCamera.m_position.z -= m_2DScalingSpeed * deltaSeconds * 1.5f;
			//}
			g_theInput->ConsumeMouseWheelInput();
		}
		if (g_theInput->IsKeyDown( KEYCODE_LEFTMOUSE )) {
			g_theInput->SetCursorMode( true, true );
			Vec2 delta = g_theInput->GetCursorClientDelta();
			m_map->m_sphereModeSphereZDegrees += delta.x * 90.f;
			//m_map->m_sphereModeSphereYDegrees -= delta.y * 90.f;
		}
		else {
			g_theInput->SetCursorMode( false, false );
		}
		float speed = 40.f;
		if (g_theInput->IsKeyDown( 'W' )) {
			m_worldCamera.m_position.y +=  speed * deltaSeconds;
		}
		if (g_theInput->IsKeyDown( 'S' )) {
			m_worldCamera.m_position.y -= speed * deltaSeconds;
		}
		if (g_theInput->IsKeyDown( 'A' )) {
			m_worldCamera.m_position.x -= speed * deltaSeconds;
		}
		if (g_theInput->IsKeyDown( 'D' )) {
			m_worldCamera.m_position.x +=  speed * deltaSeconds;
		}
	}
	if (m_viewMode == MapViewMode::ViewMode2D || m_viewMode == MapViewMode::ViewModeSphere) {
		m_worldCamera.m_position = Vec3( GetClamped( m_worldCamera.m_position.x, 0.f, m_map->GetDimensions().x ), GetClamped( m_worldCamera.m_position.y, 0.f, m_map->GetDimensions().y ), m_worldCamera.m_position.z );
	}
	//m_worldCamera.SetCenter( m_cameraCenter );
}

static void HelpMarker( const char* desc )
{
	ImGui::TextDisabled( "(?)" );
	if (ImGui::BeginItemTooltip())
	{
		ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
		ImGui::TextUnformatted( desc );
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void Game::UpdateUI()
{
	Update2DPopupUI();
	UpdateSpherePopupUI();
	Update3DPopupUI();
	UpdateControlPanelUI();
	UpdateLegendsUI();
	UpdateProvinceUI();
	UpdateCityUI();
	UpdateArmyUI();
	UpdateHistorySimulationUI();
	UpdateHistoryLogUI();
	UpdateSaveHistoryPopupUI();
}

void Game::GenerateNewMap( MapGenerationSettings const& settings )
{
	delete m_map;
	m_map = new Map( settings );
	m_map->Startup();
	m_loadingTimer->Start();
}

void Game::Update2DPopupUI()
{
	if (m_viewMode == MapViewMode::ViewMode2D && m_map) {
		if (!ImGui::Begin( "2D Mode", NULL, 0 ))
		{
			ImGui::End();
			return;
		}
		ImGui::Text( "Cursor go beyond border to Move" );
		ImGui::Text( "Left Click to Interact" );
		ImGui::End();
	}
}

void Game::UpdateSpherePopupUI()
{
	if (m_viewMode == MapViewMode::ViewModeSphere) {
		if (!ImGui::Begin( "Sphere Mode", NULL, 0 ))
		{
			ImGui::End();
			return;
		}
		ImGui::Text( "WASD to Move" );
		ImGui::Text( "Hold LMB to Rotate" );
		ImGui::Text( "Clicking and Editing are not supported in this mode." );
		ImGui::End();
	}
}

void Game::Update3DPopupUI()
{
	if (m_viewMode == MapViewMode::ViewMode3D) {
		if (!ImGui::Begin( "3D Mode", NULL, 0 ))
		{
			ImGui::End();
			goto L1;
		}
		ImGui::Text( "Hold RMB and Hold WASD to Move" );
		ImGui::Text( "Hold Shift to speed up" );
		ImGui::Text( "Left Click to Interact" );
		
		ImGui::End();
	}
	L1:
	if (m_viewMode == MapViewMode::ViewMode3D && (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_PROVINCE_EDIT)) {
		if (!ImGui::Begin( "3D Mode Province Owner Editor", NULL, 0 ))
		{
			ImGui::End();
			return;
		}
		ImGui::Text( "Sorry, currently this tool does not support to edit the owner of a province in 3D mode!" );
		ImGui::End();
	}
}

void Game::UpdateControlPanelUI()
{
	/*
	ImGuiID dockSpaceID = ImGui::GetID( "LeftDockspace" );
	ImGui::DockSpace( dockSpaceID, ImVec2( 100.f, 100.f ), ImGuiComboFlags_PopupAlignLeft | ImGuiComboFlags_HeightLargest );
	*/
	m_isLabelDirty = false;
	// Main body of the Demo window starts here.
	if (!ImGui::Begin( "Procedural Generated World", NULL, 0 ))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	// Most "big" widgets share a common width settings by default. See 'Demo->Layout->Widgets Width' for details.
	ImGui::PushItemWidth( ImGui::GetFontSize() * -16.f );           // e.g. Leave a fixed amount of width for labels (by passing a negative value), the rest goes to widgets.
	//ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.35f);   // e.g. Use 2/3 of the space for widgets and 1/3 for labels (right align)

	if (!m_map) {
		//ImGui::Text( "dear imgui says hello! (%s) (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM );
		//ImGui::Spacing();
		ImGui::InputInt( "Seed", (int*)&m_generationSettings.m_seed, 1 );
		ImGui::SameLine(); HelpMarker( "Seed is very important for generation. With the same seed (and other settings) you can generate the same world. 'Seed == 0' will produce a random seed." );
		ImGui::InputInt( "Basic Polygon Amount", &m_generationSettings.m_basePolygons, 10 );
		ImGui::SameLine(); HelpMarker( "Basic geometry unit in the world is polygons. If the map has more polygons, the world will be more detailed, but the generation speed will slow down." );
		ImGui::InputFloat2( "Map Dimensions", &m_generationSettings.m_dimensions.x );
		ImGui::SameLine(); HelpMarker( "Dimensions tell the generator how big the map canvas is. Dimensions will also influence the aspect of the map." );
		ImGui::Checkbox( "Enable History Simulation", &m_generationSettings.m_enableHistorySimulation );
		ImGui::SameLine(); HelpMarker( "Checking this will generate history by using AI simulation." );

		m_generationSettings.m_basePolygons = GetClamped( m_generationSettings.m_basePolygons, 200, 20000 );
		m_generationSettings.m_dimensions.x = GetClamped( m_generationSettings.m_dimensions.x, 100.f, MAX_SIZE_X );
		m_generationSettings.m_dimensions.y = GetClamped( m_generationSettings.m_dimensions.y, 100.f, MAX_SIZE_Y );

		if (ImGui::CollapsingHeader( "Advanced" )) {
			ImGui::InputFloat( "Min Height", &m_generationSettings.m_minHeight );
			ImGui::SameLine(); HelpMarker( "Decides the minimum height of the terrain, should be less than zero" );
			ImGui::InputFloat( "Max Height", &m_generationSettings.m_maxHeight );
			ImGui::SameLine(); HelpMarker( "Decides the maximum height of the terrain" );
			m_generationSettings.m_minHeight = GetClamped( m_generationSettings.m_minHeight, -10000000.f, 0.f );
			ImGui::InputFloat( "Fragment Factor", &m_generationSettings.m_fragmentFactor );
			ImGui::SameLine(); HelpMarker( "Decides how big the generated continent is, higher value means bigger continent" );
			m_generationSettings.m_fragmentFactor = GetClamped( m_generationSettings.m_fragmentFactor, 0.1f, 1.f );
			ImGui::InputFloat( "Ocean Factor", &m_generationSettings.m_landRichnessFactor );
			ImGui::SameLine(); HelpMarker( "Decides if the map has more Ocean, higher value means more ocean" );
			m_generationSettings.m_fragmentFactor = GetClamped( m_generationSettings.m_fragmentFactor, 0.1f, 0.9f );
			ImGui::InputInt( "Num of Provs to Form Island", &m_generationSettings.m_numOfUnitsToHaveIsland );
			ImGui::SameLine(); HelpMarker( "Decides how many provinces will form a island" );
			m_generationSettings.m_numOfUnitsToHaveIsland = GetClamped( m_generationSettings.m_numOfUnitsToHaveIsland, 1, INT_MAX );
			ImGui::InputInt( "Num of Provs to Form Lake", &m_generationSettings.m_numOfUnitsToHaveLake );
			ImGui::SameLine(); HelpMarker( "Decides how many provinces will form a lake" );
			m_generationSettings.m_numOfUnitsToHaveLake = GetClamped( m_generationSettings.m_numOfUnitsToHaveLake, 1, INT_MAX );
			ImGui::InputInt( "Number of Cultures", &m_generationSettings.m_numOfCultures );
			ImGui::SameLine(); HelpMarker( "Decides how many cultures will be generated in the world" );
			m_generationSettings.m_numOfCultures = GetClamped( m_generationSettings.m_numOfCultures, 1, 100 );
			ImGui::InputInt( "Number of Religions", &m_generationSettings.m_numOfReligions );
			ImGui::SameLine(); HelpMarker( "Decides how many religions will be generated in the world" );
			m_generationSettings.m_numOfReligions = GetClamped( m_generationSettings.m_numOfReligions, 1, 30 );
			ImGui::InputFloat( "City Richness", &m_generationSettings.m_cityRichness );
			ImGui::SameLine(); HelpMarker( "Decides the richness of cities" );
			m_generationSettings.m_cityRichness = GetClamped( m_generationSettings.m_cityRichness, 0.f, 1.f );
			ImGui::InputFloat( "Town Richness", &m_generationSettings.m_townRichness );
			ImGui::SameLine(); HelpMarker( "Decides the richness of towns" );
			m_generationSettings.m_townRichness = GetClamped( m_generationSettings.m_townRichness, 0.f, 1.f );
			ImGui::Checkbox( "Use Western Country Names", &m_generationSettings.m_onlyUseWesternCountryPrefix );
			ImGui::SameLine(); HelpMarker( "Only use Duchy Kingdom and Empire as country name prefix" );
		}
		if (ImGui::Button( "Load Settings From File", ImVec2( 250.f, 40.f ) )) {
			LoadGenerationSettings();
		}
		if (ImGui::Button( "Clear Save and Cache Files", ImVec2( 250.f, 40.f ) )) {
			ClearSaves();
		}
		if (ImGui::Button( "Start Generation!", ImVec2( 250.f, 40.f ) )) {
			GenerateNewMap( m_generationSettings );
		}
	}
	else {
		ImGui::BeginTabBar( "#tabs" );
		//Vec4 activeColor = Rgba8().GetAsFloats();
		ImVec4 activeColor = ImVec4( 0.25f, 0.5f, 1.f, 1.f );
		ImVec4 inactiveColor = ImVec4( 0.2f, 0.2f, 0.2f, 1.f );
		//ImVec4 inactiveColor = ImGui::GetStyleColorVec4( ImGuiCol_Button );
		ImVec2 const normalButtonSize = ImVec2( 80.f, 30.f );
		if (ImGui::BeginTabItem( "View Settings" )) {

			ImGui::Text(
				Stringf( "Time: %.2f FPS: %.2f", Clock::GetSystemClock()->GetTotalSeconds(), 1.f / Clock::GetSystemClock()->GetDeltaSeconds() ).c_str() );
			ImGui::Text( "Map Layers" );

			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_POLYGON_EDGES) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Polygon", normalButtonSize )) {
					m_map->m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_POLYGON_EDGES;
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Polygon", normalButtonSize )) {
					m_map->m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_POLYGON_EDGES;
				}
			}

			ImGui::SameLine();
			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_HEIGHT_MAP) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Height", normalButtonSize )) {
					m_map->SetRenderHeightMapMode( false );
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Height", normalButtonSize )) {
					m_map->SetRenderHeightMapMode();
				}
			}

			ImGui::SameLine();
			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CLIMATE_MAP) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Climate", normalButtonSize )) {
					m_map->SetRenderClimateMapMode( false );
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Climate", normalButtonSize )) {
					m_map->SetRenderClimateMapMode();
				}
			}

			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_SUMMER_PRECIPITATION_MAP) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Precip.(S)", normalButtonSize )) {
					m_map->SetRenderPrecipitationMapMode( false );
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Precip.(S)", normalButtonSize )) {
					m_map->SetRenderPrecipitationMapMode();
				}
			}

			ImGui::SameLine();
			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_WINTER_PRECIPITATION_MAP) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Precip.(W)", normalButtonSize )) {
					m_map->SetRenderPrecipitationMapMode( false, false );
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Precip.(W)", normalButtonSize )) {
					m_map->SetRenderPrecipitationMapMode( true, false );
				}
			}

			ImGui::SameLine();
			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_SUMMER_TEMPERATURE_MAP) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Temp.(S)", normalButtonSize )) {
					m_map->SetRenderTemperatureMapMode( false, true );
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Temp.(S)", normalButtonSize )) {
					m_map->SetRenderTemperatureMapMode( true, true );
				}
			}

			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_WINTER_TEMPERATURE_MAP) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Temp.(W)", normalButtonSize )) {
					m_map->SetRenderTemperatureMapMode( false, false );
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Temp.(W)", normalButtonSize )) {
					m_map->SetRenderTemperatureMapMode( true, false );
				}
			}

			ImGui::SameLine();
			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LANDFORM_MAP) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Landform", normalButtonSize )) {
					m_map->SetRenderLandformMapMode( false );
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Landform", normalButtonSize )) {
					m_map->SetRenderLandformMapMode( true );
				}
			}

			ImGui::SameLine();
			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RIVERS) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "River", normalButtonSize )) {
					m_map->m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_RIVERS;
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "River", normalButtonSize )) {
					m_map->m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_RIVERS;
				}
			}

			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_POPULATION_MAP) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Population", normalButtonSize )) {
					m_map->SetRenderPopulationMapMode( false );
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Population", normalButtonSize )) {
					m_map->SetRenderPopulationMapMode( true );
				}
			}

			ImGui::SameLine();
			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CULTURE_MAP) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Culture", normalButtonSize )) {
					m_map->SetRenderCultureMapMode( false );
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Culture", normalButtonSize )) {
					m_map->SetRenderCultureMapMode( true );
				}
			}

			ImGui::SameLine();
			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELIGION_MAP) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Religion", normalButtonSize )) {
					m_map->SetRenderReligionMapMode( false );
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Religion", normalButtonSize )) {
					m_map->SetRenderReligionMapMode( true );
				}
			}

			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CITIES) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "City", normalButtonSize )) {
					m_map->m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_CITIES;
					m_map->m_curViewingCity = nullptr;
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "City", normalButtonSize )) {
					m_map->m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_CITIES;
				}
			}

			ImGui::SameLine();

			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_TOWNS) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Town", normalButtonSize )) {
					m_map->m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_TOWNS;
					//m_map->m_curViewingCity = nullptr;
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Town", normalButtonSize )) {
					m_map->m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_TOWNS;
				}
			}

			ImGui::SameLine();

			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CONTINENT_MAP) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Continent", normalButtonSize )) {
					m_map->SetRenderContinentMapMode( false );
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Continent", normalButtonSize )) {
					m_map->SetRenderContinentMapMode( true );
				}
			}

			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_PRODUCT_MAP) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Product", normalButtonSize )) {
					m_map->SetRenderProductMapMode( false );
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Product", normalButtonSize )) {
					m_map->SetRenderProductMapMode( true );
				}
			}
			ImGui::SameLine();

			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_COUNTRIES_MAP) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Country", normalButtonSize )) {
					m_map->SetRenderCountryMapMode( false );
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Country", normalButtonSize )) {
					m_map->SetRenderCountryMapMode( true );
				}
			}
			ImGui::SameLine();

			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_COUNTRIES_EDGES) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Boarders", normalButtonSize )) {
					m_map->m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_COUNTRIES_EDGES;
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Boarders", normalButtonSize )) {
					m_map->m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_COUNTRIES_EDGES;
				}
			}

			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_REGIONS_MAP) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Region", normalButtonSize )) {
					m_map->SetRenderRegionMapMode( false );
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Region", normalButtonSize )) {
					m_map->SetRenderRegionMapMode( true );
				}
			}

			ImGui::SameLine();
			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELATION_MAP) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Relation", normalButtonSize )) {
					m_map->SetRenderRelationMapMode( false );
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Relation", normalButtonSize )) {
					m_map->SetRenderRelationMapMode( true );
				}
			}
			ImGui::SameLine();
			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_ROADS) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Road", normalButtonSize )) {
					m_map->m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_ROADS;
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Road", normalButtonSize )) {
					m_map->m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_ROADS;
				}
			}

			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Label", normalButtonSize )) {
					m_map->m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_LABELS;
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Label", normalButtonSize )) {
					m_map->m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_LABELS;
				}
			}

			ImGui::SameLine();
			if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_ARMIES) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Army", normalButtonSize )) {
					m_map->m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_ARMIES;
					m_map->m_curViewingArmy = nullptr;
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Army", normalButtonSize )) {
					m_map->m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_ARMIES;
				}
			}

			ImGui::SameLine();
			if (m_viewMode == MapViewMode::ViewMode3D) {
				ImGui::PushStyleColor( ImGuiCol_Button, inactiveColor );
				if (ImGui::Button( "Owner Edit", normalButtonSize )) {
				}
				ImGui::PopStyleColor();
			}
			else {
				if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_PROVINCE_EDIT) {
					ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
					if (ImGui::Button( "Owner Edit", normalButtonSize )) {
						m_map->SetRenderProvEditMapMode( false );
					}
					ImGui::PopStyleColor();
				}
				else {
					if (ImGui::Button( "Owner Edit", normalButtonSize )) {
						m_map->SetRenderProvEditMapMode( true );
					}
				}
			}

			ImGui::Text( "View Mode" );
			if (m_viewMode == MapViewMode::ViewMode2D) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "2D", normalButtonSize )) {
					m_viewMode = MapViewMode::ViewMode2D;
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "2D", normalButtonSize )) {
					m_map->Reset2DCameraMode();
					m_viewMode = MapViewMode::ViewMode2D;
					g_theInput->ConsumeMouseWheelInput();
				}
			}
			ImGui::SameLine();
			if (m_viewMode == MapViewMode::ViewMode3D) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "3D", normalButtonSize )) {
					m_viewMode = MapViewMode::ViewMode3D;
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "3D", normalButtonSize )) {
					m_map->Reset3DCameraMode();
					m_viewMode = MapViewMode::ViewMode3D;
				}
			}
			ImGui::SameLine();
			if (m_viewMode == MapViewMode::ViewModeSphere) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Sphere", normalButtonSize )) {
					m_viewMode = MapViewMode::ViewModeSphere;
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Sphere", normalButtonSize )) {
					m_map->ResetSphereCameraMode();
					m_viewMode = MapViewMode::ViewModeSphere;
				}
			}
			if (ImGui::Button( "Return to Generation Settings", ImVec2( 210.f, 35.f ) )) {
				delete m_map;
				m_map = nullptr;
				m_viewMode = MapViewMode::ViewMode2D;
			}

			if (ImGui::Button( "Save Generation Settings", ImVec2( 210.f, 35.f ) )) {
				SaveCurrentGenerationSettings();
			}
			if (ImGui::Button( "Save Current World", ImVec2( 210.f, 35.f ) )) {
				m_map->SaveCurrentWorldToXml();
			}
			if (ImGui::Button( "Save World History", ImVec2( 210.f, 35.f ) )) {
				m_map->SaveHistoryToXml();
			}
			if (ImGui::Button( "Regenerate(seed + 1)", ImVec2( 210.f, 35.f ) )) {
				//MapShowingSettings showingSettings = m_map->m_showingSettings;
				m_generationSettings.m_seed++;
				GenerateNewMap( m_generationSettings );
				//m_map->m_showingSettings = showingSettings;
			}
			ImGui::EndTabItem();
		}
		if (m_map && ImGui::BeginTabItem( "Debug Layers" )) {

			if (m_map->m_showingSettings.m_debugShowPolygonCenter) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Polygon Center", ImVec2( 100.f, 30.f ) )) {
					m_map->m_showingSettings.m_debugShowPolygonCenter = !m_map->m_showingSettings.m_debugShowPolygonCenter;
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Polygon Center", ImVec2( 100.f, 30.f ) )) {
					m_map->m_showingSettings.m_debugShowPolygonCenter = !m_map->m_showingSettings.m_debugShowPolygonCenter;
				}
			}
			if (m_map->m_showingSettings.m_debugShowNearestOceanUnit) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Nearest Sea", ImVec2( 100.f, 30.f ) )) {
					m_map->m_showingSettings.m_debugShowNearestOceanUnit = !m_map->m_showingSettings.m_debugShowNearestOceanUnit;
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Nearest Sea", ImVec2( 100.f, 30.f ) )) {
					m_map->m_showingSettings.m_debugShowNearestOceanUnit = !m_map->m_showingSettings.m_debugShowNearestOceanUnit;
				}
			}
			if (m_map->m_showingSettings.m_debugShowRiverStart) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "River Start", ImVec2( 100.f, 30.f ) )) {
					m_map->m_showingSettings.m_debugShowRiverStart = !m_map->m_showingSettings.m_debugShowRiverStart;
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "River Start", ImVec2( 100.f, 30.f ) )) {
					m_map->m_showingSettings.m_debugShowRiverStart = !m_map->m_showingSettings.m_debugShowRiverStart;
				}
			}
			if (m_map->m_showingSettings.m_debugShowCountryStart) {
				ImGui::PushStyleColor( ImGuiCol_Button, activeColor );
				if (ImGui::Button( "Country Origin", ImVec2( 100.f, 30.f ) )) {
					m_map->m_showingSettings.m_debugShowCountryStart = !m_map->m_showingSettings.m_debugShowCountryStart;
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button( "Country Origin", ImVec2( 100.f, 30.f ) )) {
					m_map->m_showingSettings.m_debugShowCountryStart = !m_map->m_showingSettings.m_debugShowCountryStart;
				}
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::PopItemWidth();
	ImGui::End();
}

void Game::UpdateHistorySimulationUI()
{
	if (m_map && m_map->m_generationSettings.m_enableHistorySimulation) {
		if (!ImGui::Begin( "History Panel", NULL, 0 ))
		{
			ImGui::End();
			return;
		}
		ImGui::PushItemWidth( ImGui::GetFontSize() * -5.f );
		ImGui::Text( "Current Time:\nYear:%4d Month:%2d", m_map->m_year, m_map->m_month );
		ImGui::Text( "Viewing Time:\nYear:%4d Month:%2d", m_map->m_viewingYear, m_map->m_viewingMonth );
		ImGui::Text( "View Simulation" );
		if (ImGui::Button( "Last Month", ImVec2( 75.f, 40.f ) )) {
			m_map->ViewLastMonth();
		}
		ImGui::SameLine();
		if (ImGui::Button( "Next Month", ImVec2( 75.f, 40.f ) )) {
			m_map->ViewNextMonth();
		}
		if (ImGui::Button( "Last Year", ImVec2( 75.f, 40.f ) )) {
			m_map->ViewLastYear();
		}
		ImGui::SameLine();
		if (ImGui::Button( "Next Year", ImVec2( 75.f, 40.f ) )) {
			m_map->ViewNextYear();
		}
		ImGui::Text( "Continue Simulation" );
		//ImGui::Checkbox( "Jump to Current Time", &m_map->m_simulationJumpToCurrent );
		if (ImGui::Button( "Simulate Next Month", ImVec2( 160.f, 40.f ) )) {
			m_map->SimulateNextMonth();
		}
		if (ImGui::Button( "Simulate Next Year", ImVec2( 160.f, 40.f ) )) {
			m_map->SimulateMonths( 12 );
		}
		ImGui::Checkbox( "Auto Simulation", &m_map->m_autoRunSimulation );
		ImGui::Text( "Batch Simulation" );
		//ImGui::PushItemWidth( -ImGui::GetFontSize() );
		ImGui::Text( "Year   " ); ImGui::SameLine(); ImGui::InputInt( "##SimulateToYear", (int*)&m_simulateToTime.x, 0, 0 );
		ImGui::Text( "Month  " ); ImGui::SameLine(); ImGui::InputInt( "##SimulateToMonth", (int*)&m_simulateToTime.y, 0, 0 );
		m_simulateToTime.y = GetClamped( m_simulateToTime.y, 1, 12 );
		if (ImGui::Button( "Simulate To Above Time", ImVec2( 160.f, 40.f ) )) {
			if (m_simulateToTime.x > m_map->m_year || (m_simulateToTime.x == m_map->m_year && m_simulateToTime.y > m_map->m_month)) {
				int months = m_map->GetMonthDiffBetweenTwoTimes( m_simulateToTime.x, m_simulateToTime.y, m_map->m_year, m_map->m_month );
				m_map->SimulateMonths( months );
			}
		}
		ImGui::Text( "Year   " ); ImGui::SameLine(); ImGui::InputInt( "##SimulateYear", (int*)&m_simulateAmountTime.x, 0, 0 );
		ImGui::Text( "Month  " ); ImGui::SameLine(); ImGui::InputInt( "##SimulateMonth", (int*)&m_simulateAmountTime.y, 0, 0 );
		m_simulateAmountTime.y = GetClamped( m_simulateAmountTime.y, 0, 11 );
		if (ImGui::Button( "Simulate Above Number\n  of Year and Months", ImVec2( 160.f, 60.f ) )) {
			m_map->SimulateMonths( m_simulateAmountTime.x * 12 + m_simulateAmountTime.y );
		}
		//ImGui::PopItemWidth();

		ImGui::PopItemWidth();
		ImGui::End();
	}
}

void Game::UpdateHistoryLogUI()
{
	if (m_map && m_map->m_generationSettings.m_enableHistorySimulation) {
		if (!ImGui::Begin( "History Log", NULL, 0 ))
		{
			ImGui::End();
			return;
		}
		ImGui::PushItemWidth( ImGui::GetFontSize() * -13.f );

		for (auto const& str : m_map->m_historyLog) {
			ImGui::Text( str.c_str() );
		}

		ImGui::PopItemWidth();
		ImGui::End();
	}

}

void Game::UpdateLegendsUI()
{
	UpdateClimateLegend();
	UpdateLandformLegend();
	UpdateCultureLegend();
	UpdateReligionLegend();
	UpdateProductLegend();
	UpdateRelationLegend();
	if (m_map && (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_HEIGHT_MAP)) {
		MakeLinearValueLegend( "Height Legend", m_map->m_seaLevel, m_map->m_generationSettings.m_maxHeight, m_map->m_renderPreference.m_lowestLandHeightColor, m_map->m_renderPreference.m_highestLandHeightColor, "m", 10 );
	}
	if (m_map && (m_map->m_showingSettings.m_mapShowConfig & (SHOW_CONFIG_SUMMER_PRECIPITATION_MAP | SHOW_CONFIG_WINTER_PRECIPITATION_MAP))) {
		MakeLinearValueLegend( "Precipitation Legend", 0.f, 2000.f, m_map->m_renderPreference.m_lowestPrecipitationColor, m_map->m_renderPreference.m_highestPrecipitationColor, "mm", 10 );
	}
	if (m_map && (m_map->m_showingSettings.m_mapShowConfig & (SHOW_CONFIG_SUMMER_TEMPERATURE_MAP | SHOW_CONFIG_WINTER_TEMPERATURE_MAP))) {
		MakeLinearValueLegend( "Temperature Legend", -10.f, 30.f, m_map->m_renderPreference.m_lowestTemperatureColor, m_map->m_renderPreference.m_highestTemperatureColor, "Deg Celsius", 10);
	}
	if (m_map && (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_POPULATION_MAP)) {
		MakeLinearValueLegend( "Population Legend", 0.f, 80000.f, m_map->m_renderPreference.m_lowestPopulationColor, m_map->m_renderPreference.m_highestPopulationColor, "", 10);
	}
}
void Game::UpdateClimateLegend()
{
	if (m_map && (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CLIMATE_MAP)) {
		if (!ImGui::Begin( "Climate Legend", NULL, 0 ))
		{
			ImGui::End();
			return;
		}
		ImGui::PushItemWidth( ImGui::GetFontSize() * -13 );
		ImVec2 const normalButtonSize = ImVec2( 40.f, 40.f );
		ImGuiColorEditFlags colorFlag = ImGuiColorEditFlags_NoPicker
			| ImGuiColorEditFlags_NoOptions
			| ImGuiColorEditFlags_NoSmallPreview
			| ImGuiColorEditFlags_NoInputs
			| ImGuiColorEditFlags_NoTooltip
			| ImGuiColorEditFlags_NoLabel
			| ImGuiColorEditFlags_NoSidePreview
			| ImGuiColorEditFlags_NoDragDrop;

		ImGui::ColorButton( "Ocean", (ImVec4&)m_map->m_renderPreference.m_climateColorMapVec4[13], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Ocean" );
		ImGui::ColorButton( "Tropical Rain Forest Climate", (ImVec4&)m_map->m_renderPreference.m_climateColorMapVec4[0], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Tropical Rain forest Climate" );
		ImGui::ColorButton( "Tropical Monsoon Climate", (ImVec4&)m_map->m_renderPreference.m_climateColorMapVec4[1], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Tropical Monsoon Climate" );
		ImGui::ColorButton( "Tropical Savanna Climate", (ImVec4&)m_map->m_renderPreference.m_climateColorMapVec4[2], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Tropical Savanna Climate" );
		ImGui::ColorButton( "Hot Desert Climate", (ImVec4&)m_map->m_renderPreference.m_climateColorMapVec4[3], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Hot Desert Climate" );
		ImGui::ColorButton( "Cold Desert Climate", (ImVec4&)m_map->m_renderPreference.m_climateColorMapVec4[4], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Cold Desert Climate" );
		ImGui::ColorButton( "Mediterranean Climate", (ImVec4&)m_map->m_renderPreference.m_climateColorMapVec4[5], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Mediterranean Climate" );
		ImGui::ColorButton( "Humid Subtropical Climate", (ImVec4&)m_map->m_renderPreference.m_climateColorMapVec4[6], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Humid Subtropical Climate" );
		ImGui::ColorButton( "Oceanic Climate", (ImVec4&)m_map->m_renderPreference.m_climateColorMapVec4[7], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Oceanic Climate" );
		ImGui::ColorButton( "Humid Continental Climate with Monsoon", (ImVec4&)m_map->m_renderPreference.m_climateColorMapVec4[8], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Humid Continental Climate with Monsoon" );
		ImGui::ColorButton( "Humid Continental Climate", (ImVec4&)m_map->m_renderPreference.m_climateColorMapVec4[9], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Humid Continental Climate" );
		ImGui::ColorButton( "Subarctic Climate", (ImVec4&)m_map->m_renderPreference.m_climateColorMapVec4[10], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Subarctic Climate" );
		ImGui::ColorButton( "Tundra Climate", (ImVec4&)m_map->m_renderPreference.m_climateColorMapVec4[11], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Tundra Climate" );
		ImGui::ColorButton( "IceCap Climate", (ImVec4&)m_map->m_renderPreference.m_climateColorMapVec4[12], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "IceCap Climate" );
		ImGui::ColorButton( "None", (ImVec4&)m_map->m_renderPreference.m_climateColorMapVec4[14], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "None" );


		ImGui::PopItemWidth();
		ImGui::End();
	}

}

void Game::UpdateLandformLegend()
{
	if (m_map && (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LANDFORM_MAP)) {
		if (!ImGui::Begin( "Landform Legend", NULL, 0 ))
		{
			ImGui::End();
			return;
		}
		ImGui::PushItemWidth( ImGui::GetFontSize() * -13 );
		ImVec2 const normalButtonSize = ImVec2( 40.f, 40.f );
		ImGuiColorEditFlags colorFlag = ImGuiColorEditFlags_NoPicker
			| ImGuiColorEditFlags_NoOptions
			| ImGuiColorEditFlags_NoSmallPreview
			| ImGuiColorEditFlags_NoInputs
			| ImGuiColorEditFlags_NoTooltip
			| ImGuiColorEditFlags_NoLabel
			| ImGuiColorEditFlags_NoSidePreview
			| ImGuiColorEditFlags_NoDragDrop;

		ImGui::ColorButton( "Ocean", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[0], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Ocean" );
		ImGui::ColorButton( "Lake", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[2], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Lake" );
		ImGui::ColorButton( "Island", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[3], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Island" );
		ImGui::ColorButton( "Lowland Plain", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[4], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Lowland Plain" );
		ImGui::ColorButton( "Highland Plain", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[5], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Highland Plain" );
		ImGui::ColorButton( "Grassland", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[6], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Grassland" );
		ImGui::ColorButton( "Savana", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[7], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Savana" );
		ImGui::ColorButton( "Marsh", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[8], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Marsh" );
		ImGui::ColorButton( "Rainforest", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[9], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Rainforest" );
		ImGui::ColorButton( "Temperate Forest", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[10], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Temperate Forest" );
		ImGui::ColorButton( "Subarctic Forest", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[11], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Subarctic Forest" );
		ImGui::ColorButton( "Subtropical Forest", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[12], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Subtropical Forest" );
		ImGui::ColorButton( "Lowland Hill", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[13], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Lowland Hill" );
		ImGui::ColorButton( "Highland Hill", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[14], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Highland Hill" );
		ImGui::ColorButton( "Mountain", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[15], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Mountain" );
		ImGui::ColorButton( "Icefield", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[16], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Icefield" );
		ImGui::ColorButton( "Tundra", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[17], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Tundra" );
		ImGui::ColorButton( "Desert", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[18], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Desert" );
		ImGui::ColorButton( "Town", (ImVec4&)m_map->m_renderPreference.m_landformColorMapVec4[19], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Town" );

		ImGui::PopItemWidth();
		ImGui::End();
	}
}

void Game::UpdateCultureLegend()
{
	if (m_map && (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CULTURE_MAP)) {
		if (!ImGui::Begin( "Culture Legend", NULL, 0 ))
		{
			ImGui::End();
			return;
		}
		ImGui::PushItemWidth( ImGui::GetFontSize() * -13 );
		ImVec2 const normalButtonSize = ImVec2( 40.f, 40.f );
		ImGuiColorEditFlags colorFlag = ImGuiColorEditFlags_NoPicker
			| ImGuiColorEditFlags_NoOptions
			| ImGuiColorEditFlags_NoSmallPreview
			| ImGuiColorEditFlags_NoInputs
			| ImGuiColorEditFlags_NoTooltip
			| ImGuiColorEditFlags_NoLabel
			| ImGuiColorEditFlags_NoSidePreview
			| ImGuiColorEditFlags_NoDragDrop;

		for (auto culture : m_map->m_cultures) {
			Vec4 color = culture->m_color.GetAsFloats();
			ImGui::ColorButton( culture->m_name.c_str(), (ImVec4&)color, colorFlag, normalButtonSize );
			ImGui::SameLine();
			ImGui::Text( culture->m_name.c_str() );
		}

		ImGui::PopItemWidth();
		ImGui::End();
	}
}

void Game::UpdateReligionLegend()
{
	if (m_map && (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELIGION_MAP)) {
		if (!ImGui::Begin( "Religion Legend", NULL, 0 ))
		{
			ImGui::End();
			return;
		}
		ImGui::PushItemWidth( ImGui::GetFontSize() * -13 );
		ImVec2 const normalButtonSize = ImVec2( 40.f, 40.f );
		ImGuiColorEditFlags colorFlag = ImGuiColorEditFlags_NoPicker
			| ImGuiColorEditFlags_NoOptions
			| ImGuiColorEditFlags_NoSmallPreview
			| ImGuiColorEditFlags_NoInputs
			| ImGuiColorEditFlags_NoTooltip
			| ImGuiColorEditFlags_NoLabel
			| ImGuiColorEditFlags_NoSidePreview
			| ImGuiColorEditFlags_NoDragDrop;

		for (auto religion : m_map->m_religions) {
			Vec4 color = religion->m_color.GetAsFloats();
			ImGui::ColorButton( religion->m_name.c_str(), (ImVec4&)color, colorFlag, normalButtonSize );
			ImGui::SameLine();
			ImGui::Text( religion->m_name.c_str() );
		}

		ImGui::PopItemWidth();
		ImGui::End();
	}
}

void Game::UpdateProductLegend()
{
	if (m_map && (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_PRODUCT_MAP)) {
		if (!ImGui::Begin( "Product Legend", NULL, 0 ))
		{
			ImGui::End();
			return;
		}
		ImGui::PushItemWidth( ImGui::GetFontSize() * -13 );
		ImVec2 const normalButtonSize = ImVec2( 40.f, 40.f );
		ImGuiColorEditFlags colorFlag = ImGuiColorEditFlags_NoPicker
			| ImGuiColorEditFlags_NoOptions
			| ImGuiColorEditFlags_NoSmallPreview
			| ImGuiColorEditFlags_NoInputs
			| ImGuiColorEditFlags_NoTooltip
			| ImGuiColorEditFlags_NoLabel
			| ImGuiColorEditFlags_NoSidePreview
			| ImGuiColorEditFlags_NoDragDrop;
		ImGui::ColorButton( "Fish", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[0], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Fish      " );
		ImGui::SameLine();
		ImGui::ColorButton( "Grain", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[1], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Grain" );
		ImGui::ColorButton( "Fruit", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[2], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Fruit     " );
		ImGui::SameLine();
		ImGui::ColorButton( "Sugar", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[3], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Sugar" );
		ImGui::ColorButton( "Salt", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[4], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Salt      " );
		ImGui::SameLine();
		ImGui::ColorButton( "Livestock", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[5], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Livestock" );
		ImGui::ColorButton( "Wax", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[6], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Wax       " );
		ImGui::SameLine();
		ImGui::ColorButton( "Fur", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[7], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Fur" );
		ImGui::ColorButton( "Wood", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[8], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Wood      " );
		ImGui::SameLine();
		ImGui::ColorButton( "Ivory", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[9], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Ivory" );
		ImGui::ColorButton( "Cotton", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[10], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Cotton    " );
		ImGui::SameLine();
		ImGui::ColorButton( "Wool", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[11], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Wool" );
		ImGui::ColorButton( "Iron", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[12], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Iron      " );
		ImGui::SameLine();
		ImGui::ColorButton( "Gold", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[13], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Gold" );
		ImGui::ColorButton( "Copper", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[14], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Copper    " );
		ImGui::SameLine();
		ImGui::ColorButton( "Silver", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[15], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Silver" );
		ImGui::ColorButton( "Wine", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[16], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Wine      " );
		ImGui::SameLine();
		ImGui::ColorButton( "Spice", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[17], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Spice" );
		ImGui::ColorButton( "Tea", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[18], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Tea       " );
		ImGui::SameLine();
		ImGui::ColorButton( "Coffee", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[19], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Coffee" );
		ImGui::ColorButton( "Tobacco", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[20], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Tobacco   " );
		ImGui::SameLine();
		ImGui::ColorButton( "Glass", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[21], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Glass" );
		ImGui::ColorButton( "Silk", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[22], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Silk      " );
		ImGui::SameLine();
		ImGui::ColorButton( "Jade", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[23], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Jade" );
		ImGui::ColorButton( "Porcelain", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[24], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Porcelain " );
		ImGui::SameLine();
		ImGui::ColorButton( "Cloth", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[25], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Cloth" );
		ImGui::ColorButton( "Gem", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[26], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Gem       " );
		ImGui::SameLine();
		ImGui::ColorButton( "WarHorse", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[27], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "WarHorse" );
		ImGui::ColorButton( "Sword", (ImVec4&)m_map->m_renderPreference.m_productColorMapVec4[28], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Sword     " );

		ImGui::PopItemWidth();
		ImGui::End();
	}
}

void Game::MakeLinearValueLegend( std::string const& title, float minValue, float maxValue, Rgba8 minColor, Rgba8 maxColor, std::string const& unitName, int numOfSteps )
{
	if (!ImGui::Begin( title.c_str(), NULL, 0 ))
	{
		ImGui::End();
		return;
	}
	ImGui::PushItemWidth( ImGui::GetFontSize() * -13 );
	ImVec2 const normalButtonSize = ImVec2( 40.f, 40.f );
	ImGuiColorEditFlags colorFlag = ImGuiColorEditFlags_NoPicker
		| ImGuiColorEditFlags_NoOptions
		| ImGuiColorEditFlags_NoSmallPreview
		| ImGuiColorEditFlags_NoInputs
		| ImGuiColorEditFlags_NoTooltip
		| ImGuiColorEditFlags_NoLabel
		| ImGuiColorEditFlags_NoSidePreview
		| ImGuiColorEditFlags_NoDragDrop;

	float valueBetweenSteps = 1.f / (float)numOfSteps;
	Vec4 minColorVec = minColor.GetAsFloats();
	Vec4 maxColorVec = maxColor.GetAsFloats();

	for (int i = 0; i <= numOfSteps; i++) {
		float value = RangeMapClamped( i * valueBetweenSteps, 0.f, 1.f, minValue, maxValue );
		Vec4 colorValue = Interpolate( minColorVec, maxColorVec, i * valueBetweenSteps );
		std::string valueStr = Stringf( " %.1f", value ) + unitName;
		ImGui::ColorButton( (title + valueStr).c_str(), (ImVec4&)colorValue, colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( valueStr.c_str() );
	}

	ImGui::PopItemWidth();
	ImGui::End();
}

void Game::UpdateRelationLegend()
{
	if (m_map && (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELATION_MAP)) {
		if (!ImGui::Begin( "Relation Legend", NULL, 0 ))
		{
			ImGui::End();
			return;
		}
		ImGui::PushItemWidth( ImGui::GetFontSize() * -13 );
		ImVec2 const normalButtonSize = ImVec2( 40.f, 40.f );
		ImGuiColorEditFlags colorFlag = ImGuiColorEditFlags_NoPicker
			| ImGuiColorEditFlags_NoOptions
			| ImGuiColorEditFlags_NoSmallPreview
			| ImGuiColorEditFlags_NoInputs
			| ImGuiColorEditFlags_NoTooltip
			| ImGuiColorEditFlags_NoLabel
			| ImGuiColorEditFlags_NoSidePreview
			| ImGuiColorEditFlags_NoDragDrop;
		ImGui::ColorButton( "Friendly", (ImVec4&)m_map->m_renderPreference.m_relationColorMapVec4[0], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Friendly" );
		ImGui::ColorButton( "Alliance", (ImVec4&)m_map->m_renderPreference.m_relationColorMapVec4[1], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Alliance" );
		ImGui::ColorButton( "Hostile", (ImVec4&)m_map->m_renderPreference.m_relationColorMapVec4[2], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Hostile" );
		ImGui::ColorButton( "War", (ImVec4&)m_map->m_renderPreference.m_relationColorMapVec4[3], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "War" );
		ImGui::ColorButton( "Tributary", (ImVec4&)m_map->m_renderPreference.m_relationColorMapVec4[4], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Tributary" );
		ImGui::ColorButton( "Suzerain", (ImVec4&)m_map->m_renderPreference.m_relationColorMapVec4[5], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Suzerain" );
		ImGui::ColorButton( "Vassal", (ImVec4&)m_map->m_renderPreference.m_relationColorMapVec4[6], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Vassal" );
		ImGui::ColorButton( "Celestial", (ImVec4&)m_map->m_renderPreference.m_relationColorMapVec4[7], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Celestial" );
		ImGui::ColorButton( "TribeUnion", (ImVec4&)m_map->m_renderPreference.m_relationColorMapVec4[8], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "TribeUnion" );
		ImGui::ColorButton( "None", (ImVec4&)m_map->m_renderPreference.m_relationColorMapVec4[9], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "None" );
		ImGui::ColorButton( "Owned Core Province", (ImVec4&)m_map->m_renderPreference.m_relationColorMapVec4[10], colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Owned Core Province" );
		Vec4 invalidProvColor = m_map->m_renderPreference.m_invalidProvince.GetAsFloats();
		ImGui::ColorButton( "Owned Invalid Province", (ImVec4&)invalidProvColor, colorFlag, normalButtonSize );
		ImGui::SameLine();
		ImGui::Text( "Owned Invalid Province" );
		ImGui::PopItemWidth();
		ImGui::End();
	}
}

void Game::UpdateProvinceUI()
{
	if (m_map) {
		m_map->m_viewingCountry = false;
		m_map->m_viewingRegion = false;
	}
	if (m_map && m_map->m_curViewingUnit) {
		ImGui::Begin( "Province", &m_map->m_showProvincePanel, 0 );
		if (!m_map->m_showProvincePanel)
		{
			m_map->m_curViewingUnit->SetRenderViewingColor( false );
			m_map->m_curViewingUnit = nullptr;
			ImGui::End();
			return;
		}
		ImGui::PushItemWidth( ImGui::GetFontSize() * -13 );

		// country name change
		ImGui::BeginTabBar( "#provtabs" );
		if (ImGui::BeginTabItem( "Province" )) {
			bool needChangeEconomy = false;
			ImGui::PushItemWidth( -ImGui::GetFontSize() );
			ImGui::Text( "Name               " ); ImGui::SameLine(); ImGui::InputText( "   ProvName", &m_map->m_curViewingUnit->m_name );
			ImGui::Text( Stringf( "ID                   %d", m_map->m_curViewingUnit->m_id ).c_str() );
			ImGui::Text( "Population         " ); ImGui::SameLine();
			int prevProvPopulation = m_map->m_curViewingUnit->m_totalPopulation;
			if (ImGui::InputInt( "   Population", &m_map->m_curViewingUnit->m_totalPopulation, 0, 0 )) {
				// change the population of the owner country
				m_map->m_curViewingUnit->ResolveChangePopulation( prevProvPopulation );
				needChangeEconomy = true;
			}
			ImGui::Text( "Height             " ); ImGui::SameLine(); ImGui::InputFloat( "m  Height", &m_map->m_curViewingUnit->m_height );
			// climate
			ImGui::Text( "Climate            " ); ImGui::SameLine();
			if (ImGui::Combo( "   Prov Climate", (int*)&m_map->m_curViewingUnit->m_climate, g_climateNameMapC, (int)ClimateType::Num )) {
				if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CLIMATE_MAP) {
					m_map->m_curViewingUnit->SetRenderViewingColor( false );
					m_map->m_curViewingUnit->SetRenderClimateColor();
					m_map->m_curViewingUnit->SetRenderViewingColor( true );
				}
			}
			// product
			ImGui::Text( "Product            " ); ImGui::SameLine();
			if (ImGui::Combo( "   Prov Product", (int*)&m_map->m_curViewingUnit->m_productType, g_productNameMapC, (int)ProductType::Num)) {
				if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_PRODUCT_MAP) {
					m_map->m_curViewingUnit->SetRenderViewingColor( false );
					m_map->m_curViewingUnit->SetRenderProductColor();
					m_map->m_curViewingUnit->SetRenderViewingColor( true );
				}
				needChangeEconomy = true;
			}
			// landform
			ImGui::Text( "Landform           " ); ImGui::SameLine();
			// hard code to disable to change to town landform
			if (ImGui::Combo( "   Prov Landform", (int*)&m_map->m_curViewingUnit->m_landform, g_landformNameMapC, (int)LandformType::NUM - 1 )) {
				if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LANDFORM_MAP) {
					m_map->m_curViewingUnit->SetRenderViewingColor( false );
					m_map->m_curViewingUnit->SetRenderLandformColor();
					m_map->m_curViewingUnit->SetRenderViewingColor( true );
				}
			}

			if (m_map->m_curViewingUnit->m_longitude < 0.f) {
				ImGui::Text( Stringf( "Longitude            %.3fW", -m_map->m_curViewingUnit->m_longitude ).c_str() );
			}
			else {
				ImGui::Text( Stringf( "Longitude            %.3fE", m_map->m_curViewingUnit->m_longitude ).c_str() );
			}
			if (m_map->m_curViewingUnit->m_latitude < 0.f) {
				ImGui::Text( Stringf( "Latitude             %.3fN", -m_map->m_curViewingUnit->m_latitude ).c_str() );
			}
			else {
				ImGui::Text( Stringf( "Latitude             %.3fS", m_map->m_curViewingUnit->m_latitude ).c_str() );
			}

			ImGui::Text( Stringf( "Area Size            %.3f", m_map->m_curViewingUnit->m_areaSize ).c_str() );

			ImGui::Text( "Summer Avg. Precip." ); ImGui::SameLine(); ImGui::InputFloat( "mm  summerAvgPrecip", &m_map->m_curViewingUnit->m_summerPrecipitation );
			ImGui::Text( "Winter Avg. Precip." ); ImGui::SameLine(); ImGui::InputFloat( "mm  winterAvgPrecip", &m_map->m_curViewingUnit->m_winterPrecipitation );
			ImGui::Text( "Summer Avg. Temp.  " ); ImGui::SameLine(); ImGui::InputFloat( "    summerAvgTemp", &m_map->m_curViewingUnit->m_summerAvgTemperature );
			ImGui::Text( "Winter Avg. Temp.  " ); ImGui::SameLine(); ImGui::InputFloat( "    winterAvgTemp", &m_map->m_curViewingUnit->m_winterAvgTemperature );

			// owner


			// religion
			ImGui::Text( "Religions" );
			bool religionChanged = false;
			std::vector<Religion*> unselectedReligions;
			m_map->m_curViewingUnit->GetUnselectedReligions( unselectedReligions );
			// add a religion to the last of the list
			if ((int)unselectedReligions.size() > 0) {
				ImGui::SameLine();
				if (ImGui::Button( "+##religion", ImVec2( 18.f, 18.f ) )) {
					religionChanged = true;
					if (m_map->m_curViewingUnit->m_owner) {
						m_map->m_curViewingUnit->m_owner->AddReligionPopulation( -m_map->m_curViewingUnit->m_totalPopulation, m_map->m_curViewingUnit->m_religions );
					}
					m_map->m_curViewingUnit->m_religions.push_back( std::pair<Religion*, float>( unselectedReligions[0], 0.01f ) );
					m_map->m_curViewingUnit->SqueezeReligionInfluence( unselectedReligions[0], 0.01f, 0.f );
					m_map->m_curViewingUnit->GetUnselectedReligions( unselectedReligions );
				}
			}
			// remove the last religion in the list
			if ((int)m_map->m_curViewingUnit->m_religions.size() > 1) {
				ImGui::SameLine();
				if (ImGui::Button( "-##religion", ImVec2( 18.f, 18.f ) )) {
					religionChanged = true;
					if (m_map->m_curViewingUnit->m_owner) {
						m_map->m_curViewingUnit->m_owner->AddReligionPopulation( -m_map->m_curViewingUnit->m_totalPopulation, m_map->m_curViewingUnit->m_religions );
					}
					m_map->m_curViewingUnit->SqueezeReligionInfluence( m_map->m_curViewingUnit->m_religions.back().first, 
						-m_map->m_curViewingUnit->m_religions.back().second, m_map->m_curViewingUnit->m_religions.back().second );
					m_map->m_curViewingUnit->m_religions.pop_back();
					m_map->m_curViewingUnit->GetUnselectedReligions( unselectedReligions );
				}
			}
			for (auto& pair : m_map->m_curViewingUnit->m_religions) {
				std::vector<Religion*> thisUnselectedReligions;
				thisUnselectedReligions.push_back( pair.first );
				for (auto religion : unselectedReligions) {
					thisUnselectedReligions.push_back( religion );
				}
				int religionIndex = 0;
				float windowWidth = ImGui::GetWindowSize().x;
				ImGui::PushItemWidth( -windowWidth * 0.5f );
				// drop down menu to choose a eligible religion
				if (ImGui::Combo( Stringf("##religion choose %d", pair.first->m_id ).c_str(), &religionIndex, []( void* vector, int idx ) {return ((std::vector<Religion*>*)vector)->at( (size_t)idx )->m_name.c_str(); }, (void*)&thisUnselectedReligions, (int)thisUnselectedReligions.size())) {
					if (!religionChanged) {
						if (m_map->m_curViewingUnit->m_owner) {
							m_map->m_curViewingUnit->m_owner->AddReligionPopulation( -m_map->m_curViewingUnit->m_totalPopulation, m_map->m_curViewingUnit->m_religions );
						}
						religionChanged = true;
					}
					pair.first = thisUnselectedReligions[religionIndex];
				}
				ImGui::PopItemWidth();

				ImGui::SameLine();
				ImGui::SetCursorPosX( windowWidth * 0.5f );
				//ImGui::SameLine();
				// ImGui::Text( pair.first->m_name.c_str() );
				// change the influence
				float prevValue = pair.second;
				if (ImGui::InputFloat( Stringf( "   #religion influence %d", pair.first->m_id ).c_str(), &pair.second )) {
					if (!religionChanged) {
						if (m_map->m_curViewingUnit->m_owner) {
							m_map->m_curViewingUnit->m_owner->AddReligionPopulation( -m_map->m_curViewingUnit->m_totalPopulation, m_map->m_curViewingUnit->m_religions );
						}
						religionChanged = true;
					}
					pair.second = GetClamped( pair.second, 0.f, 1.f );
					m_map->m_curViewingUnit->SqueezeReligionInfluence( pair.first, pair.second - prevValue, prevValue );
				}
			}

			if (religionChanged) {
				// need to recalculate economy
				needChangeEconomy = true;
				m_isLabelDirty = true;
				// recalculate major religion
				m_map->m_curViewingUnit->RecalculateMajorReligion();
				// change the major religion and push the result to the owner country
				if (m_map->m_curViewingUnit->m_owner) {
					m_map->m_curViewingUnit->m_owner->AddReligionPopulation( m_map->m_curViewingUnit->m_totalPopulation, m_map->m_curViewingUnit->m_religions );
					m_map->m_curViewingUnit->m_owner->CalculateMajorReligion();
				}
				// change the color if viewing religion map
				if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELIGION_MAP) {
					m_map->m_curViewingUnit->SetRenderViewingColor( false );
					m_map->m_curViewingUnit->SetRenderRegionColor();
					m_map->m_curViewingUnit->SetRenderViewingColor( true );
				}
			}

			// culture
			ImGui::Text( "Cultures" );
			bool cultureChanged = false;
			std::vector<Culture*> unselectedCultures;
			m_map->m_curViewingUnit->GetUnselectedCultures( unselectedCultures );
			// add a culture to the last of the list
			if ((int)unselectedCultures.size() > 0) {
				ImGui::SameLine();
				if (ImGui::Button( "+##culture", ImVec2( 18.f, 18.f ) )) {
					cultureChanged = true;
					if (m_map->m_curViewingUnit->m_owner) {
						m_map->m_curViewingUnit->m_owner->AddCulturePopulation( -m_map->m_curViewingUnit->m_totalPopulation, m_map->m_curViewingUnit->m_cultures );
					}
					m_map->m_curViewingUnit->m_cultures.push_back( std::pair<Culture*, float>( unselectedCultures[0], 0.01f ) );
					m_map->m_curViewingUnit->SqueezeCultureInfluence( unselectedCultures[0], 0.01f, 0.f );
					m_map->m_curViewingUnit->GetUnselectedCultures( unselectedCultures );
				}
			}
			// remove the last culture in the list
			if ((int)m_map->m_curViewingUnit->m_cultures.size() > 1) {
				ImGui::SameLine();
				if (ImGui::Button( "-##culture", ImVec2( 18.f, 18.f ) )) {
					cultureChanged = true;
					if (m_map->m_curViewingUnit->m_owner) {
						m_map->m_curViewingUnit->m_owner->AddCulturePopulation( -m_map->m_curViewingUnit->m_totalPopulation, m_map->m_curViewingUnit->m_cultures );
					}
					m_map->m_curViewingUnit->SqueezeCultureInfluence( m_map->m_curViewingUnit->m_cultures.back().first,
						-m_map->m_curViewingUnit->m_cultures.back().second, m_map->m_curViewingUnit->m_cultures.back().second );
					m_map->m_curViewingUnit->m_cultures.pop_back();
					m_map->m_curViewingUnit->GetUnselectedCultures( unselectedCultures );
				}
			}
			for (auto& pair : m_map->m_curViewingUnit->m_cultures) {
				std::vector<Culture*> thisUnselectedCultures;
				thisUnselectedCultures.push_back( pair.first );
				for (auto culture : unselectedCultures) {
					thisUnselectedCultures.push_back( culture );
				}
				int cultureIndex = 0;
				float windowWidth = ImGui::GetWindowSize().x;
				ImGui::PushItemWidth( -windowWidth * 0.5f );
				// drop down menu to choose a eligible culture
				if (ImGui::Combo( Stringf( "##culture choose %d", pair.first->m_id ).c_str(), &cultureIndex, 
					[]( void* vector, int idx ) {return ((std::vector<Culture*>*)vector)->at( (size_t)idx )->m_name.c_str(); },
					(void*)&thisUnselectedCultures, (int)thisUnselectedCultures.size() )) {
					if (!cultureChanged) {
						if (m_map->m_curViewingUnit->m_owner) {
							m_map->m_curViewingUnit->m_owner->AddCulturePopulation( -m_map->m_curViewingUnit->m_totalPopulation, m_map->m_curViewingUnit->m_cultures );
						}
						cultureChanged = true;
					}
					pair.first = thisUnselectedCultures[cultureIndex];
				}

				ImGui::PopItemWidth();

				ImGui::SameLine();
				ImGui::SetCursorPosX( windowWidth * 0.5f );
				// ImGui::Text( pair.first->m_name.c_str() );
				// change the influence
				float prevValue = pair.second;
				if (ImGui::InputFloat( Stringf( "   #culture influence %d", pair.first->m_id ).c_str(), &pair.second )) {
					if (!cultureChanged) {
						if (m_map->m_curViewingUnit->m_owner) {
							m_map->m_curViewingUnit->m_owner->AddCulturePopulation( -m_map->m_curViewingUnit->m_totalPopulation, m_map->m_curViewingUnit->m_cultures );
						}
						cultureChanged = true;
					}
					pair.second = GetClamped( pair.second, 0.f, 1.f );
					m_map->m_curViewingUnit->SqueezeCultureInfluence( pair.first, pair.second - prevValue, prevValue );
				}
			}

			if (cultureChanged) {
				// need to recalculate economy
				needChangeEconomy = true;
				m_isLabelDirty = true;
				// recalculate major culture
				m_map->m_curViewingUnit->RecalculateMajorCulture();
				// change the major culture and push the result to the owner country
				if (m_map->m_curViewingUnit->m_owner) {
					m_map->m_curViewingUnit->m_owner->AddCulturePopulation( m_map->m_curViewingUnit->m_totalPopulation, m_map->m_curViewingUnit->m_cultures );
					m_map->m_curViewingUnit->m_owner->CalculateMajorCulture();
				}
				// change the color if viewing culture map
				if (m_map->m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CULTURE_MAP) {
					m_map->m_curViewingUnit->SetRenderViewingColor( false );
					m_map->m_curViewingUnit->SetRenderCultureColor();
					m_map->m_curViewingUnit->SetRenderViewingColor( true );
				}
			}

			ImGui::Text( Stringf( "Province Major Culture: %s", m_map->m_curViewingUnit->m_majorCulture->m_name.c_str() ).c_str() );
			ImGui::Text( "Culture Traits:" );
			ImGui::Text( Stringf( "%s", g_cultureTraitsName[(int)m_map->m_curViewingUnit->m_majorCulture->m_traits[0]] ).c_str() );
			ImGui::SameLine(); HelpMarker( g_cultureTraitsDesc[(int)m_map->m_curViewingUnit->m_majorCulture->m_traits[0]] );
			ImGui::Text( Stringf( "%s", g_cultureTraitsName[(int)m_map->m_curViewingUnit->m_majorCulture->m_traits[1]] ).c_str() );
			ImGui::SameLine(); HelpMarker( g_cultureTraitsDesc[(int)m_map->m_curViewingUnit->m_majorCulture->m_traits[1]] );
			ImGui::Text( Stringf( "%s", g_cultureTraitsName[(int)m_map->m_curViewingUnit->m_majorCulture->m_traits[2]] ).c_str() );
			ImGui::SameLine(); HelpMarker( g_cultureTraitsDesc[(int)m_map->m_curViewingUnit->m_majorCulture->m_traits[2]] );
			ImGui::Text( Stringf( "%s", g_cultureTraitsName[(int)m_map->m_curViewingUnit->m_majorCulture->m_traits[3]] ).c_str() );
			ImGui::SameLine(); HelpMarker( g_cultureTraitsDesc[(int)m_map->m_curViewingUnit->m_majorCulture->m_traits[3]] );

			ImGui::PopItemWidth();

			if (needChangeEconomy && m_map->m_curViewingUnit->m_owner) {
				m_map->m_curViewingUnit->m_owner->CalculateEconomicValue();
			}
			/*ImGui::Text(Stringf("Name: %s", m_map->m_curViewingUnit->m_name.c_str()).c_str());
			ImGui::Text( Stringf( "ID: %d", m_map->m_curViewingUnit->m_id ).c_str() );
			ImGui::Text( Stringf( "Population: %d", m_map->m_curViewingUnit->m_totalPopulation ).c_str() );
			ImGui::Text( Stringf( "Height: %.3fm", m_map->m_curViewingUnit->m_height ).c_str() );
			ImGui::Text( Stringf( "Product: %s", g_productNameMap[(int)m_map->m_curViewingUnit->m_productType].c_str() ).c_str() );
			if (m_map->m_curViewingUnit->m_longitude < 0.f) {
				ImGui::Text( Stringf( "Longitude: %.3fW", -m_map->m_curViewingUnit->m_longitude ).c_str() );
			}
			else {
				ImGui::Text( Stringf( "Longitude: %.3fE", m_map->m_curViewingUnit->m_longitude ).c_str() );
			}
			if (m_map->m_curViewingUnit->m_latitude < 0.f) {
				ImGui::Text( Stringf( "Latitude: %.3fN", -m_map->m_curViewingUnit->m_latitude ).c_str() );
			}
			else {
				ImGui::Text( Stringf( "Latitude: %.3fS", m_map->m_curViewingUnit->m_latitude ).c_str() );
			}
			ImGui::Text( Stringf( "Summer Avg. Precip.: %.3fmm", m_map->m_curViewingUnit->m_summerPrecipitation ).c_str() );
			ImGui::Text( Stringf( "Winter Avg. Precip.: %.3fmm", m_map->m_curViewingUnit->m_winterPrecipitation ).c_str() );
			ImGui::Text( Stringf( "Summer Avg. Temp.: %.3f", m_map->m_curViewingUnit->m_summerAvgTemperature ).c_str() );
			ImGui::Text( Stringf( "Winter Avg. Temp.: %.3f", m_map->m_curViewingUnit->m_winterAvgTemperature ).c_str() );
			ImGui::Text( Stringf( "Area Size: %.3f", m_map->m_curViewingUnit->m_areaSize ).c_str() );

			for (int i = 0; i < m_map->m_generationSettings.m_numOfReligions; i++) {
				if ((int)m_map->m_curViewingUnit->m_religions.size() > i) {
					ImGui::Text( Stringf( "Religion %s: %.3f", m_map->m_curViewingUnit->m_religions[i].first->m_name.c_str(), m_map->m_curViewingUnit->m_religions[i].second ).c_str() );
				}
			}
			for (int i = 0; i < m_map->m_generationSettings.m_numOfCultures; i++) {
				if ((int)m_map->m_curViewingUnit->m_cultures.size() > i) {
					ImGui::Text( Stringf( "Culture %s: %.3f", m_map->m_curViewingUnit->m_cultures[i].first->m_name.c_str(), m_map->m_curViewingUnit->m_cultures[i].second ).c_str() );
				}
			}
			*/
			ImGui::EndTabItem();
		}
// 		if (m_map->m_curViewingUnit->m_region && ImGui::BeginTabItem( "Region" )) {
// 			m_map->m_viewingRegion = true;
// 			int totalPop = m_map->m_curViewingUnit->m_region->m_totalPopulation;
// 			ImGui::Text( Stringf( "Name: %s", m_map->m_curViewingUnit->m_region->m_name.c_str() ).c_str() );
// 			ImGui::Text( Stringf( "Population: %d", totalPop ).c_str() );
// 			ImGui::EndTabItem();
// 		}
		if (m_map->m_curViewingUnit->m_owner) {
			if (ImGui::BeginTabItem( "Country" )) {
				bool needChangeEconomy = false;
				m_map->m_viewingCountry = true;

				ImGui::PushItemWidth( -ImGui::GetFontSize() );
				ImGui::Text( "Name              " ); ImGui::SameLine();
				if (ImGui::InputText( "   CountryName", &m_map->m_curViewingUnit->m_owner->m_name )) {
					m_isLabelDirty = true;
				}
				ImGui::Text( Stringf( "ID		    	  %d", m_map->m_curViewingUnit->m_owner->m_id ).c_str() );
				int totalPop = m_map->m_curViewingUnit->m_owner->m_totalPopulation;
				ImGui::Text( "Funds             " ); ImGui::SameLine(); ImGui::InputInt( "   CountryFunds", &m_map->m_curViewingUnit->m_owner->m_funds, 0, 0 );
				ImGui::Text( Stringf( "Economy			 %d", m_map->m_curViewingUnit->m_owner->m_economyValue ).c_str() );
				ImGui::Text( Stringf( "Population		  %d", totalPop ).c_str() );
				ImGui::Text( Stringf( "Military			%d", m_map->m_curViewingUnit->m_owner->m_totalMilitaryStrength ).c_str() );

				ImGui::Text( "Country Culture   " ); ImGui::SameLine();
				std::vector<Culture*> cultureList;
				m_map->GetCultureListExclude( cultureList, m_map->m_curViewingUnit->m_owner->m_countryCulture );
				cultureList.push_back( m_map->m_curViewingUnit->m_owner->m_countryCulture );
				int cultureIndex = (int)cultureList.size() - 1;
				if (ImGui::Combo( Stringf( "##country culture choose" ).c_str(), &cultureIndex,
					[]( void* vector, int idx ) {return ((std::vector<Culture*>*)vector)->at( (size_t)idx )->m_name.c_str(); },
					(void*)&cultureList, (int)cultureList.size() )) {
					m_map->m_curViewingUnit->m_owner->m_countryCulture = cultureList[cultureIndex];
					needChangeEconomy = true;
				}

				ImGui::Text( "Country Religion  " ); ImGui::SameLine();
				std::vector<Religion*> religionList;
				m_map->GetReligionListExclude( religionList, m_map->m_curViewingUnit->m_owner->m_countryReligion );
				religionList.push_back( m_map->m_curViewingUnit->m_owner->m_countryReligion );
				int religionIndex = (int)religionList.size() - 1;
				if (ImGui::Combo( Stringf( "##country religion choose" ).c_str(), &religionIndex,
					[]( void* vector, int idx ) {return ((std::vector<Religion*>*)vector)->at( (size_t)idx )->m_name.c_str(); },
					(void*)&religionList, (int)religionList.size() )) {
					m_map->m_curViewingUnit->m_owner->m_countryReligion = religionList[religionIndex];
					needChangeEconomy = true;
				}

				ImGui::Text( "Government Type   " ); ImGui::SameLine();
				std::vector<CountryGovernmentType> governmentTypeList = { CountryGovernmentType::Autocracy, CountryGovernmentType::Parliamentarism,
					CountryGovernmentType::Nomad, CountryGovernmentType::Separatism, CountryGovernmentType::Oligarchy, CountryGovernmentType::None };
				int govIndex = (int)m_map->m_curViewingUnit->m_owner->m_governmentType;
				if (ImGui::Combo( Stringf( "##country government choose" ).c_str(), &govIndex,
					[]( void* vector, int idx ) { UNUSED( vector ); return g_governmentTypeDict[idx]; },
					(void*)&governmentTypeList, (int)governmentTypeList.size() )) {
					m_map->m_curViewingUnit->m_owner->m_governmentType = governmentTypeList[govIndex];
				}

				//ImGui::Text( Stringf( "Country Culture	 %s", m_map->m_curViewingUnit->m_owner->m_countryCulture->m_name.c_str() ).c_str() );
				//ImGui::Text( Stringf( "Country Religion	%s", m_map->m_curViewingUnit->m_owner->m_countryReligion->m_name.c_str() ).c_str() );
				ImGui::Text( "" );
				float windowWidth = ImGui::GetWindowSize().x;
				ImGui::PushItemWidth( -windowWidth * 0.5f );
				ImGui::Text( "Religion" );
				ImGui::PopItemWidth();
				ImGui::SameLine();
				ImGui::SetCursorPosX( windowWidth * 0.5f );
				ImGui::Text( "Influence" );
				for (auto& pair : m_map->m_curViewingUnit->m_owner->m_religions) {
					ImGui::PushItemWidth( -windowWidth * 0.5f );
					ImGui::Text( Stringf( "%s", pair.first->m_name.c_str()).c_str() );
					ImGui::PopItemWidth();
					ImGui::SameLine();
					ImGui::SetCursorPosX( windowWidth * 0.5f );
					ImGui::Text( Stringf( "%.3f", (float)pair.second / (float)totalPop ).c_str() );
				}
				ImGui::Text( "" );
				ImGui::PushItemWidth( -windowWidth * 0.5f );
				ImGui::Text( "Culture" );
				ImGui::PopItemWidth();
				ImGui::SameLine();
				ImGui::SetCursorPosX( windowWidth * 0.5f );
				ImGui::Text( "Influence" );
				for (auto& pair : m_map->m_curViewingUnit->m_owner->m_cultures) {
					ImGui::PushItemWidth( -windowWidth * 0.5f );
					ImGui::Text( Stringf( "%s", pair.first->m_name.c_str() ).c_str() );
					ImGui::PopItemWidth();
					ImGui::SameLine();
					ImGui::SetCursorPosX( windowWidth * 0.5f );
					ImGui::Text( Stringf( "%.3f", (float)pair.second / (float)totalPop ).c_str() );
				}
				ImGui::PopItemWidth();
				if (needChangeEconomy && m_map->m_curViewingUnit->m_owner) {
					m_map->m_curViewingUnit->m_owner->CalculateEconomicValue();
				}
				/*
				ImGui::Text(Stringf("Name: %s", m_map->m_curViewingUnit->m_owner->m_name.c_str()).c_str());
				ImGui::Text( Stringf( "ID: %d", m_map->m_curViewingUnit->m_owner->m_id ).c_str() );
				int totalPop = m_map->m_curViewingUnit->m_owner->m_totalPopulation;
				ImGui::Text( Stringf( "Funds: %d", m_map->m_curViewingUnit->m_owner->m_funds ).c_str() );
				ImGui::Text( Stringf( "Economy: %d", m_map->m_curViewingUnit->m_owner->m_economyValue ).c_str() );
				ImGui::Text( Stringf( "Population: %d", totalPop ).c_str() );
				ImGui::Text( Stringf( "Military: %d", m_map->m_curViewingUnit->m_owner->m_totalMilitaryStrength ).c_str() );
				ImGui::Text( Stringf( "Country Culture: %s", m_map->m_curViewingUnit->m_owner->m_countryCulture->m_name.c_str() ).c_str() );
				ImGui::Text( Stringf( "Country Religion: %s", m_map->m_curViewingUnit->m_owner->m_countryReligion->m_name.c_str() ).c_str() );
				for (auto& pair: m_map->m_curViewingUnit->m_owner->m_religions) {
					ImGui::Text( Stringf( "Religion %s: %.3f", pair.first->m_name.c_str(), (float)pair.second / (float)totalPop ).c_str() );

				}
				for (auto& pair: m_map->m_curViewingUnit->m_owner->m_cultures) {
					ImGui::Text( Stringf( "Culture %s: %.3f", pair.first->m_name.c_str(), (float)pair.second / (float)totalPop ).c_str() );
				}
				*/
				ImGui::Text( Stringf( "Country Major Culture: %s", m_map->m_curViewingUnit->m_owner->m_countryCulture->m_name.c_str() ).c_str() );
				ImGui::Text( "Culture Traits:" );
				ImGui::Text( Stringf( "%s", g_cultureTraitsName[(int)m_map->m_curViewingUnit->m_owner->m_countryCulture->m_traits[0]] ).c_str() );
				ImGui::SameLine(); HelpMarker( g_cultureTraitsDesc[(int)m_map->m_curViewingUnit->m_owner->m_countryCulture->m_traits[0]] );
				ImGui::Text( Stringf( "%s", g_cultureTraitsName[(int)m_map->m_curViewingUnit->m_owner->m_countryCulture->m_traits[1]] ).c_str() );
				ImGui::SameLine(); HelpMarker( g_cultureTraitsDesc[(int)m_map->m_curViewingUnit->m_owner->m_countryCulture->m_traits[1]] );
				ImGui::Text( Stringf( "%s", g_cultureTraitsName[(int)m_map->m_curViewingUnit->m_owner->m_countryCulture->m_traits[2]] ).c_str() );
				ImGui::SameLine(); HelpMarker( g_cultureTraitsDesc[(int)m_map->m_curViewingUnit->m_owner->m_countryCulture->m_traits[2]] );
				ImGui::Text( Stringf( "%s", g_cultureTraitsName[(int)m_map->m_curViewingUnit->m_owner->m_countryCulture->m_traits[3]] ).c_str() );
				ImGui::SameLine(); HelpMarker( g_cultureTraitsDesc[(int)m_map->m_curViewingUnit->m_owner->m_countryCulture->m_traits[3]] );

				ImGui::EndTabItem();
			}
		}

		ImGui::EndTabBar();

		ImGui::PopItemWidth();
		ImGui::End();
	}
}

void Game::UpdateCityUI()
{
	if (m_map && m_map->m_curViewingCity) {
		ImGui::Begin( "City", &m_map->m_showCityPanel, 0 );
		if (!m_map->m_showCityPanel)
		{
			m_map->m_curViewingCity = nullptr;
			ImGui::End();
			return;
		}
		ImGui::PushItemWidth( ImGui::GetFontSize() * -13 );
		bool needChangeEconomy = false;
		ImGui::PushItemWidth( -ImGui::GetFontSize() );
		ImGui::Text( "Name               " ); ImGui::SameLine();
		if (ImGui::InputText( "   ##CityName", &m_map->m_curViewingCity->m_name )) {
			m_isLabelDirty = true;
		}
		ImGui::Text( Stringf( "ID                   %d", m_map->m_curViewingCity->m_id ).c_str() );
		ImGui::Text( "Population         " ); ImGui::SameLine();
		int prevProvPopulation = m_map->m_curViewingCity->m_totalPopulation;
		if (ImGui::InputInt( "   ##CityPopulation", &m_map->m_curViewingCity->m_totalPopulation, 0, 0 )) {
			// change the population of the owner country
			m_map->m_curViewingCity->ResolveChangePopulation( prevProvPopulation );
			needChangeEconomy = true;
		}
		ImGui::Text( "Defense            " ); ImGui::SameLine(); ImGui::InputFloat( "   ##CityDef", &m_map->m_curViewingCity->m_defense );
		if (m_map->m_curViewingCity->m_owner) {
			ImGui::Text( Stringf( "Owner: %s", m_map->m_curViewingCity->m_owner->m_name.c_str() ).c_str() );
		}

		// religion
		ImGui::Text( "Religions" );
		bool religionChanged = false;
		std::vector<Religion*> unselectedReligions;
		m_map->m_curViewingCity->GetUnselectedReligions( unselectedReligions );
		// add a religion to the last of the list
		if ((int)unselectedReligions.size() > 0) {
			ImGui::SameLine();
			if (ImGui::Button( "+##religion", ImVec2( 18.f, 18.f ) )) {
				religionChanged = true;
				if (m_map->m_curViewingCity->m_owner) {
					m_map->m_curViewingCity->m_owner->AddReligionPopulation( -m_map->m_curViewingCity->m_totalPopulation, m_map->m_curViewingCity->m_religions );
				}
				m_map->m_curViewingCity->m_religions.push_back( std::pair<Religion*, float>( unselectedReligions[0], 0.01f ) );
				m_map->m_curViewingCity->SqueezeReligionInfluence( unselectedReligions[0], 0.01f, 0.f );
				m_map->m_curViewingCity->GetUnselectedReligions( unselectedReligions );
			}
		}
		// remove the last religion in the list
		if ((int)m_map->m_curViewingCity->m_religions.size() > 1) {
			ImGui::SameLine();
			if (ImGui::Button( "-##religion", ImVec2( 18.f, 18.f ) )) {
				religionChanged = true;
				if (m_map->m_curViewingCity->m_owner) {
					m_map->m_curViewingCity->m_owner->AddReligionPopulation( -m_map->m_curViewingCity->m_totalPopulation, m_map->m_curViewingCity->m_religions );
				}
				m_map->m_curViewingCity->SqueezeReligionInfluence( m_map->m_curViewingCity->m_religions.back().first,
					-m_map->m_curViewingCity->m_religions.back().second, m_map->m_curViewingCity->m_religions.back().second );
				m_map->m_curViewingCity->m_religions.pop_back();
				m_map->m_curViewingCity->GetUnselectedReligions( unselectedReligions );
			}
		}
		for (auto& pair : m_map->m_curViewingCity->m_religions) {
			std::vector<Religion*> thisUnselectedReligions;
			thisUnselectedReligions.push_back( pair.first );
			for (auto religion : unselectedReligions) {
				thisUnselectedReligions.push_back( religion );
			}
			int religionIndex = 0;
			float windowWidth = ImGui::GetWindowSize().x;
			ImGui::PushItemWidth( -windowWidth * 0.5f );
			// drop down menu to choose a eligible religion
			if (ImGui::Combo( Stringf( "##religion choose %d", pair.first->m_id ).c_str(), &religionIndex, []( void* vector, int idx ) {return ((std::vector<Religion*>*)vector)->at( (size_t)idx )->m_name.c_str(); }, (void*)&thisUnselectedReligions, (int)thisUnselectedReligions.size() )) {
				if (!religionChanged) {
					if (m_map->m_curViewingCity->m_owner) {
						m_map->m_curViewingCity->m_owner->AddReligionPopulation( -m_map->m_curViewingCity->m_totalPopulation, m_map->m_curViewingCity->m_religions );
					}
					religionChanged = true;
				}
				pair.first = thisUnselectedReligions[religionIndex];
			}
			ImGui::PopItemWidth();

			ImGui::SameLine();
			ImGui::SetCursorPosX( windowWidth * 0.5f );
			//ImGui::SameLine();
			// ImGui::Text( pair.first->m_name.c_str() );
			// change the influence
			float prevValue = pair.second;
			if (ImGui::InputFloat( Stringf( "   #religion influence %d", pair.first->m_id ).c_str(), &pair.second )) {
				if (!religionChanged) {
					if (m_map->m_curViewingCity->m_owner) {
						m_map->m_curViewingCity->m_owner->AddReligionPopulation( -m_map->m_curViewingCity->m_totalPopulation, m_map->m_curViewingCity->m_religions );
					}
					religionChanged = true;
				}
				pair.second = GetClamped( pair.second, 0.f, 1.f );
				m_map->m_curViewingCity->SqueezeReligionInfluence( pair.first, pair.second - prevValue, prevValue );
			}
		}

		if (religionChanged) {
			needChangeEconomy = true;
			// recalculate major religion
			m_map->m_curViewingCity->RecalculateMajorReligion();
			// change the major religion and push the result to the owner country
			if (m_map->m_curViewingCity->m_owner) {
				m_map->m_curViewingCity->m_owner->AddReligionPopulation( m_map->m_curViewingCity->m_totalPopulation, m_map->m_curViewingCity->m_religions );
				m_map->m_curViewingCity->m_owner->CalculateMajorReligion();
			}
		}

		// culture
		ImGui::Text( "Cultures" );
		bool cultureChanged = false;
		std::vector<Culture*> unselectedCultures;
		m_map->m_curViewingCity->GetUnselectedCultures( unselectedCultures );
		// add a culture to the last of the list
		if ((int)unselectedCultures.size() > 0) {
			ImGui::SameLine();
			if (ImGui::Button( "+##culture", ImVec2( 18.f, 18.f ) )) {
				cultureChanged = true;
				if (m_map->m_curViewingCity->m_owner) {
					m_map->m_curViewingCity->m_owner->AddCulturePopulation( -m_map->m_curViewingCity->m_totalPopulation, m_map->m_curViewingCity->m_cultures );
				}
				m_map->m_curViewingCity->m_cultures.push_back( std::pair<Culture*, float>( unselectedCultures[0], 0.01f ) );
				m_map->m_curViewingCity->SqueezeCultureInfluence( unselectedCultures[0], 0.01f, 0.f );
				m_map->m_curViewingCity->GetUnselectedCultures( unselectedCultures );
			}
		}
		// remove the last culture in the list
		if ((int)m_map->m_curViewingCity->m_cultures.size() > 1) {
			ImGui::SameLine();
			if (ImGui::Button( "-##culture", ImVec2( 18.f, 18.f ) )) {
				cultureChanged = true;
				if (m_map->m_curViewingCity->m_owner) {
					m_map->m_curViewingCity->m_owner->AddCulturePopulation( -m_map->m_curViewingCity->m_totalPopulation, m_map->m_curViewingCity->m_cultures );
				}
				m_map->m_curViewingCity->SqueezeCultureInfluence( m_map->m_curViewingCity->m_cultures.back().first,
					-m_map->m_curViewingCity->m_cultures.back().second, m_map->m_curViewingCity->m_cultures.back().second );
				m_map->m_curViewingCity->m_cultures.pop_back();
				m_map->m_curViewingCity->GetUnselectedCultures( unselectedCultures );
			}
		}
		for (auto& pair : m_map->m_curViewingCity->m_cultures) {
			std::vector<Culture*> thisUnselectedCultures;
			thisUnselectedCultures.push_back( pair.first );
			for (auto culture : unselectedCultures) {
				thisUnselectedCultures.push_back( culture );
			}
			int cultureIndex = 0;
			float windowWidth = ImGui::GetWindowSize().x;
			ImGui::PushItemWidth( -windowWidth * 0.5f );
			// drop down menu to choose a eligible culture
			if (ImGui::Combo( Stringf( "##culture choose %d", pair.first->m_id ).c_str(), &cultureIndex,
				[]( void* vector, int idx ) {return ((std::vector<Culture*>*)vector)->at( (size_t)idx )->m_name.c_str(); },
				(void*)&thisUnselectedCultures, (int)thisUnselectedCultures.size() )) {
				if (!cultureChanged) {
					if (m_map->m_curViewingCity->m_owner) {
						m_map->m_curViewingCity->m_owner->AddCulturePopulation( -m_map->m_curViewingCity->m_totalPopulation, m_map->m_curViewingCity->m_cultures );
					}
					cultureChanged = true;
				}
				pair.first = thisUnselectedCultures[cultureIndex];
			}

			ImGui::PopItemWidth();

			ImGui::SameLine();
			ImGui::SetCursorPosX( windowWidth * 0.5f );
			// ImGui::Text( pair.first->m_name.c_str() );
			// change the influence
			float prevValue = pair.second;
			if (ImGui::InputFloat( Stringf( "   #culture influence %d", pair.first->m_id ).c_str(), &pair.second )) {
				if (!cultureChanged) {
					if (m_map->m_curViewingCity->m_owner) {
						m_map->m_curViewingCity->m_owner->AddCulturePopulation( -m_map->m_curViewingCity->m_totalPopulation, m_map->m_curViewingCity->m_cultures );
					}
					cultureChanged = true;
				}
				pair.second = GetClamped( pair.second, 0.f, 1.f );
				m_map->m_curViewingCity->SqueezeCultureInfluence( pair.first, pair.second - prevValue, prevValue );
			}
		}
		if (cultureChanged) {
			needChangeEconomy = true;
			// recalculate major culture
			m_map->m_curViewingCity->RecalculateMajorCulture();
			// change the major culture and push the result to the owner country
			if (m_map->m_curViewingCity->m_owner) {
				m_map->m_curViewingCity->m_owner->AddCulturePopulation( m_map->m_curViewingCity->m_totalPopulation, m_map->m_curViewingCity->m_cultures );
				m_map->m_curViewingCity->m_owner->CalculateMajorCulture();
			}
		}

		// attributes
		ImGui::Text( "Attributes" );
		bool attributesChanged = false;
		std::vector<CityAttribute> unselectedAttributes;
		m_map->m_curViewingCity->GetUnownedAttributes( unselectedAttributes );
		// add a culture to the last of the list
		if ((int)unselectedAttributes.size() > 0) {
			ImGui::SameLine();
			if (ImGui::Button( "+##attr", ImVec2( 18.f, 18.f ) )) {
				attributesChanged = true;
				m_map->m_curViewingCity->AddAttribute( unselectedAttributes[0] );
				m_map->m_curViewingCity->GetUnownedAttributes( unselectedAttributes );
			}
		}
		// remove the last culture in the list
		if ((int)m_map->m_curViewingCity->m_attributes.size() > 1) {
			ImGui::SameLine();
			if (ImGui::Button( "-##attr", ImVec2( 18.f, 18.f ) )) {
				attributesChanged = true;
				m_map->m_curViewingCity->RemoveAttribute( m_map->m_curViewingCity->m_attributes[m_map->m_curViewingCity->m_attributes.size() - 1] );
				m_map->m_curViewingCity->GetUnownedAttributes( unselectedAttributes );
			}
		}
		for (auto attr : m_map->m_curViewingCity->m_attributes) {
			std::vector<CityAttribute> thisUnselectedAttrs;
			thisUnselectedAttrs.push_back( attr );
			for (auto unselectedAttr : unselectedAttributes) {
				thisUnselectedAttrs.push_back( unselectedAttr );
			}
			int attrIndex = 0;
			// drop down menu to choose a eligible culture
			if (ImGui::Combo( Stringf( "##attr choose %d", (int)attr ).c_str(), &attrIndex,
				[]( void* vector, int idx ) {return g_cityAttributeNameMapC[(int)((std::vector<CityAttribute>*)vector)->at( (size_t)idx )]; },
				(void*)&thisUnselectedAttrs, (int)thisUnselectedAttrs.size() )) {
				attributesChanged = true;
				m_map->m_curViewingCity->RemoveAttribute( thisUnselectedAttrs[0] );
				m_map->m_curViewingCity->AddAttribute( thisUnselectedAttrs[attrIndex] );
			}

		}
		if (attributesChanged) {
			needChangeEconomy = true;
			// if set a capital, reset the capital
			if (m_map->m_curViewingCity->HasAttribute( CityAttribute::Capital )) {
				if (m_map->m_curViewingCity->m_owner) {
					if (m_map->m_curViewingCity->m_owner->m_capitalCity) {
						if (m_map->m_curViewingCity->m_owner->m_capitalProv == m_map->m_curViewingCity->m_provIn) {

						}
						
						m_map->m_curViewingCity->m_owner->m_capitalCity->RemoveAttribute( CityAttribute::Capital );
					}
					m_map->m_curViewingCity->m_owner->m_capitalCity = m_map->m_curViewingCity;
					m_map->m_curViewingCity->m_owner->m_capitalProv = m_map->m_curViewingCity->m_provIn;
				}
			}
			// recalculate the economy of the owner country

			// reset max defense

		}

		ImGui::PopItemWidth();

		if (needChangeEconomy && m_map->m_curViewingCity->m_owner) {
			m_map->m_curViewingCity->m_owner->CalculateEconomicValue();
		}

		/*ImGui::Text(Stringf("Name: %s", m_map->m_curViewingCity->m_name.c_str()).c_str());
		ImGui::Text( Stringf( "Debug ID: %d", m_map->m_curViewingCity->m_id ).c_str() );
		ImGui::Text( Stringf( "Population: %d", m_map->m_curViewingCity->m_totalPopulation ).c_str() ); 
		ImGui::Text( Stringf( "Defense: %.0f", m_map->m_curViewingCity->m_defense ).c_str() );
		if (m_map->m_curViewingCity->m_owner) {
			ImGui::Text( Stringf( "Owner: %s", m_map->m_curViewingCity->m_owner->m_name.c_str() ).c_str() );
		}

		for (int i = 0; i < (int)m_map->m_curViewingCity->m_religions.size(); i++) {
			ImGui::Text( Stringf( "Religion %s: %.3f", m_map->m_curViewingCity->m_religions[i].first->m_name.c_str(), m_map->m_curViewingCity->m_religions[i].second ).c_str() );
		}
		for (int i = 0; i < (int)m_map->m_curViewingCity->m_cultures.size(); i++) {
			ImGui::Text( Stringf( "Culture %s: %.3f", m_map->m_curViewingCity->m_cultures[i].first->m_name.c_str(), m_map->m_curViewingCity->m_cultures[i].second ).c_str() );
		}

		if (m_map->m_curViewingCity->m_type & CITY_FLAG_CAPITAL) {
			ImGui::Text( "Capital" );
		}
		if (m_map->m_curViewingCity->m_type & CITY_FLAG_COMMERCIAL) {
			ImGui::Text( "Commercial" );
		}
		if (m_map->m_curViewingCity->m_type & CITY_FLAG_FORT) {
			ImGui::Text( "Fort" );
		}
		if (m_map->m_curViewingCity->m_type & CITY_FLAG_HOLY_CITY) {
			ImGui::Text( "Holy City" );
		}
		if (m_map->m_curViewingCity->m_type & CITY_FLAG_NOMAD) {
			ImGui::Text( "Nomad" );
		}
		if (m_map->m_curViewingCity->m_type & CITY_FLAG_POLITICAL_CENTER) {
			ImGui::Text( "Political Center" );
		}
		if (m_map->m_curViewingCity->m_type & CITY_FLAG_PORT) {
			ImGui::Text( "Port" );
		}
		if (m_map->m_curViewingCity->m_type & CITY_FLAG_TRIBE) {
			ImGui::Text( "Tribe" );
		}
		if (m_map->m_curViewingCity->m_type & CITY_FLAG_ADJ_RIVER) {
			ImGui::Text( "Adjacent to River" );
		}
		if (m_map->m_curViewingCity->m_type & CITY_FLAG_ADJ_SEA) {
			ImGui::Text( "Adjacent to Sea" );
		}*/

		ImGui::PopItemWidth();
		ImGui::End();
	}
}

void Game::UpdateArmyUI()
{
	if (m_map && m_map->m_curViewingArmy) {
		ImGui::Begin( "Army", &m_map->m_showArmyPanel, 0 );
		if (!m_map->m_showArmyPanel)
		{
			m_map->m_curViewingArmy = nullptr;
			ImGui::End();
			return;
		}
		ImGui::PushItemWidth( ImGui::GetFontSize() * -1 );

		//ImGui::Text( Stringf( "Name: %s", m_map->m_curViewingArmy->m_name.c_str() ).c_str() );
		//ImGui::Text( Stringf( "Debug ID: %d", m_map->m_curViewingArmy->m_id ).c_str() );
		ImGui::Text( "Size         " ); ImGui::SameLine();
		if (ImGui::InputInt( "   ##ArmySize", &m_map->m_curViewingArmy->m_size, 0, 0 )) {
			if (m_map->m_curViewingArmy->m_owner) {
				m_map->m_curViewingArmy->m_owner->CalculateMilitaryStats();
			}
		}

		ImGui::Text( Stringf( "Owner          %s", m_map->m_curViewingArmy->m_owner->m_name.c_str() ).c_str() );
		/*
		ImGui::Text( Stringf( "Size: %d", m_map->m_curViewingArmy->m_size ).c_str() );
		ImGui::Text( Stringf( "Owner: %s", m_map->m_curViewingArmy->m_owner->m_name.c_str() ).c_str() );
		*/

		ImGui::PopItemWidth();
		ImGui::End();
	}
}

void Game::UpdateSaveHistoryPopupUI()
{
	if (m_map && m_map->m_historySavingModule && m_map->m_historySavingModule->IsSaving()) {
		if (!ImGui::Begin( "Saving Progress", NULL, 0 ))
		{
			ImGui::End();
			return;
		}
		ImGui::PushItemWidth( ImGui::GetFontSize() * 5 );

		int totalMonth, curMonth = 0;
		m_map->m_historySavingModule->GetSavingProgress( curMonth, totalMonth );
		ImGui::Text( "Saving History Files ..." );
		ImGui::Text( Stringf( "Progress (%d/%d) ...", curMonth, totalMonth ).c_str() );
		
		ImGui::PopItemWidth();
		ImGui::End();
	}
}

void Game::SaveCurrentGenerationSettings()
{
	XmlDocument savedSettings;
	XmlElement* rootElem = savedSettings.NewElement( "GenerationSettings" );
	savedSettings.InsertFirstChild( rootElem );
	rootElem->SetAttribute( "Seed", m_map->m_generationSettings.m_seed );
	rootElem->SetAttribute( "PolygonAmount", m_map->m_generationSettings.m_basePolygons );
	rootElem->SetAttribute( "DimensionsX", m_map->m_generationSettings.m_dimensions.x );
	rootElem->SetAttribute( "DimensionsY", m_map->m_generationSettings.m_dimensions.y );
	rootElem->SetAttribute( "MinHeight", m_map->m_generationSettings.m_minHeight );
	rootElem->SetAttribute( "MaxHeight", m_map->m_generationSettings.m_maxHeight );
	rootElem->SetAttribute( "FragmentFactor", m_map->m_generationSettings.m_fragmentFactor );
	rootElem->SetAttribute( "OceanFactor", m_map->m_generationSettings.m_landRichnessFactor );
	rootElem->SetAttribute( "NumOfCultures", m_map->m_generationSettings.m_numOfCultures );
	rootElem->SetAttribute( "NumOfReligions", m_map->m_generationSettings.m_numOfReligions );
	rootElem->SetAttribute( "UseWesternEuropeanCountryNames", m_map->m_generationSettings.m_onlyUseWesternCountryPrefix );

	std::filesystem::create_directory( Stringf( "Saves/%d", m_map->m_generationSettings.m_seed ).c_str() );
	XmlError errorCode = savedSettings.SaveFile( Stringf( "Saves/%d/GenerationSettings.xml", m_map->m_generationSettings.m_seed ).c_str() );
	if (errorCode != tinyxml2::XML_SUCCESS) {
		ERROR_RECOVERABLE( "Cannot save settings" );
	}
}

void Game::LoadGenerationSettings() {
	std::string filePath;
	g_window->ChooseFileFromBroser( filePath );
	XmlDocument genDocument;
	genDocument.LoadFile( filePath.c_str() );
	XmlElement* rootElem = genDocument.RootElement();
	if (rootElem) {
		m_generationSettings.m_seed = ParseXmlAttribute( *rootElem, "Seed", m_generationSettings.m_seed );
		m_generationSettings.m_basePolygons = ParseXmlAttribute( *rootElem, "PolygonAmount", m_generationSettings.m_basePolygons );
		m_generationSettings.m_dimensions.x = ParseXmlAttribute( *rootElem, "DimensionsX", m_generationSettings.m_dimensions.x );
		m_generationSettings.m_dimensions.y = ParseXmlAttribute( *rootElem, "DimensionsY", m_generationSettings.m_dimensions.y );
		m_generationSettings.m_minHeight = ParseXmlAttribute( *rootElem, "MinHeight", m_generationSettings.m_minHeight );
		m_generationSettings.m_maxHeight = ParseXmlAttribute( *rootElem, "MaxHeight", m_generationSettings.m_maxHeight );
		m_generationSettings.m_fragmentFactor = ParseXmlAttribute( *rootElem, "FragmentFactor", m_generationSettings.m_fragmentFactor );
		m_generationSettings.m_landRichnessFactor = ParseXmlAttribute( *rootElem, "OceanFactor", m_generationSettings.m_landRichnessFactor );
		m_generationSettings.m_numOfCultures = ParseXmlAttribute( *rootElem, "NumOfCultures", m_generationSettings.m_numOfCultures );
		m_generationSettings.m_numOfReligions = ParseXmlAttribute( *rootElem, "NumOfReligions", m_generationSettings.m_numOfReligions );
		m_generationSettings.m_onlyUseWesternCountryPrefix = ParseXmlAttribute( *rootElem, "UseWesternEuropeanCountryNames", m_generationSettings.m_onlyUseWesternCountryPrefix );
	}
}

void Game::ClearSaves()
{
	std::filesystem::remove_all( "Saves" );
}

void Game::RenderAttractScreen() const
{
	std::vector<Vertex_PCU> verts;
	float uMax = 0.98f;
	float uMin = 0.02f;
	float vMax = 0.98f;
	float vMin = 0.02f;
	if (m_map) {
		if (m_loadingTimer->HasStartedAndNotPeriodElapsed()) {
			Rgba8 color1 = Rgba8( 255, 255, 255, (unsigned char)(255.f * SmoothStop2( 1.f - m_loadingTimer->GetElapsedFraction() )) );
			Rgba8 color2 = Rgba8( 255, 255, 255);
			float speed = m_screenCamera.m_cameraBox.GetDimensions().GetLength() * 0.4f;
			float fraction = SmoothStart2( m_loadingTimer->GetElapsedFraction() );
			Vec2 BLPos = Vec2( m_screenCamera.GetCenter().x, m_screenCamera.m_cameraBox.m_mins.y ) + Vec2(-speed, -speed * 0.5f) * fraction;
			Vec2 TLPos = Vec2( m_screenCamera.GetCenter().x, m_screenCamera.m_cameraBox.m_maxs.y ) + Vec2(-speed, speed * 0.5f) * fraction;
			Vec2 BRPos = Vec2( m_screenCamera.GetCenter().x, m_screenCamera.m_cameraBox.m_mins.y ) + Vec2(speed, -speed * 0.5f) * fraction;
			Vec2 TRPos = Vec2( m_screenCamera.GetCenter().x, m_screenCamera.m_cameraBox.m_maxs.y ) + Vec2(speed, speed * 0.5f) * fraction;
			verts.emplace_back( Vec2( m_screenCamera.m_cameraBox.m_mins.x, m_screenCamera.m_cameraBox.m_maxs.y ), color2, Vec2( uMin, vMax ) );
			verts.emplace_back(m_screenCamera.m_cameraBox.m_mins, color2, Vec2( uMin, vMin ) );
			verts.emplace_back( BLPos, color1, Vec2( 0.5f, vMin ) );

			verts.emplace_back( Vec2( m_screenCamera.m_cameraBox.m_mins.x, m_screenCamera.m_cameraBox.m_maxs.y ), color2, Vec2( uMin, vMax ) );
			verts.emplace_back( BLPos, color1, Vec2( 0.5f, vMin ) );
			verts.emplace_back( TLPos, color1, Vec2( 0.5f, vMax ) );

			verts.emplace_back( TRPos, color1, Vec2( 0.5f, vMax ) );
			verts.emplace_back( BRPos, color1, Vec2( 0.5f, vMin ) );
			verts.emplace_back( Vec2( m_screenCamera.m_cameraBox.m_maxs.x, m_screenCamera.m_cameraBox.m_mins.y ), color2, Vec2( uMax, vMin ) );

			verts.emplace_back( TRPos, color1, Vec2( 0.5f, vMax ) );
			verts.emplace_back( Vec2( m_screenCamera.m_cameraBox.m_maxs.x, m_screenCamera.m_cameraBox.m_mins.y ), color2, Vec2( uMax, vMin ) );
			verts.emplace_back( m_screenCamera.m_cameraBox.m_maxs, color2, Vec2( uMax, vMax ) );

			g_theRenderer->BindShader( nullptr );
			g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Ornate Endpaper.png" ) );
			g_theRenderer->SetModelConstants();
			g_theRenderer->DrawVertexArray( verts );
		}
	}
	else {
		AddVertsForAABB2D( verts, AABB2( m_screenCamera.m_cameraBox.m_mins, Vec2( m_screenCamera.GetCenter().x, m_screenCamera.m_cameraBox.m_maxs.y ) ), Rgba8::WHITE, AABB2( Vec2( uMin, vMin ), Vec2( 0.5f, vMax ) ) );
		AddVertsForAABB2D( verts, AABB2( Vec2( m_screenCamera.GetCenter().x, m_screenCamera.m_cameraBox.m_mins.y ), m_screenCamera.m_cameraBox.m_maxs ), Rgba8::WHITE, AABB2( Vec2( 0.5f, vMin ), Vec2( uMax, vMax ) ) );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Ornate Endpaper.png" ) );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( verts );

		std::vector<Vertex_PCU> textVerts;
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 500.f ), Vec2( 1300.f, 730.f ) ), 80.f, "Procedural\nWorld\nGenerator", Rgba8( 153, 0, 0 ) );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( textVerts );
	}
}
