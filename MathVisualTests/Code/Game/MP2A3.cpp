#include "Game/MP2A3.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"

RayVsLineSeg2DTest::RayVsLineSeg2DTest()
{
}

RayVsLineSeg2DTest::~RayVsLineSeg2DTest()
{

}

void RayVsLineSeg2DTest::StartUp()
{
	m_camera2D.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 0.f, 1.f );
	m_camera2D.m_mode = CameraMode::Orthographic;
	g_theInput->SetCursorMode( false, false );
	m_rayStartPoint.x = m_randNumGen->RollRandomFloatInRange( 50.f, 750.f );
	m_rayStartPoint.y = m_randNumGen->RollRandomFloatInRange( 25.f, 375.f );
	m_rayEndPoint.x = m_randNumGen->RollRandomFloatInRange( 50.f, 750.f );
	m_rayEndPoint.y = m_randNumGen->RollRandomFloatInRange( 25.f, 375.f );

	m_lineSegs.reserve( 6 );
	RandomizeTest();
}

void RayVsLineSeg2DTest::RandomizeTest()
{
	m_lineSegs.clear();
	for (int i = 0; i < 6; i++) {
		RayCastLineInfo info;
		info.m_startPos.x = m_randNumGen->RollRandomFloatInRange( 20.f, 780.f );
		info.m_startPos.y = m_randNumGen->RollRandomFloatInRange( 10.f, 390.f );
		info.m_endPos.x = m_randNumGen->RollRandomFloatInRange( 20.f, 780.f );
		info.m_endPos.y = m_randNumGen->RollRandomFloatInRange( 10.f, 390.f );
		m_lineSegs.push_back( info );
	}
}

void RayVsLineSeg2DTest::Update( float deltaSeconds )
{
	if (g_theInput->IsKeyDown( 'W' )) {
		m_rayStartPoint.y += m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'A' )) {
		m_rayStartPoint.x -= m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'S' )) {
		m_rayStartPoint.y -= m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'D' )) {
		m_rayStartPoint.x += m_moveSpeed * deltaSeconds;
	}

	if (g_theInput->IsKeyDown( 'I' )) {
		m_rayEndPoint.y += m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'J' )) {
		m_rayEndPoint.x -= m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'K' )) {
		m_rayEndPoint.y -= m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'L' )) {
		m_rayEndPoint.x += m_moveSpeed * deltaSeconds;
	}

	if (g_theInput->IsKeyDown( 0x26 )) {
		m_rayStartPoint.y += m_moveSpeed * deltaSeconds;
		m_rayEndPoint.y += m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 0x25 )) {
		m_rayStartPoint.x -= m_moveSpeed * deltaSeconds;
		m_rayEndPoint.x -= m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 0x28 )) {
		m_rayStartPoint.y -= m_moveSpeed * deltaSeconds;
		m_rayEndPoint.y -= m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 0x27 )) {
		m_rayStartPoint.x += m_moveSpeed * deltaSeconds;
		m_rayEndPoint.x += m_moveSpeed * deltaSeconds;
	}

	if (g_theInput->IsKeyDown( KEYCODE_LEFTMOUSE )) {
		m_rayStartPoint = m_camera2D.GetCursorWorldPosition( g_window->GetNormalizedCursorPos() );
	}
	if (g_theInput->IsKeyDown( KEYCODE_RIGHTMOUSE )) {
		m_rayEndPoint = m_camera2D.GetCursorWorldPosition( g_window->GetNormalizedCursorPos() );
	}

	m_thisFrameRes = RayCastResult2D();
	m_thisFrameRes.m_impactDist = FLT_MAX;
	float rayMaxDist = (m_rayEndPoint - m_rayStartPoint).GetLength();
	Vec2 rayForwardNormal = (m_rayEndPoint - m_rayStartPoint) / rayMaxDist;
	for (auto& i : m_lineSegs) {
		RayCastResult2D thisRes = RayCastResult2D();
		if (RayCastVsLineSegment2D( thisRes, m_rayStartPoint, rayForwardNormal, rayMaxDist, i.m_startPos, i.m_endPos )) {
			if (thisRes.m_impactDist < m_thisFrameRes.m_impactDist) {
				m_thisFrameRes = thisRes;
				m_thisFrameHitInfo = &i;
			}
		}
	}
}

