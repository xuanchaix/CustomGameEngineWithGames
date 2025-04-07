#pragma once
#include "Game/GameCommon.hpp"
#include <unordered_map>

class MapPolygonUnit;
class River;
class Population;
class Culture;
class Religion;
class City;
class Continent;
class Town;
class Country;
class Region;
class Road;
class ProvinceRoadData;
class Army;

typedef MapPolygonUnit Province;

enum class LandformType {
	Ocean,
	Land,
	Lake,
	Island,
	Lowland_Plain,
	Highland_Plain,
	Grassland,
	Savanna,
	Marsh,
	Rainforest,
	Temperate_Forest,
	Subarctic_Forest,
	Subtropical_Forest,
	Lowland_Hill,
	Highland_Hill,
	Mountain,
	Icefield,
	Tundra,
	Desert,
	Town,
	NUM,
};

enum class ClimateType {
	TropicalRainforest, // AF
	TropicalMonsoon, //AM
	TropicalSavanna, //AS
	HotDesert, //BWh
	ColdDesert, // BWk
	MediterraneanClimate, // Csa Csb Csc
	HumidSubtropical,
	Oceanic,
	HumidContinentalMonsoon,
	HumidContinental,
	Subarctic,
	Tundra,
	IceCap, 
	Water,
	None,
	Num,
};

enum class ProductType {
	/* food */
	Fish,
	Grain,
	Fruit,
	Sugar,
	Salt,
	Livestock,

	/* material */
	Wax,
	Fur,
	Wood,
	Ivory,
	Cotton,
	Wool,

	/* metal */
	Iron,
	Gold,
	Copper,
	Silver,

	/* additives */
	Wine,
	Spice,
	Tea,
	Coffee,
	Tobacco,

	/* goods */
	Glass,
	Silk,
	Jade,
	Porcelain,
	Cloth,
	Gem,

	/* Military */
	WarHorse,
	Sword,

	None,
	Num,
};

#define Prov_Dirty_Flag_Populaton			0x0001
#define Prov_Dirty_Flag_Major_Culture		0x0002
#define Prov_Dirty_Flag_Cultures			0x0004
#define Prov_Dirty_Flag_Major_Religion		0x0008
#define Prov_Dirty_Flag_Religions			0x0010
#define Prov_Dirty_Flag_Owner				0x0020
#define Prov_Dirty_Flag_Legal_Countries		0x0040
#define Prov_Dirty_Flag_Army_On				0x0080


/*
std::deque<std::pair<Army*, float>> m_armiesOnProv; // army ref, dist

*/

struct StarEdge {
	Vec2 m_startPos;
	Vec2 m_endPos;
	StarEdge* m_opposite = nullptr;
	StarEdge* m_prev = nullptr;
	StarEdge* m_next = nullptr;
	MapPolygonUnit* m_owner = nullptr;
	float m_startHeight = 0.f;
	bool m_isDrawn = false;

	std::deque<Vec2> m_noisyEdges;
	void DoNoisyEdge( int recursionTimes, float ratio );

private:
	void RecursiveCalculateNoisyEdge( int numOfCurRecursive, Vec2 const& lineSegLeft, Vec2 const& lineSegRight, 
		Vec2 const& refPointTop, Vec2 const& refPointDown, float ratio, bool considerRiverAlready = false );
};

class MapPolygonUnit {
public:
	MapPolygonUnit( Vec2 const& centerPos );
	~MapPolygonUnit();

	void AddLegitimateCountry( Country* countryToAdd );
	bool IsLegitimateToCountry( Country* country ) const;
	bool IsProvOnOwnerCountryBoarder() const;

	/// polygon data
	/// in this function, height is decided, and land and sea is decided
	void InitializeUnit();
	/// in this function, lake and island is calculated
	void InitializeUnit2();
	/// in this function, climate is calculated
	void InitializeUnit3();

	void CalculateRawPopulation();

	void CalculateLowerAdjacentUnits(); // we need height data to do this
	void CalculateGeometricCenter();
	float CalculateGeometricDistance( MapPolygonUnit* unit ); // we need height data to do this
	void GetOneRouteToProvince( std::vector<MapPolygonUnit*>& out_route, MapPolygonUnit* startUnit, MapPolygonUnit* destUnit );

	void SetRenderNoColor();
	void SetRenderHeightColor();
	void SetRenderClimateColor();
	void SetRenderPrecipitationColor( bool summer );
	void SetRenderTemperatureColor( bool summer );
	void SetRenderViewingColor( bool isViewing );
	void SetRenderLandformColor();
	void SetRenderPopulationColor();
	void SetRenderCultureColor();
	void SetRenderReligionColor();
	void SetRenderContinentColor();
	void SetRenderProductColor();
	void SetRenderCountryColor();
	void SetRenderRegionColor();
	void SetRenderRelationColor();
	void SetRenderProvEditColor();

	void SetColorOfPolygonFace( Rgba8 color );

	float EvaluateProvinceArmyScore();

