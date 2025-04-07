#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

//-----------------------------------------------------------
// vertex PCU means a point's color information and uvs
// contains a 3D position, a rgba8 color and a uvTexCoordinates
// size is 24 bytes
struct Vertex_PCU 
{
public:
	Vec3 m_position;
	Rgba8 m_color;
	Vec2 m_uvTexCoords;

	Vertex_PCU();
	explicit Vertex_PCU ( Vec3 const& position, Rgba8 const& color, Vec2 const& uvTexCoords );
	explicit Vertex_PCU ( Vec2 const& position, Rgba8 const& color );
	explicit Vertex_PCU ( Vec3 const& position, Rgba8 const& color );
	explicit Vertex_PCU ( Vec2 const& position, Rgba8 const& color, Vec2 const& uvTexCoords );
	~Vertex_PCU();
};


struct Vertex_PCUTBN
{
public:
	Vec3 m_position;
	Rgba8 m_color;
	Vec2 m_uvTexCoords;
	Vec3 m_tangent;
	Vec3 m_bitangent;
	Vec3 m_normal;

	Vertex_PCUTBN();
	explicit Vertex_PCUTBN( Vec3 const& position, Rgba8 const& color, Vec2 const& uvTexCoords, Vec3 const normal = Vec3(), Vec3 const tangent = Vec3(), Vec3 const bitangent = Vec3() );
	~Vertex_PCUTBN();
};