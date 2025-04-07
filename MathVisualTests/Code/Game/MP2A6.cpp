#include "Game/MP2A6.hpp"

PachinkoMachine2DTest::PachinkoMachine2DTest()
{

}

PachinkoMachine2DTest::~PachinkoMachine2DTest()
{
	for (auto shape : m_blockShapes) {
		delete shape;
	}

	for (auto billiard : m_billiards) {
		delete billiard;
	}
}

void PachinkoMachine2DTest::StartUp()
{
	m_dimensionOfBoundingBoxes = IntVec2( 4, 4 );
	m_camera2D.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 0.f, 1.f );
	m_camera2D.m_mode = CameraMode::Orthographic;
	g_theInput->SetCursorMode( false, false );
	m_blockShapes.resize( 33 );
	m_billiards.reserve( 1000 );
	m_boundingBoxes.resize( (size_t)m_dimensionOfBoundingBoxes.x * m_dimensionOfBoundingBoxes.y );
	m_rayStart = m_camera2D.m_cameraBox.GetRandomPointInside();
	m_rayEnd = m_camera2D.m_cameraBox.GetRandomPointInside();

	float xLength = WORLD_SIZE_X / (float)m_dimensionOfBoundingBoxes.x;
	float yLength = WORLD_SIZE_Y / (float)m_dimensionOfBoundingBoxes.y;
	for (int i = 0; i < m_dimensionOfBoundingBoxes.x; i++) {
		for (int j = 0; j < m_dimensionOfBoundingBoxes.y; j++) {
			int index = j * m_dimensionOfBoundingBoxes.x + i;
			m_boundingBoxes[index].m_mins = Vec2( (float)i * xLength, (float)j * yLength );
			m_boundingBoxes[index].m_maxs = Vec2( (float)(i + 1) * xLength, (float)(j + 1) * yLength );
		}
	}

	PachinkoShapeOBB* leftWall = new PachinkoShapeOBB();
	leftWall->m_box.m_center = Vec2( -50.f, 200.f );
	leftWall->m_box.m_halfDimensions = Vec2( 50.f, 10000.f );
	leftWall->m_box.m_iBasisNormal = Vec2( 1.f, 0.f );
	leftWall->m_elasticity = 0.9f;
	m_blockShapes[30] = leftWall;
	PachinkoShapeOBB* rightWall = new PachinkoShapeOBB();
	rightWall->m_box.m_center = Vec2( 850.f, 200.f );
	rightWall->m_box.m_halfDimensions = Vec2( 50.f, 10000.f );
	rightWall->m_box.m_iBasisNormal = Vec2( 1.f, 0.f );
	rightWall->m_elasticity = 0.9f;
	m_blockShapes[31] = rightWall;
	PachinkoShapeOBB* bottomWall = new PachinkoShapeOBB();
	bottomWall->m_box.m_center = Vec2( 400.f, -50.f );
	bottomWall->m_box.m_halfDimensions = Vec2( 1000.f, 50.f );
	bottomWall->m_box.m_iBasisNormal = Vec2( 1.f, 0.f );
	bottomWall->m_elasticity = 0.9f;
	m_blockShapes[32] = bottomWall;
	RandomizeTest();
}

