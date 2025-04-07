#pragma once
#include "Game/GameCommon.hpp"

class Chunk;
struct Block;

struct BlockIter {

	BlockIter();
	BlockIter( int blockIndex, Chunk* chunk );

	Block* GetBlock() const;
	Vec3 GetWorldCenter() const;
	bool IsValid() const;

	BlockIter GetNorthNeighbor() const;
	BlockIter GetSouthNeighbor() const;
	BlockIter GetEastNeighbor() const;
	BlockIter GetWestNeighbor() const;
	BlockIter GetUpNeighbor() const;
	BlockIter GetDownNeighbor() const;

	Chunk* m_chunk = nullptr;
	int m_blockIndex = -1;

	int GetBlockLocalX() const;
	int GetBlockLocalY() const;
	int GetBlockLocalZ() const;
	int GetBlockWorldX() const;
	int GetBlockWorldY() const;
	int GetBlockWorldZ() const;

	bool IsOnWestEdge() const;
	bool IsOnEastEdge() const;
	bool IsOnSouthEdge() const;
	bool IsOnNorthEdge() const;
	bool IsOnUpEdge() const;
	bool IsOnDownEdge() const;

	bool HasEastNeighborChunk() const;
	bool HasWestNeighborChunk() const;
	bool HasSouthNeighborChunk() const;
	bool HasNorthNeighborChunk() const;

protected:
	int GetBlockRawX() const;
	int GetBlockRawY() const;
	int GetBlockRawZ() const;
};
