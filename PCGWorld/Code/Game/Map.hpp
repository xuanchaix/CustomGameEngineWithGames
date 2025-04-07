#pragma once
#include "Game/GameCommon.hpp"
#include "Game/MapPolygonUnit.hpp"
#include "Game/AStarHelper.hpp"
#include "Game/HistoryData.hpp"
#include <set>

class River;
class Forest;
class Culture;
class City;
class Country;
class Continent;
class Region;
class Road;
class Label;
class CultureNameGenerator;
class ReligionNameGenerator;
class ProvinceNameGenerator;
class Army;
class CountryInstruction;
class HistoryCrisis;
class SaveHistoryJob;

bool operator<( Vec2 const& a, Vec2 const& b );

struct FortuneHalfEdge;
class PolyUnitDataCalculationJob;
class FindCountryForProvinceJob;

struct MapRenderingPreference {
	Rgba8 m_oceanColor = Rgba8( 0, 204, 204 );
	Rgba8 m_lowestLandHeightColor = Rgba8( 0, 255, 0, 255 );
	Rgba8 m_highestLandHeightColor = Rgba8( 255, 0, 0, 255 );
	Rgba8 m_lowestOceanHeightColor = Rgba8( 0, 0, 153, 255 );
	Rgba8 m_highestOceanHeightColor =  Rgba8( 0, 0, 255, 255 );
	Rgba8 m_lowestPrecipitationColor = Rgba8( 0, 200, 0, 255 );
	Rgba8 m_highestPrecipitationColor = Rgba8( 0, 0, 255, 255 );
	Rgba8 m_lowestTemperatureColor = Rgba8( 0, 128, 255, 255 );
	Rgba8 m_highestTemperatureColor = Rgba8( 255, 128, 0, 255 );
	Rgba8 m_lowestPopulationColor =  Rgba8( 102, 204, 0 );
	Rgba8 m_highestPopulationColor = Rgba8( 255, 51, 51 );
	Rgba8 m_invalidProvince = Rgba8( 60, 60, 60 );

	std::vector<Rgba8> m_productColorMap;
	std::vector<Vec4> m_productColorMapVec4;

	std::vector<Rgba8> m_relationColorMap;
	std::vector<Vec4> m_relationColorMapVec4;

	std::vector<Rgba8> m_climateColorMap;
	std::vector<Vec4> m_climateColorMapVec4;

	std::vector<Rgba8> m_landformColorMap;
	std::vector<Vec4> m_landformColorMapVec4;
};

struct MapGenerationSettings {
	Vec2 m_dimensions = Vec2( 300.f, 150.f );
	int m_basePolygons = 10000;
	unsigned int m_seed = 147;
	float m_minHeight = -8000.f;
	float m_maxHeight = 8000.f;
	float m_fragmentFactor = 0.25f;
	float m_landRichnessFactor = 0.52f;
	int m_numOfUnitsToHaveLake = 6;
	int m_numOfUnitsToHaveIsland = 6;
	int m_sqrtBasePolygons = 0;
	int m_numOfCultures = 25;
	int m_numOfReligions = 6;
	float m_cityRichness = 0.1f;
	float m_townRichness = 0.5f;
	bool m_onlyUseWesternCountryPrefix = false;
	bool m_enableHistorySimulation = true;
};

enum class MapViewMode {
	ViewMode2D, ViewMode3D, ViewModeSphere,
};

#define SHOW_CONFIG_HEIGHT_MAP					0x00000001
#define SHOW_CONFIG_CLIMATE_MAP					0x00000002
#define SHOW_CONFIG_SUMMER_PRECIPITATION_MAP	0x00000004
#define SHOW_CONFIG_WINTER_PRECIPITATION_MAP	0x00000008
#define SHOW_CONFIG_SUMMER_TEMPERATURE_MAP		0x00000010
#define SHOW_CONFIG_WINTER_TEMPERATURE_MAP		0x00000020
#define SHOW_CONFIG_LANDFORM_MAP				0x00000040
#define SHOW_CONFIG_POPULATION_MAP				0x00000080
#define SHOW_CONFIG_CULTURE_MAP					0x00000100
#define SHOW_CONFIG_RELIGION_MAP				0x00000200
#define SHOW_CONFIG_POLYGON_EDGES				0x00000400
#define SHOW_CONFIG_RIVERS						0x00000800
#define SHOW_CONFIG_CITIES						0x00001000
#define SHOW_CONFIG_CONTINENT_MAP				0x00002000
#define SHOW_CONFIG_PRODUCT_MAP					0x00004000
#define SHOW_CONFIG_TOWNS						0x00008000
#define SHOW_CONFIG_COUNTRIES_MAP				0x00010000
#define SHOW_CONFIG_COUNTRIES_EDGES				0x00020000
#define SHOW_CONFIG_REGIONS_MAP					0x00040000
#define SHOW_CONFIG_RELATION_MAP				0x00080000
#define SHOW_CONFIG_ROADS						0x00100000
#define SHOW_CONFIG_LABELS						0x00200000
#define SHOW_CONFIG_ARMIES						0x00400000
#define SHOW_CONFIG_PROVINCE_EDIT				0x00800000

