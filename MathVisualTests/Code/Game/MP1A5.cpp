#include "Game/MP1A5.hpp"
#include "Game/GameCommon.hpp"
MP1A5::MP1A5()
{
}

MP1A5::~MP1A5()
{

}

void MP1A5::StartUp()
{
	m_camera2D.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 0.f, 1.f );
	m_camera2D.m_mode = CameraMode::Orthographic;
	g_theInput->SetCursorMode( false, false );
	//Disc, AABB2, OBB2, Capsule2, LineSegment2, “infinite” line
	
	// test point
	m_testPoint = Vec2( m_randNumGen->RollRandomFloatInRange( 0, WORLD_SIZE_X ), m_randNumGen->RollRandomFloatInRange( 0, WORLD_SIZE_Y ) );
	
	RandomizeTest();
}

void MP1A5::RandomizeTest()
{
	// disc
	m_discRadius = m_randNumGen->RollRandomFloatInRange( 5.f, WORLD_SIZE_Y * 0.2f );
	m_discCenter = Vec2( m_randNumGen->RollRandomFloatInRange( m_discRadius, WORLD_SIZE_X - m_discRadius ), m_randNumGen->RollRandomFloatInRange( m_discRadius, WORLD_SIZE_Y - m_discRadius ) );

	// aabb
	m_aabbBox.m_mins = Vec2( m_randNumGen->RollRandomFloatInRange( 100.f, WORLD_SIZE_X - 100.f ), m_randNumGen->RollRandomFloatInRange( 100.f, WORLD_SIZE_Y - 100.f ) );
	m_aabbBox.m_maxs = m_aabbBox.m_mins + Vec2( m_randNumGen->RollRandomFloatInRange( 10.f, 100.f ), m_randNumGen->RollRandomFloatInRange( 10.f, 100.f ) );

	// obb
	m_obbBox.m_halfDimensions = Vec2( m_randNumGen->RollRandomFloatInRange( 10.f, 100.f ), m_randNumGen->RollRandomFloatInRange( 10.f, 100.f ) );
	m_obbBox.m_center = Vec2( m_randNumGen->RollRandomFloatInRange( 100.f, WORLD_SIZE_X - 100.f ), m_randNumGen->RollRandomFloatInRange( 100.f, WORLD_SIZE_Y - 100.f ) );
	m_obbBox.m_iBasisNormal = Vec2::MakeFromPolarDegrees( m_randNumGen->RollRandomFloatInRange( 0.f, 360.f ) );

	// capsule
	m_capsuleBoneStartPoint = Vec2( m_randNumGen->RollRandomFloatInRange( 100.f, WORLD_SIZE_X - 100.f ), m_randNumGen->RollRandomFloatInRange( 100.f, WORLD_SIZE_Y - 100.f ) );
	m_capsuleBoneEndPoint = m_capsuleBoneStartPoint - Vec2( m_randNumGen->RollRandomFloatInRange( 10.f, 100.f ), m_randNumGen->RollRandomFloatInRange( 10.f, 100.f ) );
	m_capsuleRadius = m_randNumGen->RollRandomFloatInRange( 5.f, WORLD_SIZE_Y * 0.2f );

	// line segment
	m_lineSegmentStartPoint = Vec2( m_randNumGen->RollRandomFloatInRange( 50.f, WORLD_SIZE_X - 50.f ), m_randNumGen->RollRandomFloatInRange( 100.f, WORLD_SIZE_Y - 100.f ) );
	m_lineSegmentEndPoint = Vec2( m_randNumGen->RollRandomFloatInRange( 50.f, WORLD_SIZE_X - 50.f ), m_randNumGen->RollRandomFloatInRange( 100.f, WORLD_SIZE_Y - 100.f ) );

	// infinite line
	m_infiniteLineStartPoint = Vec2( m_randNumGen->RollRandomFloatInRange( 0, WORLD_SIZE_X ), m_randNumGen->RollRandomFloatInRange( 0, WORLD_SIZE_Y ) );
	m_infiniteLineEndPoint = Vec2( m_randNumGen->RollRandomFloatInRange( 0, WORLD_SIZE_X ), m_randNumGen->RollRandomFloatInRange( 0, WORLD_SIZE_Y ) );
	m_infiniteLineStartPoint -= (m_infiniteLineEndPoint - m_infiniteLineStartPoint).GetNormalized() * 1600.f;
	m_infiniteLineEndPoint += (m_infiniteLineEndPoint - m_infiniteLineStartPoint).GetNormalized() * 1600.f;

	// sector
	m_sectorCenter = Vec2( m_randNumGen->RollRandomFloatInRange( 50.f, WORLD_SIZE_X - 50.f ), m_randNumGen->RollRandomFloatInRange( 100.f, WORLD_SIZE_Y - 100.f ) );
	m_sectorForwardDegrees = m_randNumGen->RollRandomFloatInRange( 0.f, 360.f );
	m_sectorApertureDegrees = m_randNumGen->RollRandomFloatInRange( 0.f, 360.f );
	m_sectorRadius = m_randNumGen->RollRandomFloatInRange( 5.f, WORLD_SIZE_Y * 0.2f );
}

