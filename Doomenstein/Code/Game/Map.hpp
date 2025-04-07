#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include "Game/Actor.hpp"
#include "Game/GameModes.hpp"

class Image;
class Shader;
class SpriteSheet;
class PlayerController;
class Actor;
class ActorUID;

struct SpawnInfo {
	std::string m_actorName;
	Vec3 m_position;
	EulerAngles m_orientation;
	Vec3 m_velocity;
};

struct MapDefinition {
	std::string m_mapName;
	std::string m_imagePath;
	std::string m_shaderName;
	std::string m_spriteTexturePath;
	std::string m_gameMode;
	bool m_hasPointLight = false;
	Vec3 m_pointLightPosition = Vec3();
	EulerAngles m_pointLightOrientation = EulerAngles();
	bool m_hasDirectionalLight = false;
	Vec3 m_directionalLightPosition = Vec3();
	float m_lightAmbient = 0.2f;
	EulerAngles m_directionalLightOrientation = EulerAngles();
	IntVec2 m_spriteSheetCellCount;
	std::vector<SpawnInfo> m_spawnInfos;
	std::string m_description;
};

struct RayCastResultDoomenstein : public RayCastResult3D {
	bool m_didHitActor = false;
	ActorUID m_uid;
};

class Map {
public:
	Map( MapDefinition const& mapDef );
	virtual ~Map();

	void Startup();
	void Update();
	void RenderWorld( Camera const& renderCamera ) const;
	void RenderUI() const;

	std::string const& GetTileType( int x, int y ) const;
	std::string const& GetTileType( IntVec2 const& mapPos ) const;
	TileDefinition const& GetTileDef( int x, int y ) const;
	TileDefinition const& GetTileDef( IntVec2 const& mapPos ) const;
	TileDefinition const& GetTileDef( std::string const& tileType ) const;
	TileDefinition const& GetTileDef( Rgba8 const& tileImageColor ) const;

	int AddActorToMap( Actor* a );
	void RemoveActorToMap( Actor* a );
	ActorUID SpawnActorToMap( ActorDefinition const& actorDef, Vec3 const& postion = Vec3( 0.f, 0.f, 0.f ), EulerAngles const& orientation = EulerAngles(), Vec3 const& velocity = Vec3() );
	ActorUID GetRandomEnemySpawnPoint();

	Actor* GetActorByUID( ActorUID const uid );
	Actor* GetNearestVisibleEnemy( ActorFaction enemyFaction, Actor* inquiryActor );
	Actor* GetNearestEnemy( ActorFaction enemyFaction, Actor* inquiryActor );
	Actor* GetNearestEnemyInSector( ActorFaction enemyFaction, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius );
	void PopulateHeatMapDistanceField( TileHeatMap& out_distanceField, IntVec2 const& startCoords, float maxCost = FLT_MAX, float minCost = 0.f ) const;
	void DebugPossessNext();
	void DebugKillAllExceptSelf( ActorUID self );

	bool IsPositionInBounds( Vec3 const& position, float const tolerance = 0.f ) const;
	bool IsCoordInBounds( IntVec2 const& coords ) const;
	bool IsCoordInBounds( int x, int y ) const;
	Tile const& GetTile( IntVec2 const& coords ) const;
	Tile const& GetTile( int x, int y ) const;
	IntVec2 const GetMapPosFromWorldPos( Vec3 worldPos ) const;

	RayCastResultDoomenstein const RayCastAll( Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist, Actor* ignoreActor=nullptr, ActorFaction ignoreFaction = ActorFaction::NONE ) const;
	RayCastResult3D const RayCastWorldXY( Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist ) const;

	void DealRangeDamage( Vec3 const& position, float range, FloatRange const& damage, ActorUID damageSource, bool isDamping = true );

private:
	void PopulateMap();
	void MakeMapVerts();
	void SetupActorsToMap();

	void UpdateAllActors();
	void DeleteGarbageActors();

	void RenderShadowMap( Camera const& renderCamera ) const;
	void RenderAllActors( Camera const& renderCamera ) const;
	void RenderTiles( Camera const& renderCamera ) const;

	void CollideAllActorsWithEachOther();
	void CollideTwoActors( Actor* a, Actor* b );
	void CollideAllActorsWithMap();
	void CollideActorWithMap( Actor* a );
	void CollideActorWithBlock( Actor* a, IntVec2 const& coords );

	RayCastResult3D const RayCastWorldZ( Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist ) const;
	RayCastResultDoomenstein const RayCastWorldAllActors( Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist, Actor* ignoreActor=nullptr, ActorFaction ignoreFaction=ActorFaction::NONE ) const;
public:
	GameMode* m_gameMode;
	std::string m_name;
	Image* m_mapImage;
	Shader* m_shader;
	SpriteSheet* m_terrainSpriteSheet;
	MapDefinition const* m_mapDef;
	IntVec2 m_dimensions;
	TileHeatMap* m_tileHeatMap;
	std::vector<Vertex_PCUTBN> m_vertices;
	std::vector<unsigned int> m_indexes;
	VertexBuffer* m_mapTileVBO;
	IndexBuffer* m_mapTileIBO;
	std::vector<Tile> m_tiles;
	std::vector<Actor*> m_actors;
	std::vector<ActorUID> m_playerStart;
	std::vector<ActorUID> m_enemySpawnPoints;
	unsigned int m_actorSalt = 1;
	int m_curPlayerActorIndex = -1;
	DirectionalLightConstants m_dirLightConsts;
	bool m_blockUpdate = false;

	Camera m_light;
};