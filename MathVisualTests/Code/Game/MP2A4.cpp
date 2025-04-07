#include "Game/MP2A4.hpp"
#include "Game/GameCommon.hpp"

constexpr float rayMaxDist = 3.f;

RayVs3DTest::RayVs3DTest()
{
}

RayVs3DTest::~RayVs3DTest()
{
	for (auto shape : m_shapes) {
		delete shape;
	}
	m_shapes.clear();
}

void RayVs3DTest::StartUp()
{
	m_camera3D.SetPerspectiveView( g_window->GetAspect(), 60.f, 0.1f, 100.f );
	m_camera3D.SetRenderBasis( Vec3( 0.f, 0.f, 1.f ), Vec3( -1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ) );

	g_theInput->SetCursorMode( true, true );
	m_camera3D.m_mode = CameraMode::Perspective;
	m_camera3D.m_position = Vec3( -1.f, -1.f, 1.f );
	m_camera3D.m_orientation = EulerAngles( 45.f, 30.f, 0.f );

	m_camera2D.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 0.f, 1.f );


	RandomizeTest();

	DebugAddWorldArrow( Vec3( 0.f, 0.f, 0.f ), Vec3( 1.f, 0.f, 0.f ), 0.03f, -1.f, Rgba8( 255, 0, 0 ), Rgba8( 255, 0, 0 ), DebugRenderMode::USE_DEPTH );
	DebugAddWorldArrow( Vec3( 0.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), 0.03f, -1.f, Rgba8( 0, 255, 0 ), Rgba8( 0, 255, 0 ), DebugRenderMode::USE_DEPTH );
	DebugAddWorldArrow( Vec3( 0.f, 0.f, 0.f ), Vec3( 0.f, 0.f, 1.f ), 0.03f, -1.f, Rgba8( 0, 0, 255 ), Rgba8( 0, 0, 255 ), DebugRenderMode::USE_DEPTH );

}

