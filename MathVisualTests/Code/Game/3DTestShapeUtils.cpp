#include "Game/3DTestShapeUtils.hpp"

//----------------------------------
// Add vertices for a plane
//void MathVisualTest_AddVertsForPlane3D( std::vector<Vertex_PCU>& verts, Plane3 const& plane ) {

//}

void MathVisualTest_AddVertsForWiredPlane3D( std::vector<Vertex_PCU>& verts, Plane3 const& plane ) {
	Vec3 center = plane.m_normal * plane.m_distanceFromOrigin;
	AddVertsForSphere3D( verts, center, 0.03f, Rgba8( 120, 120, 120 ) );
	AddVertsForArrow3D( verts, center, center + plane.m_normal, 0.03f, 0.06f, Rgba8( 0, 0, 255 ) );
	AddVertsForCylinder3D( verts, Vec3(), center, 0.03f, Rgba8( 150, 150, 150 ), AABB2::IDENTITY, 4 );

	Vec3 tangent, bitangent;
	if (plane.m_normal != Vec3( 0.f, 0.f, 1.f ) && plane.m_normal != Vec3( 0.f, 0.f, -1.f )) {
		tangent = CrossProduct3D( Vec3( 0.f, 0.f, 1.f ), plane.m_normal ).GetNormalized();
		bitangent = CrossProduct3D( plane.m_normal, tangent );
	}

	for (int i = -20; i <= 20; i++) {
		OBB3 tangentOBB3( center + bitangent * (float)i, Vec3( 20.f, 0.03f, 0.03f ), tangent, bitangent, plane.m_normal );
		AddVertsForOBB3D( verts, tangentOBB3, Rgba8( 0, 255, 0 ) );
	}

	for (int i = -20; i <= 20; i++) {
		OBB3 bitangentOBB3( center + tangent * (float)i, Vec3( 20.f, 0.03f, 0.03f ), bitangent, tangent, plane.m_normal );
		AddVertsForOBB3D( verts, bitangentOBB3, Rgba8( 0, 0, 255 ) );
	}
}




TestShapeSphere::TestShapeSphere()
{
	m_type = TestShapeType::Sphere;
}

void TestShapeSphere::Translate( Vec3 const& translation )
{
	m_center += translation;
}

void TestShapeSphere::Render() const
{
	float fraction = CalculateOverlapTimerFraction();
	std::vector<Vertex_PCU> m_verts;
	m_verts.reserve( 1000 );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetModelConstants();
	if (!m_isWired) {
		if (m_isHitByRay) {
			if (m_isGrabbedByUser) {
				AddVertsForSphere3D( m_verts, m_center, m_radius, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 255, 0, 0 ), fraction ), AABB2::IDENTITY, m_numOfLatitudeSlices, m_numOfLongitudeSlices );
			}
			else {
				AddVertsForSphere3D( m_verts, m_center, m_radius, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 0, 0, 255 ), fraction ), AABB2::IDENTITY, m_numOfLatitudeSlices, m_numOfLongitudeSlices );
			}
		}
		else {
			AddVertsForSphere3D( m_verts, m_center, m_radius, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8::WHITE, fraction ), AABB2::IDENTITY, m_numOfLatitudeSlices, m_numOfLongitudeSlices );
			if (m_isGrabbedByUser) {
				g_theRenderer->SetModelConstants( Mat44(), Rgba8( 150, 0, 0 ) );
			}
		}
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" ) );
	}
	else {
		if (m_isHitByRay) {
			AddVertsForWiredSphere3D( m_verts, m_center, m_radius, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 255, 255, 255 ), fraction ), m_numOfLatitudeSlices, m_numOfLongitudeSlices );
		}
		else{
			AddVertsForWiredSphere3D( m_verts, m_center, m_radius, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 93, 126, 235 ), fraction ), m_numOfLatitudeSlices, m_numOfLongitudeSlices );
		}
		g_theRenderer->BindTexture( nullptr );
		if (m_isGrabbedByUser) {
			g_theRenderer->SetModelConstants( Mat44(), Rgba8( 255, 0, 0 ) );
		}
	}
	g_theRenderer->DrawVertexArray( m_verts );

}

Vec3 TestShapeSphere::GetNearestPointOnShape( Vec3 const& refPos ) const
{
	return GetNearestPointOnSphere3D( refPos, m_center, m_radius );
}

