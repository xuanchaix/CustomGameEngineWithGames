#include "Game/MP2A5.hpp"
#include "Game/GameCommon.hpp"

AABB2 const g_bezierBounds = AABB2( Vec2( 330.f, 170.f ), Vec2( 770.f, 370.f ) );
AABB2 const g_easingBounds = AABB2( Vec2( 30.f, 170.f ), Vec2( 230.f, 370.f ) );
AABB2 const g_splineBounds = AABB2( Vec2( 30.f, 15.f ), Vec2( 770.f, 160.f ) );

float FunkyEasingFunction( float t ) {
	float s = Hesitate5( 0.8f * sinf( PI * 0.5f * t ) - 0.2f * sinf( PI * t ) + 0.2f * sinf( 3.f * PI * t ) - 0.2f * sinf( 1.5f * t * PI ) );
	return s < 0.f ? -s : s > 1.f ? 2.f - s : s;
}

CurveTest2D::CurveTest2D()
{
	m_easingFuncs[0] = []( float t ) { return t; };
	m_easingFuncs[1] = SmoothStart2;
	m_easingFuncs[2] = SmoothStart3;
	m_easingFuncs[3] = SmoothStart4;
	m_easingFuncs[4] = SmoothStart5;
	m_easingFuncs[5] = SmoothStart6;
	m_easingFuncs[6] = SmoothStop2;
	m_easingFuncs[7] = SmoothStop3;
	m_easingFuncs[8] = SmoothStop4;
	m_easingFuncs[9] = SmoothStop5;
	m_easingFuncs[10] = SmoothStop6;
	m_easingFuncs[11] = SmoothStep3;
	m_easingFuncs[12] = SmoothStep5;
	m_easingFuncs[13] = Hesitate3;
	m_easingFuncs[14] = Hesitate5;
	m_easingFuncs[15] = FunkyEasingFunction;
	m_easingNames[0] = "Identity";
	m_easingNames[1] = "SmoothStart2";
	m_easingNames[2] = "SmoothStart3";
	m_easingNames[3] = "SmoothStart4";
	m_easingNames[4] = "SmoothStart5";
	m_easingNames[5] = "SmoothStart6";
	m_easingNames[6] = "SmoothStop2";
	m_easingNames[7] = "SmoothStop3";
	m_easingNames[8] = "SmoothStop4";
	m_easingNames[9] = "SmoothStop5";
	m_easingNames[10] = "SmoothStop6";
	m_easingNames[11] = "SmoothStep3";
	m_easingNames[12] = "SmoothStep5";
	m_easingNames[13] = "Hesitate3";
	m_easingNames[14] = "Hesitate5";
	m_easingNames[15] = "Funky";
}

CurveTest2D::~CurveTest2D()
{
	delete m_bezierCurve;
	m_bezierCurve = nullptr;
	delete m_spline;
	m_spline = nullptr;
}

void CurveTest2D::StartUp()
{
	m_camera2D.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 0.f, 1.f );
	m_camera2D.m_mode = CameraMode::Orthographic;
	g_theInput->SetCursorMode( false, false );
	m_time = 0.f;
	m_bezierCurve = new CubicBezierCurve2D( Vec2(), Vec2(), Vec2(), Vec2() );
	m_spline = new CatmullRomSpline2D(std::vector<Vec2>());
	RandomizeTest();
}

