#include "Game/GameCommon.hpp"
#include "Game/Conveyer.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Logistics.hpp"
#include "Game/PowerBuilding.hpp"
#include "Game/Resource.hpp"
#include "Game/Factory.hpp"

SFUISystem* g_uiSystem = nullptr;
SpriteSheet* g_conveyorBeltSprite;
Building* g_curChosenBuilding = nullptr;
std::vector<SFWidget*> g_refineryUIs;
std::vector<SFWidget*> g_blenderUIs;
Vec2 DirectionUnitVec[] = { Vec2( -1.f, 0.f ), Vec2( 1.f, 0.f ), Vec2( 0.f, 1.f ), Vec2( 0.f, -1.f ), Vec2( 0.f, 0.f ) };
IntVec2 DirectionUnitIntVec[] = { IntVec2( -1, 0 ), IntVec2( 1, 0 ), IntVec2( 0, 1 ), IntVec2( 0, -1 ), IntVec2( 0, 0 ) };
Vec2 OutputVec[] = { Vec2( 0.f, 0.5f ), Vec2( 1.f, 0.5f ), Vec2( 0.5f, 1.f ), Vec2( 0.5f, 0.f ), Vec2( 0.f, 0.f ) };
Vec2 InputVec[] = { Vec2( 0.99f, 0.5f ), Vec2( 0.01f, 0.5f ), Vec2( 0.5f, 0.01f ), Vec2( 0.5f, 0.99f ), Vec2( 0.f, 0.f ) };

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

void AddVertsForPowerLink( std::vector<Vertex_PCU>& verts, PowerBuilding const* powerBuilding )
{
	for (auto provider : powerBuilding->m_otherPowerBuildingInRange) {
		//if (provider->m_powerOutput) {
		AddVertsForLineSegment2D( verts, powerBuilding->GetCenterPos(), provider->GetCenterPos(), 0.08f, Rgba8( 255, 255, 255 ) );
		AddVertsForLineSegment2D( verts, powerBuilding->GetCenterPos(), provider->GetCenterPos(), 0.16f, Rgba8( 255, 255, 0, 100 ) );
		//}
	}
}

void AddVertForCursor( std::vector<Vertex_PCU>& verts, IntVec2 const& cursorCoords )
{
	AddVertsForLineSegment2D( verts, cursorCoords, Vec2( cursorCoords ) + Vec2( 0.2f, 0.f ), 0.1f, Rgba8( 255, 255, 102 ) );
	AddVertsForLineSegment2D( verts, cursorCoords, Vec2( cursorCoords ) + Vec2( 0.f, 0.2f ), 0.1f, Rgba8( 255, 255, 102 ) );

	AddVertsForLineSegment2D( verts, Vec2( cursorCoords ) + Vec2( 0.f, 1.f ), Vec2( cursorCoords ) + Vec2( 0.2f, 1.f ), 0.1f, Rgba8( 255, 255, 102 ) );
	AddVertsForLineSegment2D( verts, Vec2( cursorCoords ) + Vec2( 0.f, 1.f ), Vec2( cursorCoords ) + Vec2( 0.f, 0.8f ), 0.1f, Rgba8( 255, 255, 102 ) );

	AddVertsForLineSegment2D( verts, Vec2( cursorCoords ) + Vec2( 1.f, 1.f ), Vec2( cursorCoords ) + Vec2( 0.8f, 1.f ), 0.1f, Rgba8( 255, 255, 102 ) );
	AddVertsForLineSegment2D( verts, Vec2( cursorCoords ) + Vec2( 1.f, 1.f ), Vec2( cursorCoords ) + Vec2( 1.f, 0.8f ), 0.1f, Rgba8( 255, 255, 102 ) );

	AddVertsForLineSegment2D( verts, Vec2( cursorCoords ) + Vec2( 1.f, 0.f ), Vec2( cursorCoords ) + Vec2( 0.8f, 0.f ), 0.1f, Rgba8( 255, 255, 102 ) );
	AddVertsForLineSegment2D( verts, Vec2( cursorCoords ) + Vec2( 1.f, 0.f ), Vec2( cursorCoords ) + Vec2( 1.f, 0.2f ), 0.1f, Rgba8( 255, 255, 102 ) );
}

void DrawConveyer( Conveyer const* conveyor, IntVec2 const& LBPos, Direction dir, Rgba8 const& tint, unsigned char alpha )
{
	Rgba8 color2 = Rgba8( 255, 255, 255, alpha );
	color2 = Rgba8( color2.GetAsFloats() * tint.GetAsFloats() );
	std::vector<Vertex_PCU> verts;

	if (!conveyor) {
		int rowNum = 0;
		if (dir == Direction::Left) {
			rowNum = 1;
		}
		else if (dir == Direction::Right) {
			rowNum = 0;
		}
		else {
			rowNum = (int)dir;
		}
		int colNum = 0;
		AABB2 UV = g_conveyorBeltSprite->GetSpriteUVs( rowNum * 16 + colNum );
		UV.SetDimensions( UV.GetDimensions() * 0.5f );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ), Vec2( LBPos ) + Vec2( 1.f, 1.f ) ), color2, UV );
	}
	else {
		int rowNum = 0;
		if (conveyor->m_rear) {
			if (conveyor->m_dir == Direction::Left) {
				rowNum = 1;
			}
			else if (conveyor->m_dir == Direction::Right) {
				rowNum = 0;
			}
			else {
				rowNum = (int)conveyor->m_dir;
			}
		}
		else {
			if (conveyor->m_left) {
				if (conveyor->m_dir == Direction::Down) {
					rowNum = 9;
				}
				else if (conveyor->m_dir == Direction::Up) {
					rowNum = 6;
				}
				else if (conveyor->m_dir == Direction::Left) {
					rowNum = 10;
				}
				else if (conveyor->m_dir == Direction::Right) {
					rowNum = 5;
				}
			}
			else if (conveyor->m_right) {
				if (conveyor->m_dir == Direction::Down) {
					rowNum = 11;
				}
				else if (conveyor->m_dir == Direction::Up) {
					rowNum = 4;
				}
				else if (conveyor->m_dir == Direction::Left) {
					rowNum = 7;
				}
				else if (conveyor->m_dir == Direction::Right) {
					rowNum = 8;
				}
			}
			else {
				if (conveyor->m_dir == Direction::Left) {
					rowNum = 1;
				}
				else if (conveyor->m_dir == Direction::Right) {
					rowNum = 0;
				}
				else {
					rowNum = (int)conveyor->m_dir;
				}
			}
		}
		int colNum = RoundDownToInt( g_conveyorAnimTimer->GetElapsedFraction() * 16.f );
		if (colNum >= 16) {
			colNum = 15;
		}
		AABB2 UV = g_conveyorBeltSprite->GetSpriteUVs( rowNum * 16 + colNum );
		UV.SetDimensions( UV.GetDimensions() * 0.5f );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ), Vec2( LBPos ) + Vec2( 1.f, 1.f ) ), color2, UV );
	}

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( &g_conveyorBeltSprite->GetTexture() );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );
}