bool TestShapeSphere::RayCastVsShape( RayCastResult3D& out_result, Vec3 const& startPos, Vec3 const& forwardVec, float maxDist )
{
	return RayCastVsSphere3D( out_result, startPos, forwardVec, maxDist, m_center, m_radius );
}

TestShapeAABB3::TestShapeAABB3()
{
	m_type = TestShapeType::AABB3;
}

void TestShapeAABB3::Translate( Vec3 const& translation )
{
	m_box.m_maxs += translation;
	m_box.m_mins += translation;
}

void TestShapeAABB3::Render() const
{
	float fraction = CalculateOverlapTimerFraction();
	std::vector<Vertex_PCU> m_verts;
	m_verts.reserve( 1000 );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetModelConstants();
	if (!m_isWired) {
		if (m_isHitByRay) {
			if (m_isGrabbedByUser) {
				AddVertsForAABB3D( m_verts, m_box, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 255, 0, 0 ), fraction ), AABB2::IDENTITY );
			}
			else {
				AddVertsForAABB3D( m_verts, m_box, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 0, 0, 255 ), fraction ), AABB2::IDENTITY );
			}
		}
		else {
			if (m_isGrabbedByUser) {
				g_theRenderer->SetModelConstants( Mat44(), Rgba8( 150, 0, 0 ) );
			}
			AddVertsForAABB3D( m_verts, m_box, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8::WHITE, fraction ), AABB2::IDENTITY );
		}
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" ) );
	}
	else {
		if (m_isHitByRay) {
			AddVertsForWiredAABB3D( m_verts, m_box, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 255, 255, 255 ), fraction ) );
		}
		else {
			AddVertsForWiredAABB3D( m_verts, m_box, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 93, 126, 235 ), fraction ) );
		}
		g_theRenderer->BindTexture( nullptr );
		if (m_isGrabbedByUser) {
			g_theRenderer->SetModelConstants( Mat44(), Rgba8( 255, 0, 0 ) );
		}
	}
	g_theRenderer->DrawVertexArray( m_verts );
}

Vec3 TestShapeAABB3::GetNearestPointOnShape( Vec3 const& refPos ) const
{
	return GetNearestPointOnAABB3D( refPos, m_box );
}

bool TestShapeAABB3::RayCastVsShape( RayCastResult3D& out_result, Vec3 const& startPos, Vec3 const& forwardVec, float maxDist )
{
	return RayCastVsAABB3D( out_result, startPos, forwardVec, maxDist, m_box );
}

TestShapeZCylinder::TestShapeZCylinder()
{
	m_type = TestShapeType::ZCylinder;
}

void TestShapeZCylinder::Translate( Vec3 const& translation )
{
	m_center += Vec2( translation );
	m_minZ += translation.z;
	m_maxZ += translation.z;
}

void TestShapeZCylinder::Render() const
{
	float fraction = CalculateOverlapTimerFraction();
	std::vector<Vertex_PCU> m_verts;
	m_verts.reserve( 1000 );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetModelConstants();
	if (!m_isWired) {
		if (m_isHitByRay) {
			if (m_isGrabbedByUser) {
				AddVertsForZCylinder3D( m_verts, m_center, m_minZ, m_maxZ, m_radius, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 255, 0, 0 ), fraction ), AABB2::IDENTITY, m_slices );
			}
			else {
				AddVertsForZCylinder3D( m_verts, m_center, m_minZ, m_maxZ, m_radius, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 0, 0, 255 ), fraction ), AABB2::IDENTITY, m_slices );
			}
		}
		else {
			if (m_isGrabbedByUser) {
				g_theRenderer->SetModelConstants( Mat44(), Rgba8( 150, 0, 0 ) );
			}
			AddVertsForZCylinder3D( m_verts, m_center, m_minZ, m_maxZ, m_radius, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8::WHITE, fraction ), AABB2::IDENTITY, m_slices );
		}
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" ) );
	}
	else {
		if (m_isHitByRay) {
			AddVertsForWiredCylinder3D( m_verts, Vec3( m_center.x, m_center.y, m_minZ ), Vec3( m_center.x, m_center.y, m_maxZ ), m_radius, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 255, 255, 255 ), fraction ), m_slices );
		}
		else {
			AddVertsForWiredCylinder3D( m_verts, Vec3( m_center.x, m_center.y, m_minZ ), Vec3( m_center.x, m_center.y, m_maxZ ), m_radius, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 93, 126, 235 ), fraction ), m_slices );
		}
		g_theRenderer->BindTexture( nullptr );
		if (m_isGrabbedByUser) {
			g_theRenderer->SetModelConstants( Mat44(), Rgba8( 255, 0, 0 ) );
		}
	}
	g_theRenderer->DrawVertexArray( m_verts );
}