void PachinkoMachine2DTest::RandomizeTest()
{
	for (int i = 0; i < 30; i++) {
		delete m_blockShapes[i];
		m_blockShapes[i] = nullptr;
	}
	for (int i = 0; i < (int)m_billiards.size(); i++) {
		delete m_billiards[i];
		m_billiards[i] = nullptr;
	}
	m_billiards.clear();
	for (int i = 0; i < 10; i++) {
		PachinkoShapeDisc* shape1 = new PachinkoShapeDisc();
		shape1->m_elasticity = m_randNumGen->RollRandomFloatInRange( m_minElasticity, m_maxElasticity );
		shape1->m_center = m_camera2D.m_cameraBox.GetRandomPointInside();
		shape1->m_radius = m_randNumGen->RollRandomFloatInRange( 10.f, 20.f );
		m_blockShapes[i * 3] = shape1;

		PachinkoShapeOBB* shape2 = new PachinkoShapeOBB();
		shape2->m_elasticity = m_randNumGen->RollRandomFloatInRange( m_minElasticity, m_maxElasticity );
		shape2->m_box.m_center = m_camera2D.m_cameraBox.GetRandomPointInside();
		shape2->m_box.m_iBasisNormal = GetRandomPointOnUnitCircle2D();
		shape2->m_box.m_halfDimensions = Vec2( m_randNumGen->RollRandomFloatInRange( 10.f, 20.f ), m_randNumGen->RollRandomFloatInRange( 10.f, 20.f ) );
		m_blockShapes[i * 3 + 1] = shape2;

		PachinkoShapeCapsule* shape3 = new PachinkoShapeCapsule();
		shape3->m_elasticity = m_randNumGen->RollRandomFloatInRange( m_minElasticity, m_maxElasticity );
		shape3->m_startPos = m_camera2D.m_cameraBox.GetRandomPointInside();
		Vec2 displacement = GetRandomPointOnUnitCircle2D() * m_randNumGen->RollRandomFloatInRange( 10.f, 20.f );
		shape3->m_endPos = shape3->m_startPos + displacement;
		shape3->m_radius = m_randNumGen->RollRandomFloatInRange( 5.f, 10.f );
		m_blockShapes[i * 3 + 2] = shape3;
	}
}

void PachinkoMachine2DTest::Update( float deltaSeconds )
{
	m_verts.clear();
	m_verts.reserve( 1000000 );

	m_deltaSecondsThisFrame = deltaSeconds;

	float moveSpeed = 80.f;

	if (g_theInput->IsKeyDown( 'W' )) {
		m_rayStart.y += moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'A' )) {
		m_rayStart.x -= moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'S' )) {
		m_rayStart.y -= moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'D' )) {
		m_rayStart.x += moveSpeed * deltaSeconds;
	}

	if (g_theInput->IsKeyDown( 'I' )) {
		m_rayEnd.y += moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'J' )) {
		m_rayEnd.x -= moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'K' )) {
		m_rayEnd.y -= moveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown( 'L' )) {
		m_rayEnd.x += moveSpeed * deltaSeconds;
	}

	if (g_theInput->IsKeyDown( KEYCODE_LEFTMOUSE )) {
		m_rayStart = m_camera2D.GetCursorWorldPosition( g_window->GetNormalizedCursorPos() );
	}
	if (g_theInput->IsKeyDown( KEYCODE_RIGHTMOUSE )) {
		m_rayEnd = m_camera2D.GetCursorWorldPosition( g_window->GetNormalizedCursorPos() );
	}

	if (g_theInput->WasKeyJustPressed( 'U' )) {
		m_isUsingNormalUpdate = !m_isUsingNormalUpdate;
	}

	if (g_theInput->WasKeyJustPressed( 'B' )) {
		m_isFloorExist = !m_isFloorExist;
	}

	if (g_theInput->WasKeyJustPressed( 219 )) {
		m_physicsSimulationTime *= 0.9f;
	}

	if (g_theInput->WasKeyJustPressed( 221 )) {
		m_physicsSimulationTime *= 1.1f;
	}

	m_physicsSimulationTime = GetClamped( m_physicsSimulationTime, 0.001f, 0.1f );

	float speed = 2.f;
	if (g_theInput->WasKeyJustPressed( ' ' )) {
		PachinkoBilliard* billiard = new PachinkoBilliard();
		billiard->m_position = m_rayStart;
		billiard->m_elasticity = m_randNumGen->RollRandomFloatInRange( 0.6f, m_maxElasticity );
		billiard->m_radius = m_randNumGen->RollRandomFloatInRange( m_minRadius, m_maxRadius );
		billiard->m_velocity = speed * (m_rayEnd - m_rayStart);
		billiard->m_test = this;
		billiard->Startup();
		m_billiards.push_back( billiard );
	}

	if (g_theInput->IsKeyDown( 'N' )) {
		PachinkoBilliard* billiard = new PachinkoBilliard();
		billiard->m_position = m_rayStart;
		billiard->m_elasticity = m_randNumGen->RollRandomFloatInRange( 0.6f, m_maxElasticity );
		billiard->m_radius = m_randNumGen->RollRandomFloatInRange( m_minRadius, m_maxRadius );
		billiard->m_velocity = speed * (m_rayEnd - m_rayStart);
		billiard->m_test = this;
		billiard->Startup();
		m_billiards.push_back( billiard );
	}

	UpdatePhysics( deltaSeconds );

	for (auto billiard : m_billiards) {
		if (billiard->m_position.y < 0.f) {
			billiard->m_position.y = 450.f;
		}
	}

	for (auto shape : m_blockShapes) {
		if (shape) {
			shape->AddVerts( m_verts );
		}
	}
	for (auto disc : m_billiards) {
		if (disc) {
			disc->AddVerts( m_verts );
		}
	}
}