void AddVertsForDrill( std::vector<Vertex_PCU>& verts, IntVec2 const& LBPos, Direction dir, Rgba8 const& tint, unsigned char alpha /*= 255 */ )
{
	Rgba8 color1( 255, 255, 153, alpha );
	Rgba8 color2( 155, 155, 53, alpha );
	Rgba8 color3 = Rgba8( 255, 0, 0, 255 );
	color1 = Rgba8( color1.GetAsFloats() * tint.GetAsFloats() );
	color2 = Rgba8( color2.GetAsFloats() * tint.GetAsFloats() );
	constexpr float exportWidth = 0.2f;
	constexpr float exportLength = 0.75f;
	constexpr float tipSize = 0.2f;
	constexpr float arrowThickness = 0.05f;
	if (dir == Direction::Left) {
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( exportWidth, 0.f ), Vec2( LBPos ) + Vec2( 1.f, 1.f ) ), color1 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, 1.f - exportLength ), Vec2( LBPos ) + Vec2( exportWidth, exportLength ) ), color2 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 1.f, 0.5f ), Vec2( LBPos ) + Vec2( 0.f, 0.5f ), tipSize, arrowThickness, color3 );
	}
	else if (dir == Direction::Down) {
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportLength, 0.f ), Vec2( LBPos ) + Vec2( exportLength, exportWidth ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, exportWidth ), Vec2( LBPos ) + Vec2( 1.f, 1.f ) ), color1 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 1.f ), Vec2( LBPos ) + Vec2( 0.5f, 0.f ), tipSize, arrowThickness, color3 );
	}
	else if (dir == Direction::Right) {
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportWidth, 1.f - exportLength ), Vec2( LBPos ) + Vec2( 1.f, exportLength ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, 0.f ), Vec2( LBPos ) + Vec2( 1.f - exportWidth, 1.f ) ), color1 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.f, 0.5f ), Vec2( LBPos ) + Vec2( 1.f, 0.5f ), tipSize, arrowThickness, color3 );
	}
	else if (dir == Direction::Up) {
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportLength, 1.f - exportWidth ), Vec2( LBPos ) + Vec2( exportLength, 1.f ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, 0.f ), Vec2( LBPos ) + Vec2( 1.f, 1.f - exportWidth ) ), color1 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.f ), Vec2( LBPos ) + Vec2( 0.5f, 1.f ), tipSize, arrowThickness, color3 );
	}
}

void AddVertsForSelector( std::vector<Vertex_PCU>& verts, Selector const* selector, Rgba8 const& tint /*= Rgba8::WHITE*/, unsigned char alpha /*= 255 */ )
{
	Direction dir = selector->m_dir;
	IntVec2 LBPos = selector->m_leftBottomCoords;
	AddVertsForSelector( verts, LBPos, dir, tint, alpha );
}

void AddVertsForSelector( std::vector<Vertex_PCU>& verts, IntVec2 const& LBPos, Direction dir, Rgba8 const& tint /*= Rgba8::WHITE*/, unsigned char alpha /*= 255 */ )
{
	Rgba8 color1( 255, 128, 0, alpha );
	Rgba8 color2( 155, 204, 153, alpha );
	Rgba8 color3 = Rgba8( 255, 0, 0, 255 );
	color1 = Rgba8( color1.GetAsFloats() * tint.GetAsFloats() );
	color2 = Rgba8( color2.GetAsFloats() * tint.GetAsFloats() );
	constexpr float exportWidth = 0.2f;
	constexpr float exportLength = 0.75f;
	constexpr float tipSize = 0.2f;
	constexpr float arrowThickness = 0.05f;
	if (dir == Direction::Left) {
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( exportWidth, exportWidth ), Vec2( LBPos ) + Vec2( 1.f, 1.f - exportWidth ) ), color1 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, 1.f - exportLength ), Vec2( LBPos ) + Vec2( exportWidth, exportLength ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportLength, 0.f ), Vec2( LBPos ) + Vec2( exportLength, exportWidth ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportLength, 1.f - exportWidth ), Vec2( LBPos ) + Vec2( exportLength, 1.f ) ), color2 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 1.f, 0.5f ), Vec2( LBPos ) + Vec2( 0.f, 0.5f ), tipSize, arrowThickness, color3 );
	}
	else if (dir == Direction::Down) {
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportLength, 0.f ), Vec2( LBPos ) + Vec2( exportLength, exportWidth ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, 1.f - exportLength ), Vec2( LBPos ) + Vec2( exportWidth, exportLength ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportWidth, 1.f - exportLength ), Vec2( LBPos ) + Vec2( 1.f, exportLength ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( exportWidth, exportWidth ), Vec2( LBPos ) + Vec2( 1.f - exportWidth, 1.f ) ), color1 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 1.f ), Vec2( LBPos ) + Vec2( 0.5f, 0.f ), tipSize, arrowThickness, color3 );
	}
	else if (dir == Direction::Right) {
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportWidth, 1.f - exportLength ), Vec2( LBPos ) + Vec2( 1.f, exportLength ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportLength, 1.f - exportWidth ), Vec2( LBPos ) + Vec2( exportLength, 1.f ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportLength, 0.f ), Vec2( LBPos ) + Vec2( exportLength, exportWidth ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, exportWidth ), Vec2( LBPos ) + Vec2( 1.f - exportWidth, 1.f - exportWidth ) ), color1 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.f, 0.5f ), Vec2( LBPos ) + Vec2( 1.f, 0.5f ), tipSize, arrowThickness, color3 );
	}
	else if (dir == Direction::Up) {
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportLength, 1.f - exportWidth ), Vec2( LBPos ) + Vec2( exportLength, 1.f ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportWidth, 1.f - exportLength ), Vec2( LBPos ) + Vec2( 1.f, exportLength ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, 1.f - exportLength ), Vec2( LBPos ) + Vec2( exportWidth, exportLength ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( exportWidth, 0.f ), Vec2( LBPos ) + Vec2( 1.f - exportWidth, 1.f - exportWidth ) ), color1 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.f ), Vec2( LBPos ) + Vec2( 0.5f, 1.f ), tipSize, arrowThickness, color3 );
	}
}

