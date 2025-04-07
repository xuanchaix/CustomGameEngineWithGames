#pragma once
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"
class Game;
class SpriteAnimDefinition;

class App {
public:
	std::map<std::string, double> m_lastSoundPlaySecondsByID;
	std::map<std::string, SoundID> m_audioDict;

public:
	App();
	~App();
	void Startup();
	void Shutdown();
	void Run();
	void RunFrame();

	SoundID GetSoundId( std::string const& name ) const;
	SoundPlaybackID PlaySound( std::string const& name, float intervalTimeSeconds = 0.f, bool isLooped = false, float volume = 1.f, float balance = 0.f, float speed = 1.f );
	
	SpriteAnimDefinition* GetAnimation( AnimationName name );

private:
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();

	void HandleKey();

	void SetUpTexture();
	void SetUpBlackBoard();
	static bool SetQuitting( EventArgs& args );

private:
	Camera* m_devConsoleCamera;
	bool m_isQuitting = false;

	SpriteAnimDefinition* m_animDictionary[(int)AnimationName::NUM] = {};
};