void RayVs3DTest::RandomizeTest()
{
	for (auto shape : m_shapes) {
		delete shape;
	}
	m_shapes.clear();

	FloatRange positionRange( -3.f, 3.f );
	FloatRange radiusRange( 0.3f, 0.8f );

	TestShapeSphere* sphere1 = new TestShapeSphere();
	sphere1->m_center = Vec3( m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ) );
	sphere1->m_radius = m_randNumGen->RollRandomFloatInRange( radiusRange );
	sphere1->m_numOfLongitudeSlices = m_randNumGen->RollRandomIntInRange( 5, 24 );
	sphere1->m_numOfLatitudeSlices = m_randNumGen->RollRandomIntInRange( 5, 12 );
	m_shapes.push_back( sphere1 );

	TestShapeSphere* sphere2 = new TestShapeSphere();
	sphere2->m_center = Vec3( m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ) );
	sphere2->m_radius = m_randNumGen->RollRandomFloatInRange( radiusRange );
	sphere2->m_isWired = true;
	sphere2->m_numOfLongitudeSlices = m_randNumGen->RollRandomIntInRange( 5, 24 );
	sphere2->m_numOfLatitudeSlices = m_randNumGen->RollRandomIntInRange( 5, 12 );
	m_shapes.push_back( sphere2 );

	TestShapeSphere* sphere3 = new TestShapeSphere();
	sphere3->m_center = Vec3( m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ) );
	sphere3->m_radius = m_randNumGen->RollRandomFloatInRange( radiusRange );
	sphere3->m_isWired = true;
	sphere3->m_numOfLongitudeSlices = m_randNumGen->RollRandomIntInRange( 5, 24 );
	sphere3->m_numOfLatitudeSlices = m_randNumGen->RollRandomIntInRange( 5, 12 );
	m_shapes.push_back( sphere3 );

	TestShapeSphere* sphere4 = new TestShapeSphere();
	sphere4->m_center = Vec3( m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ) );
	sphere4->m_radius = m_randNumGen->RollRandomFloatInRange( radiusRange );
	sphere4->m_numOfLongitudeSlices = m_randNumGen->RollRandomIntInRange( 5, 24 );
	sphere4->m_numOfLatitudeSlices = m_randNumGen->RollRandomIntInRange( 5, 12 );
	m_shapes.push_back( sphere4 );

	TestShapeAABB3* aabb31 = new TestShapeAABB3();
	aabb31->m_box.m_mins = Vec3( m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ) );
	aabb31->m_box.m_maxs = aabb31->m_box.m_mins + Vec3( m_randNumGen->RollRandomFloatInRange( radiusRange ), m_randNumGen->RollRandomFloatInRange( radiusRange ), m_randNumGen->RollRandomFloatInRange( radiusRange ) );
	aabb31->m_isWired = false;
	m_shapes.push_back( aabb31 );

	TestShapeAABB3* aabb32 = new TestShapeAABB3();
	aabb32->m_box.m_mins = Vec3( m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ) );
	aabb32->m_box.m_maxs = aabb32->m_box.m_mins + Vec3( m_randNumGen->RollRandomFloatInRange( radiusRange ), m_randNumGen->RollRandomFloatInRange( radiusRange ), m_randNumGen->RollRandomFloatInRange( radiusRange ) );
	aabb32->m_isWired = true;
	m_shapes.push_back( aabb32 );

	TestShapeAABB3* aabb33 = new TestShapeAABB3();
	aabb33->m_box.m_mins = Vec3( m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ) );
	aabb33->m_box.m_maxs = aabb33->m_box.m_mins + Vec3( m_randNumGen->RollRandomFloatInRange( radiusRange ), m_randNumGen->RollRandomFloatInRange( radiusRange ), m_randNumGen->RollRandomFloatInRange( radiusRange ) );
	aabb33->m_isWired = true;
	m_shapes.push_back( aabb33 );

	TestShapeAABB3* aabb34 = new TestShapeAABB3();
	aabb34->m_box.m_mins = Vec3( m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ) );
	aabb34->m_box.m_maxs = aabb34->m_box.m_mins + Vec3( m_randNumGen->RollRandomFloatInRange( radiusRange ), m_randNumGen->RollRandomFloatInRange( radiusRange ), m_randNumGen->RollRandomFloatInRange( radiusRange ) );
	aabb34->m_isWired = false;
	m_shapes.push_back( aabb34 );

	TestShapeZCylinder* zcylinder1 = new TestShapeZCylinder();
	zcylinder1->m_center = Vec3( m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ) );
	zcylinder1->m_radius = m_randNumGen->RollRandomFloatInRange( radiusRange );
	zcylinder1->m_minZ = m_randNumGen->RollRandomFloatInRange( positionRange );
	zcylinder1->m_maxZ = zcylinder1->m_minZ + m_randNumGen->RollRandomFloatInRange( radiusRange );
	zcylinder1->m_isWired = false;
	zcylinder1->m_slices = m_randNumGen->RollRandomIntInRange( 5, 12 );
	m_shapes.push_back( zcylinder1 );

	TestShapeZCylinder* zcylinder2 = new TestShapeZCylinder();
	zcylinder2->m_center = Vec3( m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ) );
	zcylinder2->m_radius = m_randNumGen->RollRandomFloatInRange( radiusRange );
	zcylinder2->m_minZ = m_randNumGen->RollRandomFloatInRange( positionRange );
	zcylinder2->m_maxZ = zcylinder2->m_minZ + m_randNumGen->RollRandomFloatInRange( radiusRange );
	zcylinder2->m_isWired = true;
	zcylinder2->m_slices = m_randNumGen->RollRandomIntInRange( 5, 12 );
	m_shapes.push_back( zcylinder2 );

	TestShapeZCylinder* zcylinder3 = new TestShapeZCylinder();
	zcylinder3->m_center = Vec3( m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ) );
	zcylinder3->m_radius = m_randNumGen->RollRandomFloatInRange( radiusRange );
	zcylinder3->m_minZ = m_randNumGen->RollRandomFloatInRange( positionRange );
	zcylinder3->m_maxZ = zcylinder3->m_minZ + m_randNumGen->RollRandomFloatInRange( radiusRange );
	zcylinder3->m_isWired = true;
	zcylinder3->m_slices = m_randNumGen->RollRandomIntInRange( 5, 12 );
	m_shapes.push_back( zcylinder3 );

	TestShapeZCylinder* zcylinder4 = new TestShapeZCylinder();
	zcylinder4->m_center = Vec3( m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ) );
	zcylinder4->m_radius = m_randNumGen->RollRandomFloatInRange( radiusRange );
	zcylinder4->m_minZ = m_randNumGen->RollRandomFloatInRange( positionRange );
	zcylinder4->m_maxZ = zcylinder4->m_minZ + m_randNumGen->RollRandomFloatInRange( radiusRange );
	zcylinder4->m_isWired = false;
	zcylinder4->m_slices = m_randNumGen->RollRandomIntInRange( 5, 12 );
	m_shapes.push_back( zcylinder4 );

	TestShapeOBB3* obb31 = new TestShapeOBB3();
	obb31->m_obb3.m_center = Vec3( m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ) );
	obb31->m_obb3.m_halfDimensions = 0.5f * Vec3( m_randNumGen->RollRandomFloatInRange( radiusRange ), m_randNumGen->RollRandomFloatInRange( radiusRange ), m_randNumGen->RollRandomFloatInRange( radiusRange ) );
	obb31->m_obb3.m_iBasis = GetRandomDirection3D();
	obb31->m_obb3.m_jBasis = CrossProduct3D( GetRandomDirection3D(), obb31->m_obb3.m_iBasis ).GetNormalized();
	obb31->m_obb3.m_kBasis = CrossProduct3D( obb31->m_obb3.m_iBasis, obb31->m_obb3.m_jBasis );
	obb31->m_isWired = true;
	m_shapes.push_back( obb31 );

	TestShapeOBB3* obb32 = new TestShapeOBB3();
	obb32->m_obb3.m_center = Vec3( m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ) );
	obb32->m_obb3.m_halfDimensions = 0.5f * Vec3( m_randNumGen->RollRandomFloatInRange( radiusRange ), m_randNumGen->RollRandomFloatInRange( radiusRange ), m_randNumGen->RollRandomFloatInRange( radiusRange ) );
	obb32->m_obb3.m_iBasis = GetRandomDirection3D();
	obb32->m_obb3.m_jBasis = CrossProduct3D( GetRandomDirection3D(), obb32->m_obb3.m_iBasis ).GetNormalized();
	obb32->m_obb3.m_kBasis = CrossProduct3D( obb32->m_obb3.m_iBasis, obb32->m_obb3.m_jBasis );
	obb32->m_isWired = true;
	m_shapes.push_back( obb32 );

	TestShapeOBB3* obb33 = new TestShapeOBB3();
	obb33->m_obb3.m_center = Vec3( m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ) );
	obb33->m_obb3.m_halfDimensions = 0.5f * Vec3( m_randNumGen->RollRandomFloatInRange( radiusRange ), m_randNumGen->RollRandomFloatInRange( radiusRange ), m_randNumGen->RollRandomFloatInRange( radiusRange ) );
	obb33->m_obb3.m_iBasis = GetRandomDirection3D();
	obb33->m_obb3.m_jBasis = CrossProduct3D( GetRandomDirection3D(), obb33->m_obb3.m_iBasis ).GetNormalized();
	obb33->m_obb3.m_kBasis = CrossProduct3D( obb33->m_obb3.m_iBasis, obb33->m_obb3.m_jBasis );
	m_shapes.push_back( obb33 );

	TestShapeOBB3* obb34 = new TestShapeOBB3();
	obb34->m_obb3.m_center = Vec3( m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ), m_randNumGen->RollRandomFloatInRange( positionRange ) );
	obb34->m_obb3.m_halfDimensions = 0.5f * Vec3( m_randNumGen->RollRandomFloatInRange( radiusRange ), m_randNumGen->RollRandomFloatInRange( radiusRange ), m_randNumGen->RollRandomFloatInRange( radiusRange ) );
	obb34->m_obb3.m_iBasis = GetRandomDirection3D();
	obb34->m_obb3.m_jBasis = CrossProduct3D( GetRandomDirection3D(), obb34->m_obb3.m_iBasis ).GetNormalized();
	obb34->m_obb3.m_kBasis = CrossProduct3D( obb34->m_obb3.m_iBasis, obb34->m_obb3.m_jBasis );
	m_shapes.push_back( obb34 );

	TestShapePlane* plane3 = new TestShapePlane();
	plane3->m_plane.m_normal = GetRandomDirection3D();
	plane3->m_plane.m_distanceFromOrigin = m_randNumGen->RollRandomFloatInRange( positionRange );
	m_shapes.push_back( plane3 );
}