void AddVertsForResource( std::vector<Vertex_PCU>& verts, Vec2 const& centerPos, ProductDefinition const& def, float resourceRadius )
{
	UNUSED( def );
	AddVertsForAABB2D( verts, AABB2( centerPos - Vec2( resourceRadius, resourceRadius ), centerPos + Vec2( resourceRadius, resourceRadius ) ), Rgba8::WHITE, AABB2( Vec2( 0.f, 0.f ), Vec2( 0.5f, 1.f ) ) );
// 	if (def.m_id == 0) {
// 		Rgba8 color = Rgba8( 128, 128, 128 );
// 		AddVertsForDisc2D( verts, centerPos, resourceRadius, color );
// 	}
// 	else if (def.m_id == 2) {
// 		Rgba8 color = Rgba8( 153, 153, 255 );
// 		AddVertsForAABB2D( verts, AABB2( centerPos - Vec2( resourceRadius, resourceRadius ), centerPos + Vec2( resourceRadius, resourceRadius ) ), color );
// 	}
// 	else if (def.m_id == 1) {
// 		Rgba8 color = Rgba8( 255, 204, 153 );
// 		float radius = resourceRadius * 1.3f;
// 		verts.emplace_back( centerPos + Vec2( -radius * 0.866f, -radius * 0.75f ), color );
// 		verts.emplace_back( centerPos + Vec2( radius * 0.866f, -radius * 0.75f ) , color );
// 		verts.emplace_back( centerPos + Vec2( 0.f, radius * 0.75f )              , color );
// 	}
// 	else if (def.m_id == 4) {
// 		float radius = resourceRadius * 1.2f;
// 		Vec2 p1 = centerPos + Vec2( -0.587785f * radius, -0.809017f * radius ) - Vec2( 0.f, radius * 0.1f );
// 		Vec2 p2 = centerPos + Vec2( 0.587785f * radius, -0.809017f * radius ) - Vec2( 0.f, radius * 0.1f );
// 		Vec2 p3 = centerPos + Vec2( 0.951057f * radius, 0.309017f * radius ) - Vec2( 0.f, radius * 0.1f );
// 		Vec2 p4 = centerPos + Vec2( 0.f, radius ) - Vec2( 0.f, radius * 0.1f );
// 		Vec2 p5 = centerPos + Vec2( -0.951057f * radius, 0.309017f * radius ) - Vec2( 0.f, radius * 0.1f );
// 		Rgba8 color = Rgba8( 155, 255, 153 );
// 		verts.emplace_back( p1, color );
// 		verts.emplace_back( p2, color );
// 		verts.emplace_back( p3, color );
// 
// 		verts.emplace_back( p1, color );
// 		verts.emplace_back( p3, color );
// 		verts.emplace_back( p4, color );
// 
// 		verts.emplace_back( p1, color );
// 		verts.emplace_back( p4, color );
// 		verts.emplace_back( p5, color );
// 	}
// 	else if (def.m_id == 3) {
// 		float radius = resourceRadius * 1.2f;
// 		Vec2 p1 = centerPos + Vec2( -0.5f * radius, -0.866f * radius );
// 		Vec2 p2 = centerPos + Vec2( 0.5f * radius, -0.866f * radius );
// 		Vec2 p3 = centerPos + Vec2( radius, 0.f );
// 		Vec2 p4 = centerPos + Vec2( 0.5f * radius, 0.866f * radius );
// 		Vec2 p5 = centerPos + Vec2( -0.5f * radius, 0.866f * radius );
// 		Vec2 p6 = centerPos + Vec2( -radius, 0.f );
// 		Rgba8 color = Rgba8( 0, 204, 102 );
// 
// 		verts.emplace_back( p1, color );
// 		verts.emplace_back( p2, color );
// 		verts.emplace_back( p3, color );
// 
// 		verts.emplace_back( p1, color );
// 		verts.emplace_back( p3, color );
// 		verts.emplace_back( p4, color );
// 
// 		verts.emplace_back( p1, color );
// 		verts.emplace_back( p4, color );
// 		verts.emplace_back( p5, color );
// 
// 		verts.emplace_back( p1, color );
// 		verts.emplace_back( p5, color );
// 		verts.emplace_back( p6, color );
// 	}
// 	else if (def.m_id == 5) {
// 		float triangleRadius = resourceRadius * 0.5f;
// 		float radius = 0.6f * resourceRadius;
// 		AddVertsForResource( verts, centerPos + Vec2( -triangleRadius * 0.866f, -triangleRadius * 0.75f ), ProductDefinition::GetDefinition(0), radius );
// 		AddVertsForResource( verts, centerPos + Vec2( triangleRadius * 0.866f, -triangleRadius * 0.75f ),  ProductDefinition::GetDefinition(0), radius );
// 		AddVertsForResource( verts, centerPos + Vec2( 0.f, triangleRadius * 0.75f ),  ProductDefinition::GetDefinition(0), radius );
// 	}
// // 	else if (type == ResourceType::TriSquare) {
// // 		float triangleRadius = resourceRadius * 0.5f;
// // 		float radius = 0.6f * resourceRadius;
// // 		AddVertsForResource( verts, centerPos + Vec2( -triangleRadius * 0.866f, -triangleRadius * 0.75f ), ResourceType::Square, radius );
// // 		AddVertsForResource( verts, centerPos + Vec2( triangleRadius * 0.866f, -triangleRadius * 0.75f ), ResourceType::Square, radius );
// // 		AddVertsForResource( verts, centerPos + Vec2( 0.f, triangleRadius * 0.75f ), ResourceType::Square, radius );
// // 	}
// 	else if (def.m_id == 6) {
// 		float triangleRadius = resourceRadius * 0.5f;
// 		float radius = 0.6f * resourceRadius;
// 		AddVertsForResource( verts, centerPos + Vec2( -triangleRadius * 0.866f, -triangleRadius * 0.75f ),  ProductDefinition::GetDefinition(1), radius );
// 		AddVertsForResource( verts, centerPos + Vec2( triangleRadius * 0.866f, -triangleRadius * 0.75f ),  ProductDefinition::GetDefinition(1), radius );
// 		AddVertsForResource( verts, centerPos + Vec2( 0.f, triangleRadius * 0.75f ),  ProductDefinition::GetDefinition(1), radius );
// 	}
// 	else if (def.m_id == 7) {
// 		float triangleRadius = resourceRadius * 0.5f;
// 		float radius = 0.6f * resourceRadius;
// 		AddVertsForResource( verts, centerPos + Vec2( -triangleRadius * 0.866f, -triangleRadius * 0.75f ),  ProductDefinition::GetDefinition(2), radius );
// 		AddVertsForResource( verts, centerPos + Vec2( triangleRadius * 0.866f, -triangleRadius * 0.75f ),  ProductDefinition::GetDefinition(2), radius );
// 		AddVertsForResource( verts, centerPos + Vec2( 0.f, triangleRadius * 0.75f ),  ProductDefinition::GetDefinition(2), radius );
// 	}
// 	else if (def.m_id == 8) {
// 		float triangleRadius = resourceRadius * 0.5f;
// 		float radius = 0.6f * resourceRadius;
// 		AddVertsForResource( verts, centerPos + Vec2( -triangleRadius * 0.866f, -triangleRadius * 0.75f ),  ProductDefinition::GetDefinition(3), radius );
// 		AddVertsForResource( verts, centerPos + Vec2( triangleRadius * 0.866f, -triangleRadius * 0.75f ),  ProductDefinition::GetDefinition(3), radius );
// 		AddVertsForResource( verts, centerPos + Vec2( 0.f, triangleRadius * 0.75f ), ProductDefinition::GetDefinition(3), radius );
// 	}
}

