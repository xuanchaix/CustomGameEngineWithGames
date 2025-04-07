#pragma once
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"
class Game;
class Clock;

//-------------8.23.2023 class App--------------

constexpr float STRAT_BUTTON_TIME = 0.8f;

class App {

public:
	SoundID m_audioDictionary[(int)AudioName::NUM] = {};
	SoundPlaybackID m_gameModeMusic = (unsigned int)-1;
	SoundPlaybackID m_bossMusic = (unsigned int)-1;
public:
	App();
	~App();
	void Startup();
	void Shutdown();
	void Run();
	void RunFrame();

	bool IsQuitting() const { return m_isQuitting; }
	bool IsDebugMode() const { return m_debugMode; }

	void ToAttractMode();
	SoundID GetSoundId( AudioName name );

private:
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();

	void HandleKey();

	void RenderAttractMode() const;

	void SetUpAudio();

	static bool SetQuitting( EventArgs& args );
	
private:
	Camera m_attractModeCamera;
	bool m_isQuitting = false;
	bool m_debugMode = false;

	bool m_toAttractMode = false;
	bool m_attractMode = true;
	float m_startButtonA = -STRAT_BUTTON_TIME;

	SoundPlaybackID m_attractModeMusic = (unsigned int)-1;
};