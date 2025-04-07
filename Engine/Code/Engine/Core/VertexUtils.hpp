#pragma once
#include <vector>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/EulerAngles.hpp"
struct Vertex_PCU;
struct Vertex_PCUTBN;
struct AABB2;
struct OBB2;
struct Rgba8;
struct AABB3;
struct OBB3;
struct ConvexPoly2;

void TransformVertexArrayXY3D( int numVerts, Vertex_PCU* verts, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY );
void TransformVertexArrayXY3D( int numVerts, Vertex_PCU* verts, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY );
void TransformVertexArrayXY3D( std::vector<Vertex_PCU>& verts, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY );
void TransformVertexArray3D( std::vector<Vertex_PCU>& verts, Vec3 const& position, EulerAngles const& rotation );
void TransformVertexArray3D( std::vector<Vertex_PCU>& verts, Mat44 const& transform );

AABB2 const GetVertexBounds2D( std::vector<Vertex_PCU> const& verts );

//----------------------------------
// Add capsule vertexes to a vector, capsule is a line segment with a radius
void AddVertsForCapsule2D( std::vector<Vertex_PCU>& verts, Vec2 const& boneStartPoint, Vec2 const& boneEndPoint, float radius, Rgba8 const& color );
//----------------------------------
// Add disc vertexes to a vector
void AddVertsForDisc2D( std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 const& color );
//----------------------------------
// Add AABB2D vertexes to a vector
void AddVertsForAABB2D( std::vector<Vertex_PCU>& verts, AABB2 const& aabbBox, Rgba8 const& color, Vec2 const& uvAtMins = Vec2( 0.f, 0.f ), Vec2 const& uvAtMaxs = Vec2( 1.f, 1.f ) );
// Add AABB2D vertexes to a vector
void AddVertsForAABB2D( std::vector<Vertex_PCU>& verts, AABB2 const& aabbBox, Rgba8 const& color, AABB2 const& uvs );
//----------------------------------
// Add OBB2D vertexes to a vector
void AddVertsForOBB2D( std::vector<Vertex_PCU>& verts, OBB2 const& obbBox, Rgba8 const& color, Vec2 const& uvAtMins = Vec2( 0.f, 0.f ), Vec2 const& uvAtMaxs = Vec2( 1.f, 1.f ) );
// Add OBB2D vertexes to a vector
void AddVertsForOBB2D( std::vector<Vertex_PCU>& verts, OBB2 const& obbBox, Rgba8 const& color, AABB2 const& uvs );
//----------------------------------
// Add line segment vertexes to a vector, line segment is represented by two points
void AddVertsForLineSegment2D( std::vector<Vertex_PCU>& verts, Vec2 const& startPoint, Vec2 const& endPoint, float thickness, Rgba8 const& color );
//----------------------------------
// Add Sector vertexes to a vector
// tip is the center(start) of the sector
// forward is which direction the sector points to 
// aperture is how many degrees this sector has
void AddVertsForSector2D( std::vector<Vertex_PCU>& verts, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius, Rgba8 const& color );
//----------------------------------
// Add Sector vertexes to a vector
// tip is the center(start) of the sector
// forward is which direction the sector points to 
// aperture is how many degrees this sector has
void AddVertsForSector2D( std::vector<Vertex_PCU>& verts, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius, Rgba8 const& color );
//----------------------------------
// Add Arrow vertexes to a vector
void AddVertsForArrow2D( std::vector<Vertex_PCU>& verts, Vec2 const& tailPos, Vec2 const& tipPos, float arrowSize, float lineThickness, Rgba8 const& color );
//----------------------------------
// Add 2D Convex Polygon vertices to a vector
void AddVertsForConvexPoly2D( std::vector<Vertex_PCU>& verts, ConvexPoly2 const& convexPoly2, Rgba8 const& color );
//----------------------------------
// Add vertexes for a triangle, the sequence(ccw) of vertex is important
void AddVertsForTriangle3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& a, Vec3 const& b, Vec3 const& c, Rgba8 const& color = Rgba8::WHITE, Vec2 const& uv1 = Vec2(), Vec2 const& uv2 = Vec2(), Vec2 const& uv3 = Vec2() );
//----------------------------------
// Add vertexes for a quad, the sequence of vertex is important
void AddVertsForQuad3D( std::vector<Vertex_PCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& uvs = AABB2::IDENTITY );
//----------------------------------
// Add vertexes for a quad, the sequence of vertex is important
void AddVertsForQuad3D( std::vector<Vertex_PCUTBN>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& uvs = AABB2::IDENTITY );
//----------------------------------
// Add vertexes and indexes (index buffer)for a quad, the sequence of vertex is important
void AddVertsForQuad3D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& uvs = AABB2::IDENTITY );
//----------------------------------
// Add vertexes and indexes (index buffer)for a quad, the sequence of vertex is important
void AddVertsForQuad3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& uvs = AABB2::IDENTITY );
//----------------------------------
// Add vertexes for a quad, the sequence of vertex is important
void AddVertsForRoundedQuad3D( std::vector<Vertex_PCUTBN>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& uvs = AABB2::IDENTITY );
//----------------------------------
// Add vertexes for a wired quad, the sequence of vertex is important
void AddVertsForWiredQuad3D( std::vector<Vertex_PCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, float wireRadius = 0.01f );
//----------------------------------
// Add vertexes for a AABB3D which consists 6 quads
void AddVertsForAABB3D( std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& uvs = AABB2::IDENTITY );
//----------------------------------
// Add vertexes for a AABB3D which consists 6 quads
void AddVertsForAABB3D( std::vector<Vertex_PCUTBN>& verts, AABB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& uvs = AABB2::IDENTITY );
//----------------------------------
// Add vertexes and indexes (index buffer)for a AABB3D
void AddVertsForAABB3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& uvs = AABB2::IDENTITY );
//----------------------------------
// Add vertexes and indexes (index buffer)for a AABB3D
void AddVertsForAABB3D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& uvs = AABB2::IDENTITY );
//----------------------------------
// Add vertexes for a wired AABB3D which consists 6 quads
void AddVertsForWiredAABB3D( std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color = Rgba8::WHITE );
//----------------------------------
// Add vertexes for a sphere, Z is top
void AddVertsForSphere3D( std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& uvs = AABB2::IDENTITY, int numLatitudeSlices = 8, int numLongitudeSlices = 16 );
//----------------------------------
// Add vertexes for a sphere, Z is top
void AddVertsForSphere3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& uvs = AABB2::IDENTITY, int numLatitudeSlices = 8, int numLongitudeSlices = 16 );
//----------------------------------
// Add vertexes for a wired sphere, Z is top
void AddVertsForWiredSphere3D( std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, Rgba8 const& color = Rgba8::WHITE, int numLatitudeSlices = 8, int numLongitudeSlices = 16 );
//----------------------------------
// Add vertexes for a ZCylinder
void AddVertsForZCylinder3D( std::vector<Vertex_PCU>& verts, Vec2 const& center, float minZ, float maxZ, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& uvs = AABB2::IDENTITY, int numSlices = 8 );
//----------------------------------
// Add vertexes for a cylinder of any direction
void AddVertsForCylinder3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& uvs = AABB2::IDENTITY, int numSlices = 8 );
//----------------------------------
// Add vertexes for a wired cylinder of any direction
void AddVertsForWiredCylinder3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8::WHITE, int numSlices = 8 );
//----------------------------------
// Add vertexes for a cone of any direction
void AddVertsForCone3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& uvs = AABB2::IDENTITY, int numSlices = 8 );
//----------------------------------
// Add vertexes for a wired cone of any direction
void AddVertsForWiredCone3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8::WHITE, int numSlices = 8 );
//----------------------------------
// Add vertexes for an arrow which is actually a cylinder and a cone
void AddVertsForArrow3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, float arrowSize, Rgba8 const& color = Rgba8::WHITE, int numSlices = 8 );
//----------------------------------
// Add vertexes for a line segment which is actually a cylinder
void AddVertsForLineSegment3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float thickness, Rgba8 const& color, int numSlices = 8 );
//----------------------------------
// Add vertexes for a OBB3D 
void AddVertsForOBB3D( std::vector<Vertex_PCU>& verts, OBB3 const& obb3, Rgba8 const& color, AABB2 const& uvs = AABB2::IDENTITY );
//----------------------------------
// Add vertexes for wired OBB3D
void AddVertsForWiredOBB3D( std::vector<Vertex_PCU>& verts, OBB3 const& obb3, Rgba8 const& color );
//----------------------------------
// 
void CalculateTangentSpaceBasisVectors( std::vector<Vertex_PCUTBN>& vertexes, std::vector<unsigned int>& indexes, bool computeNormals, bool computeTangents );