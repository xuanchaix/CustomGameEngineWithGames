#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Entity.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include <filesystem>

Game::Game()
{
	// load random number generator
	m_randNumGen = new RandomNumberGenerator(2);
	m_gameClock = new Clock();
}

Game::~Game()
{
	// delete all pointers managed by Game
	delete m_randNumGen;
	m_randNumGen = nullptr;
	for (auto convexPtr : m_convexArray) {
		delete convexPtr;
	}
}

void Game::Startup()
{
	std::filesystem::create_directory( "Data/Scenes" );
	// set up camera
	m_screenCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );
	m_screenCamera.m_mode = CameraMode::Orthographic;
	m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 1.f, -1.f );
	m_worldCamera.m_mode = CameraMode::Orthographic;

	constexpr int numOfInitialPolys = 16;
	// generate 16 convex polys
	for (int i = 0; i < numOfInitialPolys; ++i) {
		m_convexArray.push_back( GenerateRandomConvex( i ) );
	}

	m_rayStart = GetRandomPointInAABB2D( AABB2( Vec2( 0.f, 0.f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) ) );
	m_rayEnd = GetRandomPointInAABB2D( AABB2( Vec2( 0.f, 0.f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) ) );

	RebuildAllTrees();
// 	WriteTestBuffer();
// 	ReadTestBuffer();

	SubscribeEventCallbackFunction( "Command_SaveConvexScene", SaveConvexSceneCommand );
	SubscribeEventCallbackFunction( "Command_LoadConvexScene", LoadConvexSceneCommand );
}

void Game::Update()
{
	HandleKeys();
}

void Game::Render() const
{
	// Game Camera
	g_theRenderer->BeginCamera( m_worldCamera );
	std::vector<Vertex_PCU> verts;
	if (m_drawEdgesMode) {
		for (auto const convexPtr : m_convexArray) {
			if (convexPtr == m_hoveringConvex) {
			}
			else {
				AddVertsForConvexPoly2D( verts, convexPtr->m_convexPoly, Rgba8( 204, 229, 255, 128 ) );
				AddVertsForConvexPolyEdges( verts, convexPtr->m_convexPoly, 0.5f, Rgba8( 0, 0, 153 ) );
			}
		}
		for (auto const convexPtr : m_convexArray) {
			if (convexPtr == m_hoveringConvex) {
			}
			else {
				AddVertsForConvexPoly2D( verts, convexPtr->m_convexPoly, Rgba8( 204, 229, 255, 128 ) );
				AddVertsForConvexPolyEdges( verts, convexPtr->m_convexPoly, 0.5f, Rgba8( 0, 0, 153 ) );
			}
		}
		if (m_hoveringConvex) {
			if (m_isDragging) {
				AddVertsForConvexPoly2D( verts, m_hoveringConvex->m_convexPoly, Rgba8( 60, 60, 60, 128 ) );
				AddVertsForConvexPolyEdges( verts, m_hoveringConvex->m_convexPoly, 0.8f, Rgba8( 0, 0, 0 ) );
			}
			else {
				AddVertsForConvexPoly2D( verts, m_hoveringConvex->m_convexPoly, Rgba8( 0, 102, 204, 128 ) );
				AddVertsForConvexPolyEdges( verts, m_hoveringConvex->m_convexPoly, 0.8f, Rgba8( 0, 0, 0 ) );
			}
		}
	}
	else {
		for (auto const convexPtr : m_convexArray) {
			if (convexPtr == m_hoveringConvex) {
			}
			else {
				AddVertsForConvexPolyEdges( verts, convexPtr->m_convexPoly, 0.8f, Rgba8( 0, 0, 153 ) );
			}
		}
		for (auto const convexPtr : m_convexArray) {
			if (convexPtr == m_hoveringConvex) {
			}
			else {
				AddVertsForConvexPoly2D( verts, convexPtr->m_convexPoly, Rgba8( 153, 204, 255 ) );
			}
		}
		if (m_hoveringConvex) {
			if (m_isDragging) {
				AddVertsForConvexPoly2D( verts, m_hoveringConvex->m_convexPoly, Rgba8( 60, 60, 60, 128 ) );
				AddVertsForConvexPolyEdges( verts, m_hoveringConvex->m_convexPoly, 0.8f, Rgba8( 0, 0, 0 ) );
			}
			else {
				AddVertsForConvexPoly2D( verts, m_hoveringConvex->m_convexPoly, Rgba8( 0, 102, 204 ) );
				AddVertsForConvexPolyEdges( verts, m_hoveringConvex->m_convexPoly, 0.8f, Rgba8( 0, 0, 0 ) );
			}
		}
	}

	if (m_debugDrawMode) {
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->SetModelConstants();
		for (int i = 0; i < (int)m_convexArray.size(); ++i) {
			DebugDrawRing( m_convexArray[i]->m_boundingDiscCenter, m_convexArray[i]->m_boundingRadius, 0.3f, Rgba8( 100, 100, 100, 160 ) );
			AABB2 const& box = m_convexArray[i]->m_boundingAABB;
			DebugDrawLine( box.m_mins, Vec2( box.m_mins.x, box.m_maxs.y ), 0.3f, Rgba8( 100, 100, 100, 160 ) );
			DebugDrawLine( Vec2( box.m_mins.x, box.m_maxs.y ), box.m_maxs, 0.3f, Rgba8( 100, 100, 100, 160 ) );
			DebugDrawLine( Vec2( box.m_maxs.x, box.m_mins.y ), box.m_maxs, 0.3f, Rgba8( 100, 100, 100, 160 ) );
			DebugDrawLine( Vec2( box.m_maxs.x, box.m_mins.y ), box.m_mins, 0.3f, Rgba8( 100, 100, 100, 160 ) );
		}
	}

	if (m_debugDrawBVHMode) {
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->SetModelConstants();
		for (auto& node : m_AABB2Tree.m_nodes) {
			AABB2 const& box = node.m_bounds;
			DebugDrawLine( box.m_mins, Vec2( box.m_mins.x, box.m_maxs.y ), 0.3f, Rgba8( 100, 100, 100, 160 ) );
			DebugDrawLine( Vec2( box.m_mins.x, box.m_maxs.y ), box.m_maxs, 0.3f, Rgba8( 100, 100, 100, 160 ) );
			DebugDrawLine( Vec2( box.m_maxs.x, box.m_mins.y ), box.m_maxs, 0.3f, Rgba8( 100, 100, 100, 160 ) );
			DebugDrawLine( Vec2( box.m_maxs.x, box.m_mins.y ), box.m_mins, 0.3f, Rgba8( 100, 100, 100, 160 ) );
		}
	}

	RayCastResult2D rayResult;
	float rayMaxLength = (m_rayEnd - m_rayStart).GetLength();
	Vec2 rayNormal = (m_rayEnd - m_rayStart) / rayMaxLength;
	for (int i = 0; i < (int)m_convexArray.size(); i++) {
		RayCastResult2D thisRayResult;
		m_convexArray[i]->RayCastVsConvex2D( thisRayResult, m_rayStart, rayNormal, rayMaxLength );
		if (thisRayResult.m_didImpact && (thisRayResult.m_impactDist < rayResult.m_impactDist || !rayResult.m_didImpact)) {
			rayResult = thisRayResult;
		}
	}

	if (rayResult.m_didImpact) {
		AddVertsForArrow2D( verts, m_rayStart, m_rayEnd, 1.f, 0.4f, Rgba8( 0, 0, 0 ) );
		AddVertsForLineSegment2D( verts, m_rayStart, rayResult.m_impactPos, 0.4f, Rgba8( 0, 255, 0 ) );
		AddVertsForArrow2D( verts, rayResult.m_impactPos, rayResult.m_impactPos + rayResult.m_impactNormal * 3.f, 1.f, 0.4f, Rgba8( 255, 0, 0 ) );
	}
	else {
		AddVertsForArrow2D( verts, m_rayStart, m_rayEnd, 1.f, 0.4f, Rgba8( 0, 0, 0 ) );
	}

	if ((int)m_convexArray.size() == 1) {
		for (auto const& plane : m_convexArray[0]->m_convexHull.m_boundingPlanes) {
			if (plane.GetAltitudeOfPoint( m_rayStart ) > 0.f && DotProduct2D( rayNormal, plane.m_normal ) < 0.f) {
				float dist = 0.f;
				float NdotF = DotProduct2D( rayNormal, plane.m_normal );
				if (NdotF == 0.f) {
					// cannot reach here
					continue;
				}
				else {
					float SdotN = DotProduct2D( m_rayStart, plane.m_normal );
					dist = (plane.m_distanceFromOrigin - SdotN) / NdotF;
				}

				Vec2 vert1 = plane.GetOriginPoint() + 1000.f * plane.m_normal.GetRotated90Degrees();
				Vec2 vert2 = plane.GetOriginPoint() - 1000.f * plane.m_normal.GetRotated90Degrees();
				AddVertsForLineSegment2D( verts, vert1, vert2, 0.2f, Rgba8( 255, 0, 255 ) );
				AddVertsForDisc2D( verts, m_rayStart + dist * rayNormal, 0.5f, Rgba8( 255, 0, 255 ) );
			}
			else if ((plane.GetAltitudeOfPoint( m_rayStart ) > 0.f && DotProduct2D( rayNormal, plane.m_normal ) >= 0.f)
				|| (plane.GetAltitudeOfPoint( m_rayStart ) <= 0.f && DotProduct2D( rayNormal, plane.m_normal ) >= 0.f)) {
				Vec2 vert1 = plane.GetOriginPoint() + 1000.f * plane.m_normal.GetRotated90Degrees();
				Vec2 vert2 = plane.GetOriginPoint() - 1000.f * plane.m_normal.GetRotated90Degrees();
				AddVertsForLineSegment2D( verts, vert1, vert2, 0.2f, Rgba8( 255, 0, 0 ) );
			}
			else {
				Vec2 vert1 = plane.GetOriginPoint() + 1000.f * plane.m_normal.GetRotated90Degrees();
				Vec2 vert2 = plane.GetOriginPoint() - 1000.f * plane.m_normal.GetRotated90Degrees();
				AddVertsForLineSegment2D( verts, vert1, vert2, 0.2f, Rgba8( 0, 255, 0 ) );
			}
		}
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->EndCamera( m_worldCamera );
	// UI Camera
	g_theRenderer->BeginCamera( m_screenCamera );

	std::vector<Vertex_PCU> textVerts;

	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2(Vec2(10.f, 773.f), Vec2(1600.f, 790.f)), 17.f, 
		Stringf("S/E=RayStart/End,W/R=RotateObject,L/K=ScaleObject,LMB=DragObject,F2=DrawMode,F3-ShowBVH,F4=ShowDiscs,F8=Randomize"), 
		Rgba8( 0, 0, 0 ), 0.618f, Vec2( 0.f, 0.f ) );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 10.f, 752.f ), Vec2( 1600.f, 769.f ) ), 17.f,
		Stringf( "%d convex shapes (N/M to halve/double); T=Test with %d random rays (Y/U to halve/double)", (int)m_convexArray.size(), m_numOfRandomRays ),
		Rgba8( 0, 0, 0 ), 0.618f, Vec2( 0.f, 0.f ) );

	if (m_avgDist != 0.f) {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 10.f, 731.f ), Vec2( 1600.f, 748.f ) ), 17.f,
			Stringf( "%d Rays Vs. %d objects: avg dist %.3f", m_numOfRandomRays, (int)m_convexArray.size(), m_avgDist ),
			Rgba8( 0, 0, 0 ), 0.618f, Vec2( 0.f, 0.f ) );
	}
	if (m_avgDist != 0.f) {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 10.f, 712.f ), Vec2( 1600.f, 729.f ) ), 17.f,
			Stringf( "No Optimization Cost: %.2fms Disc Rejection Optimization Cost: %.2fms AABB Rejection Optimization Cost: %.2fms", m_lastRayTestNormalTime, m_lastRayTestDiscRejectionTime, m_lastRayTestAABBRejectionTime ),
			Rgba8( 0, 0, 0 ), 0.618f, Vec2( 0.f, 0.f ) );
	}
	if (m_avgDist != 0.f) {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 10.f, 691.f ), Vec2( 1600.f, 708.f ) ), 17.f,
			Stringf( "Symmetric Quad Tree Cost: %.2fms AABB2(BVH) Tree Cost: %.2fms", m_lastRayTestSymmetricTreeTime, m_lastRayTestAABBTreeTime ),
			Rgba8( 0, 0, 0 ), 0.618f, Vec2( 0.f, 0.f ) );
	}

	
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );

	g_theRenderer->EndCamera( m_screenCamera );
}

