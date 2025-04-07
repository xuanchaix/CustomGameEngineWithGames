#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Chunk.hpp"

SpriteSheet* g_sprite = nullptr;
bool g_hiddenSurfaceRemovalEnable = true;
float g_chunkActivationDist = 250.f;
bool g_autoCreateChunks = true;
bool g_saveModifiedChunks = true;

void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color ) {
	constexpr int NUM_SIDES = 16;
	constexpr int NUM_TRIANGLES = NUM_SIDES * 2;
	constexpr int NUM_VERTS = 3 * NUM_TRIANGLES;
	constexpr float DEGREES_PER_SIDE = 360.f / (float)NUM_SIDES;
	constexpr float RADIANS_PER_SIDE = DEGREES_PER_SIDE * PI / 180.f;
	Vertex_PCU ringVertsArray[NUM_VERTS];
	for (int i = 0; i < NUM_SIDES; i++) {
		ringVertsArray[6 * i] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * radius, SinRadians( RADIANS_PER_SIDE * i ) * radius, 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 1] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 2] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 3] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 4] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 5] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness), 0 ), color, Vec2( 0, 0 ) );
	}
	for (int i = 0; i < NUM_VERTS; i++) {
		ringVertsArray[i].m_position += Vec3( center.x, center.y, 0 );
	}
	g_theRenderer->DrawVertexArray( NUM_VERTS, ringVertsArray );
}

void DebugDrawLine(Vec2 const& startPos, float orientation, float length, float thickness, Rgba8 const& color) {
	Vertex_PCU lineVertsArray[6];
	Vec3 p1 = Vec3( -thickness * 0.5f, -thickness * 0.5f, 0 );
	Vec3 p2 = Vec3( -thickness * 0.5f, thickness * 0.5f, 0 );
	Vec3 p3 = Vec3( length + thickness * 0.5f, -thickness * 0.5f, 0 );
	Vec3 p4 = Vec3( length + thickness * 0.5f, thickness * 0.5f, 0 );
	lineVertsArray[0] = Vertex_PCU( p1, color, Vec2( 0, 0 ) );
	lineVertsArray[1] = Vertex_PCU( p2, color, Vec2( 0, 0 ) );
	lineVertsArray[2] = Vertex_PCU( p3, color, Vec2( 0, 0 ) );
	lineVertsArray[3] = Vertex_PCU( p2, color, Vec2( 0, 0 ) );
	lineVertsArray[4] = Vertex_PCU( p3, color, Vec2( 0, 0 ) );
	lineVertsArray[5] = Vertex_PCU( p4, color, Vec2( 0, 0 ) );

	TransformVertexArrayXY3D( 6, lineVertsArray, 1.f, orientation, startPos );
	g_theRenderer->DrawVertexArray( 6, lineVertsArray );
}

void DebugDrawLine( Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color )
{
	Vec2 dForward = (endPos - startPos).GetNormalized();
	Vec2 forward = dForward * thickness * 0.5f;
	Vec2 left = dForward.GetRotated90Degrees() * thickness * 0.5f;
	Vertex_PCU lineVertsArray[6];
	Vec3 p1 = Vec3( (startPos - forward + left).x, (startPos - forward + left).y, 0 );
	Vec3 p2 = Vec3( (startPos - forward - left).x, (startPos - forward - left).y, 0 );
	Vec3 p3 = Vec3( (endPos + forward + left).x, (endPos + forward + left).y, 0 );
	Vec3 p4 = Vec3( (endPos + forward - left).x, (endPos + forward - left).y, 0 );
	lineVertsArray[0] = Vertex_PCU( p1, color, Vec2( 0, 0 ) );
	lineVertsArray[1] = Vertex_PCU( p2, color, Vec2( 0, 0 ) );
	lineVertsArray[2] = Vertex_PCU( p3, color, Vec2( 0, 0 ) );
	lineVertsArray[3] = Vertex_PCU( p2, color, Vec2( 0, 0 ) );
	lineVertsArray[4] = Vertex_PCU( p3, color, Vec2( 0, 0 ) );
	lineVertsArray[5] = Vertex_PCU( p4, color, Vec2( 0, 0 ) );

	g_theRenderer->DrawVertexArray( 6, lineVertsArray );
}

RandomNumberGenerator* GetRNG()
{
	return g_theGame->m_randNumGen;
}

int GetBlockIndex( IntVec3 const& localCoords )
{
	return (((localCoords.z << YBITS) | localCoords.y) << XBITS) | localCoords.x;
	//return localCoords.x | (localCoords.y << XBITS) | (localCoords.z << ZBITS_SHIFT);
}

IntVec3 GetBlockLocalCoordsByIndex( int index )
{
	return IntVec3( index & XBITS_MASK, (int)((unsigned int)(index & YBITS_MASK) >> XBITS), (int)((unsigned int)index >> ZBITS_SHIFT) );
}