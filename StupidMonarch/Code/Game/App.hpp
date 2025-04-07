#pragma once
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/Button.hpp"
#include "Game/GameCommon.hpp"
class Game;
class SpriteAnimDefinition;

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

	void ToAttractMode();
	SoundID GetSoundId( AudioName name ) const;
	bool PlaySound( AudioName name, float intervalTimeSeconds = 0.f, bool isLooped = false, float volume = 1.f, float balance = 0.f, float speed = 1.f );
	SpriteAnimDefinition* GetAnimation( AnimationName name );
	void LoadSaveToCurGame( std::string const& path );
	void CreateSave( std::string const& folder, std::string const& name );

private:
	void BeginFrame();
	void Update( float deltaSeconds );
	void Render() const;
	void EndFrame();

	void HandleKey();

	void UpdateAttractMode( float deltaSeconds );
	void RenderAttractMode() const;

	void SetUpAudio();
	void SetUpTexture();
	void SetUpBlackBoard();
	void SetUpLocalisations();

	static bool SetQuitting( EventArgs& args );
	static bool OnUIStartGameButtonClicked( EventArgs& args );
	static bool OnUILoadSavingsButtonsClicked( EventArgs& args );

private:
	Camera m_attractModeCamera;
	bool m_isQuitting = false;
	bool m_isSlowMo = false;
	bool m_isFastMo = false;
	bool m_pauseAfterUpdate = false;
	bool m_debugMode = false;

	bool m_toAttractMode = false;
	bool m_attractMode = true;

	double m_timeStart;
	double m_timeEnd;

	double m_lastSoundPlaySecondsByID[(int)AudioName::NUM] = {};
	SoundID m_audioDictionary[(int)AudioName::NUM] = {};

	Texture* attractTexture;
	SoundPlaybackID m_attractModeMusic = (unsigned int)-1;
	SpriteAnimDefinition* m_animDictionary[(int)AnimationName::NUM] = {};

	std::vector<RectButton> m_attractModeButtons;
};