void Game::HandleKeys()
{
	float deltaSeconds = m_gameClock->GetDeltaSeconds();
	// Get cursor position
	Vec2 cursorNormalizedPos = g_theInput->GetCursorNormalizedPosition();
	Vec2 cursorPos = m_worldCamera.GetCursorWorldPosition( cursorNormalizedPos );

	if (g_theInput->IsKeyDown( 'E' ) || g_theInput->IsKeyDown(KEYCODE_RIGHTMOUSE)) {
		m_rayEnd = cursorPos;
	}
	if (g_theInput->IsKeyDown( 'S' ) || g_theInput->IsKeyDown(KEYCODE_LEFTMOUSE)) {
		m_rayStart = cursorPos;
	}

	// scale object
	if (m_hoveringConvex && g_theInput->IsKeyDown( 'L' )) {
		m_hoveringConvex->Scale( 1.f * deltaSeconds, cursorPos );
		RebuildAllTrees();
	}
	if (m_hoveringConvex && g_theInput->IsKeyDown( 'K' )) {
		m_hoveringConvex->Scale( -1.f * deltaSeconds, cursorPos );
		RebuildAllTrees();
	}

	// rotate object
	if (m_hoveringConvex && g_theInput->IsKeyDown( 'W' )) {
		m_hoveringConvex->Rotate( 90.f * deltaSeconds, cursorPos );
		RebuildAllTrees();
	}
	if (m_hoveringConvex && g_theInput->IsKeyDown( 'R' )) {
		m_hoveringConvex->Rotate( -90.f * deltaSeconds, cursorPos );
		RebuildAllTrees();
	}

	// move polygon
	if (m_hoveringConvex && g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE )) {
		m_isDragging = true;
	}
	if (m_isDragging && m_hoveringConvex && g_theInput->IsKeyDown( KEYCODE_LEFTMOUSE )) {
		m_hoveringConvex->Translate( cursorPos - m_cursorPrevPos );
		RebuildAllTrees();
	}
	if (g_theInput->WasKeyJustReleased( KEYCODE_LEFTMOUSE )) {
		m_isDragging = false;
	}


	// check if cursor is inside one polygon
	if (!m_isDragging) {
		if ((m_hoveringConvex && !m_hoveringConvex->IsPointInside( cursorPos )) || !m_hoveringConvex) {
			m_hoveringConvex = nullptr;
			for (int i = (int)m_convexArray.size() - 1; i >= 0 ;--i) {
				if (m_convexArray[i]->IsPointInside( cursorPos )) {
					m_hoveringConvex = m_convexArray[i];
					break;
				}
			}
		}
	}

	// double\half ray count
	if (g_theInput->WasKeyJustPressed( 'Y' )) {
		if (m_numOfRandomRays != 1) {
			m_numOfRandomRays /= 2;
		}
	}
	if (g_theInput->WasKeyJustPressed( 'U' )) {
		if (m_numOfRandomRays != 134217728) {
			m_numOfRandomRays *= 2;
		}
	}

	// double\half polygon count
	if (g_theInput->WasKeyJustPressed( 'N' )) {
		int numOfShapesToRemove = (int)m_convexArray.size() / 2;
		if ((int)m_convexArray.size() == 1) {
			numOfShapesToRemove = 1;
		}
		for (int i = 0; i < numOfShapesToRemove; ++i) {
			if (m_convexArray.back() == m_hoveringConvex) {
				m_hoveringConvex = nullptr;
			}
			delete m_convexArray.back();
			m_convexArray.pop_back();
		}
		RebuildAllTrees();
	}
	if (g_theInput->WasKeyJustPressed( 'M' )) {
		int numOfShapesToAdd = (int)m_convexArray.size();
		if (numOfShapesToAdd == 0) {
			numOfShapesToAdd = 1;
		}
		if (numOfShapesToAdd != 2048) {
			int startOfAddIndex = (int)m_convexArray.size();
			for (int i = startOfAddIndex; i < startOfAddIndex + numOfShapesToAdd; ++i) {
				m_convexArray.push_back( GenerateRandomConvex( i ) );
			}
		}
		RebuildAllTrees();
	}

	// test with a lot of rays
	if (g_theInput->WasKeyJustPressed( 'T' )) {
		TestRays();
	}

	// save
	if (g_theInput->WasKeyJustPressed( 'Z' )) {
		SaveCurrentConvexScene( "Data/Scenes/TestSaveFile.bin" );
	}
	if (g_theInput->WasKeyJustPressed( 'X' )) {
		LoadConvexSceneToCurrent( "Data/Scenes/TestSaveFile.bin" );
	}

	// enable\disable debug draw mode
	if (g_theInput->WasKeyJustPressed( KEYCODE_F4 )) // F4 debug mode
	{
		m_debugDrawMode = !m_debugDrawMode;
	}

	if (g_theInput->WasKeyJustPressed( KEYCODE_F2 )) {
		m_drawEdgesMode = !m_drawEdgesMode;
	}

	if (g_theInput->WasKeyJustPressed( KEYCODE_F3 )) {
		m_debugDrawBVHMode = !m_debugDrawBVHMode;
	}
	// re-randomize
	if (g_theInput->WasKeyJustPressed( KEYCODE_F8 )) { // F8 to restart the game
		int numOfShapes = (int)m_convexArray.size();
		ClearScene();
		m_seed += 1;
		for (int i = 0; i < numOfShapes; ++i) {
			m_convexArray.push_back( GenerateRandomConvex( i ) );
		}
	}

	m_cursorPrevPos = cursorPos;
}

