#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Block.hpp"

constexpr int XBITS = 4;
constexpr int YBITS = 4;
constexpr int ZBITS = 7;
constexpr int XSIZE = 1 << XBITS;
constexpr int YSIZE = 1 << YBITS;
constexpr int ZSIZE = 1 << ZBITS;
constexpr int BLOCK_COUNT_EACH_CHUNK = 1 << XBITS << YBITS << ZBITS;
constexpr int XBITS_MASK = (int)((unsigned int)0xffffffff >> (32 - XBITS));
constexpr int YBITS_MASK = (int)(((unsigned int)0xffffffff >> (32 - YBITS)) << XBITS);
constexpr int ZBITS_MASK = (int)(((unsigned int)0xffffffff >> (32 - ZBITS)) << XBITS << YBITS);
constexpr int X_RESET_MASK = ~XBITS_MASK;
constexpr int Y_RESET_MASK = ~YBITS_MASK;
constexpr int Z_RESET_MASK = ~ZBITS_MASK;

constexpr int ZBITS_SHIFT = XBITS + YBITS;

struct BlockIter;
class BlockTemplate;

enum class ChunkState {
	MISSING, 
	ON_DISK,
	CONSTRUCTING,

	ACTIVATING_QUEUED_LOAD,
	ACTIVATING_LOADING,
	ACTIVATING_LOAD_COMPLETE,

	ACTIVATING_QUEUED_GENERATE,
	ACTIVATING_GENERATING,
	ACTIVATING_GENERATE_COMPLETED,

	ACTIVE,

	DEACTIVATING_QUEUED_SAVE,
	DEACTIVATING_SAVING,
	DEACTIVATING_SAVE_COMPLETED,
	DECONSTRUCTING,

	NUM,
};

class Chunk {
public:
	Chunk( IntVec2 const& coords );
	virtual ~Chunk();
	void StartUp();
	void Update( float deltaSeconds );
	void Render() const;

	void GenerateBlocks();
	void SetSkyLight();

	IntVec3 GetBlockWorldCoords( IntVec3 const& localCoords ) const;
	IntVec3 GetBlockLocalCoords( IntVec3 const& worldCoords ) const;
	BlockDefinition const& GetBlockDefinition( IntVec3 const& localCoords ) const;
	Vec2 GetWorldCenterXY() const;
	void DigTopBlockOfStack( IntVec2 const& localCoords );
	void PutTopBlockOfStack( IntVec2 const& localCoords );
	
	void SetBlockType( int blockIndex, unsigned char blockType, bool isPut=true );
	void SaveToFile() const;
	bool ReadFromFile();

	void MarkDirty();

	int GetBlockIndex( IntVec3 const& localCoords ) const;
	IntVec3 GetBlockLocalCoordsByIndex( int index ) const;

	void AddBlockTemplate( BlockTemplate const& temp, IntVec3 const& localOriginPos );
protected:
	bool IsCoordsInBounds( IntVec3 const& coords ) const;
	void BuildVertexArrayAndBuffer();
	bool IsBlockSurfaceHidden( IntVec3 const& coords, IntVec3 const& step ) const;
	Rgba8 CalculateLightColorForBlock( BlockIter const& blockIter ) const;

	void ClearBlocksOverlapSphere( Vec3 const& sphereCenter, float sphereRadius );
	void ClearBlocksOverlapCapsule( Vec3 const& capsuleStart, Vec3 const& capsuleEnd, float capsuleRadius );
public:
	bool m_isDirty = false;
	bool m_needsToSave = false;
	IntVec3 m_chunkOriginWorldCoords;
	AABB3 m_bounds;
	IntVec2 m_coords;
	std::vector<Vertex_PCU> m_verts;
	VertexBuffer* m_vertexBuffer = nullptr;
	Block* m_blocks;

	Vec2 m_worldCenterXY;

	Chunk* m_northNeighbor = nullptr;
	Chunk* m_southNeighbor = nullptr;
	Chunk* m_eastNeighbor = nullptr;
	Chunk* m_westNeighbor = nullptr;
	std::atomic<ChunkState> m_state = ChunkState::CONSTRUCTING;
};