void DrawResource( Vec2 const& centerPos, ProductDefinition const& type, float radius /*= RESOURCE_RADIUS */ )
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 6 );
	AddVertsForResource( verts, centerPos, type, radius );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( type.m_texture );
	//g_theRenderer->SetModelConstants( Mat44::CreateTranslation3D( Vec3( 0.f, 0.f, 0.1f ) ) );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );
}

void AddVertsForRouter( std::vector<Vertex_PCU>& verts, IntVec2 const& LBPos, Rgba8 const& tint /*= Rgba8::WHITE*/, unsigned char alpha /*= 255 */ )
{
	Rgba8 color1( 255, 128, 0, alpha );
	Rgba8 color2( 155, 204, 153, alpha );
	color1 = Rgba8( color1.GetAsFloats() * tint.GetAsFloats() );
	color2 = Rgba8( color2.GetAsFloats() * tint.GetAsFloats() );
	AddVertsForAABB2D( verts, AABB2( LBPos, Vec2( LBPos ) + Vec2( 1.f, 1.f ) ), color1 );
	AddVertsForDisc2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.5f ), 0.5f, color2 );
}

void AddVertsForOverflowGate( std::vector<Vertex_PCU>& verts, IntVec2 const& LBPos, Direction dir, Rgba8 const& tint /*= Rgba8::WHITE*/, unsigned char alpha /*= 255 */ )
{
	Rgba8 color1( 255, 128, 0, alpha );
	Rgba8 color2( 50, 50, 50, alpha );
	Rgba8 color3 = Rgba8( 255, 0, 0, 255 );
	color1 = Rgba8( color1.GetAsFloats() * tint.GetAsFloats() );
	color2 = Rgba8( color2.GetAsFloats() * tint.GetAsFloats() );
	constexpr float exportWidth = 0.2f;
	constexpr float exportLength = 0.75f;
	constexpr float tipSize = 0.2f;
	constexpr float arrowThickness = 0.05f;
	if (dir == Direction::Left) {
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, exportWidth ), Vec2( LBPos ) + Vec2( 1.f, 1.f - exportWidth ) ), color1 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportLength, 0.f ), Vec2( LBPos ) + Vec2( exportLength, exportWidth ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportLength, 1.f - exportWidth ), Vec2( LBPos ) + Vec2( exportLength, 1.f ) ), color2 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 1.f, 0.5f ), Vec2( LBPos ) + Vec2( 0.f, 0.5f ), tipSize, arrowThickness, color3 );
	}
	else if (dir == Direction::Down) {
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, 1.f - exportLength ), Vec2( LBPos ) + Vec2( exportWidth, exportLength ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportWidth, 1.f - exportLength ), Vec2( LBPos ) + Vec2( 1.f, exportLength ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( exportWidth, 0.f ), Vec2( LBPos ) + Vec2( 1.f - exportWidth, 1.f ) ), color1 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 1.f ), Vec2( LBPos ) + Vec2( 0.5f, 0.f ), tipSize, arrowThickness, color3 );
	}
	else if (dir == Direction::Right) {
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportLength, 1.f - exportWidth ), Vec2( LBPos ) + Vec2( exportLength, 1.f ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportLength, 0.f ), Vec2( LBPos ) + Vec2( exportLength, exportWidth ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, exportWidth ), Vec2( LBPos ) + Vec2( 1.f, 1.f - exportWidth ) ), color1 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.f, 0.5f ), Vec2( LBPos ) + Vec2( 1.f, 0.5f ), tipSize, arrowThickness, color3 );
	}
	else if (dir == Direction::Up) {
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 1.f - exportWidth, 1.f - exportLength ), Vec2( LBPos ) + Vec2( 1.f, exportLength ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, 1.f - exportLength ), Vec2( LBPos ) + Vec2( exportWidth, exportLength ) ), color2 );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( exportWidth, 0.f ), Vec2( LBPos ) + Vec2( 1.f - exportWidth, 1.f ) ), color1 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.f ), Vec2( LBPos ) + Vec2( 0.5f, 1.f ), tipSize, arrowThickness, color3 );
	}
}

void DrawWareHouse( IntVec2 const& LBPos, Rgba8 const& tint /*= Rgba8::WHITE*/, unsigned char alpha /*= 255 */ )
{
	Rgba8 color1( 255, 255, 255, alpha );
	color1 = Rgba8( color1.GetAsFloats() * tint.GetAsFloats() );
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, AABB2( LBPos, Vec2( LBPos ) + Vec2( 1.f, 1.f ) ), color1, AABB2( Vec2( 0.f, 0.1f ), Vec2( 1.f, 1.f ) ) );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Buildings/warehouse.png"));
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );
}