void Game::RebuildAllTrees()
{
	RebuildAABB2Tree();
	RebuildSymmetricQuadTree();
}

void Game::RebuildAABB2Tree()
{
	if ((int)m_convexArray.size() != 0) {
		int numOfBVH = int( (int)log2( (double)m_convexArray.size() ) - 3 ) > 3 ? int( (int)log2( (double)m_convexArray.size() ) - 3 ) : 3;
		m_AABB2Tree.BuildTree( m_convexArray, numOfBVH, AABB2( Vec2( 0.f, 0.f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) ) );
	}
	else {
		m_AABB2Tree.BuildTree( m_convexArray, 0, AABB2( Vec2( 0.f, 0.f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) ) );
	}
}

void Game::RebuildSymmetricQuadTree()
{
	m_symQuadTree.BuildTree( m_convexArray, 4, AABB2( Vec2( 0.f, 0.f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) ) );
}

void Game::TestRays()
{
	// build rays
	std::vector<Vec2> rayStartPos;
	std::vector<Vec2> rayForwardNormal;
	std::vector<float> rayMaxDist;
	for (int i = 0; i < m_numOfRandomRays; ++i) {
		Vec2 startPos = GetRandomPointInAABB2D( AABB2( Vec2( 0.f, 0.f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) ) );
		Vec2 endPos = GetRandomPointInAABB2D( AABB2( Vec2( 0.f, 0.f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) ) );
		rayStartPos.push_back( startPos );
		Vec2 disp = endPos - startPos;
		float dist = disp.GetLength();
		rayMaxDist.push_back( dist );
		rayForwardNormal.push_back( disp / dist );
	}

	// test no optimization
	double startTime = GetCurrentTimeSeconds();
	float sumDist = 0.f;
	int correctNumOfRayHit = 0;
	RayCastResult2D rayRes;
	for (int j = 0; j < m_numOfRandomRays; ++j) {
		float minDist = FLT_MAX;
		for (int i = 0; i < (int)m_convexArray.size(); ++i) {
			if (m_convexArray[i]->RayCastVsConvex2D( rayRes, rayStartPos[j], rayForwardNormal[j], rayMaxDist[j], false )) {
				if (rayRes.m_impactDist < minDist) {
					minDist = rayRes.m_impactDist;
				}
			}
		}
		if (minDist != FLT_MAX) {
			sumDist += minDist;
			++correctNumOfRayHit;
		}
	}
	double endTime = GetCurrentTimeSeconds();
	m_avgDist = sumDist / (float)correctNumOfRayHit;
	m_lastRayTestNormalTime = float( (endTime - startTime) * 1000.0 );

	// test disc rejection
	startTime = GetCurrentTimeSeconds();
	sumDist = 0.f;
	int numOfRayHit = 0;
	for (int j = 0; j < m_numOfRandomRays; ++j) {
		float minDist = FLT_MAX;
		for (int i = 0; i < (int)m_convexArray.size(); ++i) {
			if (m_convexArray[i]->RayCastVsConvex2D( rayRes, rayStartPos[j], rayForwardNormal[j], rayMaxDist[j], true )) {
				if (rayRes.m_impactDist < minDist) {
					minDist = rayRes.m_impactDist;
				}
			}
		}
		if (minDist != FLT_MAX) {
			sumDist += minDist;
			++numOfRayHit;
		}
	}
	endTime = GetCurrentTimeSeconds();
	float thisAvgDist = sumDist / (float)numOfRayHit;
	//GUARANTEE_OR_DIE( abs( thisAvgDist - m_avgDist ) < 0.001f, "Error!" );
	GUARANTEE_OR_DIE( numOfRayHit == correctNumOfRayHit, "Error!" );
	m_lastRayTestDiscRejectionTime = float( (endTime - startTime) * 1000.0 );

	// test AABB2 rejection
	startTime = GetCurrentTimeSeconds();
	sumDist = 0.f;
	numOfRayHit = 0;
	for (int j = 0; j < m_numOfRandomRays; ++j) {
		float minDist = FLT_MAX;
		for (int i = 0; i < (int)m_convexArray.size(); ++i) {
			if (m_convexArray[i]->RayCastVsConvex2D( rayRes, rayStartPos[j], rayForwardNormal[j], rayMaxDist[j], false, true )) {
				if (rayRes.m_impactDist < minDist) {
					minDist = rayRes.m_impactDist;
				}
			}
		}
		if (minDist != FLT_MAX) {
			sumDist += minDist;
			++numOfRayHit;
		}
	}
	endTime = GetCurrentTimeSeconds();
	thisAvgDist = sumDist / (float)numOfRayHit;
	//GUARANTEE_OR_DIE( abs( thisAvgDist - m_avgDist ) < 0.001f, "Error!" );
	GUARANTEE_OR_DIE( numOfRayHit == correctNumOfRayHit, "Error!" );
	m_lastRayTestAABBRejectionTime = float( (endTime - startTime) * 1000.0 );

	// test symmetric quad tree
	startTime = GetCurrentTimeSeconds();
	sumDist = 0.f;
	numOfRayHit = 0;
	for (int j = 0; j < m_numOfRandomRays; ++j) {
		std::vector<Convex2*> latentRes;
		m_symQuadTree.SolveRayResult( rayStartPos[j], rayForwardNormal[j], rayMaxDist[j], m_convexArray, latentRes );
		float minDist = FLT_MAX;
		for (int i = 0; i < (int)latentRes.size(); ++i) {
			if (latentRes[i]->RayCastVsConvex2D( rayRes, rayStartPos[j], rayForwardNormal[j], rayMaxDist[j], true, false )) {
				if (rayRes.m_impactDist < minDist) {
					minDist = rayRes.m_impactDist;
				}
			}
		}
		if (minDist != FLT_MAX) {
			sumDist += minDist;
			++numOfRayHit;
		}
	}
	endTime = GetCurrentTimeSeconds();
	thisAvgDist = sumDist / (float)numOfRayHit;
	//GUARANTEE_OR_DIE( abs( thisAvgDist - m_avgDist ) < 0.001f, "Error!" );
	GUARANTEE_OR_DIE( numOfRayHit == correctNumOfRayHit, "Error!" );
	m_lastRayTestSymmetricTreeTime = float( (endTime - startTime) * 1000.0 );
	
	// test AABB2 BVH tree
	startTime = GetCurrentTimeSeconds();
	sumDist = 0.f;
	numOfRayHit = 0;
	for (int j = 0; j < m_numOfRandomRays; ++j) {
		std::vector<Convex2*> latentRes;
		m_AABB2Tree.SolveRayResult( rayStartPos[j], rayForwardNormal[j], rayMaxDist[j], latentRes );
		float minDist = FLT_MAX;
		for (int i = 0; i < (int)latentRes.size(); ++i) {
			if (latentRes[i]->RayCastVsConvex2D( rayRes, rayStartPos[j], rayForwardNormal[j], rayMaxDist[j], true, false )) {
				if (rayRes.m_impactDist < minDist) {
					minDist = rayRes.m_impactDist;
				}
			}
		}
		if (minDist != FLT_MAX) {
			sumDist += minDist;
			++numOfRayHit;
		}
	}
	endTime = GetCurrentTimeSeconds();
	thisAvgDist = sumDist / (float)numOfRayHit;
	//GUARANTEE_OR_DIE( abs( thisAvgDist - m_avgDist ) < 0.001f, "Error!" );
	GUARANTEE_OR_DIE( numOfRayHit == correctNumOfRayHit, "Error!" );
	m_lastRayTestAABBTreeTime = float( (endTime - startTime) * 1000.0 );
}