void PachinkoMachine2DTest::Render() const
{
	g_theRenderer->BeginCamera( m_camera2D );

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( m_verts );

	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;

	AddVertsForArrow2D( verts, m_rayStart, m_rayEnd, 10.f, 1.f, Rgba8( 255, 255, 0 ) );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );
	DebugDrawRing( m_rayStart, m_minRadius, 1.5f, Rgba8( 51, 51, 255 ) );
	DebugDrawRing( m_rayStart, m_maxRadius, 1.5f, Rgba8( 51, 51, 255 ) );


	g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 5.f, 385.f ), 10.f, "Mode (F6/F7 for prev/next):  Pachinko Machine (2D)", Rgba8( 255, 255, 0 ), 0.6f );
	if (m_isUsingNormalUpdate) {
		g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 5.f, 370.f ), 10.f, Stringf( "F8 to randomize; LMB/RMB/WASD/IJKL move, T=slow, space/N=ball (%d), B=bottom warp on, variable timestep(U), dt=%.1fms", (int)m_billiards.size(), m_deltaSecondsThisFrame * 1000.f ), Rgba8( 0, 204, 204 ), 0.6f );
	}
	else {
		g_ASCIIFont->AddVertsForText2D( textVerts, Vec2( 5.f, 370.f ), 10.f, Stringf( "F8 to randomize; LMB/RMB/WASD/IJKL move, T=slow, space/N=ball (%d), B=bottom warp on, timestep=%.2fms(U,[,]), dt=%.1fms", (int)m_billiards.size(), m_physicsSimulationTime * 1000.f, m_deltaSecondsThisFrame * 1000.f ), Rgba8( 0, 204, 204 ), 0.6f );

	}
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->DrawVertexArray( textVerts );

	g_theRenderer->EndCamera( m_camera2D );
}

void PachinkoMachine2DTest::UpdatePhysics( float deltaSeconds )
{
	if (m_isUsingNormalUpdate) {
		for (auto billiard : m_billiards) {
			billiard->UpdatePhysics( deltaSeconds );
		}
		for (int i = 0; i < (int)m_billiards.size(); i++) {
			for (int j = i + 1; j < (int)m_billiards.size(); j++) {
				if (!DoDiscsOverlap( m_billiards[i]->m_position, m_billiards[i]->m_radius, m_billiards[j]->m_position, m_billiards[j]->m_radius )) {
					continue;
				}
				BounceDiscOutOfEachOther2D( m_billiards[i]->m_position, m_billiards[i]->m_radius, m_billiards[i]->m_velocity, m_billiards[i]->m_elasticity, m_billiards[j]->m_position, m_billiards[j]->m_radius, m_billiards[j]->m_velocity, m_billiards[j]->m_elasticity );
			}
		}
		for (auto billiard : m_billiards) {
			for (int i = 0; i < 30; i++) {
				m_blockShapes[i]->BounceOff( billiard );
			}
		}
		for (auto billiard : m_billiards) {
			if (m_isFloorExist) {
				for (int i = 30; i < 33; i++) {
					m_blockShapes[i]->BounceOff( billiard );
				}
			}
			else {
				for (int i = 30; i < 32; i++) {
					m_blockShapes[i]->BounceOff( billiard );
				}
			}
		}
	}
	else {
		m_physicsSecondsDebt += deltaSeconds;
		while (m_physicsSecondsDebt >= m_physicsSimulationTime) {
			m_physicsSecondsDebt -= m_physicsSimulationTime;
			for (auto billiard : m_billiards) {
				billiard->UpdatePhysics( m_physicsSimulationTime );
			}
			for (int i = 0; i < (int)m_billiards.size(); i++) {
				for (int j = i + 1; j < (int)m_billiards.size(); j++) {
					BounceDiscOutOfEachOther2D( m_billiards[i]->m_position, m_billiards[i]->m_radius, m_billiards[i]->m_velocity, m_billiards[i]->m_elasticity, m_billiards[j]->m_position, m_billiards[j]->m_radius, m_billiards[j]->m_velocity, m_billiards[j]->m_elasticity );
				}
			}
			for (auto billiard : m_billiards) {
				for (int i = 0; i < 30; i++) {
					m_blockShapes[i]->BounceOff( billiard );
				}
			}
			for (auto billiard : m_billiards) {
				if (m_isFloorExist) {
					for (int i = 30; i < 33; i++) {
						m_blockShapes[i]->BounceOff( billiard );
					}
				}
				else {
					for (int i = 30; i < 32; i++) {
						m_blockShapes[i]->BounceOff( billiard );
					}
				}
			}
		}
	}
}

