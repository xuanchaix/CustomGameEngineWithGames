#pragma once
//-----------------10.13.2023 class Window -------------------------
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <string>
class InputSystem;

//struct HDC__;
//typedef struct HDC__* HDC;

// Config for Window class
struct WindowConfig {
	std::string m_windowTitle = "Untitled";
	//InputSystem* m_inputSystem = nullptr;
	float m_clientAspect = 2.0f;
	bool m_isFullScreen = false;
	IntVec2 m_windowSize = IntVec2( -1, -1 );
	IntVec2 m_windowPosition = IntVec2( -1, -1 );
};

bool IsMouseWheelPresent();

class Window {
public:
	Window(WindowConfig const& wConfig);
	~Window();

	void StartUp();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	WindowConfig const& GetConfig() const;
	float GetAspect() const;
	/// <summary>
	/// Get HWND
	/// </summary>
	/// <returns></returns>
	void* GetWindowHandle() const;
	/// <summary>
	/// Get HDC
	/// </summary>
	/// <returns></returns>
	void* GetDisplayContext() const;
	/// <summary>
	/// Get Window instance
	/// </summary>
	/// <returns></returns>
	static Window* GetWindowContext();

	IntVec2 GetClientDimensions() const;

	Vec2 const GetNormalizedCursorPos() const;

	bool IsFocus() const;

	void SetFullScreen( bool toFullScreen );

	bool ChooseFileFromBroser( std::string& filePath );

protected:
	void CreateOSWindow();
	void RunMessagePump();

	WindowConfig m_config;
	static Window* s_mainWindow;
	void*/*HWND*/ m_windowHandle = nullptr;
	void*/*HDC*/ m_displayContext = nullptr;
};
