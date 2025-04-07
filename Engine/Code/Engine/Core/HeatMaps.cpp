#include "Engine/Core/HeatMaps.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include <string>

TileHeatMap::TileHeatMap( IntVec2 const& dimensions )
	:m_dimensions( dimensions )
{
	m_values.resize( (size_t)dimensions.x * dimensions.y );
	SetAllValues( 0.f );
}

void TileHeatMap::SetAllValues( float newValue /*= 0.f */ )
{
	for (int i = 0; i < (int)m_values.size(); i++) {
		m_values[i] = newValue;
	}
}

void TileHeatMap::SetTileValue( IntVec2 const& tileCoords, float newValue )
{
	m_values[(size_t)tileCoords.x + (size_t)tileCoords.y * m_dimensions.x] = newValue;
}

void TileHeatMap::AddTileValue( IntVec2 const& tileCoords, float addValue )
{
	m_values[(size_t)tileCoords.x + (size_t)tileCoords.y * m_dimensions.x] += addValue;
}

float TileHeatMap::GetTileValue( IntVec2 const& tileCoords ) const
{
	return m_values[(size_t)tileCoords.x + (size_t)tileCoords.y * m_dimensions.x];
}

float TileHeatMap::GetTileValue( IntVec2 const& tileCoords, float outOfBoundValue ) const
{
	if (tileCoords.x >= 0 && tileCoords.x < m_dimensions.x && tileCoords.y >= 0 && tileCoords.y < m_dimensions.y) {
		return m_values[(size_t)tileCoords.x + (size_t)tileCoords.y * m_dimensions.x];
	}
	return outOfBoundValue;
}

float TileHeatMap::GetMaxValueExceptSpecialValue( float specialValue ) const
{
	float retMaxValue = -FLT_MAX;
	for (float f : m_values) {
		if (f > retMaxValue && f != specialValue) {
			retMaxValue = f;
		}
	}
	return retMaxValue;
}

bool TileHeatMap::RayCastVsGrid2D( RayCastResult2D& out_rayCastRes, Ray2D const& ray2D, float opaqueValue /*= FLT_MAX */ ) const
{
	return RayCastVsGrid2D( out_rayCastRes, ray2D.m_startPos, ray2D.m_forwardNormal, ray2D.m_maxDist, opaqueValue );
}

bool TileHeatMap::RayCastVsGrid2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, Vec2 const& forwardNormal, float maxDist, float opaqueValue /*= FLT_MAX */ ) const
{
	// record the ray information
	out_rayCastRes.m_rayForwardNormal = forwardNormal;
	out_rayCastRes.m_rayMaxLength = maxDist;
	out_rayCastRes.m_rayStartPos = startPos;
	// calculate current tile
	IntVec2 curTile = RoundDownPos( startPos );
	if (!IsCoordsInBounds( curTile )) {
		out_rayCastRes.m_didImpact = false;
		return false;
	}
	// if current tile is solid
	if (GetTileValue( curTile ) == opaqueValue) {
		out_rayCastRes.m_didImpact = true;
		out_rayCastRes.m_impactDist = 0.f;
		out_rayCastRes.m_impactNormal = -forwardNormal;
		out_rayCastRes.m_impactPos = startPos;
		return true;
	}
	// calculate essential variables
	float fwdDistPerXCrossing = 1 / abs( forwardNormal.x );
	int tileStepDirectionX = forwardNormal.x < 0 ? -1 : 1;
	float xAtFirstXCrossing = curTile.x + ((float)tileStepDirectionX + 1.f) * 0.5f;
	float xDistToFirstXCrossing = xAtFirstXCrossing - startPos.x;
	float fwdDistAtNextXCrossing = abs( xDistToFirstXCrossing ) * fwdDistPerXCrossing;

	float fwdDistPerYCrossing = 1 / abs( forwardNormal.y );
	int tileStepDirectionY = forwardNormal.y < 0 ? -1 : 1;
	float yAtFirstYCrossing = curTile.y + ((float)tileStepDirectionY + 1.f) * 0.5f;
	float yDistToFirstYCrossing = yAtFirstYCrossing - startPos.y;
	float fwdDistAtNextYCrossing = abs( yDistToFirstYCrossing ) * fwdDistPerYCrossing;

	for (;;) {
		// if first hit y(vertical) side
		if (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing) {
			// if reach max distance
			if (fwdDistAtNextXCrossing > maxDist) {
				out_rayCastRes.m_didImpact = false;
				out_rayCastRes.m_impactDist = maxDist;
				out_rayCastRes.m_impactNormal = Vec2( 0.f, 0.f );
				out_rayCastRes.m_impactPos = startPos + forwardNormal * maxDist;
				return false;
			}
			// forward step
			curTile.x += tileStepDirectionX;
			// ray cast hit opaque!
			if (GetTileValue( curTile ) == opaqueValue) {
				out_rayCastRes.m_didImpact = true;
				out_rayCastRes.m_impactDist = fwdDistAtNextXCrossing;
				out_rayCastRes.m_impactNormal = Vec2( -(float)tileStepDirectionX, 0.f );
				out_rayCastRes.m_impactPos = forwardNormal * out_rayCastRes.m_impactDist + startPos;
				return true;
			}
			// pass this tile
			fwdDistAtNextXCrossing += fwdDistPerXCrossing;
		}
		// if first hit x(horizontal) side
		else {
			// if reach max distance
			if (fwdDistAtNextYCrossing > maxDist) {
				out_rayCastRes.m_didImpact = false;
				out_rayCastRes.m_impactDist = maxDist;
				out_rayCastRes.m_impactNormal = Vec2( 0.f, 0.f );
				out_rayCastRes.m_impactPos = startPos + forwardNormal * maxDist;
				return false;
			}
			// forward step
			curTile.y += tileStepDirectionY;
			// ray cast hit opaque!
			if (GetTileValue( curTile ) == opaqueValue) {
				out_rayCastRes.m_didImpact = true;
				out_rayCastRes.m_impactDist = fwdDistAtNextYCrossing;
				out_rayCastRes.m_impactNormal = Vec2( -(float)tileStepDirectionY, 0.f );
				out_rayCastRes.m_impactPos = forwardNormal * out_rayCastRes.m_impactDist + startPos;
				return true;
			}
			// pass this tile
			fwdDistAtNextYCrossing += fwdDistPerYCrossing;
		}
	}
}