#define SHOW_CONFIG_COLORFUL_MAP				(SHOW_CONFIG_RELATION_MAP | SHOW_CONFIG_REGIONS_MAP | SHOW_CONFIG_COUNTRIES_MAP | SHOW_CONFIG_PRODUCT_MAP | SHOW_CONFIG_CONTINENT_MAP | SHOW_CONFIG_RELIGION_MAP | SHOW_CONFIG_CULTURE_MAP | SHOW_CONFIG_POPULATION_MAP | SHOW_CONFIG_LANDFORM_MAP | SHOW_CONFIG_HEIGHT_MAP | SHOW_CONFIG_CLIMATE_MAP | SHOW_CONFIG_SUMMER_PRECIPITATION_MAP | SHOW_CONFIG_WINTER_PRECIPITATION_MAP | SHOW_CONFIG_SUMMER_TEMPERATURE_MAP | SHOW_CONFIG_WINTER_TEMPERATURE_MAP | SHOW_CONFIG_PROVINCE_EDIT)

struct MapShowingSettings {
	uint32_t m_mapShowConfig =SHOW_CONFIG_POLYGON_EDGES | SHOW_CONFIG_COUNTRIES_EDGES | SHOW_CONFIG_CITIES | SHOW_CONFIG_LABELS;
	bool m_debugShowPolygonCenter = false;
	bool m_debugShowNearestOceanUnit = false;
	bool m_debugShowRiverStart = false;
	bool m_debugShowCountryStart = false;
};

class Map {
public:
	Map( MapGenerationSettings const& settings );
	~Map();

	void Startup();
	void Update();
	void Render() const;
	Vec2 GetDimensions() const;
	MapGenerationSettings const& GetGenerationSettings() const;

	unsigned int GetSeed() const;

	void SetRenderHeightMapMode( bool setRender = true );
	void SetRenderClimateMapMode( bool setRender = true );
	void SetRenderPrecipitationMapMode( bool setRender = true, bool summer = true );
	void SetRenderTemperatureMapMode( bool setRender = true, bool summer = true );
	void SetRenderLandformMapMode( bool setRender = true );
	void SetRenderPopulationMapMode( bool setRender = true );
	void SetRenderCultureMapMode( bool setRender = true );
	void SetRenderReligionMapMode( bool setRender = true );
	void SetRenderContinentMapMode( bool setRender = true );
	void SetRenderProductMapMode( bool setRender = true );
	void SetRenderCountryMapMode( bool setRender = true );
	void SetRenderRegionMapMode( bool setRender = true );
	void SetRenderRelationMapMode( bool setRender = true );
	void SetRenderProvEditMapMode( bool setRender = true );
	void Reset2DCameraMode();
	void Reset3DCameraMode();
	void ResetSphereCameraMode();

	MapPolygonUnit* GetUnitByPos( Vec2 const& pos ) const;
	MapPolygonUnit* GetUnitByPosFast( Vec2 const& pos ) const;
	MapPolygonUnit* GetCurHoveringAtUnit3D( Vec3 const& mouseWorldPos ) const;
	City* GetCityByPos( Vec2 const& pos ) const;
	City* GetCurHoveringCity3D( Vec3 const& mouseWorldPos ) const;
	Army* GetArmyByPos( Vec2 const& pos ) const;
	Army* GetCurHoveringArmy3D( Vec3 const& mouseWorldPos ) const;
	bool IsTwoProvConnectedInSameCountry( MapPolygonUnit* prov1, MapPolygonUnit* prov2 ) const;

	void PM_ClampLinesIntoBounds( Vec2& pos1, Vec2& pos2 );
	Vec2 MP_GetRandomPointInAABB2D( AABB2 const& box );
	Vec2 MP_GetRandomPointInDisc2D( Vec2 const& center, float radius );

	void SetUpHistorySimulation();
	void SimulateNextMonth();
	void SimulateNextMonthWithoutRefresh();
	void SimulateMonths( int numOfMonth );
	void ViewNextMonth();
	void ViewNextYear();
	void ViewLastMonth();
	void ViewLastYear();
	//void ViewSpecificMonth( int year, int month );
	void AddInstructionToQueue( CountryInstruction* instr );
	bool IsTwoCountryPassEndWarTime( Country* country1, Country* country2, int numOfMonth = 36 );
	int GetMonthDiffBetweenTwoTimes( int year1, int month1, int year2, int month2 );
	int GetTotalMonthCount() const;
	void GetYearAndMonthFromTotalMonth( int& year, int& month, int totalMonth ) const;
	void LoadHistoryCacheFromDisk( HistoryData& data, int year, int month ) const;
	void AddHistoryLog( std::string const& log );