Vec3 TestShapeZCylinder::GetNearestPointOnShape( Vec3 const& refPos ) const
{
	return GetNearestPointOnZCylinder3D( refPos, m_center, m_radius, m_minZ, m_maxZ );
}

bool TestShapeZCylinder::RayCastVsShape( RayCastResult3D& out_result, Vec3 const& startPos, Vec3 const& forwardVec, float maxDist )
{
	return RayCastVsCylinderZ3D( out_result, startPos, forwardVec, maxDist, m_center, m_minZ, m_maxZ, m_radius );
}

TestShape::~TestShape()
{

}

void TestShape::Update( float deltaSeconds )
{
	if (!m_isoverlapped) {
		m_overlapTimer = 0.f;
	}
	if (m_isoverlapped) {
		m_overlapTimer += deltaSeconds;
		if (m_overlapTimer >= 0.5f) {
			m_overlapTimer -= 0.5f;
		}
		m_isoverlapped = false;
	}
	m_isHitByRay = false;
}

void TestShape::CheckOverlap( TestShape* other )
{
	if (m_type == TestShapeType::AABB3 && other->m_type == TestShapeType::AABB3) {
		if (DoAABB3sOverlap3D( ((TestShapeAABB3*)this)->m_box, ((TestShapeAABB3*)other)->m_box )) {
			m_isoverlapped = true;
			other->m_isoverlapped = true;
		}
	}
	else if (m_type == TestShapeType::Sphere && other->m_type == TestShapeType::Sphere) {
		if (DoSpheresOverlap( ((TestShapeSphere*)this)->m_center, ((TestShapeSphere*)this)->m_radius, ((TestShapeSphere*)other)->m_center, ((TestShapeSphere*)other)->m_radius )) {
			m_isoverlapped = true;
			other->m_isoverlapped = true;
		}
	}
	else if (m_type == TestShapeType::ZCylinder && other->m_type == TestShapeType::ZCylinder) {
		if (DoZCylindersOverlap3D( ((TestShapeZCylinder*)this)->m_center, ((TestShapeZCylinder*)this)->m_radius, ((TestShapeZCylinder*)this)->m_minZ, ((TestShapeZCylinder*)this)->m_maxZ,
			((TestShapeZCylinder*)other)->m_center, ((TestShapeZCylinder*)other)->m_radius, ((TestShapeZCylinder*)other)->m_minZ, ((TestShapeZCylinder*)other)->m_maxZ )) {
			m_isoverlapped = true;
			other->m_isoverlapped = true;
		}
	}
	else if (m_type == TestShapeType::Sphere && other->m_type == TestShapeType::ZCylinder) {
		if (DoZCylinderAndSphereOverlap3D( ((TestShapeZCylinder*)other)->m_center, ((TestShapeZCylinder*)other)->m_radius, ((TestShapeZCylinder*)other)->m_minZ, ((TestShapeZCylinder*)other)->m_maxZ,
			((TestShapeSphere*)this)->m_center, ((TestShapeSphere*)this)->m_radius )) {
			m_isoverlapped = true;
			other->m_isoverlapped = true;
		}
	}
	else if (m_type == TestShapeType::ZCylinder && other->m_type == TestShapeType::Sphere) {
		if (DoZCylinderAndSphereOverlap3D( ((TestShapeZCylinder*)this)->m_center, ((TestShapeZCylinder*)this)->m_radius, ((TestShapeZCylinder*)this)->m_minZ, ((TestShapeZCylinder*)this)->m_maxZ,
			((TestShapeSphere*)other)->m_center, ((TestShapeSphere*)other)->m_radius )) {
			m_isoverlapped = true;
			other->m_isoverlapped = true;
		}
	}
	else if (m_type == TestShapeType::Sphere && other->m_type == TestShapeType::AABB3) {
		if (DoSphereAndAABB3Overlap3D( ((TestShapeSphere*)this)->m_center, ((TestShapeSphere*)this)->m_radius, ((TestShapeAABB3*)other)->m_box )) {
			m_isoverlapped = true;
			other->m_isoverlapped = true;
		}
	}
	else if (m_type == TestShapeType::AABB3 && other->m_type == TestShapeType::Sphere) {
		if (DoSphereAndAABB3Overlap3D( ((TestShapeSphere*)other)->m_center, ((TestShapeSphere*)other)->m_radius, ((TestShapeAABB3*)this)->m_box )) {
			m_isoverlapped = true;
			other->m_isoverlapped = true;
		}
	}
	else if (m_type == TestShapeType::ZCylinder && other->m_type == TestShapeType::AABB3) {
		if (DoAABB3AndZCylinderOverlap3D( ((TestShapeAABB3*)other)->m_box,
			((TestShapeZCylinder*)this)->m_center, ((TestShapeZCylinder*)this)->m_radius, ((TestShapeZCylinder*)this)->m_minZ, ((TestShapeZCylinder*)this)->m_maxZ )) {
			m_isoverlapped = true;
			other->m_isoverlapped = true;
		}
	}
	else if (m_type == TestShapeType::AABB3 && other->m_type == TestShapeType::ZCylinder) {
		if (DoAABB3AndZCylinderOverlap3D( ((TestShapeAABB3*)this)->m_box,
			((TestShapeZCylinder*)other)->m_center, ((TestShapeZCylinder*)other)->m_radius, ((TestShapeZCylinder*)other)->m_minZ, ((TestShapeZCylinder*)other)->m_maxZ )) {
			m_isoverlapped = true;
			other->m_isoverlapped = true;
		}
	}
	else if (m_type == TestShapeType::OBB3 && other->m_type == TestShapeType::Plane) {
		if (DoOBB3AndPlaneOverlap3D( ((TestShapeOBB3*)this)->m_obb3, ((TestShapePlane*)other)->m_plane )) {
			m_isoverlapped = true;
			other->m_isoverlapped = true;
		}
	}
	else if (m_type == TestShapeType::AABB3 && other->m_type == TestShapeType::Plane) {
		if (DoAABB3AndPlaneOverlap3D( ((TestShapeAABB3*)this)->m_box, ((TestShapePlane*)other)->m_plane )) {
			m_isoverlapped = true;
			other->m_isoverlapped = true;
		}
	}
	else if (m_type == TestShapeType::Sphere && other->m_type == TestShapeType::Plane) {
		if (DoSphereAndPlaneOverlap3D( ((TestShapeSphere*)this)->m_center, ((TestShapeSphere*)this)->m_radius, ((TestShapePlane*)other)->m_plane )) {
			m_isoverlapped = true;
			other->m_isoverlapped = true;
		}
	}
	else if (m_type == TestShapeType::ZCylinder && other->m_type == TestShapeType::Plane) {
		if (DoZCylinderAndPlaneOverlap3D( ((TestShapeZCylinder*)this)->m_center, ((TestShapeZCylinder*)this)->m_radius, ((TestShapeZCylinder*)this)->m_minZ, ((TestShapeZCylinder*)this)->m_maxZ, ((TestShapePlane*)other)->m_plane )) {
			m_isoverlapped = true;
			other->m_isoverlapped = true;
		}
	}
	else if (m_type == TestShapeType::AABB3 && other->m_type == TestShapeType::OBB3) {
		if (DoAABB3AndOBB3Overlap3D( ((TestShapeAABB3*)this)->m_box, ((TestShapeOBB3*)other)->m_obb3 )) {
			m_isoverlapped = true;
			other->m_isoverlapped = true;
		}
	}
	else if (m_type == TestShapeType::Sphere && other->m_type == TestShapeType::OBB3) {
		if (DoSphereAndOBB3Overlap3D( ((TestShapeSphere*)this)->m_center, ((TestShapeSphere*)this)->m_radius, ((TestShapeOBB3*)other)->m_obb3 )) {
			m_isoverlapped = true;
			other->m_isoverlapped = true;
		}
	}
	else if (m_type == TestShapeType::OBB3 && other->m_type == TestShapeType::OBB3) {
		if (DoOBB3sOverlap3D( ((TestShapeOBB3*)this)->m_obb3, ((TestShapeOBB3*)other)->m_obb3 )) {
			m_isoverlapped = true;
			other->m_isoverlapped = true;
		}
	}
}