void RayVs3DTest::Update( float deltaSeconds )
{
	if (!g_window->IsFocus()) {
		g_theInput->SetCursorMode( false, false );
	}
	else {
		g_theInput->SetCursorMode( true, true );
	}

	float speed = 2.f;
	Vec3 forwardNormal = m_camera3D.m_orientation.GetIFwd();
	Vec2 horizontalForwardNormal = Vec2( forwardNormal ).GetNormalized();
	Vec2 horizontalLeftNormal = horizontalForwardNormal.GetRotated90Degrees();
	Vec3 translation;
	if (g_theInput->IsKeyDown( 'W' )) {
		translation += horizontalForwardNormal * deltaSeconds * speed;
	}
	if (g_theInput->IsKeyDown( 'A' )) {
		translation += horizontalLeftNormal * deltaSeconds * speed;
	}
	if (g_theInput->IsKeyDown( 'S' )) {
		translation -= horizontalForwardNormal * deltaSeconds * speed;
	}
	if (g_theInput->IsKeyDown( 'D' )) {
		translation -= horizontalLeftNormal * deltaSeconds * speed;
	}
	if (g_theInput->IsKeyDown( 'E' )) {
		translation += Vec3( 0.f, 0.f, 1.f ) * deltaSeconds * speed;
	}
	if (g_theInput->IsKeyDown( 'Q' )) {
		translation += Vec3( 0.f, 0.f, -1.f ) * deltaSeconds * speed;
	}
	if (g_theInput->WasKeyJustPressed( ' ' )) {
		m_referencePosLocked = !m_referencePosLocked;
	}
	m_camera3D.m_position += translation;

	if (deltaSeconds != 0.f) {
		Vec2 cursorDisp = g_theInput->GetCursorClientDelta();
		m_camera3D.m_orientation.m_yawDegrees -= 0.075f * cursorDisp.x * g_window->GetClientDimensions().x;
		m_camera3D.m_orientation.m_pitchDegrees -= 0.075f * cursorDisp.y * g_window->GetClientDimensions().y;
	}
	m_camera3D.m_orientation.m_pitchDegrees = GetClamped( m_camera3D.m_orientation.m_pitchDegrees, -89.9f, 89.9f );

	if (g_theInput->WasKeyJustPressed( KEYCODE_LEFTMOUSE )) {
		if (m_movingObject == nullptr && m_rayHitShape != nullptr) {
			m_movingObject = m_rayHitShape;
			m_movingObject->m_isGrabbedByUser = true;
		}
		else if(m_movingObject != nullptr) {
			m_movingObject->m_isGrabbedByUser = false;
			m_movingObject = nullptr;
		}
	}

	if (m_movingObject) {
		m_movingObject->Translate( translation );
	}

	if (!m_referencePosLocked) {
		m_referencePostion = m_camera3D.m_position;
		m_referenceForwardNormal = m_camera3D.m_orientation.GetIFwd();
	}

	for (auto shape : m_shapes) {
		shape->Update( deltaSeconds );
	}

	for (int i = 0; i < (int)m_shapes.size(); i++) {
		for (int j = i + 1; j < (int)m_shapes.size(); j++) {
			m_shapes[i]->CheckOverlap( m_shapes[j] );
		}
	}

	m_thisFrameRayRes.m_didImpact = false;
	m_thisFrameRayRes.m_impactDist = FLT_MAX;
	m_rayHitShape = nullptr;
	for (auto shape : m_shapes) {
		RayCastResult3D thisRes;
		if (shape->RayCastVsShape( thisRes, m_referencePostion, m_referenceForwardNormal, rayMaxDist )) {
			if (thisRes.m_impactDist < m_thisFrameRayRes.m_impactDist) {
				m_rayHitShape = shape;
				m_thisFrameRayRes = thisRes;
			}
		}
	}

	if (m_rayHitShape) {
		m_rayHitShape->m_isHitByRay = true;
	}

	Vec3 axesPos = m_camera3D.m_position + m_camera3D.m_orientation.GetIFwd() * 0.2f;
	DebugAddWorldArrow( axesPos, axesPos + Vec3( 0.01f, 0.f, 0.f ), 0.0005f, 0.f, Rgba8( 255, 0, 0 ), Rgba8( 255, 0, 0 ), DebugRenderMode::USE_DEPTH );
	DebugAddWorldArrow( axesPos, axesPos + Vec3( 0.f, 0.01f, 0.f ), 0.0005f, 0.f, Rgba8( 0, 255, 0 ), Rgba8( 0, 255, 0 ), DebugRenderMode::USE_DEPTH );
	DebugAddWorldArrow( axesPos, axesPos + Vec3( 0.f, 0.f, 0.01f ), 0.0005f, 0.f, Rgba8( 0, 0, 255 ), Rgba8( 0, 0, 255 ), DebugRenderMode::USE_DEPTH );

}

