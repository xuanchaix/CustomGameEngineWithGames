#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/RayCastUtils.hpp"
#include <vector>

struct AABB2;
struct Vertex_PCU;

class TileHeatMap {
public:
	TileHeatMap( IntVec2 const& dimensions );

	/// Set value of all tiles
	void SetAllValues( float newValue = 0.f );
	/// Set value of Tile(x, y)
	void SetTileValue( IntVec2 const& tileCoords, float newValue );
	/// Add a value to Tile(x, y)'s value
	void AddTileValue( IntVec2 const& tileCoords, float addValue );

	float GetTileValue( IntVec2 const& tileCoords ) const;
	float GetTileValue( IntVec2 const& tileCoords, float outOfBoundValue ) const;
	float GetMaxValueExceptSpecialValue( float specialValue ) const;
	
	//---------------------------------------------------------------------
	// 2D Ray Cast in grids, opaque value means tiles with that value can stop the ray and the ray will impact
	bool RayCastVsGrid2D( RayCastResult2D& out_rayCastRes, Ray2D const& ray2D, float opaqueValue = FLT_MAX ) const;
	//---------------------------------------------------------------------
	// 2D Ray Cast in grids, opaque value means tiles with that value can stop the ray and the ray will impact
	bool RayCastVsGrid2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, Vec2 const& forwardNormal, float maxDist, float opaqueValue = FLT_MAX ) const;
	//---------------------------------------------------------------------
	// 2D Ray Cast in grids, opaque value means tiles with that value can stop the ray and the ray will impact
	bool RayCastVsGrid2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, Vec2 const& endPos, float opaqueValue = FLT_MAX ) const;

	void AddVertsForDebugDraw( std::vector<Vertex_PCU>& verts, AABB2 const& bounds, FloatRange const& valueRange = FloatRange( 0.f, 1.f ),
		Rgba8 const& lowColor = Rgba8( 0, 0, 0, 100 ), Rgba8 const& highColor = Rgba8( 255, 255, 255, 255 ),
		float specialValue = FLT_MAX, Rgba8 const& specialColor = Rgba8( 51, 51, 255, 255 ) ) const;
	void AddTextVertsForDebugDraw( std::vector<Vertex_PCU>& verts, AABB2 const& bounds, float specialValue = FLT_MAX ) const;

private:
	IntVec2 const RoundDownPos( Vec2 const& pos ) const;
	bool IsCoordsInBounds( IntVec2 const& coords ) const;
private:
	IntVec2 m_dimensions = IntVec2( -1, -1 );
	std::vector<float> m_values;
};