	void SaveCurrentWorldToXml() const;
	void SaveHistoryToXml();

	void GetCultureListExclude( std::vector<Culture*>& out_cultures, Culture* exCulture ) const;
	void GetReligionListExclude( std::vector<Religion*>& out_religions, Religion* exReligion ) const;

	void RefreshAllLabels();
	void GetHistoryData( int year, int month, HistoryData& data );
public:
	// need to be record through months and years
	std::vector<MapPolygonUnit*> m_mapPolygonUnits;
	std::vector<City*> m_cities;
	std::vector<Town*> m_towns;
	std::vector<Country*> m_countries;

	std::vector<River*> m_rivers;
	//std::vector<Forest*> m_forests;
	std::vector<Continent*> m_continents;
	std::vector<Culture*> m_cultures;
	std::vector<Religion*> m_religions;
	std::vector<Road*> m_roads;

	// need to be refreshed in history records
	std::vector<Region*> m_regions;
	std::vector<Label*> m_cityLabels;
	std::vector<Label*> m_townLabels;
	std::vector<Label*> m_countryLabels;
	std::vector<Label*> m_cultureLabels;
	std::vector<Label*> m_religionLabels;
	std::vector<Label*> m_regionLabels;
	std::vector<Label*> m_continentLabels;

	MapShowingSettings m_showingSettings;
	MapGenerationSettings m_generationSettings;
	MapRenderingPreference m_renderPreference;
	std::vector<Rgba8> m_colorVertexArray;

	Vec2 m_dimensions;
	AABB2 m_bounds;
	float m_diagonalLength = 0.f;
	unsigned int m_heightSeed;
	unsigned int m_precipitationSeed;
	unsigned int m_temperatureSeed;
	unsigned int m_landformSeed;
	unsigned int m_riverStartSeed;
	unsigned int m_populationSeed;
	unsigned int m_citySeed;
	unsigned int m_productSeed;
	unsigned int m_townSeed;
	unsigned int m_cultureNameSeed;
	unsigned int m_religionNameSeed;
	std::set<MapPolygonUnit*> m_oceanDirtySet;
	std::set<MapPolygonUnit*> m_islandDirtySet;
	RandomNumberGenerator* m_mapRNG = nullptr;
	RandomNumberGenerator* m_productRNG = nullptr;
	RandomNumberGenerator* m_historyRNG = nullptr;
	std::vector<float> m_productPrice;

	float m_seaLevel = 0.f; // sea level decided by user input max height and min height

	bool m_viewingCountry = false;
	bool m_viewingRegion = false;
	bool m_showProvincePanel = false;
	bool m_showCityPanel = false;
	bool m_showArmyPanel = false;
	MapPolygonUnit* m_curViewingUnit = nullptr;
	City* m_curViewingCity = nullptr;
	Army* m_curViewingArmy = nullptr;

	AStarHelper m_aStarHelper;

	CultureNameGenerator* m_cultureNameGenerator = nullptr;
	ReligionNameGenerator* m_religionNameGenerator = nullptr;
	ProvinceNameGenerator* m_regionContinentNameGenerator = nullptr;

	int m_year = 0;
	int m_month = 1;
	int m_viewingYear = 0;
	int m_viewingMonth = 1;
	bool m_simulationJumpToCurrent = true;
	bool m_autoRunSimulation = false;
	std::deque<CountryInstruction*> m_instructionQueue;
	std::vector<HistoryData*> m_historyData;
	std::vector<HistoryCrisis*> m_crisis;

	static const unsigned int INVALID_ARMY_ID = (unsigned int)-1;
	static const unsigned int INVALID_HISTORY_CRISIS_ID = (unsigned int)-1;

	Strings m_historyLog;
	std::string m_generationLog;
	//std::string m_runTimeLog;

	std::mutex m_saveLoadMutex;

	HistorySavingSolver* m_historySavingModule = nullptr;
	SaveHistoryJob* m_saveHistoryJob = nullptr;