Convex2* Game::GenerateRandomConvex( int index ) const
{
	int numOfVerts = (Get1dNoiseUint( index, m_seed ) & 0x7fffffff) % (8 - 3 + 1) + 3;
	if (numOfVerts >= 7) { // make sure there are not too many 7\8 vertices convex polys
		numOfVerts = (Get1dNoiseUint( index, m_seed + 1 ) & 0x7fffffff) % (8 - 3 + 1) + 3;
	}
	std::vector<float> angles;
	for (int i = 0; i < numOfVerts; ++i) {
		angles.push_back( Get2dNoiseZeroToOne( index, i, m_seed + 5 ) * 360.f / (float)numOfVerts + (float)i * 360.f / (float)numOfVerts );
	}
	//std::sort( angles.begin(), angles.end() );
	float radius = Get1dNoiseZeroToOne( index, m_seed + 2 ) * 12.f + 4.f;
	//float radius = Get1dNoiseZeroToOne( index, m_seed + 2 ) * 1.f + 1.f;
	Vec2 centerPos = Vec2(Get1dNoiseZeroToOne( index, m_seed + 3 ) * (WORLD_SIZE_X - 2.f * radius) + radius,
						Get1dNoiseZeroToOne( index, m_seed + 4 ) * (WORLD_SIZE_Y - 2.f * radius) + radius);
	std::vector<Vec2> vertPos;
	for (int i = 0; i < numOfVerts; ++i) {
		vertPos.push_back( Vec2::MakeFromPolarDegrees( angles[i], radius ) + centerPos );
	}
	Convex2* retValue = new Convex2( vertPos );
	retValue->m_boundingDiscCenter = centerPos;
	retValue->m_boundingRadius = radius;
	retValue->RebuildBoundingBox();
	return retValue;
}

void Game::AddVertsForConvexPolyEdges( std::vector<Vertex_PCU>& verts, ConvexPoly2 const& convexPoly2, float thickness,  Rgba8 const& color ) const
{
	int vertexCount = convexPoly2.GetVertexCount();
	if (vertexCount <= 1) { // cannot draw a polygon with less than 3 vertices
		return;
	}
	std::vector<Vec2> const& vertexArray = convexPoly2.GetVertexArray();
	for (int i = 0; i < vertexCount - 1; ++i) {
		AddVertsForLineSegment2D( verts, vertexArray[i], vertexArray[i + 1], thickness, color );
	}
	AddVertsForLineSegment2D( verts, vertexArray[vertexCount - 1], vertexArray[0], thickness, color );
}

void Game::AddVertsForConvexHullPlanes( std::vector<Vertex_PCU>& verts, ConvexHull2 const& convexHull2, float thickness, Rgba8 const& color ) const
{
	int planeCount = (int)convexHull2.m_boundingPlanes.size();
	for (int i = 0; i < planeCount; ++i) {
		Vec2 vert1 = convexHull2.m_boundingPlanes[i].GetOriginPoint() + 1000.f * convexHull2.m_boundingPlanes[i].m_normal.GetRotated90Degrees();
		Vec2 vert2 = convexHull2.m_boundingPlanes[i].GetOriginPoint() - 1000.f * convexHull2.m_boundingPlanes[i].m_normal.GetRotated90Degrees();
		AddVertsForLineSegment2D( verts, vert1, vert2, thickness, color );
	}
}

