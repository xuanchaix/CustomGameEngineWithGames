#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"

class Unit;
class Effect;

enum class GameState {
	ViewingMap,
	ClickOnUnitMovement, MovementAnimation,
	ClickOnUnitAttack, AttackAnimation,
	WaitingForTheOtherPlayer, EndTurn, StartTurn, Win
};

struct MapDefinition {
	MapDefinition( XmlElement* elem );
	std::string m_name;
	std::string m_overlayShader;
	IntVec2 m_gridSize;
	Vec3 m_worldBoundsMin;
	Vec3 m_worldBoundsMax;
	std::string m_tiles;
	std::string m_unitLayout1;
	std::string m_unitLayout2;

	static MapDefinition const& GetDefinition( std::string const& defName );

	static std::vector<MapDefinition> s_definitions;
};

bool operator < ( IntVec2 const& a, IntVec2 const& b );

class Map {
public:
	Map( MapDefinition const& def );
	virtual ~Map();
	virtual void StartUp();
	virtual void Update( float deltaSeconds );
	virtual void UpdateGameState();
	virtual void Render() const;
	virtual void RenderUI() const;
	void DeleteGarbage();
	Effect* SpawnEffect( EffectType type, Vec3 const& position = Vec3(), EulerAngles const& orientation = EulerAngles());
	int GetDistBetweenTwoTiles( Tile* t1, Tile* t2 ) const;
	void GetRouteBetweenTwoTiles( std::vector<Tile*>& out_route, Tile* t1, Tile* t2 ) const;

	Tile* GetTile( IntVec2 const& coords ) const;
	Tile* GetTile( int x, int y ) const;
	void HandleMouseInput();
	Tile* GetTileByPos( Vec2 const& pos ) const;
	void EndTurn();


	void PerformAttackAction();
	void EndCurUnitTurn();
	void SelectNextUnitInMovementMode();
	void SelectPrevUnitInMovementMode();
	void SelectNextUnitInViewingMode();
	void SelectPrevUnitInViewingMode();

	bool m_isOnlineNetworkingGame = false;
	bool m_player1Wins = false;
	GameState m_gameState = GameState::StartTurn;
	AABB3 m_cameraBounds;
	AABB2 m_mapBounds;
	MapDefinition const& m_def;
	std::map<IntVec2, Tile*> m_tiles;
	Tile* m_curHoveringTile = nullptr;
	VertexBuffer* m_tileHexVertexBuffer = nullptr;
	IndexBuffer* m_tileHexIndexBuffer = nullptr;
	VertexBuffer* m_currentHoveringHexBuffer = nullptr;
	IndexBuffer* m_currentHoveringHexIndexBuffer = nullptr;
	ConstantBuffer* m_lightConstantBuffer = nullptr;
	Material* m_moonSurfaceMat = nullptr;
	DirectionalLightConstants m_dlc;
	std::vector<Vertex_PCUTBN> m_tbnVerts;
	std::vector<unsigned int> m_tbnInts;

	int m_curPlayer = 1;
	int m_networkPlayer = 1;
	std::vector<Unit*> m_units;
	std::vector<Unit*> m_player1AvailableUnits;
	std::vector<Unit*> m_player2AvailableUnits;
	std::vector<Effect*> m_effects;
	Unit* m_selectingUnit = nullptr;
};

Map* GetCurMap();

bool Command_PlayerReady( EventArgs& args );
bool Command_StartTurn( EventArgs& args );
bool Command_SetFocusedHex( EventArgs& args );
bool Command_SelectFocusedUnit( EventArgs& args );
bool Command_SelectPreviousUnit( EventArgs& args );
bool Command_SelectNextUnit( EventArgs& args );
bool Command_Move( EventArgs& args );
bool Command_Stay( EventArgs& args );
bool Command_HoldFire( EventArgs& args );
bool Command_Attack( EventArgs& args );
bool Command_Cancel( EventArgs& args );
bool Command_EndTurn( EventArgs& args );
bool Command_PlayerQuit( EventArgs& args );