void MP1A5::Update( float deltaSeconds )
{
	if (g_theInput->IsKeyDown( 'W' )) {
		m_testPoint.y += m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'A' )) {
		m_testPoint.x -= m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'S' )) {
		m_testPoint.y -= m_moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'D' )) {
		m_testPoint.x += m_moveSpeed * deltaSeconds;
	}
}

void MP1A5::Render() const
{
	g_theRenderer->BeginCamera( m_camera2D );
	std::vector<Vertex_PCU> verts;
	verts.reserve( 1200 );
	Vec2 nearestPoint;

	// disc
	if (IsPointInsideDisc2D( m_testPoint, m_discCenter, m_discRadius )) {
		AddVertsForDisc2D( verts, m_discCenter, m_discRadius, Rgba8( 137, 164, 255, 255 ) );
	}
	else {
		AddVertsForDisc2D( verts, m_discCenter, m_discRadius, Rgba8( 93, 126, 235, 255 ) );
	}
	nearestPoint = GetNearestPointOnDisc2D( m_testPoint, m_discCenter, m_discRadius );
	AddVertsForDisc2D( verts, nearestPoint, 3.f, Rgba8( 255, 255, 0, 255 ) );
	AddVertsForLineSegment2D( verts, m_testPoint, nearestPoint, 1.f, Rgba8( 255, 255, 255, 30 ) );

	// aabb
	if (IsPointInsideAABB2D( m_testPoint, m_aabbBox )) {
		AddVertsForAABB2D( verts, m_aabbBox, Rgba8( 137, 164, 255, 255 ) );
	}
	else {
		AddVertsForAABB2D( verts, m_aabbBox, Rgba8( 93, 126, 235, 255 ) );
	}
	nearestPoint = GetNearestPointOnAABB2D( m_testPoint, m_aabbBox );
	AddVertsForDisc2D( verts, nearestPoint, 3.f, Rgba8( 255, 255, 0, 255 ) );
	AddVertsForLineSegment2D( verts, m_testPoint, nearestPoint, 1.f, Rgba8( 255, 255, 255, 30 ) );
	
	// obb
	if (IsPointInsideOBB2D( m_testPoint, m_obbBox )) {
		AddVertsForOBB2D( verts, m_obbBox, Rgba8( 137, 164, 255, 255 ) );
	}
	else {
		AddVertsForOBB2D( verts, m_obbBox, Rgba8( 93, 126, 235, 255 ) );
	}
	nearestPoint = GetNearestPointOnOBB2D( m_testPoint, m_obbBox );
	AddVertsForDisc2D( verts, nearestPoint, 3.f, Rgba8( 255, 255, 0, 255 ) );
	AddVertsForLineSegment2D( verts, m_testPoint, nearestPoint, 1.f, Rgba8( 255, 255, 255, 30 ) );
	
	// capsule
	if (IsPointInsideCapsule2D( m_testPoint, m_capsuleBoneStartPoint, m_capsuleBoneEndPoint, m_capsuleRadius )) {
		AddVertsForCapsule2D( verts, m_capsuleBoneStartPoint, m_capsuleBoneEndPoint, m_capsuleRadius, Rgba8( 137, 164, 255, 255 ) );
	}
	else {
		AddVertsForCapsule2D( verts, m_capsuleBoneStartPoint, m_capsuleBoneEndPoint, m_capsuleRadius, Rgba8( 93, 126, 235, 255 ) );
	}
	nearestPoint = GetNearestPointOnCapsule2D( m_testPoint, m_capsuleBoneStartPoint, m_capsuleBoneEndPoint, m_capsuleRadius );
	AddVertsForDisc2D( verts, nearestPoint, 3.f, Rgba8( 255, 255, 0, 255 ) );
	AddVertsForLineSegment2D( verts, m_testPoint, nearestPoint, 1.f, Rgba8( 255, 255, 255, 30 ) );
	
	// line segment
	AddVertsForLineSegment2D( verts, m_lineSegmentStartPoint, m_lineSegmentEndPoint, 2.f, Rgba8( 93, 126, 235, 255 ) );
	nearestPoint = GetNearestPointOnLineSegment2D( m_testPoint, m_lineSegmentStartPoint, m_lineSegmentEndPoint );
	AddVertsForDisc2D( verts, nearestPoint, 3.f, Rgba8( 255, 255, 0, 255 ) );
	AddVertsForLineSegment2D( verts, m_testPoint, nearestPoint, 1.f, Rgba8( 255, 255, 255, 30 ) );
	
	// infinite line	
	AddVertsForLineSegment2D( verts, m_infiniteLineStartPoint, m_infiniteLineEndPoint, 2.f, Rgba8( 93, 126, 235, 255 ) );
	nearestPoint = GetNearestPointOnLineSegment2D( m_testPoint, m_infiniteLineStartPoint, m_infiniteLineEndPoint );
	AddVertsForDisc2D( verts, nearestPoint, 3.f, Rgba8( 255, 255, 0, 255 ) );
	AddVertsForLineSegment2D( verts, m_testPoint, nearestPoint, 1.f, Rgba8( 255, 255, 255, 30 ) );

	// sector
	if (IsPointInsideOrientedSector2D( m_testPoint, m_sectorCenter, m_sectorForwardDegrees, m_sectorApertureDegrees, m_sectorRadius )) {
		AddVertsForSector2D( verts, m_sectorCenter, m_sectorForwardDegrees, m_sectorApertureDegrees, m_sectorRadius, Rgba8( 137, 164, 255, 255 ) );
	}
	else {
		AddVertsForSector2D( verts, m_sectorCenter, m_sectorForwardDegrees, m_sectorApertureDegrees, m_sectorRadius, Rgba8( 93, 126, 235, 255 ) );
	}
	nearestPoint = GetNearestPointOnOrientedSector2D( m_testPoint, m_sectorCenter, m_sectorForwardDegrees, m_sectorApertureDegrees, m_sectorRadius );
	AddVertsForDisc2D( verts, nearestPoint, 3.f, Rgba8( 255, 255, 0, 255 ) );
	AddVertsForLineSegment2D( verts, m_testPoint, nearestPoint, 1.f, Rgba8( 255, 255, 255, 30 ) );

	// test point
	AddVertsForDisc2D( verts, m_testPoint, 2.f, Rgba8( 255, 255, 255, 255 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( (int)verts.size(), verts.data() );
	g_theRenderer->EndCamera( m_camera2D );
}