void CurveTest2D::RandomizeTest()
{
	m_time = 0.f;
	m_curEasingCurve = m_randNumGen->RollRandomIntInRange( 0, 15 );

	m_bezierCurve->m_startPos = Vec2( m_randNumGen->RollRandomFloatInRange( g_bezierBounds.m_mins.x, g_bezierBounds.m_maxs.x ), m_randNumGen->RollRandomFloatInRange( g_bezierBounds.m_mins.y, g_bezierBounds.m_maxs.y ) );
	m_bezierCurve->m_guidePos1 = Vec2( m_randNumGen->RollRandomFloatInRange( g_bezierBounds.m_mins.x, g_bezierBounds.m_maxs.x ), m_randNumGen->RollRandomFloatInRange( g_bezierBounds.m_mins.y, g_bezierBounds.m_maxs.y ) );
	m_bezierCurve->m_guidePos2 = Vec2( m_randNumGen->RollRandomFloatInRange( g_bezierBounds.m_mins.x, g_bezierBounds.m_maxs.x ), m_randNumGen->RollRandomFloatInRange( g_bezierBounds.m_mins.y, g_bezierBounds.m_maxs.y ) );
	m_bezierCurve->m_endPos = Vec2( m_randNumGen->RollRandomFloatInRange( g_bezierBounds.m_mins.x, g_bezierBounds.m_maxs.x ), m_randNumGen->RollRandomFloatInRange( g_bezierBounds.m_mins.y, g_bezierBounds.m_maxs.y ) );

	int numOfPoints = m_randNumGen->RollRandomIntInRange( 4, 9 );
	//int numOfPoints = 1;
	std::vector<Vec2> points;
	if (numOfPoints > 1) {
		float pivotIdentity = (g_splineBounds.m_maxs.x - g_splineBounds.m_mins.x) / (float)(numOfPoints + 1);
		for (int i = 0; i < numOfPoints; i++) {
			Vec2 point = Vec2( m_randNumGen->RollRandomFloatInRange( g_splineBounds.m_mins.x + pivotIdentity * (float)i, g_splineBounds.m_mins.x + pivotIdentity * (float)(i + 2) ),
				m_randNumGen->RollRandomFloatInRange( g_splineBounds.m_mins.y, g_splineBounds.m_maxs.y ) );
			points.push_back( point );
		}
	}
	else {
		Vec2 point = Vec2( m_randNumGen->RollRandomFloatInRange( g_splineBounds.m_mins.x, g_splineBounds.m_maxs.x ), m_randNumGen->RollRandomFloatInRange( g_splineBounds.m_mins.y, g_splineBounds.m_maxs.y ) );
		points.push_back( point );
	}
	if (m_spline->GetNumOfCurves() != 0) {
		m_splineTime = m_splineTime * (float)(numOfPoints - 1) / (float)m_spline->GetNumOfCurves();
		m_splineTime = (float)RoundDownToInt( m_splineTime ) + m_time;
	}
	m_spline->ResetAllPoints( points );

}

void CurveTest2D::Update( float deltaSeconds )
{
	m_time += deltaSeconds * 0.5f;
	if (m_time > 1.f) {
		m_time = 0.f;
	}
	m_splineTime += deltaSeconds * 0.5f;
	if (m_splineTime > (float)(m_spline->GetNumOfPoints() - 1)) {
		m_splineTime = 0.f;
	}
	if (g_theInput->WasKeyJustPressed( 'N' )) {
		if (m_subdivisions != 1) {
			m_subdivisions /= 2;
		}
	}
	if (g_theInput->WasKeyJustPressed( 'M' )) {
		m_subdivisions *= 2;
	}
	if (g_theInput->WasKeyJustPressed( 'E' )) {
		m_curEasingCurve = (m_curEasingCurve + 1) % 16;
	}
	if (g_theInput->WasKeyJustPressed( 'W' )) {
		m_curEasingCurve = (m_curEasingCurve - 1 + 16) % 16;
	}
	if (g_theInput->WasKeyJustPressed( KEYCODE_F1 )) {
		m_showBounds = !m_showBounds;
	}
}

void CurveTest2D::Render() const
{
	g_theRenderer->BeginCamera( m_camera2D );
	RenderEasingCurve( g_easingBounds );
	RenderBezierCurve( g_bezierBounds );
	RenderSpline( g_splineBounds );
	g_theRenderer->EndCamera( m_camera2D );
}

