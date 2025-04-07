#include "Game/Tile.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"

std::vector<TileDefinition> TileDefinition::s_definitions;

TileDefinition::TileDefinition( XmlElement* elem )
{
	m_symbol = ParseXmlAttribute( *elem, "symbol", m_symbol );
	m_name = ParseXmlAttribute( *elem, "name", m_name );
	m_isBlocked = ParseXmlAttribute( *elem, "isBlocked", m_isBlocked );
}

TileDefinition const& TileDefinition::GetDefinition( std::string const& defName )
{
	for (auto const& def : s_definitions) {
		if (def.m_name == defName) {
			return def;
		}
	}
	ERROR_AND_DIE( Stringf( "Error! No such map definition named %s", defName.c_str() ) );
}

TileDefinition const& TileDefinition::GetDefinition( char symbol )
{
	for (auto const& def : s_definitions) {
		if (def.m_symbol == symbol) {
			return def;
		}
	}
	ERROR_AND_DIE( Stringf( "Error! No such tile definition named %c", symbol ) );
}

Tile::Tile( IntVec2 const& coords,  TileDefinition const& def )
	:m_coords(coords)
	,m_def(def)
{
	m_center = Vec2( (float)coords.x * 0.86602540378f, (float)coords.y + (float)coords.x * 0.5f );
	float innerRadius = 0.5f;
	float halfEdge = 0.57735026919f * innerRadius;
	float outerRadius = halfEdge * 2.f;
	Vec2 p1 = Vec2( m_center.x - halfEdge, m_center.y + innerRadius );
	Vec2 p2 = Vec2( m_center.x + halfEdge, m_center.y + innerRadius );
	Vec2 p3 = Vec2( m_center.x + outerRadius, m_center.y );
	Vec2 p4 = Vec2( m_center.x + halfEdge, m_center.y - innerRadius );
	Vec2 p5 = Vec2( m_center.x - halfEdge, m_center.y - innerRadius );
	Vec2 p6 = Vec2( m_center.x - outerRadius, m_center.y );
	m_vertexes.push_back( p1 );
	m_vertexes.push_back( p2 );
	m_vertexes.push_back( p3 );
	m_vertexes.push_back( p4 );
	m_vertexes.push_back( p5 );
	m_vertexes.push_back( p6 );
	m_map = g_theGame->m_map;
}

void Tile::InitializeNeighbors()
{
	auto iter = m_map->m_tiles.find( m_coords + IntVec2( 1, 0 ) );
	if (iter != m_map->m_tiles.end()) {
		m_plusX = iter->second;
	}
	iter = m_map->m_tiles.find( m_coords + IntVec2( -1, 0 ) );
	if (iter != m_map->m_tiles.end()) {
		m_minusX = iter->second;
	}
	iter = m_map->m_tiles.find( m_coords + IntVec2( 0, 1 ) );
	if (iter != m_map->m_tiles.end()) {
		m_plusY = iter->second;
	}
	iter = m_map->m_tiles.find( m_coords + IntVec2( 0, -1 ) );
	if (iter != m_map->m_tiles.end()) {
		m_minusY = iter->second;
	}
	iter = m_map->m_tiles.find( m_coords + IntVec2( 1, -1 ) );
	if (iter != m_map->m_tiles.end()) {
		m_plusZ = iter->second;
	}
	iter = m_map->m_tiles.find( m_coords + IntVec2( -1, 1 ) );
	if (iter != m_map->m_tiles.end()) {
		m_minusZ = iter->second;
	}
}

bool Tile::IsPointInsideHexagon( Vec2 const& pos )
{
	double pointX = pos.x;
	double pointY = pos.y;
	int count = 0;
	for (int i = 0; i < (int)m_vertexes.size(); i++) {
		double p1x = m_vertexes[i].x;
		double p1y = m_vertexes[i].y;
		double p2x = m_vertexes[(i + 1) % (int)m_vertexes.size()].x;
		double p2y = m_vertexes[(i + 1) % (int)m_vertexes.size()].y;
		if ((pointY < p1y != pointY < p2y) && (pointX < p1x + (pointY - p1y) / (p2y - p1y) * (p2x - p1x))) {
			++count;
		}
	}
	return count % 2 == 1;
}

Vec3 Tile::GetCenter() const
{
	return m_center;
}

Vec3 Tile::GetUnitPosition() const
{
	return m_center;
}

void Tile::GetAllNeighbors( std::vector<Tile*>& out_neighbors ) const
{
	if (m_plusX) {
		out_neighbors.push_back( m_plusX );
	}
	if (m_plusY) {
		out_neighbors.push_back( m_plusY );
	}
	if (m_plusZ) {
		out_neighbors.push_back( m_plusZ );
	}
	if (m_minusX) {
		out_neighbors.push_back( m_minusX );
	}
	if (m_minusY) {
		out_neighbors.push_back( m_minusY );
	}
	if (m_minusZ) {
		out_neighbors.push_back( m_minusZ );
	}
}

UnitDirection Tile::GetDirectionToTile( Tile* tile ) const
{
	if (tile == m_minusX) {
		return UnitDirection::MinusX;
	}
	if (tile == m_plusX) {
		return UnitDirection::PlusX;
	}
	if (tile == m_minusY) {
		return UnitDirection::MinusY;
	}
	if (tile == m_plusY) {
		return UnitDirection::PlusY;
	}
	if (tile == m_minusZ) {
		return UnitDirection::MinusZ;
	}
	if (tile == m_plusZ) {
		return UnitDirection::PlusZ;
	}
	return UnitDirection::None;
}

