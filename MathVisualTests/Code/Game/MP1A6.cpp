#include "Game/MP1A6.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"

MP1A6::MP1A6()
{
}

MP1A6::~MP1A6()
{

}

void MP1A6::StartUp()
{
	m_camera2D.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 0.f, 1.f );
	m_camera2D.m_mode = CameraMode::Orthographic;
	g_theInput->SetCursorMode( false, false );
	m_rayStartPoint.x = m_randNumGen->RollRandomFloatInRange( 50.f, 750.f );
	m_rayStartPoint.y = m_randNumGen->RollRandomFloatInRange( 25.f, 375.f );
	m_rayEndPoint.x = m_randNumGen->RollRandomFloatInRange( 50.f, 750.f );
	m_rayEndPoint.y = m_randNumGen->RollRandomFloatInRange( 25.f, 375.f );

	discs.reserve( 12 );
	RandomizeTest();
}

void MP1A6::RandomizeTest()
{
	discs.clear();
	for (int i = 0; i < 12; i++) {
		RayCastDiscInfo dInfo;
		dInfo.m_center.x = m_randNumGen->RollRandomFloatInRange( 50.f + 59 * i, 50.f + 59 * (i + 1) );
		if (i % 2) {
			dInfo.m_center.y = m_randNumGen->RollRandomFloatInRange( 25.f, 225.f );
		}
		else {
			dInfo.m_center.y = m_randNumGen->RollRandomFloatInRange( 225.f, 375.f );
		}
		dInfo.m_radius = m_randNumGen->RollRandomFloatInRange( 10.f, 70.f );
		discs.push_back( dInfo );
	}
}

void MP1A6::Update( float deltaSeconds )
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
	m_thisFrameRes.m_impactDist = 99999999999.f;
	for (auto& i : discs) {
		RayCastResult2D thisRes = RayCastResult2D();
		if (RayCastVsDisc2D( thisRes, m_rayStartPoint, m_rayEndPoint, i.m_center, i.m_radius )) {
			if (thisRes.m_impactDist < m_thisFrameRes.m_impactDist) {
				m_thisFrameRes = thisRes;
				m_thisFrameHitCenter = i.m_center;
			}
		}
	}
}

void MP1A6::Render() const
{
	g_theRenderer->BeginCamera( m_camera2D );
	std::vector<Vertex_PCU> verts;
	verts.reserve( 1500 );
	
	if (m_thisFrameRes.m_didImpact) {
		for (auto& i : discs) {
			if (i.m_center == m_thisFrameHitCenter) {
				AddVertsForDisc2D( verts, i.m_center, i.m_radius, Rgba8( 137, 164, 255, 255 ) );
			}
			else {
				AddVertsForDisc2D( verts, i.m_center, i.m_radius, Rgba8( 93, 126, 235, 255 ) );
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
		for (auto& i : discs) {
			AddVertsForDisc2D( verts, i.m_center, i.m_radius, Rgba8( 93, 126, 235, 255 ) );
		}
		AddVertsForArrow2D( verts, m_rayStartPoint, m_rayEndPoint, 10.f, 1.f, Rgba8( 0, 255, 0, 255 ) );
	}


	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );

	std::vector<Vertex_PCU> textVerts;
	textVerts.reserve( 1000 );
	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 5.f, 385.f ), 10.f, "Mode (F6/F7 for prev/next):  Ray Cast vs. Discs (2D)", Rgba8( 255, 255, 0 ), 0.6f );
	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 5.f, 370.f ), 10.f, "F8 to randomize; LMB/RMB set ray start/end; WASD move start, IJKL move end, arrows move ray, hold T = slow", Rgba8( 0, 204, 204 ), 0.6f );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->DrawVertexArray( textVerts );

	g_theRenderer->EndCamera( m_camera2D );
}

