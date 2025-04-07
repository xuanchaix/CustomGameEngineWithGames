#pragma once
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"
class Game;
class SpriteAnimDefinition;

constexpr float STRAT_BUTTON_TIME = 0.8f;


class App {

public:
	bool m_isPaused = false;
	int m_framePerSecond;

public:
	App();
	~App();
	void Startup( char const* commandLine );
	void Shutdown();
	void Run();
	void RunFrame();

	bool IsDebugMode() const { return m_debugMode; }

	void ToAttractMode();
	SoundID GetSoundId( AudioName name ) const;
	bool PlaySound( AudioName name, float intervalTimeSeconds = 0.f, bool isLooped = false, float volume = 1.f, float balance = 0.f, float speed = 1.f );
	SpriteAnimDefinition* GetAnimation( AnimationName name );

private:
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();

	void HandleKey();

	void RenderAttractMode() const;

	void SetUpAudio();
	void SetUpTexture();
	void SetUpBlackBoard();
	static bool SetQuitting( EventArgs& args );

private:
	Camera* m_attractModeCamera;
	bool m_debugMode = false;

	bool m_isQuitting3 = false;
	bool m_isQuitting2 = false;
	bool m_isQuitting = false;
	bool m_toAttractMode = false;
	bool m_attractMode = false;
	float m_startButtonA = -STRAT_BUTTON_TIME;

	double m_lastSoundPlaySecondsByID[(int)AudioName::NUM] = {};
	SoundID m_audioDictionary[(int)AudioName::NUM] = {};

	SoundPlaybackID m_attractModeMusic = (unsigned int)-1;
	SpriteAnimDefinition* m_animDictionary[(int)AnimationName::NUM] = {};
};