float TestShape::CalculateOverlapTimerFraction() const
{
	if (m_isoverlapped) {
		float normalizedTimer = m_overlapTimer * 4.f;
		if (normalizedTimer <= 1.f) {
			return normalizedTimer;
		}
		else {
			return 2.f - normalizedTimer;
		}
	}
	return 1.f;
}

TestShapePlane::TestShapePlane()
{
	m_type = TestShapeType::Plane;
}

void TestShapePlane::Translate( Vec3 const& translation )
{
	m_plane.Translate( translation );
}

void TestShapePlane::Render() const
{
	float fraction = CalculateOverlapTimerFraction();
	std::vector<Vertex_PCU> m_verts;
	m_verts.reserve( 1000 );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );

	MathVisualTest_AddVertsForWiredPlane3D( m_verts, m_plane );
	g_theRenderer->BindTexture( nullptr );

	//if (m_isGrabbedByUser) {
	//	g_theRenderer->SetModelConstants( Mat44(), Rgba8( 255, 0, 0 ) );
	//}
	//else 
	if (m_isHitByRay) {
		g_theRenderer->SetModelConstants( Mat44(), Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 255, 255, 255 ), fraction ) );
	}
	else {
		g_theRenderer->SetModelConstants( Mat44(), Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 93, 126, 235 ), fraction ) );
	}

	g_theRenderer->DrawVertexArray( m_verts );
}

