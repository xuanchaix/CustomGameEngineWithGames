#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"

class Entity;
class Renderer;
class Clock;
class Map;

class Game {
public:
	RandomNumberGenerator* m_randNumGen = nullptr;
	Clock* m_gameClock = nullptr;
	Camera m_worldCamera;
	Camera m_screenCamera;
	Map* m_map = nullptr;

public:
	Game();
	~Game();

	void Startup();
	void Update();
	void Render() const;

	void HandleKeys();

private:
	bool m_smallMapMode = false;
	bool m_wholeMapMode = false;
};



