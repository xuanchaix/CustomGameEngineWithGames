#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Engine/Renderer/Camera.hpp"

class Entity;
class Renderer;
class Clock;
class PlayerController;
class AIController;
class Prop;
class GameMode;

enum class GameState {
	NONE,
	ATTRACT,
	PLAYING,
	LOBBY,
	VICTORY,
	COUNT
};


class Game {
public:
	RandomNumberGenerator* m_randNumGen = nullptr;
	Clock* m_gameClock = nullptr;
	Camera m_gameScreenCamera;
	Map* m_curMap;
	SoundPlaybackID m_gameMusic = (unsigned int)-1;
	SoundPlaybackID m_attractModeMusic = (unsigned int)-1;
	int m_numOfPlayers = 0;
	std::vector<PlayerController*> m_players;
	std::vector<AIController*> m_AIs;
public:
	Game();
	~Game();

	void Startup();
	void Update();
	void Render() const;

	void EnterState( GameState state );
	GameState GetState() const;
	PlayerController* GetPlayerController( int playerId ) const;
	AIController* CreateNewAIController( std::string const& aiBehavior );
	SoundPlaybackID PlaySound3D( std::string const& name, Vec3 const& position, ActorUID callingActor, bool addToUpdateList = false, float intervalTimeSeconds = 0.f, bool isLooped = false, float volume = 1.f, float balance = 0.f, float speed = 1.f );

	MapDefinition const& GetMapDefByName( std::string name ) const;
	void HandleKeys();
	bool IsMultiplayerPlaying() const;

private:
	void LoadDefinitions();
	void LoadMapDefs();
	void SetUpMaps();

	GameMode* CreateGameMode( Map* map, std::string const& gameModeName );

	void UpdateState();
	void UpdateDebugRender();

	void UpdateAttractMode();
	void UpdatePlayingMode();
	void UpdateLobbyState();
	void UpdateVictoryMode();
	void RenderAttractMode() const;
	void RenderPlayingMode() const;
	void RenderLobbyState() const;
	void RenderVictoryMode() const;

	void UpdateSounds();

private:
	//EntityList m_entityArray;
	Timer* m_timer;

	GameState m_currentState = GameState::ATTRACT;
	GameState m_nextFrameState = GameState::ATTRACT;

	std::vector<MapDefinition> m_mapDefs;
	std::vector<Map*> m_maps;
	std::map<SoundPlaybackID, ActorUID> m_3DSounds;

	float m_victoryTimer = 0.f;

	int m_mapChoose = 0;
	int m_numOfMaps = 0;
	Timer* m_mapChooseButtonJoystickCooldownTimer = nullptr;
};