	Timer* m_2D3DSwitchTimer = nullptr;
	Vec3 m_viewSwitchTargetPosition;
	Vec3 m_viewSwitchStartPosition;
	float m_switchStartYaw = 0.f;
	float m_switchStartPitch = 0.f;

//	float m_sphereModeSphereYDegrees = 0.f;
	float m_sphereModeSphereZDegrees = 0.f;
protected:
	// history simulation part
	void CountriesBeginTurn();
	void CountriesEndTurn();
	void PutCountryInstructions();
	void ExecuteCountryBehaviors();
	void ResolveBattles();
	void ExecuteInstruction( CountryInstruction const* instr );
	void MapEndTurn();
	void RecordHistory();
	void UpdateColorfulMaps();
	void ReadHistoryCache( HistoryData const& data );
	void RearrangeHistoryCache();
	bool DoHistoryExist( int year, int month ) const;
	void ProcessHistoryCrisis();
	void AddNewCrisisToList( HistoryCrisis* crisis );
	HistoryData const& GetHistoryData( int year, int month );
	Army* GetArmyByGlobalID( unsigned int id ) const;
	HistoryCrisis* GetCrisisByGlobalID( unsigned int id ) const;
	Country* CreateNewCountry( std::vector<Province*> const& initialProvince, Culture* countryCulture, Religion* countryReligion, CountryGovernmentType govType );

	// map generation part
	void StartUpMap();
	void ReadMapRenderingPreferences();
	void PopulateMapWithPolygons( int numOfPolygons = 10000 );
	void CalculateBiomeDataForPolygons();
	void GenerateRivers();
	void GenerateForests();
	void GenerateContinents();
	void GenerateHumanDataForPolygons();
	void GenerateCultures();
	void GenerateReligions();
	void GenerateCitiesAndTowns();
	void GenerateCountriesAndRelations();
	void ConnectCountries();
	void GenerateProvinceProducts();
	void GenerateRegions();
	void GenerateRoads();
	void GenerateArmies();
	void GenerateVertexBuffers();
	void InitializeLabels();

	void HandleKeys();
	void UpdateLabels();
	void UpdateEdges();
	void UpdateSwitching2D3D();
	void UpdateAutoSimulation();
	void UpdateBufferColors();
	void UpdateSaveStatus();

	void Render2D() const;
	void Prepare2DMapFor3D() const;
	void Render3D() const;
	void Prepare2DMapForSphere() const;
	void RenderSphere() const;

	void GenerateProvinceEditUI();

	void DebugCompareHistory( HistoryData const& prev, HistoryData const& cur ) const;
	
	// populate map stage helper functions
	void PM_AddEdgeToPolygonUnit( MapPolygonUnit* polygonUnit, FortuneHalfEdge* edge );
	VertexBuffer* m_polygonsEdgesVertexBuffer = nullptr;
	IndexBuffer* m_polygonEdgesIndexBuffer = nullptr;
	VertexBuffer* m_polygonsEdgesVertexBuffer3D = nullptr;
	IndexBuffer* m_polygonEdgesIndexBuffer3D = nullptr;
	VertexBuffer* m_polygonsFacesVertexBuffer = nullptr;
	VertexBuffer* m_polygonsFaces3DVertexBuffer = nullptr;
	VertexBuffer* m_edgeBlockVertexBuffer = nullptr;
	VertexBuffer* m_mapBorderVertexBuffer = nullptr;
	VertexBuffer* m_polygonCenterVertexBuffer = nullptr;
	VertexBuffer* m_polygonFacesColorVertexBuffer = nullptr;
	VertexBuffer* m_polygonuvVertexBuffer = nullptr;
	VertexBuffer* m_polygonuv3DVertexBuffer = nullptr;
	VertexBuffer* m_riverVertexBuffer = nullptr;
	VertexBuffer* m_cityVertexBuffer = nullptr;
	VertexBuffer* m_townVertexBuffer = nullptr;
	VertexBuffer* m_countriesEdgesVertexBuffer = nullptr;
	VertexBuffer* m_countriesEdgesVertexBuffer3D = nullptr;
	VertexBuffer* m_roadVertexBuffer = nullptr;
	VertexBuffer* m_polygonsNormalVertexBuffer = nullptr;
	VertexBuffer* m_provEditVertexBuffer = nullptr;
	VertexBuffer* m_provEditHighLightVertexBuffer = nullptr;
	VertexBuffer* m_3DSphereVertexBuffer = nullptr;
	Shader* m_polygonFaceShader = nullptr;
	Shader* m_2DTextureShader = nullptr;
	Shader* m_3DShader = nullptr;
	Shader* m_sphereShader = nullptr;
	Shader* m_2DPolygonTextureShader = nullptr;

	std::deque<PolyUnitDataCalculationJob*> m_polyDataCalculationJobs;
	std::deque<FindCountryForProvinceJob*> m_findCountryForProvinceJobs;

	Timer* m_polygonInspectionEdgeBlurringTimer = nullptr;

	std::vector<CountryEndWarTreaty> m_endWarTimes;
	unsigned int m_globalArmyID = 1;
	unsigned int m_globalHistoryCrisisID = 1;
	
	bool m_renderProvinceEditUIHighlight = false;
	MapPolygonUnit* m_hoveringProvince = nullptr;

};