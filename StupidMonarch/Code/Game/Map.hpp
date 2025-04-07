#pragma once
#include <map>
#include "Game/GameCommon.hpp"

class Image;
class Game;
class Province;
class Force;
class Army;

enum class LinkType {
	Default,
	River,
	BigRiver,
};

enum class MapViewMode {
	VIEW_FORCE_MAP,
	VIEW_ECONOMY_MAP,
};

struct Link {
	Link( LinkType type, int id );
	LinkType m_type;
	int m_provIdToLink;
};

class Map {
	friend class Game;
	friend class StupidMonarchAI;
public:
	Map(Game* game);
	~Map();

	void StartUp( std::string const& mapPath );
	void Update( float deltaTime );
	void Render() const;

	void NextTurn();
	void ResolveCombat();
	bool IsAdjacent( Province* p1, Province* p2 ) const;
	bool IsTwoProvsConnected( Province* prov1, Province* prov2, Force* ownedForce ) const;

	void GetAllProvsArmyCanGo( std::vector<Province*>& provCanGo, Army const* army ) const;
	Province* GetProvinceByWorldPos( Vec2 const& worldPos ) const;
	LinkType GetProvLinkType( Province* provFrom, Province* provTo ) const;
	LinkType GetAdjProvLinkType( Province* provFrom, Province* provTo ) const;
	IntVec2 const& GetDimensions() const;
	Force* GetForceByNickName( std::string const& nickname ) const;
	void GetForceRank( std::vector<Force*>& forceRank ) const;

private:
	void LoadForces();
	void LoadProvinces( int startYear );
	void LoadHistory( int year );
	void LoadLinkInfo();
	void CreateVertexBuffer();

	IntVec2 GetMapPosFromWorldPos( Vec2 const& worldPos ) const;
	int GetProvinceIdFromMapPos( IntVec2 const& mapPos ) const;
	int GetProvIdFromColor( Rgba8 const& color ) const;
	Province* GetProvFromColor( Rgba8 const& color ) const;
public:
	IntVec2 m_dimensions;
	MapViewMode m_viewMode = MapViewMode::VIEW_FORCE_MAP;
	Image* m_mapImage = nullptr;
	Image* m_riverImage = nullptr;
	Game* m_game;
	std::vector<Province*> m_provinces;
	int m_curForceIndex = -1;
	std::vector<Force*> m_forcesAsOrder;
	std::map<Rgba8 const, int> m_mapFromColorToProvId;
	
	std::vector<std::vector<Link>> m_provLinkInfo;
	std::vector<Vertex_PCU> m_edgeLines;

	VertexBuffer* m_vertexBuffer;

	int m_startOfEdge;
	int m_numOfEdgeVerts;
	int m_startOfRiver;
	int m_numOfRiverVerts;
};