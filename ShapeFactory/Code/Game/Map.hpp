#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include <functional>

class Building;
class Resource;
class Bridge;
class PowerBuilding;
class PowerNetwork;
class ProjectileDefinition;
class Projectile;
class EnemyBase;
class EnemyDefinition;

enum class MapUIState {
	ViewingMap, ConstructBuilding, BuildingSettings, 
};

class Map {
public:
	Map();
	virtual ~Map();
	void StartUp();
	void Update();
	void Render() const;
	void RenderUI() const;

	IntVec2 GetCoordsFromTileIndex( int index ) const;
	Tile* GetTileFromCoords( IntVec2 const& coords ) const;
	Building* GetBuildingFromCoords( IntVec2 const& coords ) const;
	bool IsCoordsInBounds( IntVec2 const& coords ) const;

	bool IsTileSolid( IntVec2 const& coords ) const;
	bool IsTileBuilding( IntVec2 const& coords ) const;

	IntVec2 GetMapCenter() const;
	IntVec2 GetTileCoordsFromPos( Vec2 const& pos ) const;

	bool CreateNewResource( int resourceID, Vec2 const& pos, Building* requestBuilding );
	void AddResourceToList( Resource* resource );

	Projectile* CreateNewProjectile( Vec2 const& pos, float orientationDegrees, ProjectileDefinition const& def );
	EnemyBase* SpawnNewEnemy( Vec2 const& pos, float orientationDegrees, EnemyDefinition const& def );
public:
	std::vector<Resource*> m_resources;
	int m_numOfResource[NumOfProductTypes] = {};
	bool m_debugRenderBuildingStats = true;
	std::vector<Entity*> m_entities;
protected:
	void SetupUI();
	void HandleKeys();
	void HandleResourceMovement();
	void HandleProduction();
	void UpdateAllPowerNetworks();
	void UpdateEntities();
	void CheckCollision();
	bool CheckResourceJumpQueue( Resource* resource ) const;
	void AddEntityToList( Entity* entity );
	void RemoveEntityFromList( Entity* entity );
	void RenderEntities() const;

	bool CheckIfConveyerCanBeBuilt( IntVec2 coords ) const;
	bool CheckIfDrillCanBeBuilt( IntVec2 coords ) const;
	bool CheckIfLogisticBuildingCanBeBuilt( IntVec2 coords ) const;
	bool CheckIfSelectorCanBeBuilt( IntVec2 coords ) const;
	bool CheckIfRouterCanBeBuilt( IntVec2 coords ) const;
	bool CheckIfOverflowGateCanBeBuilt( IntVec2 coords ) const;
	bool CheckIfWareHouseCanBeBuilt( IntVec2 coords ) const;
	bool CheckIfExporterCanBeBuilt( IntVec2 coords ) const;
	bool CheckIfJunctionCanBeBuilt( IntVec2 coords ) const;
	bool CheckIfBridgeCanBeBuilt( IntVec2 coords ) const;
	bool CheckIfPowerPlantCanBeBuilt( IntVec2 coords ) const;
	bool CheckIfPowerNodeCanBeBuilt( IntVec2 coords ) const;
	bool CheckIfBlenderCanBeBuilt( IntVec2 coords ) const;
	bool CheckIfRefineryCanBeBuilt( IntVec2 coords ) const;
	bool CheckIfTowerCanBeBuilt( IntVec2 coords ) const;
	void BuildNewConveyer( IntVec2 coords );
	void BuildNewDrill( IntVec2 coords );
	void BuildNewSelector( IntVec2 coords );
	void BuildNewRouter( IntVec2 coords );
	void BuildNewOverflowGate( IntVec2 coords );
	void BuildNewWareHouse( IntVec2 coords );
	void BuildNewExporter( IntVec2 coords );
	void BuildNewJunction( IntVec2 coords );
	void BuildNewBridge( IntVec2 coords );
	void BuildNewPowerPlant( IntVec2 coords );
	void BuildNewPowerNode( IntVec2 coords );
	void BuildNewPowerBuilding( PowerBuilding* building );
	void BuildNewBlender( IntVec2 coords );
	void BuildNewRefinery( IntVec2 coords );
	void BuildNewTower( IntVec2 coords );

	void RemoveNetwork( PowerNetwork* network );
	void RemoveBuilding( IntVec2 coords, bool collectResource = true );
	void RemoveResource( Resource* resourceToRemove );
	void RemoveResourceAndAddToAmount( Resource* resourceToRemove );
	void RemoveAllGarbageResource();
	void RemoveAllGarbageEntity();
	void RemovePowerBuilding( PowerBuilding* building );

	IntVec2 GetRandomTileCoordsOnMap();
	IntVec2 GetRandomTileCoordsInRange( IntVec2 const& minInclusive, IntVec2 const& maxInclusive );
	void DoWormRandomTileGeneration( TileDefinition const& tile, IntVec2 const& startCoords, int numOfSteps );
	void DoPerfectWormRandomTileGeneration( TileDefinition const& tile, IntVec2 const& startCoords, int numOfSteps );
	bool DoEntitiesHaveInteraction( Entity* a, Entity* b ) const;

	void PopulateTileHeatMapDistanceField( TileHeatMap& out_distanceField, IntVec2 const& startCoords, float maxCost, float minCost, bool treatBuildingsAsSolid ) const;

	void DoConstruction( IntVec2 const& coords );
	Vec2 ConvertWorldPosToScreen( Vec2 const& worldPos ) const;

	void SetBuildingSettingsUIActive( SFWidget* widget, Vec2 const& center );
	void DeActiveAllUIWidget();
	void SetConstructionPanelActive( bool active );
protected:
	IntVec2 m_dimensions;
	std::vector<Tile> m_tiles;
	std::map<IntVec2, Building*> m_buildings;
	std::vector<Building*> m_buildingList;
	VertexBuffer* m_tileMapVertexBuffer = nullptr;
	VertexBuffer* m_stoneTerrainVertexBuffer = nullptr;
	VertexBuffer* m_ironTerrainVertexBuffer = nullptr;
	VertexBuffer* m_copperTerrainVertexBuffer = nullptr;
	VertexBuffer* m_coalTerrainVertexBuffer = nullptr;
	VertexBuffer* m_catalystTerrainVertexBuffer = nullptr;
	VertexBuffer* m_wallVertexBuffer = nullptr;
	RandomNumberGenerator m_mapRNG;
	TileHeatMap* m_heatMapToBase = nullptr;
	Timer testTimer;
	Direction m_curPreBuildingDir = Direction::Right;
	BuildingType m_curPreBuilding = BuildingType::None;
	std::vector<PowerNetwork*> m_powerNetworks;

	IntVec2 m_cursorTileCoords;
	IntVec2 m_cursorLastFrameTileCoords;

	MapUIState m_UIState = MapUIState::ViewingMap;

	std::vector<SFWidget*> m_curActiveWidget;

	Bridge* m_lastBuiltBridge = nullptr;
// UI event functions
protected:
	static bool UISelectionBuildBuilding( EventArgs& args );
};