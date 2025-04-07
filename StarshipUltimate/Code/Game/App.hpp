#pragma once
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"
class Game;
class SpriteAnimDefinition;
class SettingsScreen;

enum class AppState {ATTRACT_MODE, PLAY_MODE, PAUSE_MODE, SETTINGS_MODE};

class App {

public:
	AppState m_appState = AppState::ATTRACT_MODE;
	AppState m_appStateNextFrame = AppState::ATTRACT_MODE;
	bool m_isPaused = false;
	int m_framePerSecond;

	float m_musicVolume = 1.f;
	float m_soundVolume = 1.f;
	SettingsScreen* m_settingsScreen = nullptr;

	bool m_autoShootMainWeapon = false;
	bool m_autoShootSubWeapon = false;
	bool m_playerNoDie = false;
	bool m_fullScreen = false;
	bool m_isQuitting = false;
	SoundPlaybackID m_attractModeMusic = (SoundPlaybackID)-1;

	bool m_bossRushFlag = false;
public:
	App();
	~App();
	void Startup();
	void Shutdown();
	void Run();
	void RunFrame();

	void GoToAppMode( AppState modeToGo );
	SoundID GetSoundId( AudioName name ) const;
	bool PlaySound( AudioName name, float intervalTimeSeconds = 0.f, bool isLooped = false, float volume = 1.f, float balance = 0.f, float speed = 1.f );
	SpriteAnimDefinition* GetAnimation( AnimationName name );

private:
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();

	void LoadDefinitions();
	void UpdateAppState();
	void UpdateAttractMode( float deltaSeconds );
	void UpdateGameMode();

	void RenderAttractMode() const;
	void RenderGameMode() const;

	void SetUpAudio();
	void SetUpTexture();
	void SetUpBlackBoard();
	static bool SetQuitting( EventArgs& args );

	void SaveGameSettings();
	void LoadGameSettings();
private:
	Camera* m_attractModeCamera;

	double m_lastSoundPlaySecondsByID[(int)AudioName::NUM] = {};
	SoundID m_audioDictionary[(int)AudioName::NUM] = {};

	Texture* attractTexture;
	SpriteAnimDefinition* m_animDictionary[(int)AnimationName::NUM] = {};

	int m_curHoveringButtom = 0;
	int m_numOfAttractButtons = 4;

};