void RayVs3DTest::Render() const
{
	DebugRenderWorld( m_camera3D );

	g_theRenderer->BeginCamera( m_camera3D );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetModelConstants();
	std::vector<Vertex_PCU> verts;
	for (auto& shape : m_shapes) {
		shape->Render();
		Vec3 nearestPoint = shape->GetNearestPointOnShape( m_referencePostion );
		AddVertsForSphere3D( verts, nearestPoint, 0.02f, Rgba8( 255, 255, 0 ) );
	}
	if (m_thisFrameRayRes.m_didImpact) {
		AddVertsForSphere3D( verts, m_thisFrameRayRes.m_impactPos, 0.03f );
		AddVertsForArrow3D( verts, m_thisFrameRayRes.m_impactPos, m_thisFrameRayRes.m_impactPos + m_thisFrameRayRes.m_impactNormal * 0.1f, 0.01f, 0.03f, Rgba8( 255, 255, 0 ) );
	}
	if (m_referencePosLocked) {
		if (m_thisFrameRayRes.m_didImpact) {
			AddVertsForArrow3D( verts, m_thisFrameRayRes.m_impactPos, m_referencePostion + m_referenceForwardNormal * rayMaxDist, 0.01f, 0.03f, Rgba8( 192, 192, 192 ) );
			AddVertsForCylinder3D( verts, m_referencePostion, m_thisFrameRayRes.m_impactPos, 0.01f, Rgba8( 255, 0, 0 ) );
		}
		else {
			AddVertsForArrow3D( verts, m_referencePostion, m_referencePostion + m_referenceForwardNormal * rayMaxDist, 0.01f, 0.03f, Rgba8( 0, 255, 0 ) );
		}
	}

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );
	g_theRenderer->EndCamera( m_camera3D );

	RenderUI();
	DebugRenderScreen( m_camera2D );
}

void RayVs3DTest::RenderUI() const
{
	g_theRenderer->BeginCamera( m_camera2D );
	std::vector<Vertex_PCU> textVerts;
	textVerts.reserve( 1000 );
	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 5.f, 385.f ), 10.f, "Mode (F6/F7 for prev/next):  Ray Cast vs. 3D", Rgba8( 255, 255, 0 ), 0.6f );
	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 5.f, 370.f ), 10.f, "F8 to randomize; WASD fly horizontally, Q/E fly vertically, space lock reference position, T slow", Rgba8( 0, 204, 204 ), 0.6f );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->DrawVertexArray( textVerts );
	g_theRenderer->EndCamera( m_camera2D );
}