void Game::WriteTestBuffer()
{
	std::vector<uint8_t> buffer;
	buffer.reserve( 1000 );
	BufferWriter bufWrite = BufferWriter( buffer );
	bufWrite.SetPreferredEndianMode( EndianMode::Little );
	bufWrite.AppendChar( 'T' );
	bufWrite.AppendChar( 'E' );
	bufWrite.AppendChar( 'S' );
	bufWrite.AppendChar( 'T' );
	bufWrite.AppendByte( 2 ); // Version 2
	bufWrite.AppendByte( (unsigned char)bufWrite.GetPreferredEndianMode() );
	bufWrite.AppendBool( false );
	bufWrite.AppendBool( true );
	bufWrite.AppendUint32( 0x12345678 );
	bufWrite.AppendInt32( -7 ); // signed 32-bit int
	bufWrite.AppendFloat( 1.f ); // in memory looks like hex: 00 00 80 3F (or 3F 80 00 00 in big endian)
	bufWrite.AppendDouble( 3.1415926535897932384626433832795 ); // actually 3.1415926535897931 (best it can do)
	bufWrite.AppendZeroTerminatedString( "Hello" ); // written with a trailing 0 ('\0') after (6 bytes total)
	bufWrite.AppendLengthPrecededString( "Is this thing on?" ); // uint 17, then 17 chars (no zero-terminator after)
	bufWrite.AppendRgba8( Rgba8( 200, 100, 50, 255 ) ); // four bytes in RGBA order (endian-independent)
	bufWrite.AppendByte( 8 ); // 0x08 == 8 (byte)
	bufWrite.AppendRgb8( Rgba8( 238, 221, 204, 255 ) ); // written as 3 bytes (RGB) only; ignores Alpha
	bufWrite.AppendByte( 9 ); // 0x09 == 9 (byte)
	bufWrite.AppendIntVec2( IntVec2( 1920, 1080 ) );
	bufWrite.AppendVec2( Vec2( -0.6f, 0.8f ) );
	bufWrite.AppendVertexPCU( Vertex_PCU( Vec3( 3.f, 4.f, 5.f ), Rgba8( 100, 101, 102, 103 ), Vec2( 0.125f, 0.625f ) ) );

	bufWrite.SetPreferredEndianMode( EndianMode::Big );
	bufWrite.AppendChar( 'T' );
	bufWrite.AppendChar( 'E' );
	bufWrite.AppendChar( 'S' );
	bufWrite.AppendChar( 'T' );
	bufWrite.AppendByte( 2 ); // Version 2
	bufWrite.AppendByte( (unsigned char)bufWrite.GetPreferredEndianMode() );
	bufWrite.AppendBool( false );
	bufWrite.AppendBool( true );
	bufWrite.AppendUint32( 0x12345678 );
	bufWrite.AppendInt32( -7 ); // signed 32-bit int
	bufWrite.AppendFloat( 1.f ); // in memory looks like hex: 00 00 80 3F (or 3F 80 00 00 in big endian)
	bufWrite.AppendDouble( 3.1415926535897932384626433832795 ); // actually 3.1415926535897931 (best it can do)
	bufWrite.AppendZeroTerminatedString( "Hello" ); // written with a trailing 0 ('\0') after (6 bytes total)
	bufWrite.AppendLengthPrecededString( "Is this thing on?" ); // uint 17, then 17 chars (no zero-terminator after)
	bufWrite.AppendRgba8( Rgba8( 200, 100, 50, 255 ) ); // four bytes in RGBA order (endian-independent)
	bufWrite.AppendByte( 8 ); // 0x08 == 8 (byte)
	bufWrite.AppendRgb8( Rgba8( 238, 221, 204, 255 ) ); // written as 3 bytes (RGB) only; ignores Alpha
	bufWrite.AppendByte( 9 ); // 0x09 == 9 (byte)
	bufWrite.AppendIntVec2( IntVec2( 1920, 1080 ) );
	bufWrite.AppendVec2( Vec2( -0.6f, 0.8f ) );
	bufWrite.AppendVertexPCU( Vertex_PCU( Vec3( 3.f, 4.f, 5.f ), Rgba8( 100, 101, 102, 103 ), Vec2( 0.125f, 0.625f ) ) );
	BufferWriteToFile( buffer, "TestFile.bin" );
}

void Game::ReadTestBuffer()
{
	std::vector<uint8_t> buffer;
	buffer.reserve( 1000 );
	FileReadToBuffer( buffer, "TestFile.bin" );

	BufferReader bufParse = BufferReader( buffer );
	{
		// Parse known test file elements
		bufParse.SetBufferEndianMode( EndianMode::Little );
		char fourCC0_T = bufParse.ParseChar(); // 'T' == 0x54 hex == 84 decimal
		char fourCC1_E = bufParse.ParseChar(); // 'E' == 0x45 hex == 84 decimal
		char fourCC2_S = bufParse.ParseChar(); // 'S' == 0x53 hex == 69 decimal
		char fourCC3_T = bufParse.ParseChar(); // 'T' == 0x54 hex == 84 decimal
		unsigned char version = bufParse.ParseByte(); // version 2
		EndianMode mode = (EndianMode)bufParse.ParseByte(); // 1 for little endian, or 2 for big endian
		bool shouldBeFalse = bufParse.ParseBool(); // written in buffer as byte 0 or 1
		bool shouldBeTrue = bufParse.ParseBool(); // written in buffer as byte 0 or 1
		unsigned int largeUint = bufParse.ParseUint32(); // 0x12345678
		int negativeSeven = bufParse.ParseInt32(); // -7 (as signed 32-bit int)
		float oneF = bufParse.ParseFloat(); // 1.0f
		double pi = bufParse.ParseDouble(); // 3.1415926535897932384626433832795 (or as best it can)

		std::string helloString, isThisThingOnString;
		bufParse.ParseZeroTerminatedString( helloString ); // written with a trailing 0 ('\0') after (6 bytes total)
		bufParse.ParseLengthPrecededString( isThisThingOnString ); // written as uint 17, then 17 characters (no zero-terminator after)

		Rgba8 rustColor = bufParse.ParseRgba8(); // Rgba8( 200, 100, 50, 255 )
		unsigned char eight = bufParse.ParseByte(); // 0x08 == 8 (byte)
		Rgba8 seashellColor = bufParse.ParseRgb8(); // Rgba8( 238, 221, 204) written as 3 bytes (RGB) only; assume alpha is 255
		unsigned char nine = bufParse.ParseByte(); // 0x09 == 9 (byte)
		IntVec2 highDefRes = bufParse.ParseIntVec2(); // IntVector2( 1920, 1080 )
		Vec2 normal2D = bufParse.ParseVec2(); // Vector2( -0.6f, 0.8f )
		Vertex_PCU vertex = bufParse.ParseVertexPCU(); // VertexPCU( 3.f, 4.f, 5.f, Rgba(100,101,102,103), 0.125f, 0.625f ) );

		// Validate actual values parsed
		GUARANTEE_OR_DIE( fourCC0_T == 'T', "" );
		GUARANTEE_OR_DIE( fourCC1_E == 'E', "" );
		GUARANTEE_OR_DIE( fourCC2_S == 'S', "" );
		GUARANTEE_OR_DIE( fourCC3_T == 'T', "" );
		GUARANTEE_OR_DIE( version == 2, "" );
		GUARANTEE_OR_DIE( mode == EndianMode::Little, "" ); // verify that we're receiving things in the endianness we expect
		GUARANTEE_OR_DIE( shouldBeFalse == false, "" );
		GUARANTEE_OR_DIE( shouldBeTrue == true, "" );
		GUARANTEE_OR_DIE( largeUint == 0x12345678, "" );
		GUARANTEE_OR_DIE( negativeSeven == -7, "" );
		GUARANTEE_OR_DIE( oneF == 1.f, "" );
		GUARANTEE_OR_DIE( pi == 3.1415926535897932384626433832795, "" );
		GUARANTEE_OR_DIE( helloString == "Hello", "" );
		GUARANTEE_OR_DIE( isThisThingOnString == "Is this thing on?", "" );
		GUARANTEE_OR_DIE( rustColor == Rgba8( 200, 100, 50, 255 ), "" );
		GUARANTEE_OR_DIE( eight == 8, "" );
		GUARANTEE_OR_DIE( seashellColor == Rgba8( 238, 221, 204 ), "" );
		GUARANTEE_OR_DIE( nine == 9, "" );
		GUARANTEE_OR_DIE( highDefRes == IntVec2( 1920, 1080 ), "" );
		GUARANTEE_OR_DIE( normal2D == Vec2( -0.6f, 0.8f ), "" );
		GUARANTEE_OR_DIE( vertex.m_position == Vec3( 3.f, 4.f, 5.f ), "" );
		GUARANTEE_OR_DIE( vertex.m_color == Rgba8( 100, 101, 102, 103 ), "" );
		GUARANTEE_OR_DIE( vertex.m_uvTexCoords == Vec2( 0.125f, 0.625f ), "" );
	}

	{
		bufParse.SetBufferEndianMode( EndianMode::Big );
		char fourCC0_T = bufParse.ParseChar(); // 'T' == 0x54 hex == 84 decimal
		char fourCC1_E = bufParse.ParseChar(); // 'E' == 0x45 hex == 84 decimal
		char fourCC2_S = bufParse.ParseChar(); // 'S' == 0x53 hex == 69 decimal
		char fourCC3_T = bufParse.ParseChar(); // 'T' == 0x54 hex == 84 decimal
		unsigned char version = bufParse.ParseByte(); // version 2
		EndianMode mode = (EndianMode)bufParse.ParseByte(); // 1 for little endian, or 2 for big endian
		bool shouldBeFalse = bufParse.ParseBool(); // written in buffer as byte 0 or 1
		bool shouldBeTrue = bufParse.ParseBool(); // written in buffer as byte 0 or 1
		unsigned int largeUint = bufParse.ParseUint32(); // 0x12345678
		int negativeSeven = bufParse.ParseInt32(); // -7 (as signed 32-bit int)
		float oneF = bufParse.ParseFloat(); // 1.0f
		double pi = bufParse.ParseDouble(); // 3.1415926535897932384626433832795 (or as best it can)

		std::string helloString, isThisThingOnString;
		bufParse.ParseZeroTerminatedString( helloString ); // written with a trailing 0 ('\0') after (6 bytes total)
		bufParse.ParseLengthPrecededString( isThisThingOnString ); // written as uint 17, then 17 characters (no zero-terminator after)

		Rgba8 rustColor = bufParse.ParseRgba8(); // Rgba8( 200, 100, 50, 255 )
		unsigned char eight = bufParse.ParseByte(); // 0x08 == 8 (byte)
		Rgba8 seashellColor = bufParse.ParseRgb8(); // Rgba8( 238, 221, 204) written as 3 bytes (RGB) only; assume alpha is 255
		unsigned char nine = bufParse.ParseByte(); // 0x09 == 9 (byte)
		IntVec2 highDefRes = bufParse.ParseIntVec2(); // IntVector2( 1920, 1080 )
		Vec2 normal2D = bufParse.ParseVec2(); // Vector2( -0.6f, 0.8f )
		Vertex_PCU vertex = bufParse.ParseVertexPCU(); // VertexPCU( 3.f, 4.f, 5.f, Rgba(100,101,102,103), 0.125f, 0.625f ) );

		// Validate actual values parsed
		GUARANTEE_OR_DIE( fourCC0_T == 'T', "" );
		GUARANTEE_OR_DIE( fourCC1_E == 'E', "" );
		GUARANTEE_OR_DIE( fourCC2_S == 'S', "" );
		GUARANTEE_OR_DIE( fourCC3_T == 'T', "" );
		GUARANTEE_OR_DIE( version == 2, "" );
		GUARANTEE_OR_DIE( mode == EndianMode::Big, "" ); // verify that we're receiving things in the endianness we expect
		GUARANTEE_OR_DIE( shouldBeFalse == false, "" );
		GUARANTEE_OR_DIE( shouldBeTrue == true, "" );
		GUARANTEE_OR_DIE( largeUint == 0x12345678, "" );
		GUARANTEE_OR_DIE( negativeSeven == -7, "" );
		GUARANTEE_OR_DIE( oneF == 1.f, "" );
		GUARANTEE_OR_DIE( pi == 3.1415926535897932384626433832795, "" );
		GUARANTEE_OR_DIE( helloString == "Hello", "" );
		GUARANTEE_OR_DIE( isThisThingOnString == "Is this thing on?", "" );
		GUARANTEE_OR_DIE( rustColor == Rgba8( 200, 100, 50, 255 ), "" );
		GUARANTEE_OR_DIE( eight == 8, "" );
		GUARANTEE_OR_DIE( seashellColor == Rgba8( 238, 221, 204 ), "" );
		GUARANTEE_OR_DIE( nine == 9, "" );
		GUARANTEE_OR_DIE( highDefRes == IntVec2( 1920, 1080 ), "" );
		GUARANTEE_OR_DIE( normal2D == Vec2( -0.6f, 0.8f ), "" );
		GUARANTEE_OR_DIE( vertex.m_position == Vec3( 3.f, 4.f, 5.f ), "" );
		GUARANTEE_OR_DIE( vertex.m_color == Rgba8( 100, 101, 102, 103 ), "" );
		GUARANTEE_OR_DIE( vertex.m_uvTexCoords == Vec2( 0.125f, 0.625f ), "" );
	}
}

