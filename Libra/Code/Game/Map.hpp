#pragma once
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>

struct Vec2;
class PlayerTank;
class SpriteSheet;
class TileHeatMap;
struct Ray2D;
struct RayCastResult2D;
enum class AudioName;

enum class MapState { PLAYING, PLAYER_DEAD, WIN, ENTER_MAP, EXIT_MAP, FINISH_EXIT };
enum class MapRenderState { NORMAL, DISTANCE_MAP, SOLID_MAP, AMPHIBIOUS_MAP, ENEMY_PATH_FINDING_MAP, NUM };
enum class TaskType { Default = -1, Breakthrough, Defense, Clear, Capture };

typedef std::vector<Entity*> EntityList;

Rgba8 const ENEMY_SCORPIO = Rgba8( 255, 0, 0, 255 );
Rgba8 const ENEMY_LEO = Rgba8( 155, 0, 0, 255 );
Rgba8 const ENEMY_ARIES = Rgba8( 55, 0, 0, 255 );
Rgba8 const ENEMY_CAPRICORN = Rgba8( 255, 100, 100, 255 );
Rgba8 const ENEMY_CANCER = Rgba8( 255, 200, 200, 255 );
Rgba8 const ALLY_SCORPIO = Rgba8( 0, 0, 255, 255 );
Rgba8 const ALLY_LEO = Rgba8( 0, 0, 155, 255 );
Rgba8 const ALLY_ARIES = Rgba8( 0, 0, 55, 255 );
Rgba8 const ALLY_CAPRICORN = Rgba8( 100, 100, 255, 255 );
Rgba8 const ALLY_CANCER = Rgba8( 200, 200, 255, 255 );

struct EnemyReinfDef {
	float m_timeToCome;
	IntVec2 m_tileToCome;
	int m_numOfAries = 0;
	int m_numOfLeo = 0;
	int m_numOfCancer = 0;
	int m_numOfCapricorn = 0;
};

struct MapDefinition {
	std::string m_mapName;
	IntVec2 m_dimensions = IntVec2( 0, 0 );
	int m_numOfLeo = 0;
	int m_numOfScorpio = 0;
	int m_numOfAries = 0;
	int m_numOfCapricorn = 0;
	int m_numOfCancer = 0;
	std::string m_fillTile = "DEFAULT";
	std::string m_borderTile = "DEFAULT";
	std::string m_bunkerWallTile = "DEFAULT";
	std::string m_bunkerFloorTile = "DEFAULT";
	std::string m_worm1Tile = "DEFAULT";
	int m_worm1Count = 0;
	int m_worm1MaxLength = 0;
	std::string m_worm2Tile = "DEFAULT";
	int m_worm2Count = 0;
	int m_worm2MaxLength = 0;
	std::string m_worm3Tile = "DEFAULT";
	int m_worm3Count = 0;
	int m_worm3MaxLength = 0;

	std::string m_mapImageName;
	IntVec2 m_mapImageOffset;
	std::string m_startEntityImagePath;

	int m_addReinforcements = 0;
	std::vector<int> m_reinforcements;
	std::string m_task1Type = "Breakthrough";
	float m_task1Time = 0.f;

	int m_enemyReinforcements = 0;
	std::vector<EnemyReinfDef> m_enemyReinfWaveDefs;
};


class Map {
public:
	Map();
	~Map();

	void Startup( MapDefinition const& config );
	void Update( float deltaTime );
	void Render() const;

	MapDefinition const& GetMapDef() const;
	IntVec2 const GetMapPosFromWorldPos( Vec2 const& worldPos ) const;
	Vec2 const GetWorldPosFromMapPosCenter( IntVec2 const& mapPos ) const;
	Vec2 const GetWorldPosFromMapPosLeftBottom( IntVec2 const& mapPos ) const;
	std::string const& GetTileType( int x, int y ) const;
	std::string const& GetTileType( IntVec2 const& mapPos ) const;
	TileDefinition const& GetTileDef( int x, int y ) const;
	TileDefinition const& GetTileDef( IntVec2 const& mapPos ) const;
	TileDefinition const& GetTileDef( std::string const& tileType ) const;
	TileDefinition const& GetTileDef( Rgba8 const& tileImageColor ) const;
	IntVec2 const& GetDimensions() const;
	Entity* GetNearestEnemyActor( Entity const* inquiryer );
	//------------------------------
	Vec2 const GetNextPosGoTo( Vec2 const& startPos, Vec2 const& targetPosition, TileHeatMap const& tileHeatMap ) const;
	Vec2 const GetNextPosGoTo( Entity const* inquiryer, Vec2 const& targetPosition, TileHeatMap const& tileHeatMap ) const;
	//------------------------------
	// return next tile center world position inquiryer should go to
	Vec2 const GetDistanceTileHeatMapForTargetPos( Entity const* inquiryer, Vec2 const& targetPosition, TileHeatMap& out_tileHeatMap ) const;
	//------------------------------
	// return next tile center world position inquiryer should go to
	Vec2 const GetRandomWonderPosAndDistanceTileHeatMap( Entity const* inquiryer, Vec2& out_WonderPos, TileHeatMap& out_tileHeatMap ) const;
	//------------------------------
	// generate a reversed path of points to goal 
	void GenerateEntityPathToGoal( std::vector<Vec2>& out_pathPoints, TileHeatMap const& tileHeatMap, Vec2 const& startPos, Vec2 const& targetPos ) const;
	//------------------------------
	Entity* SpawnNewEntity(EntityType type, Vec2 const& position, EntityFaction faction, Entity const* spawner = nullptr, float orientationDegrees = 0.f);
	void SpawnExplosion( Vec2 const& position, float sizeFactor, float lifeSpanSeconds );
	void CallReinforcemets( Entity* caller );
	void AddEntityToMap( Entity& e );
	bool RemoveEntityFromMap( Entity& e );
	PlayerTank* GetPlayer() const;
	void SetPlayer(PlayerTank* player);
	Vec2 const GetEnterPoint() const;

