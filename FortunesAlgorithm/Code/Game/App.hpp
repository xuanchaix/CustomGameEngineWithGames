#pragma once
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"
class Game;
class SpriteAnimDefinition;

constexpr float STRAT_BUTTON_TIME = 0.8f;

enum class AppState {};

class App {

public:
	bool m_isPaused = false;
	int m_framePerSecond;

public:
	App();
	~App();
	void Startup();
	void Shutdown();
	void Run();
	void RunFrame();

	bool IsDebugMode() const { return m_debugMode; }
	SpriteAnimDefinition* GetAnimation( AnimationName name );

private:
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();

	void HandleKey();

	void SetUpAudio();
	void SetUpTexture();
	void SetUpBlackBoard();
	static bool SetQuitting( EventArgs& args );

private:
	Camera* m_attractModeCamera;
	bool m_debugMode = false;

	bool m_isQuitting = false;

	SpriteAnimDefinition* m_animDictionary[(int)AnimationName::NUM] = {};
};
