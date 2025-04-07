#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Engine/Renderer/Camera.hpp"
#include <vector>

class Entity;
class Renderer;
class Map;

enum class GameState {PLAYING, PAUSED};

class Game {
public:
	RandomNumberGenerator* m_randNumGen;

public:
	Game();
	~Game();

	void Startup();
	void Update( float deltaTime );
	void Render() const;
	void GoToNextMap();
	void GoToPreviousMap();
	Map* GetCurrentMap() const;
	int& GetNumOfReinforcements();
private:
	void LoadMapDefinitions();
	void HandleKey();
	void CallBackGoToNextMap();
	MapDefinition const& GetMapDef( std::string const& name ) const;
private:
	std::vector<Map*> m_maps;
	std::vector<MapDefinition> m_mapDefinitions;
	GameState m_curGameState = GameState::PLAYING;
	Map* m_curMap = nullptr;
	int m_curMapIndex = 0;
	int m_numOfReinforcements = 0;
	SoundPlaybackID m_gameMusic;
};



