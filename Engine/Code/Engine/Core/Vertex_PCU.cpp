#include "Engine/Core/Vertex_PCU.hpp"


Vertex_PCU::Vertex_PCU(){}

Vertex_PCU::Vertex_PCU(Vec3 const& position, Rgba8 const& color, Vec2 const& uvTexCoords) 
	:m_position(position)
	,m_color(color)
	,m_uvTexCoords(uvTexCoords)
{
}

Vertex_PCU::Vertex_PCU( Vec2 const& position, Rgba8 const& color )
	:m_color( color )
	,m_position( Vec3( position.x, position.y, 0.f ) )
	,m_uvTexCoords( Vec2( 0.f, 0.f ) )
{
}

Vertex_PCU::Vertex_PCU( Vec2 const& position, Rgba8 const& color, Vec2 const& uvTexCoords )
	:m_color( color )
	,m_uvTexCoords( uvTexCoords )
	,m_position( Vec3( position.x, position.y, 0.f ) )
{
}

Vertex_PCU::Vertex_PCU( Vec3 const& position, Rgba8 const& color )
	:m_position( position )
	, m_color( color )
	, m_uvTexCoords( Vec2(0.f, 0.f) )
{

}

Vertex_PCU::~Vertex_PCU(){}

Vertex_PCUTBN::Vertex_PCUTBN()
{

}

Vertex_PCUTBN::Vertex_PCUTBN( Vec3 const& position, Rgba8 const& color, Vec2 const& uvTexCoords, Vec3 const normal /*= Vec3()*/, Vec3 const tangent /*= Vec3()*/, Vec3 const bitangent /*= Vec3() */ )
	:m_position(position)
	,m_color(color)
	,m_uvTexCoords(uvTexCoords)
	,m_normal(normal)
	,m_tangent(tangent)
	,m_bitangent(bitangent)
{
}

Vertex_PCUTBN::~Vertex_PCUTBN()
{

}