void PachinkoShapeDisc::BounceOff( PachinkoBilliard* mobileBilliard )
{
	BounceDiscOutOfFixedDisc2D( mobileBilliard->m_position, mobileBilliard->m_radius, mobileBilliard->m_velocity, mobileBilliard->m_elasticity, m_center, m_radius, m_elasticity );
}

void PachinkoShapeDisc::AddVerts( std::vector<Vertex_PCU>& verts )
{
	AddVertsForDisc2D( verts, m_center, m_radius, Rgba8::Interpolate( Rgba8( 255, 0, 0 ), Rgba8( 0, 255, 0 ), m_elasticity ) );
}

void PachinkoShapeOBB::BounceOff( PachinkoBilliard* mobileBilliard )
{
	BounceDiscOutOfFixedOBB2D( mobileBilliard->m_position, mobileBilliard->m_radius, mobileBilliard->m_velocity, mobileBilliard->m_elasticity, m_box, m_elasticity );
}

void PachinkoShapeOBB::AddVerts( std::vector<Vertex_PCU>& verts )
{
	AddVertsForOBB2D( verts, m_box, Rgba8::Interpolate( Rgba8( 255, 0, 0 ), Rgba8( 0, 255, 0 ), m_elasticity ) );
}

void PachinkoShapeCapsule::BounceOff( PachinkoBilliard* mobileBilliard )
{
	BounceDiscOutOfFixedCapsule2D( mobileBilliard->m_position, mobileBilliard->m_radius, mobileBilliard->m_velocity, mobileBilliard->m_elasticity, m_startPos, m_endPos, m_radius, m_elasticity );
}

void PachinkoShapeCapsule::AddVerts( std::vector<Vertex_PCU>& verts )
{
	AddVertsForCapsule2D( verts, m_startPos, m_endPos, m_radius, Rgba8::Interpolate( Rgba8( 255, 0, 0 ), Rgba8( 0, 255, 0 ), m_elasticity ) );
}

PachinkoBilliard::~PachinkoBilliard()
{
}

void PachinkoBilliard::Startup()
{
	m_color = Rgba8::Interpolate( Rgba8( 0, 0, 255 ), Rgba8( 255, 255, 255 ), m_elasticity );
}

void PachinkoBilliard::UpdatePhysics( float deltaSeconds )
{
	m_velocity.y -= 400.f * deltaSeconds;
	m_position += m_velocity * deltaSeconds;
}

void PachinkoBilliard::AddVerts( std::vector<Vertex_PCU>& verts )
{
	AddVertsForDisc2D( verts, m_position, m_radius, m_color );
	//g_theRenderer->SetModelConstants( Mat44::CreateTranslation2D( m_position ) );
	//g_theRenderer->BindTexture( nullptr );
	//g_theRenderer->DrawVertexBuffer( m_vertexBuffer, m_vertexBuffer->GetVertexCount() );
}

PachinkoShape::~PachinkoShape()
{
}