Vec3 TestShapePlane::GetNearestPointOnShape( Vec3 const& refPos ) const
{
	return m_plane.GetNearestPoint( refPos );
}

bool TestShapePlane::RayCastVsShape( RayCastResult3D& out_result, Vec3 const& startPos, Vec3 const& forwardVec, float maxDist )
{
	return RayCastVsPlane3D( out_result, startPos, forwardVec, maxDist, m_plane );
}

TestShapeOBB3::TestShapeOBB3()
{
	m_type = TestShapeType::OBB3;
}

void TestShapeOBB3::Translate( Vec3 const& translation )
{
	return m_obb3.Transalate( translation );
}

void TestShapeOBB3::Render() const
{
	float fraction = CalculateOverlapTimerFraction();
	std::vector<Vertex_PCU> m_verts;
	m_verts.reserve( 1000 );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetModelConstants();
	if (!m_isWired) {
		if (m_isHitByRay) {
			if (m_isGrabbedByUser) {
				AddVertsForOBB3D( m_verts, m_obb3, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 255, 0, 0 ), fraction ), AABB2::IDENTITY );
			}
			else {
				AddVertsForOBB3D( m_verts, m_obb3, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 0, 0, 255 ), fraction ), AABB2::IDENTITY );
			}
		}
		else {
			if (m_isGrabbedByUser) {
				g_theRenderer->SetModelConstants( Mat44(), Rgba8( 150, 0, 0 ) );
			}
			AddVertsForOBB3D( m_verts, m_obb3, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8::WHITE, fraction ), AABB2::IDENTITY );
		}
		g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" ) );
	}
	else {
		if (m_isHitByRay) {
			AddVertsForWiredOBB3D( m_verts, m_obb3, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 255, 255, 255 ), fraction ) );
		}
		else {
			AddVertsForWiredOBB3D( m_verts, m_obb3, Rgba8::Interpolate( Rgba8( 0, 0, 0 ), Rgba8( 93, 126, 235 ), fraction ) );
		}
		g_theRenderer->BindTexture( nullptr );
		if (m_isGrabbedByUser) {
			g_theRenderer->SetModelConstants( Mat44(), Rgba8( 255, 0, 0 ) );
		}
	}
	g_theRenderer->DrawVertexArray( m_verts );
}

Vec3 TestShapeOBB3::GetNearestPointOnShape( Vec3 const& refPos ) const
{
	return m_obb3.GetNearestPoint( refPos );
}

bool TestShapeOBB3::RayCastVsShape( RayCastResult3D& out_result, Vec3 const& startPos, Vec3 const& forwardVec, float maxDist )
{
	return RayCastVsOBB3D( out_result, startPos, forwardVec, maxDist, m_obb3 );
}