	TileHeatMap const* GetAmphibiousTileHeatMap() const;

	bool IsPointInSolid( Vec2 const& pointWorldPos ) const;
	bool IsTileSolid( IntVec2 const& mapPos ) const;
	bool IsTileWater( IntVec2 const& mapPos ) const;
	bool IsTileScorpio( IntVec2 const& mapPos ) const;
	bool IsTileDestructible( IntVec2 const& mapPos ) const;
	void RaycastVsTiles( Ray2D const& rayInfo, RayCastResult2D& outRayCastResult, TileHeatMap const& tileHeatMap, float opaqueValue = FLT_MAX ) const;
	bool HasLineOfSight( Entity const* rayShooter, Entity const* target, RayCastResult2D& outRayCastResult, TileHeatMap const& tileHeatMap, float opaqueValue = FLT_MAX ) const;
	bool HasLineOfSight( Vec2 const& startPos, Vec2 const& targetPos, RayCastResult2D& outRayCastResult, TileHeatMap const& tileHeatMap, float opaqueValue = FLT_MAX ) const;

	bool PlaySound( AudioName name, Vec2 const& playPosition, float intervalTimeSeconds = 0.f, bool isLooped = false, float volume = 1.f, float speed = 1.f );
private:
	void PopulateMap( MapDefinition const& config );
	void LoadEnemy( MapDefinition const& config );
	void GrowWorm( std::string const& tileType, int wormCount, int wormMaxLength );

	void UpdateState( float deltaTime );
	void UpdateEntityLists( float deltaTime );
	void UpdateEntityList( EntityList& entityArray, float deltaTime );
	void UpdateCamera();
	void DeleteGarbage();
	void DeleteGarbageInEntityList( Entity* deletedEntity, EntityList& entityArray );

	void RenderEntityLists() const;
	void RenderEntityList( EntityList const& entityArray ) const;
	void RenderTilemap() const;
	void RenderPauseMode() const;
	void RenderEnterExitMode() const;
	void RenderUI() const;

	void HandleKeys();
	void HandleTasks();

	void PopulateHeatMapDistanceField( TileHeatMap& out_distanceField, IntVec2 const& startCoords, float maxCost = FLT_MAX, float minCost = 0.f, bool treatWaterAsSolid = true, bool treatScorpioAsSolid = false ) const;
	void BuildSolidTileHeatMap();
	void BuildAmphibiousTileHeatMap();

	void AddEntityToAllEntityLists( Entity* entity );
	void AddEntityToEntityList( Entity* entity, EntityList& entityList );

	void SpawnEnemyReinforcemets( EnemyReinfDef const& reinfDef );

	void ChangeTileType( int x, int y, TileDefinition const& newType );
	void ChangeTileType( IntVec2 const& pos, TileDefinition const& newType );
	void TileGetDamage( IntVec2 mapPos, float damage );
	Tile const& GetTile( IntVec2 mapPos );

	IntVec2 const GetXYPosFromArrayIndex( int i ) const;
	bool IsIndexInBound( IntVec2 index ) const;

	// collisions
	void AllEntitiesCorrectCollisionWithEachOther( float deltaTime );
	void DealCorrectCollision( Entity& a, Entity& b );
	void AllEntitiesCorrectCollisionWithWall();
	void EntityCorrectCollisionWithWall( Entity* entity );
	void EntityCorrectCollisionWithOneTile( Entity* entity, IntVec2 const& tile );
	void BulletCorrectCollisionWithWall( Bullet* entity );
	void CheckBulletCollision();
public:
	MapState m_curMapState = MapState::ENTER_MAP;
	float m_enterExitTimer = 0.f;
	bool m_doControllerShake = false;

private:
	MapDefinition const* m_mapDef = nullptr;
	IntVec2 m_dimensions = IntVec2( 0, 0 );
	std::vector<Tile> m_tiles;
	EntityList m_allEntityList;
	EntityList m_entityListsByType[(int)EntityType::NUM];
	EntityList m_actorListsByFaction[(int)EntityFaction::NUM];

	Camera m_worldCamera;
	Camera m_screenCamera;

	PlayerTank* m_playerTank = nullptr;
	bool m_isDebugMode = false;
	bool m_isHighCameraMode = false;
	bool m_isNoClipMode = false;
	bool m_doScreenShake = false;

	float m_toRespwanModeTime = 3.f;
	Texture* m_deadScreen = nullptr;
	Texture* m_winScreen = nullptr;

	Vec2 m_exitPoint;
	Vec2 m_enterPoint = Vec2( 1.5f, 1.5f );

	SpriteSheet* m_tileSpriteSheet = nullptr;
	Texture* m_tileSpriteTexture = nullptr;

	TileHeatMap* m_startToEndHeatMap = nullptr;
	TileHeatMap* m_solidTileHeatMap = nullptr;
	TileHeatMap* m_amphibiousTileHeatMap = nullptr;

	MapRenderState m_mapRenderState = MapRenderState::NORMAL;
	int m_thisRenderHeatMapEntityIndex = 0;

	int m_curReinforcements = 0;
	int m_curEnemyReinforcements = 0;

	TaskType m_taskType = TaskType::Breakthrough;

	float m_timerFromStart = 0.f;
	float m_timerForTask;

	Entity* m_theBuilding = nullptr;
	Vec2 m_captureCenter;
};