void AddVertsForExporter( std::vector<Vertex_PCU>& verts, IntVec2 const& LBPos, Direction dir, Rgba8 const& tint /*= Rgba8::WHITE*/, unsigned char alpha /*= 255 */ )
{
	Rgba8 color2( 155, 204, 153, alpha );
	Rgba8 color3( 255, 0, 0, 255 );
	color2 = Rgba8( color2.GetAsFloats() * tint.GetAsFloats() );

	constexpr float tipSize = 0.2f;
	constexpr float arrowThickness = 0.05f;
	AddVertsForAABB2D( verts, AABB2( LBPos, Vec2( LBPos ) + Vec2( 1.f, 1.f ) ), color2 );
	if (dir == Direction::Left) {
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 1.f, 0.5f ), Vec2( LBPos ) + Vec2( 0.f, 0.5f ), tipSize, arrowThickness, color3 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.5f ), Vec2( LBPos ) + Vec2( 0.5f, 1.f ), tipSize, arrowThickness, color3 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.5f ), Vec2( LBPos ) + Vec2( 0.5f, 0.f ), tipSize, arrowThickness, color3 );
	}
	else if (dir == Direction::Down) {
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 1.f ), Vec2( LBPos ) + Vec2( 0.5f, 0.f ), tipSize, arrowThickness, color3 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.5f ), Vec2( LBPos ) + Vec2( 0.f, 0.5f ), tipSize, arrowThickness, color3 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.5f ), Vec2( LBPos ) + Vec2( 1.f, 0.5f ), tipSize, arrowThickness, color3 );
	}
	else if (dir == Direction::Right) {
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.f, 0.5f ), Vec2( LBPos ) + Vec2( 1.f, 0.5f ), tipSize, arrowThickness, color3 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.5f ), Vec2( LBPos ) + Vec2( 0.5f, 0.f ), tipSize, arrowThickness, color3 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.5f ), Vec2( LBPos ) + Vec2( 0.5f, 1.f ), tipSize, arrowThickness, color3 );
	}
	else if (dir == Direction::Up) {
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.f ), Vec2( LBPos ) + Vec2( 0.5f, 1.f ), tipSize, arrowThickness, color3 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.5f ), Vec2( LBPos ) + Vec2( 0.f, 0.5f ), tipSize, arrowThickness, color3 );
		AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.5f ), Vec2( LBPos ) + Vec2( 1.f, 0.5f ), tipSize, arrowThickness, color3 );
	}
}

void AddVertsForJunction( std::vector<Vertex_PCU>& verts, IntVec2 const& LBPos, Rgba8 const& tint /*= Rgba8::WHITE*/, unsigned char alpha /*= 255 */ )
{
	Rgba8 color1( 255, 128, 0, alpha );
	Rgba8 color2( 155, 204, 153, alpha );
	color1 = Rgba8( color1.GetAsFloats() * tint.GetAsFloats() );
	color2 = Rgba8( color2.GetAsFloats() * tint.GetAsFloats() );

	AddVertsForAABB2D( verts, AABB2( LBPos, Vec2( LBPos ) + Vec2( 1.f, 1.f ) ), color2 );
	AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, 0.2f ), Vec2( LBPos ) + Vec2( 1.f, 0.8f ) ), color1 );
	AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.2f, 0.f ), Vec2( LBPos ) + Vec2( 0.8f, 1.f ) ), color1 );
}

