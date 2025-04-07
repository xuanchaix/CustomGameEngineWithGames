#pragma once
#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/Vec2.hpp"

extern const unsigned char KEYCODE_F1;
extern const unsigned char KEYCODE_F2;
extern const unsigned char KEYCODE_F3;
extern const unsigned char KEYCODE_F4;
extern const unsigned char KEYCODE_F5;
extern const unsigned char KEYCODE_F6;
extern const unsigned char KEYCODE_F7;
extern const unsigned char KEYCODE_F8;
extern const unsigned char KEYCODE_F9;
extern const unsigned char KEYCODE_F10;
extern const unsigned char KEYCODE_F11;
extern const unsigned char KEYCODE_F12;

extern const unsigned char KEYCODE_ESC;
extern const unsigned char KEYCODE_SPACE;
extern const unsigned char KEYCODE_ENTER;

extern const unsigned char KEYCODE_UPARROW;
extern const unsigned char KEYCODE_DOWNARROW;
extern const unsigned char KEYCODE_LEFTARROW;
extern const unsigned char KEYCODE_RIGHTARROW;
extern const unsigned char KEYCODE_LEFTMOUSE;
extern const unsigned char KEYCODE_RIGHTMOUSE;
extern const unsigned char KEYCODE_TILDE;
extern const unsigned char KEYCODE_LEFTBRACKET;
extern const unsigned char KEYCODE_RIGHTBRACKET;
extern const unsigned char KEYCODE_BACKSPACE;
extern const unsigned char KEYCODE_INSERT;
extern const unsigned char KEYCODE_DELETE;
extern const unsigned char KEYCODE_HOME;
extern const unsigned char KEYCODE_END;

constexpr int NUM_KEYCODES = 256;
constexpr int NUM_XBOX_CONTROLLERS = 4;

class Window;

struct CursorState
{
	Vec2 m_cursorClientDelta = Vec2( 0.5f, 0.5f );
	Vec2 m_cursorClientPosition;

	bool m_hiddenMode = false;
	bool m_relativeMode = false;
};

struct InputSystemConfig {
	Window* m_curWindow = nullptr;
};

class InputSystem {

public:
	InputSystem(InputSystemConfig const& iConfig);
	~InputSystem();
	void StartUp();
	void ShutDown();
	void BeginFrame();
	void EndFrame();
	bool WasKeyJustPressed( unsigned char KeyCode ) const;
	bool WasKeyJustReleased( unsigned char KeyCode ) const;
	bool IsKeyDown( unsigned char KeyCode ) const;
	bool HandleKeyPressed( unsigned char KeyCode );
	bool HandleKeyReleased( unsigned char KeyCode );
	int GetMouseWheelInput() const;
	void ConsumeMouseWheelInput();

	void SetCursorMode( bool hiddenMode, bool relativeMode );
	Vec2 const GetCursorClientDelta() const;
	Vec2 const GetCursorNormalizedPosition() const;

	static bool Event_KeyPressed( EventArgs& args );
	static bool Event_KeyReleased( EventArgs& args );
	static bool Event_MouseWheelInput( EventArgs& args );
	static bool Event_SetCursorMode( EventArgs& args );
	static bool Event_GetCursorMode( EventArgs& args );

	XboxController& GetController( int controllerId );

protected:
	InputSystemConfig m_config;
	KeyButtonState m_keyStates[NUM_KEYCODES];
	XboxController m_controllers[NUM_XBOX_CONTROLLERS];
	short m_mouseWheelInput;
	CursorState m_cursorState;
	bool m_skipUpdateCursorRelativeNextFrame = true;
};