void RayVsLineSeg2DTest::Render() const
{
	g_theRenderer->BeginCamera( m_camera2D );
	std::vector<Vertex_PCU> verts;
	verts.reserve( 1500 );

	if (m_thisFrameRes.m_didImpact) {
		for (auto& i : m_lineSegs) {
			if (i.m_startPos == m_thisFrameHitInfo->m_startPos && i.m_endPos == m_thisFrameHitInfo->m_endPos) {
				AddVertsForLineSegment2D( verts, i.m_startPos, i.m_endPos, 1.f, Rgba8( 137, 164, 255, 255 ) );
			}
			else {
				AddVertsForLineSegment2D( verts, i.m_startPos, i.m_endPos, 1.f, Rgba8( 93, 126, 235, 255 ) );
			}
		}
		AddVertsForArrow2D( verts, m_rayStartPoint, m_rayEndPoint, 10.f, 1.f, Rgba8( 96, 96, 96, 255 ) );
		if (m_rayStartPoint != m_thisFrameRes.m_impactPos) {
			AddVertsForArrow2D( verts, m_rayStartPoint, m_thisFrameRes.m_impactPos, 10.f, 1.f, Rgba8( 255, 0, 0, 255 ) );
		}
		AddVertsForArrow2D( verts, m_thisFrameRes.m_impactPos, m_thisFrameRes.m_impactPos + m_thisFrameRes.m_impactNormal * 20.f, 5.f, 1.f, Rgba8( 255, 255, 0, 255 ) );
		AddVertsForDisc2D( verts, m_thisFrameRes.m_impactPos, 2.f, Rgba8( 255, 255, 255, 255 ) );
	}
	else {
		for (auto& i : m_lineSegs) {
			AddVertsForLineSegment2D( verts, i.m_startPos, i.m_endPos, 1.f, Rgba8( 93, 126, 235, 255 ) );
		}
		AddVertsForArrow2D( verts, m_rayStartPoint, m_rayEndPoint, 10.f, 1.f, Rgba8( 0, 255, 0, 255 ) );
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );

	std::vector<Vertex_PCU> textVerts;
	textVerts.reserve( 1000 );
	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 5.f, 385.f ), 10.f, "Mode (F6/F7 for prev/next):  Ray Cast vs. Line Segments (2D)", Rgba8( 255, 255, 0 ), 0.6f );
	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 5.f, 370.f ), 10.f, "F8 to randomize; LMB/RMB set ray start/end; WASD move start, IJKL move end, arrows move ray, hold T = slow", Rgba8( 0, 204, 204 ), 0.6f );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->DrawVertexArray( textVerts );
	g_theRenderer->EndCamera( m_camera2D );
}

RayVsAABB2DTest::RayVsAABB2DTest()
{

}

RayVsAABB2DTest::~RayVsAABB2DTest()
{

}

void RayVsAABB2DTest::StartUp()
{
	m_camera2D.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 0.f, 1.f );
	m_camera2D.m_mode = CameraMode::Orthographic;
	g_theInput->SetCursorMode( false, false );
	m_rayStartPoint.x = m_randNumGen->RollRandomFloatInRange( 50.f, 750.f );
	m_rayStartPoint.y = m_randNumGen->RollRandomFloatInRange( 25.f, 375.f );
	m_rayEndPoint.x = m_randNumGen->RollRandomFloatInRange( 50.f, 750.f );
	m_rayEndPoint.y = m_randNumGen->RollRandomFloatInRange( 25.f, 375.f );

	m_aabb2s.reserve( 8 );
	RandomizeTest();
}

void RayVsAABB2DTest::RandomizeTest()
{
	m_aabb2s.clear();
	for (int i = 0; i < 8; i++) {
		Vec2 mins, maxs;
		if (i % 2 == 0) {
			mins = Vec2( m_randNumGen->RollRandomFloatInRange( 90.f * (float)i + 10.f, 90.f * (float)(i + 1) + 5.f ), m_randNumGen->RollRandomFloatInRange( 10.f, 125.f ) );
			maxs = Vec2( m_randNumGen->RollRandomFloatInRange( 90.f * (float)(i + 1) + 10.f, 90.f * (float)(i + 2) + 80.f ), m_randNumGen->RollRandomFloatInRange( 135.f, 250.f ) );
		}
		else {
			mins = Vec2( m_randNumGen->RollRandomFloatInRange( 90.f * (float)(i - 1) + 10.f, 90.f * (float)i + 5.f ), m_randNumGen->RollRandomFloatInRange( 150.f, 215.f ) );
			maxs = Vec2( m_randNumGen->RollRandomFloatInRange( 90.f * (float)i + 10.f, 90.f * (float)(i + 1) + 80.f ), m_randNumGen->RollRandomFloatInRange( 225.f, 390.f ) );
		}
		m_aabb2s.push_back( AABB2( mins, maxs ) );
	}
}