void CurveTest2D::RenderEasingCurve( AABB2 const& bounds ) const
{
	AABB2 box = AABB2( bounds.m_mins + Vec2( 30.f, 30.f ), bounds.m_maxs - Vec2( 30.f, 30.f ) );
	std::vector<Vertex_PCU> verts;
	verts.reserve( 1000 );
	if (m_showBounds) {
		AddVertsForAABB2D( verts, bounds, Rgba8( 200, 50, 50 ), AABB2::IDENTITY );
	}
	AddVertsForAABB2D( verts, box, Rgba8( 0, 50, 80 ), AABB2::IDENTITY );
	Vec2 dist = box.m_maxs - box.m_mins;
	for (int i = 0; i < 64; i++) {
		AddVertsForLineSegment2D( verts, box.m_mins + dist * Vec2( (float)i / (float)64, m_easingFuncs[m_curEasingCurve]( (float)i / (float)64 ) ), box.m_mins + dist * Vec2( (float)(i + 1) / (float)64, m_easingFuncs[m_curEasingCurve]( (float)(i + 1) / (float)64 ) ), 1.f, Rgba8( 128, 128, 128 ) );
	}
	for (int i = 0; i < m_subdivisions; i++) {
		AddVertsForLineSegment2D( verts, box.m_mins + dist * Vec2( (float)i / (float)m_subdivisions, m_easingFuncs[m_curEasingCurve]( (float)i / (float)m_subdivisions ) ), box.m_mins + dist * Vec2( (float)(i + 1) / (float)m_subdivisions, m_easingFuncs[m_curEasingCurve]( (float)(i + 1) / (float)m_subdivisions ) ), 1.f, Rgba8( 0, 255, 128 ) );
	}
	AddVertsForDisc2D( verts, box.m_mins + dist * Vec2( m_time, m_easingFuncs[m_curEasingCurve]( m_time ) ), 3.f, Rgba8::WHITE );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );

	verts.clear();
	g_ASCIIFont->AddVertsForTextInBox2D( verts, AABB2( Vec2( box.m_mins - Vec2( 0.f, 30.f ) ), Vec2( box.m_maxs.x, box.m_mins.y - 10.f ) ), 15.f, m_easingNames[m_curEasingCurve], Rgba8( 0, 255, 128 ) );
	g_ASCIIFont->AddVertsForText2D( verts, Vec2( 5.f, 385.f ), 10.f, "Mode (F6/F7 for prev/next):  Easing, Curves, Splines (2D)", Rgba8( 255, 255, 0 ), 0.6f );
	g_ASCIIFont->AddVertsForText2D( verts, Vec2( 5.f, 370.f ), 10.f, Stringf( "F8 to randomize; W/E = prev/next Easing function; N/M = curve subdivisions (%d), hold T = slow", m_subdivisions ), Rgba8( 0, 204, 204 ), 0.6f );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->DrawVertexArray( verts );
}

void CurveTest2D::RenderBezierCurve( AABB2 const& bounds ) const
{
	std::vector<Vertex_PCU> verts;
	verts.reserve( 1000 );
	if (m_showBounds) {
		AddVertsForAABB2D( verts, bounds, Rgba8( 200, 50, 50 ), AABB2::IDENTITY );
	}

	AddVertsForLineSegment2D( verts, m_bezierCurve->m_startPos, m_bezierCurve->m_guidePos1, 1.f, Rgba8( 0, 102, 204 ) );
	AddVertsForLineSegment2D( verts, m_bezierCurve->m_guidePos1, m_bezierCurve->m_guidePos2, 1.f, Rgba8( 0, 102, 204 ) );
	AddVertsForLineSegment2D( verts, m_bezierCurve->m_guidePos2, m_bezierCurve->m_endPos, 1.f, Rgba8( 0, 102, 204 ) );

	for (int i = 0; i < 64; i++) {
		float t = (float)i / 64.f;
		float nt = (float)(i + 1) / 64.f;
		AddVertsForLineSegment2D( verts, m_bezierCurve->EvaluateAtParametric( t ), m_bezierCurve->EvaluateAtParametric( nt ), 1.f, Rgba8( 128, 128, 128 ) );
	}
	for (int i = 0; i < m_subdivisions; i++) {
		float t = (float)i / m_subdivisions;
		float nt = (float)(i + 1) / m_subdivisions;
		AddVertsForLineSegment2D( verts, m_bezierCurve->EvaluateAtParametric( t ), m_bezierCurve->EvaluateAtParametric( nt ), 1.f, Rgba8( 0, 255, 128 ) );
	}

	AddVertsForDisc2D( verts, m_bezierCurve->m_startPos, 2.f, Rgba8( 102, 178, 255 ) );
	AddVertsForDisc2D( verts, m_bezierCurve->m_endPos, 2.f, Rgba8( 102, 178, 255 ) );
	AddVertsForDisc2D( verts, m_bezierCurve->m_guidePos1, 2.f, Rgba8( 102, 178, 255 ) );
	AddVertsForDisc2D( verts, m_bezierCurve->m_guidePos2, 2.f, Rgba8( 102, 178, 255 ) );

	float length = m_bezierCurve->GetApproximateLength( m_subdivisions );
	AddVertsForDisc2D( verts, m_bezierCurve->EvaluateAtApproximateDistance( m_time * length, m_subdivisions ), 3.f, Rgba8( 0, 255, 128 ) );
	AddVertsForDisc2D( verts, m_bezierCurve->EvaluateAtParametric( m_time ), 3.f, Rgba8::WHITE );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );
}