void Game::SaveCurrentConvexScene( std::string const& path )
{
	constexpr size_t offsetFromStartPrivateToDataSize = 4;

	std::vector<uint8_t> buffer;
	BufferWriter bufWrite = BufferWriter( buffer );

	bufWrite.AppendChar( 'G' ); // header
	bufWrite.AppendChar( 'H' );
	bufWrite.AppendChar( 'C' );
	bufWrite.AppendChar( 'S' );
	bufWrite.AppendByte( 33 ); // cohort number
	bufWrite.AppendByte( 1 ); // major version
	bufWrite.AppendByte( 1 ); // minor version
	bufWrite.AppendByte( (uint8_t)bufWrite.GetLocalEndianMode() );

	bufWrite.AppendByte( 0 ); // leave blank, future location of table of contents
	bufWrite.AppendByte( 0 );
	bufWrite.AppendByte( 0 );
	bufWrite.AppendByte( 0 );

	bufWrite.AppendChar( 'E' );
	bufWrite.AppendChar( 'N' );
	bufWrite.AppendChar( 'D' );
	bufWrite.AppendChar( 'H' );

	// first chunk: scene info chunk
	bufWrite.AppendChar( 'G' ); // header
	bufWrite.AppendChar( 'H' );
	bufWrite.AppendChar( 'C' );
	bufWrite.AppendChar( 'K' );
	bufWrite.AppendByte( 0x01 ); // chunk type
	bufWrite.AppendByte( (uint8_t)bufWrite.GetLocalEndianMode() ); // endian
	bufWrite.AppendByte( 0 ); // place holder for chunk data size
	bufWrite.AppendByte( 0 );
	bufWrite.AppendByte( 0 );
	bufWrite.AppendByte( 0 );

	size_t startPrivatePosOfSceneInfoChunk = buffer.size();

	//-------------------------------chunk data-------------------------------
	bufWrite.AppendAABB2( m_worldCamera.m_cameraBox );
	bufWrite.AppendUshort( (unsigned short)m_convexArray.size() );
	//-------------------------------chunk data-------------------------------

	size_t endPrivatePosOfSceneInfoChunk = buffer.size();
	bufWrite.WriteIntToPos( startPrivatePosOfSceneInfoChunk - offsetFromStartPrivateToDataSize, unsigned int(endPrivatePosOfSceneInfoChunk - startPrivatePosOfSceneInfoChunk) );

	bufWrite.AppendChar( 'E' );
	bufWrite.AppendChar( 'N' );
	bufWrite.AppendChar( 'D' );
	bufWrite.AppendChar( 'C' );

	// second chunk: objects chunk
	bufWrite.AppendChar( 'G' ); // header
	bufWrite.AppendChar( 'H' );
	bufWrite.AppendChar( 'C' );
	bufWrite.AppendChar( 'K' );
	bufWrite.AppendByte( 0x02 ); // chunk type
	bufWrite.AppendByte( (uint8_t)bufWrite.GetLocalEndianMode() ); // endian
	bufWrite.AppendByte( 0 ); // place holder for chunk data size
	bufWrite.AppendByte( 0 );
	bufWrite.AppendByte( 0 );
	bufWrite.AppendByte( 0 );

	size_t startPrivatePosOfObjectChunk = buffer.size();

	//-------------------------------chunk data-------------------------------
	bufWrite.AppendUshort( (unsigned short)m_convexArray.size() ); // num of objects in the scene
	for (auto convex : m_convexArray) {
		std::vector<Vec2> const& vertices = convex->m_convexPoly.GetVertexArray();
		int numOfVertices = (int)vertices.size();
		bufWrite.AppendByte( (uint8_t)numOfVertices );
		for (int i = 0; i < numOfVertices; ++i) {
			bufWrite.AppendVec2( vertices[i] );
		}
	}
	//-------------------------------chunk data-------------------------------

	size_t endPrivatePosOfObjectChunk = buffer.size();
	bufWrite.WriteIntToPos( startPrivatePosOfObjectChunk - offsetFromStartPrivateToDataSize, unsigned int(endPrivatePosOfObjectChunk - startPrivatePosOfObjectChunk) );

	bufWrite.AppendChar( 'E' );
	bufWrite.AppendChar( 'N' );
	bufWrite.AppendChar( 'D' );
	bufWrite.AppendChar( 'C' );

	// chunk: bounding discs chunk
	bufWrite.AppendChar( 'G' ); // header
	bufWrite.AppendChar( 'H' );
	bufWrite.AppendChar( 'C' );
	bufWrite.AppendChar( 'K' );
	bufWrite.AppendByte( 0x81 ); // chunk type
	bufWrite.AppendByte( (uint8_t)bufWrite.GetLocalEndianMode() ); // endian
	bufWrite.AppendByte( 0 ); // place holder for chunk data size
	bufWrite.AppendByte( 0 );
	bufWrite.AppendByte( 0 );
	bufWrite.AppendByte( 0 );

	size_t startPrivatePosOfBoundingDiscsChunk = buffer.size();

	//-------------------------------chunk data-------------------------------
	bufWrite.AppendUshort( (unsigned short)m_convexArray.size() ); // num of objects in the scene
	for (auto convex : m_convexArray) {
		bufWrite.AppendVec2( convex->m_boundingDiscCenter );	
		bufWrite.AppendFloat( convex->m_boundingRadius );
	}
	//-------------------------------chunk data-------------------------------

	size_t endPrivatePosOfBoundingDiscsChunk = buffer.size();
	bufWrite.WriteIntToPos( startPrivatePosOfBoundingDiscsChunk - offsetFromStartPrivateToDataSize, unsigned int( endPrivatePosOfBoundingDiscsChunk - startPrivatePosOfBoundingDiscsChunk ) );

	bufWrite.AppendChar( 'E' );
	bufWrite.AppendChar( 'N' );
	bufWrite.AppendChar( 'D' );
	bufWrite.AppendChar( 'C' );

	// chunk: convex hull chunk
	bufWrite.AppendChar( 'G' ); // header
	bufWrite.AppendChar( 'H' );
	bufWrite.AppendChar( 'C' );
	bufWrite.AppendChar( 'K' );
	bufWrite.AppendByte( 0x80 ); // chunk type
	bufWrite.AppendByte( (uint8_t)bufWrite.GetLocalEndianMode() ); // endian
	bufWrite.AppendByte( 0 ); // place holder for chunk data size
	bufWrite.AppendByte( 0 );
	bufWrite.AppendByte( 0 );
	bufWrite.AppendByte( 0 );

	size_t startPrivatePosOfConvexHullChunk = buffer.size();

	//-------------------------------chunk data-------------------------------
	bufWrite.AppendUshort( (unsigned short)m_convexArray.size() ); // num of objects in the scene
	for (auto convex : m_convexArray) {
		std::vector<Plane2> const& planes = convex->m_convexHull.m_boundingPlanes;
		int numOfPlanes = (int)planes.size();
		bufWrite.AppendByte( (uint8_t)numOfPlanes );
		for (int i = 0; i < numOfPlanes; ++i) {
			bufWrite.AppendPlane2( planes[i] );
		}
	}
	//-------------------------------chunk data-------------------------------

	size_t endPrivatePosOfConvexHullChunk = buffer.size();
	bufWrite.WriteIntToPos( startPrivatePosOfConvexHullChunk - offsetFromStartPrivateToDataSize, unsigned int( endPrivatePosOfConvexHullChunk - startPrivatePosOfConvexHullChunk ) );

	bufWrite.AppendChar( 'E' );
	bufWrite.AppendChar( 'N' );
	bufWrite.AppendChar( 'D' );
	bufWrite.AppendChar( 'C' );

	// table of contents
	constexpr int startPrivateToHeader = -10;
	constexpr unsigned int headerEndingSize = 14;
	bufWrite.WriteIntToPos( 8, unsigned int(buffer.size()) );
	bufWrite.AppendChar( 'G' ); // header
	bufWrite.AppendChar( 'H' );
	bufWrite.AppendChar( 'T' );
	bufWrite.AppendChar( 'C' );

	bufWrite.AppendByte( 4 ); // num of chunks in this file

	bufWrite.AppendByte( 0x01 ); // chunk type
	bufWrite.AppendUint32( unsigned int((int)startPrivatePosOfSceneInfoChunk + startPrivateToHeader) ); // chunk start pos
	bufWrite.AppendUint32( unsigned int(endPrivatePosOfSceneInfoChunk - startPrivatePosOfSceneInfoChunk) + headerEndingSize ); // chunk size including header and footer

	bufWrite.AppendByte( 0x02 ); // chunk type
	bufWrite.AppendUint32( unsigned int((int)startPrivatePosOfObjectChunk + startPrivateToHeader) ); // chunk start pos
	bufWrite.AppendUint32( unsigned int(endPrivatePosOfObjectChunk - startPrivatePosOfObjectChunk) + headerEndingSize ); // chunk size including header and footer
	
	bufWrite.AppendByte( 0x81 ); // chunk type
	bufWrite.AppendUint32( unsigned int( (int)startPrivatePosOfBoundingDiscsChunk + startPrivateToHeader ) ); // chunk start pos
	bufWrite.AppendUint32( unsigned int( endPrivatePosOfBoundingDiscsChunk - startPrivatePosOfBoundingDiscsChunk ) + headerEndingSize ); // chunk size including header and footer

	bufWrite.AppendByte( 0x80 ); // chunk type
	bufWrite.AppendUint32( unsigned int( (int)startPrivatePosOfConvexHullChunk + startPrivateToHeader ) ); // chunk start pos
	bufWrite.AppendUint32( unsigned int( endPrivatePosOfConvexHullChunk - startPrivatePosOfConvexHullChunk ) + headerEndingSize ); // chunk size including header and footer

	bufWrite.AppendChar( 'E' );
	bufWrite.AppendChar( 'N' );
	bufWrite.AppendChar( 'D' );
	bufWrite.AppendChar( 'T' );

	BufferWriteToFile( buffer, path );
}