void DrawBridge( IntVec2 const& LBPos, Direction dir, bool isInput, Rgba8 const& tint /*= Rgba8::WHITE*/, unsigned char alpha /*= 255 */ )
{
// 	Rgba8 color1( 255, 128, 0, alpha );
// 	color1 = Rgba8( color1.GetAsFloats() * tint.GetAsFloats() );
// 	Rgba8 color3 = Rgba8( 255, 0, 0, 255 );
// 
// 	constexpr float tipSize = 0.2f;
// 	constexpr float arrowThickness = 0.05f;
// 	if (dir == Direction::Down) {
// 		Vec2 p1 = Vec2( LBPos ) + Vec2( 0.f, 1.f );
// 		Vec2 p2 = Vec2( LBPos ) + Vec2( 0.3f, 0.f );
// 		Vec2 p3 = Vec2( LBPos ) + Vec2( 0.7f, 0.f );
// 		Vec2 p4 = Vec2( LBPos ) + Vec2( 1.f, 1.f );
// 		verts.emplace_back( p1, color1 );
// 		verts.emplace_back( p2, color1 );
// 		verts.emplace_back( p3, color1 );
// 		verts.emplace_back( p1, color1 );
// 		verts.emplace_back( p3, color1 );
// 		verts.emplace_back( p4, color1 );
// 		if (isInput) {
// 			AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 1.f ), Vec2( LBPos ) + Vec2( 0.5f, 0.f ), tipSize, arrowThickness, color3 );
// 		}
// 	}
// 	else if (dir == Direction::Right) {
// 		Vec2 p1 = Vec2( LBPos ) + Vec2( 0.f, 1.f );
// 		Vec2 p2 = Vec2( LBPos );
// 		Vec2 p3 = Vec2( LBPos ) + Vec2( 1.f, 0.3f );
// 		Vec2 p4 = Vec2( LBPos ) + Vec2( 1.f, 0.7f );
// 		verts.emplace_back( p1, color1 );
// 		verts.emplace_back( p2, color1 );
// 		verts.emplace_back( p3, color1 );
// 		verts.emplace_back( p1, color1 );
// 		verts.emplace_back( p3, color1 );
// 		verts.emplace_back( p4, color1 );
// 		if (isInput) {
// 			AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.f, 0.5f ), Vec2( LBPos ) + Vec2( 1.f, 0.5f ), tipSize, arrowThickness, color3 );
// 		}
// 	}
// 	else if (dir == Direction::Up) {
// 		Vec2 p1 = Vec2( LBPos ) + Vec2( 0.3f, 1.f );
// 		Vec2 p2 = Vec2( LBPos );
// 		Vec2 p3 = Vec2( LBPos ) + Vec2( 1.f, 0.f );
// 		Vec2 p4 = Vec2( LBPos ) + Vec2( 0.7f, 1.f );
// 		verts.emplace_back( p1, color1 );
// 		verts.emplace_back( p2, color1 );
// 		verts.emplace_back( p3, color1 );
// 		verts.emplace_back( p1, color1 );
// 		verts.emplace_back( p3, color1 );
// 		verts.emplace_back( p4, color1 );
// 		if (isInput) {
// 			AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.f ), Vec2( LBPos ) + Vec2( 0.5f, 1.f ), tipSize, arrowThickness, color3 );
// 		}
// 	}
// 	else if (dir == Direction::Left) {
// 		Vec2 p1 = Vec2( LBPos ) + Vec2( 0.f, 0.7f );
// 		Vec2 p2 = Vec2( LBPos ) + Vec2( 0.f, 0.3f );
// 		Vec2 p3 = Vec2( LBPos ) + Vec2( 1.f, 0.f );
// 		Vec2 p4 = Vec2( LBPos ) + Vec2( 1.f, 1.f );
// 		verts.emplace_back( p1, color1 );
// 		verts.emplace_back( p2, color1 );
// 		verts.emplace_back( p3, color1 );
// 		verts.emplace_back( p1, color1 );
// 		verts.emplace_back( p3, color1 );
// 		verts.emplace_back( p4, color1 );
// 		if (isInput) {
// 			AddVertsForArrow2D( verts, Vec2( LBPos ) + Vec2( 1.f, 0.5f ), Vec2( LBPos ) + Vec2( 0.f, 0.5f ), tipSize, arrowThickness, color3 );
// 		}
// 	}
	Rgba8 color1( 255, 255, 255, alpha );
	color1 = Rgba8( color1.GetAsFloats() * tint.GetAsFloats() );
	std::vector<Vertex_PCU> verts;
	AABB2 uvBounds;
	if (isInput) {
		if (dir == Direction::Down) {
			uvBounds = AABB2( Vec2( 0.5f, 0.5f ), Vec2( 0.75f, 0.75f ) );
			uvBounds.SetDimensions( uvBounds.GetDimensions() * 0.4f );
			AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, 0.3f ), Vec2( LBPos ) + Vec2( 1.f, 1.3f ) ), color1, uvBounds );
		}
		else if (dir == Direction::Up) {
			uvBounds = AABB2( Vec2( 0.f, 0.5f ), Vec2( 0.25f, 0.75f ) );
			uvBounds.SetDimensions( uvBounds.GetDimensions() * 0.4f );
			AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, -0.3f ), Vec2( LBPos ) + Vec2( 1.f, 0.7f ) ), color1, uvBounds );
		}
		else if (dir == Direction::Left) {
			uvBounds = AABB2( Vec2( 0.75f, 0.5f ), Vec2( 1.f, 0.75f ) );
			uvBounds.SetDimensions( uvBounds.GetDimensions() * 0.4f );
			AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.3f, 0.f ), Vec2( LBPos ) + Vec2( 1.3f, 1.f ) ), color1, uvBounds );
		}
		else if (dir == Direction::Right) {
			uvBounds = AABB2( Vec2( 0.25f, 0.5f ), Vec2( 0.5f, 0.75f ) );
			uvBounds.SetDimensions( uvBounds.GetDimensions() * 0.4f );
			AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( -0.3f, 0.f ), Vec2( LBPos ) + Vec2( 0.7f, 1.f ) ), color1, uvBounds );
		}
	}
	else {
		if (dir == Direction::Up) {
			uvBounds = AABB2( Vec2( 0.f, 0.75f ), Vec2( 0.25f, 1.f ) );
			uvBounds.SetDimensions( uvBounds.GetDimensions() * 0.4f );
			AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, -0.3f ), Vec2( LBPos ) + Vec2( 1.f, 0.7f ) ), color1, uvBounds );
		}
		else if (dir == Direction::Down) {
			uvBounds = AABB2( Vec2( 0.5f, 0.75f ), Vec2( 0.75f, 1.f ) );
			uvBounds.SetDimensions( uvBounds.GetDimensions() * 0.4f );
			AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, 0.3f ), Vec2( LBPos ) + Vec2( 1.f, 1.3f ) ), color1, uvBounds );
		}
		else if (dir == Direction::Right) {
			uvBounds = AABB2( Vec2( 0.25f, 0.75f ), Vec2( 0.5f, 1.f ) );
			uvBounds.SetDimensions( uvBounds.GetDimensions() * 0.4f );
			AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( -0.3f, 0.f ), Vec2( LBPos ) + Vec2( 0.7f, 1.f ) ), color1, uvBounds );
		}
		else if (dir == Direction::Left) {
			uvBounds = AABB2( Vec2( 0.75f, 0.75f ), Vec2( 1.f, 1.f ) );
			uvBounds.SetDimensions( uvBounds.GetDimensions() * 0.4f );
			AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.3f, 0.f ), Vec2( LBPos ) + Vec2( 1.3f, 1.f ) ), color1, uvBounds );
		}
	}

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Buildings/tunnel.png" ) );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );
}

void AddVertsForPowerPlant( std::vector<Vertex_PCU>& verts, IntVec2 const& LBPos, Rgba8 const& tint /*= Rgba8::WHITE*/, unsigned char alpha /*= 255 */ )
{
	Rgba8 color1( 255, 128, 0, alpha );
	Rgba8 color2( 50, 50, 50, alpha );
	color1 = Rgba8( color1.GetAsFloats() * tint.GetAsFloats() );
	color2 = Rgba8( color2.GetAsFloats() * tint.GetAsFloats() );

	constexpr float radius = 0.2f;

	AddVertsForAABB2D( verts, AABB2( LBPos, Vec2( LBPos ) + Vec2( 1.f, 1.f ) ), color1 );
	AddVertsForDisc2D( verts, Vec2( LBPos ) + Vec2( 0.25f, 0.25f ), radius, color2 );
	AddVertsForDisc2D( verts, Vec2( LBPos ) + Vec2( 0.75f, 0.25f ), radius, color2 );
	AddVertsForDisc2D( verts, Vec2( LBPos ) + Vec2( 0.25f, 0.75f ), radius, color2 );
	AddVertsForDisc2D( verts, Vec2( LBPos ) + Vec2( 0.75f, 0.75f ), radius, color2 );

}

void DrawPowerNode( IntVec2 const& LBPos, Rgba8 const& tint /*= Rgba8::WHITE*/, unsigned char alpha /*= 255 */ )
{
// 	Rgba8 color1( 255, 128, 0, alpha );
// 	Rgba8 color2( 200, 100, 0, alpha );
// 	Rgba8 color3( 160, 80, 0, alpha );
// 	color1 = Rgba8( color1.GetAsFloats() * tint.GetAsFloats() );
// 	color2 = Rgba8( color2.GetAsFloats() * tint.GetAsFloats() );
// 	color3 = Rgba8( color3.GetAsFloats() * tint.GetAsFloats() );
// 
// 	constexpr float radius = 0.5f;
// 
// 	AddVertsForDisc2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.5f ), radius, color1 );
// 	AddVertsForDisc2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.5f ), radius * 0.75f, color2 );
// 	AddVertsForDisc2D( verts, Vec2( LBPos ) + Vec2( 0.5f, 0.5f ), radius * 0.5f, color3 );
	Rgba8 color1( 255, 255, 255, alpha );
	color1 = Rgba8( color1.GetAsFloats() * tint.GetAsFloats() );
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, AABB2( LBPos, Vec2( LBPos ) + Vec2( 1.f, 2.f ) ), color1, AABB2( Vec2( 0.f, 0.f ), Vec2( 0.25f, 1.f ) ) );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Buildings/power-node.png" ) );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );
}

