#pragma once
#include "Game/GameCommon.hpp"
#include "Game/BlockIter.hpp"
#include <deque>
#include <unordered_set>
#include <set>
class Chunk;
class Player;
struct BlockDefinition;

extern unsigned int g_terrainSeed;
extern unsigned int g_hillinessSeed;
extern unsigned int g_humiditySeed;
extern unsigned int g_temperatureSeed;
extern unsigned int g_oceannessSeed;
extern unsigned int g_treeDensitySeed;
extern unsigned int g_treeSeed;
extern unsigned int g_wormSeed;

bool operator<( IntVec2 const& a, IntVec2 const& b );

struct GameRayCast3DRes : public RayCastResult3D {
	BlockIter m_res;
};

struct SimplerMinerConstants {
	Vec4 m_cameraWorldPos = Vec4( 0.f, 0.f, 0.f, 1.f );
	Vec4 m_indoorLightColor = Vec4( 1.0f, 0.9f, 0.8f, 1.f );
	Vec4 m_outdoorLightColor = Vec4( 1.f, 1.f, 1.f, 1.f );
	Vec4 m_skyColor = Vec4( 0.f, 0.f, 0.f, 1.f );
	float m_fogNearDist;
	float m_fogFarDist;
	float m_pad0;
	float m_pad1;
};

class SimpleMinerJob :public Job {
public:
	SimpleMinerJob( Chunk* chunk );
	virtual void Execute() = 0;
	Chunk* m_chunk = nullptr;
};

class ChunkGenerateJob : public SimpleMinerJob {
public:
	ChunkGenerateJob( Chunk* chunk );
	virtual void Execute() override;
};

class ChunkSaveJob : public SimpleMinerJob {
public:
	ChunkSaveJob( Chunk* chunk );
	virtual void Execute() override;
};

class ChunkLoadJob : public SimpleMinerJob {
public:
	ChunkLoadJob( Chunk* chunk );
	virtual void Execute() override;
};

constexpr int SIMPLE_MINER_WORLD_CONSTANTS_SLOT = 8;

class World {
public:
	World();
	virtual ~World();
	void StartUp();
	void Update();
	void Render() const;
	void RenderUI() const;

	Player* GetPlayer() const;
	Chunk* CreateChunk( IntVec2 const& chunkCoords );
	IntVec2 GetChunkCoordsByWorldPos( Vec2 const& worldXYPos ) const;
	size_t GetVertsCount() const;
	IntVec3 GetBlockCoordsByWorldPosition( Vec3 const& worldPos ) const;
	/// Note: this function is expensive
	BlockIter CreatBlockIter( Vec3 const& worldPos ) const;
	void MarkLightingDirty( BlockIter const& iter );
	float GetDayTimeFraction() const;
	bool IsTimeInNight() const;
	std::string GetCurDayTimeText() const;
protected:
	void DoChunkDynamicActivation();
	bool ActivateTheNearestInRangeChunk( IntVec2 const& playerChunkCoords );
	bool ActivateAllInRangeChunks( IntVec2 const& playerChunkCoords );
	bool DeactivateTheFarthestOutRangeChunk( IntVec2 const& playerChunkCoords );
	//void ActivateChunk( IntVec2 const& coords );
	void DeactivateChunk( Chunk* chunk );
	void StartUpChunk( Chunk* chunk );

	bool GetChunkByCoords( IntVec2 const& coords, Chunk** out_chunkPtr ) const;
	Chunk* GetChunkByCoords( IntVec2 const& coords ) const;
	IntVec2 GetPlayerCurrentChunkCoords() const;
	Vec2 GetChunkWorldCenterXY( IntVec2 const& chunkCoords ) const;

	void DigTheTopBlock();
	void PutTheTopBlock();
	void PutBlockToPos();
	void ChangeCurrentBlockType();

	void ProcessDirtyLighting();
	void ProcessNextDirtyLightBlock();
	void UndirtyAllBlocksInChunk( Chunk* chunk );

	bool RayCastVsWorld( GameRayCast3DRes& out_rayCastRes, Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist ) const;

	void UpdateTimeAndSkyColor();

	void DeconstructChunk(Chunk* chunk);
public:
	//std::vector<Chunk*> m_activeChunks;
	std::map<IntVec2, Chunk*> m_activeChunks;
	std::deque<BlockIter> m_blockLightingQueue;
	std::vector<SimpleMinerJob*> m_chunkGenerationJobs;
	std::vector<ChunkSaveJob*> m_chunkSaveJobs;
	std::vector<Chunk*> m_dirtyChunks;
	std::set<IntVec2> m_queuedGenerateChunks;

	float m_chunkActivationRange;
	float m_chunkDeactivationRange;
	int m_maxChunks; // neighborhood
	int m_maxChunksRadiusX;
	int m_maxChunksRadiusY;

	Player* m_player;

	unsigned char m_curBlockIDToPut = 1;

	bool m_hasRayHitRes = false;
	GameRayCast3DRes m_hitRes;
	bool m_rayLock = false;
	Vec3 m_rayStartPos;
	Vec3 m_rayForwardNormal;
	float m_rayMaxDist;

	float m_days = 0.f;
	float m_worldTimeScale = 200.f;
	float m_lightningStrength = 0.f;
	float m_glowStrength = 1.f;
	Rgba8 m_skyColor;

	BlockDefinition const& m_airDef;

	SimplerMinerConstants m_worldConstans;
	ConstantBuffer* m_worldCBO = nullptr;
	Shader* m_worldShader = nullptr;

};