	bool IsOcean() const;
	bool IsLand() const;
	bool IsWater() const;
	bool IsCapitalProv() const;

	bool IsPointInsideConvexPolygon( Vec2 const& point ) const;
	bool IsPointInsideConcavePolygon( Vec2 const& point ) const;
	Vec2 GetRandomPointNearCenter() const;
	bool IsAdjacentToUnit( MapPolygonUnit* theOtherUnit ) const;
	bool IsAdjacentToCountry( Country* country ) const;

	void HD_AddCultureInfluence( Culture* cultureToAdd );
	void RecalculateMajorCulture();
	float HD_GetCultureInfluence( Culture* culture );

	void HD_AddReligionInfluence( Religion* religionToAdd );
	void RecalculateMajorReligion();
	float HD_GetReligionInfluence( Religion* religion );

	void HD_GenerateCities( bool forceOperation = false, int tryAmountOverride = 0 );
	void HD_GenerateTowns();

	void CalculateProduct();
	void CalculateArtificialProduct();
	void CalculateMetalProduct();

	// simulation
	void GrowPopulationOneMonth();
	bool IsBeingSieged() const;
	bool IsConnectedByRoad( MapPolygonUnit* other ) const;
	void RemoveUnqualifiedLegitimateCountry();

	// history
	//void AddHistoryData( void* startLocation ) const;
	float GetEconomyValue() const;

	void ResolveChangePopulation( int prevTotalPopulation );
	void GetUnselectedCultures( std::vector<Culture*>& out_cultures );
	void GetUnselectedReligions( std::vector<Religion*>& out_religions );
	void SqueezeReligionInfluence( Religion* religion, float influenceToAdd, float prevValue );
	void SqueezeCultureInfluence( Culture* culture, float influenceToAdd, float prevValue );

	int m_id = -1;
	float m_height = 0.f;
	float m_longitude = 0.f; // west is negative east is positive
	float m_latitude = 0.f; // north is negative south is positive
	Direction m_nearestSeaDir = Direction::NUM;
	float m_nearestSeaDistance = -1.f;
	//Vec2 m_nearestSeaPos = Vec2();
	ClimateType m_climate = ClimateType::None;
	float m_summerPrecipitation = 0.f;
	float m_winterPrecipitation = 0.f;
	float m_summerAvgTemperature = 0.f;
	float m_winterAvgTemperature = 0.f;
	float m_areaSize = 0.f;
	int m_totalPopulation = 0;
	//int m_tempResolvedPopulation = 0;
	//std::unordered_map<unsigned int, Population*> m_pops;
	Culture* m_majorCulture = nullptr;
	std::vector<std::pair<Culture*, float>> m_cultures;
	Religion* m_majorReligion = nullptr;
	std::vector<std::pair<Religion*, float>> m_religions;
	std::vector<City*> m_cities;
	std::vector<Town*> m_towns;
	ProductType m_productType = ProductType::None;
	Continent* m_continent = nullptr;
	Region* m_region = nullptr;
	Country* m_owner = nullptr;
	std::vector<Country*> m_legitimateCountries;
	std::string m_name = "default";
	float m_provinceArmyScore = 0.f;
	std::deque<std::pair<Army*, float>> m_armiesOnProv; // army ref, dist

	LandformType m_landform = LandformType::NUM;
	bool m_isCoast = false;
	bool m_isFarAwayFakeUnit = false;
	Vec2 m_centerPosition;
	Vec2 m_geoCenterPos;
	std::vector<MapPolygonUnit*> m_adjacentUnits;
	std::vector<MapPolygonUnit*> m_adjLowerUnits;
	std::vector<MapPolygonUnit*> m_adjKindOfLowerUnits;
	std::vector<StarEdge*> m_edges;
	River* m_riverOnThis = nullptr;
	Vec2 m_riverAnchorPos;
	ProvinceRoadData* m_roadData = nullptr;
	float m_roughRadius = 0.f;
	size_t m_startInVertexBuffer = 0;
	size_t m_sizeInVertexBuffer = 0;
	size_t m_startIn3DVertexBuffer = 0;
	size_t m_sizeIn3DVertexBuffer = 0;
	int m_cultureStepValue = -1;
	VertexBuffer* m_edgeShowingBuffer = nullptr;
	VertexBuffer* m_edgeShowingBuffer3D = nullptr;
	//bool m_consumeSeaStep = false;

	bool m_developmentFlag = false;
	bool m_warFlag = false;
	//bool m_isColorDirty = true;
	//bool m_isColorDirty2 = false;

	uint16_t m_dirtyBits = 0xffff;
protected:
	void SetLake();
	void SetIsland();
	void CalculateSeaDistance();
	void CalculateClimate();
	void CalculatePrecipitation();
	void CalculateTemperature();
	void SetLandform();
	float GetDistanceToSeaFromThisUnit( Vec2 const& pos ) const;
	Rgba8 m_curColor = Rgba8::WHITE;
	Rgba8 m_colorBeforeClick = Rgba8::WHITE;

};