void DrawRefinery( IntVec2 const& LBPos, Rgba8 const& tint /*= Rgba8::WHITE*/, unsigned char alpha /*= 255 */ )
{
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, AABB2( LBPos, Vec2( LBPos ) + Vec2( 2.f, 2.f ) ), Rgba8( tint.r, tint.g, tint.b, unsigned char( (float)tint.a * (float)alpha / 255.f ) ), AABB2( Vec2( 0.1f, 0.1f ), Vec2( 0.9f, 1.f ) ) );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Buildings/refinery.png" ) );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );
}

void DrawBlender( Blender const* blender,  IntVec2 const& LBPos, Rgba8 const& tint /*= Rgba8::WHITE*/, unsigned char alpha /*= 255 */ )
{
	std::vector<Vertex_PCU> verts;
	Rgba8 color1( 255, 255, 255, alpha );
	color1 = Rgba8( color1.GetAsFloats() * tint.GetAsFloats() );
	constexpr float multiplier = 0.15f;
	if (blender) {
		AABB2 uv = blender->m_animationSprite.GetSpriteDefAtTime( blender->m_animationTime ).GetUVs();
		//uv.SetDimensions( uv.GetDimensions() * (1.f - multiplier * 2.f) );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) - Vec2( 2.f, 2.f ) * multiplier, Vec2( LBPos ) + Vec2( 2.f, 2.f ) * (1.f + multiplier) ), color1, uv );
	}
	else {
		AABB2 uv = AABB2( Vec2( 0.f, 0.75f ), Vec2( 0.125f, 1.f ) );
		//uv.SetDimensions( uv.GetDimensions() * (1.f - multiplier * 2.f) );
		AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) - Vec2( 2.f, 2.f ) * multiplier, Vec2( LBPos ) + Vec2( 2.f, 2.f ) * (1.f + multiplier) ), color1, uv );
	}

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	if (blender) {
		g_theRenderer->BindTexture( blender->m_animationSprite.GetTexture() );
	}
	else {
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Buildings/blender.png" ) );
	}
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );
}

void DrawStraightArcher( IntVec2 const& LBPos, Direction dir, Rgba8 const& tint /*= Rgba8::WHITE*/, unsigned char alpha /*= 255 */ )
{
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, AABB2( LBPos, Vec2( LBPos ) + Vec2( 1.f, 1.f ) ), Rgba8( tint.r, tint.g, tint.b, unsigned char( (float)tint.a * (float)alpha / 255.f ) ) );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Buildings/GunTower/gun-turret-base.png" ) );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );
	
	verts.clear();
	AABB2 uv;
	if (dir == Direction::Up) {
		uv = AABB2( Vec2( 0.0f, 0.75f ), Vec2( 0.2f, 1.f ) );
	}
	else if (dir == Direction::Down) {
		uv = AABB2( Vec2( 0.0f, 0.25f ), Vec2( 0.2f, 0.5f ) );
	}
	else if (dir == Direction::Left) {
		uv = AABB2( Vec2( 0.0f, 0.f ), Vec2( 0.2f, 0.25f ) );
	}
	else if (dir == Direction::Right) {
		uv = AABB2( Vec2( 0.0f, 0.5f ), Vec2( 0.2f, 0.75f ) );
	}
	AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, 0.25f ), Vec2( LBPos ) + Vec2( 1.f, 1.25f ) ), Rgba8( tint.r, tint.g, tint.b, unsigned char( (float)tint.a * (float)alpha / 255.f ) ), uv );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Buildings/GunTower/gun-turret-raising.png" ) );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );
}

void DrawThreeDirectionPike( IntVec2 const& LBPos, Direction dir, Rgba8 const& tint /*= Rgba8::WHITE*/, unsigned char alpha /*= 255 */ )
{
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, AABB2( LBPos, Vec2( LBPos ) + Vec2( 1.f, 1.f ) ), Rgba8( tint.r, tint.g, tint.b, unsigned char( (float)tint.a * (float)alpha / 255.f ) ) );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Buildings/GunTower/gun-turret-base.png" ) );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );

	verts.clear();
	AABB2 uv;
	constexpr float dirDiff = 15.f;
	if (dir == Direction::Up) {
		uv = AABB2( Vec2( 0.0f, 0.75f ), Vec2( 0.2f, 1.f ) );
	}
	else if (dir == Direction::Down) {
		uv = AABB2( Vec2( 0.0f, 0.25f ), Vec2( 0.2f, 0.5f ) );
	}
	else if (dir == Direction::Left) {
		uv = AABB2( Vec2( 0.0f, 0.f ), Vec2( 0.2f, 0.25f ) );
	}
	else if (dir == Direction::Right) {
		uv = AABB2( Vec2( 0.0f, 0.5f ), Vec2( 0.2f, 0.75f ) );
	}
	AddVertsForOBB2D( verts, OBB2( Vec2( LBPos ) + Vec2( 0.5f, 0.75f ), Vec2::MakeFromPolarDegrees( dirDiff ), Vec2( 0.5f, 0.5f ) ), Rgba8( tint.r, tint.g, tint.b, unsigned char( (float)tint.a * (float)alpha / 255.f ) ), uv );
	AddVertsForOBB2D( verts, OBB2( Vec2( LBPos ) + Vec2( 0.5f, 0.75f ), Vec2::MakeFromPolarDegrees( -dirDiff ), Vec2( 0.5f, 0.5f ) ), Rgba8( tint.r, tint.g, tint.b, unsigned char( (float)tint.a * (float)alpha / 255.f ) ), uv );
	AddVertsForAABB2D( verts, AABB2( Vec2( LBPos ) + Vec2( 0.f, 0.25f ), Vec2( LBPos ) + Vec2( 1.f, 1.25f ) ), Rgba8( tint.r, tint.g, tint.b, unsigned char( (float)tint.a * (float)alpha / 255.f ) ), uv );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Buildings/GunTower/gun-turret-raising.png" ) );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );
}