void Game::LoadConvexSceneToCurrent( std::string const& path )
{
	std::vector<Convex2*> tempConvexArray;

	std::vector<uint8_t> buffer;
	FileReadToBuffer( buffer, path );

	BufferReader bufRead = BufferReader( buffer );

	// do checks
	if (bufRead.ParseChar() != 'G') {
		ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}
	if (bufRead.ParseChar() != 'H') {
		ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}
	if (bufRead.ParseChar() != 'C') {
		ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}
	if (bufRead.ParseChar() != 'S') {
		ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}
	if (bufRead.ParseByte() != 33) {
		ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}
	if (bufRead.ParseByte() != 1) {
		ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}
	if (bufRead.ParseByte() != 1) {
		ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}
	// set endian mode
	bufRead.SetBufferEndianMode( (EndianMode)bufRead.ParseByte() );
	// jump to table of contents
	size_t tocLocation = (size_t)bufRead.ParseUint32();
	bufRead.SetCurReadPosition( tocLocation );
	if (bufRead.ParseChar() != 'G') {
		ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}
	if (bufRead.ParseChar() != 'H') {
		ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}
	if (bufRead.ParseChar() != 'T') {
		ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}
	if (bufRead.ParseChar() != 'C') {
		ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}

	std::vector<std::pair<uint8_t, std::pair<uint32_t, uint32_t>>>  tableOfContents;
	int numOfChunks = (int)bufRead.ParseByte();
	// parse table of contents
	for (int i = 0; i < numOfChunks; ++i) {
		uint8_t sizeType = bufRead.ParseByte();
		uint32_t chunkStartPos = bufRead.ParseUint32();
		uint32_t chunkSize = bufRead.ParseUint32();
		tableOfContents.push_back( std::pair<uint8_t, std::pair<uint32_t, uint32_t>>( sizeType, std::pair<uint32_t, uint32_t>( chunkStartPos, chunkSize ) ) );
	}

	if (bufRead.ParseChar() != 'E') {
		ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}
	if (bufRead.ParseChar() != 'N') {
		ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}
	if (bufRead.ParseChar() != 'D') {
		ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}
	if (bufRead.ParseChar() != 'T') {
		ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}

	uint16_t recordedNumOfPolygonsInSceneInfo = (uint16_t)(-1);
	bool hasSceneInfo = false;
	bool hasObjectInfo = false;
	bool hasBoundingBoxInfo = false;
	bool hasBVHAABB2TreeInfo = false;
	bool hasSymmetricTreeInfo = false;
	for (auto& pair : tableOfContents) {
		bufRead.SetCurReadPosition( pair.second.first );
		size_t chunkRecordedSize = (size_t)pair.second.second;
		uint8_t chunkType = pair.first;

		size_t chunkStartPos = bufRead.GetCurReadPosition();
		// start of chunk
		if (bufRead.ParseChar() != 'G') {
			ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
			return;
		}
		if (bufRead.ParseChar() != 'H') {
			ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
			return;
		}
		if (bufRead.ParseChar() != 'C') {
			ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
			return;
		}
		if (bufRead.ParseChar() != 'K') {
			ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
			return;
		}
		if (bufRead.ParseByte() != chunkType) { // check chunk type
			g_devConsole->AddLine( DevConsole::INFO_ERROR, "Chunk type in chunk header is not same as chunk type in table of contents" );
			return;
		}
		// set endian mode
		bufRead.SetBufferEndianMode( (EndianMode)bufRead.ParseByte() );
		uint32_t sizeOfPrivateChunk = bufRead.ParseUint32();
		size_t startPos = bufRead.GetCurReadPosition();

		// private data
		if (chunkType == 0x01) {
			hasSceneInfo = true;
			m_worldCamera.m_cameraBox = bufRead.ParseAABB2();
			recordedNumOfPolygonsInSceneInfo = bufRead.ParseUshort();
		}
		else if (chunkType == 0x02) { // convex polys
			hasObjectInfo = true;
			uint16_t numOfObjects = bufRead.ParseUshort();
			if (recordedNumOfPolygonsInSceneInfo != (uint16_t)(-1) && recordedNumOfPolygonsInSceneInfo != numOfObjects) {
				g_devConsole->AddLine( DevConsole::INFO_ERROR, "Num of objects in chunk data is not same as Num of objects in scene info" );
				return;
			}
			for (int i = 0; i < (int)numOfObjects; ++i) {
				uint8_t numOfVerts = bufRead.ParseByte();
				std::vector<Vec2> verts;
				for (int j = 0; j < (int)numOfVerts; ++j) {
					verts.push_back( bufRead.ParseVec2() );
				}
				Convex2* newConvex = new Convex2();
				newConvex->m_convexPoly = ConvexPoly2( verts );
				tempConvexArray.push_back( newConvex );
			}
		}
		else if (chunkType == 0x81) { // bounding discs
			uint16_t numOfObjects = bufRead.ParseUshort();
			if (recordedNumOfPolygonsInSceneInfo != (uint16_t)(-1) && recordedNumOfPolygonsInSceneInfo != numOfObjects) {
				g_devConsole->AddLine( DevConsole::INFO_ERROR, "Num of objects in chunk data is not same as Num of objects in scene info" );
				return;
			}
			for (int i = 0; i < (int)numOfObjects; ++i) {
				tempConvexArray[i]->m_boundingDiscCenter = bufRead.ParseVec2();
				tempConvexArray[i]->m_boundingRadius = bufRead.ParseFloat();
			}
		}
		else if (chunkType == 0x80) { // convex hulls
			uint16_t numOfObjects = bufRead.ParseUshort();
			if (recordedNumOfPolygonsInSceneInfo != (uint16_t)(-1) && recordedNumOfPolygonsInSceneInfo != numOfObjects) {
				g_devConsole->AddLine( DevConsole::INFO_ERROR, "Num of objects in chunk data is not same as Num of objects in scene info" );
				return;
			}
			for (int i = 0; i < (int)numOfObjects; ++i) {
				uint8_t numOfPlanes = bufRead.ParseByte();
				std::vector<Plane2> planes;
				for (int j = 0; j < (int)numOfPlanes; ++j) {
					planes.push_back( bufRead.ParsePlane2() );
				}
				tempConvexArray[i]->m_convexHull = ConvexHull2( planes );
			}
		}
		else {
			continue;
		}
		size_t endPos = bufRead.GetCurReadPosition();
		if (endPos - startPos != sizeOfPrivateChunk) {
			g_devConsole->AddLine( DevConsole::INFO_ERROR, "Chunk size in chunk header is not same as current reading size" );
			return;
		}

		// end of chunk
		if (bufRead.ParseChar() != 'E') {
			ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
			return;
		}
		if (bufRead.ParseChar() != 'N') {
			ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
			return;
		}
		if (bufRead.ParseChar() != 'D') {
			ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
			return;
		}
		if (bufRead.ParseChar() != 'C') {
			ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
			return;
		}

		size_t chunkEndPos = bufRead.GetCurReadPosition();

		if (chunkEndPos - chunkStartPos != chunkRecordedSize) {
			g_devConsole->AddLine( DevConsole::INFO_ERROR, "Chunk size in table of contents is not same as current reading size" );
			//ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
			return;
		}
	}
	if (!hasSceneInfo) {
		g_devConsole->AddLine( DevConsole::INFO_ERROR, "These is no scene info in the save file" );
		//ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}
	if (!hasObjectInfo) {
		g_devConsole->AddLine( DevConsole::INFO_ERROR, "These is no object info in the save file" );
		//ReadSaveFileError( bufRead.GetCurReadPosition() - 1 );
		return;
	}
	ClearScene();
	m_convexArray = tempConvexArray;
	for (auto convex : m_convexArray) {
		if ((int)convex->m_convexHull.m_boundingPlanes.size() == 0) {
			convex->m_convexHull = ConvexHull2( convex->m_convexPoly );
		}
		if (!hasBoundingBoxInfo) {
			convex->RebuildBoundingBox();
		}
	}
	if (!hasBVHAABB2TreeInfo) {
		RebuildAABB2Tree();
	}
	if (!hasSymmetricTreeInfo) {
		RebuildSymmetricQuadTree();
	}
}

void Game::ReadSaveFileError( size_t location )
{
	g_devConsole->AddLine( DevConsole::INFO_ERROR, Stringf( "Error in save file location %d", location ) );
}

bool Game::SaveConvexSceneCommand( EventArgs& args )
{
	g_theGame->SaveCurrentConvexScene( std::string( "Data/Scenes/" ) + args.GetValue( "name", "default" ) );
	return true;
}

bool Game::LoadConvexSceneCommand( EventArgs& args )
{
	g_theGame->LoadConvexSceneToCurrent( std::string( "Data/Scenes/" ) + args.GetValue( "name", "default" ) );
	return true;
}

void Game::ClearScene()
{
	for (int i = 0; i < (int)m_convexArray.size(); ++i) {
		delete m_convexArray[i];
	}
	m_convexArray.clear();
	m_hoveringConvex = nullptr;
}