bool TileHeatMap::RayCastVsGrid2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, Vec2 const& endPos, float opaqueValue /*= FLT_MAX */ ) const
{
	Vec2 forwardVector = endPos - startPos;
	float length = forwardVector.GetLength();
	return RayCastVsGrid2D( out_rayCastRes, startPos, forwardVector / length, length, opaqueValue );
}

void TileHeatMap::AddVertsForDebugDraw( std::vector<Vertex_PCU>& verts, AABB2 const& bounds, FloatRange const& valueRange,
	Rgba8 const& lowColor, Rgba8 const& highColor,
	float specialValue, Rgba8 const& specialColor) const
{
	int x = 0, y = 0;
	Vec2 leftBottomCorner = bounds.m_mins;
	Vec2 rightTopCorner = bounds.m_maxs;
	float widthPerTile = (rightTopCorner.x - leftBottomCorner.x) / (float)m_dimensions.x;
	float heightPerTile = (rightTopCorner.y - leftBottomCorner.y) / (float)m_dimensions.y;
	for (size_t i = 0; i < m_values.size(); i++) {
		if (m_values[i] == specialValue) {
			AddVertsForAABB2D( verts, 
				AABB2( leftBottomCorner + Vec2( x * widthPerTile, y * heightPerTile ), leftBottomCorner + Vec2( (x + 1) * widthPerTile, (y + 1) * heightPerTile ) ),
				specialColor, AABB2::IDENTITY );
		}
		else {
			AddVertsForAABB2D( verts, 
				AABB2( leftBottomCorner + Vec2( x * widthPerTile, y * heightPerTile ), leftBottomCorner + Vec2( (x + 1) * widthPerTile, (y + 1) * heightPerTile ) ),
				Rgba8::Interpolate( lowColor, highColor, RangeMapClamped( m_values[i], valueRange.m_min, valueRange.m_max, 0.f, 1.f ) ),
				AABB2::IDENTITY );
		}
		x++;
		if (x > m_dimensions.x - 1) {
			x = 0;
			y++;
		}
	}
}

//------------------------------------------------
// Only for test, make the game too slow
void TileHeatMap::AddTextVertsForDebugDraw( std::vector<Vertex_PCU>& verts, AABB2 const& bounds, float specialValue /*= FLT_MAX */ ) const
{
	int x = 0, y = 0;
	Vec2 leftBottomCorner = bounds.m_mins;
	Vec2 rightTopCorner = bounds.m_maxs;
	float widthPerTile = (rightTopCorner.x - leftBottomCorner.x) / (float)m_dimensions.x;
	float heightPerTile = (rightTopCorner.y - leftBottomCorner.y) / (float)m_dimensions.y;
	for (size_t i = 0; i < m_values.size(); i++) {
		if (m_values[i] == specialValue) {
			AddVertsForTextTriangles2D( verts, "NA", leftBottomCorner + Vec2( (x + 0.2f) * widthPerTile, (y + 0.2f) * heightPerTile),
				heightPerTile * 0.5f, Rgba8( 255, 0, 0, 255 ) );
		}
		else {
			AddVertsForTextTriangles2D( verts, std::to_string( RoundDownToInt(m_values[i]) ), leftBottomCorner + Vec2( (x + 0.2f) * widthPerTile, (y + 0.2f) * heightPerTile ),
				heightPerTile * 0.6f, Rgba8( 255, 0, 0, 255 ) );
		}
		x++;
		if (x > m_dimensions.x - 1) {
			x = 0;
			y++;
		}
	}
}

IntVec2 const TileHeatMap::RoundDownPos( Vec2 const& pos ) const
{
	return IntVec2( RoundDownToInt( pos.x ), RoundDownToInt( pos.y ) );
}

bool TileHeatMap::IsCoordsInBounds( IntVec2 const& coords ) const
{
	return coords.x >= 0 && coords.x < m_dimensions.x && coords.y >= 0 && coords.y < m_dimensions.y;
}
