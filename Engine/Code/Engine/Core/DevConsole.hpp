#pragma once
#include <string>
#include <vector>
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EventSystem.hpp"
#include <mutex>

class Renderer;
struct AABB2;
class BitmapFont;
class Timer;
class Camera;

enum class DevConsoleMode{
	HIDDEN, EMERGE
};

struct DevConsoleConfig {
	Renderer* m_defaultRenderer = nullptr;
	Camera* m_camera = nullptr;
	BitmapFont* m_defaultFont = nullptr;
	float m_fontAspect = 0.618f;
	float m_maxRenderLines = 50.5f;
	bool m_startOpen = false;
	//int m_maxSavedLines = 100000;
};

class DevConsole {
public:
	DevConsole( DevConsoleConfig const& config );
	~DevConsole();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void Execute( std::string const& consoleCommandText, bool isRemote = false );
	void AddLine( Rgba8 const& color, std::string const& text );
	void Render( AABB2 const& bounds, Renderer* rendererOverride = nullptr ) const;

	DevConsoleMode GetMode() const;
	void SetMode( DevConsoleMode mode );
	void ToggleMode( DevConsoleMode mode );

	void ExecuteXmlCommandScriptNode( XmlElement const& commandScriptXmlElement );
	void ExecuteXmlCommandScriptFile( std::string const& commandScriptXmlFilePathName );

	static Rgba8 const INFO_ERROR;
	static Rgba8 const INFO_WARNING;
	static Rgba8 const INFO_MAJOR;
	static Rgba8 const INFO_MINOR;
	static Rgba8 const INFO_COMMAND_ECHO;
	static Rgba8 const INFO_COMMAND_REMOTE_ECHO;
	static Rgba8 const INFO_INPUT_TEXT;
	static Rgba8 const INFO_INSERTION_POINT;

protected:

	struct DevConsoleLine {
		DevConsoleLine( std::string const& text, Rgba8 const& color, int frameNum, bool isInstruction );
		Rgba8 m_color;
		std::string m_text;
		int m_frameNum;
		double m_secondsFromBegin;
		bool m_isInstruction;
	};

	void Render_OpenFull( AABB2 const& bounds, Renderer& renderer, BitmapFont& font, float fontAspect = 1.f ) const;
	
	static bool Event_KeyPressed( EventArgs& args );
	static bool Event_CharInput( EventArgs& args );
	static bool Command_Clear( EventArgs& args );
	static bool Command_Echo( EventArgs& args );
	static bool Command_Help( EventArgs& args );
	static bool Command_RemoteCommand( EventArgs& args );
	static bool Command_BurstTest( EventArgs& args );
protected:
	DevConsoleConfig m_config;
	DevConsoleMode m_mode = DevConsoleMode::HIDDEN;
	std::vector<DevConsoleLine> m_lines;
	int m_frameNumber = 0;
	std::string m_inputText;

	int m_insertionPointPosition = 0;
	bool m_insertionPointVisible = true;
	Timer* m_insertionPointBlinkTimer;
	std::vector<std::string> m_commandHistory;
	int m_historyIndex = 0;

	mutable std::mutex m_mutex;
};