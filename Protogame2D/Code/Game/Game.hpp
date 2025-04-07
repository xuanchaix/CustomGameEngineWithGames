#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"

class Entity;
class Renderer;
class Clock;


class Game {
public:
	RandomNumberGenerator* m_randNumGen = nullptr;
	Clock* m_gameClock = nullptr;
	Camera m_worldCamera;
	Camera m_screenCamera;

public:
	Game();
	~Game();

	void Startup();
	void Update();
	void Render() const;

	void HandleKeys();

private:
	EntityList m_entityArray;

	std::deque<Job*> m_jobList;
	int m_jobListFirst = 0;
};