void CurveTest2D::RenderSpline( AABB2 const& bounds ) const
{

	std::vector<Vertex_PCU> verts;
	verts.reserve( 1000 );
	if (m_showBounds) {
		AddVertsForAABB2D( verts, bounds, Rgba8( 200, 50, 50 ), AABB2::IDENTITY );
	}

	// draw line segments between points
	int num = m_spline->GetNumOfPoints();
	for (int i = 0; i < num - 1; i++) {
		Vec2 point1 = m_spline->GetPointAtIndex( i );
		Vec2 point2 = m_spline->GetPointAtIndex( i + 1 );
		AddVertsForLineSegment2D( verts, point1, point2, 1.f, Rgba8( 0, 102, 204 ) );
	}

	//float curveIndex = (float)RoundDownToInt( m_splineTime );
	// draw the curve
	for (int k = 0; k < num - 1; k++) {
		CubicHermiteCurve2D const& curve = m_spline->GetCubicHermiteCurveAtIndex( k );
		for (int i = 0; i < 64; i++) {
			float t = (float)i / 64.f;
			float nt = (float)(i + 1) / 64.f;
			AddVertsForLineSegment2D( verts, curve.EvaluateAtParametric( t ), curve.EvaluateAtParametric( nt ), 1.f, Rgba8( 128, 128, 128 ) );
		}
		for (int i = 0; i < m_subdivisions; i++) {
			float t = (float)i / m_subdivisions;
			float nt = (float)(i + 1) / m_subdivisions;
			AddVertsForLineSegment2D( verts, curve.EvaluateAtParametric( t ), curve.EvaluateAtParametric( nt ), 1.f, Rgba8( 0, 255, 128 ) );
		}
	}

	// draw the velocity
	for (int i = 1; i < num - 1; i++) {
		Vec2 point = m_spline->GetPointAtIndex( i );
		Vec2 vel = m_spline->GetVelocityAtIndex( i );
		AddVertsForArrow2D( verts, point, point + vel, 3.f, 1.5f, Rgba8( 200, 0, 0 ) );
	}

	// draw the curve anchor points
	for (int i = 0; i < num; i++) {
		Vec2 point = m_spline->GetPointAtIndex( i );
		AddVertsForDisc2D( verts, point, 2.f, Rgba8( 102, 178, 255 ) );
	}

	// draw the running points
	float length = m_spline->GetApproximateLength( m_subdivisions );
	AddVertsForDisc2D( verts, m_spline->EvaluateAtApproximateDistance( m_splineTime / (float)(m_spline->GetNumOfCurves()) * length, m_subdivisions ), 3.f, Rgba8( 0, 255, 128 ) );
	AddVertsForDisc2D( verts, m_spline->EvaluateAtParametric( m_splineTime ), 3.f, Rgba8::WHITE );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );
}