void AddVertsForHealthBar( std::vector<Vertex_PCU>& verts, Building const* building )
{
	constexpr float healthBarHeight = 0.1f;
	constexpr float healthBarHalfHeight = healthBarHeight * 0.5f;
	constexpr float healthBarAboveHeight = 0.2f;
	constexpr float healthBarRatio = 0.8f;
	float width = (building->m_physicsBounds.m_maxs.x - building->m_physicsBounds.m_mins.x);
	float healthBarBaseY = building->GetWorldPhysicsBounds().m_maxs.y + healthBarAboveHeight;
	float healthBarLeftX = building->GetWorldPhysicsBounds().m_mins.x + (1.f - healthBarRatio) * 0.5f * width;
	width *= healthBarRatio;
	float curHealthRatio = building->m_health / building->m_maxHealth;

	AddVertsForAABB2D( verts, AABB2( Vec2( healthBarLeftX, healthBarBaseY - healthBarHalfHeight ), Vec2( healthBarLeftX + width, healthBarBaseY + healthBarHalfHeight ) ), Rgba8::RED );
	AddVertsForAABB2D( verts, AABB2( Vec2( healthBarLeftX + width * (1.f - curHealthRatio), healthBarBaseY - healthBarHalfHeight ), Vec2( healthBarLeftX + width, healthBarBaseY + healthBarHalfHeight ) ), Rgba8( 0, 255, 0 ) );
}

void AddVertsForResourcePanel( std::vector<Vertex_PCU>& verts, std::vector<Vertex_PCU>& textVerts, Vec2 const& middleLeftPos, int* resourcesAmount, Rgba8 const& backgroundColor /*= Rgba8::WHITE*/, Rgba8 const& textColor /*= Rgba8::BLACK*/ )
{
	UNUSED( verts );
	constexpr float textHeight = 20.f;
	int numOfResourcesTypeHave = 0;
	for (int i = 0; i < NumOfProductTypes; ++i) {
		if (resourcesAmount[i] > 0) {
			++numOfResourcesTypeHave;
		}
	}
	std::vector<Vertex_PCU> localVerts;
	AddVertsForAABB2D( localVerts, AABB2( middleLeftPos - Vec2( textHeight * 0.6f + 5.f, textHeight * std::max( 3.f, (float)(numOfResourcesTypeHave - 3) ) ), middleLeftPos + Vec2( 120.f, textHeight * 3.f + 5.f ) ), backgroundColor );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
	g_theRenderer->DrawVertexArray( localVerts );

	int k = 0;
	for (int i = 0; i < NumOfProductTypes; ++i) {
		if (resourcesAmount[i] > 0) {
			DrawResource( Vec2( middleLeftPos + Vec2( 0.f, textHeight * (2.5f - k) ) ), ProductDefinition::GetDefinition( i ), textHeight * 0.5f );
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( middleLeftPos + Vec2( textHeight * 1.2f, textHeight * (3.f - k) ), middleLeftPos + Vec2( 170.f, textHeight * (2.f - k) ) ), textHeight, Stringf( "%d", resourcesAmount[i] ), textColor, 0.618f, Vec2( 0.f, 0.5f ), TextBoxMode::OVERRUN );
			++k;
		}
	}
}

void AddVertsForElectricityPanel( std::vector<Vertex_PCU>& verts, std::vector<Vertex_PCU>& textVerts, Vec2 const& middleLeftPos, PowerNetwork* network, Rgba8 const& backgroundColor /*= Rgba8::WHITE*/, Rgba8 const& textColor /*= Rgba8::BLACK */ )
{
	constexpr float textHeight = 16.f;
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( middleLeftPos + Vec2( 0.f, textHeight * 0.5f ), middleLeftPos + Vec2( 200.f, textHeight * 1.5f ) ), textHeight, Stringf( "Power Generated: %.1f", network->m_powerGenerated ), textColor, 0.618f, Vec2( 0.f, 0.5f ) );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( middleLeftPos + Vec2( 0.f, textHeight * -0.5f ), middleLeftPos + Vec2( 200.f, textHeight * 0.5f ) ), textHeight, Stringf( "Power Consumed:  %.1f", network->m_powerConsumed ), textColor, 0.618f, Vec2( 0.f, 0.5f ) );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( middleLeftPos + Vec2( 0.f, textHeight * -1.5f ), middleLeftPos + Vec2( 200.f, textHeight * -0.5f ) ), textHeight, Stringf( "Power Remained:  %.1f", network->m_netWorkPower ), textColor, 0.618f, Vec2( 0.f, 0.5f ) );

	AddVertsForAABB2D( verts, AABB2( middleLeftPos - Vec2( textHeight * 0.5f, textHeight * 2.f ), middleLeftPos + Vec2( 230.f, textHeight * 2.f ) ), backgroundColor );
}

Map* GetCurMap()
{
	return g_theGame->m_map;
}

Direction GetInversedDir( Direction dir )
{
	if (dir == Direction::Down) {
		return Direction::Up;
	}
	else if (dir == Direction::Up) {
		return Direction::Down;
	}
	else if (dir == Direction::Left) {
		return Direction::Right;
	}
	else if (dir == Direction::Right) {
		return Direction::Left;
	}
	return Direction::None;
}

Direction GetTurnLeftDir( Direction dir )
{
	if (dir == Direction::Down) {
		return Direction::Right;
	}
	else if (dir == Direction::Up) {
		return Direction::Left;
	}
	else if (dir == Direction::Left) {
		return Direction::Down;
	}
	else if (dir == Direction::Right) {
		return Direction::Up;
	}
	return Direction::None;
}

Direction GetTurnRightDir( Direction dir )
{
	if (dir == Direction::Down) {
		return Direction::Left;
	}
	else if (dir == Direction::Up) {
		return Direction::Right;
	}
	else if (dir == Direction::Left) {
		return Direction::Up;
	}
	else if (dir == Direction::Right) {
		return Direction::Down;
	}
	return Direction::None;
}

Direction GetDir( IntVec2 const& unitIntVec2 )
{
	if (unitIntVec2 == DirectionUnitIntVec[(int)Direction::Down]) {
		return Direction::Down;
	}
	if (unitIntVec2 == DirectionUnitIntVec[(int)Direction::Up]) {
		return Direction::Up;
	}
	if (unitIntVec2 == DirectionUnitIntVec[(int)Direction::Left]) {
		return Direction::Left;
	}
	if (unitIntVec2 == DirectionUnitIntVec[(int)Direction::Right]) {
		return Direction::Right;
	}
	return Direction::None;
}

float GetOrientationDegreesFromDir( Direction dir )
{
	if (dir == Direction::Down) {
		return -90.f;
	}
	else if (dir == Direction::Left) {
		return 180.f;
	}
	else if (dir == Direction::Right) {
		return 0.f;
	}
	else if (dir == Direction::Up) {
		return 90.f;
	}
	else {
		return 0.f;
	}
}