void RayVsAABB2DTest::Update( float deltaSeconds )
{
	if (g_theInput->IsKeyDown( 'W' )) {
		m_rayStartPoint.y += m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'A' )) {
		m_rayStartPoint.x -= m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'S' )) {
		m_rayStartPoint.y -= m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'D' )) {
		m_rayStartPoint.x += m_moveSpeed * deltaSeconds;
	}

	if (g_theInput->IsKeyDown( 'I' )) {
		m_rayEndPoint.y += m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'J' )) {
		m_rayEndPoint.x -= m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'K' )) {
		m_rayEndPoint.y -= m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'L' )) {
		m_rayEndPoint.x += m_moveSpeed * deltaSeconds;
	}

	if (g_theInput->IsKeyDown( 0x26 )) {
		m_rayStartPoint.y += m_moveSpeed * deltaSeconds;
		m_rayEndPoint.y += m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 0x25 )) {
		m_rayStartPoint.x -= m_moveSpeed * deltaSeconds;
		m_rayEndPoint.x -= m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 0x28 )) {
		m_rayStartPoint.y -= m_moveSpeed * deltaSeconds;
		m_rayEndPoint.y -= m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 0x27 )) {
		m_rayStartPoint.x += m_moveSpeed * deltaSeconds;
		m_rayEndPoint.x += m_moveSpeed * deltaSeconds;
	}

	if (g_theInput->IsKeyDown( KEYCODE_LEFTMOUSE )) {
		m_rayStartPoint = m_camera2D.GetCursorWorldPosition( g_window->GetNormalizedCursorPos() );
	}
	if (g_theInput->IsKeyDown( KEYCODE_RIGHTMOUSE )) {
		m_rayEndPoint = m_camera2D.GetCursorWorldPosition( g_window->GetNormalizedCursorPos() );
	}

	if (g_theInput->IsKeyDown( 'X' )) {
		if (g_theInput->IsKeyDown( KEYCODE_RIGHTMOUSE )) {
			m_rayStartPoint.y = m_rayEndPoint.y;
		}
		else {
			m_rayEndPoint.y = m_rayStartPoint.y;
		}
	}
	else if (g_theInput->IsKeyDown( 'Y' )) {
		if (g_theInput->IsKeyDown( KEYCODE_RIGHTMOUSE )) {
			m_rayStartPoint.x = m_rayEndPoint.x;
		}
		else {
			m_rayEndPoint.x = m_rayStartPoint.x;
		}
	}

	m_thisFrameRes = RayCastResult2D();
	m_thisFrameRes.m_impactDist = FLT_MAX;
	float rayMaxDist = (m_rayEndPoint - m_rayStartPoint).GetLength();
	Vec2 rayForwardNormal = (m_rayEndPoint - m_rayStartPoint) / rayMaxDist;
	for (auto& i : m_aabb2s) {
		RayCastResult2D thisRes = RayCastResult2D();
		if (RayCastVsAABB2D( thisRes, m_rayStartPoint, rayForwardNormal, rayMaxDist, i )) {
			if (thisRes.m_impactDist < m_thisFrameRes.m_impactDist) {
				m_thisFrameRes = thisRes;
				m_thisFrameHitInfo = &i;
			}
		}
	}
}

void RayVsAABB2DTest::Render() const
{
	g_theRenderer->BeginCamera( m_camera2D );
	std::vector<Vertex_PCU> verts;
	verts.reserve( 1500 );

	if (m_thisFrameRes.m_didImpact) {
		for (auto& i : m_aabb2s) {
			if (i.m_mins == m_thisFrameHitInfo->m_mins && i.m_maxs == m_thisFrameHitInfo->m_maxs) {
				AddVertsForAABB2D( verts, i, Rgba8( 137, 164, 255, 255 ) );
			}
			else {
				AddVertsForAABB2D( verts, i, Rgba8( 93, 126, 235, 255 ) );
			}
		}
		AddVertsForArrow2D( verts, m_rayStartPoint, m_rayEndPoint, 10.f, 1.f, Rgba8( 96, 96, 96, 255 ) );
		if (m_rayStartPoint != m_thisFrameRes.m_impactPos) {
			AddVertsForArrow2D( verts, m_rayStartPoint, m_thisFrameRes.m_impactPos, 10.f, 1.f, Rgba8( 255, 0, 0, 255 ) );
		}
		AddVertsForArrow2D( verts, m_thisFrameRes.m_impactPos, m_thisFrameRes.m_impactPos + m_thisFrameRes.m_impactNormal * 20.f, 5.f, 1.f, Rgba8( 255, 255, 0, 255 ) );
		AddVertsForDisc2D( verts, m_thisFrameRes.m_impactPos, 2.f, Rgba8( 255, 255, 255, 255 ) );
	}
	else {
		for (auto& i : m_aabb2s) {
			AddVertsForAABB2D( verts, i, Rgba8( 93, 126, 235, 255 ) );
		}
		AddVertsForArrow2D( verts, m_rayStartPoint, m_rayEndPoint, 10.f, 1.f, Rgba8( 0, 255, 0, 255 ) );
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );

	std::vector<Vertex_PCU> textVerts;
	textVerts.reserve( 1000 );
	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 5.f, 385.f ), 10.f, "Mode (F6/F7 for prev/next):  Ray Cast vs. AABB (2D)", Rgba8( 255, 255, 0 ), 0.6f );
	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 5.f, 370.f ), 10.f, "F8 to randomize; LMB/RMB set ray start/end; WASD move start, IJKL move end, arrows move ray, hold T = slow", Rgba8( 0, 204, 204 ), 0.6f );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->DrawVertexArray( textVerts );
	g_theRenderer->EndCamera( m_camera2D );
}
