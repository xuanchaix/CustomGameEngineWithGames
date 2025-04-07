#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/VoronoiHelper.hpp"
#include "Game/MapPolygonUnit.hpp"
#include "Game/River.hpp"
#include "Game/Culture.hpp"
#include "Game/Religion.hpp"
#include "Game/City.hpp"
#include "Game/Country.hpp"
#include "Game/Continent.hpp"
#include "Game/Region.hpp"
#include "Game/AStarHelper.hpp"
#include "Game/Road.hpp"
#include "Game/label.hpp"
#include "Game/NameGenerator.hpp"
#include "Game/Army.hpp"
#include "Game/CountryInstructions.hpp"
#include "Game/Battle.hpp"
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <iostream>
#include <thread>

class SaveHistoryJob :public Job {
public:
	SaveHistoryJob( HistorySavingSolver* solver ):m_solver(solver) {};
	virtual ~SaveHistoryJob() {};
	virtual void Execute() override;

	HistorySavingSolver* m_solver = nullptr;
};

void SaveHistoryJob::Execute()
{
	m_solver->StartSave();
	m_solver->Update();
}

class PolyUnitDataCalculationJob :public Job {
public:
	PolyUnitDataCalculationJob( MapPolygonUnit* unit ) { m_unit = unit; };
	virtual ~PolyUnitDataCalculationJob() {};
	virtual void Execute() override;

	MapPolygonUnit* m_unit = nullptr;
};

void PolyUnitDataCalculationJob::Execute()
{
	m_unit->InitializeUnit3();
}

class FindCountryForProvinceJob :public Job {
public:
	FindCountryForProvinceJob( MapPolygonUnit* prov, int cultureState, std::vector<Country*>& generatedCountries )
		: m_generatedCountries( generatedCountries )
		, m_prov( prov )
		, m_cultureState( cultureState ) {};
	virtual ~FindCountryForProvinceJob() {};
	virtual void Execute() override;
	int m_cultureState = 1;
	std::vector<Country*>& m_generatedCountries;
	MapPolygonUnit* m_prov;
};

void FindCountryForProvinceJob::Execute()
{
	// if there is a center country, the center country is more powerful
	if (m_cultureState == 5) {
		Country* nearestCountry = nullptr;
		float nearestDist = FLT_MAX;
		for (int i = 0; i < (int)m_generatedCountries.size(); i++) {
			MapPolygonUnit* startUnit = m_generatedCountries[i]->m_provinces[0];
			float thisDist = 0.f;
			if (i == 0) {
				//thisDist = GetDistanceSquared2D( startUnit->m_geoCenterPos, prov->m_geoCenterPos ) * 0.25f;
				thisDist = startUnit->CalculateGeometricDistance( m_prov ) * 0.5f;
			}
			else {
				//thisDist = GetDistanceSquared2D( startUnit->m_geoCenterPos, prov->m_geoCenterPos );
				thisDist = startUnit->CalculateGeometricDistance( m_prov );
			}
			if (thisDist < nearestDist) {
				nearestDist = thisDist;
				nearestCountry = m_generatedCountries[i];
			}
		}
		nearestCountry->GainProvince( m_prov );
	}
	else {
		// give to the nearest country
		Country* nearestCountry = nullptr;
		float nearestDist = FLT_MAX;
		for (auto country : m_generatedCountries) {
			MapPolygonUnit* startUnit = country->m_provinces[0];
			//float thisDist = GetDistanceSquared2D( startUnit->m_geoCenterPos, prov->m_geoCenterPos );
			float thisDist = startUnit->CalculateGeometricDistance( m_prov );
			if (thisDist < nearestDist) {
				nearestDist = thisDist;
				nearestCountry = country;
			}
		}
		// find a qualified country
		nearestCountry->GainProvince( m_prov );
	}
}

class LoadHistoryFromDiskJob : public Job {
public:
	LoadHistoryFromDiskJob( std::string const& path, int year, int month, Map* map )
		: m_path( path )
		, m_year( year )
		, m_month( month )
		, m_map( map ) {
		m_type = Loading_Job;
	};
	virtual ~LoadHistoryFromDiskJob() {};
	virtual void Execute() override;
	std::string m_path;
	int m_year = 0;
	int m_month = 0;
	Map* m_map = nullptr;
};

void LoadHistoryFromDiskJob::Execute()
{
	HistoryData* data = new HistoryData();
	m_map->LoadHistoryCacheFromDisk( *data, m_year, m_month );
	m_map->m_saveLoadMutex.lock();
	m_map->m_historyData[m_year * 12 + m_month - 1] = data;
	m_map->m_saveLoadMutex.unlock();
}

Map::Map( MapGenerationSettings const& settings )
	:m_generationSettings(settings)
{
	m_polygonInspectionEdgeBlurringTimer = new Timer( 1.6f, Clock::GetSystemClock() );
}

Map::~Map()
{
	auto t = std::time( nullptr );
	tm time;
#if defined(_MSC_VER)
	localtime_s( &time, &t );
#else
	time = *std::localtime( &t );
#endif
	std::ostringstream oss;
	oss << std::put_time( &time, "%Y_%m_%d_%H_%M_%S" );
	auto str = oss.str();
	StringWriteToFile( m_generationLog, Stringf( "Log/GenerationLog_%s.log", str.c_str() ) );
	std::string historyLogStr = "";
	for (auto const& sstr : m_historyLog) {
		historyLogStr += sstr;
	}
	StringWriteToFile( historyLogStr, Stringf( "Saves/%d/HistoryLog.log", m_generationSettings.m_seed ) );

	delete m_historySavingModule;
	delete m_polygonInspectionEdgeBlurringTimer;
	delete m_2D3DSwitchTimer;
	for (int i = 0; i < (int)m_mapPolygonUnits.size(); i++) {
		delete m_mapPolygonUnits[i];
	}
	for (int i = 0; i < (int)m_rivers.size(); i++) {
		delete m_rivers[i];
	}
	for (int i = 0; i < (int)m_cultures.size(); i++) {
		delete m_cultures[i];
	}
	for (int i = 0; i < (int)m_continents.size(); i++) {
		delete m_continents[i];
	}
	for (int i = 0; i < (int)m_religions.size(); i++) {
		delete m_religions[i];
	}
	for (int i = 0; i < (int)m_cities.size(); i++) {
		delete m_cities[i];
	}
	for (int i = 0; i < (int)m_towns.size(); i++) {
		delete m_towns[i];
	}
	for (int i = 0; i < (int)m_countries.size(); i++) {
		delete m_countries[i];
	}
	for (int i = 0; i < (int)m_regions.size(); i++) {
		delete m_regions[i];
	}
	for (int i = 0; i < (int)m_cityLabels.size(); i++) {
		delete m_cityLabels[i];
	}
	for (int i = 0; i < (int)m_townLabels.size(); i++) {
		delete m_townLabels[i];
	}
	for (int i = 0; i < (int)m_countryLabels.size(); i++) {
		delete m_countryLabels[i];
	}
	for (int i = 0; i < (int)m_cultureLabels.size(); i++) {
		delete m_cultureLabels[i];
	}
	for (int i = 0; i < (int)m_religionLabels.size(); i++) {
		delete m_religionLabels[i];
	}
	for (int i = 0; i < (int)m_regionLabels.size(); i++) {
		delete m_regionLabels[i];
	}
	for (int i = 0; i < (int)m_continentLabels.size(); i++) {
		delete m_continentLabels[i];
	}
	for (auto historyData : m_historyData) {
		delete historyData;
	}

	delete m_polygonuv3DVertexBuffer;
	delete m_mapBorderVertexBuffer;
	delete m_polygonEdgesIndexBuffer;
	delete m_polygonEdgesIndexBuffer3D;
	delete m_polygonsEdgesVertexBuffer3D;
	delete m_polygonsFaces3DVertexBuffer;
	delete m_polygonsEdgesVertexBuffer;
	delete m_edgeBlockVertexBuffer;
	delete m_polygonCenterVertexBuffer;
	delete m_polygonsFacesVertexBuffer;
	delete m_mapRNG;
	delete m_productRNG;
	delete m_polygonFacesColorVertexBuffer;
	delete m_polygonuvVertexBuffer;
	delete m_riverVertexBuffer;
	delete m_cityVertexBuffer;
	delete m_townVertexBuffer;
	delete m_countriesEdgesVertexBuffer;
	delete m_roadVertexBuffer;
	delete m_historyRNG;
	delete m_religionNameGenerator;
	delete m_regionContinentNameGenerator;
	delete m_countriesEdgesVertexBuffer3D;
	delete m_polygonsNormalVertexBuffer;
	delete m_provEditVertexBuffer;
	delete m_provEditHighLightVertexBuffer;
}

std::vector<MapPolygonUnit*> route;

void Map::Startup()
{
	double allStartTime = GetCurrentTimeSeconds();
	StartUpMap();

	PCGWorld_Log( "Populating Map..." );
	double startTime = GetCurrentTimeSeconds();
	PopulateMapWithPolygons( m_generationSettings.m_basePolygons );
	double endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Populating Map finished, time: %.3fs", endTime - startTime ) );

	PCGWorld_Log( "Calculating Polygon Data..." );
	startTime = GetCurrentTimeSeconds();
	CalculateBiomeDataForPolygons();
	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Calculating Polygon Data finished, time: %.3fs", endTime - startTime ) );

	PCGWorld_Log( "Generating Rivers..." );
	startTime = GetCurrentTimeSeconds();
	GenerateRivers();
	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Generating Rivers finished, time: %.3fs", endTime - startTime ) );

	/*PCGWorld_Log("Generating Forests...");
	startTime = GetCurrentTimeSeconds();
	GenerateForests();
	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Generating Forests finished, time: %.3fs", endTime - startTime ) );*/

	PCGWorld_Log( "Generating Continents..." );
	startTime = GetCurrentTimeSeconds();
	GenerateContinents();
	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Generating Continents finished, time: %.3fs", endTime - startTime ) );

	PCGWorld_Log( "Generating Humans..." );
	startTime = GetCurrentTimeSeconds();
	GenerateHumanDataForPolygons();
	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Generating Humans finished, time: %.3fs", endTime - startTime ) );

	PCGWorld_Log( "Creating Vertex Buffers..." );
	startTime = GetCurrentTimeSeconds();
	GenerateVertexBuffers();
	InitializeLabels();
	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Creating Vertex Buffers finished, time: %.3fs", endTime - startTime ) );

	SetRenderCountryMapMode();
	double allEndTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "All generation finished, time: %.3fs", allEndTime - allStartTime ) );

	for (auto country : m_countries) {
		country->CalculateEconomicValue();
		country->CalculateMilitaryStats();
	}

	if (m_generationSettings.m_enableHistorySimulation) {
		SetUpHistorySimulation();
	}
	// A* test
	//m_aStarHelper.CalculateRoute( m_mapPolygonUnits[1762], m_mapPolygonUnits[3224], route );
	//float a = m_mapPolygonUnits[4732]->CalculateGeometricDistance( m_mapPolygonUnits[4496] );
	//float bb = m_mapPolygonUnits[4732]->CalculateGeometricDistance( m_mapPolygonUnits[4615] );
	//m_aStarHelper.CalculateRoute( m_mapPolygonUnits[4615], m_mapPolygonUnits[4732], route );
}

void Map::Update()
{
	HandleKeys();
	// update the color array
	UpdateBufferColors();
	
	UpdateEdges();

	UpdateSwitching2D3D();
	// update labels
	UpdateLabels();

	UpdateAutoSimulation();

	UpdateSaveStatus();
}

void Map::Render() const
{
	if (g_theGame->m_viewMode == MapViewMode::ViewMode2D) {
		Render2D();
	}
	else if (g_theGame->m_viewMode == MapViewMode::ViewMode3D) {
		Prepare2DMapFor3D();
		Render3D();
	}
	else if (g_theGame->m_viewMode == MapViewMode::ViewModeSphere) {
		Prepare2DMapForSphere();
		RenderSphere();
	}
	
}

Vec2 Map::GetDimensions() const
{
	return m_dimensions;
}

MapGenerationSettings const& Map::GetGenerationSettings() const
{
	return m_generationSettings;
}

unsigned int Map::GetSeed() const
{
	return m_generationSettings.m_seed;
}

void Map::SetRenderHeightMapMode( bool setRender )
{
	if (setRender && !(m_showingSettings.m_mapShowConfig & SHOW_CONFIG_HEIGHT_MAP)) {
		m_showingSettings.m_mapShowConfig = (m_showingSettings.m_mapShowConfig & ~SHOW_CONFIG_COLORFUL_MAP) | SHOW_CONFIG_HEIGHT_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderHeightColor();
		}
		if (m_curViewingUnit) {
			m_curViewingUnit->SetRenderViewingColor( true );
		}
	}
	else if (!setRender) {
		m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_HEIGHT_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderNoColor();
		}
	}
}

void Map::SetRenderClimateMapMode( bool setRender /*= true */ )
{
	if (setRender && !(m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CLIMATE_MAP)) {
		m_showingSettings.m_mapShowConfig = (m_showingSettings.m_mapShowConfig & ~SHOW_CONFIG_COLORFUL_MAP) | SHOW_CONFIG_CLIMATE_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderClimateColor();
		}
		if (m_curViewingUnit) {
			m_curViewingUnit->SetRenderViewingColor( true );
		}
	}
	else if (!setRender) {
		m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_CLIMATE_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderNoColor();
		}
	}
}

void Map::SetRenderPrecipitationMapMode( bool setRender /*= true */, bool summer )
{
	if (summer) {
		if (setRender && !(m_showingSettings.m_mapShowConfig & SHOW_CONFIG_SUMMER_PRECIPITATION_MAP)) {
			m_showingSettings.m_mapShowConfig = (m_showingSettings.m_mapShowConfig & ~SHOW_CONFIG_COLORFUL_MAP) | SHOW_CONFIG_SUMMER_PRECIPITATION_MAP;
			for (auto unit : m_mapPolygonUnits) {
				unit->SetRenderPrecipitationColor( summer );
			}
			if (m_curViewingUnit) {
				m_curViewingUnit->SetRenderViewingColor( true );
			}
		}
		else if (!setRender) {
			m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_SUMMER_PRECIPITATION_MAP;
			for (auto unit : m_mapPolygonUnits) {
				unit->SetRenderNoColor();
			}
		}
	}
	else {
		if (setRender && !(m_showingSettings.m_mapShowConfig & SHOW_CONFIG_WINTER_PRECIPITATION_MAP)) {
			m_showingSettings.m_mapShowConfig = (m_showingSettings.m_mapShowConfig & ~SHOW_CONFIG_COLORFUL_MAP) | SHOW_CONFIG_WINTER_PRECIPITATION_MAP;
			for (auto unit : m_mapPolygonUnits) {
				unit->SetRenderPrecipitationColor( summer );
			}
			if (m_curViewingUnit) {
				m_curViewingUnit->SetRenderViewingColor( true );
			}
		}
		else if (!setRender) {
			m_showingSettings.m_mapShowConfig  ^= SHOW_CONFIG_WINTER_PRECIPITATION_MAP;
			for (auto unit : m_mapPolygonUnits) {
				unit->SetRenderNoColor();
			}
		}
	}
}

void Map::SetRenderTemperatureMapMode( bool setRender /*= true*/, bool summer /*= true */ )
{
	if (summer) {
		if (setRender && !(m_showingSettings.m_mapShowConfig & SHOW_CONFIG_SUMMER_TEMPERATURE_MAP)) {
			m_showingSettings.m_mapShowConfig = (m_showingSettings.m_mapShowConfig & ~SHOW_CONFIG_COLORFUL_MAP) | SHOW_CONFIG_SUMMER_TEMPERATURE_MAP;
			for (auto unit : m_mapPolygonUnits) {
				unit->SetRenderTemperatureColor( summer );
			}
			if (m_curViewingUnit) {
				m_curViewingUnit->SetRenderViewingColor( true );
			}
		}
		else if (!setRender) {
			m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_SUMMER_TEMPERATURE_MAP;
			for (auto unit : m_mapPolygonUnits) {
				unit->SetRenderNoColor();
			}
		}
	}
	else {
		if (setRender && !(m_showingSettings.m_mapShowConfig & SHOW_CONFIG_WINTER_TEMPERATURE_MAP)) {
			m_showingSettings.m_mapShowConfig = (m_showingSettings.m_mapShowConfig & ~SHOW_CONFIG_COLORFUL_MAP) | SHOW_CONFIG_WINTER_TEMPERATURE_MAP;
			for (auto unit : m_mapPolygonUnits) {
				unit->SetRenderTemperatureColor( summer );
			}
			if (m_curViewingUnit) {
				m_curViewingUnit->SetRenderViewingColor( true );
			}
		}
		else if (!setRender) {
			m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_WINTER_TEMPERATURE_MAP;
			for (auto unit : m_mapPolygonUnits) {
				unit->SetRenderNoColor();
			}
		}
	}
}

void Map::SetRenderLandformMapMode( bool setRender /*= true */ )
{
	if (setRender && !(m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LANDFORM_MAP)) {
		m_showingSettings.m_mapShowConfig = (m_showingSettings.m_mapShowConfig & ~SHOW_CONFIG_COLORFUL_MAP) | SHOW_CONFIG_LANDFORM_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderLandformColor();
		}
		if (m_curViewingUnit) {
			m_curViewingUnit->SetRenderViewingColor( true );
		}
	}
	else if (!setRender) {
		m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_LANDFORM_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderNoColor();
		}
	}
}

void Map::SetRenderPopulationMapMode( bool setRender /*= true */ )
{
	if (setRender && !(m_showingSettings.m_mapShowConfig & SHOW_CONFIG_POPULATION_MAP)) {
		m_showingSettings.m_mapShowConfig = (m_showingSettings.m_mapShowConfig & ~SHOW_CONFIG_COLORFUL_MAP) | SHOW_CONFIG_POPULATION_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderPopulationColor();
		}
		if (m_curViewingUnit) {
			m_curViewingUnit->SetRenderViewingColor( true );
		}
	}
	else if (!setRender) {
		m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_POPULATION_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderNoColor();
		}
	}
}

void Map::SetRenderCultureMapMode( bool setRender /*= true */ )
{
	if (setRender && !(m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CULTURE_MAP)) {
		m_showingSettings.m_mapShowConfig = (m_showingSettings.m_mapShowConfig & ~SHOW_CONFIG_COLORFUL_MAP) | SHOW_CONFIG_CULTURE_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderCultureColor();
		}
		if (m_curViewingUnit) {
			m_curViewingUnit->SetRenderViewingColor( true );
		}
	}
	else if (!setRender) {
		m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_CULTURE_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderNoColor();
		}
	}
}

void Map::SetRenderReligionMapMode( bool setRender /*= true */ )
{
	if (setRender && !(m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELIGION_MAP)) {
		m_showingSettings.m_mapShowConfig = (m_showingSettings.m_mapShowConfig & ~SHOW_CONFIG_COLORFUL_MAP) | SHOW_CONFIG_RELIGION_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderReligionColor();
		}
		if (m_curViewingUnit) {
			m_curViewingUnit->SetRenderViewingColor( true );
		}
	}
	else if (!setRender) {
		m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_RELIGION_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderNoColor();
		}
	}
}

void Map::SetRenderContinentMapMode( bool setRender /*= true */ )
{
	if (setRender && !(m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CONTINENT_MAP)) {
		m_showingSettings.m_mapShowConfig = (m_showingSettings.m_mapShowConfig & ~SHOW_CONFIG_COLORFUL_MAP) | SHOW_CONFIG_CONTINENT_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderContinentColor();
		}
		if (m_curViewingUnit) {
			m_curViewingUnit->SetRenderViewingColor( true );
		}
	}
	else if (!setRender) {
		m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_CONTINENT_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderNoColor();
		}
	}
}

void Map::SetRenderProductMapMode( bool setRender /*= true */ )
{
	if (setRender && !(m_showingSettings.m_mapShowConfig & SHOW_CONFIG_PRODUCT_MAP)) {
		m_showingSettings.m_mapShowConfig = (m_showingSettings.m_mapShowConfig & ~SHOW_CONFIG_COLORFUL_MAP) | SHOW_CONFIG_PRODUCT_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderProductColor();
		}
		if (m_curViewingUnit) {
			m_curViewingUnit->SetRenderViewingColor( true );
		}
	}
	else if (!setRender) {
		m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_PRODUCT_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderNoColor();
		}
	}
}

void Map::SetRenderCountryMapMode( bool setRender /*= true */ )
{
	if (setRender && !(m_showingSettings.m_mapShowConfig & SHOW_CONFIG_COUNTRIES_MAP)) {
		m_showingSettings.m_mapShowConfig = (m_showingSettings.m_mapShowConfig & ~SHOW_CONFIG_COLORFUL_MAP) | SHOW_CONFIG_COUNTRIES_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderCountryColor();
		}
		if (m_curViewingUnit) {
			m_curViewingUnit->SetRenderViewingColor( true );
		}
	}
	else if (!setRender) {
		m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_COUNTRIES_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderNoColor();
		}
	}
}

void Map::SetRenderRegionMapMode( bool setRender /*= true */ )
{
	if (setRender && !(m_showingSettings.m_mapShowConfig & SHOW_CONFIG_REGIONS_MAP)) {
		m_showingSettings.m_mapShowConfig = (m_showingSettings.m_mapShowConfig & ~SHOW_CONFIG_COLORFUL_MAP) | SHOW_CONFIG_REGIONS_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderRegionColor();
		}
		if (m_curViewingUnit) {
			m_curViewingUnit->SetRenderViewingColor( true );
		}
	}
	else if (!setRender) {
		m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_REGIONS_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderNoColor();
		}
	}
}

void Map::SetRenderRelationMapMode( bool setRender /*= true */ )
{
	if (setRender && !(m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELATION_MAP)) {
		m_showingSettings.m_mapShowConfig = (m_showingSettings.m_mapShowConfig & ~SHOW_CONFIG_COLORFUL_MAP) | SHOW_CONFIG_RELATION_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderRelationColor();
		}
		if (m_curViewingUnit) {
			m_curViewingUnit->SetRenderViewingColor( true );
		}
	}
	else if (!setRender) {
		m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_RELATION_MAP;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderNoColor();
		}
	}
}

void Map::SetRenderProvEditMapMode( bool setRender /*= true */ )
{
	if (setRender && !(m_showingSettings.m_mapShowConfig & SHOW_CONFIG_PROVINCE_EDIT)) {
		m_showingSettings.m_mapShowConfig = (m_showingSettings.m_mapShowConfig & ~SHOW_CONFIG_COLORFUL_MAP) | SHOW_CONFIG_PROVINCE_EDIT;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderProvEditColor();
		}
		if (m_curViewingUnit) {
			m_curViewingUnit->SetRenderViewingColor( true );
		}
		GenerateProvinceEditUI();
	}
	else if (!setRender) {
		m_showingSettings.m_mapShowConfig ^= SHOW_CONFIG_PROVINCE_EDIT;
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderNoColor();
		}
	}
}

void Map::Reset2DCameraMode()
{
	if (g_theGame->m_viewMode == MapViewMode::ViewMode3D) {
		m_viewSwitchStartPosition = g_theGame->m_worldCamera.m_position;
		m_viewSwitchTargetPosition = g_theGame->m_worldCamera.m_position - Vec3( 0.f, -g_theGame->m_worldCamera.m_position.z * 0.4f, g_theGame->m_worldCamera.m_position.z * 0.2f );
		m_switchStartYaw = g_theGame->m_worldCamera.m_orientation.m_yawDegrees;
		m_switchStartPitch = g_theGame->m_worldCamera.m_orientation.m_pitchDegrees;
		m_2D3DSwitchTimer->Start();
	}
	else if (g_theGame->m_viewMode == MapViewMode::ViewModeSphere) {

	}
}

void Map::Reset3DCameraMode()
{
	if (g_theGame->m_viewMode == MapViewMode::ViewMode2D) {
		m_viewSwitchStartPosition = g_theGame->m_worldCamera.m_position;
		m_viewSwitchTargetPosition = g_theGame->m_worldCamera.m_position + Vec3( 0.f, -g_theGame->m_worldCameraScale * 0.4f, g_theGame->m_worldCameraScale * 0.2f );
		m_2D3DSwitchTimer->Start();
	}
	else if (g_theGame->m_viewMode == MapViewMode::ViewModeSphere) {
		m_viewSwitchStartPosition = g_theGame->m_worldCamera.m_position;
		m_viewSwitchTargetPosition = g_theGame->m_worldCamera.m_position + Vec3( 0.f, -g_theGame->m_worldCameraScale * 0.4f, g_theGame->m_worldCameraScale * 0.2f );
		m_2D3DSwitchTimer->Start();
	}
}

void Map::ResetSphereCameraMode()
{
	if (g_theGame->m_viewMode == MapViewMode::ViewMode2D) {

	}
	else if (g_theGame->m_viewMode == MapViewMode::ViewMode3D) {
		m_viewSwitchStartPosition = g_theGame->m_worldCamera.m_position;
		m_viewSwitchTargetPosition = g_theGame->m_worldCamera.m_position - Vec3( 0.f, -g_theGame->m_worldCamera.m_position.z * 0.4f, g_theGame->m_worldCamera.m_position.z * 0.2f );
		m_switchStartYaw = g_theGame->m_worldCamera.m_orientation.m_yawDegrees;
		m_switchStartPitch = g_theGame->m_worldCamera.m_orientation.m_pitchDegrees;
		m_2D3DSwitchTimer->Start();
	}
}

MapPolygonUnit* Map::GetUnitByPos( Vec2 const& pos ) const
{
	if (!m_bounds.IsPointInside( pos )) {
		return nullptr;
	}
	for (auto unit : m_mapPolygonUnits) {
		if (!unit->m_isFarAwayFakeUnit && unit->IsPointInsideConcavePolygon( pos )) {
			return unit;
		}
	}
	return nullptr;
}

MapPolygonUnit* Map::GetUnitByPosFast( Vec2 const& pos ) const
{
	if (!m_bounds.IsPointInside( pos )) {
		return nullptr;
	}
	for (auto unit : m_mapPolygonUnits) {
		if (!unit->m_isFarAwayFakeUnit && IsPointInsideDisc2D( pos, unit->m_centerPosition, unit->m_roughRadius )) {
			if (unit->IsPointInsideConvexPolygon( pos )) {
				return unit;
			}
		}
	}
	return nullptr;
}

MapPolygonUnit* Map::GetCurHoveringAtUnit3D( Vec3 const& mouseWorldPos ) const
{
	Vec3 forwardVec = (mouseWorldPos - g_theGame->m_worldCamera.m_position).GetNormalized();
	Vec3 startPos = mouseWorldPos;
	for (auto unit : m_mapPolygonUnits) {
		if (!unit->m_isFarAwayFakeUnit && (int)unit->m_edges.size() >= 3) {
			for (int i = 0; i < (int)unit->m_edges.size(); ++i) {
				Vec3 pos1 = Vec3( unit->m_edges[i]->m_startPos, unit->m_edges[i]->m_startHeight );
				Vec3 pos2 = Vec3( unit->m_edges[(i + 1) % (int)unit->m_edges.size()]->m_startPos, unit->m_edges[(i + 1) % (int)unit->m_edges.size()]->m_startHeight);
				Vec3 pos3 = Vec3( unit->m_geoCenterPos, unit->m_height * HEIGHT_FACTOR );
				Plane3 plane;
				plane.m_normal = CrossProduct3D( pos1 - pos3, pos2 - pos3 );
				plane.m_distanceFromOrigin = DotProduct3D( pos1, plane.m_normal );
				RayCastResult3D res;
				RayCastVsPlane3D( res, startPos, forwardVec, 100.f, plane );
				if (res.m_didImpact && plane.m_normal == res.m_impactNormal) {
					if (IsPointInsideTriangle2D( res.m_impactPos, pos1, pos2, pos3 )) {
						return unit;
					}
				}
			}
		}
	}
	return nullptr;
}

City* Map::GetCityByPos( Vec2 const& pos ) const
{
	if (!m_bounds.IsPointInside( pos )) {
		return nullptr;
	}
	for (auto city : m_cities) {
		if (city->m_iconBounds.IsPointInside( pos )) {
			return city;
		}
	}
	return nullptr;

}

City* Map::GetCurHoveringCity3D( Vec3 const& mouseWorldPos ) const
{
	Vec3 forwardVec = (mouseWorldPos - g_theGame->m_worldCamera.m_position).GetNormalized();
	if (forwardVec.z >= 0.f) {
		return nullptr;
	}
	Vec3 startPos = mouseWorldPos;
	for (auto city : m_cities) {
		double heightDiff = (double)startPos.z - (double)city->m_height - 0.1f;
		if (heightDiff >= 0.f) {
			double t = heightDiff / -forwardVec.z;
			Vec3 hitPos = startPos + (float)t * forwardVec;
			if (IsPointInsideAABB2D( hitPos, city->m_biggerIconBounds )) {
				return city;
			}
		}
	}
	return nullptr;
}

Army* Map::GetArmyByPos( Vec2 const& pos ) const
{
	if (!m_bounds.IsPointInside( pos )) {
		return nullptr;
	}
	for (auto country : m_countries) {
		for (auto army : country->m_armies) {
			if ( IsPointInsideDisc2D(pos, army->GetPosition(), army->GetOuterRadius())) {
				return army;
			}
		}
	}
	return nullptr;
}

Army* Map::GetCurHoveringArmy3D( Vec3 const& mouseWorldPos ) const
{
	Vec3 forwardVec = (mouseWorldPos - g_theGame->m_worldCamera.m_position).GetNormalized();
	if (forwardVec.z >= 0.f) {
		return nullptr;
	}
	Vec3 startPos = mouseWorldPos;
	for (auto country : m_countries) {
		for (auto army : country->m_armies) {
			Vec3 armyPos = army->GetPosition3D();
			float heightDiff = startPos.z - armyPos.z - 0.1f;
			if (heightDiff >= 0.f) {
				double t = heightDiff / -forwardVec.z;
				Vec3 hitPos = startPos + (float)t * forwardVec;
				if (IsPointInsideDisc2D( hitPos, armyPos, army->GetOuterRadius() * 1.5f )) {
					return army;
				}
			}
		}
	}
	return nullptr;
}

bool Map::IsTwoProvConnectedInSameCountry( MapPolygonUnit* prov1, MapPolygonUnit* prov2 ) const
{
	if (prov1->m_owner == nullptr || prov2->m_owner == nullptr || prov1->m_owner != prov2->m_owner) {
		return false;
	}
	std::deque<MapPolygonUnit*> queue;
	queue.push_back( prov1 );
	std::vector<MapPolygonUnit*> dirtyList;
	dirtyList.push_back( prov1 );
	while (!queue.empty()) {
		MapPolygonUnit* thisProv = queue.front();
		queue.pop_front();
		if (thisProv == prov2) {
			return true;
		}
		for (auto prov : thisProv->m_adjacentUnits) {
			if (prov->m_owner == prov1->m_owner && std::find( dirtyList.begin(), dirtyList.end(), prov ) == dirtyList.end()) {
				queue.push_back( prov );
				dirtyList.push_back( prov );
			}
		}
	}
	return false;
}

Vec2 Map::MP_GetRandomPointInAABB2D( AABB2 const& box )
{
	float x = m_mapRNG->RollRandomFloatInRange( box.m_mins.x, box.m_maxs.x );
	float y = m_mapRNG->RollRandomFloatInRange( box.m_mins.y, box.m_maxs.y );
	return Vec2( x, y );
}

Vec2 Map::MP_GetRandomPointInDisc2D( Vec2 const& center, float radius )
{
	Vec2 point;
	float length;
	do {
		point.x = m_mapRNG->RollRandomFloatZeroToOne() * 2.f - 1.f;
		point.y = m_mapRNG->RollRandomFloatZeroToOne() * 2.f - 1.f;
		length = point.GetLengthSquared();
	} while (length > 1.f);
	return point * radius + center;
}

void Map::SetUpHistorySimulation()
{
	RecordHistory();
	for (auto country : m_countries) {
		country->SetUpSimulation();
	}
}

void Map::SimulateNextMonth()
{
	PCGWorld_Log( "Next Month..." );
	double veryStartTime = GetCurrentTimeSeconds();
	//if (m_simulationJumpToCurrent) {
	if (m_viewingYear == m_year && m_viewingMonth == m_month) {
		RearrangeHistoryCache();
	}
	if (m_viewingYear != m_year || m_viewingMonth != m_month) {
		//ViewSpecificMonth( m_year, m_month );
		//ViewNextMonth();
		m_viewingMonth = m_month;
		m_viewingYear = m_year;

		RearrangeHistoryCache();
		ReadHistoryCache( GetHistoryData( m_year, m_month ) );
		//UpdateColorfulMaps();
	}
	
	SimulateNextMonthWithoutRefresh();
	UpdateColorfulMaps();

	double endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Next Month Simulation finished, time: %.3fs", endTime - veryStartTime ) );
}

void Map::SimulateNextMonthWithoutRefresh()
{
	/*	double startTime = GetCurrentTimeSeconds();*/
		//if (m_simulationJumpToCurrent) {

	CountriesBeginTurn();

	PutCountryInstructions();
	// 	double endTime = GetCurrentTimeSeconds();
	// 	PCGWorld_Log( Stringf( "Country Decision finished, time: %.3fs", endTime - startTime ) );
	//	startTime = endTime;
	ExecuteCountryBehaviors();
	// 	endTime = GetCurrentTimeSeconds();
	// 	PCGWorld_Log( Stringf( "Country Instruction Execution finished, time: %.3fs", endTime - startTime ) );
	// 	startTime = endTime;
	ResolveBattles();

	ProcessHistoryCrisis();

	// 	endTime = GetCurrentTimeSeconds();
	// 	PCGWorld_Log( Stringf( "Resolving Battles finished, time: %.3fs", endTime - startTime ) );
	// 	startTime = endTime;
	MapEndTurn();
	// 	endTime = GetCurrentTimeSeconds();
	// 	PCGWorld_Log( Stringf( "Map End Month finished, time: %.3fs", endTime - startTime ) );
	// 	startTime = endTime;
	CountriesEndTurn();
	// 	endTime = GetCurrentTimeSeconds();
	// 	PCGWorld_Log( Stringf( "Countries End Month finished, time: %.3fs", endTime - startTime ) );
	// 	startTime = endTime;
	// 	endTime = GetCurrentTimeSeconds();
	// 	PCGWorld_Log( Stringf( "Reset Map Colors finished, time: %.3fs", endTime - startTime ) );
	// 	startTime = endTime;
	// 	endTime = GetCurrentTimeSeconds();
	// 	PCGWorld_Log( Stringf( "History Save Execution finished, time: %.3fs", endTime - startTime ) );
	++m_month;
	if (m_month == 13) {
		m_month = 1;
		++m_year;
	}
	m_viewingMonth = m_month;
	m_viewingYear = m_year;
	//}
	//else {

	//}
	RecordHistory();
	RearrangeHistoryCache();
}

void Map::SimulateMonths( int numOfMonth )
{
	//if (m_simulationJumpToCurrent) {
	if (m_viewingYear == m_year && m_viewingMonth == m_month) {
		RearrangeHistoryCache();
	}
	if (m_viewingYear != m_year || m_viewingMonth != m_month) {
		//ViewSpecificMonth( m_year, m_month );
		//ViewNextMonth();
		m_viewingMonth = m_month;
		m_viewingYear = m_year;
		RearrangeHistoryCache();
		ReadHistoryCache( GetHistoryData( m_year, m_month ) );
		//UpdateColorfulMaps();
	}
	for (int _ = 0; _ < numOfMonth; _++) {
		double veryStartTime = GetCurrentTimeSeconds();
		PCGWorld_Log( Stringf( "Year %d Month %d---Next Month...", m_year, m_month ) );
		SimulateNextMonthWithoutRefresh();
		double endTime = GetCurrentTimeSeconds();
		PCGWorld_Log( Stringf( "Next Month Simulation finished, time: %.3fs", endTime - veryStartTime ) );
	}
	UpdateColorfulMaps();
}

void Map::ViewNextMonth()
{
	PCGWorld_Log( "Viewing Next Month..." );
	if (!DoHistoryExist( m_viewingYear, m_viewingMonth )) {
		return;
	}
	if (m_viewingYear == m_year && m_viewingMonth == m_month) {
		return;
	}
	double startTime = GetCurrentTimeSeconds();

	++m_viewingMonth;
	if (m_viewingMonth == 13) {
		m_viewingMonth = 1;
		++m_viewingYear;
	}
	if ((m_viewingYear == m_year && m_viewingMonth > m_month) || m_viewingYear > m_year) {
		m_viewingMonth = m_month;
		m_viewingYear = m_year;
	}

	ReadHistoryCache( GetHistoryData( m_viewingYear, m_viewingMonth ) );
	UpdateColorfulMaps();
	RearrangeHistoryCache();

	double endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Load Next Month finished, time: %.3fs", endTime - startTime ) );
}

void Map::ViewNextYear()
{
	PCGWorld_Log( "Viewing Next Year..." );
	double startTime = GetCurrentTimeSeconds();
	m_viewingYear = m_viewingYear + 1;
	if (m_viewingYear > m_year || (m_viewingYear == m_year && m_viewingMonth > m_month)) {
		m_viewingYear = m_year;
		m_viewingMonth = m_month;
	}

	ReadHistoryCache( GetHistoryData( m_viewingYear, m_viewingMonth ) );
	UpdateColorfulMaps();
	RearrangeHistoryCache();

	double endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Load Next Year finished, time: %.3fs", endTime - startTime ) );
}

void Map::ViewLastMonth()
{
	PCGWorld_Log( "Viewing Previous Month..." );
	if (m_viewingYear == 0 && m_viewingMonth == 1) {
		return;
	}
	double startTime = GetCurrentTimeSeconds();
	if (m_viewingMonth == 1) {
		m_viewingMonth = 12;
	}
	else {
		m_viewingMonth = m_viewingMonth - 1;
	}
	if (m_viewingMonth == 12) {
		m_viewingYear = m_viewingYear - 1;
	}
	else {
		m_viewingYear = m_viewingYear;
	}
	if (m_viewingYear < 0) {
		m_viewingYear = 0;
		m_viewingMonth = 1;
	}

	ReadHistoryCache( GetHistoryData( m_viewingYear, m_viewingMonth ) );
	UpdateColorfulMaps();
	RearrangeHistoryCache();

	double endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Load Previous Month finished, time: %.3fs", endTime - startTime ) );
}

void Map::ViewLastYear()
{
	PCGWorld_Log( "Viewing Previous Year..." );
	double startTime = GetCurrentTimeSeconds();
	m_viewingYear = m_viewingYear - 1;
	if (m_viewingYear < 0) {
		m_viewingYear = 0;
		m_viewingMonth = 1;
	}

	ReadHistoryCache( GetHistoryData( m_viewingYear, m_viewingMonth ) );
	UpdateColorfulMaps();
	RearrangeHistoryCache();

	double endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Load Previous Year finished, time: %.3fs", endTime - startTime ) );
}

// void Map::ViewSpecificMonth( int year, int month )
// {
// 	m_viewingYear = year;
// 	m_viewingMonth = month;
// 	LoadHistory( GetHistoryData( m_viewingYear, m_viewingMonth ) );
// 	UpdateColorfulMaps();
// 
// }

void Map::CountriesBeginTurn()
{
// 	for (auto prov : m_mapPolygonUnits) {
// 		prov->RemoveUnqualifiedLegitimateCountry();
// 	}
	std::sort( m_crisis.begin(), m_crisis.end(), []( HistoryCrisis* a, HistoryCrisis* b ) {
		if (!a) {
			return false;
		}
		else if (!b) {
			return true;
		}
		else {
			return a->m_globalID < b->m_globalID;
		}
		} );
	for (auto country : m_countries) {
		country->BeginTurn();
	}
}

void Map::CountriesEndTurn()
{
	for (auto country : m_countries) {
		country->EndTurn();
	}

// 	for (auto prov : m_mapPolygonUnits) {
// 		prov->RemoveUnqualifiedLegitimateCountry();
// 	}
}

void Map::PutCountryInstructions()
{
	for (auto country : m_countries) {
		if (country->IsExist()) {
			country->ExecuteCountryBehavior();
		}
		else {
			country->RemoveAllRelations();
		}
	}
}

void Map::ExecuteCountryBehaviors()
{
	while (!m_instructionQueue.empty()) {
		CountryInstruction* instr = m_instructionQueue.front();
		ExecuteInstruction( instr );
		m_instructionQueue.pop_front();
	}
}

void Map::ResolveBattles()
{
	for (auto prov : m_mapPolygonUnits) {
		if (prov->IsWater()) {

		}
		else {
			if ((int)prov->m_armiesOnProv.size() > 1) {
				// decide which one arrives first
				std::sort( prov->m_armiesOnProv.begin(), prov->m_armiesOnProv.end(),
					[]( std::pair<Army*, float> const& a, std::pair<Army*, float> const& b )->bool {
						return a.second < b.second;
					} );
				// regroup armies, make sure armies from the same country will fight together
				std::deque<ArmyBattleWrapper> armyListRegroup;
				// armyListRegroup.reserve( prov->m_armiesOnProv.size() );
				for (auto& pair : prov->m_armiesOnProv) {
					bool findPrevArmy = false;
					for (auto& armyWrapper : armyListRegroup) {
						if (armyWrapper.m_country == pair.first->m_owner) {
							armyWrapper.m_army.push_back( pair.first );
							armyWrapper.m_totalSize += pair.first->m_size;
							findPrevArmy = true;
							break;
						}
					}
					if (!findPrevArmy) {
						ArmyBattleWrapper wrapper;
						wrapper.m_army.push_back( pair.first );
						wrapper.m_country = pair.first->m_owner;
						wrapper.m_distance = pair.second;
						wrapper.m_totalSize = pair.first->m_size;
						armyListRegroup.push_back( wrapper );
					}
				}
				// if there is only one country's armies in this province, group them together
				if ((int)armyListRegroup.size() == 0) {
					ERROR_RECOVERABLE( "Code cannot reach here!!!" );
				}
				else if ((int)armyListRegroup.size() == 1) {
					// integrate the other armies into the first one
					for (int i = (int)prov->m_armiesOnProv.size() - 1; i >= 1; i--) {
						prov->m_armiesOnProv[0].first->IntegrateArmy( prov->m_armiesOnProv[i].first );
					}
					for (int i = (int)prov->m_armiesOnProv.size() - 1; i >= 1; i--) {
						RemoveArmyFromMap( prov->m_armiesOnProv[i].first, this );
					}
				}
				else { // start battle
					// do damage to the province
					prov->m_warFlag = true;
					while ((int)armyListRegroup.size() > 1) {
						// first two fight with each other
						BattleResult result = ResolveBattleArmyFromMap( &armyListRegroup[0], &armyListRegroup[1], this );
						if (armyListRegroup[0].m_totalSize + armyListRegroup[1].m_totalSize >= 100000) {
							if (result == BattleResult::Attacker_Win_Defender_Eliminated || result == BattleResult::Attacker_Win_Defender_Retreat) {
								AddHistoryLog( Stringf( "Year%d %s: a great battle occurred in %s, and attacker %s's army won %s's army\n", m_year, g_monthShortNameMap[m_month - 1].c_str(),
									prov->m_name.c_str(), armyListRegroup[1].m_army[0]->m_owner->m_name.c_str(), armyListRegroup[0].m_army[0]->m_owner->m_name.c_str() ) );
							}
							else {
								AddHistoryLog( Stringf( "Year%d %s: a great battle occurred in %s, and defender %s's army won %s's army\n", m_year, g_monthShortNameMap[m_month - 1].c_str(),
									prov->m_name.c_str(), armyListRegroup[0].m_army[0]->m_owner->m_name.c_str(), armyListRegroup[1].m_army[0]->m_owner->m_name.c_str() ) );
							}
						}
						if (result == BattleResult::Attacker_Win_Defender_Eliminated) {
							// remove all defenders
							for (auto army : armyListRegroup[0].m_army) {
								RemoveArmyFromMap( army, this );
							}
							armyListRegroup.pop_front();
						}
						else if (result == BattleResult::Attacker_Win_Defender_Retreat) {
							// integrate all defenders
							for (int i = 1; i < (int)armyListRegroup[0].m_army.size(); i++) {
								armyListRegroup[0].m_army[0]->IntegrateArmy( armyListRegroup[0].m_army[i] );
							}
							for (int i = 1; i < (int)armyListRegroup[0].m_army.size(); i++) {
								RemoveArmyFromMap( armyListRegroup[0].m_army[i], this );
							}
							// retreat to the best province
							Army* retreatArmy = armyListRegroup[0].m_army[0];
							// reset the size after battle
							retreatArmy->m_size = armyListRegroup[0].m_totalSize;
							RetreatFromProvince( retreatArmy, this );
							armyListRegroup.pop_front();
						}
						else if (result == BattleResult::Defender_Win_Attacker_Eliminated) {
							// remove all attackers
							for (auto army : armyListRegroup[1].m_army) {
								RemoveArmyFromMap( army, this );
							}
							armyListRegroup.erase( armyListRegroup.begin() + 1 );
						}
						else if (result == BattleResult::Defender_Win_Attacker_Retreat) {
							// integrate all attackers
							for (int i = 1; i < (int)armyListRegroup[1].m_army.size(); i++) {
								armyListRegroup[1].m_army[0]->IntegrateArmy( armyListRegroup[1].m_army[i] );
							}
							for (int i = 1; i < (int)armyListRegroup[1].m_army.size(); i++) {
								RemoveArmyFromMap( armyListRegroup[1].m_army[i], this );
							}
							// retreat to the best province
							Army* retreatArmy = armyListRegroup[1].m_army[0];
							// reset the size after battle
							retreatArmy->m_size = armyListRegroup[1].m_totalSize;
							RetreatFromProvince( retreatArmy, this );
							armyListRegroup.erase( armyListRegroup.begin() + 1 );
						}
					}
					// all battle finished, integrate the final winner
					for (int i = 1; i < (int)armyListRegroup[0].m_army.size(); i++) {
						armyListRegroup[0].m_army[0]->IntegrateArmy( armyListRegroup[0].m_army[i] );
					}
					for (int i = 1; i < (int)armyListRegroup[0].m_army.size(); i++) {
						RemoveArmyFromMap( armyListRegroup[0].m_army[i], this );
					}
					GUARANTEE_OR_DIE( (int)prov->m_armiesOnProv.size() == 1, "army amount in province should be 1 after battle!" );
				}
			}
			// siege
			if ((int)prov->m_armiesOnProv.size() == 1) {
				if (prov->m_owner && prov->m_armiesOnProv[0].first->m_owner != prov->m_owner && prov->m_armiesOnProv[0].first->m_owner->IsInWarWith( prov->m_owner )) {
					// do damage to the province
					prov->m_warFlag = true;
					Army* siegingArmy = prov->m_armiesOnProv[0].first;
					SiegeResult res = ResolveSiegeFromMap( prov->m_armiesOnProv[0].first, prov, this );
					if (res == SiegeResult::Attacker_Lose_Morale) {
						siegingArmy->m_size = RoundDownToInt( (float)siegingArmy->m_size * 0.95f );
					}
					else if (res == SiegeResult::Defender_Surrender) {
						AddHistoryLog( Stringf( "Year%d %s: %s's army successfully captured province %s from %s\n", m_year, g_monthShortNameMap[m_month - 1].c_str(),
							siegingArmy->m_owner->m_name.c_str(), prov->m_name.c_str(), prov->m_owner->m_name.c_str() ) );
						prov->m_owner->LoseProvince( prov, true );
						for (auto city: prov->m_cities) {
							city->m_warFlag = true;
						}
						for (auto town : prov->m_towns) {
							town->m_warFlag = true;
						}
						// ToDo: may cause issues
						// lose(kill) population of different culture\religion
// 						if (!prov->IsLegitimateToCountry( siegingArmy->m_owner )) {
// 							int populationToLose = 0;
// 							for (auto city : prov->m_cities) {
// 								int otherCulturePopulation = (int)round( (1.f - city->GetCultureInfluence( siegingArmy->m_owner->m_countryCulture )) * city->m_totalPopulation );
// 								int losePop = (int)round( otherCulturePopulation * 0.01f );
// 								populationToLose += losePop;
// 								city->m_totalPopulation -= losePop;
// 								city->SqueezeCultureInfluence( siegingArmy->m_owner->m_countryCulture, 0.01f, city->GetCultureInfluence( siegingArmy->m_owner->m_countryCulture ) );
// 								city->SqueezeReligionInfluence( siegingArmy->m_owner->m_countryReligion, 0.03f, city->GetReligionInfluence( siegingArmy->m_owner->m_countryReligion ) );
// 							}
// 							for (auto town : prov->m_towns) {
// 								int otherCulturePopulation = (int)round( (1.f - town->GetCultureInfluence( siegingArmy->m_owner->m_countryCulture )) * town->m_totalPopulation );
// 								int losePop = (int)round( otherCulturePopulation * 0.005f );
// 								populationToLose += losePop;
// 								town->m_totalPopulation -= losePop;
// 								town->SqueezeCultureInfluence( siegingArmy->m_owner->m_countryCulture, 0.005f, town->GetCultureInfluence( siegingArmy->m_owner->m_countryCulture ) );
// 								town->SqueezeReligionInfluence( siegingArmy->m_owner->m_countryReligion, 0.005f, town->GetReligionInfluence( siegingArmy->m_owner->m_countryReligion ) );
// 							}
// 							int otherCulturePopulation = (int)round( (1.f - prov->HD_GetCultureInfluence( siegingArmy->m_owner->m_countryCulture )) * prov->m_totalPopulation );
// 							int losePop = (int)round( otherCulturePopulation * 0.005f );
// 							populationToLose += losePop;
// 							prov->m_totalPopulation -= losePop;
// 							prov->SqueezeCultureInfluence( siegingArmy->m_owner->m_countryCulture, 0.005f, prov->HD_GetCultureInfluence( siegingArmy->m_owner->m_countryCulture ) );
// 							prov->SqueezeReligionInfluence( siegingArmy->m_owner->m_countryReligion, 0.005f, prov->HD_GetCultureInfluence( siegingArmy->m_owner->m_countryCulture ) );
// 							siegingArmy->m_owner->m_funds += populationToLose * 5;
// 						}
						siegingArmy->m_owner->GainProvince( prov );
					}
				}
				else if (!prov->m_owner || prov->m_armiesOnProv[0].first->m_owner->IsInWarWith( prov->m_owner )) {
					ERROR_RECOVERABLE( "Cannot reach here or there was a bug!" );
				}
			}
		}
	}
}

void Map::ExecuteInstruction( CountryInstruction const* instr )
{
	if (instr->m_type == InstructionType::BuildArmy) {
		InstructionBuildArmy* instrc = (InstructionBuildArmy*)instr;
		Army* newArmy = new Army( instrc->m_prov->m_owner, instrc->m_prov, instrc->m_numOfSoldiersRecruited );
		newArmy->m_globalID = m_globalArmyID;
		++m_globalArmyID;
		if (m_globalArmyID == INVALID_ARMY_ID) {
			++m_globalArmyID;
		}
		newArmy->m_isActive = true;
		instrc->m_prov->m_armiesOnProv.push_back( std::pair<Army*, float>( newArmy, 0.f ) );
		instrc->m_prov->m_owner->m_armies.push_back( newArmy );
	}
	else if (instr->m_type == InstructionType::Assimilate) {
		InstructionAssimilate* instrc = (InstructionAssimilate*)instr;
		{
			float ratio = std::min( 100.f / (float)instrc->m_province->m_totalPopulation, 1.f );
			//if(instrc->m_province) origin is hard to be converted
			float addSum = 0.f;
			std::pair<Culture*, float>* cultureToAdd = nullptr;
			for (auto& pair : instrc->m_province->m_cultures) {
				if (pair.first == instrc->m_culture) {
					cultureToAdd = &pair;
				}
				else {
					float cultureFactor = 1.f;
					if (pair.first->HasTrait( CultureTrait::Unruly )) {
						cultureFactor -= 0.25f;
					}
					if (instrc->m_province->m_owner->m_countryCulture->HasTrait(CultureTrait::Sedentary)) {
						cultureFactor -= 0.2f;
					}
					float sub = pair.second * ratio * cultureFactor;
					pair.second -= sub;
					addSum += sub;
				}
			}
			if (cultureToAdd) {
				cultureToAdd->second += addSum;
			}
			else {
				instrc->m_province->m_cultures.push_back( std::pair<Culture*, float>( instrc->m_culture, addSum ) );
			}
			instrc->m_province->RecalculateMajorCulture();
		}
		for (auto city : instrc->m_province->m_cities) {
			float ratio = std::min( 150.f / (float)city->m_totalPopulation, 1.f );
			float addSum = 0.f;
			std::pair<Culture*, float>* cultureToAdd = nullptr;
			for (auto& pair : city->m_cultures) {
				if (pair.first == instrc->m_culture) {
					cultureToAdd = &pair;
				}
				else {
					float sub = pair.second * ratio;
					pair.second -= sub;
					addSum += sub;
				}
			}
			if (cultureToAdd) {
				cultureToAdd->second += addSum;
			}
			else {
				city->m_cultures.push_back( std::pair<Culture*, float>( instrc->m_culture, addSum ) );
			}
			city->RecalculateMajorCulture();
		}
		for (auto town : instrc->m_province->m_towns) {
			float ratio = std::min( 120.f / (float)town->m_totalPopulation, 1.f );
			float addSum = 0.f;
			std::pair<Culture*, float>* cultureToAdd = nullptr;
			for (auto& pair : town->m_cultures) {
				if (pair.first == instrc->m_culture) {
					cultureToAdd = &pair;
				}
				else {
					float sub = pair.second * ratio;
					pair.second -= sub;
					addSum += sub;
				}
			}
			if (cultureToAdd) {
				cultureToAdd->second += addSum;
			}
			else {
				town->m_cultures.push_back( std::pair<Culture*, float>( instrc->m_culture, addSum ) );
			}
			town->RecalculateMajorCulture();
		}
	}
	else if (instr->m_type == InstructionType::BuildRelationship) {
		InstructionRelationship* instrc = (InstructionRelationship*)instr;
		if (!instrc->countryFrom->IsExist() || !instrc->countryTo->IsExist()) {
			return;
		}
		if (instrc->countryFrom->IsVassal() && instrc->countryTo->IsVassal() && instrc->countryTo->m_relationSuzerain == instrc->countryFrom->m_relationSuzerain) {
			return;
		}
		CountryRelationType curRelationship = instrc->countryFrom->GetRelationTo( instrc->countryTo );
		if (curRelationship == CountryRelationType::War && instrc->m_relationWant != CountryRelationType::War) {
			m_endWarTimes.push_back( CountryEndWarTreaty{ instrc->countryFrom, instrc->countryTo, m_year, m_month } );
		}
		if (instrc->m_relationWant == CountryRelationType::Tributary || instrc->m_relationWant == CountryRelationType::Vassal) {
			instrc->countryFrom->RemoveAllRelations();
		}
		else {
			instrc->countryFrom->RemoveAllRelationWithCountry( instrc->countryTo );
		}
		instrc->countryFrom->AddRelationWithCountry( instrc->countryTo, instrc->m_relationWant );
		if (instrc->m_relationWant == CountryRelationType::War) {
			AddHistoryLog( Stringf( "Year%d %s: %s declared war on %s\n", m_year, g_monthShortNameMap[m_month - 1].c_str(), instrc->countryFrom->m_name.c_str(), instrc->countryTo->m_name.c_str() ) );
		}
		else if (instrc->m_relationWant == CountryRelationType::Alliance) {
			AddHistoryLog( Stringf( "Year%d %s: %s confirmed alliance with %s\n", m_year, g_monthShortNameMap[m_month - 1].c_str(), instrc->countryFrom->m_name.c_str(), instrc->countryTo->m_name.c_str() ) );
		}
		else if (curRelationship == CountryRelationType::War && instrc->m_relationWant != CountryRelationType::War) {
			if (instrc->m_relationWant == CountryRelationType::Vassal) {
				AddHistoryLog( Stringf( "Year%d %s: %s ended war with %s, %s became %s's vassal\n", m_year, g_monthShortNameMap[m_month - 1].c_str(),
					instrc->countryFrom->m_name.c_str(), instrc->countryTo->m_name.c_str(), instrc->countryFrom->m_name.c_str(), instrc->countryTo->m_name.c_str() ) );
			}
			else if (instrc->m_relationWant == CountryRelationType::Tributary) {
				AddHistoryLog( Stringf( "Year%d %s: %s ended war with %s, %s became %s's tributary\n", m_year, g_monthShortNameMap[m_month - 1].c_str(), 
					instrc->countryFrom->m_name.c_str(), instrc->countryTo->m_name.c_str(), instrc->countryFrom->m_name.c_str(), instrc->countryTo->m_name.c_str() ) );
			}
			else if (instrc->m_relationWant == CountryRelationType::None) {
				AddHistoryLog( Stringf( "Year%d %s: %s ended war with %s, %s is annexed by %s\n", m_year, g_monthShortNameMap[m_month - 1].c_str(),
					instrc->countryFrom->m_name.c_str(), instrc->countryTo->m_name.c_str(), instrc->countryFrom->m_name.c_str(), instrc->countryTo->m_name.c_str() ) );
			}
			else {
				AddHistoryLog( Stringf( "Year%d %s: %s ended war with %s\n", m_year, g_monthShortNameMap[m_month - 1].c_str(), instrc->countryFrom->m_name.c_str(), instrc->countryTo->m_name.c_str() ) );
			}
		}
	}
	else if (instr->m_type == InstructionType::Convert) {
		InstructionConvert* instrc = (InstructionConvert*)instr;
		{
			float ratio = std::min( 150.f / (float)instrc->m_province->m_totalPopulation, 1.f );
			float addSum = 0.f;
			std::pair<Religion*, float>* religionToAdd = nullptr;
			for (auto& pair : instrc->m_province->m_religions) {
				if (pair.first == instrc->m_religion) {
					religionToAdd = &pair;
				}
				else {
					float sub = pair.second * ratio;
					pair.second -= sub;
					addSum += sub;
				}
			}
			if (religionToAdd) {
				religionToAdd->second += addSum;
			}
			else {
				instrc->m_province->m_religions.push_back( std::pair<Religion*, float>( instrc->m_religion, addSum ) );
			}
			instrc->m_province->RecalculateMajorReligion();
		}
		for (auto city : instrc->m_province->m_cities) {
			float ratio = std::min( 200.f / (float)city->m_totalPopulation, 1.f );
			float addSum = 0.f;
			std::pair<Religion*, float>* religionToAdd = nullptr;
			for (auto& pair : city->m_religions) {
				if (pair.first == instrc->m_religion) {
					religionToAdd = &pair;
				}
				else {
					float sub = pair.second * ratio;
					pair.second -= sub;
					addSum += sub;
				}
			}
			if (religionToAdd) {
				religionToAdd->second += addSum;
			}
			else {
				city->m_religions.push_back( std::pair<Religion*, float>( instrc->m_religion, addSum ) );
			}
			city->RecalculateMajorReligion();
		}
		for (auto town : instrc->m_province->m_towns) {
			float ratio = std::min( 180.f / (float)town->m_totalPopulation, 1.f );
			float addSum = 0.f;
			std::pair<Religion*, float>* religionToAdd = nullptr;
			for (auto& pair : town->m_religions) {
				if (pair.first == instrc->m_religion) {
					religionToAdd = &pair;
				}
				else {
					float sub = pair.second * ratio;
					pair.second -= sub;
					addSum += sub;
				}
			}
			if (religionToAdd) {
				religionToAdd->second += addSum;
			}
			else {
				town->m_religions.push_back( std::pair<Religion*, float>( instrc->m_religion, addSum ) );
			}
			town->RecalculateMajorReligion();
		}
	}
	else if (instr->m_type == InstructionType::Develop) {
		InstructionDevelop* instrc = (InstructionDevelop*)instr;
		instrc->m_prov->m_developmentFlag = true;
	}
	else if (instr->m_type == InstructionType::MoveArmy) {
		InstructionMoveArmy* instrc = (InstructionMoveArmy*)instr;
		if (instrc->m_fromProv->m_armiesOnProv.empty() || (!instrc->m_fromProv->IsWater() && instrc->m_fromProv->m_armiesOnProv[0].first != instrc->m_army)) {
			ERROR_RECOVERABLE( "Army is not on the from province!" );
			return;
		}
		instrc->m_fromProv->m_armiesOnProv.pop_front();
		instrc->m_army->m_provIn = instrc->m_toProv;
		instrc->m_toProv->m_armiesOnProv.push_back(
			std::pair<Army*, float>( instrc->m_army, 
				GetDistanceSquared2D( instrc->m_fromProv->m_geoCenterPos, instrc->m_toProv->m_geoCenterPos ) ) );
	}
	else if (instr->m_type == InstructionType::RecruitArmy) {
		InstructionRecruitArmy* instrc = (InstructionRecruitArmy*)instr;
		instrc->m_army->m_size += instrc->m_numOfSoldiersRecruited;
	}
	else if (instr->m_type == InstructionType::Legalize) {
		InstructionLegalize* instrc = (InstructionLegalize*)instr;
		instrc->m_province->AddLegitimateCountry( instrc->m_country );
		//instrc->m_province->RemoveUnqualifiedLegitimateCountry();
		AddHistoryLog( Stringf( "Year%d %s: people from %s(%d) accepted the rule of %s\n", m_year, g_monthShortNameMap[m_month - 1].c_str(),
			instrc->m_province->m_name.c_str(), instrc->m_province->m_id, instrc->m_country->m_name.c_str() ) );
	}
}

void Map::MapEndTurn()
{
	// population growth
	for (auto prov : m_mapPolygonUnits) {
		if (!prov->IsWater() && !prov->m_isFarAwayFakeUnit) {
			prov->GrowPopulationOneMonth();
		}
	}
	for (auto city : m_cities) {
		city->GrowPopulationOneMonth();
	}
	for (auto town : m_towns) {
		town->GrowPopulationOneMonth();
	}

	// city restoration
	for (auto city : m_cities) {
		city->Restore();
	}

	// armies without military passage go back to their country
	std::vector<Army*> armyListToDestroy;
	for (auto country : m_countries) {
		if (country->IsExist()) {
			for (auto army : country->m_armies) {
				if (army->m_provIn->m_owner && !country->DoesCountryHaveMilitaryAccess( army->m_provIn->m_owner )) {
					// for all country provinces, sort distance
					std::vector<Province*> countryProv = country->m_provinces;
					std::sort( countryProv.begin(), countryProv.end(),
						[&]( Province* a, Province* b ) {
							return GetDistanceSquared2D( army->m_provIn->m_geoCenterPos, a->m_geoCenterPos ) <
								GetDistanceSquared2D( army->m_provIn->m_geoCenterPos, b->m_geoCenterPos ); } );
					// choose the first valid province
					bool findProvToGo = false;
					for (auto prov : countryProv) {
						if ((int)prov->m_armiesOnProv.size() == 0) {
							findProvToGo = true;
							for (int i = 0; i < (int)army->m_provIn->m_armiesOnProv.size(); ++i) {
								if (army->m_provIn->m_armiesOnProv[i].first == army) {
									army->m_provIn->m_armiesOnProv.erase( army->m_provIn->m_armiesOnProv.begin() + i );
									break;
								}
							}
							army->m_provIn = prov;
							prov->m_armiesOnProv.push_back( std::pair<Army*, float>( army, 0.f ) );
						}
					}
					// if no province, destroy the army
					if (!findProvToGo) {
						armyListToDestroy.push_back( army );
					}
				}
			}
		}
	}

	for (auto country : m_countries) {
		if (!country->IsExist()) {
			for (auto army : country->m_armies) {
				armyListToDestroy.push_back( army );
			}
		}
	}

	for (auto army : armyListToDestroy) {
		RemoveArmyFromMap( army, this );
	}

	// set army distance to prov to 0
	for (auto prov : m_mapPolygonUnits) {
		if ((int)prov->m_armiesOnProv.size() == 0) {
		}
		else if ((int)prov->m_armiesOnProv.size() == 1) {
			prov->m_armiesOnProv[0].second = 0.f;
		}
		else {
			if (!prov->IsWater()) {
				ERROR_RECOVERABLE( "Cannot have 2 armies in the same province in the end of the month!" );
			}
		}
	}

}

void Map::RecordHistory()
{
	m_historyData.reserve( m_historyData.size() + 2 );
	if (std::abs( GetMonthDiffBetweenTwoTimes( m_viewingYear, m_viewingMonth, m_year, m_month ) ) <= MAX_SAVE_MONTH) {
		HistoryData* data = new HistoryData( this );
		//std::vector<uint8_t> binData;
		//data->DumpToBinaryFormat( binData );
		m_historyData.push_back( data );
#ifdef DEBUG_COMPARE_HISTORY
		// check history correctness
  		if (m_year != 0 || m_month != 1) {
  			HistoryData prevData;
  			int curYear = m_year;
  			int curMonth = m_month;
//   			++curMonth;
//   			if (curMonth == 13) {
//   				curMonth = 1;
//   				++curYear;
//   			}
  			LoadHistoryCacheFromDisk( prevData, curYear, curMonth );
			DebugCompareHistory( prevData, *data );
		}
#endif
	}
	else {
		HistoryData data( this );
		std::vector<uint8_t> binData;
		data.DumpToBinaryFormat( binData );
		std::filesystem::create_directory( Stringf( "Saves/%d", m_generationSettings.m_seed ).c_str() );
		std::filesystem::create_directory( Stringf( "Saves/%d/HistoryCache", m_generationSettings.m_seed ).c_str() );
		BufferWriteToFile( binData, Stringf( "Saves/%d/HistoryCache/Year%d_Month%d_History.his", m_generationSettings.m_seed, m_viewingYear, m_viewingMonth ) );
		m_historyData.push_back( nullptr );
	}
}

void Map::LoadHistoryCacheFromDisk( HistoryData& data, int year, int month ) const
{
	std::vector<uint8_t> binData;
	size_t size = 10 * (size_t)std::filesystem::file_size( Stringf( "Saves/%d/HistoryCache/Year%d_Month%d_History.his", m_generationSettings.m_seed, year, month ) );
	binData.resize( size );

	struct zip_t* zip = zip_open( Stringf( "Saves/%d/HistoryCache/Year%d_Month%d_History.his", m_generationSettings.m_seed, year, month ).c_str(),
		0, 'r' );
	{
		zip_entry_open( zip, "History.his" );
		{
			zip_entry_noallocread( zip, binData.data(), size );
		}
		zip_entry_close( zip );
	}
	zip_close( zip );

	//FileReadToBuffer( binData, Stringf( "Saves/%d/HistoryCache/Year%d_Month%d_History.his", m_generationSettings.m_seed, year, month ) );
	data.LoadFromBinaryFormat( binData );
}

void Map::AddHistoryLog( std::string const& log )
{
	m_historyLog.push_back( log );
}

void Map::UpdateColorfulMaps()
{
	// recalculate some vertex buffer
	// create country edges vertex buffers
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> vertsForEachCountry;
	vertsForEachCountry.reserve( 1000 );
	for (auto country : m_countries) {
		for (auto prov : country->m_provinces) {
			for (auto edge : prov->m_edges) {
				// if this is the boarder of the country, then add verts to vector
				if (edge->m_opposite && edge->m_opposite->m_owner && edge->m_opposite->m_owner->m_owner != country) {
					if (m_bounds.IsPointInside( edge->m_startPos ) || m_bounds.IsPointInside( edge->m_endPos )) {
						if ((int)edge->m_noisyEdges.size() == 0) {
							Vec2 startPos = edge->m_startPos;
							Vec2 endPos = edge->m_endPos;
							PM_ClampLinesIntoBounds( startPos, endPos );
							verts.emplace_back( startPos, Rgba8( 20, 20, 20 ) );
							verts.emplace_back( endPos, Rgba8( 20, 20, 20 ) );
							vertsForEachCountry.emplace_back( Vec3( startPos.x, startPos.y, 0.01f ), Rgba8::WHITE );
							vertsForEachCountry.emplace_back( Vec3( endPos.x, endPos.y, 0.01f ), Rgba8::WHITE );
						}
						else {
							for (int k = 0; k < (int)(edge->m_noisyEdges.size() - 1); k++) {
								verts.emplace_back( edge->m_noisyEdges[k], Rgba8( 20, 20, 20 ) );
								verts.emplace_back( edge->m_noisyEdges[k + 1], Rgba8( 20, 20, 20 ) );
								vertsForEachCountry.emplace_back( Vec3( edge->m_noisyEdges[k].x, edge->m_noisyEdges[k].y, 0.01f ), Rgba8::WHITE );
								vertsForEachCountry.emplace_back( Vec3( edge->m_noisyEdges[k + 1].x, edge->m_noisyEdges[k + 1].y, 0.01f ), Rgba8::WHITE );
							}
						}
					}
				}
			}

		}
		if (vertsForEachCountry.size() > 0) {
			//country->m_edgeShowingBuffer = g_theRenderer->CreateVertexBuffer( vertsForEachCountry.size() * sizeof( Vertex_PCU ) );
			if (!country->m_edgeShowingBuffer) {
				country->m_edgeShowingBuffer = g_theRenderer->CreateVertexBuffer( vertsForEachCountry.size() * sizeof( Vertex_PCU ) );
			}
			g_theRenderer->CopyCPUToGPU( vertsForEachCountry.data(), vertsForEachCountry.size() * sizeof( Vertex_PCU ), country->m_edgeShowingBuffer );
			country->m_edgeShowingBuffer->SetAsLinePrimitive( true );
			vertsForEachCountry.clear();
		}
	}

	//m_countriesEdgesVertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_countriesEdgesVertexBuffer );
	m_countriesEdgesVertexBuffer->SetAsLinePrimitive( true );

	verts.clear();

	// create 3D country edges vertex buffers
	vertsForEachCountry.clear();
	vertsForEachCountry.reserve( 1000 );
	for (auto country : m_countries) {
		for (auto prov : country->m_provinces) {
			for (auto edge : prov->m_edges) {
				// if this is the boarder of the country, then add verts to vector
				if (edge->m_opposite && edge->m_opposite->m_owner && edge->m_opposite->m_owner->m_owner != country) {
					if (m_bounds.IsPointInside( edge->m_startPos ) || m_bounds.IsPointInside( edge->m_endPos )) {
						constexpr float factor = HEIGHT_FACTOR;

						float prevHeight = m_generationSettings.m_minHeight;
						float oppoHeight = m_generationSettings.m_minHeight;
						float nextHeight = m_generationSettings.m_minHeight;
						if (edge->m_prev->m_opposite) {
							prevHeight = edge->m_prev->m_opposite->m_owner->m_height;
						}
						if (edge->m_opposite) {
							oppoHeight = edge->m_opposite->m_owner->m_height;
						}
						if (edge->m_next->m_opposite) {
							nextHeight = edge->m_next->m_opposite->m_owner->m_height;
						}
						float startHeight = (prov->m_height + oppoHeight + prevHeight) / 3.f;
						float endHeight = (prov->m_height + oppoHeight + nextHeight) / 3.f;
						if ((int)edge->m_noisyEdges.size() == 0) {
							Vec2 startPos = edge->m_startPos;
							Vec2 endPos = edge->m_endPos;
							PM_ClampLinesIntoBounds( startPos, endPos );
							verts.emplace_back( Vec3( startPos, startHeight * factor ), Rgba8( 20, 20, 20 ) );
							verts.emplace_back( Vec3( endPos, endHeight * factor ), Rgba8( 20, 20, 20 ) );
							vertsForEachCountry.emplace_back( Vec3( startPos, startHeight * factor ), Rgba8::WHITE );
							vertsForEachCountry.emplace_back( Vec3( endPos, endHeight * factor ), Rgba8::WHITE );
						}
						else {
							int noisyEdgeSize = (int)edge->m_noisyEdges.size();
							for (int k = 0; k < noisyEdgeSize - 1; k++) {
								float firstHeight = factor * Interpolate( startHeight, endHeight, (float)k / (float)(noisyEdgeSize - 1) );
								float secondHeight = factor * Interpolate( startHeight, endHeight, (float)(k + 1) / (float)(noisyEdgeSize - 1) );
								verts.emplace_back( Vec3( edge->m_noisyEdges[k], firstHeight ), Rgba8( 20, 20, 20 ) );
								verts.emplace_back( Vec3( edge->m_noisyEdges[k + 1], secondHeight ), Rgba8( 20, 20, 20 ) );
								vertsForEachCountry.emplace_back( Vec3( edge->m_noisyEdges[k], firstHeight ), Rgba8::WHITE );
								vertsForEachCountry.emplace_back( Vec3( edge->m_noisyEdges[k + 1], secondHeight ), Rgba8::WHITE );
							}
						}
					}
				}
			}

		}
		if (vertsForEachCountry.size() > 0) {
			if (!country->m_edgeShowingBuffer3D) {
				country->m_edgeShowingBuffer3D = g_theRenderer->CreateVertexBuffer( vertsForEachCountry.size() * sizeof( Vertex_PCU ) );
			}
			g_theRenderer->CopyCPUToGPU( vertsForEachCountry.data(), vertsForEachCountry.size() * sizeof( Vertex_PCU ), country->m_edgeShowingBuffer3D );
			country->m_edgeShowingBuffer3D->SetAsLinePrimitive( true );
			vertsForEachCountry.clear();
		}
	}

	//m_countriesEdgesVertexBuffer3D = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_countriesEdgesVertexBuffer3D );
	m_countriesEdgesVertexBuffer3D->SetAsLinePrimitive( true );
	verts.clear();

	for (auto city : m_cities) {
		if (city->HasAttribute(CityAttribute::Capital)) {
			AddVertsForAABB2D( verts, AABB2( city->m_iconBounds.m_mins
				- Vec2( CITY_ICON_SIDE_LENGTH * 0.5f, CITY_ICON_SIDE_LENGTH * 0.5f ), city->m_iconBounds.m_maxs
				+ Vec2( CITY_ICON_SIDE_LENGTH * 0.5f, CITY_ICON_SIDE_LENGTH * 0.5f ) ), Rgba8( 255, 51, 51 ) );
		}
		else {
			AddVertsForAABB2D( verts, city->m_iconBounds, Rgba8( 180, 0, 0 ) );
		}
	}
	//m_cityVertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_cityVertexBuffer );

	RefreshAllLabels();

	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_POPULATION_MAP) {
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderPopulationColor();
		}
	}
	else if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CULTURE_MAP) {
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderCultureColor();
		}
	}
	else if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELATION_MAP) {
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderRelationColor();
		}
	}
	else if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELIGION_MAP) {
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderReligionColor();
		}
	}
	else if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_PRODUCT_MAP) {
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderProductColor();
		}
	}
	else if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_COUNTRIES_MAP) {
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderCountryColor();
		}
	}
	else if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_PROVINCE_EDIT) {
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderProvEditColor();
		}
		GenerateProvinceEditUI();
	}

	if (m_curViewingUnit) {
		m_curViewingUnit->SetRenderViewingColor( true );
	}
}

void Map::ReadHistoryCache( HistoryData const& data )
{
	for (auto country : m_countries) {
		for (auto army : country->m_armies) {
			army->m_isActive = false;
		}
		country->m_provinces.clear();
		country->m_cities.clear();
		country->m_towns.clear();
		country->m_relationAllianceCountries.clear();
		country->m_relationFriendlyCountries.clear();
		country->m_relationHostileCountries.clear();
		country->m_relationWarCountries.clear();
		country->m_relationTributaries.clear();
		country->m_relationVassals.clear();
		country->m_relationSuzerain = nullptr;
		country->m_relationCelestialEmpire = nullptr;
	}
	for (int i = 0; i < (int)data.m_countryData.size(); ++i) {
		HistoryCountryData const& countryData = data.m_countryData[i];
		if (countryData.m_exist) {
			Country* country = m_countries[i];
			if (countryData.m_capitalCityID != -1) {
				country->m_capitalCity = m_cities[countryData.m_capitalCityID];
				country->m_capitalCity->AddAttribute( CityAttribute::Capital );
			}
			else {
				country->m_capitalCity = nullptr;
			}
			if (countryData.m_capitalProvID != -1) {
				country->m_capitalProv = m_mapPolygonUnits[countryData.m_capitalProvID];
			}
			else {
				country->m_capitalProv = nullptr;
			}

			country->m_countryCulture = m_cultures[countryData.m_countryCultureID];
			country->m_countryReligion = m_religions[countryData.m_countryReligionID];
			country->m_funds = countryData.m_funds;
			country->m_isCelestial = countryData.m_isCelestial;
			country->m_governmentType = (CountryGovernmentType)countryData.m_governmentType;

			for (int j = 0; j < countryData.m_allianceCountriesID.size(); ++j) {
				country->m_relationAllianceCountries.push_back( m_countries[countryData.m_allianceCountriesID[j]] );
			}
			for (int j = 0; j < countryData.m_friendlyCountriesID.size(); ++j) {
				country->m_relationFriendlyCountries.push_back( m_countries[countryData.m_friendlyCountriesID[j]] );
			}
			for (int j = 0; j < countryData.m_hostileCountriesID.size(); ++j) {
				country->m_relationHostileCountries.push_back( m_countries[countryData.m_hostileCountriesID[j]] );
			}
			country->m_warTime.clear();
			for (int j = 0; j < countryData.m_warCountriesID.size(); ++j) {
				country->m_relationWarCountries.push_back( m_countries[countryData.m_warCountriesID[j].first] );
				country->m_warTime.push_back( std::pair<Country*, int>( m_countries[countryData.m_warCountriesID[j].first], countryData.m_warCountriesID[j].second ) );
			}
			for (int j = 0; j < countryData.m_tributaryCountriesID.size(); ++j) {
				country->m_relationTributaries.push_back( m_countries[countryData.m_tributaryCountriesID[j]] );
			}
			for (int j = 0; j < countryData.m_vassalCountriesID.size(); ++j) {
				country->m_relationVassals.push_back( m_countries[countryData.m_vassalCountriesID[j]] );
			}
			if (countryData.m_suzerainCountryID != -1) {
				country->m_relationSuzerain = m_countries[countryData.m_suzerainCountryID];
			}
			if (countryData.m_celestialCountryID != -1) {
				country->m_relationCelestialEmpire = m_countries[countryData.m_celestialCountryID];
			}
		}
	}
	for (auto prov : m_mapPolygonUnits) {
		prov->m_armiesOnProv.clear();
	}
	for (int i = 0; i < m_crisis.size(); ++i) {
		if (m_crisis[i]) {
			m_crisis[i]->m_isActive = false;
		}
	}
	for (int i = 0; i < (int)data.m_crisisData.size(); ++i) {
		HistoryCrisisData const& crisisData = data.m_crisisData[i];
		HistoryCrisis* crisis = GetCrisisByGlobalID( crisisData.m_globalID );
		if (crisis) {
			crisis->m_progress = crisisData.m_progress;
			crisis->m_isActive = true;
		}
		else {
			crisis = new HistoryCrisis();
			AddNewCrisisToList( crisis );
			crisis->m_country = m_countries[crisisData.m_countryID];
			crisis->m_globalID = crisisData.m_globalID;
			crisis->m_type = (CrisisType)crisisData.m_type;
			if (crisisData.m_type == (int)CrisisType::ReligionConflict) {
				crisis->m_cultureOrReligion = m_religions[crisisData.m_cultureOrReligionID];
			}
			else if (crisisData.m_type == (int)CrisisType::CultureConflict) {
				crisis->m_cultureOrReligion = m_cultures[crisisData.m_cultureOrReligionID];
			}
			crisis->m_progress = crisisData.m_progress;
			crisis->m_isActive = true;
		}
	}
	for (int i = 0; i < m_crisis.size(); ++i) {
		if (m_crisis[i] && !m_crisis[i]->m_isActive) {
			delete m_crisis[i];
			m_crisis[i] = nullptr;
		}
	}
	for (int i = 0; i < (int)data.m_armyData.size(); ++i) {
		HistoryArmyData const& armyData = data.m_armyData[i];
		Army* army = GetArmyByGlobalID( data.m_armyData[i].m_globalID );
		if (army) {
			army->m_provIn = m_mapPolygonUnits[armyData.m_provInID];
			army->m_provIn->m_armiesOnProv.push_back( std::pair<Army*, float>( army, 0.f ) );
			army->m_combatValue = armyData.m_combatValue;
			army->m_size = armyData.m_size;
			army->m_isActive = true;
			army->m_goingTarget = armyData.m_targetProvID != -1 ? m_mapPolygonUnits[armyData.m_targetProvID] : nullptr;
		}
		else { // create a new army
			Army* newArmy = new Army( m_countries[armyData.m_ownerID], m_mapPolygonUnits[armyData.m_provInID], armyData.m_size );
			newArmy->m_globalID = armyData.m_globalID;
			newArmy->m_combatValue = armyData.m_combatValue;
			newArmy->m_provIn->m_armiesOnProv.push_back( std::pair<Army*, float>( newArmy, 0.f ) );
			newArmy->m_owner->m_armies.push_back( newArmy );
			newArmy->m_isActive = true;
		}
	}
	for (int i = 0; i < (int)data.m_provinceData.size(); ++i ) {
		HistoryProvinceData const& provData = data.m_provinceData[i];
		MapPolygonUnit* prov = m_mapPolygonUnits[i];
		if (provData.m_isWater) {
			if (!prov->IsWater()) {
				ERROR_RECOVERABLE( "This history data should be water!" );
			}
			continue;
		}
		prov->m_totalPopulation = provData.m_population;
		prov->m_legitimateCountries.clear();
		for (int j = 0; j < (int)provData.m_legalCountriesID.size(); j++) {
			//if (m_countries[provData.m_legalCountriesID[j]]->IsExist()) {
			prov->m_legitimateCountries.push_back( m_countries[provData.m_legalCountriesID[j]] );
			//}
		}
		prov->m_cultures.clear();
		for (int j = 0; j < (int)provData.m_cultures.size(); j++) {
			prov->m_cultures.push_back( std::pair<Culture*, float>( m_cultures[provData.m_cultures[j].first], provData.m_cultures[j].second ) );
		}
		prov->m_religions.clear();
		for (int j = 0; j < (int)provData.m_religions.size(); j++) {
			prov->m_religions.push_back( std::pair<Religion*, float>( m_religions[provData.m_religions[j].first], provData.m_religions[j].second ) );
		}
		prov->RecalculateMajorCulture();
		prov->RecalculateMajorReligion();
		if (provData.m_ownerCountryID != -1) {
			prov->m_owner = m_countries[provData.m_ownerCountryID];
			m_countries[provData.m_ownerCountryID]->m_provinces.push_back( prov );
			for (auto city : prov->m_cities) {
				m_countries[provData.m_ownerCountryID]->m_cities.push_back( city );
			}
			for (auto town : prov->m_cities) {
				m_countries[provData.m_ownerCountryID]->m_towns.push_back( town );
			}
		}
		else {
			prov->m_owner = nullptr;
		}
	}
	for (int i = 0; i < (int)data.m_cityData.size(); ++i) {
		HistoryCityData const& cityData = data.m_cityData[i];
		City* city = m_cities[i];
		city->m_defense = cityData.m_defenseValue;
		city->m_owner = m_countries[cityData.m_ownerID];
		city->m_cultures.clear();
		for (int j = 0; j < (int)cityData.m_cultures.size(); j++) {
			city->m_cultures.push_back( std::pair<Culture*, float>( m_cultures[cityData.m_cultures[j].first], cityData.m_cultures[j].second ) );
		}
		city->m_religions.clear();
		for (int j = 0; j < (int)cityData.m_religions.size(); j++) {
			city->m_religions.push_back( std::pair<Religion*, float>( m_religions[cityData.m_religions[j].first], cityData.m_religions[j].second ) );
		}
		city->m_totalPopulation = cityData.m_population;
		city->SetRawAttribute( cityData.m_type );
		city->RecalculateMajorCulture();
		city->RecalculateMajorReligion();
		city->SyncAttributes();
	}
	for (int i = 0; i < (int)data.m_townData.size(); ++i) {
		HistoryTownData const& townData = data.m_townData[i];
		Town* town = m_towns[i];
		town->m_defense = townData.m_defenseValue;
		town->m_owner = m_countries[townData.m_ownerID];
		town->m_cultures.clear();
		for (int j = 0; j < (int)townData.m_cultures.size(); j++) {
			town->m_cultures.push_back( std::pair<Culture*, float>( m_cultures[townData.m_cultures[j].first], townData.m_cultures[j].second ) );
		}
		town->m_religions.clear();
		for (int j = 0; j < (int)townData.m_religions.size(); j++) {
			town->m_religions.push_back( std::pair<Religion*, float>( m_religions[townData.m_religions[j].first], townData.m_religions[j].second ) );
		}
		town->m_totalPopulation = townData.m_population;
		town->RecalculateMajorCulture();
		town->RecalculateMajorReligion();
	}
	
	// delete inactive army
	for (auto country : m_countries) {
		for (int i = 0; i < (int)country->m_armies.size();) {
			if (!country->m_armies[i]->m_isActive) {
				RemoveArmyFromMap( country->m_armies[i], GetCurMap() );
			}
			else {
				++i;
			}
		}

	}
	for (auto country : m_countries) {
		// recalculating and normalize the economy data
		//country->EndTurn();
		// recalculate all provinces for country
		std::vector<MapPolygonUnit*> provs = country->m_provinces;
		country->m_provinces.clear();
		country->m_cities.clear();
		country->m_towns.clear();
		country->m_totalPopulation = 0;
		country->m_cultures.clear();
		country->m_religions.clear();
		for (auto prov : provs) {
			country->GainProvince( prov, false );
		}
		country->CalculateMilitaryStats();
		country->ReCalculateCultureAndReligion();
		country->CalculateMajorCulture();
		country->CalculateMajorReligion();
		country->CalculateEconomicValue();
	}
// 	for (auto prov : m_mapPolygonUnits) {
// 		prov->RemoveUnqualifiedLegitimateCountry();
// 	}
}

void Map::RearrangeHistoryCache()
{
	int curViewingYearIndex = 12 * m_viewingYear + m_viewingMonth - 1;
	for (int i = 0; i < (int)m_historyData.size(); ++i) {
		// load from file
		if (std::abs( i - curViewingYearIndex ) <= MAX_SAVE_MONTH && m_historyData[i] == nullptr) {
			int year = (i + 1) % 12 == 0 ? (i + 1) / 12 - 1 : (i + 1) / 12;
			int month = (i + 1) % 12 == 0 ? 12 : (i + 1) % 12;
			m_historyData[i] = new HistoryData();
			LoadHistoryCacheFromDisk( *m_historyData[i], year, month);

			//LoadHistoryFromDiskJob* job = new LoadHistoryFromDiskJob( Stringf( "Saves/%d/HistoryCache/Year%d_Month%d_History.his", m_generationSettings.m_seed, year, month ), year, month, this );
			//g_theJobSystem->AddJob( job );
		}
		// save to file
		else if (std::abs( i - curViewingYearIndex ) >= MAX_SAVE_MONTH && m_historyData[i] != nullptr) {
			std::vector<uint8_t> binData;
			m_historyData[i]->DumpToBinaryFormat( binData );
			std::filesystem::create_directory( Stringf( "Saves/%d", m_generationSettings.m_seed ).c_str() );
			std::filesystem::create_directory( Stringf( "Saves/%d/HistoryCache", m_generationSettings.m_seed ).c_str() );
			std::string filePath = Stringf( "Saves/%d/HistoryCache/Year%d_Month%d_History.his", m_generationSettings.m_seed, (i + 1) % 12 == 0 ? (i + 1) / 12 - 1 : (i + 1) / 12, (i + 1) % 12 == 0 ? 12 : (i + 1) % 12 );
			struct zip_t* zip = zip_open( filePath.c_str(),
				ZIP_DEFAULT_COMPRESSION_LEVEL, 'w' );
			{
				zip_entry_open( zip, "History.his" );
				{
					zip_entry_write( zip, binData.data(), binData.size() );
				}
				zip_entry_close( zip );
			}
			zip_close( zip );
			//BufferWriteToFile( binData, Stringf( "Saves/%d/HistoryCache/Year%d_Month%d_History.his", m_generationSettings.m_seed, (i + 1) % 12 == 0 ? (i + 1) / 12 - 1: (i + 1) / 12, (i + 1) % 12 == 0 ? 12 : (i + 1) % 12 ) );
			delete m_historyData[i];
			m_historyData[i] = nullptr;
		}
	}
}

bool Map::DoHistoryExist( int year, int month ) const
{
	int index = 12 * year + month - 1;
	if (index >= (int)m_historyData.size()) {
		return false;
	}
	return true;
}

void Map::ProcessHistoryCrisis()
{
	std::sort( m_crisis.begin(), m_crisis.end(), []( HistoryCrisis* a, HistoryCrisis* b ) {
		if (a == nullptr) {
			return false;
		}
		if (b == nullptr) {
			return true;
		}
		return a->m_globalID < b->m_globalID; 
		} );
	// create new crisis
	for (auto country : m_countries) {
		if (country->IsExist()) {
			// large country with too less money -> civil war
			if (country->m_totalPopulation > 100000 && country->m_funds < 0) {
				HistoryCrisis* newCrisis = new HistoryCrisis();
				newCrisis->m_type = CrisisType::CivilWar;
				newCrisis->m_country = country;
				newCrisis->m_progress = 0.f;
				newCrisis->m_globalID = m_globalHistoryCrisisID;
				++m_globalHistoryCrisisID;
				if (m_globalHistoryCrisisID == INVALID_ARMY_ID) {
					++m_globalHistoryCrisisID;
				}
				AddNewCrisisToList( newCrisis );
			}
			// different culture -> culture conflict
			for (auto prov : country->m_provinces) {
				if (prov->m_majorCulture != country->m_countryCulture && !prov->IsLegitimateToCountry( country ) && prov->m_totalPopulation > 10000) {
					bool hasSameCrisis = false;
					// make sure no same crisis is in progress
					for (auto crisis : m_crisis) {
						if (crisis && crisis->m_country == country && crisis->m_cultureOrReligion == prov->m_majorCulture) {
							hasSameCrisis = true;
						}
					}
					if (!hasSameCrisis) {
						HistoryCrisis* newCrisis = new HistoryCrisis();
						newCrisis->m_type = CrisisType::CultureConflict;
						newCrisis->m_country = country;
						//newCrisis->m_progress = country->GetCultureInfluence( prov->m_majorCulture );
						newCrisis->m_cultureOrReligion = prov->m_majorCulture;
						newCrisis->m_globalID = m_globalHistoryCrisisID;
						++m_globalHistoryCrisisID;
						if (m_globalHistoryCrisisID == INVALID_ARMY_ID) {
							++m_globalHistoryCrisisID;
						}
						AddNewCrisisToList( newCrisis );
					}
				}
			}
			// different religion -> religion conflict
			for (auto prov : country->m_provinces) {
				if (prov->m_majorReligion != country->m_countryReligion && !prov->IsLegitimateToCountry( country ) && prov->m_totalPopulation > 10000) {
					bool hasSameCrisis = false;
					// make sure no same crisis is in progress
					for (auto crisis : m_crisis) {
						if (crisis && crisis->m_country == country && crisis->m_cultureOrReligion == prov->m_majorReligion) {
							hasSameCrisis = true;
						}
					}
					if (!hasSameCrisis) {
						HistoryCrisis* newCrisis = new HistoryCrisis();
						newCrisis->m_type = CrisisType::ReligionConflict;
						newCrisis->m_country = country;
						//newCrisis->m_progress = country->GetReligionInfluence( prov->m_majorReligion );
						newCrisis->m_cultureOrReligion = prov->m_majorReligion;
						newCrisis->m_globalID = m_globalHistoryCrisisID;
						++m_globalHistoryCrisisID;
						if (m_globalHistoryCrisisID == INVALID_ARMY_ID) {
							++m_globalHistoryCrisisID;
						}
						AddNewCrisisToList( newCrisis );
					}
				}
			}
		}
	}

	// process all crisis
	for (int i = 0; i < (int)m_crisis.size(); ++i) {
		HistoryCrisis* crisis = m_crisis[i];
		if (crisis) {
			if (!crisis->m_country->IsExist()) {
				delete crisis;
				m_crisis[i] = nullptr;
			}
			else if (crisis->m_progress >= 1.f) {
				// do nothing
			}
			else if (crisis->m_type == CrisisType::CivilWar) {
				// if has enough funds the reason of civil war is eliminated
				if (crisis->m_country->m_funds > crisis->m_country->m_economyValue * 2) {
					delete crisis;
					m_crisis[i] = nullptr;
				}
				else {
					float totalProgress = 0.f;
					float progressFactor = 1.f;
					if (crisis->m_country->m_countryCulture->HasTrait( CultureTrait::Filial )) {
						progressFactor -= 0.3f;
					}
					if (crisis->m_country->m_funds < -crisis->m_country->m_economyValue) {
						totalProgress += 0.005f;
					}
					if (crisis->m_country->m_funds < crisis->m_country->m_economyValue) {
						totalProgress += 0.005f;
					}
					crisis->m_progress += totalProgress * progressFactor;
				}
			}
			else if (crisis->m_type == CrisisType::CultureConflict) {
				bool canProgress = false;
				for (auto prov : crisis->m_country->m_provinces) {
					if (prov->m_majorCulture == crisis->m_cultureOrReligion) {
						canProgress = true;
					}
				}
				if (!canProgress) {
					delete crisis;
					m_crisis[i] = nullptr;
				}
				else {
					float totalProgress = 0.005f;
					float progressFactor = 1.f;
					if (((Culture*)crisis->m_cultureOrReligion)->HasTrait( CultureTrait::Docile )) {
						progressFactor -= 0.5f;
					}
					if (((Culture*)crisis->m_cultureOrReligion)->HasTrait( CultureTrait::Free )) {
						progressFactor += 0.5f;
					}
					if (crisis->m_country->m_countryCulture->HasTrait( CultureTrait::Xenophobe )) {
						progressFactor += 0.2f;
					}
					if (crisis->m_country->m_countryCulture->HasTrait( CultureTrait::Tolerant )) {
						progressFactor -= 0.2f;
					}
					if (crisis->m_country->GetCulturePopulation( (Culture*)crisis->m_cultureOrReligion ) > 50000) {
						totalProgress += 0.008f;
					}
					if (crisis->m_country->GetCulturePopulation( (Culture*)crisis->m_cultureOrReligion ) > 200000) {
						totalProgress += 0.008f;
					}
					crisis->m_progress += totalProgress * progressFactor;
				}
			}
			else if (crisis->m_type == CrisisType::ReligionConflict) {
				bool canProgress = false;
				for (auto prov : crisis->m_country->m_provinces) {
					if (prov->m_majorReligion == crisis->m_cultureOrReligion) {
						canProgress = true;
					}
				}
				if (!canProgress) {
					delete crisis;
					m_crisis[i] = nullptr;
				}
				else {
					float totalProgress = 0.005f;
					float progressFactor = 1.f;
					if (crisis->m_country->m_countryCulture->HasTrait( CultureTrait::Tolerant )) {
						progressFactor -= 0.2f;
					}
					if (crisis->m_country->GetReligionPopulation( (Religion*)crisis->m_cultureOrReligion ) > 50000) {
						totalProgress += 0.008f;
					}
					if (crisis->m_country->GetReligionPopulation( (Religion*)crisis->m_cultureOrReligion ) > 200000) {
						totalProgress += 0.008f;
					}
					crisis->m_progress += totalProgress * progressFactor;
				}
			}
		}
	}

	// trigger crisis
	for (int i = 0; i < (int)m_crisis.size(); ++i) {
		HistoryCrisis* crisis = m_crisis[i];
		if (crisis && crisis->m_progress >= 1.f) {
			if (crisis->m_type == CrisisType::CivilWar) {
				crisis->m_country->RemoveAllRelations();
				std::vector<Army*> armyCopy = crisis->m_country->m_armies;
				for (auto army : armyCopy) {
					RemoveArmyFromMap( army, this );
				}
				// separate the country into several countries
				int numOfCountries = std::min( std::max( (int)crisis->m_country->m_provinces.size() / 20, 3 ), (int)crisis->m_country->m_provinces.size() );
				std::vector<Province*> startProvs;
				std::vector<std::vector<Province*>> provToHave;
				startProvs.reserve( numOfCountries );
				for (int _ = 0; _ < numOfCountries; ++_) {
					Province* testProv = nullptr;
					do {
						testProv = crisis->m_country->m_provinces[m_historyRNG->RollRandomIntLessThan( (int)crisis->m_country->m_provinces.size() )];
					} while (std::find( startProvs.begin(), startProvs.end(), testProv ) != startProvs.end());
					startProvs.push_back( testProv );
				}
				provToHave.resize( numOfCountries );
				for (auto province : crisis->m_country->m_provinces) {
					int minIndex = -1;
					float minValue = FLT_MAX;
					for (int k = 0; k < (int)startProvs.size(); ++k) {
						float dist = GetDistanceSquared2D( province->m_geoCenterPos, startProvs[k]->m_geoCenterPos );
						if (dist < minValue) {
							minIndex = k;
							minValue = dist;
						}
					}
					provToHave[minIndex].push_back( province );
				}

				int n = 0;
				for (auto const& provList : provToHave) {
					//CreateNewCountry( provList, crisis->m_country->m_countryCulture, crisis->m_country->m_countryReligion, crisis->m_country->m_governmentType );
					std::vector<Country*> countryList;
					for (auto country : startProvs[n]->m_legitimateCountries) {
						if (country->m_countryCulture == crisis->m_country->m_countryCulture && country->m_countryReligion == crisis->m_country->m_countryReligion) {
							countryList.push_back( country );
						}
					}
					if ((int)countryList.size() > 0) {
						Country* country = countryList[m_historyRNG->RollRandomIntLessThan( (int)countryList.size() )];
						bool isExist = country->IsExist();
						if (!isExist) {
							country->Reinitialize();
							country->m_countryCulture = crisis->m_country->m_countryCulture;
							country->m_countryReligion = crisis->m_country->m_countryReligion;
						}
						for (auto prov : provList) {
							crisis->m_country->LoseProvince( prov, true );
							country->GainProvince( prov );
						}
						if (!isExist) {
							country->ResetCapitalProvince();
							country->RemoveAllRelations();
							country->CalculateEconomicValue();
							country->SetUpSimulation();
							country->m_funds *= 5;
						}
					}
					else {
						// create a new country
						CreateNewCountry( provList, crisis->m_country->m_countryCulture, crisis->m_country->m_countryReligion, crisis->m_country->m_governmentType );
					}
					++n;
				}
				AddHistoryLog( Stringf( "Year%d %s: Civil war began in %s\n", m_year, g_monthShortNameMap[m_month - 1].c_str(), crisis->m_country->m_name.c_str() ) );
			}
			else if (crisis->m_type == CrisisType::CultureConflict) {
				std::vector<Province*> provToSeparate;
				std::set<Country*> countryNameToUse;
				for (auto prov : crisis->m_country->m_provinces) {
					if (prov->m_majorCulture == crisis->m_cultureOrReligion) {
						provToSeparate.push_back( prov );
					}
					for (auto country : prov->m_legitimateCountries) {
						if (country->m_countryCulture == crisis->m_cultureOrReligion && country != crisis->m_country ){
							if (country->IsExist()
								&& (country->GetRelationTo( crisis->m_country ) == CountryRelationType::None
									|| country->GetRelationTo( crisis->m_country ) == CountryRelationType::Hostile
									|| country->GetRelationTo( crisis->m_country ) == CountryRelationType::War)) {
								countryNameToUse.insert( country );
							}
							else if (!country->IsExist()) {
								countryNameToUse.insert( country );
							}
						}
					}
				}
				std::vector<Country*> countryList = std::vector<Country*>( countryNameToUse.begin(), countryNameToUse.end() );
				std::sort( countryList.begin(), countryList.end(), []( Country* a, Country* b ) { return a->m_id < b->m_id; } );
				// establish or join a country
				if ((int)countryList.size() > 0) {
					Country* country = countryList[m_historyRNG->RollRandomIntLessThan( (int)countryList.size() )];
					bool isExist = country->IsExist();
					if (!isExist) {
						country->Reinitialize();
						country->m_countryCulture = (Culture*)crisis->m_cultureOrReligion;
						country->m_countryReligion = crisis->m_country->m_countryReligion;
					}
					for (auto prov : provToSeparate) {
						crisis->m_country->LoseProvince( prov, true );
						country->GainProvince( prov );
					}
					if (!isExist) {
						country->ResetCapitalProvince();
						country->RemoveAllRelations();
						country->CalculateEconomicValue();
						country->SetUpSimulation();
						country->m_funds *= 5;
					}
					// add war relation to countries which receive these provinces
					// only this can avoid the issue, can not put this out of if condition
					if (!country->IsInWarWith( crisis->m_country )) {
						country->RemoveAllRelationWithCountry( crisis->m_country );
						country->AddWarRelation( crisis->m_country );
					}
				}
				else {
					// create a new country
					Country* newCountry = CreateNewCountry( provToSeparate, (Culture*)crisis->m_cultureOrReligion, crisis->m_country->m_countryReligion, CountryGovernmentType::Separatism );
					newCountry->AddWarRelation( crisis->m_country );
				}
				AddHistoryLog( Stringf( "Year%d %s: Several provinces with %s culture major influence separated from %s because of culture conflict\n",
					m_year, g_monthShortNameMap[m_month - 1].c_str(), ((Culture*)crisis->m_cultureOrReligion)->m_name.c_str(), crisis->m_country->m_name.c_str() ) );
			}
			else if (crisis->m_type == CrisisType::ReligionConflict) {
				std::vector<Province*> provToSeparate;
				std::set<Country*> countryNameToUse;
				for (auto prov : crisis->m_country->m_provinces) {
					if (prov->m_majorReligion == crisis->m_cultureOrReligion) {
						provToSeparate.push_back( prov );
					}
					for (auto country : prov->m_legitimateCountries) {
						if (country->m_countryReligion == crisis->m_cultureOrReligion && country != crisis->m_country) {
							if (country->IsExist()
								&& (country->GetRelationTo( crisis->m_country ) == CountryRelationType::None
									|| country->GetRelationTo( crisis->m_country ) == CountryRelationType::Hostile
									|| country->GetRelationTo( crisis->m_country ) == CountryRelationType::War)) {
								countryNameToUse.insert( country );
							}
							else if (!country->IsExist()) {
								countryNameToUse.insert( country );
							}
						}
					}
				}
				std::vector<Country*> countryList = std::vector<Country*>( countryNameToUse.begin(), countryNameToUse.end() );
				std::sort( countryList.begin(), countryList.end(), []( Country* a, Country* b ) { return a->m_id < b->m_id; } );
				// establish or join a country
				if ((int)countryList.size() > 0) {
					Country* country = countryList[m_historyRNG->RollRandomIntLessThan( (int)countryList.size() )];
					bool isExist = country->IsExist();
					if (!isExist) {
						country->Reinitialize();
						country->m_countryCulture = crisis->m_country->m_countryCulture;
						country->m_countryReligion = (Religion*)crisis->m_cultureOrReligion;
					}
					for (auto prov : provToSeparate) {
						crisis->m_country->LoseProvince( prov, true );
						country->GainProvince( prov );
					}
					if (!isExist) {
						country->ResetCapitalProvince();
						country->RemoveAllRelations();
						country->CalculateEconomicValue();
						country->SetUpSimulation();
						country->m_funds *= 5;
					}
					// add war relation to countries which receive these provinces
					// only this can avoid the issue, can not put this out of if condition
					if (!country->IsInWarWith( crisis->m_country )) {
						country->RemoveAllRelationWithCountry( crisis->m_country );
						country->AddWarRelation( crisis->m_country );
					}
				}
				else {
					// create a new country
					Country* newCountry = CreateNewCountry( provToSeparate, crisis->m_country->m_countryCulture, (Religion*)crisis->m_cultureOrReligion, CountryGovernmentType::Separatism );
					newCountry->AddWarRelation( crisis->m_country );
				}
				AddHistoryLog( Stringf( "Year%d %s: Several provinces with %s religion major influence separated from %s because of religion conflict\n",
					m_year, g_monthShortNameMap[m_month - 1].c_str(), ((Religion*)crisis->m_cultureOrReligion)->m_name.c_str(), crisis->m_country->m_name.c_str() ) );
			}
			delete crisis;
			m_crisis[i] = nullptr;
		}
	}
}

void Map::AddNewCrisisToList( HistoryCrisis* crisis )
{
	for (int i = 0; i < (int)m_crisis.size(); ++i) {
		if (m_crisis[i] == nullptr) {
			m_crisis[i] = crisis;
			return;
		}
	}
	m_crisis.push_back( crisis );
}

HistoryData const& Map::GetHistoryData( int year, int month )
{
	//HistoryData* hisData = new HistoryData();
	//LoadHistoryCacheFromDisk( *hisData, year, month );
	//m_historyData.push_back( hisData );
	int index = 12 * year + month - 1;
	while (m_historyData[index] == nullptr) {
		//std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		m_historyData[index] = new HistoryData();
		int const& i = index;
		LoadHistoryCacheFromDisk( *m_historyData[index], (i + 1) % 12 == 0 ? (i + 1) / 12 - 1: (i + 1) / 12, (i + 1) % 12 == 0 ? 12 : (i + 1) % 12);
	}
	//while (g_theJobSystem->RetriveOldestCompletedJob());
	return *m_historyData[index];
	//return *m_historyData[m_historyData.size() - 1];
}

void Map::GetHistoryData( int year, int month, HistoryData& data )
{
	int index = 12 * year + month - 1;
	if (m_historyData[index] == nullptr) {
		int const& i = index;
		LoadHistoryCacheFromDisk( data, (i + 1) % 12 == 0 ? (i + 1) / 12 - 1 : (i + 1) / 12, (i + 1) % 12 == 0 ? 12 : (i + 1) % 12 );
	}
	else {
		data = *m_historyData[index];
	}
}

Army* Map::GetArmyByGlobalID( unsigned int id ) const
{
	for (auto country : m_countries) {
		for (auto army : country->m_armies) {
			if (army->m_globalID == id) {
				return army;
			}
		}
	}
	return nullptr;
}

HistoryCrisis* Map::GetCrisisByGlobalID( unsigned int id ) const
{
	for (auto crisis : m_crisis) {
		if (crisis && crisis->m_globalID == id) {
			return crisis;
		}
	}
	return nullptr;
}

Country* Map::CreateNewCountry( std::vector<Province*> const& initialProvince, Culture* countryCulture, Religion* countryReligion, CountryGovernmentType govType )
{
	Country* newCountry = new Country();
	m_countries.push_back( newCountry );
	newCountry->m_id = (int)m_countries.size() - 1;
	//newCountry->m_originProv = ;
	newCountry->m_countryCulture = countryCulture;
	newCountry->m_countryReligion = countryReligion;
	newCountry->m_governmentType = govType;
	for (auto prov : initialProvince) {
		prov->m_owner->LoseProvince( prov, true );
		newCountry->GainProvince( prov );
	}
	newCountry->m_name = newCountry->m_countryCulture->m_countryNameGenerator->GenerateCountryName( newCountry );
	for (auto prov : newCountry->m_provinces) {
		prov->AddLegitimateCountry( newCountry );
	}
	newCountry->ResetCapitalProvince();
	newCountry->CalculateEconomicValue();
	newCountry->SetUpSimulation();
	newCountry->m_funds *= 3;
	m_countryLabels.push_back( new Label( newCountry, LabelType::Country ) );
	m_countryLabels[m_countryLabels.size() - 1]->ReCalculateVertexData();
	return newCountry;
}

void Map::AddInstructionToQueue( CountryInstruction* instr )
{
	m_instructionQueue.push_back( instr );
}

bool Map::IsTwoCountryPassEndWarTime( Country* country1, Country* country2, int numOfMonth /*= 24 */ )
{
	for (int i = 0; i < (int)m_endWarTimes.size();) {
		CountryEndWarTreaty const& treaty = m_endWarTimes[i];
		int diff = GetMonthDiffBetweenTwoTimes( m_year, m_month, treaty.m_year, treaty.m_month );
		if (diff > numOfMonth) {
			if ((treaty.m_country1 == country1 && treaty.m_country2 == country2)
				|| (treaty.m_country1 == country2 && treaty.m_country2 == country1)) {
				m_endWarTimes.erase( m_endWarTimes.begin() + i );
				return true;
			}
			m_endWarTimes.erase( m_endWarTimes.begin() + i );
		}
		else {
			if ((treaty.m_country1 == country1 && treaty.m_country2 == country2)
				|| (treaty.m_country1 == country2 && treaty.m_country2 == country1)) {
				return false;
			}
			++i;
		}
	}
	return true;
}

int Map::GetMonthDiffBetweenTwoTimes( int year1, int month1, int year2, int month2 )
{
	int yearDiff = std::abs( year2 - year1 );
	int monthDiff;
	if (year2 > year1) {
		monthDiff = month2 - month1;
	}
	else if (year2 < year1) {
		monthDiff = month1 - month2;
	}
	else {
		monthDiff = abs( month1 - month2 );
	}
	return yearDiff * 12 + monthDiff;
}

int Map::GetTotalMonthCount() const
{
	return m_year * 12 + m_month - 1;
}

void Map::GetYearAndMonthFromTotalMonth( int& year, int& month, int totalMonth ) const
{
	year = totalMonth / 12;
	month = totalMonth - year * 12 + 1;
}

void Map::SaveCurrentWorldToXml() const
{
	// provinces
	XmlDocument save;
	XmlElement* rootElem = save.NewElement( "World" );
	// dimensions and time
	SetXmlAttribute( rootElem, "Dimensions", m_dimensions );
	SetXmlAttribute( rootElem, "Year", m_viewingYear );
	SetXmlAttribute( rootElem, "Month", m_viewingMonth );
	save.InsertFirstChild( rootElem );
	XmlElement* provRootElem = save.NewElement( "Provinces" );
	rootElem->InsertEndChild( provRootElem );
	for (auto province : m_mapPolygonUnits) {
		if (province->m_isFarAwayFakeUnit) {
			continue;
		}
		XmlElement* provElem = save.NewElement( "Province" );
		provRootElem->InsertEndChild( provElem );
		SetXmlAttribute( provElem, "ID", province->m_id );
		SetXmlAttribute( provElem, "Name", province->m_name );
		SetXmlAttribute( provElem, "Height", province->m_height );
		SetXmlAttribute( provElem, "Longitude", province->m_longitude );
		SetXmlAttribute( provElem, "Latitude", province->m_latitude );
		SetXmlAttribute( provElem, "Climate", g_climateNameMap[(int)province->m_climate] );
		if (!province->IsWater()) {
			SetXmlAttribute( provElem, "SummerPrecipitation", province->m_summerPrecipitation );
			SetXmlAttribute( provElem, "WinterPrecipitation", province->m_winterPrecipitation );
			SetXmlAttribute( provElem, "SummerAvgTemperature", province->m_summerAvgTemperature );
			SetXmlAttribute( provElem, "WinterAvgTemperature", province->m_winterAvgTemperature );
			SetXmlAttribute( provElem, "Population", province->m_totalPopulation );
		}
		SetXmlAttribute( provElem, "AreaSize", province->m_areaSize );
		if (!province->IsWater()) {
			SetXmlAttribute( provElem, "MajorCulture", province->m_majorCulture->m_name );
			SetXmlAttribute( provElem, "MajorReligion", province->m_majorReligion->m_name );
			SetXmlAttribute( provElem, "Products", g_productNameMap[(int)province->m_productType] );
			SetXmlAttribute( provElem, "Continent", province->m_continent->m_name );
			SetXmlAttribute( provElem, "Region", province->m_region->m_name );
		}
		if (province->m_owner) {
			SetXmlAttribute( provElem, "OwnerCountry", province->m_owner->m_name );
		}
		else {
			SetXmlAttribute( provElem, "OwnerCountry", "None");
		}
		SetXmlAttribute( provElem, "Landform", g_landformNameMap[(int)province->m_landform] );
		SetXmlAttribute( provElem, "Coast", province->m_isCoast );
		if (!province->IsWater()) {
			XmlElement* provCultureElem = save.NewElement( "ProvinceCultures" );
			provElem->InsertEndChild( provCultureElem );
			for (auto& pair : province->m_cultures) {
				provCultureElem->SetAttribute( pair.first->m_name.c_str(), pair.second );
			}
			XmlElement* provReligionElem = save.NewElement( "ProvinceReligions" );
			provElem->InsertEndChild( provReligionElem );
			for (auto& pair : province->m_religions) {
				provReligionElem->SetAttribute( pair.first->m_name.c_str(), pair.second );
			}
			XmlElement* provCityElems = save.NewElement( "ProvinceCities" );
			provElem->InsertEndChild( provCityElems );
			for (auto city : province->m_cities) {
				XmlElement* provCityElem = save.NewElement( "City" );
				provCityElems->InsertEndChild( provCityElem );
				SetXmlAttribute( provCityElem, "Name", city->m_name.c_str() );
			}
			XmlElement* provTownElems = save.NewElement( "ProvinceTowns" );
			provElem->InsertEndChild( provTownElems );
			for (auto town : province->m_towns) {
				XmlElement* provTownElem = save.NewElement( "Town" );
				provTownElems->InsertEndChild( provTownElem );
				SetXmlAttribute( provTownElem, "Name", town->m_name.c_str() );
			}
			XmlElement* provLegitimateCountryElems = save.NewElement( "ProvinceLegitimateCountries" );
			provElem->InsertEndChild( provLegitimateCountryElems );
			for (auto country : province->m_legitimateCountries) {
				XmlElement* provLegitCountryElem = save.NewElement( "LegitimateCountry" );
				provLegitimateCountryElems->InsertEndChild( provLegitCountryElem );
				SetXmlAttribute( provLegitCountryElem, "Name", country->m_name.c_str() );
			}
		}

		std::string pointList;
		for (auto edge : province->m_edges) {
			for (auto const& vec2 : edge->m_noisyEdges) {
				pointList += Stringf( "%f,%f ", vec2.x, vec2.y );
			}
		}
		pointList = pointList.substr( 0, pointList.size() - 1 );
		SetXmlAttribute( provElem, "EdgePoints", pointList.c_str() );
	}

	// cities
	XmlElement* cityRootElem = save.NewElement( "Cities" );
	rootElem->InsertEndChild( cityRootElem );
	for (auto city : m_cities) {
		XmlElement* cityElem = save.NewElement( "City" );
		cityRootElem->InsertEndChild( cityElem );
		SetXmlAttribute( cityElem, "ID", city->m_id );
		SetXmlAttribute( cityElem, "Name", city->m_name );
		SetXmlAttribute( cityElem, "Position", city->m_position );
		SetXmlAttribute( cityElem, "Defense", city->m_defense );
		SetXmlAttribute( cityElem, "MaxDefense", city->m_maxDefense );
		SetXmlAttribute( cityElem, "ProvinceID", city->m_provIn->m_id );
		SetXmlAttribute( cityElem, "ProvinceName", city->m_provIn->m_name );
		SetXmlAttribute( cityElem, "OwnerCountry", city->m_owner->m_name );
		SetXmlAttribute( cityElem, "Region", city->m_region->m_name );
		XmlElement* cityCultureElem = save.NewElement( "CityCultures" );
		cityElem->InsertEndChild( cityCultureElem );
		for (auto& pair : city->m_cultures) {
			cityCultureElem->SetAttribute( pair.first->m_name.c_str(), pair.second );
		}
		XmlElement* cityReligionElem = save.NewElement( "CityReligions" );
		cityElem->InsertEndChild( cityReligionElem );
		for (auto& pair : city->m_religions) {
			cityReligionElem->SetAttribute( pair.first->m_name.c_str(), pair.second );
		}
		SetXmlAttribute( cityElem, "Attributes", city->GetCityAttributeAsString() );
	}

	// towns
	XmlElement* townRootElem = save.NewElement( "Towns" );
	rootElem->InsertEndChild( townRootElem );
	for (auto town : m_towns) {
		XmlElement* townElem = save.NewElement( "Town" );
		townRootElem->InsertEndChild( townElem );
		SetXmlAttribute( townElem, "ID", town->m_id );
		SetXmlAttribute( townElem, "Name", town->m_name );
		SetXmlAttribute( townElem, "Position", town->m_position );
		//SetXmlAttribute( townElem, "Defense", town->m_defense );
		//SetXmlAttribute( townElem, "MaxDefense", town->m_maxDefense );
		SetXmlAttribute( townElem, "ProvinceID", town->m_provIn->m_id );
		SetXmlAttribute( townElem, "ProvinceName", town->m_provIn->m_name );
		SetXmlAttribute( townElem, "OwnerCountry", town->m_owner->m_name );
		SetXmlAttribute( townElem, "Region", town->m_region->m_name );
		XmlElement* townCultureElem = save.NewElement( "TownCultures" );
		townElem->InsertEndChild( townCultureElem );
		for (auto& pair : town->m_cultures) {
			townCultureElem->SetAttribute( pair.first->m_name.c_str(), pair.second );
		}
		XmlElement* townReligionElem = save.NewElement( "TownReligions" );
		townElem->InsertEndChild( townReligionElem );
		for (auto& pair : town->m_religions) {
			townReligionElem->SetAttribute( pair.first->m_name.c_str(), pair.second );
		}
	}

	// countries
	XmlElement* countryRootElem = save.NewElement( "Countries" );
	rootElem->InsertEndChild( countryRootElem );
	for (auto country : m_countries) {
		XmlElement* countryElem = save.NewElement( "Country" );
		countryRootElem->InsertEndChild( countryElem );
		SetXmlAttribute( countryElem, "ID", country->m_id );
		SetXmlAttribute( countryElem, "Name", country->m_name );
		SetXmlAttribute( countryElem, "Color", country->m_color );
		SetXmlAttribute( countryElem, "Exist", country->IsExist() );
		if (country->IsExist()) {
			SetXmlAttribute( countryElem, "Population", country->m_totalPopulation );
			SetXmlAttribute( countryElem, "Funds", country->m_funds );
			SetXmlAttribute( countryElem, "Economy", country->m_economyValue );
			SetXmlAttribute( countryElem, "MilitaryStrength", country->m_totalMilitaryStrength );
			SetXmlAttribute( countryElem, "CountryCulture", country->m_countryCulture->m_name );
			SetXmlAttribute( countryElem, "CountryReligion", country->m_countryReligion->m_name );
			SetXmlAttribute( countryElem, "majorCulture", country->m_majorCulture->m_name );
			SetXmlAttribute( countryElem, "majorReligion", country->m_majorReligion->m_name );
			SetXmlAttribute( countryElem, "GovernmentType", g_governmentTypeDict[(int)country->m_governmentType] );
			if (country->m_capitalProv) {
				SetXmlAttribute( countryElem, "CapitalProvinceID", country->m_capitalProv->m_id );
				SetXmlAttribute( countryElem, "CapitalProvinceName", country->m_capitalProv->m_name );
			}
			if (country->m_capitalCity) {
				SetXmlAttribute( countryElem, "CapitalCityID", country->m_capitalCity->m_id );
				SetXmlAttribute( countryElem, "CapitalCityName", country->m_capitalCity->m_name );
			}
			if (country->m_originProv) {
				SetXmlAttribute( countryElem, "OriginProvinceID", country->m_originProv->m_id );
				SetXmlAttribute( countryElem, "OriginProvinceName", country->m_originProv->m_name );
			}
			XmlElement* countryCultureElem = save.NewElement( "CountryCultures" );
			countryCultureElem->InsertEndChild( countryCultureElem );
			for (auto& pair : country->m_cultures) {
				countryCultureElem->SetAttribute( pair.first->m_name.c_str(), (float)pair.second / (float)country->m_totalPopulation );
			}
			XmlElement* countryReligionElem = save.NewElement( "CountryReligions" );
			countryReligionElem->InsertEndChild( countryReligionElem );
			for (auto& pair : country->m_religions) {
				countryReligionElem->SetAttribute( pair.first->m_name.c_str(), (float)pair.second / (float)country->m_totalPopulation );
			}
			std::string provList;
			for (auto province : country->m_provinces) {
				provList += Stringf( "%d ", province->m_id );
			}
			provList = provList.substr( 0, provList.size() - 1 );
			SetXmlAttribute( countryElem, "OwnedProvincesList", provList );
			std::string cityList;
			for (auto city : country->m_cities) {
				cityList += Stringf( "%d ", city->m_id );
			}
			cityList = cityList.substr( 0, cityList.size() - 1 );
			SetXmlAttribute( countryElem, "OwnedCitiesList", cityList );
			std::string townList;
			for (auto town : country->m_towns) {
				townList += Stringf( "%d ", town->m_id );
			}
			townList = townList.substr( 0, townList.size() - 1 );
			SetXmlAttribute( countryElem, "OwnedTownsList", townList );

			if ((int)country->m_relationFriendlyCountries.size() > 0) {
				XmlElement* friendlyCountriesElem = save.NewElement( "FriendlyCountries" );
				countryElem->InsertEndChild( friendlyCountriesElem );
				for (auto otherCountry : country->m_relationFriendlyCountries) {
					XmlElement* friendlyCountryElem = save.NewElement( "FriendlyCountry" );
					friendlyCountriesElem->InsertEndChild( friendlyCountryElem );
					SetXmlAttribute( friendlyCountryElem, "Name", otherCountry->m_name.c_str() );
					SetXmlAttribute( friendlyCountryElem, "ID", otherCountry->m_id );
				}
			}
			if ((int)country->m_relationAllianceCountries.size() > 0) {
				XmlElement* allianceCountriesElem = save.NewElement( "AllianceCountries" );
				countryElem->InsertEndChild( allianceCountriesElem );
				for (auto otherCountry : country->m_relationAllianceCountries) {
					XmlElement* allianceCountryElem = save.NewElement( "AllianceCountry" );
					allianceCountriesElem->InsertEndChild( allianceCountryElem );
					SetXmlAttribute( allianceCountryElem, "Name", otherCountry->m_name.c_str() );
					SetXmlAttribute( allianceCountryElem, "ID", otherCountry->m_id );
				}
			}
			if ((int)country->m_relationHostileCountries.size() > 0) {
				XmlElement* hostileCountriesElem = save.NewElement( "HostileCountries" );
				countryElem->InsertEndChild( hostileCountriesElem );
				for (auto otherCountry : country->m_relationHostileCountries) {
					XmlElement* hostileCountryElem = save.NewElement( "HostileCountry" );
					hostileCountriesElem->InsertEndChild( hostileCountryElem );
					SetXmlAttribute( hostileCountryElem, "Name", otherCountry->m_name.c_str() );
					SetXmlAttribute( hostileCountryElem, "ID", otherCountry->m_id );
				}
			}
			if ((int)country->m_relationWarCountries.size() > 0) {
				XmlElement* warCountriesElem = save.NewElement( "WarCountries" );
				countryElem->InsertEndChild( warCountriesElem );
				for (auto otherCountry : country->m_relationWarCountries) {
					XmlElement* warCountryElem = save.NewElement( "WarCountry" );
					warCountriesElem->InsertEndChild( warCountryElem );
					SetXmlAttribute( warCountryElem, "Name", otherCountry->m_name.c_str() );
					SetXmlAttribute( warCountryElem, "ID", otherCountry->m_id );
				}
			}
			if (country->m_relationSuzerain) {
				SetXmlAttribute( countryElem, "SuzerainCountry", country->m_relationSuzerain->m_name );
			}
			if ((int)country->m_relationVassals.size() > 0) {
				XmlElement* vassalCountriesElem = save.NewElement( "Vassals" );
				countryElem->InsertEndChild( vassalCountriesElem );
				for (auto otherCountry : country->m_relationVassals) {
					XmlElement* vassalCountryElem = save.NewElement( "Vassal" );
					vassalCountriesElem->InsertEndChild( vassalCountryElem );
					SetXmlAttribute( vassalCountryElem, "Name", otherCountry->m_name.c_str() );
					SetXmlAttribute( vassalCountryElem, "ID", otherCountry->m_id );
				}
			}
			if (country->m_isCelestial) {
				SetXmlAttribute( countryElem, "CelestialEmpire", country->m_isCelestial );
			}
			if (country->m_relationCelestialEmpire) {
				SetXmlAttribute( countryElem, "CelestialEmpireSuzerain", country->m_relationCelestialEmpire->m_name );
			}
			if ((int)country->m_relationTributaries.size() > 0) {
				XmlElement* tributaryCountriesElem = save.NewElement( "Tributaries" );
				countryElem->InsertEndChild( tributaryCountriesElem );
				for (auto otherCountry : country->m_relationTributaries) {
					XmlElement* tributaryCountryElem = save.NewElement( "Tributary" );
					tributaryCountriesElem->InsertEndChild( tributaryCountryElem );
					SetXmlAttribute( tributaryCountryElem, "Name", otherCountry->m_name.c_str() );
					SetXmlAttribute( tributaryCountryElem, "ID", otherCountry->m_id );
				}
			}

			XmlElement* armiesElem = save.NewElement( "Armies" );
			countryElem->InsertEndChild( armiesElem );
			for (auto army : country->m_armies) {
				XmlElement* armyElem = save.NewElement( "Army" );
				armiesElem->InsertEndChild( armyElem );
				SetXmlAttribute( armyElem, "CombatValue", army->m_combatValue );
				SetXmlAttribute( armyElem, "ProvinceID", army->m_provIn->m_id );
				SetXmlAttribute( armyElem, "ProvinceName", army->m_provIn->m_name );
				SetXmlAttribute( armyElem, "Size", army->m_size );
			}
			/*
			// armies
			std::vector<Country*> m_tribeUnions;
			*/
		}
	}
	// rivers
	XmlElement* riverRootElem = save.NewElement( "Rivers" );
	rootElem->InsertEndChild( riverRootElem );
	for (auto river : m_rivers) {
		if (river->m_isGarbage) {
			continue;
		}
		XmlElement* riverElem = save.NewElement( "River" );
		riverRootElem->InsertEndChild( riverElem );
		std::string anchorPoints;
		for (auto& pos : river->m_anchorPoint) {
			anchorPoints += Stringf( "%f,%f ", pos.x, pos.y );
		}
		anchorPoints = anchorPoints.substr( 0, anchorPoints.size() - 1 );
		SetXmlAttribute( riverElem, "AnchorPoints", anchorPoints );
		SetXmlAttribute( riverElem, "HasDelta", river->m_hasDelta );
		SetXmlAttribute( riverElem, "Length", river->m_length );
		SetXmlAttribute( riverElem, "QuantityOfFlow", river->m_quantityOfFlow );
		SetXmlAttribute( riverElem, "Sandiness", river->m_sandiness );
	}

	// continents
	XmlElement* continentRootElem = save.NewElement( "Continents" );
	rootElem->InsertEndChild( continentRootElem );
	for (auto continent : m_continents) {
		if (continent) {
			XmlElement* continentElem = save.NewElement( "Continent" );
			continentRootElem->InsertEndChild( continentElem );
			SetXmlAttribute( continentElem, "Name", continent->m_name );
			SetXmlAttribute( continentElem, "Color", continent->m_color );
			SetXmlAttribute( continentElem, "ID", continent->m_id );
			SetXmlAttribute( continentElem, "IsEasternContinent", continent->m_eastContinent );
			SetXmlAttribute( continentElem, "IsGoldInDesert", continent->m_goldInDesertFlag );
			SetXmlAttribute( continentElem, "IsNewContinent", continent->m_newContinent );
			std::string ownedProvIDs;
			for (auto prov : continent->m_containedUnits) {
				ownedProvIDs += Stringf( "%d ", prov->m_id );
			}
			ownedProvIDs = ownedProvIDs.substr( 0, ownedProvIDs.size() - 1 );
			SetXmlAttribute( continentElem, "OwnedProvinces", ownedProvIDs );
		}
	}
	// regions
	XmlElement* regionRootElem = save.NewElement( "Regions" );
	rootElem->InsertEndChild( regionRootElem );
	for (auto region : m_regions) {
		XmlElement* regionElem = save.NewElement( "Region" );
		regionRootElem->InsertEndChild( regionElem );
		SetXmlAttribute( regionElem, "Name", region->m_name );
		SetXmlAttribute( regionElem, "Color", region->m_color );
		SetXmlAttribute( regionElem, "ID", region->m_id );
		std::string ownedProvIDs;
		for (auto prov : region->m_containedUnits) {
			ownedProvIDs += Stringf( "%d ", prov->m_id );
		}
		ownedProvIDs = ownedProvIDs.substr( 0, ownedProvIDs.size() - 1 );
		SetXmlAttribute( regionElem, "OwnedProvinces", ownedProvIDs );
	}


	// cultures
	XmlElement* cultureRootElem = save.NewElement( "Cultures" );
	rootElem->InsertEndChild( cultureRootElem );
	for (auto culture : m_cultures) {
		XmlElement* cultureElem = save.NewElement( "Culture" );
		cultureRootElem->InsertEndChild( cultureElem );
		SetXmlAttribute( cultureElem, "Name", culture->m_name );
		SetXmlAttribute( cultureElem, "Color", culture->m_color );
		SetXmlAttribute( cultureElem, "ID", culture->m_id );
		SetXmlAttribute( cultureElem, "OriginType", g_cultureOriginNameMap[(int)culture->m_origin] );
		SetXmlAttribute( cultureElem, "OriginProvinceName", culture->m_cultureOriginUnit->m_name );
		SetXmlAttribute( cultureElem, "OriginProvinceID", culture->m_cultureOriginUnit->m_id );
		SetXmlAttribute( cultureElem, "Traits", Stringf("%s %s %s %s", g_cultureTraitsName[(int)culture->m_traits[0]], 
			g_cultureTraitsName[(int)culture->m_traits[1]], g_cultureTraitsName[(int)culture->m_traits[2]], g_cultureTraitsName[(int)culture->m_traits[3]]));
	}

	// religions
	XmlElement* religionRootElem = save.NewElement( "Religions" );
	rootElem->InsertEndChild( religionRootElem );
	for (auto religion : m_religions) {
		XmlElement* religionElem = save.NewElement( "Religion" );
		religionRootElem->InsertEndChild( religionElem );
		SetXmlAttribute( religionElem, "Name", religion->m_name );
		SetXmlAttribute( religionElem, "Color", religion->m_color );
		SetXmlAttribute( religionElem, "ID", religion->m_id );
		SetXmlAttribute( religionElem, "OriginProvinceName", religion->m_religionOriginUnit->m_name );
		SetXmlAttribute( religionElem, "OriginProvinceID", religion->m_religionOriginUnit->m_id );
	}

	// roads
	XmlElement* roadRootElem = save.NewElement( "Roads" );
	rootElem->InsertEndChild( roadRootElem );
	for (auto road : m_roads) {
		XmlElement* roadElem = save.NewElement( "Road" );
		roadRootElem->InsertEndChild( roadElem );
		std::string anchorPoints;
		for (auto& pos : road->m_anchorPoints) {
			anchorPoints += Stringf( "%f,%f ", pos.first.x, pos.first.y );
		}
		anchorPoints = anchorPoints.substr( 0, anchorPoints.size() - 1 );
		SetXmlAttribute( roadElem, "AnchorPoints", anchorPoints );
	}

	std::filesystem::create_directory( Stringf( "Saves/%d", m_generationSettings.m_seed ).c_str() );
	XmlError errorCode = save.SaveFile( Stringf( "Saves/%d/World_Year%d_Month%d.xml", m_generationSettings.m_seed, m_viewingYear, m_viewingMonth ).c_str() );
	if (errorCode != tinyxml2::XML_SUCCESS) {
		ERROR_RECOVERABLE( "Cannot save settings" );
	}
}

void Map::SaveHistoryToXml()
{
	SaveHistoryJob* job = new SaveHistoryJob( m_historySavingModule );
	g_theJobSystem->AddJob( job );
	m_saveHistoryJob = job;
}

void Map::GetCultureListExclude( std::vector<Culture*>& out_cultures, Culture* exCulture ) const
{
	out_cultures.clear();
	if (m_cultures.size() >= 2) {
		out_cultures.reserve( m_cultures.size() - 1 );
		for (auto culture : m_cultures) {
			if (culture != exCulture) {
				out_cultures.push_back( culture );
			}
		}
	}
}

void Map::GetReligionListExclude( std::vector<Religion*>& out_religions, Religion* exReligion ) const
{
	out_religions.clear();
	if (m_religions.size() >= 2) {
		out_religions.reserve( m_religions.size() - 1 );
		for (auto religion : m_religions) {
			if (religion != exReligion) {
				out_religions.push_back( religion );
			}
		}
	}
}

void Map::RefreshAllLabels()
{
	for (auto label : m_cityLabels) {
		label->ReCalculateVertexData();
	}
	for (auto label : m_townLabels) {
		label->ReCalculateVertexData();
	}
	for (auto label : m_countryLabels) {
		label->ReCalculateVertexData();
	}
	for (auto label : m_cultureLabels) {
		label->ReCalculateVertexData();
	}
	for (auto label : m_religionLabels) {
		label->ReCalculateVertexData();
	}
}

/*void Map::LoadCurrentWorldFromXml()
{

}*/

void Map::PM_ClampLinesIntoBounds( Vec2& pos1, Vec2& pos2 )
{
	bool pos1Inside = m_bounds.IsPointInside( pos1 );
	bool pos2Inside = m_bounds.IsPointInside( pos2 );
	if ((pos1Inside && pos2Inside) || (!pos1Inside && !pos2Inside)) {
		return;
	}
	Vec2* insidePos = nullptr;
	Vec2* outsidePos = nullptr;
	if (pos1Inside) {
		insidePos = &pos1;
		outsidePos = &pos2;
	}
	else {
		insidePos = &pos2;
		outsidePos = &pos1;
	}
	// solve 4 bounding box edges
	Vec2 fwdVec = (*outsidePos - *insidePos).GetNormalized();

	float minX = m_bounds.m_mins.x;
	float tToMinX = (minX - insidePos->x) / fwdVec.x;
	if (tToMinX >= 0.f) {
		float yOfMinX = insidePos->y + fwdVec.y * tToMinX;
		if (yOfMinX >= m_bounds.m_mins.y && yOfMinX <= m_bounds.m_maxs.y) {
			outsidePos->x = minX;
			outsidePos->y = yOfMinX;
			return;
		}
	}

	float maxX = m_bounds.m_maxs.x;
	float tToMaxX = (maxX - insidePos->x) / fwdVec.x;
	if (tToMaxX >= 0.f) {
		float yOfMaxX = insidePos->y + fwdVec.y * tToMaxX;
		if (yOfMaxX >= m_bounds.m_mins.y && yOfMaxX <= m_bounds.m_maxs.y) {
			outsidePos->x = maxX;
			outsidePos->y = yOfMaxX;
			return;
		}
	}

	float minY = m_bounds.m_mins.y;
	float tToMinY = (minY - insidePos->y) / fwdVec.y;
	if (tToMinY >= 0.f) {
		float xOfMinY = insidePos->x + fwdVec.x * tToMinY;
		if (xOfMinY >= m_bounds.m_mins.x && xOfMinY <= m_bounds.m_maxs.x) {
			outsidePos->x = xOfMinY;
			outsidePos->y = minY;
			return;
		}
	}

	float maxY = m_bounds.m_maxs.y;
	float tToMaxY = (maxY - insidePos->y) / fwdVec.y;
	if (tToMaxY >= 0.f) {
		float xOfMaxY = insidePos->x + fwdVec.x * tToMaxY;
		if (xOfMaxY >= m_bounds.m_mins.x && xOfMaxY <= m_bounds.m_maxs.x) {
			outsidePos->x = xOfMaxY;
			outsidePos->y = maxY;
			return;
		}
	}
	return;
}

void Map::StartUpMap()
{
	m_historySavingModule = new HistorySavingSolver( this );
	m_mapRNG = new RandomNumberGenerator( m_generationSettings.m_seed );
	m_2D3DSwitchTimer = new Timer( 0.8f, Clock::GetSystemClock() );
	m_generationSettings.m_seed = m_mapRNG->GetSeed();
	m_historyRNG = new RandomNumberGenerator( m_generationSettings.m_seed - 1 );
	m_bounds = g_theGame->m_worldCamera.m_cameraBox;
	m_dimensions = m_generationSettings.m_dimensions;
	m_diagonalLength = m_dimensions.GetLength();
	m_bounds = AABB2( Vec2(), Vec2( m_dimensions.x, m_dimensions.y ) );
	g_theGame->m_cameraCenter = m_bounds.GetCenter();
	float cameraScale = Maxf( m_dimensions.x / WORLD_SIZE_X, m_dimensions.y / WORLD_SIZE_Y ) * 1.1f;

	g_theGame->m_worldCamera.SetRenderBasis( Vec3( 0.f, 0.f, 1.f ), Vec3( -1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ) );
	g_theGame->m_worldCamera.m_mode = CameraMode::Perspective;
	g_theGame->m_worldCamera.SetPerspectiveView( g_window->GetAspect(), 90.f, 0.1f, 50000.f );
	g_theGame->m_worldCamera.m_position = Vec3( m_dimensions.x * 0.5f, m_dimensions.y * 0.5f, 10.f * cameraScale );
	//g_theGame->m_worldCamera2D.m_orientation = EulerAngles( 90.f, 60.f, 0.f );
	g_theGame->m_worldCamera.m_orientation = EulerAngles( 90.f, 90.f, 0.f );
	//g_theGame->m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 1.f, -1.f );
	//g_theGame->m_worldCamera.Scale2D( cameraScale, cameraScale );
	//g_theGame->m_worldCamera.m_mode = CameraMode::Orthographic;
	m_polygonFaceShader = g_theRenderer->CreateShader( "Data/Shaders/BasicShader", VertexType::PCU_SEPARATED );
	m_2DTextureShader = g_theRenderer->CreateShader( "Data/Shaders/2DTextureShader", VertexType::PCU );
	m_3DShader = g_theRenderer->CreateShader( "Data/Shaders/3DModeShader", VertexType::PCUN_SEPARATED );
	m_sphereShader = g_theRenderer->CreateShader( "Data/Shaders/SphereModeShader", VertexType::PCU );
	m_2DPolygonTextureShader = g_theRenderer->CreateShader( "Data/Shaders/2DPolygonTextureShader", VertexType::PCU_SEPARATED );

	m_heightSeed = m_generationSettings.m_seed + 1;
	m_precipitationSeed = m_generationSettings.m_seed + 2;
	m_temperatureSeed = m_generationSettings.m_seed + 3;
	m_landformSeed = m_generationSettings.m_seed + 4;
	m_riverStartSeed = m_generationSettings.m_seed + 5;
	m_populationSeed = m_generationSettings.m_seed + 6;
	m_citySeed = m_generationSettings.m_seed + 7;
	m_productSeed = m_generationSettings.m_seed + 8;
	m_townSeed = m_generationSettings.m_seed + 9;
	m_cultureNameSeed = m_generationSettings.m_seed + 10;
	m_religionNameSeed = m_generationSettings.m_seed + 11;
	m_regionContinentNameGenerator = new ProvinceNameGenerator( m_generationSettings.m_seed + 12, "default" );

	m_generationSettings.m_numOfUnitsToHaveLake = m_generationSettings.m_basePolygons / 600;
	m_generationSettings.m_numOfUnitsToHaveIsland = m_generationSettings.m_basePolygons / 500;
	m_generationSettings.m_sqrtBasePolygons = RoundDownToInt( sqrtf( (float)m_generationSettings.m_basePolygons ) );

	m_productRNG = new RandomNumberGenerator( m_productSeed );

	m_productPrice = {
		/* food */
		/*Fish*/2.f, /*Grain*/2.f, /*Fruit*/2.5f, /*Sugar*/3.5f, /*Salt*/3.5f, /*Livestock*/2.5f,
		/* material */
		/*Wax*/2.5f, /*Fur*/3.5f, /*Wood*/2.f, /*Ivory*/5.f, /*Cotton*/3.f, /*Wool*/2.5f,
		/* metal */
		/*Iron*/4.f, /*Gold*/50.f, /*Copper*/6.f, /*Silver*/20.f,
		/* additives */
		/*Wine*/2.5f, /*Spice*/6.f, /*Tea*/5.f, /*Coffee*/4.5f, /*Tobacco*/5.f,
		/* goods */
		/*Glass*/4.5f, /*Silk*/6.f, /*Jade*/8.f, /*Porcelain*/5.f, /*Cloth*/3.f, /*Gem*/7.5f,
		/* Military */
		/*WarHorse*/6.f, /*Sword*/4.f,
		};

	ReadMapRenderingPreferences();

	if (m_generationSettings.m_enableHistorySimulation) {
		m_historyData.reserve( 1000 );
	}

	std::filesystem::create_directory( Stringf( "Saves" ) );
	std::filesystem::create_directory( Stringf( "Saves/%d", m_generationSettings.m_seed ) );
}

void Map::ReadMapRenderingPreferences()
{
	if (std::filesystem::exists( "Config/MapRenderingPreferences.xml" )) {
		XmlDocument prefSettingsDocument;
		prefSettingsDocument.LoadFile( "Config/MapRenderingPreferences.xml" );
		XmlElement* rootElem = prefSettingsDocument.RootElement();
		m_renderPreference.m_highestLandHeightColor = ParseXmlAttribute( *rootElem, "HighestLandHeightColor", m_renderPreference.m_highestLandHeightColor );
		m_renderPreference.m_lowestLandHeightColor = ParseXmlAttribute( *rootElem, "LowestLandHeightColor", m_renderPreference.m_lowestLandHeightColor );
		m_renderPreference.m_highestOceanHeightColor = ParseXmlAttribute( *rootElem, "HighestOceanHeightColor", m_renderPreference.m_highestOceanHeightColor );
		m_renderPreference.m_lowestOceanHeightColor = ParseXmlAttribute( *rootElem, "LowestOceanHeightColor", m_renderPreference.m_lowestOceanHeightColor );
		m_renderPreference.m_highestPopulationColor = ParseXmlAttribute( *rootElem, "HighestPopulationColor", m_renderPreference.m_highestPopulationColor );
		m_renderPreference.m_lowestPopulationColor = ParseXmlAttribute( *rootElem, "LowestPopulationColor", m_renderPreference.m_lowestPopulationColor );
		m_renderPreference.m_highestPrecipitationColor = ParseXmlAttribute( *rootElem, "HighestPrecipitationColor", m_renderPreference.m_highestPrecipitationColor );
		m_renderPreference.m_lowestPrecipitationColor = ParseXmlAttribute( *rootElem, "LowestPrecipitationColor", m_renderPreference.m_lowestPrecipitationColor );
		m_renderPreference.m_highestTemperatureColor = ParseXmlAttribute( *rootElem, "HighestTemperatureColor", m_renderPreference.m_highestTemperatureColor );
		m_renderPreference.m_lowestTemperatureColor = ParseXmlAttribute( *rootElem, "LowestTemperatureColor", m_renderPreference.m_lowestTemperatureColor );
		m_renderPreference.m_invalidProvince = ParseXmlAttribute( *rootElem, "ILegalProvinceColor", m_renderPreference.m_invalidProvince );
		m_renderPreference.m_oceanColor = ParseXmlAttribute( *rootElem, "OceanColor", m_renderPreference.m_oceanColor );

		XmlElement* productColorElem = rootElem->FirstChildElement( "ProductColor" );
		Rgba8 productColor;
		productColor = ParseXmlAttribute( *productColorElem, "Fish", Rgba8( 74, 78, 130 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // fish
		productColor = ParseXmlAttribute( *productColorElem, "Grain", Rgba8( 239, 227, 116 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // grain
		productColor = ParseXmlAttribute( *productColorElem, "Fruit", Rgba8( 128, 255, 0 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // fruit
		productColor = ParseXmlAttribute( *productColorElem, "Sugar", Rgba8( 193, 169, 64 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // sugar
		productColor = ParseXmlAttribute( *productColorElem, "Salt", Rgba8( 255, 255, 255 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // salt
		productColor = ParseXmlAttribute( *productColorElem, "Livestock", Rgba8( 255, 209, 220 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // livestock
		productColor = ParseXmlAttribute( *productColorElem, "Wax", Rgba8( 244, 232, 209 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // wax

		productColor = ParseXmlAttribute( *productColorElem, "Fur", Rgba8( 81, 64, 53 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // fur
		productColor = ParseXmlAttribute( *productColorElem, "Wood", Rgba8( 129, 113, 94 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // wood
		productColor = ParseXmlAttribute( *productColorElem, "Ivory", Rgba8( 225, 221, 210 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // Ivory
		productColor = ParseXmlAttribute( *productColorElem, "Cotton", Rgba8( 212, 108, 139 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // cotton
		productColor = ParseXmlAttribute( *productColorElem, "Wool", Rgba8( 138, 43, 226 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // wool
		productColor = ParseXmlAttribute( *productColorElem, "Iron", Rgba8( 98, 91, 87 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // iron
		productColor = ParseXmlAttribute( *productColorElem, "Gold", Rgba8( 255, 215, 0 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // gold
		productColor = ParseXmlAttribute( *productColorElem, "Copper", Rgba8( 184, 115, 51 ));
		m_renderPreference.m_productColorMap.push_back( productColor ); // copper
		productColor = ParseXmlAttribute( *productColorElem, "Silver", Rgba8( 192, 192, 192 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // silver

		productColor = ParseXmlAttribute( *productColorElem, "Wine", Rgba8( 100, 1, 37 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // wine
		productColor = ParseXmlAttribute( *productColorElem, "Spice", Rgba8( 175, 68, 103 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // spice
		productColor = ParseXmlAttribute( *productColorElem, "Tea", Rgba8( 158, 175, 160 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // tea
		productColor = ParseXmlAttribute( *productColorElem, "Coffee", Rgba8( 77, 57, 0 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // coffee
		productColor = ParseXmlAttribute( *productColorElem, "Tobacco", Rgba8( 108, 56, 33 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // tobacco
		productColor = ParseXmlAttribute( *productColorElem, "Glass", Rgba8( 51, 255, 255 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // glass
		productColor = ParseXmlAttribute( *productColorElem, "Silk", Rgba8( 107, 41, 127 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // silk
		productColor = ParseXmlAttribute( *productColorElem, "Jade", Rgba8( 46, 223, 163 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // jade
		productColor = ParseXmlAttribute( *productColorElem, "Porcelain", Rgba8( 11, 61, 146 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // porcelain

		productColor = ParseXmlAttribute( *productColorElem, "Cloth", Rgba8( 229, 82, 117 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // cloth
		productColor = ParseXmlAttribute( *productColorElem, "Gem", Rgba8( 0, 79, 109 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // gem
		productColor = ParseXmlAttribute( *productColorElem, "Warhorse", Rgba8( 139, 69, 19 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // warhorse
		productColor = ParseXmlAttribute( *productColorElem, "Sword", Rgba8( 0, 0, 0 ) );
		m_renderPreference.m_productColorMap.push_back( productColor ); // sword

		XmlElement* climateColorElem = rootElem->FirstChildElement( "ClimateColor" );
		Rgba8 climateColor;
		// populate climate color map
		climateColor = ParseXmlAttribute( *climateColorElem, "TropicalRainforest", Rgba8( 51, 102, 0 ) );
		m_renderPreference.m_climateColorMap.push_back( climateColor );
		climateColor = ParseXmlAttribute( *climateColorElem, "TropicalMonsoon", Rgba8( 255, 153, 153 ) );
		m_renderPreference.m_climateColorMap.push_back( climateColor );
		climateColor = ParseXmlAttribute( *climateColorElem, "TropicalSavanna", Rgba8( 102, 204, 0 ) );
		m_renderPreference.m_climateColorMap.push_back( climateColor );
		climateColor = ParseXmlAttribute( *climateColorElem, "HotDesert", Rgba8( 255, 255, 0 ) );
		m_renderPreference.m_climateColorMap.push_back( climateColor );
		climateColor = ParseXmlAttribute( *climateColorElem, "ColdDesert", Rgba8( 255, 255, 204 ) );
		m_renderPreference.m_climateColorMap.push_back( climateColor );
		climateColor = ParseXmlAttribute( *climateColorElem, "MediterraneanClimate", Rgba8( 204, 153, 255 ) );
		m_renderPreference.m_climateColorMap.push_back( climateColor );
		climateColor = ParseXmlAttribute( *climateColorElem, "HumidSubtropical", Rgba8( 255, 204, 153 ) );
		m_renderPreference.m_climateColorMap.push_back( climateColor );
		climateColor = ParseXmlAttribute( *climateColorElem, "Oceanic", Rgba8( 102, 178, 255 ) );
		m_renderPreference.m_climateColorMap.push_back( climateColor );
		climateColor = ParseXmlAttribute( *climateColorElem, "HumidContinentalMonsoon", Rgba8( 204, 255, 153 ) );
		m_renderPreference.m_climateColorMap.push_back( climateColor );
		climateColor = ParseXmlAttribute( *climateColorElem, "HumidContinental", Rgba8( 153, 255, 204 ) );
		m_renderPreference.m_climateColorMap.push_back( climateColor );
		climateColor = ParseXmlAttribute( *climateColorElem, "Subarctic", Rgba8( 0, 102, 204 ) );
		m_renderPreference.m_climateColorMap.push_back( climateColor );
		climateColor = ParseXmlAttribute( *climateColorElem, "Tundra", Rgba8( 153, 153, 0 ) );
		m_renderPreference.m_climateColorMap.push_back( climateColor );
		climateColor = ParseXmlAttribute( *climateColorElem, "IceCap", Rgba8( 224, 224, 224 ) );
		m_renderPreference.m_climateColorMap.push_back( climateColor );
		climateColor = ParseXmlAttribute( *climateColorElem, "Water", Rgba8( 0, 204, 204 ) );
		m_renderPreference.m_climateColorMap.push_back( climateColor );
		climateColor = ParseXmlAttribute( *climateColorElem, "None", Rgba8( 0, 0, 0 ) );
		m_renderPreference.m_climateColorMap.push_back( climateColor );


		XmlElement* relationColorElem = rootElem->FirstChildElement( "RelationshipColor" );
		Rgba8 relationColor;
		relationColor = ParseXmlAttribute( *relationColorElem, "Friendly", Rgba8( 128, 255, 0 ) );
		m_renderPreference.m_relationColorMap.push_back( relationColor ); // Friendly
		relationColor = ParseXmlAttribute( *relationColorElem, "Alliance", Rgba8( 0, 128, 255 ) );
		m_renderPreference.m_relationColorMap.push_back( relationColor ); // Alliance
		relationColor = ParseXmlAttribute( *relationColorElem, "Hostile", Rgba8( 255, 153, 153 ) );
		m_renderPreference.m_relationColorMap.push_back( relationColor ); // Hostile
		relationColor = ParseXmlAttribute( *relationColorElem, "War", Rgba8( 153, 0, 0 ) );
		m_renderPreference.m_relationColorMap.push_back( relationColor ); // War

		relationColor = ParseXmlAttribute( *relationColorElem, "Tributary", Rgba8( 255, 153, 255 ) );
		m_renderPreference.m_relationColorMap.push_back( relationColor ); // Tributary
		relationColor = ParseXmlAttribute( *relationColorElem, "Suzerain", Rgba8( 0, 204, 102 ) );
		m_renderPreference.m_relationColorMap.push_back( relationColor ); // Suzerain
		relationColor = ParseXmlAttribute( *relationColorElem, "Vassal", Rgba8( 153, 153, 255 ) );
		m_renderPreference.m_relationColorMap.push_back( relationColor ); // Vassal
		relationColor = ParseXmlAttribute( *relationColorElem, "Celestial", Rgba8( 255, 255, 51 ) );
		m_renderPreference.m_relationColorMap.push_back( relationColor ); // Celestial

		relationColor = ParseXmlAttribute( *relationColorElem, "TribeUnion", Rgba8( 153, 255, 204 ) );
		m_renderPreference.m_relationColorMap.push_back( relationColor ); // TribeUnion
		relationColor = ParseXmlAttribute( *relationColorElem, "None", Rgba8( 128, 128, 128 ) );
		m_renderPreference.m_relationColorMap.push_back( relationColor ); // None
		relationColor = ParseXmlAttribute( *relationColorElem, "Self", Rgba8( 224, 224, 224 ) );
		m_renderPreference.m_relationColorMap.push_back( relationColor ); // Self


		XmlElement* landformColorElem = rootElem->FirstChildElement( "LandformColor" );
		Rgba8 landformColor;
		landformColor = ParseXmlAttribute( *landformColorElem, "Ocean", Rgba8( 0, 204, 204 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Land", Rgba8( 0, 0, 0 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor ); // will not exist in the final terrain
		landformColor = ParseXmlAttribute( *landformColorElem, "Lake", Rgba8( 102, 255, 255 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Island", Rgba8( 0, 0, 204 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Lowland_Plain", Rgba8( 204, 255, 153 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Highland_Plain", Rgba8( 255, 51, 153 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Grassland", Rgba8( 51, 255, 51 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Savanna", Rgba8( 153, 76, 0 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Marsh", Rgba8( 51, 51, 0 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Rainforest", Rgba8( 0, 102, 0 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Temperate_Forest", Rgba8( 0, 204, 102 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Subarctic_Forest", Rgba8( 25, 51, 0 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Subtropical_Forest", Rgba8( 128, 255, 0 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Lowland_Hill", Rgba8( 255, 204, 153 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Highland_Hill", Rgba8( 255, 0, 255 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Mountain", Rgba8( 102, 0, 204 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Icefield", Rgba8( 204, 204, 204 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Tundra", Rgba8( 153, 153, 0 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Desert", Rgba8( 255, 255, 0 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
		landformColor = ParseXmlAttribute( *landformColorElem, "Town", Rgba8( 255, 0, 0 ) );
		m_renderPreference.m_landformColorMap.push_back( landformColor );
	}
	else {
		// product color
		m_renderPreference.m_productColorMap.push_back( Rgba8( 74, 78, 130 ) ); // fish
		m_renderPreference.m_productColorMap.push_back( Rgba8( 239, 227, 116 ) ); // grain
		m_renderPreference.m_productColorMap.push_back( Rgba8( 128, 255, 0 ) ); // fruit
		m_renderPreference.m_productColorMap.push_back( Rgba8( 193, 169, 64 ) ); // sugar
		m_renderPreference.m_productColorMap.push_back( Rgba8( 255, 255, 255 ) ); // salt
		m_renderPreference.m_productColorMap.push_back( Rgba8( 255, 209, 220 ) ); // livestock
		m_renderPreference.m_productColorMap.push_back( Rgba8( 244, 232, 209 ) ); // wax

		m_renderPreference.m_productColorMap.push_back( Rgba8( 81, 64, 53 ) ); // fur
		m_renderPreference.m_productColorMap.push_back( Rgba8( 129, 113, 94 ) ); // wood
		m_renderPreference.m_productColorMap.push_back( Rgba8( 225, 221, 210 ) ); // Ivory
		m_renderPreference.m_productColorMap.push_back( Rgba8( 212, 108, 139 ) ); // cotton
		m_renderPreference.m_productColorMap.push_back( Rgba8( 138, 43, 226 ) ); // wool
		m_renderPreference.m_productColorMap.push_back( Rgba8( 98, 91, 87 ) ); // iron
		m_renderPreference.m_productColorMap.push_back( Rgba8( 255, 215, 0 ) ); // gold
		m_renderPreference.m_productColorMap.push_back( Rgba8( 184, 115, 51 ) ); // copper
		m_renderPreference.m_productColorMap.push_back( Rgba8( 192, 192, 192 ) ); // silver

		m_renderPreference.m_productColorMap.push_back( Rgba8( 100, 1, 37 ) ); // wine
		m_renderPreference.m_productColorMap.push_back( Rgba8( 175, 68, 103 ) ); // spice
		m_renderPreference.m_productColorMap.push_back( Rgba8( 158, 175, 160 ) ); // tea
		m_renderPreference.m_productColorMap.push_back( Rgba8( 77, 57, 0 ) ); // coffee
		m_renderPreference.m_productColorMap.push_back( Rgba8( 108, 56, 33 ) ); // tobacco
		m_renderPreference.m_productColorMap.push_back( Rgba8( 51, 255, 255 ) ); // glass
		m_renderPreference.m_productColorMap.push_back( Rgba8( 107, 41, 127 ) ); // silk
		m_renderPreference.m_productColorMap.push_back( Rgba8( 46, 223, 163 ) ); // jade
		m_renderPreference.m_productColorMap.push_back( Rgba8( 11, 61, 146 ) ); // porcelain

		m_renderPreference.m_productColorMap.push_back( Rgba8( 229, 82, 117 ) ); // cloth
		m_renderPreference.m_productColorMap.push_back( Rgba8( 0, 79, 109 ) ); // gem
		m_renderPreference.m_productColorMap.push_back( Rgba8( 139, 69, 19 ) ); // warhorse
		m_renderPreference.m_productColorMap.push_back( Rgba8( 0, 0, 0 ) ); // sword

		// populate climate color map
		m_renderPreference.m_climateColorMap.push_back( Rgba8( 51, 102, 0 ) );
		m_renderPreference.m_climateColorMap.push_back( Rgba8( 255, 153, 153 ) );
		m_renderPreference.m_climateColorMap.push_back( Rgba8( 102, 204, 0 ) );
		m_renderPreference.m_climateColorMap.push_back( Rgba8( 255, 255, 0 ) );
		m_renderPreference.m_climateColorMap.push_back( Rgba8( 255, 255, 204 ) );
		m_renderPreference.m_climateColorMap.push_back( Rgba8( 204, 153, 255 ) );
		m_renderPreference.m_climateColorMap.push_back( Rgba8( 255, 204, 153 ) );
		m_renderPreference.m_climateColorMap.push_back( Rgba8( 102, 178, 255 ) );
		m_renderPreference.m_climateColorMap.push_back( Rgba8( 204, 255, 153 ) );
		m_renderPreference.m_climateColorMap.push_back( Rgba8( 153, 255, 204 ) );
		m_renderPreference.m_climateColorMap.push_back( Rgba8( 0, 102, 204 ) );
		m_renderPreference.m_climateColorMap.push_back( Rgba8( 153, 153, 0 ) );
		m_renderPreference.m_climateColorMap.push_back( Rgba8( 224, 224, 224 ) );
		m_renderPreference.m_climateColorMap.push_back( Rgba8( 0, 204, 204 ) );
		m_renderPreference.m_climateColorMap.push_back( Rgba8( 0, 0, 0 ) );

		// relationship color
		m_renderPreference.m_relationColorMap.push_back( Rgba8( 128, 255, 0 ) ); // Friendly
		m_renderPreference.m_relationColorMap.push_back( Rgba8( 0, 128, 255 ) ); // Alliance
		m_renderPreference.m_relationColorMap.push_back( Rgba8( 255, 153, 153 ) ); // Hostile
		m_renderPreference.m_relationColorMap.push_back( Rgba8( 153, 0, 0 ) ); // War

		m_renderPreference.m_relationColorMap.push_back( Rgba8( 255, 153, 255 ) ); // Tributary
		m_renderPreference.m_relationColorMap.push_back( Rgba8( 0, 204, 102 ) ); // Suzerain
		m_renderPreference.m_relationColorMap.push_back( Rgba8( 153, 153, 255 ) ); // Vassal
		m_renderPreference.m_relationColorMap.push_back( Rgba8( 255, 255, 51 ) ); // Celestial

		m_renderPreference.m_relationColorMap.push_back( Rgba8( 153, 255, 204 ) ); // TribeUnion
		m_renderPreference.m_relationColorMap.push_back( Rgba8( 128, 128, 128 ) ); // None
		m_renderPreference.m_relationColorMap.push_back( Rgba8( 224, 224, 224 ) ); // Self

		// populate landform color map
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 0, 204, 204 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 0, 0, 0 ) ); // will not exist in the final terrain
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 102, 255, 255 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 0, 0, 204 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 204, 255, 153 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 255, 51, 153 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 51, 255, 51 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 153, 76, 0 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 51, 51, 0 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 0, 102, 0 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 0, 204, 102 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 25, 51, 0 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 128, 255, 0 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 255, 204, 153 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 255, 0, 255 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 102, 0, 204 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 204, 204, 204 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 153, 153, 0 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 255, 255, 0 ) );
		m_renderPreference.m_landformColorMap.push_back( Rgba8( 255, 0, 0 ) );

	}
	
	for (int i = 0; i < (int)m_renderPreference.m_productColorMap.size(); i++) {
		m_renderPreference.m_productColorMapVec4.push_back( m_renderPreference.m_productColorMap[i].GetAsFloats() );
	}
	for (int i = 0; i < (int)m_renderPreference.m_relationColorMap.size(); i++) {
		m_renderPreference.m_relationColorMapVec4.push_back( m_renderPreference.m_relationColorMap[i].GetAsFloats() );
	}
	for (int i = 0; i < (int)m_renderPreference.m_climateColorMap.size(); i++) {
		m_renderPreference.m_climateColorMapVec4.push_back( m_renderPreference.m_climateColorMap[i].GetAsFloats() );
	}
	for (int i = 0; i < (int)m_renderPreference.m_landformColorMap.size(); i++) {
		m_renderPreference.m_landformColorMapVec4.push_back( m_renderPreference.m_landformColorMap[i].GetAsFloats() );
	}
}

void Map::PopulateMapWithPolygons( int numOfPolygons )
{
	std::vector<Vec2> randomPoints;
	double startTime = GetCurrentTimeSeconds();
	
	// first generate some random points and make them more average
	int numOfRestrictBox = m_generationSettings.m_sqrtBasePolygons;
	std::vector<AABB2> bounds;
	int counter = 0;
	Vec2 size = (m_bounds.m_maxs - m_bounds.m_mins) / (float)numOfRestrictBox;
	for (int i = 0; i < numOfRestrictBox; i++) {
		for (int j = 0; j < numOfRestrictBox; j++) {
			bounds.push_back( AABB2( m_bounds.m_mins + Vec2( i * size.x, j * size.y ), m_bounds.m_mins + Vec2( (i + 1) * size.x, (j + 1) * size.y ) ) );
		}
	}
	while (1) {
		for (int i = 0; i < numOfRestrictBox * numOfRestrictBox; i++) {
			randomPoints.push_back( MP_GetRandomPointInAABB2D( bounds[i] ) );
			counter++;
			if (counter >= numOfPolygons) {
				goto ExitLoop;
			}
		}
	}
ExitLoop:
	
	FortuneAlgorithmSolverClass fortuneSolver;
	std::vector<FortuneHalfEdge*> outEdges;
	//for (int i = 0; i < numOfPolygons; i++) {
	//	randomPoints.push_back( MP_GetRandomPointInAABB2D( m_bounds ) );
	//}

	// push back 4 far away points 
	randomPoints.push_back( Vec2( -EDGE_GUARD_X, (m_bounds.m_mins.y + m_bounds.m_maxs.y) * 0.5f ) );
	randomPoints.push_back( Vec2( EDGE_GUARD_X, (m_bounds.m_mins.y + m_bounds.m_maxs.y) * 0.5f ) );
	randomPoints.push_back( Vec2( (m_bounds.m_mins.x + m_bounds.m_maxs.x) * 0.5f, -EDGE_GUARD_Y ) );
	randomPoints.push_back( Vec2( (m_bounds.m_mins.x + m_bounds.m_maxs.x) * 0.5f, EDGE_GUARD_Y ) );

	double endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Choosing Random Points finished, time: %.3fs", endTime - startTime ) );

	// solve Voronoi diagram

	startTime = GetCurrentTimeSeconds();
	fortuneSolver.FortuneAlgorithmSolver( randomPoints, outEdges );

	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Generating Voronoi Diagram finished, time: %.3fs", endTime - startTime ) );

	startTime = GetCurrentTimeSeconds();
	// create star edge map
	std::map<Vec2, MapPolygonUnit*> polygonUnitsMap;
	int index = 0;
	for (auto& edge : outEdges) {
		Vec2 sitePos = FortuneVec2dToVec2f( edge->m_sitePos );
		auto iter = polygonUnitsMap.find( sitePos );
		// find a site position already in map: insert that edge to that position
		if (iter != polygonUnitsMap.end()) {
			PM_AddEdgeToPolygonUnit( iter->second, edge );
		}
		// do not find a site position: create a new one and push it back in the vector
		else if(sitePos.x != EDGE_GUARD_X && sitePos.x != -EDGE_GUARD_X && sitePos.y != EDGE_GUARD_Y && sitePos.y != -EDGE_GUARD_Y) {
			MapPolygonUnit* unit = new MapPolygonUnit( sitePos );
			m_mapPolygonUnits.emplace_back( unit );
			unit->m_id = index;
			++index;
			polygonUnitsMap[sitePos] = unit;
			PM_AddEdgeToPolygonUnit( unit, edge );
		}
		else {
			MapPolygonUnit* unit = new MapPolygonUnit( sitePos );
			m_mapPolygonUnits.emplace_back( unit );
			unit->m_id = index;
			++index;
			polygonUnitsMap[sitePos] = unit;
			unit->m_isFarAwayFakeUnit = true;
			PM_AddEdgeToPolygonUnit( unit, edge );
		}
	}

	// delete the output edges
	for (auto edges : outEdges) {
		delete edges;
	}

	// sort the edges (to form a circular order)
	for (auto unit : m_mapPolygonUnits) {
		if (!unit->m_isFarAwayFakeUnit && (int)unit->m_edges.size() > 0) {
			std::vector<StarEdge*> orderedStarEdge;
			StarEdge* edge = unit->m_edges[0];
			StarEdge* curEdge = edge->m_next;
			orderedStarEdge.push_back( edge );
			while (edge != curEdge && curEdge) {
				orderedStarEdge.push_back( curEdge );
				curEdge = curEdge->m_next;
			}
			unit->m_edges = orderedStarEdge;
		}
	}

	// rework all edges that are out of bounds
	for (auto unit : m_mapPolygonUnits) {
		if (!unit->m_isFarAwayFakeUnit) {
			for (auto edge : unit->m_edges) {
				if (!m_bounds.IsPointInside( edge->m_startPos ) && !m_bounds.IsPointInside( edge->m_endPos )) {
				}
				else if (!m_bounds.IsPointInside( edge->m_startPos )) {
					PM_ClampLinesIntoBounds( edge->m_startPos, edge->m_endPos );
					//edge->m_prev->m_endPos = edge->m_startPos;
				}
				else if (!m_bounds.IsPointInside( edge->m_endPos )) {
					PM_ClampLinesIntoBounds( edge->m_startPos, edge->m_endPos );
					//edge->m_next->m_startPos = edge->m_endPos;
				}
			}
			for (int i = 0; i < (int)unit->m_edges.size(); ++i) {
				StarEdge* edge = unit->m_edges[i];

				if (!m_bounds.IsPointInsideOrOn( edge->m_startPos ) && !m_bounds.IsPointInsideOrOn( edge->m_endPos ) &&
					m_bounds.IsPointInsideOrOn( edge->m_prev->m_startPos ) && m_bounds.IsPointInsideOrOn( edge->m_prev->m_endPos )) {
					StarEdge* beginEdge = edge->m_prev;
					StarEdge* iter = edge->m_next;
					std::vector<StarEdge*> starEdgesNeedToMove;
					starEdgesNeedToMove.push_back( edge );
					while (!m_bounds.IsPointInsideOrOn( iter->m_startPos ) && !m_bounds.IsPointInsideOrOn( iter->m_endPos )) {
						starEdgesNeedToMove.push_back( iter );
						iter = iter->m_next;
					}
					int beginPos = 0;
					for (int j = 0; j < (int)unit->m_edges.size(); ++j) {
						if (unit->m_edges[j] == beginEdge) {
							beginPos = j;
						}
						for (int k = 0; k < (int)starEdgesNeedToMove.size(); ++k) {
							if (starEdgesNeedToMove[k] == unit->m_edges[j]) {
								unit->m_edges.erase( unit->m_edges.begin() + j );
								--j;
								break;
							}
						}
					}
					StarEdge* newEdge = new StarEdge();
					newEdge->m_next = iter;
					iter->m_prev = newEdge;
					newEdge->m_owner = iter->m_owner;
					newEdge->m_prev = beginEdge;
					beginEdge->m_next = newEdge;
					newEdge->m_startPos = newEdge->m_prev->m_endPos;
					newEdge->m_endPos = newEdge->m_next->m_startPos;
					unit->m_edges.insert( unit->m_edges.begin() + beginPos + 1, newEdge );
					break;
				}
			}
			// add edges to make sure all edges are connected together
			for (int i = 0;  i < (int)unit->m_edges.size(); ++i) {
				StarEdge* edge = unit->m_edges[i];
				if (edge->m_endPos != edge->m_next->m_startPos) {
					StarEdge* newEdge = new StarEdge();
					newEdge->m_next = edge->m_next;
					edge->m_next->m_prev = newEdge;
					newEdge->m_owner = edge->m_owner;
					newEdge->m_prev = edge;
					edge->m_next = newEdge;
					newEdge->m_startPos = newEdge->m_prev->m_endPos;
					newEdge->m_endPos = newEdge->m_next->m_startPos;
					unit->m_edges.insert( unit->m_edges.begin() + i + 1, newEdge );
				}
			}
			// consider corners
			for (int i = 0; i < (int)unit->m_edges.size(); ++i) {
				StarEdge* edge = unit->m_edges[i];
				// right bottom
				if (edge->m_startPos.y == 0.f && edge->m_endPos.x == m_dimensions.x) {
					edge->m_endPos = Vec2( m_dimensions.x, 0.f );
					StarEdge* newEdge = new StarEdge();
					newEdge->m_next = edge->m_next;
					edge->m_next->m_prev = newEdge;
					newEdge->m_owner = edge->m_owner;
					newEdge->m_prev = edge;
					edge->m_next = newEdge;
					newEdge->m_startPos = newEdge->m_prev->m_endPos;
					newEdge->m_endPos = newEdge->m_next->m_startPos;
					unit->m_edges.insert( unit->m_edges.begin() + i + 1, newEdge );
					break;
				}
				// left bottom
				if (edge->m_startPos.x == 0.f && edge->m_endPos.y == 0.f) {
					edge->m_endPos = Vec2( 0.f, 0.f );
					StarEdge* newEdge = new StarEdge();
					newEdge->m_next = edge->m_next;
					edge->m_next->m_prev = newEdge;
					newEdge->m_owner = edge->m_owner;
					newEdge->m_prev = edge;
					edge->m_next = newEdge;
					newEdge->m_startPos = newEdge->m_prev->m_endPos;
					newEdge->m_endPos = newEdge->m_next->m_startPos;
					unit->m_edges.insert( unit->m_edges.begin() + i + 1, newEdge );
					break;
				}
				// left top
				if (edge->m_startPos.y == m_dimensions.y && edge->m_endPos.x == 0.f) {
					edge->m_endPos = Vec2( 0.f, m_dimensions.y );
					StarEdge* newEdge = new StarEdge();
					newEdge->m_next = edge->m_next;
					edge->m_next->m_prev = newEdge;
					newEdge->m_owner = edge->m_owner;
					newEdge->m_prev = edge;
					edge->m_next = newEdge;
					newEdge->m_startPos = newEdge->m_prev->m_endPos;
					newEdge->m_endPos = newEdge->m_next->m_startPos;
					unit->m_edges.insert( unit->m_edges.begin() + i + 1, newEdge );
					break;
				}
				// right top
				if (edge->m_startPos.x == m_dimensions.x && edge->m_endPos.y == m_dimensions.y) {
					edge->m_endPos = Vec2( m_dimensions.x, m_dimensions.y );
					StarEdge* newEdge = new StarEdge();
					newEdge->m_next = edge->m_next;
					edge->m_next->m_prev = newEdge;
					newEdge->m_owner = edge->m_owner;
					newEdge->m_prev = edge;
					edge->m_next = newEdge;
					newEdge->m_startPos = newEdge->m_prev->m_endPos;
					newEdge->m_endPos = newEdge->m_next->m_startPos;
					unit->m_edges.insert( unit->m_edges.begin() + i + 1, newEdge );
					break;
				}
			}
		}
	}

	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Post Generation Works finished, time: %.3fs", endTime - startTime ) );
}

void Map::CalculateBiomeDataForPolygons()
{
	double startTime = GetCurrentTimeSeconds();
	for (auto unit : m_mapPolygonUnits) {
		unit->InitializeUnit();
	}
	double endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Height Map Generation finished, time: %.3fs", endTime - startTime ) );

	startTime = GetCurrentTimeSeconds();
	std::sort( m_mapPolygonUnits.begin(), m_mapPolygonUnits.end(), []( MapPolygonUnit* a, MapPolygonUnit* b ) 
		{ return a->m_centerPosition.y < b->m_centerPosition.y; } );
	m_oceanDirtySet.clear();
	m_islandDirtySet.clear();
	for (auto unit : m_mapPolygonUnits) {
		unit->InitializeUnit2();
	}
	m_oceanDirtySet.clear();
	m_islandDirtySet.clear();
	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Sea/Land/Island/Lake Generation finished, time: %.3fs", endTime - startTime ) );
	// todo? integrate oceans

	startTime = GetCurrentTimeSeconds();
	for (auto unit : m_mapPolygonUnits) {
		PolyUnitDataCalculationJob* job = new PolyUnitDataCalculationJob( unit );
		m_polyDataCalculationJobs.push_back( job );
		g_theJobSystem->AddJob( job );
	}

	while (!m_polyDataCalculationJobs.empty()) {
		if (g_theJobSystem->RetrieveJob( m_polyDataCalculationJobs.front() )) {
			delete m_polyDataCalculationJobs.front();
			m_polyDataCalculationJobs.pop_front();
		}
	}

	for (auto unit : m_mapPolygonUnits) {
		if (!unit->m_isFarAwayFakeUnit) {
			for (auto edge : unit->m_edges) {
				constexpr float factor = HEIGHT_FACTOR;
				float prevHeight = m_generationSettings.m_minHeight;
				float oppoHeight = m_generationSettings.m_minHeight;
				if (edge->m_prev->m_opposite) {
					prevHeight = edge->m_prev->m_opposite->m_owner->m_height;
				}
				if (edge->m_opposite) {
					oppoHeight = edge->m_opposite->m_owner->m_height;
				}
				edge->m_startHeight = (unit->m_height + oppoHeight + prevHeight) / 3.f * factor;
			}
		}
	}

	m_aStarHelper.AStarHelperInit( m_mapPolygonUnits );

	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Climate/Temperature/Precipitation/Landform Generation finished, time: %.3fs", endTime - startTime ) );
}

void Map::GenerateRivers()
{
	// for each polygon calculate the adjacent polygons lower to it
	for (auto unit : m_mapPolygonUnits) {
		unit->CalculateLowerAdjacentUnits();
	}

	// randomize river start position
	for (auto unit : m_mapPolygonUnits) {
		float chanceToAdd = 0.f;
		if (unit->m_height > 6000.f) {
			chanceToAdd = 0.2f;
		}
		else if (unit->m_height > 4000.f) {
			chanceToAdd = 0.15f;
		}
		else if (unit->m_height > 3000.f) {
			chanceToAdd = 0.1f;
		}
		else if (unit->m_height > 1000.f) {
			chanceToAdd = 0.05f;
		}
		else if (unit->m_height < 200.f) {
			chanceToAdd = -0.3f;
		}
		if (unit->m_summerPrecipitation > 1200.f) {
			chanceToAdd += 0.1f;
		}
		else if (unit->m_summerPrecipitation < 100.f) {
			chanceToAdd -= 0.2f;
		}
		if (unit->m_winterPrecipitation > 1200.f) {
			chanceToAdd += 0.1f;
		}
		else if (unit->m_summerPrecipitation < 100.f) {
			chanceToAdd -= 0.2f;
		}
		float chance = 0.5f + 0.5f * Compute2dPerlinNoise( unit->m_centerPosition.x, unit->m_centerPosition.y, 1.f, 5, 0.5f, 2.f, true, m_riverStartSeed );
		chance += chanceToAdd;
		if (chance > 0.8f) {
			River* newRiver = new River();
			newRiver->m_routeUnits.push_back( unit );
			newRiver->m_anchorPoint.push_back( unit->GetRandomPointNearCenter() );
			m_rivers.push_back( newRiver );
		}
	}

	// calculate river route
	for (auto river : m_rivers) {
		river->CalculateRiverRoute();
	}

	// delete garbage river (merged to other rivers)
	for (int i = 0; i < (int)m_rivers.size();) {
		if (m_rivers[i]->m_isGarbage) {
			delete m_rivers[i];
			m_rivers.erase( m_rivers.begin() + i );
		}
		else {
			++i;
		}
	}

	for (auto river : m_rivers) {
		river->CalculateRiverStats();
	}

}

void Map::GenerateForests()
{
	/* IMPORTANT!
	forests are represented by landform now, still considering if I need to put separate forest props into the world
	
	*/
	// like rivers, find start position then expand
	// randomize forest start position
	/*for (auto unit : m_mapPolygonUnits) {
		float chanceToAdd = 0.f;
		if (unit->m_height > 3000.f) {
			chanceToAdd -= 0.2f;
		}
		else if (unit->m_height > 2000.f) {
			chanceToAdd += 0.2f;
		}

		if (unit->m_landform == LandformType::Rainforest) {
			chanceToAdd += 0.2f;
		}
		else if (unit->m_landform == LandformType::Subarctic_Forest) {
			chanceToAdd += 0.2f;
		}
		else if (unit->m_landform == LandformType::Subtropical_Forest) {
			chanceToAdd += 0.1f;
		}
		else if (unit->m_landform == LandformType::Temperate_Forest) {
			chanceToAdd += 0.1f;
		}

		float chance = 0.5f + 0.5f * Compute2dPerlinNoise( unit->m_centerPosition.x, unit->m_centerPosition.y, 1.f, 5, 0.5f, 2.f, true, m_forestStartSeed );
		chance += chanceToAdd;
		if (chance > 0.8f) {
			Forest* newForest = new Forest();
			m_forests.push_back( newForest );
		}
	}*/
}

void Map::GenerateContinents()
{
	// now just simply add provinces that are together (not separated by the sea) into one continent
	// so use bfs
	std::vector<MapPolygonUnit*> allNotInContinentProvs = m_mapPolygonUnits;
	std::vector<Continent*> islands;
	while (!allNotInContinentProvs.empty()) {
		// find first not in continent province
		if (allNotInContinentProvs[allNotInContinentProvs.size() - 1]->m_isFarAwayFakeUnit) {
			allNotInContinentProvs.pop_back(); // do not need far away units
		}
		else if (allNotInContinentProvs[allNotInContinentProvs.size() - 1]->IsWater()) {
			allNotInContinentProvs.pop_back(); // do not need water units
		}
		else { // do one bfs for that province and add all provinces on land to the continent
			std::deque<MapPolygonUnit*> unitToBeSolveQueue;
			unitToBeSolveQueue.push_back( allNotInContinentProvs[allNotInContinentProvs.size() - 1] );
			//allNotInContinentProvs.pop_back();
			Continent* newContinent = new Continent();
			float rnd = m_mapRNG->RollRandomFloatZeroToOne();
			if (rnd < 0.6f) {
				newContinent->m_goldInDesertFlag = true;
			}
			rnd = m_mapRNG->RollRandomFloatZeroToOne();
			if (rnd < 0.4f) {
				newContinent->m_newContinent = true;
			}
			rnd = m_mapRNG->RollRandomFloatZeroToOne();
			if (rnd < 0.5f) {
				newContinent->m_eastContinent = true;
			}
			if (allNotInContinentProvs[allNotInContinentProvs.size() - 1]->m_landform == LandformType::Island) {
				islands.push_back( newContinent );
			}
			else {
				newContinent->m_id = (int)m_continents.size();
				m_continents.push_back( newContinent );
			}
			while (!unitToBeSolveQueue.empty()) {
				MapPolygonUnit* unit = unitToBeSolveQueue.front();
				unitToBeSolveQueue.pop_front();
				newContinent->m_containedUnits.push_back( unit );
				unit->m_continent = newContinent;
				auto findRes = std::find( allNotInContinentProvs.begin(), allNotInContinentProvs.end(), unit );
				if (findRes != allNotInContinentProvs.end()) {
					allNotInContinentProvs.erase( findRes );
					for (auto adjUnit : unit->m_adjacentUnits) {
						if (!adjUnit->IsWater() && adjUnit->m_continent == nullptr) {
							unitToBeSolveQueue.push_back( adjUnit );
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < (int)islands.size(); ++i) {
		float minAvgDistance = FLT_MAX;
		Continent* nearestContinent = nullptr;
		for (auto continent : m_continents) {
			float totalNearestDist = 0.f;
			for (auto unit : islands[i]->m_containedUnits) {
				float nearestDistance = FLT_MAX;
				for (auto continentUnit : continent->m_containedUnits) {
					float thisDistance = GetDistance2D( unit->m_geoCenterPos, continentUnit->m_geoCenterPos );
					if (thisDistance < nearestDistance) {
						nearestDistance = thisDistance;
					}
				}
				totalNearestDist += nearestDistance;
			}
			totalNearestDist /= (float)islands[i]->m_containedUnits.size();
			if (totalNearestDist < minAvgDistance) {
				minAvgDistance = totalNearestDist;
				nearestContinent = continent;
			}
		}
		if (nearestContinent) {
			for (auto unit : islands[i]->m_containedUnits) {
				unit->m_continent = nearestContinent;
				nearestContinent->m_containedUnits.push_back( unit );
			}
			delete islands[i];
			islands[i] = nullptr;
		}
	}

	for (auto island : islands) {
		m_continents.push_back( island );
	}

	for (auto continent : m_continents) {
		if (continent) {
			m_continentLabels.push_back( new Label( continent, LabelType::Continent ) );
		}
	}
}

void Map::GenerateHumanDataForPolygons()
{
	double startTime = GetCurrentTimeSeconds();
	// calculate population
	for (auto unit : m_mapPolygonUnits) {
		unit->CalculateRawPopulation();
	}
	double endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Population Generation finished, time: %.3fs", endTime - startTime ) );

	startTime = GetCurrentTimeSeconds();
	GenerateCultures();
	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Cultures Generation finished, time: %.3fs", endTime - startTime ) );

	startTime = GetCurrentTimeSeconds();
	GenerateReligions();
	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Religions Generation finished, time: %.3fs", endTime - startTime ) );

	startTime = GetCurrentTimeSeconds();
	GenerateCitiesAndTowns();
	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Cities and Towns Generation finished, time: %.3fs", endTime - startTime ) );

	startTime = GetCurrentTimeSeconds();
	GenerateProvinceProducts();
	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Province Products Generation finished, time: %.3fs", endTime - startTime ) );

	startTime = GetCurrentTimeSeconds();
	GenerateCountriesAndRelations();
	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Countries Generation finished, time: %.3fs", endTime - startTime ) );

	startTime = GetCurrentTimeSeconds();
	GenerateRegions();
	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Regions Generation finished, time: %.3fs", endTime - startTime ) );

	startTime = GetCurrentTimeSeconds();
	GenerateRoads();
	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Roads Generation finished, time: %.3fs", endTime - startTime ) );

	startTime = GetCurrentTimeSeconds();
	GenerateArmies();
	endTime = GetCurrentTimeSeconds();
	PCGWorld_Log( Stringf( "Armies Generation finished, time: %.3fs", endTime - startTime ) );
}

void Map::GenerateCultures()
{
	m_cultureNameGenerator = new CultureNameGenerator( m_cultureNameSeed );
	// create cultures
	// randomly choose original position of the cultures
	float minDistBetweenCultures = Maxf( m_dimensions.x, m_dimensions.y ) / (float)m_generationSettings.m_numOfCultures;
	float minDistSquared = minDistBetweenCultures * minDistBetweenCultures;
	for (int i = 0; i < m_generationSettings.m_numOfCultures; i++) {
		int tryTime = 0;
		while (true) {
			// roll a random position
			Vec2 rndPos = MP_GetRandomPointInAABB2D( m_bounds );
			// check if this position fits the condition
			MapPolygonUnit* unit = GetUnitByPosFast( rndPos );
			if (unit && !unit->IsOcean()) {
				bool isNearOtherCulture = false;
				// check if the culture is too near other existed cultures
				if (tryTime <= 10000) {
					for (auto culture : m_cultures) {
						if (GetDistanceSquared2D( culture->m_cultureOriginUnit->m_geoCenterPos, unit->m_geoCenterPos ) < minDistSquared) {
							isNearOtherCulture = true;
							break;
						}
					}
				}
				if (isNearOtherCulture) {
					continue;
				}
				// near big river
				if (unit->m_riverOnThis && unit->m_riverOnThis->m_quantityOfFlow > 8000.f && unit->m_landform == LandformType::Lowland_Plain) {
					Culture* newCulture = new Culture( (int)m_cultures.size() );
					newCulture->m_cultureOriginUnit = unit;
					newCulture->m_origin = CultureGeoOrigin::RiverOrigin;
					m_cultures.push_back( newCulture );
					break;
				}
				// near the ocean
				else if (unit->m_isCoast && m_mapRNG->RollRandomFloatZeroToOne() < 0.5f) {
					Culture* newCulture = new Culture( (int)m_cultures.size() );
					newCulture->m_cultureOriginUnit = unit;
					newCulture->m_origin = CultureGeoOrigin::OceanOrigin;
					m_cultures.push_back( newCulture );
					break;
				}
				// in the grasslands
				else if (unit->m_landform == LandformType::Grassland) {
					Culture* newCulture = new Culture( (int)m_cultures.size() );
					newCulture->m_cultureOriginUnit = unit;
					newCulture->m_origin = CultureGeoOrigin::GrasslandOrigin;
					m_cultures.push_back( newCulture );
					break;
				}
				// in the forests
				else if (unit->m_landform == LandformType::Temperate_Forest || unit->m_landform == LandformType::Subtropical_Forest
					|| unit->m_landform == LandformType::Subarctic_Forest || unit->m_landform == LandformType::Rainforest) {
					Culture* newCulture = new Culture( (int)m_cultures.size() );
					newCulture->m_cultureOriginUnit = unit;
					newCulture->m_origin = CultureGeoOrigin::ForestOrigin;
					m_cultures.push_back( newCulture );
					break;
				}
				// in high mountains
				else if (unit->m_height > 3000.f) {
					Culture* newCulture = new Culture( (int)m_cultures.size() );
					newCulture->m_cultureOriginUnit = unit;
					newCulture->m_origin = CultureGeoOrigin::MountainOrigin;
					m_cultures.push_back( newCulture );
					break;
				}
			}
			++tryTime;
		}
	}

	// perform culture expansion
	for (int i = 0; i < (int)m_cultures.size(); i++) {
		m_cultures[i]->m_cultureOriginUnit->HD_AddCultureInfluence( m_cultures[i] );
		m_cultures[i]->m_id = i;
	}

	// spread
	int step = 0;
	int countOfUnit = (int)m_cultures.size();
	while (countOfUnit < m_generationSettings.m_basePolygons) {
		for (auto unit : m_mapPolygonUnits) {
			if (!unit->m_isFarAwayFakeUnit && unit->m_cultureStepValue == -1) {
				//bool uninitializedUnit = unit->m_majorCulture ? false : true;
				bool addInfluence = false;

				for (auto adjUnit : unit->m_adjacentUnits) {
					if (!adjUnit->m_isFarAwayFakeUnit && adjUnit->m_majorCulture && adjUnit->m_cultureStepValue + 1 == step) {
						//if (unit->HD_GetCultureInfluence( adjUnit->m_majorCulture ) == 0.f) {
						unit->HD_AddCultureInfluence( adjUnit->m_majorCulture );
						unit->RecalculateMajorCulture();
						addInfluence = true;
						//}
						unit->m_cultureStepValue = step;
					}
				}
				if (addInfluence) {
					++countOfUnit;
				}
			}
		}
		++step;
	}

	// final spread
	for (int i = 0; i < 20; i++) {
		for (auto unit : m_mapPolygonUnits) {
			for (auto culture : m_cultures) {
				unit->HD_AddCultureInfluence( culture );
			}
		}
		for (auto unit : m_mapPolygonUnits) {
			for (int j = (int)m_cultures.size() - 1; j >= 0; j--) {
				unit->HD_AddCultureInfluence( m_cultures[j] );
			}
		}
	}
	for (auto unit : m_mapPolygonUnits) {
		unit->RecalculateMajorCulture();
	}

	for (auto culture : m_cultures) {
		m_cultureLabels.push_back( new Label( culture, LabelType::Culture ) );
	}

	for (auto prov : m_mapPolygonUnits) {
		if (!prov->m_isFarAwayFakeUnit && !prov->IsWater()) {
			prov->m_name = prov->m_majorCulture->m_provinceNameGenerator->GenerateProvinceName( prov );
		}
		else if (!prov->m_isFarAwayFakeUnit && prov->m_landform == LandformType::Lake) {
			prov->m_name = "Lake";
		}
		else if (!prov->m_isFarAwayFakeUnit && prov->m_landform == LandformType::Ocean) {
			prov->m_name = "Ocean";
		}
	}
}

void Map::GenerateReligions()
{
	m_religionNameGenerator = new ReligionNameGenerator( m_religionNameSeed );
	// create religions
	// randomly choose original position of the religions
	float minDistBetweenReligions = Maxf( m_dimensions.x, m_dimensions.y ) / (float)m_generationSettings.m_numOfReligions;
	float minReligionDistSquared = minDistBetweenReligions * minDistBetweenReligions;
	for (int i = 0; i < m_generationSettings.m_numOfReligions; i++) {
		int tryTime = 0;
		while (true) {
			// roll a random position
			Vec2 rndPos = MP_GetRandomPointInAABB2D( m_bounds );
			// check if this position fits the condition
			MapPolygonUnit* unit = GetUnitByPosFast( rndPos );
			if (unit && !unit->IsOcean()) {
				bool isNearOtherReligion = false;
				// check if the religion is too near other existed religions
				if (tryTime <= 10000) {
					for (auto religion : m_religions) {
						if (GetDistanceSquared2D( religion->m_religionOriginUnit->m_geoCenterPos, unit->m_geoCenterPos ) < minReligionDistSquared) {
							isNearOtherReligion = true;
							break;
						}
					}
				}
				if (isNearOtherReligion) {
					continue;
				}
				//if (unit->m_totalPopulation > 20000.f) {
				Religion* religion = new Religion( (int)m_religions.size() );
				religion->m_religionOriginUnit = unit;
				m_religions.push_back( religion );
				break;
				//}
			}
			++tryTime;
		}
	}

	// add religion start unit
	for (int i = 0; i < (int)m_religions.size(); i++) {
		m_religions[i]->m_religionOriginUnit->HD_AddReligionInfluence( m_religions[i] );
		m_religions[i]->m_id = i;
	}

	// spread
	for (auto unit : m_mapPolygonUnits) {
		unit->m_cultureStepValue = -1;
	}
	int step = 0;
	int countOfUnit = (int)m_religions.size();
	while (countOfUnit < m_generationSettings.m_basePolygons) {
		for (auto unit : m_mapPolygonUnits) {
			if (!unit->m_isFarAwayFakeUnit && unit->m_cultureStepValue == -1) {
				//bool uninitializedUnit = unit->m_majorReligion ? false : true;
				bool addInfluence = false;

				for (auto adjUnit : unit->m_adjacentUnits) {
					if (!adjUnit->m_isFarAwayFakeUnit && adjUnit->m_majorReligion && adjUnit->m_cultureStepValue + 1 == step) {
						unit->HD_AddReligionInfluence( adjUnit->m_majorReligion );
						unit->RecalculateMajorReligion();
						addInfluence = true;
						unit->m_cultureStepValue = step;
					}
				}
				if (addInfluence) {
					++countOfUnit;
				}
			}
		}
		++step;
	}

	// final spread
	for (int i = 0; i < 20; i++) {
		for (auto unit : m_mapPolygonUnits) {
			for (auto religion : m_religions) {
				unit->HD_AddReligionInfluence( religion );
			}
		}
		for (auto unit : m_mapPolygonUnits) {
			for (int j = (int)m_religions.size() - 1; j >= 0; j--) {
				unit->HD_AddReligionInfluence( m_religions[j] );
			}
		}
	}
	for (auto unit : m_mapPolygonUnits) {
		unit->RecalculateMajorReligion();
	}

	for (auto religion : m_religions) {
		m_religionLabels.push_back( new Label( religion, LabelType::Religion ) );
	}
}

void Map::GenerateCitiesAndTowns()
{
	// choose city position
	for (auto unit : m_mapPolygonUnits) {
		unit->HD_GenerateCities();
	}

	for (auto city : m_cities) {
		city->Initialize();
	}

	for (auto unit : m_mapPolygonUnits) {
		unit->HD_GenerateTowns();
	}
	for (auto town: m_towns) {
		town->Initialize();
	}

}

void Map::GenerateCountriesAndRelations()
{
	// mostly one culture will have one to several countries
	// there may be several empires which include multiple cultures

	// first loop through all cultures and decide its state
	for (auto culture : m_cultures) {
		int cultureState = 0;
		float rnd = m_mapRNG->RollRandomFloatZeroToOne();
		if (culture->m_origin == CultureGeoOrigin::MountainOrigin) {
			if (rnd < 0.3f) {
				cultureState = 1;
			}
			else if (rnd < 0.6f) {
				cultureState = 2;
			}
			else if (rnd < 0.95f) {
				cultureState = 4;
			}
			else {
				cultureState = 3;
			}
		}
		else if (culture->m_origin == CultureGeoOrigin::GrasslandOrigin) {
			if (rnd < 0.3f) {
				cultureState = 2;
			}
			else if (rnd < 0.4f) {
				cultureState = 3;
			}
			else if (rnd < 0.9f) {
				cultureState = 4;
			}
			else {
				cultureState = 5;
			}
		}
		else if (culture->m_origin == CultureGeoOrigin::ForestOrigin) {
			if (rnd < 0.2f) {
				cultureState = 1;
			}
			else if (rnd < 0.6f) {
				cultureState = 2;
			}
			else if (rnd < 0.95f) {
				cultureState = 4;
			}
			else {
				cultureState = 3;
			}
		}
		else if (culture->m_origin == CultureGeoOrigin::OceanOrigin) {
			if (rnd < 0.3f) {
				cultureState = 2;
			}
			else if (rnd < 0.6f) {
				cultureState = 5;
			}
			else if (rnd < 0.7f) {
				cultureState = 1;
			}
			else if (rnd < 0.9f) {
				cultureState = 3;
			}
			else {
				cultureState = 4;
			}
		}
		else if (culture->m_origin == CultureGeoOrigin::RiverOrigin) {
			if (rnd < 0.1f) {
				cultureState = 1;
			}
			else if (rnd < 0.6f) {
				cultureState = 2;
			}
			else if (rnd < 0.7f) {
				cultureState = 3;
			}
			else if (rnd < 0.8f) {
				cultureState = 5;
			}
			else {
				cultureState = 6;
			}
		}
		else {
			ERROR_AND_DIE( "Culture Origin Error!" );
		}
		// just randomly decide the state of the culture
		std::vector<MapPolygonUnit*> provs;
		culture->GetAllMajorCultureProvs( provs );
		int numOfProvs = (int)provs.size();
		int numOfCountries = 0;
		// 1. scattered
		if (cultureState == 1) {
			numOfCountries = numOfProvs / 8;
		}
		// 2. several countries
		else if (cultureState == 2) {
			numOfCountries = std::max( numOfProvs / 20, m_mapRNG->RollRandomIntInRange( 3, 5 ) );
		}
		// 3. unified
		else if (cultureState == 3) {
			numOfCountries = 1;
		}
		// 4. tribe union
		else if (cultureState == 4) {
			// no tribe unions for now
			//numOfCountries = numOfProvs / 9;
			cultureState = 1;
			numOfCountries = numOfProvs / 8;
		}
		// 5. center orientated
		else if (cultureState == 5) {
			if (numOfProvs > 40) {
				numOfCountries = 1 + GetClamped( numOfProvs / 8 - 1, 0, INT_MAX );
			}
			else { // fall back to unified
				numOfCountries = 1;
			}
		}
		// 6. celestial empire
		else if (cultureState == 6) {
			numOfCountries = 1;
		}

		culture->m_initialState = cultureState;
		numOfCountries = numOfCountries <= 0 ? 1 : numOfCountries;

		CountryGovernmentType countryGovType = CountryGovernmentType::None;
		// 1. scattered 2. several countries 3. unified 4. tribe union 5. center orientated 6. celestial empire
		if (cultureState == 1 && (culture->m_origin == CultureGeoOrigin::RiverOrigin || culture->m_origin == CultureGeoOrigin::ForestOrigin)) {
			if (m_mapRNG->RollRandomFloatZeroToOne() < 0.5f) {
				countryGovType = CountryGovernmentType::Autocracy;
			}
			else {
				countryGovType = CountryGovernmentType::Oligarchy;
			}
		}
		else if (cultureState == 1) {
			if (m_mapRNG->RollRandomFloatZeroToOne() < 0.5f) {
				countryGovType = CountryGovernmentType::Nomad;
			}
			else {
				countryGovType = CountryGovernmentType::Oligarchy;
			}
		}
		else if (cultureState == 2) {
			if (m_mapRNG->RollRandomFloatZeroToOne() < 0.1f) {
				countryGovType = CountryGovernmentType::Parliamentarism;
			}
			else {
				countryGovType = CountryGovernmentType::Autocracy;
			}
		}
		else if (cultureState == 3) {
			countryGovType = CountryGovernmentType::Autocracy;
		}
		else if (cultureState == 4) {
			countryGovType = CountryGovernmentType::Nomad;
		}
		else if (cultureState == 5) {
			countryGovType = CountryGovernmentType::Autocracy;
		}
		else if (cultureState == 6) {
			countryGovType = CountryGovernmentType::Autocracy;
		}
		// randomly choose start province
		std::vector<Country*> generatedCountries;
		for (int i = 0; i < numOfCountries; i++) {
			if (provs.size() == 0) {
				break;
			}
			int tryTimes = 0;
			int provMinDist = 2;
			if (cultureState == 1 || cultureState == 4 || cultureState == 5) {
				provMinDist = 1;
			}
			else if (cultureState == 2) {
				provMinDist = 3;
			}
			int provDist = 0;
			int rndIndex = 0;
			do {
				rndIndex = m_mapRNG->RollRandomIntLessThan( (int)provs.size() );
				tryTimes++;
				provDist = INT_MAX;
				for (auto country: generatedCountries) {
					std::vector<MapPolygonUnit*> thisRoute;
					country->m_originProv->GetOneRouteToProvince( thisRoute, country->m_originProv, provs[rndIndex] );
					if ((int)thisRoute.size() < provDist) {
						provDist = (int)thisRoute.size();
					}
				}
			} while (provDist < provMinDist + 2 && tryTimes < 1000);
			MapPolygonUnit* startProv = provs[rndIndex];
			provs.erase( provs.begin() + rndIndex );
			Country* newCountry = new Country();
			m_countries.push_back( newCountry );
			newCountry->m_id = (int)m_countries.size() - 1;
			generatedCountries.push_back( newCountry );
			newCountry->GainProvince( startProv );
			newCountry->m_originProv = startProv;
			newCountry->m_countryCulture = culture;
			newCountry->m_countryReligion = startProv->m_majorReligion;
			newCountry->m_governmentType = countryGovType;

			// add relations to country
			if (cultureState == 4) { // tribe union, add union to all prev countries
				for (auto country : generatedCountries) {
					if (country != newCountry) {
						country->AddTribeUnionRelation( newCountry );
					}
				}
			}
			else if (cultureState == 5) {
				if (i != 0) {
					newCountry->AddBeingVassalRelation( generatedCountries[0] );
				}
			}
		}

		// expand countries
		if (numOfCountries == 1) {
			if (cultureState == 6) {
				generatedCountries[0]->m_isCelestial = true;
			}
			for (auto prov : provs) {
				generatedCountries[0]->GainProvince( prov );
			}
		}
		else {
			/*for (auto prov : provs) {
				FindCountryForProvinceJob* job = new FindCountryForProvinceJob( prov, cultureState, generatedCountries );
				m_findCountryForProvinceJobs.push_back( job );
				g_theJobSystem->AddJob( job );
			}

			while (!m_findCountryForProvinceJobs.empty()) {
				if (g_theJobSystem->RetrieveJob( m_findCountryForProvinceJobs.front() )) {
					delete m_findCountryForProvinceJobs.front();
					m_findCountryForProvinceJobs.pop_front();
				}
			}*/
			for (auto prov : provs) {
				// if there is a center country, the center country is more powerful
				if (cultureState == 5) {
					Country* nearestCountry = nullptr;
					float nearestDist = FLT_MAX;
					for (int i = 0; i < (int)generatedCountries.size(); i++) {
						MapPolygonUnit* startUnit = generatedCountries[i]->m_provinces[0];
						float thisDist = 0.f;
						if (i == 0) {
							//thisDist = GetDistanceSquared2D( startUnit->m_geoCenterPos, prov->m_geoCenterPos ) * 0.25f;
							thisDist = startUnit->CalculateGeometricDistance( prov ) * 0.25f;
						}
						else {
							//thisDist = GetDistanceSquared2D( startUnit->m_geoCenterPos, prov->m_geoCenterPos );
							thisDist = startUnit->CalculateGeometricDistance( prov );
						}
						if (thisDist < nearestDist) {
							nearestDist = thisDist;
							nearestCountry = generatedCountries[i];
						}
					}
					nearestCountry->GainProvince( prov );
				}
				else {
					// give to the nearest country
					Country* nearestCountry = nullptr;
					float nearestDist = FLT_MAX;
					for (auto country : generatedCountries) {
						MapPolygonUnit* startUnit = country->m_provinces[0];
						//float thisDist = GetDistanceSquared2D( startUnit->m_geoCenterPos, prov->m_geoCenterPos );
						float thisDist = startUnit->CalculateGeometricDistance( prov );

						if (thisDist < nearestDist) {
							nearestDist = thisDist;
							nearestCountry = country;
						}
					}
					// find a qualified country
					nearestCountry->GainProvince( prov );
				}
			}
		}
	}

	// set the capital to the most influenced province
	for (auto country : m_countries) {
		country->ResetCapitalProvince();
	}
	// set the capital city of countries
	for (auto country : m_countries) {
		if (country->m_capitalProv->m_cities.size() > 0) {
			std::sort( country->m_capitalProv->m_cities.begin(), country->m_capitalProv->m_cities.end(), []( City const* a, City const* b ) {return a->m_totalPopulation > b->m_totalPopulation; } );
			country->LoseProvince( country->m_capitalProv );
			City* mostPopulusCity = country->m_capitalProv->m_cities[0];
			mostPopulusCity->AddAttribute( CityAttribute::Capital );
			mostPopulusCity->m_totalPopulation = (int)((float)mostPopulusCity->m_totalPopulation * 1.25f);
			country->m_capitalCity = mostPopulusCity;
			country->GainProvince( country->m_capitalProv );
		}
		else { // if there is no city in the capital province, create one
			country->LoseProvince( country->m_capitalProv );
			country->m_capitalProv->HD_GenerateCities( true, 1 );
			City* newCapitalCity = country->m_capitalProv->m_cities[0];
			newCapitalCity->Initialize();
			newCapitalCity->AddAttribute( CityAttribute::Capital );
			newCapitalCity->m_totalPopulation = (int)((float)newCapitalCity->m_totalPopulation * 1.25f);
			country->m_capitalCity = newCapitalCity;
			country->GainProvince( country->m_capitalProv );
		}
	}

	// we need to make sure provinces are connected unless it's a scattered culture
	ConnectCountries();

	// add legitimate provinces
	for (auto country : m_countries) {
		for (auto prov : country->m_provinces) {
			prov->AddLegitimateCountry( country );
		}
	}

	// procedurally annex some province or country
	// 1. annex some country
	for (int i = 0; i < (int)m_countries.size() / 3; i++) {
		Country* country1 = m_countries[m_mapRNG->RollRandomIntLessThan( (int)m_countries.size() )];
		std::vector<Country*> adjCountries;
		country1->GetAdjacentCountries( adjCountries );
		if ((int)adjCountries.size() > 0) {
			Country* country2 = adjCountries[m_mapRNG->RollRandomIntLessThan( (int)adjCountries.size() )];
			if (country1->m_countryCulture != country2->m_countryCulture && country1->m_countryReligion == country2->m_countryReligion) {
				// annex country will let surrounding same culture country be angry with that country
				if (country1->m_totalPopulation > country2->m_totalPopulation) {
					std::vector<Country*> country2AdjCountries;
					country2->GetAdjacentCountries( country2AdjCountries );
					for (auto sameCultureCountry : country2AdjCountries) {
						if (sameCultureCountry->m_countryCulture == country2->m_countryCulture) {
							CountryRelationType relation = sameCultureCountry->GetRelationTo( country1 );
							if (relation == CountryRelationType::None) {
								sameCultureCountry->AddHostileRelation( country1 );
							}
							else if (relation == CountryRelationType::Hostile) {
								sameCultureCountry->RemoveHostileRelation( country1 );
								sameCultureCountry->AddWarRelation( country1 );
							}
						}
					}
					for (auto sameCultureCountry : m_countries) {
						if (sameCultureCountry->m_countryCulture == country2->m_countryCulture) {
							CountryRelationType relation = sameCultureCountry->GetRelationTo( country1 );
							if (relation == CountryRelationType::None) {
								sameCultureCountry->AddHostileRelation( country1 );
							}
						}
					}
					country1->AnnexCountry( country2 );
				}
				else {
					std::vector<Country*> country1AdjCountries;
					country1->GetAdjacentCountries( country1AdjCountries );
					for (auto sameCultureCountry : country1AdjCountries) {
						if (sameCultureCountry->m_countryCulture == country1->m_countryCulture) {
							CountryRelationType relation = sameCultureCountry->GetRelationTo( country2 );
							if (relation == CountryRelationType::None) {
								sameCultureCountry->AddHostileRelation( country2 );
							}
							else if (relation == CountryRelationType::Hostile) {
								sameCultureCountry->RemoveHostileRelation( country2 );
								sameCultureCountry->AddWarRelation( country2 );
							}
						}
					}
					for (auto sameCultureCountry : m_countries) {
						if (sameCultureCountry->m_countryCulture == country1->m_countryCulture) {
							CountryRelationType relation = sameCultureCountry->GetRelationTo( country2 );
							if (relation == CountryRelationType::None) {
								sameCultureCountry->AddHostileRelation( country2 );
							}
						}
					}
					country2->AnnexCountry( country1 );
				}
			}
		}
	}

	// 2. annex some province
	for (int i = 0; i < (int)m_mapPolygonUnits.size() / 10; i++) {
		MapPolygonUnit* unitToBeAnnexed1 = m_mapPolygonUnits[m_mapRNG->RollRandomIntLessThan( (int)m_mapPolygonUnits.size() )];
		if (!unitToBeAnnexed1->IsWater() && !unitToBeAnnexed1->IsCapitalProv()) {
			if (unitToBeAnnexed1->m_adjacentUnits.size() > 0) {
				MapPolygonUnit* unitToBeAnnexed2 = unitToBeAnnexed1->m_adjacentUnits[m_mapRNG->RollRandomIntLessThan( (int)unitToBeAnnexed1->m_adjacentUnits.size() )];
				if (unitToBeAnnexed1->m_owner != nullptr && unitToBeAnnexed2->m_owner != nullptr
					&& unitToBeAnnexed1->m_owner != unitToBeAnnexed2->m_owner) {
					CountryRelationType relation1to2 = unitToBeAnnexed1->m_owner->GetRelationTo( unitToBeAnnexed2->m_owner );
					CountryRelationType relation2to1 = unitToBeAnnexed2->m_owner->GetRelationTo( unitToBeAnnexed1->m_owner );
					// annex province will let the original owner angry
					if (relation1to2 == CountryRelationType::None && relation2to1 == CountryRelationType::None) {
						unitToBeAnnexed1->m_owner->AddHostileRelation( unitToBeAnnexed2->m_owner );
					}
					else if (relation1to2 == CountryRelationType::Hostile && relation2to1 == CountryRelationType::Hostile) {
						unitToBeAnnexed1->m_owner->RemoveHostileRelation( unitToBeAnnexed2->m_owner );
						unitToBeAnnexed1->m_owner->AddWarRelation( unitToBeAnnexed2->m_owner );
					}
					unitToBeAnnexed1->m_owner->LoseProvince( unitToBeAnnexed1, true );
					unitToBeAnnexed2->m_owner->GainProvince( unitToBeAnnexed1 );
				}
			}
		}
	}

	ConnectCountries();

	// 2. generate some cross-cultural empire
	// do not do this currently 
	// because there are already some cross culture countries are generated by annexing others
	//for (int i = 0; i < 2; i++) {

	//}

	// set the relationship of celestial empire
	for (auto country : m_countries) {
		if (country->IsCelestial()) {
			std::vector<Country*> adjCountries;
			country->GetAdjacentCountries( adjCountries );
			for (auto adjCountry : adjCountries) {
				CountryRelationType relation = country->GetRelationTo( adjCountry );
				if (relation == CountryRelationType::None && adjCountry->m_totalPopulation < country->m_totalPopulation * 0.2f) {
					country->AddTributaryRelation( adjCountry );
				}
			}
		}
	}

	// add friendly and alliance relationship
	for (int i = 0; i < (int)m_countries.size(); i++) {
		Country* country1 = m_countries[i];
		for (int j = i + 1; j < (int)m_countries.size(); j++) {
			Country* country2 = m_countries[j];
			CountryRelationType relation = country1->GetRelationTo( country2 );
			// requires no relationship
			if (relation == CountryRelationType::None) {
				// same culture and same religion, more possible to be friends
				if (country1->m_countryCulture == country2->m_countryCulture && country1->m_countryReligion == country2->m_countryReligion) {
					float rnd = m_mapRNG->RollRandomFloatZeroToOne();
					// if a country is in war, it will be more likely to find allies
					if (country1->IsInWar() && country2->IsInWar()) {
						if (country1->HasNoRelationToCountryList( country2->m_relationWarCountries )
							&& country2->HasNoRelationToCountryList( country1->m_relationWarCountries )) {
							if (rnd < 0.2f) {
								country1->AddFriendlyRelation( country2 );
							}
							else if (rnd < 0.4f) {
								country1->AddAllianceRelation( country2 );
							}
						}
						else {
							bool hasSameEnemy = false;
							for (auto warCountry : country2->m_relationWarCountries) {
								if (country1->IsInWarWith( warCountry )) {
									hasSameEnemy = true;
									break;
								}
							}
							if (hasSameEnemy) {
								if (rnd < 0.3f) {
									country1->AddFriendlyRelation( country2 );
								}
								else{
									country1->AddAllianceRelation( country2 );
								}
							}
						}
					}
					else if(country1->IsInWar()){
						if (country2->HasNoRelationToCountryList( country1->m_relationWarCountries )) {
							if (rnd < 0.2f) {
								country1->AddFriendlyRelation( country2 );
							}
							else if (rnd < 0.4f) {
								country1->AddAllianceRelation( country2 );
							}
						}
					}
					else if (country2->IsInWar()) {
						if (country1->HasNoRelationToCountryList( country2->m_relationWarCountries )) {
							if (rnd < 0.2f) {
								country1->AddFriendlyRelation( country2 );
							}
							else if (rnd < 0.4f) {
								country1->AddAllianceRelation( country2 );
							}
						}
					}
					else {
						if (rnd < 0.3f) {
							country1->AddFriendlyRelation( country2 );
						}
						else if (rnd < 0.4f) {
							country1->AddAllianceRelation( country2 );
						}
					}
				}
				// same culture and different religion, more possible to be enemy
				else if (country1->m_countryCulture == country2->m_countryCulture && country1->m_countryReligion != country2->m_countryReligion) {
					if (!country1->IsMemberOfUnion() && !country2->IsMemberOfUnion()) {
						if (country1->IsAdjacentToCountry( country2 )) {
							float rnd = m_mapRNG->RollRandomFloatZeroToOne();
							if (rnd < 0.7f) {
								country1->AddHostileRelation( country2 );
							}
						}
						else {
							float rnd = m_mapRNG->RollRandomFloatZeroToOne();
							if (rnd < 0.2f) {
								country1->AddHostileRelation( country2 );
							}
						}
					}
				}
				// same religion different culture, likely to be friends
				else if (country1->m_countryCulture != country2->m_countryCulture && country1->m_countryReligion == country2->m_countryReligion) {
					if (!country1->IsMemberOfUnion() && !country2->IsMemberOfUnion()) {
						if (country1->IsAdjacentToCountry( country2 )) {
							float rnd = m_mapRNG->RollRandomFloatZeroToOne();
							if (rnd < 0.1f) {
								country1->AddFriendlyRelation( country2 );
							}
						}
						else {
							float rnd = m_mapRNG->RollRandomFloatZeroToOne();
							if (rnd < 0.3f) {
								country1->AddFriendlyRelation( country2 );
							}
						}
					}
				}
			}
		}
	}

	// suzerain will enter vassal's war
	for (auto country : m_countries) {
		if (country->IsVassal() && country->IsInWar()) {
			for (auto countryInWarWith : country->m_relationWarCountries) {
				CountryRelationType type = countryInWarWith->GetRelationTo( country->m_relationSuzerain );
				if (type == CountryRelationType::Hostile) {
					countryInWarWith->RemoveHostileRelation( country->m_relationSuzerain );
					countryInWarWith->AddWarRelation( country->m_relationSuzerain );
				}
				else if (type == CountryRelationType::None) {
					countryInWarWith->AddWarRelation( country->m_relationSuzerain );
				}
				else if (type == CountryRelationType::Friendly) {
					countryInWarWith->RemoveFriendlyRelation( country->m_relationSuzerain );
					countryInWarWith->AddWarRelation( country->m_relationSuzerain );
				}
			}
		}
	}

	// a chance to enter ally war
	for (auto country : m_countries) {
		for (auto ally : country->m_relationAllianceCountries) {
			if (ally->IsInWar()) {
				for (auto warCountry : ally->m_relationWarCountries) {
					CountryRelationType relation = country->GetRelationTo( warCountry );
					if (relation == CountryRelationType::Hostile || relation == CountryRelationType::None) {
						if (relation == CountryRelationType::Hostile) {
							country->RemoveHostileRelation( warCountry );
						}
						country->AddWarRelation( warCountry );
					}
				}
			}
		}
	}

	// all vassals will enter suzerain's war
	for (auto country : m_countries) {
		if (country->IsVassal()) {
			if (country->m_relationSuzerain->IsInWar()) {
				for (auto warCountry : country->m_relationSuzerain->m_relationWarCountries) {
					country->RemoveAllRelationWithCountry( warCountry );
					country->AddWarRelation( warCountry );
				}
			}
		}
	}

	// add more legitimate provinces
	for (auto country : m_countries) {
		for (auto prov : country->m_provinces) {
			// same culture and same religion, can be involved in the country
			if (prov->m_majorCulture == country->m_countryCulture && prov->m_majorReligion == country->m_countryReligion) {
				prov->AddLegitimateCountry( country );
			}
			// do not consider these two now
			else if (prov->m_majorCulture == country->m_countryCulture) {
				
			}
			else if (prov->m_majorReligion == country->m_countryReligion) {

			}
		}
	}

	// generate country names
	for (auto country : m_countries) {
		country->m_name = country->m_countryCulture->m_countryNameGenerator->GenerateCountryName( country );
	}

	// create labels for countries
	for (auto country : m_countries) {
		m_countryLabels.push_back( new Label( country, LabelType::Country ) );
	}

	// generate city names
	for (auto city : m_cities) {
		city->m_name = city->m_majorCulture->m_cityTownNameGenerator->GenerateCityName( city );
	}
	for (auto town : m_towns) {
		town->m_name = town->m_majorCulture->m_cityTownNameGenerator->GenerateTownName( town );
	}
}

void Map::ConnectCountries()
{
	// for any provinces that has an owner
	for (auto prov : m_mapPolygonUnits) {
		if (!prov->m_isFarAwayFakeUnit && !prov->IsWater()) {
			if (!IsTwoProvConnectedInSameCountry( prov, prov->m_owner->m_capitalProv )) {
				bool findSameCultureCountry = false;
				for (auto adjProv : prov->m_adjacentUnits) {
					if (adjProv->m_majorCulture == prov->m_majorCulture && prov->m_owner != adjProv->m_owner && adjProv->m_owner) {
						prov->m_owner->LoseProvince( prov, true );
						adjProv->m_owner->GainProvince( prov );
						findSameCultureCountry = true;
						break;
					}
				}
				bool findSameReligionCountry = false;
				if (!findSameCultureCountry) {
					for (auto adjProv : prov->m_adjacentUnits) {
						if (prov->m_owner != adjProv->m_owner && adjProv->m_owner && adjProv->m_majorReligion == prov->m_majorReligion) {
							prov->m_owner->LoseProvince( prov, true );
							adjProv->m_owner->GainProvince( prov );
							findSameReligionCountry = true;
							break;
						}
					}
				}
				if (!findSameReligionCountry) {
					for (auto adjProv : prov->m_adjacentUnits) {
						if (prov->m_owner != adjProv->m_owner && adjProv->m_owner) {
							prov->m_owner->LoseProvince( prov, true );
							adjProv->m_owner->GainProvince( prov );
							break;
						}
					}
				}
			}
		}
	}

}

void Map::GenerateProvinceProducts()
{
	for (auto unit : m_mapPolygonUnits) {
		unit->CalculateProduct();
	}
}

void Map::GenerateRegions()
{
	// sort the adjacent provinces
	//for (auto unit : m_mapPolygonUnits) {
	//	std::sort( unit->m_adjacentUnits.begin(), unit->m_adjacentUnits.end(), [&]( MapPolygonUnit* a, MapPolygonUnit* b ) {return GetDistanceSquared2D( a->m_geoCenterPos, unit->m_geoCenterPos ) < GetDistanceSquared2D( b->m_geoCenterPos, unit->m_geoCenterPos ); } );
	//}

	std::vector<MapPolygonUnit*> copiedList = m_mapPolygonUnits;
	int maxProvInRegion = GetClamped( m_generationSettings.m_basePolygons / 1500, 4, 10000 );
	int minProvInRegion = GetClamped( m_generationSettings.m_basePolygons / 5000, 2, 10000 );
	while (!copiedList.empty()) {
		MapPolygonUnit* thisUnit = copiedList.back();
		copiedList.pop_back();
		if (!thisUnit->IsWater()) {
			Region* newRegion = new Region();
			newRegion->m_id = (int)m_regions.size();
			m_regions.push_back( newRegion );
			newRegion->GainProvince( thisUnit );
			int numOfProv = m_mapRNG->RollRandomIntInRange( minProvInRegion, maxProvInRegion ) - 1;
			for (int i = 0; i < numOfProv; i++) {
				std::vector<MapPolygonUnit*> adjAvailableProvs;
				// add all adj available units into list
				for (auto regionUnit : newRegion->m_containedUnits) {
					// find all adj units
					for (auto adjUnit : regionUnit->m_adjacentUnits) {
						if (!adjUnit->IsWater() 
							&& std::find( newRegion->m_containedUnits.begin(), newRegion->m_containedUnits.end(), adjUnit ) == newRegion->m_containedUnits.end()
							&& std::find( copiedList.begin(), copiedList.end(), adjUnit ) != copiedList.end()) {
							adjAvailableProvs.push_back( adjUnit );
						}
					}
				}
				float maxWeight = 0.f;
				MapPolygonUnit* findResult = nullptr;
				for (auto testUnit : adjAvailableProvs) {
					float weight = 0.f;
					if (testUnit->m_owner == thisUnit->m_owner) {
						weight += 0.5f;
					}
					if (testUnit->m_majorCulture == testUnit->m_majorCulture) {
						weight += 0.4f;
					}
					if (testUnit->m_landform == testUnit->m_landform) {
						weight += 0.5f;
					}
					if (testUnit->m_majorReligion == testUnit->m_majorReligion) {
						weight += 0.3f;
					}
					weight += 1.f - GetClamped( 7.5f * (float)numOfProv * GetDistance2D( testUnit->m_geoCenterPos, thisUnit->m_geoCenterPos ) / Maxf( m_dimensions.x, m_dimensions.y ), 0.f, 1.f );
					
					if (weight >= 0.9f && weight > maxWeight) {
						findResult = testUnit;
						maxWeight = weight;
					}
				}
				if (findResult) {
					newRegion->GainProvince( findResult );
					copiedList.erase( std::find( copiedList.begin(), copiedList.end(), findResult ) );
				}
				else {
					break;
				}
			}
		}
	}

	for (auto region : m_regions) {
		m_regionLabels.push_back( new Label( region, LabelType::Region ) );
	}
}

static inline auto const cityDistCmp =
[]( std::pair<City*, float>const& a, std::pair<City*, float> const& b ) {
	return a.second > b.second;
	};
void Map::GenerateRoads()
{
	for (int i = 0; i < (int)m_cities.size(); i++) {
		City* city1 = m_cities[i];
		std::priority_queue<std::pair<City*, float>, std::vector<std::pair<City*, float>>, decltype(cityDistCmp)> m_cityDistQueue;
		for (int j = 0; j < (int)m_cities.size(); j++) {
			if (j == i) {
				continue;
			}
			City* city2 = m_cities[j];
			float distSquared = GetDistanceSquared2D( city1->m_position, city2->m_position );
			m_cityDistQueue.push( std::pair<City*, float>( city2, distSquared ) );
		}
		int numOfCitiesNeedToBeConnected = 3;
		if (city1->HasAttribute(CityAttribute::Capital)) {
			numOfCitiesNeedToBeConnected += 1;
		}
		if (city1->HasAttribute(CityAttribute::Commercial)) {
			numOfCitiesNeedToBeConnected += 1;
		}
		if (city1->HasAttribute(CityAttribute::Port)) {
			numOfCitiesNeedToBeConnected += 2;
		}
		for(int _ = 0; _ < numOfCitiesNeedToBeConnected; _++){
			if (m_cityDistQueue.empty()) {
				break;
			}
			City* minCity = m_cityDistQueue.top().first;
			m_cityDistQueue.pop();
			Road* newRoad = new Road( city1, minCity );
			newRoad->Initialize();
			m_roads.push_back( newRoad );
		}
		City* minCity = m_cityDistQueue.top().first;
		if (minCity->m_owner == city1->m_owner) {
			m_cityDistQueue.pop();
			Road* newRoad = new Road( city1, minCity );
			newRoad->Initialize();
			m_roads.push_back( newRoad );
		}
	}

	for (int i = 0; i < (int)m_roads.size();) {
		if (m_roads[i]->m_isGarbage) {
			delete m_roads[i];
			m_roads.erase( m_roads.begin() + i );
		}
		else {
			++i;
		}
	}
}

void Map::GenerateArmies()
{
	/*float (*EvaluateProvinceScore)(MapPolygonUnit*) = [](MapPolygonUnit* prov)
		{ 
			float value = 0.f;
			float populationValue = SmoothStop2( RangeMapClamped( (float)prov->m_totalPopulation, 0.f, 40000.f, 0.f, 1.f ) );
			value += populationValue;
			value += (int)prov->m_cities.size() * 0.5f;
			value += (int)prov->m_towns.size() * 0.1f;
			if (prov->m_majorCulture != prov->m_owner->m_countryCulture) {
				value += populationValue;
			}
			if (prov->m_majorReligion != prov->m_owner->m_countryReligion) {
				value += populationValue;
			}
			if (prov->IsCapitalProv()) {
				value *= 2.f;
			}
			return value; 
		
		};*/
	// evaluate the score of each province of each country
	for (auto country : m_countries) {
		if (!country->IsExist()) {
			continue;
		}
		// find out what provinces need to be place an army
		std::vector<MapPolygonUnit*> provs = country->m_provinces;
		std::sort( provs.begin(), provs.end(), [](MapPolygonUnit* a, MapPolygonUnit* b) 
			{
				return a->EvaluateProvinceArmyScore() > b->EvaluateProvinceArmyScore();
			} );
		// calculate how many armies should be placed
		int numOfArmies = std::max( (int)country->m_provinces.size() / 20, 1 );

		// calculate how many soldiers need to be placed
		int totalManPower = 0;
		for (auto& culturePair : country->m_cultures) {
			// main culture can give more man power
			if (culturePair.first == country->m_countryCulture) {
				totalManPower += (culturePair.second / 50);
			}
			else {
				totalManPower += (culturePair.second / 200);
			}
		}
		
		float totalScore = 0.f;
		for (int i = 0; i < numOfArmies; i++) {
			totalScore += provs[i]->m_provinceArmyScore;
		}

		for (int i = 0; i < numOfArmies; i++) {
			Army* newArmy = new Army( country, provs[i], std::max( int( abs( totalManPower / totalScore * provs[i]->m_provinceArmyScore ) ), 100 ) );
			newArmy->m_globalID = m_globalArmyID;
			++m_globalArmyID;
			if (m_globalArmyID == INVALID_ARMY_ID) {
				++m_globalArmyID;
			}
			newArmy->m_isActive = true;
			provs[i]->m_armiesOnProv.push_back( std::pair<Army*, float>( newArmy, 0.f ) );
			country->m_armies.push_back( newArmy );
		}
	}
}

void Map::GenerateVertexBuffers()
{
	// make noisy edges
	for (auto unit : m_mapPolygonUnits) {
		if (!unit->m_isFarAwayFakeUnit && (int)unit->m_edges.size() > 0) {
			for (auto edge : unit->m_edges) {
				float dist = (edge->m_startPos - edge->m_endPos).GetLength() / m_diagonalLength;
				if (dist < 0.001f) {
					edge->DoNoisyEdge( 0, m_mapRNG->RollRandomPositiveNegative() > 0 ? m_mapRNG->RollRandomFloatInRange( 0.2f, 0.4f ) : 1.f - m_mapRNG->RollRandomFloatInRange( 0.2f, 0.4f ) );
				}
				else if (dist < 0.002f) {
					edge->DoNoisyEdge( 1, m_mapRNG->RollRandomPositiveNegative() > 0 ? m_mapRNG->RollRandomFloatInRange( 0.2f, 0.4f ) : 1.f - m_mapRNG->RollRandomFloatInRange( 0.2f, 0.4f ) );
				}
				else if (dist < 0.004f) {
					edge->DoNoisyEdge( 2, m_mapRNG->RollRandomPositiveNegative() > 0 ? m_mapRNG->RollRandomFloatInRange( 0.2f, 0.4f ) : 1.f - m_mapRNG->RollRandomFloatInRange( 0.2f, 0.4f ) );
				}
				else if (dist < 0.006f) {
					edge->DoNoisyEdge( 3, m_mapRNG->RollRandomPositiveNegative() > 0 ? m_mapRNG->RollRandomFloatInRange( 0.2f, 0.4f ) : 1.f - m_mapRNG->RollRandomFloatInRange( 0.2f, 0.4f ) );
				}
				else {
					edge->DoNoisyEdge( 4, m_mapRNG->RollRandomPositiveNegative() > 0 ? m_mapRNG->RollRandomFloatInRange( 0.2f, 0.4f ) : 1.f - m_mapRNG->RollRandomFloatInRange( 0.2f, 0.4f ) );
				}
			}
		}
	}
	// re-arrange and correct noisy edges
// 	for (auto unit : m_mapPolygonUnits) {
// 		if (!unit->m_isFarAwayFakeUnit) {
// 			for (int i = 0; i < (int)unit->m_edges.size(); i++) {
// 				if ((int)unit->m_edges[i]->m_noisyEdges.size() >= 3) {
// 					for (int j = 0; j < (int)unit->m_edges[i]->m_noisyEdges.size() - 1; j++) {
// 						// if we do not do cross product, there will be only half are ccw
// 						if (CrossProduct2D( unit->m_edges[i]->m_noisyEdges[j] - unit->m_edges[i]->m_owner->m_geoCenterPos,
// 							unit->m_edges[i]->m_noisyEdges[j + 1] - unit->m_edges[i]->m_owner->m_geoCenterPos ) < 0.f) {
// 							Vec2 tempVec2 = unit->m_edges[i]->m_noisyEdges[j];
// 							unit->m_edges[i]->m_noisyEdges[j] = unit->m_edges[i]->m_noisyEdges[j + 1];
// 							unit->m_edges[i]->m_noisyEdges[j + 1] = tempVec2;
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}
	// create vertex buffers
	// create polygon edges vertex buffers
	std::vector<Vertex_PCU> verts;
	verts.reserve( 100000 );
	std::vector<Vertex_PCU> vertsForEachPolygon;
	vertsForEachPolygon.reserve( 1000 );
	std::vector<unsigned int> indexes;
	for (int i = 0; i < (int)m_mapPolygonUnits.size(); i++) {
		for (int j = 0; j < (int)m_mapPolygonUnits[i]->m_edges.size(); j++) {
			if (m_bounds.IsPointInside( m_mapPolygonUnits[i]->m_edges[j]->m_startPos )
				|| m_bounds.IsPointInside( m_mapPolygonUnits[i]->m_edges[j]->m_endPos )) {
				m_mapPolygonUnits[i]->m_edges[j]->m_isDrawn = true;
				if (m_mapPolygonUnits[i]->m_edges[j]->m_opposite && m_mapPolygonUnits[i]->m_edges[j]->m_opposite->m_isDrawn) {
					if (m_mapPolygonUnits[i]->m_isFarAwayFakeUnit || (int)m_mapPolygonUnits[i]->m_edges[j]->m_noisyEdges.size() == 0) {
						Vec2 startPos = m_mapPolygonUnits[i]->m_edges[j]->m_startPos;
						Vec2 endPos = m_mapPolygonUnits[i]->m_edges[j]->m_endPos;
						PM_ClampLinesIntoBounds( startPos, endPos );
// 						if (!m_bounds.IsPointInside( startPos ) || !m_bounds.IsPointInside( endPos )) {
// 							continue;
// 						}
						vertsForEachPolygon.emplace_back( startPos, Rgba8::WHITE );
						vertsForEachPolygon.emplace_back( endPos, Rgba8::WHITE );
					}
					else {
						for (int k = 0; k < (int)(m_mapPolygonUnits[i]->m_edges[j]->m_noisyEdges.size() - 1); k++) {
							Vec2 startPos = m_mapPolygonUnits[i]->m_edges[j]->m_noisyEdges[k];
							Vec2 endPos = m_mapPolygonUnits[i]->m_edges[j]->m_noisyEdges[k + 1];
							PM_ClampLinesIntoBounds( startPos, endPos );
// 							if (!m_bounds.IsPointInside( startPos ) || !m_bounds.IsPointInside( endPos )) {
// 								continue;
// 							}
							vertsForEachPolygon.emplace_back( startPos, Rgba8::WHITE );
							vertsForEachPolygon.emplace_back( endPos, Rgba8::WHITE );
						}
					}
				}
				else {
					if (m_mapPolygonUnits[i]->m_isFarAwayFakeUnit || (int)m_mapPolygonUnits[i]->m_edges[j]->m_noisyEdges.size() == 0) {
						Vec2 startPos = m_mapPolygonUnits[i]->m_edges[j]->m_startPos;
						Vec2 endPos = m_mapPolygonUnits[i]->m_edges[j]->m_endPos;
						PM_ClampLinesIntoBounds( startPos, endPos );
// 						if (!m_bounds.IsPointInside( startPos ) || !m_bounds.IsPointInside( endPos )) {
// 							continue;
// 						}
						indexes.push_back( (unsigned int)verts.size() );
						verts.emplace_back( startPos, Rgba8( 96, 96, 96 ) );
						indexes.push_back( (unsigned int)verts.size() );
						verts.emplace_back( endPos, Rgba8( 96, 96, 96 ) );
						vertsForEachPolygon.emplace_back( startPos, Rgba8::WHITE );
						vertsForEachPolygon.emplace_back( endPos, Rgba8::WHITE );
					}
					else {
						bool skippedLast = false;
						for (int k = 0; k < (int)(m_mapPolygonUnits[i]->m_edges[j]->m_noisyEdges.size() - 1); k++) {
							Vec2 startPos = m_mapPolygonUnits[i]->m_edges[j]->m_noisyEdges[k];
							Vec2 endPos = m_mapPolygonUnits[i]->m_edges[j]->m_noisyEdges[k + 1];
							PM_ClampLinesIntoBounds( startPos, endPos );
// 							if (!m_bounds.IsPointInside( startPos ) || !m_bounds.IsPointInside( endPos )) {
// 								skippedLast = true;
// 								continue;
// 							}
							if (k == 0 || skippedLast) {
								skippedLast = false;
								indexes.push_back( (unsigned int)verts.size() );
								verts.emplace_back( startPos, Rgba8( 96, 96, 96 ) );
								indexes.push_back( (unsigned int)verts.size() );
								verts.emplace_back( endPos, Rgba8( 96, 96, 96 ) );
							}
							else {
								indexes.push_back( (unsigned int)verts.size() - 1 );
								indexes.push_back( (unsigned int)verts.size() );
								verts.emplace_back( endPos, Rgba8( 96, 96, 96 ) );
							}
							vertsForEachPolygon.emplace_back( startPos, Rgba8::WHITE );
							vertsForEachPolygon.emplace_back( endPos, Rgba8::WHITE );
						}
					}
				}
			}
		}
		if (vertsForEachPolygon.size() > 0) {
			m_mapPolygonUnits[i]->m_edgeShowingBuffer = g_theRenderer->CreateVertexBuffer( vertsForEachPolygon.size() * sizeof( Vertex_PCU ) );
			g_theRenderer->CopyCPUToGPU( vertsForEachPolygon.data(), vertsForEachPolygon.size() * sizeof( Vertex_PCU ), m_mapPolygonUnits[i]->m_edgeShowingBuffer );
			m_mapPolygonUnits[i]->m_edgeShowingBuffer->SetAsLinePrimitive( true );
			vertsForEachPolygon.clear();
		}
	}

	indexes.push_back( (unsigned int)verts.size() );
	verts.emplace_back( m_bounds.m_mins, Rgba8( 0, 0, 0 ) );
	indexes.push_back( (unsigned int)verts.size() );
	verts.emplace_back( Vec2( m_bounds.m_mins.x, m_bounds.m_maxs.y ), Rgba8( 0, 0, 0 ) );
	indexes.push_back( (unsigned int)verts.size() );
	verts.emplace_back( m_bounds.m_mins, Rgba8( 0, 0, 0 ) );
	indexes.push_back( (unsigned int)verts.size() );
	verts.emplace_back( Vec2( m_bounds.m_maxs.x, m_bounds.m_mins.y ), Rgba8( 0, 0, 0 ) );
	indexes.push_back( (unsigned int)verts.size() );
	verts.emplace_back( m_bounds.m_maxs, Rgba8( 0, 0, 0 ) );
	indexes.push_back( (unsigned int)verts.size() );
	verts.emplace_back( Vec2( m_bounds.m_maxs.x, m_bounds.m_mins.y ), Rgba8( 0, 0, 0 ) );
	indexes.push_back( (unsigned int)verts.size() );
	verts.emplace_back( m_bounds.m_maxs, Rgba8( 0, 0, 0 ) );
	indexes.push_back( (unsigned int)verts.size() );
	verts.emplace_back( Vec2( m_bounds.m_mins.x, m_bounds.m_maxs.y ), Rgba8( 0, 0, 0 ) );

	m_polygonEdgesIndexBuffer = g_theRenderer->CreateIndexBuffer( indexes.size() * sizeof( unsigned int ) );
	g_theRenderer->CopyCPUToGPU( indexes.data(), indexes.size() * sizeof( unsigned int ), m_polygonEdgesIndexBuffer );
	m_polygonsEdgesVertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_polygonsEdgesVertexBuffer );
	m_polygonsEdgesVertexBuffer->SetAsLinePrimitive( true );

	vertsForEachPolygon.clear();
	verts.clear();
	indexes.clear();
	for (int i = 0; i < (int)m_mapPolygonUnits.size(); i++) {
		for (int j = 0; j < (int)m_mapPolygonUnits[i]->m_edges.size(); j++) {
			m_mapPolygonUnits[i]->m_edges[j]->m_isDrawn = false;
		}
	}

	for (int i = 0; i < (int)m_mapPolygonUnits.size(); i++) {
		int edgeSize = (int)m_mapPolygonUnits[i]->m_edges.size();
		for (int j = 0; j < edgeSize; j++) {
			if (m_bounds.IsPointInside( m_mapPolygonUnits[i]->m_edges[j]->m_startPos )
				|| m_bounds.IsPointInside( m_mapPolygonUnits[i]->m_edges[j]->m_endPos )) {
				m_mapPolygonUnits[i]->m_edges[j]->m_isDrawn = true;
				if (m_mapPolygonUnits[i]->m_edges[j]->m_opposite && m_mapPolygonUnits[i]->m_edges[j]->m_opposite->m_isDrawn) {

					constexpr float factor = HEIGHT_FACTOR;
					MapPolygonUnit* unit = m_mapPolygonUnits[i];

					float prevHeight = m_generationSettings.m_minHeight;
					float oppoHeight = m_generationSettings.m_minHeight;
					float nextHeight = m_generationSettings.m_minHeight;
					if (unit->m_edges[j]->m_prev->m_opposite) {
						prevHeight = unit->m_edges[j]->m_prev->m_opposite->m_owner->m_height;
					}
					if (unit->m_edges[j]->m_opposite) {
						oppoHeight = unit->m_edges[j]->m_opposite->m_owner->m_height;
					}
					if (unit->m_edges[j]->m_next->m_opposite) {
						nextHeight = unit->m_edges[j]->m_next->m_opposite->m_owner->m_height;
					}
					float startHeight = (unit->m_height + oppoHeight + prevHeight) / 3.f;
					float endHeight = (unit->m_height + oppoHeight + nextHeight) / 3.f; 
					if (unit->m_isFarAwayFakeUnit || (int)edgeSize == 0) {
						Vec2 startPos = m_mapPolygonUnits[i]->m_edges[j]->m_startPos;
						Vec2 endPos = m_mapPolygonUnits[i]->m_edges[j]->m_endPos;
						PM_ClampLinesIntoBounds( startPos, endPos );
						vertsForEachPolygon.emplace_back( Vec3( startPos, factor * startHeight ), Rgba8::WHITE );
						vertsForEachPolygon.emplace_back( Vec3( endPos, factor * endHeight ), Rgba8::WHITE );
					}
					else {
						int noisyEdgeSize = (int)m_mapPolygonUnits[i]->m_edges[j]->m_noisyEdges.size();
						for (int k = 0; k < noisyEdgeSize - 1; k++) {
							Vec2 startPos = m_mapPolygonUnits[i]->m_edges[j]->m_noisyEdges[k];
							Vec2 endPos = m_mapPolygonUnits[i]->m_edges[j]->m_noisyEdges[k + 1];
							PM_ClampLinesIntoBounds( startPos, endPos );
							float firstHeight = factor * Interpolate( startHeight, endHeight, (float)k / (float)(noisyEdgeSize - 1) );
							float secondHeight = factor * Interpolate( startHeight, endHeight, (float)(k + 1) / (float)(noisyEdgeSize - 1) );
							vertsForEachPolygon.emplace_back( Vec3( startPos, firstHeight ), Rgba8::WHITE );
							vertsForEachPolygon.emplace_back( Vec3( endPos, secondHeight ), Rgba8::WHITE );
						}
					}
				}
				else {
					constexpr float factor = HEIGHT_FACTOR;
					MapPolygonUnit* unit = m_mapPolygonUnits[i];
					float prevHeight = m_generationSettings.m_minHeight;
					float oppoHeight = m_generationSettings.m_minHeight;
					float nextHeight = m_generationSettings.m_minHeight;
					if (unit->m_edges[j]->m_prev->m_opposite) {
						prevHeight = unit->m_edges[j]->m_prev->m_opposite->m_owner->m_height;
					}
					if (unit->m_edges[j]->m_opposite) {
						oppoHeight = unit->m_edges[j]->m_opposite->m_owner->m_height;
					}
					if (unit->m_edges[j]->m_next->m_opposite) {
						nextHeight = unit->m_edges[j]->m_next->m_opposite->m_owner->m_height;
					}
					float startHeight = (unit->m_height + oppoHeight + prevHeight) / 3.f;
					float endHeight = (unit->m_height + oppoHeight + nextHeight ) / 3.f;
					if (unit->m_isFarAwayFakeUnit || (int)edgeSize == 0) {
						Vec2 startPos = m_mapPolygonUnits[i]->m_edges[j]->m_startPos;
						Vec2 endPos = m_mapPolygonUnits[i]->m_edges[j]->m_endPos;
						PM_ClampLinesIntoBounds( startPos, endPos );
						indexes.push_back( (unsigned int)verts.size() );
						verts.emplace_back( Vec3( startPos, factor * startHeight ), Rgba8( 96, 96, 96 ) );
						indexes.push_back( (unsigned int)verts.size() );
						verts.emplace_back( Vec3( endPos, factor * endHeight ), Rgba8( 96, 96, 96 ) );
						vertsForEachPolygon.emplace_back( Vec3( startPos, factor * startHeight ), Rgba8::WHITE );
						vertsForEachPolygon.emplace_back( Vec3( endPos, factor * endHeight ), Rgba8::WHITE );
					}
					else {
						int noisyEdgeSize = (int)m_mapPolygonUnits[i]->m_edges[j]->m_noisyEdges.size();
						for (int k = 0; k < noisyEdgeSize - 1; k++) {
							Vec2 startPos = m_mapPolygonUnits[i]->m_edges[j]->m_noisyEdges[k];
							Vec2 endPos = m_mapPolygonUnits[i]->m_edges[j]->m_noisyEdges[k + 1];
							PM_ClampLinesIntoBounds( startPos, endPos );
							float firstHeight = factor * Interpolate( startHeight, endHeight, (float)k / (float)(noisyEdgeSize - 1) );
							float secondHeight = factor * Interpolate( startHeight, endHeight, (float)(k + 1) / (float)(noisyEdgeSize - 1) );
							if (k == 0) {
								indexes.push_back( (unsigned int)verts.size() );
								verts.emplace_back( Vec3( startPos, firstHeight ), Rgba8( 96, 96, 96 ) );
								indexes.push_back( (unsigned int)verts.size() );
								verts.emplace_back( Vec3( endPos, secondHeight ), Rgba8( 96, 96, 96 ) );
							}
							else {
								indexes.push_back( (unsigned int)verts.size() - 1 );
								indexes.push_back( (unsigned int)verts.size() );
								verts.emplace_back( Vec3( endPos, secondHeight ), Rgba8( 96, 96, 96 ) );
							}
							vertsForEachPolygon.emplace_back( Vec3( startPos, firstHeight ), Rgba8::WHITE );
							vertsForEachPolygon.emplace_back( Vec3( endPos, secondHeight ), Rgba8::WHITE );
						}
					}
				}
			}
		}
		if (vertsForEachPolygon.size() > 0) {
			m_mapPolygonUnits[i]->m_edgeShowingBuffer3D = g_theRenderer->CreateVertexBuffer( vertsForEachPolygon.size() * sizeof( Vertex_PCU ) );
			g_theRenderer->CopyCPUToGPU( vertsForEachPolygon.data(), vertsForEachPolygon.size() * sizeof( Vertex_PCU ), m_mapPolygonUnits[i]->m_edgeShowingBuffer3D );
			m_mapPolygonUnits[i]->m_edgeShowingBuffer3D->SetAsLinePrimitive( true );
			vertsForEachPolygon.clear();
		}
	}

	indexes.push_back( (unsigned int)verts.size() );
	verts.emplace_back( Vec3( m_bounds.m_mins, m_generationSettings.m_minHeight * HEIGHT_FACTOR ), Rgba8( 0, 0, 0 ) );
	indexes.push_back( (unsigned int)verts.size() );
	verts.emplace_back( Vec3( m_bounds.m_mins.x, m_bounds.m_maxs.y, m_generationSettings.m_minHeight* HEIGHT_FACTOR ), Rgba8( 0, 0, 0 ) );
	indexes.push_back( (unsigned int)verts.size() );
	verts.emplace_back( Vec3( m_bounds.m_mins, m_generationSettings.m_minHeight* HEIGHT_FACTOR ), Rgba8( 0, 0, 0 ) );
	indexes.push_back( (unsigned int)verts.size() );
	verts.emplace_back( Vec3( m_bounds.m_maxs.x, m_bounds.m_mins.y, m_generationSettings.m_minHeight* HEIGHT_FACTOR ), Rgba8( 0, 0, 0 ) );
	indexes.push_back( (unsigned int)verts.size() );
	verts.emplace_back( Vec3( m_bounds.m_maxs, m_generationSettings.m_minHeight* HEIGHT_FACTOR ), Rgba8( 0, 0, 0 ) );
	indexes.push_back( (unsigned int)verts.size() );
	verts.emplace_back( Vec3( m_bounds.m_maxs.x, m_bounds.m_mins.y, m_generationSettings.m_minHeight* HEIGHT_FACTOR ), Rgba8( 0, 0, 0 ) );
	indexes.push_back( (unsigned int)verts.size() );
	verts.emplace_back( Vec3( m_bounds.m_maxs, m_generationSettings.m_minHeight* HEIGHT_FACTOR ), Rgba8( 0, 0, 0 ) );
	indexes.push_back( (unsigned int)verts.size() );
 	verts.emplace_back( Vec3( m_bounds.m_mins.x, m_bounds.m_maxs.y, m_generationSettings.m_minHeight * HEIGHT_FACTOR ), Rgba8( 0, 0, 0 ) );

	m_polygonEdgesIndexBuffer3D = g_theRenderer->CreateIndexBuffer( indexes.size() * sizeof( unsigned int ) );
	g_theRenderer->CopyCPUToGPU( indexes.data(), indexes.size() * sizeof( unsigned int ), m_polygonEdgesIndexBuffer3D );
	m_polygonsEdgesVertexBuffer3D = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_polygonsEdgesVertexBuffer3D );
	m_polygonsEdgesVertexBuffer3D->SetAsLinePrimitive( true );

	verts.clear();
	indexes.clear();
	
	// create bounds blocker buffers (not in use now)
	AddVertsForAABB2D( verts, AABB2( Vec2( m_bounds.m_maxs.x, -EDGE_GUARD_Y ), Vec2( EDGE_GUARD_X, EDGE_GUARD_Y ) ), Rgba8::WHITE );
	AddVertsForAABB2D( verts, AABB2( Vec2( -EDGE_GUARD_X, m_bounds.m_maxs.y ), Vec2( EDGE_GUARD_X, EDGE_GUARD_Y ) ), Rgba8::WHITE );
	AddVertsForAABB2D( verts, AABB2( Vec2( -EDGE_GUARD_X, -EDGE_GUARD_Y ), Vec2( m_bounds.m_mins.x, EDGE_GUARD_Y ) ), Rgba8::WHITE );
	AddVertsForAABB2D( verts, AABB2( Vec2( -EDGE_GUARD_X, -EDGE_GUARD_Y ), Vec2( EDGE_GUARD_X, m_bounds.m_mins.y ) ), Rgba8::WHITE );
	m_edgeBlockVertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_edgeBlockVertexBuffer );

	verts.clear();

	// create sites vertex buffer
	float radiusOfSite = 10.f * Minf( m_generationSettings.m_dimensions.x, m_generationSettings.m_dimensions.y ) / m_generationSettings.m_basePolygons;
	for (int i = 0; i < (int)m_mapPolygonUnits.size(); i++) {
		AddVertsForAABB2D( verts, AABB2( m_mapPolygonUnits[i]->m_centerPosition - Vec2( radiusOfSite, radiusOfSite )
			, m_mapPolygonUnits[i]->m_centerPosition + Vec2( radiusOfSite, radiusOfSite ) ), Rgba8( 255, 153, 153 ) );
	}

	m_polygonCenterVertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_polygonCenterVertexBuffer );

	// create polygon rendering vertex buffer
	std::vector<Vec3> polygonPos;
	std::vector<Vec2> polygonUVs;
	polygonPos.reserve( 1000000 );

	for (auto unit : m_mapPolygonUnits) {
		unit->m_startInVertexBuffer = polygonPos.size();
		if (!unit->m_isFarAwayFakeUnit) {
			for (int i = 0; i < (int)unit->m_edges.size(); i++) {
				if ((int)unit->m_edges[i]->m_noisyEdges.size() >= 3) {
					for (int j = 0; j < (int)unit->m_edges[i]->m_noisyEdges.size() - 1; j++) {
						// if we do not do cross product, there will be only half are ccw
						constexpr float minUV = 0.05f;
						constexpr float maxUV = 0.95f;
						Vec2 startPos = unit->m_edges[i]->m_owner->m_geoCenterPos;
 						Vec2 endPos1 = unit->m_edges[i]->m_noisyEdges[j];
						Vec2 endPos2 = unit->m_edges[i]->m_noisyEdges[j + 1];
						polygonPos.emplace_back( startPos );
						polygonPos.emplace_back( endPos1 );
						polygonPos.emplace_back( endPos2 );

						polygonUVs.emplace_back( RangeMapClamped( startPos.x / m_dimensions.x, 0.f, 1.f, minUV, maxUV ), RangeMapClamped( startPos.y / m_dimensions.y, 0.f, 1.f, minUV, maxUV ) );
						polygonUVs.emplace_back( RangeMapClamped( endPos1.x / m_dimensions.x, 0.f, 1.f, minUV, maxUV ), RangeMapClamped( endPos1.y / m_dimensions.y, 0.f, 1.f, minUV, maxUV ) );
						polygonUVs.emplace_back( RangeMapClamped( endPos2.x / m_dimensions.x, 0.f, 1.f, minUV, maxUV ), RangeMapClamped( endPos2.y / m_dimensions.y, 0.f, 1.f, minUV, maxUV ) );
						m_colorVertexArray.emplace_back( Rgba8( 255, 255, 255, 100 ) );
						m_colorVertexArray.emplace_back( Rgba8( 255, 255, 255, 100 ) );
						m_colorVertexArray.emplace_back( Rgba8( 255, 255, 255, 100 ) );
					}
				}
				else {
					constexpr float minUV = 0.05f;
					constexpr float maxUV = 0.95f;
					Vec2 startPos = unit->m_edges[i]->m_owner->m_geoCenterPos;
					Vec2 endPos1 = unit->m_edges[i]->m_startPos;
					Vec2 endPos2 = unit->m_edges[i]->m_endPos;
// 					PM_ClampLinesIntoBounds( startPos, endPos1 );
// 					PM_ClampLinesIntoBounds( startPos, endPos2 );

					//if (!m_bounds.IsPointInside( startPos ) || !m_bounds.IsPointInside( endPos1 ) || !m_bounds.IsPointInside( endPos2 )) {
					//	continue;
					//}
					//if (CrossProduct2D( unit->m_edges[i]->m_startPos - unit->m_edges[i]->m_owner->m_centerPosition,
					//	unit->m_edges[i]->m_endPos - unit->m_edges[i]->m_owner->m_centerPosition ) > 0.f) {
						polygonPos.emplace_back( startPos );
						polygonPos.emplace_back( endPos1 );
						polygonPos.emplace_back( endPos2 );
					//}
					//else {
					//	polygonPos.emplace_back( unit->m_edges[i]->m_owner->m_centerPosition );
					//	polygonPos.emplace_back( unit->m_edges[i]->m_startPos );
					//	polygonPos.emplace_back( unit->m_edges[i]->m_endPos );
					//}
					m_colorVertexArray.emplace_back( Rgba8( 255, 255, 255, 100 ) );
					m_colorVertexArray.emplace_back( Rgba8( 255, 255, 255, 100 ) );
					m_colorVertexArray.emplace_back( Rgba8( 255, 255, 255, 100 ) );
					polygonUVs.emplace_back( RangeMapClamped( startPos.x / m_dimensions.x, 0.f, 1.f, minUV, maxUV ), RangeMapClamped( startPos.y / m_dimensions.y, 0.f, 1.f, minUV, maxUV ) );
					polygonUVs.emplace_back( RangeMapClamped( endPos1.x / m_dimensions.x, 0.f, 1.f, minUV, maxUV ), RangeMapClamped( endPos1.y / m_dimensions.y, 0.f, 1.f, minUV, maxUV ) );
					polygonUVs.emplace_back( RangeMapClamped( endPos2.x / m_dimensions.x, 0.f, 1.f, minUV, maxUV ), RangeMapClamped( endPos2.y / m_dimensions.y, 0.f, 1.f, minUV, maxUV ) );
				}
			}
			/*StarEdge* edge0 = unit->m_edges[0];
			StarEdge* edge1 = edge0->m_next;
			StarEdge* edge2 = edge1->m_next;
			while (edge2 != edge0) {
				polygonPos.emplace_back( edge0->m_startPos );
				polygonPos.emplace_back( edge1->m_startPos );
				polygonPos.emplace_back( edge2->m_startPos );
				m_colorVertexArray.emplace_back( Rgba8( 255, 255, 255, 100 ) );
				m_colorVertexArray.emplace_back( Rgba8( 255, 255, 255, 100 ) );
				m_colorVertexArray.emplace_back( Rgba8( 255, 255, 255, 100 ) );
				polygonUVs.emplace_back( 0.f, 0.f );
				polygonUVs.emplace_back( 0.f, 0.f );
				polygonUVs.emplace_back( 0.f, 0.f );
				edge1 = edge2;
				edge2 = edge2->m_next;
			}*/
		}
		unit->m_sizeInVertexBuffer = polygonPos.size() - unit->m_startInVertexBuffer;
	}
	m_polygonsFacesVertexBuffer = g_theRenderer->CreateVertexBuffer( polygonPos.size() * sizeof( Vec3 ), sizeof( Vec3 ) );
	g_theRenderer->CopyCPUToGPU( polygonPos.data(), polygonPos.size() * sizeof( Vec3 ), m_polygonsFacesVertexBuffer );
	m_polygonFacesColorVertexBuffer = g_theRenderer->CreateVertexBuffer( m_colorVertexArray.size() * sizeof( Rgba8 ), sizeof( Rgba8 ) );
	g_theRenderer->CopyCPUToGPU( m_colorVertexArray.data(), m_colorVertexArray.size() * sizeof( Rgba8 ), m_polygonFacesColorVertexBuffer );
	m_polygonuvVertexBuffer = g_theRenderer->CreateVertexBuffer( polygonUVs.size() * sizeof( Vec2 ), sizeof( Vec2 ) );
	g_theRenderer->CopyCPUToGPU( polygonUVs.data(), polygonUVs.size() * sizeof( Vec2 ), m_polygonuvVertexBuffer );

	verts.clear();
	// create 3D polygon rendering vertex buffer
	polygonPos.clear();
	polygonUVs.clear();
	std::vector<Vec3> normal;
	normal.reserve( 100000 );

	for (auto unit : m_mapPolygonUnits) {
		unit->m_startIn3DVertexBuffer = polygonPos.size();
		if (!unit->m_isFarAwayFakeUnit) {
			for (int i = 0; i < (int)unit->m_edges.size(); i++) {
				float prevHeight = m_generationSettings.m_minHeight;
				float oppoHeight = m_generationSettings.m_minHeight;
				float nextHeight = m_generationSettings.m_minHeight;
				if (unit->m_edges[i]->m_prev->m_opposite) {
					prevHeight = unit->m_edges[i]->m_prev->m_opposite->m_owner->m_height;
				}
				if (unit->m_edges[i]->m_opposite) {
					oppoHeight = unit->m_edges[i]->m_opposite->m_owner->m_height;
				}
				if (unit->m_edges[i]->m_next->m_opposite) {
					nextHeight = unit->m_edges[i]->m_next->m_opposite->m_owner->m_height;
				}
				float startHeight = (unit->m_height + oppoHeight + prevHeight) / 3.f;
				float endHeight = (unit->m_height + oppoHeight + nextHeight) / 3.f;
				int edgeSize = (int)unit->m_edges[i]->m_noisyEdges.size();
				constexpr float factor = HEIGHT_FACTOR;
 				if (edgeSize >= 3) {
					for (int j = 0; j < edgeSize - 1; j++) {
						Vec2 startPos = unit->m_edges[i]->m_owner->m_geoCenterPos;
						Vec2 endPos1 = unit->m_edges[i]->m_noisyEdges[j];
						Vec2 endPos2 = unit->m_edges[i]->m_noisyEdges[j + 1];
						PM_ClampLinesIntoBounds( startPos, endPos1 );
						PM_ClampLinesIntoBounds( startPos, endPos2 );
						constexpr float minUV = 0.f;
						constexpr float maxUV = 1.f;
						Vec3 pos1 = Vec3( startPos, factor * unit->m_height );
						Vec3 pos2 = Vec3( endPos1, factor * Interpolate( startHeight, endHeight, (float)j / (float)(edgeSize - 1) ) );
						Vec3 pos3 = Vec3( endPos2, factor * Interpolate( startHeight, endHeight, (float)(j + 1) / (float)(edgeSize - 1) ) );
 						polygonPos.emplace_back( pos1 );
 						polygonPos.emplace_back( pos2 );
 						polygonPos.emplace_back( pos3 );
						polygonUVs.emplace_back( RangeMapClamped( startPos.x / m_dimensions.x, 0.f, 1.f, minUV, maxUV ), RangeMapClamped( startPos.y / m_dimensions.y, 0.f, 1.f, minUV, maxUV ) );
						polygonUVs.emplace_back( RangeMapClamped( endPos1.x / m_dimensions.x, 0.f, 1.f, minUV, maxUV ), RangeMapClamped( endPos1.y / m_dimensions.y, 0.f, 1.f, minUV, maxUV ) );
						polygonUVs.emplace_back( RangeMapClamped( endPos2.x / m_dimensions.x, 0.f, 1.f, minUV, maxUV ), RangeMapClamped( endPos2.y / m_dimensions.y, 0.f, 1.f, minUV, maxUV ) );
						normal.emplace_back( CrossProduct3D( pos2 - pos1, pos3 - pos1 ) );
						normal.emplace_back( CrossProduct3D( pos3 - pos2, pos1 - pos2 ) );
						normal.emplace_back( CrossProduct3D( pos1 - pos3, pos2 - pos3 ) );
					}
 				}
				else {
					constexpr float minUV = 0.f;
					constexpr float maxUV = 1.f;
					Vec2 startPos = unit->m_edges[i]->m_owner->m_geoCenterPos;
					Vec2 endPos1 = unit->m_edges[i]->m_startPos;
					Vec2 endPos2 = unit->m_edges[i]->m_endPos;
					Vec3 pos1 = Vec3( startPos, factor * unit->m_height );
					Vec3 pos2 = Vec3( endPos1, factor * startHeight );
					Vec3 pos3 = Vec3( endPos2, factor * endHeight );
					PM_ClampLinesIntoBounds( startPos, endPos1 );
					PM_ClampLinesIntoBounds( startPos, endPos2 );
					polygonPos.emplace_back( pos1 );
					polygonPos.emplace_back( pos2 );
					polygonPos.emplace_back( pos3 );
					polygonUVs.emplace_back( RangeMapClamped( startPos.x / m_dimensions.x, 0.f, 1.f, minUV, maxUV ), RangeMapClamped( startPos.y / m_dimensions.y, 0.f, 1.f, minUV, maxUV ) );
					polygonUVs.emplace_back( RangeMapClamped( endPos1.x / m_dimensions.x, 0.f, 1.f, minUV, maxUV ), RangeMapClamped( endPos1.y / m_dimensions.y, 0.f, 1.f, minUV, maxUV ) );
					polygonUVs.emplace_back( RangeMapClamped( endPos2.x / m_dimensions.x, 0.f, 1.f, minUV, maxUV ), RangeMapClamped( endPos2.y / m_dimensions.y, 0.f, 1.f, minUV, maxUV ) );
					normal.emplace_back( CrossProduct3D( pos2 - pos1, pos3 - pos1 ) );
					normal.emplace_back( CrossProduct3D( pos3 - pos2, pos1 - pos2 ) );
					normal.emplace_back( CrossProduct3D( pos1 - pos3, pos2 - pos3 ) );
				}
			}
		}
		unit->m_sizeIn3DVertexBuffer = polygonPos.size() - unit->m_startIn3DVertexBuffer;
	}
	m_polygonuv3DVertexBuffer = g_theRenderer->CreateVertexBuffer( polygonUVs.size() * sizeof( Vec2 ), sizeof( Vec2 ) );
	g_theRenderer->CopyCPUToGPU( polygonUVs.data(), polygonUVs.size() * sizeof( Vec2 ), m_polygonuv3DVertexBuffer );

	m_polygonsFaces3DVertexBuffer = g_theRenderer->CreateVertexBuffer( polygonPos.size() * sizeof( Vec3 ), sizeof( Vec3 ) );
	g_theRenderer->CopyCPUToGPU( polygonPos.data(), polygonPos.size() * sizeof( Vec3 ), m_polygonsFaces3DVertexBuffer );

	m_polygonsNormalVertexBuffer = g_theRenderer->CreateVertexBuffer( normal.size() * sizeof( Vec3 ), sizeof( Vec3 ) );
	g_theRenderer->CopyCPUToGPU( normal.data(), normal.size() * sizeof( Vec3 ), m_polygonsNormalVertexBuffer );

	// create map boarder vertex buffer
	verts.clear();
	float boarderWidth = m_dimensions.GetLength() / 15.f;
	constexpr float uMin = 0.15f;
	constexpr float uMax = 1.f - uMin;
	constexpr float vMin = 0.16f;
	constexpr float vMax = 1.f - vMin;
	constexpr float outerUMin = 0.07f;
	constexpr float outerUMax = 0.93f;
	constexpr float outerVMin = 0.07f;
	constexpr float outerVMax = 0.93f;
	Rgba8 tintColor = Rgba8( 241, 233, 210 );
	verts.emplace_back( Vec2( 0.f, 0.f ), tintColor, Vec2(uMin, vMin));
	verts.emplace_back( Vec2( m_dimensions.x + boarderWidth, -boarderWidth ), tintColor, Vec2(outerUMax, outerVMin) );
	verts.emplace_back( Vec2( m_dimensions.x, 0.f ), tintColor, Vec2(uMax, vMin) );

	verts.emplace_back( Vec2( -boarderWidth, -boarderWidth ), tintColor, Vec2(outerUMin, outerVMin) );
	verts.emplace_back( Vec2( m_dimensions.x + boarderWidth, -boarderWidth ), tintColor, Vec2(outerUMax, outerVMin) );
	verts.emplace_back( Vec2( 0.f, 0.f ), tintColor, Vec2(uMin, vMin) );

	verts.emplace_back( Vec2( 0.f, 0.f ), tintColor, Vec2( uMin, vMin ) );
	verts.emplace_back( Vec2( -boarderWidth, m_dimensions.y + boarderWidth ), tintColor, Vec2( outerUMin, outerVMax ) );
	verts.emplace_back( Vec2( -boarderWidth, -boarderWidth ), tintColor, Vec2( outerUMin, outerVMin ) );

	verts.emplace_back( Vec2( 0.f, 0.f ), tintColor, Vec2( uMin, vMin ) );
	verts.emplace_back( Vec2( 0.f, m_dimensions.y ), tintColor, Vec2( uMin, vMax ) );
	verts.emplace_back( Vec2( -boarderWidth, m_dimensions.y + boarderWidth ), tintColor, Vec2( outerUMin, outerVMax ) );

	verts.emplace_back( Vec2( 0.f, m_dimensions.y ), tintColor, Vec2( uMin, vMax ) );
	verts.emplace_back( Vec2( m_dimensions.x, m_dimensions.y ), tintColor, Vec2( uMax, vMax ) );
	verts.emplace_back( Vec2( -boarderWidth, m_dimensions.y + boarderWidth ), tintColor, Vec2( outerUMin, outerVMax ) );

	verts.emplace_back( Vec2( m_dimensions.x, m_dimensions.y ), tintColor, Vec2( uMax, vMax ) );
	verts.emplace_back( Vec2( m_dimensions.x + boarderWidth, m_dimensions.y + boarderWidth ), tintColor, Vec2( outerUMax, outerVMax ) );
	verts.emplace_back( Vec2( -boarderWidth, m_dimensions.y + boarderWidth ), tintColor, Vec2( outerUMin, outerVMax ) );

	verts.emplace_back( Vec2( m_dimensions.x, 0.f ), tintColor, Vec2( uMax, vMin ) );
	verts.emplace_back( Vec2( m_dimensions.x + boarderWidth, -boarderWidth ), tintColor, Vec2( outerUMax, outerVMin ) );
	verts.emplace_back( Vec2( m_dimensions.x + boarderWidth, m_dimensions.y + boarderWidth ), tintColor, Vec2( outerUMax, outerVMax ) );

	verts.emplace_back( Vec2( m_dimensions.x, 0.f ), tintColor, Vec2( uMax, vMin ) );
	verts.emplace_back( Vec2( m_dimensions.x + boarderWidth, m_dimensions.y + boarderWidth ), tintColor, Vec2( outerUMax, outerVMax ) );
	verts.emplace_back( Vec2( m_dimensions.x, m_dimensions.y ), tintColor, Vec2( uMax, vMax ) );

	m_mapBorderVertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_mapBorderVertexBuffer );
	verts.clear();

	for (auto city : m_cities) {
		if (city->HasAttribute(CityAttribute::Capital)) {
			AddVertsForAABB2D( verts, AABB2( city->m_iconBounds.m_mins
				- Vec2( CITY_ICON_SIDE_LENGTH * 0.5f, CITY_ICON_SIDE_LENGTH * 0.5f ), city->m_iconBounds.m_maxs
				+ Vec2( CITY_ICON_SIDE_LENGTH * 0.5f, CITY_ICON_SIDE_LENGTH * 0.5f ) ), Rgba8( 255, 51, 51 ) );
		}
		else {
			AddVertsForAABB2D( verts, city->m_iconBounds, Rgba8( 180, 0, 0 ) );
		}
	}
	m_cityVertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_cityVertexBuffer );

	verts.clear();
	for (auto town : m_towns) {
		AddVertsForAABB2D( verts, town->m_iconBounds, Rgba8( 0, 0, 0 ) );
	}
	m_townVertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_townVertexBuffer );

	// create country edges vertex buffers
	for (int i = 0; i < (int)m_mapPolygonUnits.size(); i++) {
		for (int j = 0; j < (int)m_mapPolygonUnits[i]->m_edges.size(); j++) {
			m_mapPolygonUnits[i]->m_edges[j]->m_isDrawn = false;
		}
	}
	verts.clear();
	std::vector<Vertex_PCU> vertsForEachCountry;
	vertsForEachCountry.reserve( 1000 );
	for (auto country : m_countries) {
		for (auto prov : country->m_provinces) {
			for (auto edge : prov->m_edges) {
				// if this is the boarder of the country, then add verts to vector
				if (edge->m_opposite && edge->m_opposite->m_owner && edge->m_opposite->m_owner->m_owner != country) {
					if (m_bounds.IsPointInside( edge->m_startPos ) || m_bounds.IsPointInside( edge->m_endPos )) {
						edge->m_isDrawn = true;
						if (edge->m_opposite && edge->m_opposite->m_isDrawn) {
							if ((int)edge->m_noisyEdges.size() == 0) {
								Vec2 startPos = edge->m_startPos;
								Vec2 endPos = edge->m_endPos;
								PM_ClampLinesIntoBounds( startPos, endPos );
								vertsForEachCountry.emplace_back( Vec3( startPos.x, startPos.y, 0.01f ), Rgba8::WHITE );
								vertsForEachCountry.emplace_back( Vec3( endPos.x, endPos.y, 0.01f ), Rgba8::WHITE );
							}
							else {
								for (int k = 0; k < (int)(edge->m_noisyEdges.size() - 1); k++) {
									vertsForEachCountry.emplace_back( Vec3( edge->m_noisyEdges[k].x, edge->m_noisyEdges[k].y, 0.01f ), Rgba8::WHITE );
									vertsForEachCountry.emplace_back( Vec3( edge->m_noisyEdges[k + 1].x, edge->m_noisyEdges[k + 1].y, 0.01f ), Rgba8::WHITE );
								}
							}
						}
						else {
							if ((int)edge->m_noisyEdges.size() == 0) {
								Vec2 startPos = edge->m_startPos;
								Vec2 endPos = edge->m_endPos;
								PM_ClampLinesIntoBounds( startPos, endPos );
								verts.emplace_back( startPos, Rgba8( 20, 20, 20 ) );
								verts.emplace_back( endPos, Rgba8( 20, 20, 20 ) );
								vertsForEachCountry.emplace_back( Vec3( startPos.x, startPos.y, 0.01f ), Rgba8::WHITE );
								vertsForEachCountry.emplace_back( Vec3( endPos.x, endPos.y, 0.01f ), Rgba8::WHITE );
							}
							else {
								for (int k = 0; k < (int)(edge->m_noisyEdges.size() - 1); k++) {
									verts.emplace_back( edge->m_noisyEdges[k], Rgba8( 20, 20, 20 ) );
									verts.emplace_back( edge->m_noisyEdges[k + 1], Rgba8( 20, 20, 20 ) );
									vertsForEachCountry.emplace_back( Vec3( edge->m_noisyEdges[k].x, edge->m_noisyEdges[k].y, 0.01f ), Rgba8::WHITE );
									vertsForEachCountry.emplace_back( Vec3( edge->m_noisyEdges[k + 1].x, edge->m_noisyEdges[k + 1].y, 0.01f ), Rgba8::WHITE );
								}
							}
						}
					}
				}
			}
		
		}
		if (vertsForEachCountry.size() > 0) {
			country->m_edgeShowingBuffer = g_theRenderer->CreateVertexBuffer( vertsForEachCountry.size() * sizeof( Vertex_PCU ) );
			g_theRenderer->CopyCPUToGPU( vertsForEachCountry.data(), vertsForEachCountry.size() * sizeof( Vertex_PCU ), country->m_edgeShowingBuffer );
			country->m_edgeShowingBuffer->SetAsLinePrimitive( true );
			vertsForEachCountry.clear();
		}
	}

	m_countriesEdgesVertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_countriesEdgesVertexBuffer );
	m_countriesEdgesVertexBuffer->SetAsLinePrimitive( true );
	verts.clear();

	// create 3D country edges vertex buffers
	for (int i = 0; i < (int)m_mapPolygonUnits.size(); i++) {
		for (int j = 0; j < (int)m_mapPolygonUnits[i]->m_edges.size(); j++) {
			m_mapPolygonUnits[i]->m_edges[j]->m_isDrawn = false;
		}
	}
	vertsForEachCountry.clear();
	vertsForEachCountry.reserve( 1000 );
	for (auto country : m_countries) {
		for (auto prov : country->m_provinces) {
			for (auto edge : prov->m_edges) {
				// if this is the boarder of the country, then add verts to vector
				if (edge->m_opposite && edge->m_opposite->m_owner && edge->m_opposite->m_owner->m_owner != country) {
					if (m_bounds.IsPointInside( edge->m_startPos ) || m_bounds.IsPointInside( edge->m_endPos )) {
						edge->m_isDrawn = true;
						if (edge->m_opposite && edge->m_opposite->m_isDrawn) {
							constexpr float factor = HEIGHT_FACTOR;

							float prevHeight = m_generationSettings.m_minHeight;
							float oppoHeight = m_generationSettings.m_minHeight;
							float nextHeight = m_generationSettings.m_minHeight;
							if (edge->m_prev->m_opposite) {
								prevHeight = edge->m_prev->m_opposite->m_owner->m_height;
							}
							if (edge->m_opposite) {
								oppoHeight = edge->m_opposite->m_owner->m_height;
							}
							if (edge->m_next->m_opposite) {
								nextHeight = edge->m_next->m_opposite->m_owner->m_height;
							}
							float startHeight = (prov->m_height + oppoHeight + prevHeight) / 3.f;
							float endHeight = (prov->m_height + oppoHeight + nextHeight) / 3.f; 
							if ((int)edge->m_noisyEdges.size() == 0) {
								Vec2 startPos = edge->m_startPos;
								Vec2 endPos = edge->m_endPos;
								PM_ClampLinesIntoBounds( startPos, endPos );
								vertsForEachCountry.emplace_back( Vec3( startPos, startHeight * factor ), Rgba8::WHITE );
								vertsForEachCountry.emplace_back( Vec3( endPos, endHeight * factor ), Rgba8::WHITE );
							}
							else {
								int noisyEdgeSize = (int)edge->m_noisyEdges.size();
								for (int k = 0; k < noisyEdgeSize - 1; k++) {
									float firstHeight = factor * Interpolate( startHeight, endHeight, (float)k / (float)(noisyEdgeSize - 1) );
									float secondHeight = factor * Interpolate( startHeight, endHeight, (float)(k + 1) / (float)(noisyEdgeSize - 1) );
									vertsForEachCountry.emplace_back( Vec3( edge->m_noisyEdges[k], firstHeight ), Rgba8::WHITE );
									vertsForEachCountry.emplace_back( Vec3( edge->m_noisyEdges[k + 1], secondHeight ), Rgba8::WHITE );
								}
							}
						}
						else{
							constexpr float factor = HEIGHT_FACTOR;

							float prevHeight = m_generationSettings.m_minHeight;
							float oppoHeight = m_generationSettings.m_minHeight;
							float nextHeight = m_generationSettings.m_minHeight;
							if (edge->m_prev->m_opposite) {
								prevHeight = edge->m_prev->m_opposite->m_owner->m_height;
							}
							if (edge->m_opposite) {
								oppoHeight = edge->m_opposite->m_owner->m_height;
							}
							if (edge->m_next->m_opposite) {
								nextHeight = edge->m_next->m_opposite->m_owner->m_height;
							}
							float startHeight = (prov->m_height + oppoHeight + prevHeight) / 3.f;
							float endHeight = (prov->m_height + oppoHeight + nextHeight) / 3.f; 
							if ((int)edge->m_noisyEdges.size() == 0) {
								Vec2 startPos = edge->m_startPos;
								Vec2 endPos = edge->m_endPos;
								PM_ClampLinesIntoBounds( startPos, endPos );
								verts.emplace_back( Vec3( startPos, startHeight * factor ), Rgba8( 20, 20, 20 ) );
								verts.emplace_back( Vec3( endPos, endHeight * factor ), Rgba8( 20, 20, 20 ) );
								vertsForEachCountry.emplace_back( Vec3( startPos, startHeight * factor ), Rgba8::WHITE );
								vertsForEachCountry.emplace_back( Vec3( endPos, endHeight * factor ), Rgba8::WHITE );
							}
							else {
								int noisyEdgeSize = (int)edge->m_noisyEdges.size();
								for (int k = 0; k < noisyEdgeSize - 1; k++) {
									float firstHeight = factor * Interpolate( startHeight, endHeight, (float)k / (float)(noisyEdgeSize - 1) );
									float secondHeight = factor * Interpolate( startHeight, endHeight, (float)(k + 1) / (float)(noisyEdgeSize - 1) );
									verts.emplace_back( Vec3( edge->m_noisyEdges[k], firstHeight ), Rgba8( 20, 20, 20 ) );
									verts.emplace_back( Vec3( edge->m_noisyEdges[k + 1], secondHeight ), Rgba8( 20, 20, 20 ) );
									vertsForEachCountry.emplace_back( Vec3( edge->m_noisyEdges[k], firstHeight ), Rgba8::WHITE );
									vertsForEachCountry.emplace_back( Vec3( edge->m_noisyEdges[k + 1], secondHeight ), Rgba8::WHITE );
								}
							}
						}
						
					}
				}
			}

		}
		if (vertsForEachCountry.size() > 0) {
			country->m_edgeShowingBuffer3D = g_theRenderer->CreateVertexBuffer( vertsForEachCountry.size() * sizeof( Vertex_PCU ) );
			g_theRenderer->CopyCPUToGPU( vertsForEachCountry.data(), vertsForEachCountry.size() * sizeof( Vertex_PCU ), country->m_edgeShowingBuffer3D );
			country->m_edgeShowingBuffer3D->SetAsLinePrimitive( true );
			vertsForEachCountry.clear();
		}
	}

	m_countriesEdgesVertexBuffer3D = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_countriesEdgesVertexBuffer3D );
	m_countriesEdgesVertexBuffer3D->SetAsLinePrimitive( true );
	verts.clear();

	// create region edges vertex buffers
	std::vector<Vertex_PCU> vertsForEachRegion;
	vertsForEachRegion.reserve( 1000 );
	for (auto region : m_regions) {
		for (auto prov : region->m_containedUnits) {
			for (auto edge : prov->m_edges) {
				// if this is the boarder of the region, then add verts to vector
				if (edge->m_opposite && edge->m_opposite->m_owner && edge->m_opposite->m_owner->m_region != region) {
					if (m_bounds.IsPointInside( edge->m_startPos ) || m_bounds.IsPointInside( edge->m_endPos )) {
						if ((int)edge->m_noisyEdges.size() == 0) {
							Vec2 startPos = edge->m_startPos;
							Vec2 endPos = edge->m_endPos;
							PM_ClampLinesIntoBounds( startPos, endPos );
							verts.emplace_back( startPos, Rgba8( 20, 20, 20 ) );
							verts.emplace_back( endPos, Rgba8( 20, 20, 20 ) );
							vertsForEachRegion.emplace_back( Vec3( startPos.x, startPos.y, 0.01f ), Rgba8::WHITE );
							vertsForEachRegion.emplace_back( Vec3( endPos.x, endPos.y, 0.01f ), Rgba8::WHITE );
						}
						else {
							for (int k = 0; k < (int)(edge->m_noisyEdges.size() - 1); k++) {
								verts.emplace_back( edge->m_noisyEdges[k], Rgba8( 20, 20, 20 ) );
								verts.emplace_back( edge->m_noisyEdges[k + 1], Rgba8( 20, 20, 20 ) );
								vertsForEachRegion.emplace_back( Vec3( edge->m_noisyEdges[k].x, edge->m_noisyEdges[k].y, 0.01f ), Rgba8::WHITE );
								vertsForEachRegion.emplace_back( Vec3( edge->m_noisyEdges[k + 1].x, edge->m_noisyEdges[k + 1].y, 0.01f ), Rgba8::WHITE );
							}
						}
					}
				}
			}

		}
		if (vertsForEachRegion.size() > 0) {
			region->m_edgeShowingBuffer = g_theRenderer->CreateVertexBuffer( vertsForEachRegion.size() * sizeof( Vertex_PCU ) );
			g_theRenderer->CopyCPUToGPU( vertsForEachRegion.data(), vertsForEachRegion.size() * sizeof( Vertex_PCU ), region->m_edgeShowingBuffer );
			region->m_edgeShowingBuffer->SetAsLinePrimitive( true );
			vertsForEachRegion.clear();
		}
	}

	verts.clear();
	// add roads verts
	for (auto road : m_roads) {
		for (auto const& curve : road->m_roadCurve) {
			curve.AddVertsForCurve2D( verts, 0.1f, Rgba8( 155, 155, 53 ), 64 );
		}
	}

	if ((int)verts.size() != 0) {
		m_roadVertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
		g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_roadVertexBuffer );
	}

	// add river verts
	std::vector<Vertex_PCU> riverVerts;
	for (auto river : m_rivers) {
		if (!river->m_isGarbage) {
			river->AddVertsForRiver( riverVerts, 0.1f, Rgba8( 0, 0, 255 ), 16 );
			//if (river->m_hasDelta && river->m_endUnit->IsOcean()) {
			//	AddVertsForAABB2D( riverVerts, AABB2( river->m_endUnit->m_geoCenterPos - Vec2( 1.f, 1.f ), river->m_endUnit->m_geoCenterPos + Vec2( 1.f, 1.f ) ), Rgba8( 255, 255, 255 ) );
			//}
		}
	}

	if ((int)riverVerts.size() != 0) {
		m_riverVertexBuffer = g_theRenderer->CreateVertexBuffer( riverVerts.size() * sizeof( Vertex_PCU ) );
		g_theRenderer->CopyCPUToGPU( riverVerts.data(), riverVerts.size() * sizeof( Vertex_PCU ), m_riverVertexBuffer );
	}

	verts.clear();

	AddVertsForSphere3D( verts, Vec3(  ), (m_dimensions.x + m_dimensions.y) * 0.5f, Rgba8::WHITE, AABB2::IDENTITY, 1000, 1000 );
	m_3DSphereVertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_3DSphereVertexBuffer );
}

void Map::InitializeLabels()
{
	for (auto label : m_cityLabels) {
		label->ReCalculateVertexData();
	}
	for (auto label : m_townLabels) {
		label->ReCalculateVertexData();
	}
	for (auto label : m_countryLabels) {
		label->ReCalculateVertexData();
	}
	for (auto label : m_cultureLabels) {
		label->ReCalculateVertexData();
	}
	for (auto label : m_religionLabels) {
		label->ReCalculateVertexData();
	}
	for (auto label : m_regionLabels) {
		label->ReCalculateVertexData();
	}
	for (auto label : m_continentLabels) {
		label->ReCalculateVertexData();
	}
}

void Map::HandleKeys()
{
	if (g_theGame->m_viewMode == MapViewMode::ViewMode2D) {
		Vec2 mousePos = g_theInput->GetCursorNormalizedPosition();
		Vec3 mouseWorldPos = g_theGame->m_worldCamera.PerspectiveScreenPosToWorld( mousePos );
		float zeroPlaneFraction = GetFractionWithinRange( 0.f, g_theGame->m_worldCamera.m_position.z, mouseWorldPos.z );
		Vec2 cursorWorldPos = Interpolate( g_theGame->m_worldCamera.m_position, mouseWorldPos, zeroPlaneFraction );
		if (!IsPointInsideAABB2D( cursorWorldPos, m_bounds )) {
			return;
		}

		m_hoveringProvince = GetUnitByPos( cursorWorldPos );
		if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_PROVINCE_EDIT) {
			float provEditUIWidth = (float)((double)m_diagonalLength / sqrt( (double)m_mapPolygonUnits.size() ) * 0.1);
			if (m_hoveringProvince && m_hoveringProvince->m_owner && m_curViewingUnit && m_curViewingUnit->m_owner && m_hoveringProvince->m_owner != m_curViewingUnit->m_owner
				&& IsPointInsideAABB2D( cursorWorldPos, AABB2(m_hoveringProvince->m_geoCenterPos - Vec2(provEditUIWidth, provEditUIWidth), m_hoveringProvince->m_geoCenterPos + Vec2(provEditUIWidth, provEditUIWidth)) )) {
				m_renderProvinceEditUIHighlight = true;
			}
			else {
				m_renderProvinceEditUIHighlight = false;
			}
		}

		if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE )) {
			if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_ARMIES) {
				m_curViewingArmy = GetArmyByPos( cursorWorldPos );
			}
			else {
				m_curViewingArmy = nullptr;
			}
			if (!m_curViewingArmy && (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CITIES)) {
				m_curViewingCity = GetCityByPos( cursorWorldPos );
			}
			else {
				m_curViewingCity = nullptr;
			}
			// armies have higher priority
			if (m_curViewingArmy) {
				m_showArmyPanel = true;
				if (m_curViewingUnit) {
					// if viewing a unit, disable it
					m_curViewingUnit->SetRenderViewingColor( false );
					m_polygonInspectionEdgeBlurringTimer->Stop();
					m_curViewingUnit = nullptr;
					m_showProvincePanel = false;
				}
				if (m_curViewingCity) {
					m_curViewingCity = nullptr;
					m_showCityPanel = false;
				}
			}
			// cities have higher priority
			else if (m_curViewingCity) {
				m_showCityPanel = true;
				if (m_curViewingUnit) {
					// if viewing a unit, disable it
					m_curViewingUnit->SetRenderViewingColor( false );
					m_polygonInspectionEdgeBlurringTimer->Stop();
					m_curViewingUnit = nullptr;
					m_showProvincePanel = false;
				}
			}
			else {
				// if do not click any city or army
				// if is in province owner edit mode
				if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_PROVINCE_EDIT) {
					float provEditUIWidth = (float)((double)m_diagonalLength / sqrt( (double)m_mapPolygonUnits.size() ) * 0.1);
					if (m_hoveringProvince && m_hoveringProvince->m_owner && m_curViewingUnit && m_curViewingUnit->m_owner && m_hoveringProvince->m_owner != m_curViewingUnit->m_owner
						&& IsPointInsideAABB2D( cursorWorldPos, AABB2( m_hoveringProvince->m_geoCenterPos - Vec2( provEditUIWidth, provEditUIWidth ), m_hoveringProvince->m_geoCenterPos + Vec2( provEditUIWidth, provEditUIWidth ) ) )) {
						m_hoveringProvince->m_owner->LoseProvince( m_hoveringProvince, true );
						m_curViewingUnit->m_owner->GainProvince( m_hoveringProvince );
						UpdateColorfulMaps();
						for (auto unit : m_mapPolygonUnits) {
							unit->SetRenderProvEditColor();
						}
						if (m_curViewingUnit) {
							m_curViewingUnit->SetRenderViewingColor( true );
						}
					}
					else {
						// not click in an add box
						if (m_curViewingUnit) {
							m_curViewingUnit->SetRenderViewingColor( false );
							m_polygonInspectionEdgeBlurringTimer->Stop();
						}
						m_curViewingUnit = m_hoveringProvince;
						if (m_curViewingUnit) {
							m_showProvincePanel = true;
							m_curViewingUnit->SetRenderViewingColor( true );
							m_polygonInspectionEdgeBlurringTimer->Start();
						}
					}
				}
				else {
					// if is in normal mode
					if (m_curViewingUnit) {
						m_curViewingUnit->SetRenderViewingColor( false );
						m_polygonInspectionEdgeBlurringTimer->Stop();
					}
					m_curViewingUnit = m_hoveringProvince;
					if (m_curViewingUnit) {
						m_showProvincePanel = true;
						m_curViewingUnit->SetRenderViewingColor( true );
						m_polygonInspectionEdgeBlurringTimer->Start();
					}
				}
			}
			if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELATION_MAP) {
				for (auto unit : m_mapPolygonUnits) {
					unit->SetRenderRelationColor();
				}
			}
			if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_PROVINCE_EDIT) {
				GenerateProvinceEditUI();
			}
		}
	}
	else if (g_theGame->m_viewMode == MapViewMode::ViewMode3D) {
		Vec2 mousePos = g_theInput->GetCursorNormalizedPosition();
		Vec3 mouseWorldPos = g_theGame->m_worldCamera.PerspectiveScreenPosToWorld( mousePos );
		float zeroPlaneFraction = GetFractionWithinRange( 0.f, g_theGame->m_worldCamera.m_position.z, mouseWorldPos.z );
		Vec2 cursorWorldPos = Interpolate( g_theGame->m_worldCamera.m_position, mouseWorldPos, zeroPlaneFraction );

		if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE )) {
			if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_ARMIES) {
				m_curViewingArmy = GetCurHoveringArmy3D( mouseWorldPos );
			}
			else {
				m_curViewingArmy = nullptr;
			}
			if (!m_curViewingArmy && (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CITIES)) {
				m_curViewingCity = GetCurHoveringCity3D( mouseWorldPos );
			}
			else {
				m_curViewingCity = nullptr;
			}
			// armies have higher priority
			if (m_curViewingArmy) {
				m_showArmyPanel = true;
				if (m_curViewingUnit) {
					// if viewing a unit, disable it
					m_curViewingUnit->SetRenderViewingColor( false );
					m_polygonInspectionEdgeBlurringTimer->Stop();
					m_curViewingUnit = nullptr;
					m_showProvincePanel = false;
				}
				if (m_curViewingCity) {
					m_curViewingCity = nullptr;
					m_showCityPanel = false;
				}
			}
			// cities have higher priority
			else if (m_curViewingCity) {
				m_showCityPanel = true;
				if (m_curViewingUnit) {
					// if viewing a unit, disable it
					m_curViewingUnit->SetRenderViewingColor( false );
					m_polygonInspectionEdgeBlurringTimer->Stop();
					m_curViewingUnit = nullptr;
					m_showProvincePanel = false;
				}
			}
			else {
				// if do not click any city or army
				if (m_curViewingUnit) {
					m_curViewingUnit->SetRenderViewingColor( false );
					m_polygonInspectionEdgeBlurringTimer->Stop();
				}
				m_curViewingUnit = GetCurHoveringAtUnit3D( mouseWorldPos );
				if (m_curViewingUnit) {
					m_showProvincePanel = true;
					m_curViewingUnit->SetRenderViewingColor( true );
					m_polygonInspectionEdgeBlurringTimer->Start();
				}
			}
			if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELATION_MAP) {
				for (auto unit : m_mapPolygonUnits) {
					unit->SetRenderRelationColor();
				}
			}
		}
	}

	// step test culture spread
	/*static int step = 0;
	if (g_theInput->WasKeyJustPressed( 'H' )) {
		for (auto unit : m_mapPolygonUnits) {
			if (!unit->m_isFarAwayFakeUnit && unit->m_cultureStepValue == -1) {
				bool uninitializedUnit = unit->m_majorCulture ? false : true;
				bool addInfluence = false;

				for (auto adjUnit : unit->m_adjacentUnits) {
					if (!adjUnit->m_isFarAwayFakeUnit && adjUnit->m_majorCulture && adjUnit->m_cultureStepValue + 1 == step) {
						//if (unit->HD_GetCultureInfluence( adjUnit->m_majorCulture ) == 0.f) {
							unit->HD_AddCultureInfluence( adjUnit->m_majorCulture );
							addInfluence = true;
						//}
						unit->m_cultureStepValue = step;
					}
				}
				if (addInfluence && uninitializedUnit) {
				}
			}
		}
		++step;		
		for (auto unit : m_mapPolygonUnits) {
			unit->SetRenderCultureColor();
		}
	}*/
}

void Map::UpdateLabels()
{
	for (auto label : m_cityLabels) {
		label->UpdateColor();
	}
	for (auto label : m_countryLabels) {
		label->UpdateColor();
	}
	for (auto label : m_townLabels) {
		label->UpdateColor();
	}
	for (auto label : m_cultureLabels) {
		label->UpdateColor();
	}
	for (auto label : m_religionLabels) {
		label->UpdateColor();
	}
}

void Map::UpdateEdges()
{
	// make the timer loop
	m_polygonInspectionEdgeBlurringTimer->DecrementPeriodIfElapsed();
}

void Map::UpdateSwitching2D3D()
{
	// switching between 2D and 3D
	if (m_2D3DSwitchTimer->HasStartedAndNotPeriodElapsed()) {
		if (g_theGame->m_viewMode == MapViewMode::ViewMode2D) {
			g_theGame->m_worldCamera.m_orientation = EulerAngles( Interpolate( m_switchStartYaw, 90.f, SmoothStop2( m_2D3DSwitchTimer->GetElapsedFraction() ) ), Interpolate( m_switchStartPitch, 90.f, SmoothStop2( m_2D3DSwitchTimer->GetElapsedFraction() ) ), 0.f );
			g_theGame->m_worldCamera.m_position = Interpolate( m_viewSwitchStartPosition, m_viewSwitchTargetPosition, SmoothStop2( m_2D3DSwitchTimer->GetElapsedFraction() ) );

		}
		else if (g_theGame->m_viewMode == MapViewMode::ViewMode3D) {
			g_theGame->m_worldCamera.m_orientation = EulerAngles( 90.f, Interpolate( 90.f, 60.f, SmoothStop2( m_2D3DSwitchTimer->GetElapsedFraction() ) ), 0.f );
			g_theGame->m_worldCamera.m_position = Interpolate( m_viewSwitchStartPosition, m_viewSwitchTargetPosition, SmoothStop2( m_2D3DSwitchTimer->GetElapsedFraction() ) );
		}
		else if (g_theGame->m_viewMode == MapViewMode::ViewModeSphere) {
			g_theGame->m_worldCamera.m_orientation = EulerAngles( Interpolate( m_switchStartYaw, 90.f, SmoothStop2( m_2D3DSwitchTimer->GetElapsedFraction() ) ), Interpolate( m_switchStartPitch, 90.f, SmoothStop2( m_2D3DSwitchTimer->GetElapsedFraction() ) ), 0.f );
			g_theGame->m_worldCamera.m_position = Interpolate( m_viewSwitchStartPosition, m_viewSwitchTargetPosition, SmoothStop2( m_2D3DSwitchTimer->GetElapsedFraction() ) );
		}
	}
	if (m_2D3DSwitchTimer->HasPeriodElapsed()) {
		if (g_theGame->m_viewMode == MapViewMode::ViewMode2D || g_theGame->m_viewMode == MapViewMode::ViewModeSphere) {
			m_2D3DSwitchTimer->Stop();
			g_theGame->m_worldCamera.m_orientation = EulerAngles( 90.f, 90.f, 0.f );
			g_theGame->m_worldCameraScale = g_theGame->m_worldCamera.m_position.z;
		}
		else if (g_theGame->m_viewMode == MapViewMode::ViewMode3D) {
			m_2D3DSwitchTimer->Stop();
			g_theGame->m_worldCamera.m_orientation = EulerAngles( 90.f, 60.f, 0.f );
			g_theGame->m_worldCamera.m_position = m_viewSwitchTargetPosition;
		}
	}
}

void Map::UpdateAutoSimulation()
{
	if (m_autoRunSimulation) {
#ifdef DEBUG_COMPARE_HISTORY
		ViewLastMonth();
#endif
		SimulateNextMonth();
	}
}

void Map::UpdateBufferColors()
{
	g_theRenderer->CopyCPUToGPU( m_colorVertexArray.data(), m_colorVertexArray.size() * sizeof( Rgba8 ), m_polygonFacesColorVertexBuffer );
}

void Map::UpdateSaveStatus()
{
	if (m_saveHistoryJob && g_theJobSystem->RetrieveJob( m_saveHistoryJob )) {
		delete m_saveHistoryJob;
		m_saveHistoryJob = nullptr;
	}
}

void Map::Render2D() const
{
	// render base polygons and colors
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_COUNTRIES_MAP
		|| m_showingSettings.m_mapShowConfig & SHOW_CONFIG_REGIONS_MAP
		|| m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELIGION_MAP
		|| m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CULTURE_MAP
		|| m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CONTINENT_MAP
		) {
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Manila Paper.png" ) );
	}
	else {
		g_theRenderer->BindTexture( nullptr );
	}
	g_theRenderer->BindShader( m_polygonFaceShader );
	g_theRenderer->SetModelConstants();
	VertexBuffer* buffers[] = { m_polygonsFacesVertexBuffer, m_polygonFacesColorVertexBuffer, m_polygonuvVertexBuffer };
	g_theRenderer->DrawVertexBuffers( 3, buffers, m_polygonsFacesVertexBuffer->GetVertexCount(), 0 );

	// render rivers
	if ((m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RIVERS) && m_riverVertexBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexBuffer( m_riverVertexBuffer, m_riverVertexBuffer->GetVertexCount() );
	}
	// render roads
	if ((m_showingSettings.m_mapShowConfig & SHOW_CONFIG_ROADS) && m_roadVertexBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexBuffer( m_roadVertexBuffer, m_roadVertexBuffer->GetVertexCount() );
	}
// 	// render culture origins
// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CULTURE_MAP) {
// 		std::vector<Vertex_PCU> cultureCenterVerts;
// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
// 		for (int i = 0; i < (int)m_cultures.size(); i++) {
// 			AddVertsForAABB2D( cultureCenterVerts, AABB2( m_cultures[i]->m_cultureOriginUnit->m_geoCenterPos - Vec2( 0.3f, 0.3f ), m_cultures[i]->m_cultureOriginUnit->m_geoCenterPos + Vec2( 0.3f, 0.3f ) ), Rgba8( 255, 255, 255 ) );
// 		}
// 		g_theRenderer->BindTexture( nullptr );
// 		g_theRenderer->BindShader( nullptr );
// 		g_theRenderer->SetModelConstants();
// 		g_theRenderer->DrawVertexArray( cultureCenterVerts );
// 	}
// 	// render religion origins
// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELIGION_MAP) {
// 		std::vector<Vertex_PCU> religionCenterVerts;
// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
// 		for (int i = 0; i < (int)m_religions.size(); i++) {
// 			AddVertsForAABB2D( religionCenterVerts, AABB2( m_religions[i]->m_religionOriginUnit->m_geoCenterPos - Vec2( 0.3f, 0.3f ), m_religions[i]->m_religionOriginUnit->m_geoCenterPos + Vec2( 0.3f, 0.3f ) ), Rgba8( 255, 255, 255 ) );
// 		}
// 		g_theRenderer->BindTexture( nullptr );
// 		g_theRenderer->BindShader( nullptr );
// 		g_theRenderer->SetModelConstants();
// 		g_theRenderer->DrawVertexArray( religionCenterVerts );
// 	}
	// render polygon edges
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_POLYGON_EDGES) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexIndexed( m_polygonsEdgesVertexBuffer, m_polygonEdgesIndexBuffer, m_polygonEdgesIndexBuffer->GetIndexCount(), 0 );
	}
	// render country edges
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_COUNTRIES_EDGES) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexBuffer( m_countriesEdgesVertexBuffer, m_countriesEdgesVertexBuffer->GetVertexCount() );
	}
	// render towns
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_TOWNS) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexBuffer( m_townVertexBuffer, m_townVertexBuffer->GetVertexCount() );
		if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS) {
			for (auto label : m_townLabels) {
				label->Render();
			}
		}
	}
	// render cities
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CITIES) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexBuffer( m_cityVertexBuffer, m_cityVertexBuffer->GetVertexCount() );
		if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS) {
			for (auto label : m_cityLabels) {
				label->Render();
			}
		}
	}
	// render armies
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_ARMIES) {
		std::vector<Vertex_PCU> armiesVerts;
		armiesVerts.reserve( 10000 );
		for (auto country : m_countries) {
			for (auto army : country->m_armies) {
				Vec2 armyPos = army->GetPosition();
				AddVertsForDisc2D( armiesVerts, armyPos, army->GetOuterRadius(), Rgba8( 255, 255, 255, 150 ) );
				AddVertsForDisc2D( armiesVerts, armyPos, army->GetInnerRadius(),
					Rgba8( (unsigned char)std::max( army->m_owner->m_color.r - 50, 0 ),
						(unsigned char)std::max( army->m_owner->m_color.g - 50, 0 ),
						(unsigned char)std::max( army->m_owner->m_color.b - 50, 0 ),
						army->m_owner->m_color.a ) );
			}
		}
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( armiesVerts );
	}
	// render blurring inspecting edges
	if (m_viewingCountry && m_curViewingUnit && m_curViewingUnit->m_owner && m_curViewingUnit->m_owner->m_edgeShowingBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants( Mat44(), Rgba8( 255, 255, 190, (unsigned char)(255 * SmoothStop3( 1.f - 2.f * abs( m_polygonInspectionEdgeBlurringTimer->GetElapsedFraction() - 0.5f ) )) ) );
		g_theRenderer->DrawVertexBuffer( m_curViewingUnit->m_owner->m_edgeShowingBuffer, m_curViewingUnit->m_owner->m_edgeShowingBuffer->GetVertexCount() );
	}
	if (m_viewingRegion && m_curViewingUnit && m_curViewingUnit->m_region && m_curViewingUnit->m_region->m_edgeShowingBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants( Mat44(), Rgba8( 255, 255, 190, (unsigned char)(255 * SmoothStop3( 1.f - 2.f * abs( m_polygonInspectionEdgeBlurringTimer->GetElapsedFraction() - 0.5f ) )) ) );
		g_theRenderer->DrawVertexBuffer( m_curViewingUnit->m_region->m_edgeShowingBuffer, m_curViewingUnit->m_region->m_edgeShowingBuffer->GetVertexCount() );
	}
	if (m_curViewingUnit && m_curViewingUnit->m_edgeShowingBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants( Mat44(), Rgba8( 255, 255, 255, (unsigned char)(255 * SmoothStop3( 1.f - 2.f * abs( m_polygonInspectionEdgeBlurringTimer->GetElapsedFraction() - 0.5f ) )) ) );
		g_theRenderer->DrawVertexBuffer( m_curViewingUnit->m_edgeShowingBuffer, m_curViewingUnit->m_edgeShowingBuffer->GetVertexCount() );
	}

	// render culture labels
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CULTURE_MAP) {
		for (auto label : m_cultureLabels) {
			label->Render();
		}
	}
	// render religion labels
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELIGION_MAP) {
		for (auto label : m_religionLabels) {
			label->Render();
		}
	}
	// render country labels
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_COUNTRIES_MAP) {
		for (auto label : m_countryLabels) {

			label->Render();
		}
	}
	// render region labels
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_REGIONS_MAP) {
		for (auto label : m_regionLabels) {
			label->Render();
		}
	}
	// render continent labels
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CONTINENT_MAP) {
		for (auto label : m_continentLabels) {
			label->Render();
		}
	}

	// DEBUG render polygon center
	if (m_showingSettings.m_debugShowPolygonCenter) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexBuffer( m_polygonCenterVertexBuffer, m_polygonCenterVertexBuffer->GetVertexCount() );
	}
	// DEBUG render river start
	if (m_showingSettings.m_debugShowRiverStart) {
		float radiusOfSite = 10.f * Minf( m_generationSettings.m_dimensions.x, m_generationSettings.m_dimensions.y ) / m_generationSettings.m_basePolygons;
		std::vector<Vertex_PCU> riverStartVerts;
		for (int i = 0; i < (int)m_rivers.size(); i++) {
			AddVertsForAABB2D( riverStartVerts, AABB2( m_rivers[i]->m_anchorPoint[0] - Vec2( radiusOfSite, radiusOfSite ), m_rivers[i]->m_anchorPoint[0] + Vec2( radiusOfSite, radiusOfSite ) ), Rgba8( 0, 0, 255 ) );
		}
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( riverStartVerts );
	}
	// DEBUG render nearest sea directions
	if (m_showingSettings.m_debugShowNearestOceanUnit) {
		std::vector<Vertex_PCU> nearestSea;
		for (auto unit : m_mapPolygonUnits) {
			if (!unit->m_isFarAwayFakeUnit) {
				if (unit->m_nearestSeaDir == Direction::East) {
					AddVertsForLineSegment2D( nearestSea, unit->m_centerPosition, unit->m_centerPosition + m_diagonalLength * Vec2( unit->m_nearestSeaDistance, 0.f ), 0.1f, Rgba8( 102, 255, 255 ) );
				}
				else if (unit->m_nearestSeaDir == Direction::West) {
					AddVertsForLineSegment2D( nearestSea, unit->m_centerPosition, unit->m_centerPosition + m_diagonalLength * Vec2( -unit->m_nearestSeaDistance, 0.f ), 0.1f, Rgba8( 102, 255, 255 ) );
				}
				else if (unit->m_nearestSeaDir == Direction::Middle) {
					// do nothing
				}
			}
		}
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( nearestSea );
	}
	// debug show country start province
	if ((m_showingSettings.m_mapShowConfig & SHOW_CONFIG_COUNTRIES_MAP) && m_showingSettings.m_debugShowCountryStart) {
		std::vector<Vertex_PCU> countryStart;
		for (auto country : m_countries) {
			AddVertsForDisc2D( countryStart, country->m_originProv->m_geoCenterPos, 0.3f, Rgba8::WHITE );
		}
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray( countryStart );
	}

	// render 2D edges bounds
	if (g_theGame->m_viewMode == MapViewMode::ViewMode2D) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexBuffer( m_edgeBlockVertexBuffer, m_edgeBlockVertexBuffer->GetVertexCount() );
	}

	// render map boarders
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Photograph Frame.png" ) );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexBuffer( m_mapBorderVertexBuffer, m_mapBorderVertexBuffer->GetVertexCount() );

	// ToDo: change debug draw to vertex buffer
	if (m_curViewingCity) {
		DebugDrawRing( m_curViewingCity->m_position, CITY_ICON_SIDE_LENGTH * 2.f, 0.05f, Rgba8( 255, 0, 0 ) );
	}
	if (m_curViewingArmy) {
		DebugDrawRing( m_curViewingArmy->GetPosition(), CITY_ICON_SIDE_LENGTH * 2.f, 0.05f, Rgba8( 255, 0, 0 ) );
	}

	// render prov edit UI
	if ((m_showingSettings.m_mapShowConfig & SHOW_CONFIG_PROVINCE_EDIT) && m_provEditVertexBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexBuffer( m_provEditVertexBuffer, m_provEditVertexBuffer->GetVertexCount() );
	}

	if ((m_showingSettings.m_mapShowConfig & SHOW_CONFIG_PROVINCE_EDIT) && m_provEditVertexBuffer && m_renderProvinceEditUIHighlight && m_hoveringProvince) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants( Mat44::CreateTranslation2D( m_hoveringProvince->m_geoCenterPos ) );
		g_theRenderer->DrawVertexBuffer( m_provEditHighLightVertexBuffer, m_provEditHighLightVertexBuffer->GetVertexCount() );

	}
}

void Map::Prepare2DMapFor3D() const
{
	g_theRenderer->SetScreenRenderTargetView(g_theGame->m_worldCamera);
	Mat44 modelMatrix = Mat44::CreateTranslation2D( m_dimensions );
	// render rivers
	if ((m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RIVERS) && m_riverVertexBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexBuffer( m_riverVertexBuffer, m_riverVertexBuffer->GetVertexCount() );
	}
	// render roads
	if ((m_showingSettings.m_mapShowConfig & SHOW_CONFIG_ROADS) && m_roadVertexBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexBuffer( m_roadVertexBuffer, m_roadVertexBuffer->GetVertexCount() );
	}
	// 	// render culture origins
	// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CULTURE_MAP) {
	// 		std::vector<Vertex_PCU> cultureCenterVerts;
	// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	// 		for (int i = 0; i < (int)m_cultures.size(); i++) {
	// 			AddVertsForAABB2D( cultureCenterVerts, AABB2( m_cultures[i]->m_cultureOriginUnit->m_geoCenterPos - Vec2( 0.3f, 0.3f ), m_cultures[i]->m_cultureOriginUnit->m_geoCenterPos + Vec2( 0.3f, 0.3f ) ), Rgba8( 255, 255, 255 ) );
	// 		}
	// 		g_theRenderer->BindTexture( nullptr );
	// 		g_theRenderer->BindShader( nullptr );
	// 		g_theRenderer->SetModelConstants();
	// 		g_theRenderer->DrawVertexArray( cultureCenterVerts );
	// 	}
	// 	// render religion origins
	// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELIGION_MAP) {
	// 		std::vector<Vertex_PCU> religionCenterVerts;
	// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	// 		for (int i = 0; i < (int)m_religions.size(); i++) {
	// 			AddVertsForAABB2D( religionCenterVerts, AABB2( m_religions[i]->m_religionOriginUnit->m_geoCenterPos - Vec2( 0.3f, 0.3f ), m_religions[i]->m_religionOriginUnit->m_geoCenterPos + Vec2( 0.3f, 0.3f ) ), Rgba8( 255, 255, 255 ) );
	// 		}
	// 		g_theRenderer->BindTexture( nullptr );
	// 		g_theRenderer->BindShader( nullptr );
	// 		g_theRenderer->SetModelConstants();
	// 		g_theRenderer->DrawVertexArray( religionCenterVerts );
	// 	}
	// render towns
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_TOWNS) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexBuffer( m_townVertexBuffer, m_townVertexBuffer->GetVertexCount() );
		if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS) {
			for (auto label : m_townLabels) {
				label->Render(m_2DTextureShader, modelMatrix);
			}
		}
	}
	// render cities
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CITIES) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexBuffer( m_cityVertexBuffer, m_cityVertexBuffer->GetVertexCount() );
		if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS) {
			for (auto label : m_cityLabels) {
				label->Render(m_2DTextureShader, modelMatrix);
			}
		}
	}
	// render armies
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_ARMIES) {
		std::vector<Vertex_PCU> armiesVerts;
		armiesVerts.reserve( 10000 );
		for (auto country : m_countries) {
			for (auto army : country->m_armies) {
				Vec2 armyPos = army->GetPosition();
				AddVertsForDisc2D( armiesVerts, armyPos, army->GetOuterRadius(), Rgba8( 255, 255, 255, 150 ) );
				AddVertsForDisc2D( armiesVerts, armyPos, army->GetInnerRadius(),
					Rgba8( (unsigned char)std::max( army->m_owner->m_color.r - 50, 0 ),
						(unsigned char)std::max( army->m_owner->m_color.g - 50, 0 ),
						(unsigned char)std::max( army->m_owner->m_color.b - 50, 0 ),
						army->m_owner->m_color.a ) );
			}
		}
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexArray( armiesVerts );
	}

	// render culture labels
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CULTURE_MAP) {
		for (auto label : m_cultureLabels) {
			label->Render(m_2DTextureShader, modelMatrix);
		}
	}
	// render religion labels
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELIGION_MAP) {
		for (auto label : m_religionLabels) {
			label->Render(m_2DTextureShader, modelMatrix);
		}
	}
	// render country labels
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_COUNTRIES_MAP) {
		for (auto label : m_countryLabels) {
			label->Render(m_2DTextureShader, modelMatrix);
		}
	}
	// render region labels
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_REGIONS_MAP) {
		for (auto label : m_regionLabels) {
			label->Render(m_2DTextureShader, modelMatrix);
		}
	}
	// render continent labels
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CONTINENT_MAP) {
		for (auto label : m_continentLabels) {
			label->Render(m_2DTextureShader, modelMatrix);
		}
	}

	// DEBUG render polygon center
	if (m_showingSettings.m_debugShowPolygonCenter) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexBuffer( m_polygonCenterVertexBuffer, m_polygonCenterVertexBuffer->GetVertexCount() );
	}
	// DEBUG render river start
	if (m_showingSettings.m_debugShowRiverStart) {
		float radiusOfSite = 10.f * Minf( m_generationSettings.m_dimensions.x, m_generationSettings.m_dimensions.y ) / m_generationSettings.m_basePolygons;
		std::vector<Vertex_PCU> riverStartVerts;
		for (int i = 0; i < (int)m_rivers.size(); i++) {
			AddVertsForAABB2D( riverStartVerts, AABB2( m_rivers[i]->m_anchorPoint[0] - Vec2( radiusOfSite, radiusOfSite ), m_rivers[i]->m_anchorPoint[0] + Vec2( radiusOfSite, radiusOfSite ) ), Rgba8( 0, 0, 255 ) );
		}
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexArray( riverStartVerts );
	}
	// DEBUG render nearest sea directions
	if (m_showingSettings.m_debugShowNearestOceanUnit) {
		std::vector<Vertex_PCU> nearestSea;
		for (auto unit : m_mapPolygonUnits) {
			if (!unit->m_isFarAwayFakeUnit) {
				if (unit->m_nearestSeaDir == Direction::East) {
					AddVertsForLineSegment2D( nearestSea, unit->m_centerPosition, unit->m_centerPosition + m_diagonalLength * Vec2( unit->m_nearestSeaDistance, 0.f ), 0.1f, Rgba8( 102, 255, 255 ) );
				}
				else if (unit->m_nearestSeaDir == Direction::West) {
					AddVertsForLineSegment2D( nearestSea, unit->m_centerPosition, unit->m_centerPosition + m_diagonalLength * Vec2( -unit->m_nearestSeaDistance, 0.f ), 0.1f, Rgba8( 102, 255, 255 ) );
				}
				else if (unit->m_nearestSeaDir == Direction::Middle) {
					// do nothing
				}
			}
		}
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexArray( nearestSea );
	}
	// debug show country start province
	if ((m_showingSettings.m_mapShowConfig & SHOW_CONFIG_COUNTRIES_MAP) && m_showingSettings.m_debugShowCountryStart) {
		std::vector<Vertex_PCU> countryStart;
		for (auto country : m_countries) {
			AddVertsForDisc2D( countryStart, country->m_originProv->m_geoCenterPos, 0.3f, Rgba8::WHITE );
		}
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexArray( countryStart );
	}

	// render 2D edges bounds
	if (g_theGame->m_viewMode == MapViewMode::ViewMode2D) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexBuffer( m_edgeBlockVertexBuffer, m_edgeBlockVertexBuffer->GetVertexCount() );
	}

	// render map boarders
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Photograph Frame.png" ) );
	g_theRenderer->BindShader( m_2DTextureShader );
	g_theRenderer->SetModelConstants( modelMatrix );
	g_theRenderer->DrawVertexBuffer( m_mapBorderVertexBuffer, m_mapBorderVertexBuffer->GetVertexCount() );

	g_theRenderer->ResetScreenRenderTargetView();
	// ToDo: change debug draw to vertex buffer
// 	if (m_curViewingCity) {
// 		DebugDrawRing( m_curViewingCity->m_position, CITY_ICON_SIDE_LENGTH * 2.f, 0.05f, Rgba8( 255, 0, 0 ) );
// 	}
// 	if (m_curViewingArmy) {
// 		DebugDrawRing( m_curViewingArmy->GetPosition(), CITY_ICON_SIDE_LENGTH * 2.f, 0.05f, Rgba8( 255, 0, 0 ) );
// 	}
}

void Map::Render3D() const
{
	g_theRenderer->SetBasicRenderTargetView();
	// render base polygons and colors
	g_theRenderer->BindShader( m_3DShader );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( g_theRenderer->GetCurScreenAsTexture() );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	VertexBuffer* buffers[] = { m_polygonsFaces3DVertexBuffer, m_polygonFacesColorVertexBuffer, m_polygonuv3DVertexBuffer, m_polygonsNormalVertexBuffer };
	g_theRenderer->DrawVertexBuffers( 4, buffers, m_polygonsFaces3DVertexBuffer->GetVertexCount(), 0 );

// 	// render rivers
// 	if ((m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RIVERS) && m_riverVertexBuffer) {
// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
// 		g_theRenderer->BindTexture( nullptr );
// 		g_theRenderer->BindShader( nullptr );
// 		g_theRenderer->SetModelConstants();
// 		g_theRenderer->DrawVertexBuffer( m_riverVertexBuffer, m_riverVertexBuffer->GetVertexCount() );
// 	}
// 	// render roads
// 	if ((m_showingSettings.m_mapShowConfig & SHOW_CONFIG_ROADS) && m_roadVertexBuffer) {
// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
// 		g_theRenderer->BindTexture( nullptr );
// 		g_theRenderer->BindShader( nullptr );
// 		g_theRenderer->SetModelConstants();
// 		g_theRenderer->DrawVertexBuffer( m_roadVertexBuffer, m_roadVertexBuffer->GetVertexCount() );
// 	}
// 	// render culture origins
// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CULTURE_MAP) {
// 		std::vector<Vertex_PCU> cultureCenterVerts;
// 		for (int i = 0; i < (int)m_cultures.size(); i++) {
// 			AddVertsForAABB2D( cultureCenterVerts, AABB2( m_cultures[i]->m_cultureOriginUnit->m_geoCenterPos - Vec2( 0.3f, 0.3f ), m_cultures[i]->m_cultureOriginUnit->m_geoCenterPos + Vec2( 0.3f, 0.3f ) ), Rgba8( 255, 255, 255 ) );
// 		}
// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
// 		g_theRenderer->BindTexture( nullptr );
// 		g_theRenderer->BindShader( nullptr );
// 		g_theRenderer->SetModelConstants();
// 		g_theRenderer->DrawVertexArray( cultureCenterVerts );
// 	}
// 	// render religion origins
// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELIGION_MAP) {
// 		std::vector<Vertex_PCU> religionCenterVerts;
// 		for (int i = 0; i < (int)m_religions.size(); i++) {
// 			AddVertsForAABB2D( religionCenterVerts, AABB2( m_religions[i]->m_religionOriginUnit->m_geoCenterPos - Vec2( 0.3f, 0.3f ), m_religions[i]->m_religionOriginUnit->m_geoCenterPos + Vec2( 0.3f, 0.3f ) ), Rgba8( 255, 255, 255 ) );
// 		}
// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
// 		g_theRenderer->BindTexture( nullptr );
// 		g_theRenderer->BindShader( nullptr );
// 		g_theRenderer->SetModelConstants();
// 		g_theRenderer->DrawVertexArray( religionCenterVerts );
// 	}
	// render polygon edges
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_POLYGON_EDGES) {
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->DrawVertexIndexed( m_polygonsEdgesVertexBuffer3D, m_polygonEdgesIndexBuffer3D, m_polygonEdgesIndexBuffer3D->GetIndexCount(), 0 );

	}
	// render country edges
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_COUNTRIES_EDGES) {
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->DrawVertexBuffer( m_countriesEdgesVertexBuffer3D, m_countriesEdgesVertexBuffer3D->GetVertexCount() );
	}
// 	// render towns
// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_TOWNS) {
// 		g_theRenderer->BindTexture( nullptr );
// 		g_theRenderer->BindShader( nullptr );
// 		g_theRenderer->SetModelConstants();
// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
// 		g_theRenderer->DrawVertexBuffer( m_townVertexBuffer, m_townVertexBuffer->GetVertexCount() );
// 		if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS) {
// 			for (auto label : m_townLabels) {
// 				label->Render();
// 			}
// 		}
// 	}
// 	// render cities
// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CITIES) {
// 		g_theRenderer->BindTexture( nullptr );
// 		g_theRenderer->BindShader( nullptr );
// 		g_theRenderer->SetModelConstants();
// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
// 		g_theRenderer->DrawVertexBuffer( m_cityVertexBuffer, m_cityVertexBuffer->GetVertexCount() );
// 		if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS) {
// 			for (auto label : m_cityLabels) {
// 				label->Render();
// 			}
// 		}
// 	}
// 	// render armies
// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_ARMIES) {
// 		std::vector<Vertex_PCU> armiesVerts;
// 		armiesVerts.reserve( 10000 );
// 		for (auto country : m_countries) {
// 			for (auto army : country->m_armies) {
// 				Vec2 armyPos = army->GetPosition();
// 				AddVertsForDisc2D( armiesVerts, armyPos, army->GetOuterRadius(), Rgba8( 255, 255, 255, 150 ) );
// 				AddVertsForDisc2D( armiesVerts, armyPos, army->GetInnerRadius(),
// 					Rgba8( (unsigned char)std::max( army->m_owner->m_color.r - 50, 0 ),
// 						(unsigned char)std::max( army->m_owner->m_color.g - 50, 0 ),
// 						(unsigned char)std::max( army->m_owner->m_color.b - 50, 0 ),
// 						army->m_owner->m_color.a ) );
// 			}
// 		}
// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
// 		g_theRenderer->BindTexture( nullptr );
// 		g_theRenderer->BindShader( nullptr );
// 		g_theRenderer->SetModelConstants();
// 		g_theRenderer->DrawVertexArray( armiesVerts );
// 	}
	// render blurring inspecting edges
	if (m_viewingCountry && m_curViewingUnit && m_curViewingUnit->m_owner && m_curViewingUnit->m_owner->m_edgeShowingBuffer) {
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetModelConstants( Mat44(), Rgba8( 255, 255, 190, (unsigned char)(255 * SmoothStop3( 1.f - 2.f * abs( m_polygonInspectionEdgeBlurringTimer->GetElapsedFraction() - 0.5f ) )) ) );
		g_theRenderer->DrawVertexBuffer( m_curViewingUnit->m_owner->m_edgeShowingBuffer3D, m_curViewingUnit->m_owner->m_edgeShowingBuffer3D->GetVertexCount() );
	}
	if (m_viewingRegion && m_curViewingUnit && m_curViewingUnit->m_region && m_curViewingUnit->m_region->m_edgeShowingBuffer) {
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetModelConstants( Mat44(), Rgba8( 255, 255, 190, (unsigned char)(255 * SmoothStop3( 1.f - 2.f * abs( m_polygonInspectionEdgeBlurringTimer->GetElapsedFraction() - 0.5f ) )) ) );
		g_theRenderer->DrawVertexBuffer( m_curViewingUnit->m_region->m_edgeShowingBuffer, m_curViewingUnit->m_region->m_edgeShowingBuffer->GetVertexCount() );
	}
	if (m_curViewingUnit && m_curViewingUnit->m_edgeShowingBuffer) {
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetModelConstants( Mat44(), Rgba8( 255, 255, 255, (unsigned char)(255 * SmoothStop3( 1.f - 2.f * abs( m_polygonInspectionEdgeBlurringTimer->GetElapsedFraction() - 0.5f ) )) ) );
		g_theRenderer->DrawVertexBuffer( m_curViewingUnit->m_edgeShowingBuffer3D, m_curViewingUnit->m_edgeShowingBuffer3D->GetVertexCount() );
	}

// 	// render culture labels
// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CULTURE_MAP) {
// 		for (auto label : m_cultureLabels) {
// 			label->Render();
// 		}
// 	}
// 	// render religion labels
// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELIGION_MAP) {
// 		for (auto label : m_religionLabels) {
// 			label->Render();
// 		}
// 	}
// 	// render country labels
// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_COUNTRIES_MAP) {
// 		for (auto label : m_countryLabels) {
// 			label->Render();
// 		}
// 	}
// 	// render region labels
// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_REGIONS_MAP) {
// 		for (auto label : m_regionLabels) {
// 			label->Render();
// 		}
// 	}
// 	// render continent labels
// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CONTINENT_MAP) {
// 		for (auto label : m_continentLabels) {
// 			label->Render();
// 		}
// 	}
// 
// 	// DEBUG render polygon center
// 	if (m_showingSettings.m_debugShowPolygonCenter) {
// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
// 		g_theRenderer->BindTexture( nullptr );
// 		g_theRenderer->BindShader( nullptr );
// 		g_theRenderer->SetModelConstants();
// 		g_theRenderer->DrawVertexBuffer( m_polygonCenterVertexBuffer, m_polygonCenterVertexBuffer->GetVertexCount() );
// 	}
// 	// DEBUG render river start
// 	if (m_showingSettings.m_debugShowRiverStart) {
// 		float radiusOfSite = 10.f * Minf( m_generationSettings.m_dimensions.x, m_generationSettings.m_dimensions.y ) / m_generationSettings.m_basePolygons;
// 		std::vector<Vertex_PCU> riverStartVerts;
// 		for (int i = 0; i < (int)m_rivers.size(); i++) {
// 			AddVertsForAABB2D( riverStartVerts, AABB2( m_rivers[i]->m_anchorPoint[0] - Vec2( radiusOfSite, radiusOfSite ), m_rivers[i]->m_anchorPoint[0] + Vec2( radiusOfSite, radiusOfSite ) ), Rgba8( 0, 0, 255 ) );
// 		}
// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
// 		g_theRenderer->BindTexture( nullptr );
// 		g_theRenderer->BindShader( nullptr );
// 		g_theRenderer->SetModelConstants();
// 		g_theRenderer->DrawVertexArray( riverStartVerts );
// 	}
// 	// DEBUG render nearest sea directions
// 	if (m_showingSettings.m_debugShowNearestOceanUnit) {
// 		std::vector<Vertex_PCU> nearestSea;
// 		for (auto unit : m_mapPolygonUnits) {
// 			if (!unit->m_isFarAwayFakeUnit) {
// 				if (unit->m_nearestSeaDir == Direction::East) {
// 					AddVertsForLineSegment2D( nearestSea, unit->m_centerPosition, unit->m_centerPosition + m_diagonalLength * Vec2( unit->m_nearestSeaDistance, 0.f ), 0.1f, Rgba8( 102, 255, 255 ) );
// 				}
// 				else if (unit->m_nearestSeaDir == Direction::West) {
// 					AddVertsForLineSegment2D( nearestSea, unit->m_centerPosition, unit->m_centerPosition + m_diagonalLength * Vec2( -unit->m_nearestSeaDistance, 0.f ), 0.1f, Rgba8( 102, 255, 255 ) );
// 				}
// 				else if (unit->m_nearestSeaDir == Direction::Middle) {
// 					// do nothing
// 				}
// 			}
// 		}
// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
// 		g_theRenderer->BindTexture( nullptr );
// 		g_theRenderer->BindShader( nullptr );
// 		g_theRenderer->SetModelConstants();
// 		g_theRenderer->DrawVertexArray( nearestSea );
// 	}
// 	// debug show country start province
// 	if ((m_showingSettings.m_mapShowConfig & SHOW_CONFIG_COUNTRIES_MAP) && m_showingSettings.m_debugShowCountryStart) {
// 		std::vector<Vertex_PCU> countryStart;
// 		for (auto country : m_countries) {
// 			AddVertsForDisc2D( countryStart, country->m_originProv->m_geoCenterPos, 0.3f, Rgba8::WHITE );
// 		}
// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
// 		g_theRenderer->BindTexture( nullptr );
// 		g_theRenderer->BindShader( nullptr );
// 		g_theRenderer->SetModelConstants();
// 		g_theRenderer->DrawVertexArray( countryStart );
// 	}

	// render map boarders
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Photograph Frame.png" ) );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetModelConstants( Mat44::CreateTranslation3D( Vec3( 0.f, 0.f, m_generationSettings.m_minHeight * HEIGHT_FACTOR ) ) );
	g_theRenderer->DrawVertexBuffer( m_mapBorderVertexBuffer, m_mapBorderVertexBuffer->GetVertexCount() );
	

// 	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
// 	if (m_curViewingCity) {
// 		DebugDrawRing( m_curViewingCity->m_position, CITY_ICON_SIDE_LENGTH * 2.f, 0.05f, Rgba8( 255, 0, 0 ) );
// 	}
// 	if (m_curViewingArmy) {
// 		DebugDrawRing( m_curViewingArmy->GetPosition(), CITY_ICON_SIDE_LENGTH * 2.f, 0.05f, Rgba8( 255, 0, 0 ) );
// 	}
}

void Map::Prepare2DMapForSphere() const
{
	g_theRenderer->SetScreenRenderTargetView( g_theGame->m_worldCamera );
	Mat44 modelMatrix = Mat44::CreateTranslation2D( m_dimensions );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( m_2DPolygonTextureShader );
	g_theRenderer->SetModelConstants( modelMatrix, m_renderPreference.m_oceanColor );
	VertexBuffer* buffers[] = { m_polygonsFacesVertexBuffer, m_polygonFacesColorVertexBuffer, m_polygonuvVertexBuffer };
	g_theRenderer->DrawVertexBuffers( 3, buffers, m_polygonsFacesVertexBuffer->GetVertexCount(), 0 );

	// render rivers
	if ((m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RIVERS) && m_riverVertexBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexBuffer( m_riverVertexBuffer, m_riverVertexBuffer->GetVertexCount() );
	}
	// render roads
	if ((m_showingSettings.m_mapShowConfig & SHOW_CONFIG_ROADS) && m_roadVertexBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexBuffer( m_roadVertexBuffer, m_roadVertexBuffer->GetVertexCount() );
	}
	// 	// render culture origins
	// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CULTURE_MAP) {
	// 		std::vector<Vertex_PCU> cultureCenterVerts;
	// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	// 		for (int i = 0; i < (int)m_cultures.size(); i++) {
	// 			AddVertsForAABB2D( cultureCenterVerts, AABB2( m_cultures[i]->m_cultureOriginUnit->m_geoCenterPos - Vec2( 0.3f, 0.3f ), m_cultures[i]->m_cultureOriginUnit->m_geoCenterPos + Vec2( 0.3f, 0.3f ) ), Rgba8( 255, 255, 255 ) );
	// 		}
	// 		g_theRenderer->BindTexture( nullptr );
	// 		g_theRenderer->BindShader( nullptr );
	// 		g_theRenderer->SetModelConstants();
	// 		g_theRenderer->DrawVertexArray( cultureCenterVerts );
	// 	}
	// 	// render religion origins
	// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELIGION_MAP) {
	// 		std::vector<Vertex_PCU> religionCenterVerts;
	// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	// 		for (int i = 0; i < (int)m_religions.size(); i++) {
	// 			AddVertsForAABB2D( religionCenterVerts, AABB2( m_religions[i]->m_religionOriginUnit->m_geoCenterPos - Vec2( 0.3f, 0.3f ), m_religions[i]->m_religionOriginUnit->m_geoCenterPos + Vec2( 0.3f, 0.3f ) ), Rgba8( 255, 255, 255 ) );
	// 		}
	// 		g_theRenderer->BindTexture( nullptr );
	// 		g_theRenderer->BindShader( nullptr );
	// 		g_theRenderer->SetModelConstants();
	// 		g_theRenderer->DrawVertexArray( religionCenterVerts );
	// 	}
	// render polygon edges
// 	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_POLYGON_EDGES) {
// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
// 		g_theRenderer->BindTexture( nullptr );
// 		g_theRenderer->BindShader( m_2DTextureShader );
// 		g_theRenderer->SetModelConstants(modelMatrix);
// 		g_theRenderer->DrawVertexIndexed( m_polygonsEdgesVertexBuffer, m_polygonEdgesIndexBuffer, m_polygonEdgesIndexBuffer->GetIndexCount(), 0 );
// 	}
	// render country edges
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_COUNTRIES_EDGES) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants(modelMatrix);
		g_theRenderer->DrawVertexBuffer( m_countriesEdgesVertexBuffer, m_countriesEdgesVertexBuffer->GetVertexCount() );
	}
	// render towns
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_TOWNS) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexBuffer( m_townVertexBuffer, m_townVertexBuffer->GetVertexCount() );
		if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS) {
			for (auto label : m_townLabels) {
				label->Render( m_2DTextureShader, modelMatrix );
			}
		}
	}
	// render cities
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CITIES) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexBuffer( m_cityVertexBuffer, m_cityVertexBuffer->GetVertexCount() );
		if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS) {
			for (auto label : m_cityLabels) {
				label->Render( m_2DTextureShader, modelMatrix );
			}
		}
	}
	// render armies
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_ARMIES) {
		std::vector<Vertex_PCU> armiesVerts;
		armiesVerts.reserve( 10000 );
		for (auto country : m_countries) {
			for (auto army : country->m_armies) {
				Vec2 armyPos = army->GetPosition();
				AddVertsForDisc2D( armiesVerts, armyPos, army->GetOuterRadius(), Rgba8( 255, 255, 255, 150 ) );
				AddVertsForDisc2D( armiesVerts, armyPos, army->GetInnerRadius(),
					Rgba8( (unsigned char)std::max( army->m_owner->m_color.r - 50, 0 ),
						(unsigned char)std::max( army->m_owner->m_color.g - 50, 0 ),
						(unsigned char)std::max( army->m_owner->m_color.b - 50, 0 ),
						army->m_owner->m_color.a ) );
			}
		}
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexArray( armiesVerts );
	}

	// render culture labels
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CULTURE_MAP) {
		for (auto label : m_cultureLabels) {
			label->Render( m_2DTextureShader, modelMatrix );
		}
	}
	// render religion labels
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_RELIGION_MAP) {
		for (auto label : m_religionLabels) {
			label->Render( m_2DTextureShader, modelMatrix );
		}
	}
	// render country labels
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_COUNTRIES_MAP) {
		for (auto label : m_countryLabels) {
			label->Render( m_2DTextureShader, modelMatrix );
		}
	}
	// render region labels
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_REGIONS_MAP) {
		for (auto label : m_regionLabels) {
			label->Render( m_2DTextureShader, modelMatrix );
		}
	}
	// render continent labels
	if (m_showingSettings.m_mapShowConfig & SHOW_CONFIG_LABELS && m_showingSettings.m_mapShowConfig & SHOW_CONFIG_CONTINENT_MAP) {
		for (auto label : m_continentLabels) {
			label->Render( m_2DTextureShader, modelMatrix );
		}
	}

	// DEBUG render polygon center
	if (m_showingSettings.m_debugShowPolygonCenter) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexBuffer( m_polygonCenterVertexBuffer, m_polygonCenterVertexBuffer->GetVertexCount() );
	}
	// DEBUG render river start
	if (m_showingSettings.m_debugShowRiverStart) {
		float radiusOfSite = 10.f * Minf( m_generationSettings.m_dimensions.x, m_generationSettings.m_dimensions.y ) / m_generationSettings.m_basePolygons;
		std::vector<Vertex_PCU> riverStartVerts;
		for (int i = 0; i < (int)m_rivers.size(); i++) {
			AddVertsForAABB2D( riverStartVerts, AABB2( m_rivers[i]->m_anchorPoint[0] - Vec2( radiusOfSite, radiusOfSite ), m_rivers[i]->m_anchorPoint[0] + Vec2( radiusOfSite, radiusOfSite ) ), Rgba8( 0, 0, 255 ) );
		}
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexArray( riverStartVerts );
	}
	// DEBUG render nearest sea directions
	if (m_showingSettings.m_debugShowNearestOceanUnit) {
		std::vector<Vertex_PCU> nearestSea;
		for (auto unit : m_mapPolygonUnits) {
			if (!unit->m_isFarAwayFakeUnit) {
				if (unit->m_nearestSeaDir == Direction::East) {
					AddVertsForLineSegment2D( nearestSea, unit->m_centerPosition, unit->m_centerPosition + m_diagonalLength * Vec2( unit->m_nearestSeaDistance, 0.f ), 0.1f, Rgba8( 102, 255, 255 ) );
				}
				else if (unit->m_nearestSeaDir == Direction::West) {
					AddVertsForLineSegment2D( nearestSea, unit->m_centerPosition, unit->m_centerPosition + m_diagonalLength * Vec2( -unit->m_nearestSeaDistance, 0.f ), 0.1f, Rgba8( 102, 255, 255 ) );
				}
				else if (unit->m_nearestSeaDir == Direction::Middle) {
					// do nothing
				}
			}
		}
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexArray( nearestSea );
	}
	// debug show country start province
	if ((m_showingSettings.m_mapShowConfig & SHOW_CONFIG_COUNTRIES_MAP) && m_showingSettings.m_debugShowCountryStart) {
		std::vector<Vertex_PCU> countryStart;
		for (auto country : m_countries) {
			AddVertsForDisc2D( countryStart, country->m_originProv->m_geoCenterPos, 0.3f, Rgba8::WHITE );
		}
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexArray( countryStart );
	}

	// render 2D edges bounds
	if (g_theGame->m_viewMode == MapViewMode::ViewMode2D) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( m_2DTextureShader );
		g_theRenderer->SetModelConstants( modelMatrix );
		g_theRenderer->DrawVertexBuffer( m_edgeBlockVertexBuffer, m_edgeBlockVertexBuffer->GetVertexCount() );
	}

	// render map boarders
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Photograph Frame.png" ) );
	g_theRenderer->BindShader( m_2DTextureShader );
	g_theRenderer->SetModelConstants( modelMatrix );
	g_theRenderer->DrawVertexBuffer( m_mapBorderVertexBuffer, m_mapBorderVertexBuffer->GetVertexCount() );

	g_theRenderer->ResetScreenRenderTargetView();
}

void Map::RenderSphere() const
{
	g_theRenderer->SetBasicRenderTargetView();
	//g_theRenderer->ClearScreen( Rgba8( 0, 0, 102 ), Rgba8::WHITE );
	// render sphere
	g_theRenderer->BindShader( nullptr );
	Mat44 modelMatrix = Mat44::CreateTranslation3D( Vec3( m_dimensions.x * 0.5f, m_dimensions.y * 0.5f, -(m_dimensions.x + m_dimensions.y) ) );
	g_theRenderer->SetModelConstants( modelMatrix );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	std::vector<Vertex_PCU> verts;
	verts.reserve( 6 );
	AddVertsForQuad3D( verts, Vec3( -m_dimensions * 100.f, 0.f ), Vec3( m_dimensions.x * 100.f, -m_dimensions.y * 100.f, 0.f ),
		Vec3( m_dimensions * 100.f, 0.f ), Vec3( -m_dimensions.x * 100.f, m_dimensions.y * 100.f, 0.f ), Rgba8( 0, 0, 102 ) );
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->BindShader( m_sphereShader );
	EulerAngles orientation;
	orientation.m_rollDegrees = 90.f;
	orientation.m_pitchDegrees = m_sphereModeSphereZDegrees;
	//orientation.m_yawDegrees = m_sphereModeSphereYDegrees;
	modelMatrix = Mat44::CreateTranslation3D( Vec3( m_dimensions.x * 0.5f, m_dimensions.y * 0.5f, -(m_dimensions.x + m_dimensions.y) * 0.5f ) );
	modelMatrix.Append( orientation.GetAsMatrix_IFwd_JLeft_KUp() );
	g_theRenderer->SetModelConstants( modelMatrix );
	g_theRenderer->BindTexture( g_theRenderer->GetCurScreenAsTexture() );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->DrawVertexBuffer( m_3DSphereVertexBuffer, m_3DSphereVertexBuffer->GetVertexCount(), 0 );
}

void Map::GenerateProvinceEditUI()
{
	delete m_provEditVertexBuffer;
	m_provEditVertexBuffer = nullptr;
	if (!m_curViewingUnit || !m_curViewingUnit->m_owner) {
		return;
	}
	std::vector<Vertex_PCU> verts;
	float width = (float)((double)m_diagonalLength / sqrt( (double)m_mapPolygonUnits.size() ) * 0.1);
	float edgeWidth = width * 0.1f;
	float halfCrossWidth = width * 0.2f;
	float halfCrossHeight = width * 0.6f;
	for (auto prov : m_mapPolygonUnits) {
		if (prov->IsLand() && prov->m_owner != m_curViewingUnit->m_owner) {
			AddVertsForAABB2D( verts, AABB2( prov->m_geoCenterPos - Vec2( halfCrossWidth, halfCrossHeight ), prov->m_geoCenterPos + Vec2( halfCrossWidth, halfCrossHeight ) ), Rgba8( 0, 200, 0, 255 ) );
			AddVertsForAABB2D( verts, AABB2( prov->m_geoCenterPos - Vec2( halfCrossHeight, halfCrossWidth ), prov->m_geoCenterPos + Vec2( halfCrossHeight, halfCrossWidth ) ), Rgba8( 0, 200, 0, 255 ) );
			AddVertsForLineSegment2D( verts, prov->m_geoCenterPos + Vec2( -width, -width ), prov->m_geoCenterPos + Vec2( -width, width ), edgeWidth, Rgba8( 0, 200, 0, 150 ) );
			AddVertsForLineSegment2D( verts, prov->m_geoCenterPos + Vec2( -width, width ), prov->m_geoCenterPos + Vec2( width, width ), edgeWidth, Rgba8( 0, 200, 0, 150 ) );
			AddVertsForLineSegment2D( verts, prov->m_geoCenterPos + Vec2( width, width ), prov->m_geoCenterPos + Vec2( width, -width ), edgeWidth, Rgba8( 0, 200, 0, 150 ) );
			AddVertsForLineSegment2D( verts, prov->m_geoCenterPos + Vec2( width, -width ), prov->m_geoCenterPos + Vec2( -width, -width ), edgeWidth, Rgba8( 0, 200, 0, 150 ) );
		}
// 		else if (prov->IsLand() && prov->m_owner == m_curViewingUnit->m_owner) {
// 			AddVertsForAABB2D( verts, AABB2( prov->m_geoCenterPos - Vec2( halfCrossHeight, halfCrossWidth ), prov->m_geoCenterPos + Vec2( halfCrossHeight, halfCrossWidth ) ), Rgba8( 255, 0, 0, 255 ) );
// 			AddVertsForLineSegment2D( verts, prov->m_geoCenterPos + Vec2( -width, -width ), prov->m_geoCenterPos + Vec2( -width, width ), edgeWidth, Rgba8( 255, 0, 0, 150 ) );
// 			AddVertsForLineSegment2D( verts, prov->m_geoCenterPos + Vec2( -width, width ), prov->m_geoCenterPos + Vec2( width, width ), edgeWidth, Rgba8( 255, 0, 0, 150 ) );
// 			AddVertsForLineSegment2D( verts, prov->m_geoCenterPos + Vec2( width, width ), prov->m_geoCenterPos + Vec2( width, -width ), edgeWidth, Rgba8( 255, 0, 0, 150 ) );
// 			AddVertsForLineSegment2D( verts, prov->m_geoCenterPos + Vec2( width, -width ), prov->m_geoCenterPos + Vec2( -width, -width ), edgeWidth, Rgba8( 255, 0, 0, 150 ) );
// 		}
	}

	m_provEditVertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_provEditVertexBuffer );

	if (!m_provEditHighLightVertexBuffer) {
		verts.clear();
		AddVertsForAABB2D( verts, AABB2( -Vec2( halfCrossWidth, halfCrossHeight ), Vec2( halfCrossWidth, halfCrossHeight ) ), Rgba8( 150, 255, 150, 255 ) );
		AddVertsForAABB2D( verts, AABB2( -Vec2( halfCrossHeight, halfCrossWidth ), Vec2( halfCrossHeight, halfCrossWidth ) ), Rgba8( 150, 255, 150, 255 ) );
		AddVertsForLineSegment2D( verts, Vec2( -width, -width ), Vec2( -width, width ), edgeWidth, Rgba8( 150, 255, 150, 150 ) );
		AddVertsForLineSegment2D( verts, Vec2( -width, width ), Vec2( width, width ), edgeWidth, Rgba8( 150, 255, 150, 150 ) );
		AddVertsForLineSegment2D( verts, Vec2( width, width ), Vec2( width, -width ), edgeWidth, Rgba8( 150, 255, 150, 150 ) );
		AddVertsForLineSegment2D( verts, Vec2( width, -width ), Vec2( -width, -width ), edgeWidth, Rgba8( 150, 255, 150, 150 ) );

		m_provEditHighLightVertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
		g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_provEditHighLightVertexBuffer );
	}
}

void Map::DebugCompareHistory( HistoryData const& prev, HistoryData const& cur ) const
{
	if (prev.m_crisisData.size() != cur.m_crisisData.size()) {
		ERROR_RECOVERABLE( "11" );
	}
	else {
		for (int i = 0; i < (int)cur.m_crisisData.size(); ++i) {
			if (cur.m_crisisData[i].m_countryID != prev.m_crisisData[i].m_countryID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_crisisData[i].m_cultureOrReligionID != prev.m_crisisData[i].m_cultureOrReligionID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_crisisData[i].m_globalID != prev.m_crisisData[i].m_globalID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_crisisData[i].m_progress != prev.m_crisisData[i].m_progress) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_crisisData[i].m_type != prev.m_crisisData[i].m_type) {
				ERROR_RECOVERABLE( "11" );
			}
		}
	}
	if (prev.m_armyData.size() != cur.m_armyData.size()) {
		ERROR_RECOVERABLE( "11" );
	}
	else {
		for (int i = 0; i < (int)cur.m_armyData.size(); ++i) {
			if (cur.m_armyData[i].m_globalID != prev.m_armyData[i].m_globalID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_armyData[i].m_ownerID != prev.m_armyData[i].m_ownerID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_armyData[i].m_provInID != prev.m_armyData[i].m_provInID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_armyData[i].m_targetProvID != prev.m_armyData[i].m_targetProvID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_armyData[i].m_size != prev.m_armyData[i].m_size) {
				//ERROR_RECOVERABLE( "11" );
			}
		}
	}
	if (prev.m_countryData.size() != cur.m_countryData.size()) {
		ERROR_RECOVERABLE( "11" );
	}
	else {
		for (int i = 0; i < (int)cur.m_countryData.size(); ++i) {
			if (cur.m_countryData[i].m_allianceCountriesID != prev.m_countryData[i].m_allianceCountriesID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_countryData[i].m_capitalCityID != prev.m_countryData[i].m_capitalCityID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_countryData[i].m_capitalProvID != prev.m_countryData[i].m_capitalProvID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_countryData[i].m_celestialCountryID != prev.m_countryData[i].m_celestialCountryID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_countryData[i].m_countryCultureID != prev.m_countryData[i].m_countryCultureID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_countryData[i].m_countryReligionID != prev.m_countryData[i].m_countryReligionID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_countryData[i].m_friendlyCountriesID != prev.m_countryData[i].m_friendlyCountriesID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_countryData[i].m_funds != prev.m_countryData[i].m_funds) {
				//ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_countryData[i].m_hostileCountriesID != prev.m_countryData[i].m_hostileCountriesID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_countryData[i].m_isCelestial != prev.m_countryData[i].m_isCelestial) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_countryData[i].m_suzerainCountryID != prev.m_countryData[i].m_suzerainCountryID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_countryData[i].m_tributaryCountriesID != prev.m_countryData[i].m_tributaryCountriesID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_countryData[i].m_vassalCountriesID != prev.m_countryData[i].m_vassalCountriesID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_countryData[i].m_warCountriesID != prev.m_countryData[i].m_warCountriesID) {
				ERROR_RECOVERABLE( "11" );
			}
		}
	}
	if (prev.m_provinceData.size() != cur.m_provinceData.size()) {
		ERROR_RECOVERABLE( "11" );
	}
	else {
		for (int i = 0; i < (int)cur.m_provinceData.size(); ++i) {
			if (cur.m_provinceData[i].m_cultures != prev.m_provinceData[i].m_cultures) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_provinceData[i].m_isWater != prev.m_provinceData[i].m_isWater) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_provinceData[i].m_legalCountriesID != prev.m_provinceData[i].m_legalCountriesID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_provinceData[i].m_ownerCountryID != prev.m_provinceData[i].m_ownerCountryID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_provinceData[i].m_population != prev.m_provinceData[i].m_population) {
				//ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_provinceData[i].m_religions != prev.m_provinceData[i].m_religions) {
				ERROR_RECOVERABLE( "11" );
			}
		}
	}
	
	if (prev.m_cityData.size() != cur.m_cityData.size()) {
		ERROR_RECOVERABLE( "11" );
	}
	else {
		for (int i = 0; i < (int)cur.m_cityData.size(); ++i) {
			if (cur.m_cityData[i].m_defenseValue != prev.m_cityData[i].m_defenseValue) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_cityData[i].m_cultures != prev.m_cityData[i].m_cultures) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_cityData[i].m_ownerID != prev.m_cityData[i].m_ownerID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_cityData[i].m_population != prev.m_cityData[i].m_population) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_cityData[i].m_religions != prev.m_cityData[i].m_religions) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_cityData[i].m_type != prev.m_cityData[i].m_type) {
				ERROR_RECOVERABLE( "11" );
			}
		}
	}
	
	if (prev.m_townData.size() != cur.m_townData.size()) {
		ERROR_RECOVERABLE( "11" );
	}
	else {
		for (int i = 0; i < (int)cur.m_townData.size(); ++i) {
			if (cur.m_townData[i].m_defenseValue != prev.m_townData[i].m_defenseValue) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_townData[i].m_cultures != prev.m_townData[i].m_cultures) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_townData[i].m_ownerID != prev.m_townData[i].m_ownerID) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_townData[i].m_population != prev.m_townData[i].m_population) {
				ERROR_RECOVERABLE( "11" );
			}
			if (cur.m_townData[i].m_religions != prev.m_townData[i].m_religions) {
				ERROR_RECOVERABLE( "11" );
			}
		}
	}
}

void Map::PM_AddEdgeToPolygonUnit( MapPolygonUnit* polygonUnit, FortuneHalfEdge* edge )
{
	StarEdge* newEdge = new StarEdge();
	edge->m_addrInProj = newEdge;
	newEdge->m_startPos = FortuneVec2dToVec2f( edge->m_vertexPos );
	newEdge->m_endPos = FortuneVec2dToVec2f( edge->m_opposite->m_vertexPos );
	newEdge->m_owner = polygonUnit;
	if (edge->m_opposite && edge->m_opposite->m_addrInProj) {
		StarEdge* oppoEdge = (StarEdge*)edge->m_opposite->m_addrInProj;
		newEdge->m_opposite = oppoEdge;
		oppoEdge->m_opposite = newEdge;
	}
	if (edge->m_next && edge->m_next->m_addrInProj) {
		StarEdge* nextEdge = (StarEdge*)edge->m_next->m_addrInProj;
		newEdge->m_next = nextEdge;
		nextEdge->m_prev = newEdge;
	}
	if (edge->m_prev && edge->m_prev->m_addrInProj) {
		StarEdge* prevEdge = (StarEdge*)edge->m_prev->m_addrInProj;
		newEdge->m_prev = prevEdge;
		prevEdge->m_next = newEdge;
	}
	polygonUnit->m_edges.push_back( newEdge );
}

bool operator<( Vec2 const& a, Vec2 const& b )
{
	if (a.y > b.y) {
		return false;
	}
	else if (a.y < b.y) {
		return true;
	}
	else {
		return a.x < b.x;
	}
}
