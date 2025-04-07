#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"

class Entity;
class Renderer;
class Clock;
class Player;
class Prop;

class Game {
public:
	RandomNumberGenerator* m_randNumGen = nullptr;
	Clock* m_gameClock = nullptr;
	Camera m_screenCamera;

public:
	Game();
	~Game();

	void Startup();
	void Update();
	void Render() const;

	void HandleKeys() const;
	void AddEntityToEntityArries( Entity* entityToAdd );

private:
	void UpdateEntityArrays();
	void UpdateEntityArray( EntityList& entityArray );
	void UpdateDebugRender();

	void SetupTestProps();
	void SetupGrids( int minX = -50, int maxX = 50, int minY = -50, int maxY = 50 );

private:
	EntityList m_entityArray;
	Player* m_player;
	Timer* m_timer;

	std::vector<Vertex_PCU> m_gridVerts;
};



