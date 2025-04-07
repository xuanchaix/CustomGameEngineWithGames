#include "Game/BlockIter.hpp"
#include "Game/Chunk.hpp"

BlockIter::BlockIter( int blockIndex, Chunk* chunk )
	:m_blockIndex(blockIndex)
	,m_chunk(chunk)
{

}

BlockIter::BlockIter()
{

}

Block* BlockIter::GetBlock() const
{
	return &m_chunk->m_blocks[m_blockIndex];
}

Vec3 BlockIter::GetWorldCenter() const
{
	IntVec3 worldCoords = m_chunk->GetBlockWorldCoords( GetBlockLocalCoordsByIndex( m_blockIndex ) );
	return Vec3( worldCoords.x + 0.5f, worldCoords.y + 0.5f, worldCoords.z + 0.5f );
}

bool BlockIter::IsValid() const
{
	return m_chunk != nullptr;
}

BlockIter BlockIter::GetNorthNeighbor() const
{
	// to next chunk
	if (GetBlockRawY() == YBITS_MASK) {
		//if (m_chunk->m_northNeighbor) {
		return BlockIter( m_blockIndex & Y_RESET_MASK, m_chunk->m_northNeighbor );
		//}
		//else {
		//	return BlockIter();
		//}
	}
	else {
		// Y + 1
		return BlockIter( m_blockIndex + XSIZE, m_chunk );
	}
}

BlockIter BlockIter::GetSouthNeighbor() const
{
	// to next chunk
	if (GetBlockRawY() == 0) {
		//if (m_chunk->m_southNeighbor) {
		return BlockIter( m_blockIndex | YBITS_MASK, m_chunk->m_southNeighbor );
		//}
		//else {
		//	return BlockIter();
		//}
	}
	else {
		// Y - 1
		return BlockIter( m_blockIndex - XSIZE, m_chunk );
	}
}

BlockIter BlockIter::GetEastNeighbor() const
{
	// to next chunk
	if (GetBlockRawX() == XBITS_MASK) {
		//if (m_chunk->m_eastNeighbor) {
		return BlockIter( m_blockIndex & X_RESET_MASK, m_chunk->m_eastNeighbor );
		//}
		//else {
		//	return BlockIter();
		//}
	}
	else {
		// X + 1
		return BlockIter( m_blockIndex + 1, m_chunk );
	}
}

BlockIter BlockIter::GetWestNeighbor() const
{
	// to next chunk
	if (GetBlockRawX() == 0) {
		//if (m_chunk->m_westNeighbor) {
		return BlockIter( m_blockIndex | XBITS_MASK, m_chunk->m_westNeighbor );
		//}
		//else {
		//	return BlockIter();
		//}
	}
	else {
		// X - 1
		return BlockIter( m_blockIndex - 1, m_chunk );
	}
}

BlockIter BlockIter::GetUpNeighbor() const
{
	// top most, no block above
	if (GetBlockRawZ() == ZBITS_MASK) {
		return BlockIter();
	}
	else {
		return BlockIter( m_blockIndex + (1 << ZBITS_SHIFT), m_chunk );
	}
}

BlockIter BlockIter::GetDownNeighbor() const
{
	// down most, no block above
	if (GetBlockRawZ() == 0) {
		return BlockIter();
	}
	else {
		return BlockIter( m_blockIndex - (1 << ZBITS_SHIFT), m_chunk );
	}
}

int BlockIter::GetBlockRawX() const
{
	return m_blockIndex & XBITS_MASK;
}

int BlockIter::GetBlockRawY() const
{
	return m_blockIndex & YBITS_MASK;
}

int BlockIter::GetBlockRawZ() const
{
	return m_blockIndex & ZBITS_MASK;
}

int BlockIter::GetBlockLocalX() const 
{
	return m_blockIndex & XBITS_MASK;
}

int BlockIter::GetBlockLocalY() const 
{
	return (int)((unsigned int)(m_blockIndex & YBITS_MASK) >> XBITS);
}

int BlockIter::GetBlockLocalZ() const 
{
	return (int)((unsigned int)m_blockIndex >> ZBITS_SHIFT);
}

int BlockIter::GetBlockWorldX() const
{
	return m_chunk->m_chunkOriginWorldCoords.x + GetBlockLocalX();
}

int BlockIter::GetBlockWorldY() const
{
	return m_chunk->m_chunkOriginWorldCoords.y + GetBlockLocalY();
}

int BlockIter::GetBlockWorldZ() const
{
	return m_chunk->m_chunkOriginWorldCoords.z + GetBlockLocalZ();
}

bool BlockIter::IsOnWestEdge() const
{
	return GetBlockLocalX() == 0;
}

bool BlockIter::IsOnEastEdge() const
{
	return GetBlockLocalX() == XBITS_MASK;
}

bool BlockIter::IsOnSouthEdge() const
{
	return GetBlockRawY() == 0;
}

bool BlockIter::IsOnNorthEdge() const
{
	return GetBlockRawY() == YBITS_MASK;
}

bool BlockIter::IsOnUpEdge() const
{
	return GetBlockRawZ() == ZBITS_MASK;
}

bool BlockIter::IsOnDownEdge() const
{
	return GetBlockRawZ() == 0;
}

bool BlockIter::HasEastNeighborChunk() const
{
	return m_chunk->m_eastNeighbor != nullptr;
}

bool BlockIter::HasWestNeighborChunk() const
{
	return m_chunk->m_westNeighbor != nullptr;
}

bool BlockIter::HasSouthNeighborChunk() const
{
	return m_chunk->m_southNeighbor != nullptr;
}

bool BlockIter::HasNorthNeighborChunk() const
{
	return m_chunk->m_northNeighbor != nullptr;
}
