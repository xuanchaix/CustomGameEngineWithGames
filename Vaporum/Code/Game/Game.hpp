#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"

class Entity;
class Renderer;
class Clock;
class Player;
class Prop;
class Map;

enum class AppState {
	AttractScreen, InMenu, InGame, Pause,
};

class Game {
public:
	RandomNumberGenerator* m_randNumGen = nullptr;
	Clock* m_gameClock = nullptr;
	Camera m_screenCamera;
	Camera m_playerPOV;
	Map* m_map = nullptr;
	bool m_returnToMenu = false;

public:
	Game();
	~Game();

	void Startup();
	void Update();
	void Render() const;

	void HandleKeys();

	static bool Command_LoadMap( EventArgs& args );
private:
	void LoadDefinitions();
	void UpdateDebugRender();
	void SetUpCameras();

	void UpdateUI();
	void RenderUI() const;
private:
	Timer* m_timer;
	AppState m_state = AppState::AttractScreen;
	int m_menuButtonHoveringID = 0; // 1, 2, 3
};



