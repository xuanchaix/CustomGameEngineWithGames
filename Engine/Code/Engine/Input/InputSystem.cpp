#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Renderer/Window.hpp"

extern EventSystem* g_theEventSystem;
extern InputSystem* g_theInput;

const unsigned char KEYCODE_F1 = VK_F1;
const unsigned char KEYCODE_F2 = VK_F2;
const unsigned char KEYCODE_F3 = VK_F3;
const unsigned char KEYCODE_F4 = VK_F4;
const unsigned char KEYCODE_F5 = VK_F5;
const unsigned char KEYCODE_F6 = VK_F6;
const unsigned char KEYCODE_F7 = VK_F7;
const unsigned char KEYCODE_F8 = VK_F8;
const unsigned char KEYCODE_F9 = VK_F9;
const unsigned char KEYCODE_F10 = VK_F10;
const unsigned char KEYCODE_F11 = VK_F11;
const unsigned char KEYCODE_F12 = VK_F12;

const unsigned char KEYCODE_ESC = VK_ESCAPE;
const unsigned char KEYCODE_SPACE = VK_SPACE;
const unsigned char KEYCODE_ENTER = VK_RETURN;

const unsigned char KEYCODE_UPARROW = VK_UP;
const unsigned char KEYCODE_DOWNARROW = VK_DOWN;
const unsigned char KEYCODE_LEFTARROW = VK_LEFT;
const unsigned char KEYCODE_RIGHTARROW = VK_RIGHT;
const unsigned char KEYCODE_LEFTMOUSE = VK_LBUTTON;
const unsigned char KEYCODE_RIGHTMOUSE = VK_RBUTTON;
const unsigned char KEYCODE_TILDE = 0xC0;
const unsigned char KEYCODE_LEFTBRACKET = 0xDB;
const unsigned char KEYCODE_RIGHTBRACKET = 0xDD;
const unsigned char KEYCODE_BACKSPACE = VK_BACK;
const unsigned char KEYCODE_INSERT = VK_INSERT;
const unsigned char KEYCODE_DELETE = VK_DELETE;
const unsigned char KEYCODE_HOME = VK_HOME;
const unsigned char KEYCODE_END = VK_END;

InputSystem::InputSystem( InputSystemConfig const& iConfig )
{
	m_config = iConfig;
	for (int i = 0; i < NUM_XBOX_CONTROLLERS; i++) {
		m_controllers[i].SetId( i );
	}
}

InputSystem::~InputSystem()
{

}

void InputSystem::StartUp()
{
	g_theEventSystem->SubscribeEventCallbackFunction( "KeyPressed", Event_KeyPressed );
	g_theEventSystem->SubscribeEventCallbackFunction( "KeyReleased", Event_KeyReleased );
	g_theEventSystem->SubscribeEventCallbackFunction( "MouseWheelInput", Event_MouseWheelInput );
	g_theEventSystem->SubscribeEventCallbackFunction( "SetCursorMode", Event_SetCursorMode );
	g_theEventSystem->SubscribeEventCallbackFunction( "GetCursorMode", Event_GetCursorMode );
}

void InputSystem::ShutDown()
{

}

void InputSystem::BeginFrame()
{
	for (int i = 0; i < NUM_XBOX_CONTROLLERS; i++) {
		m_controllers[i].Update();
	}
	
	if (m_cursorState.m_hiddenMode) {
		while (::ShowCursor( 0 ) >= 0);
	}
	else {
		while (::ShowCursor( 1 ) < 0);
	}
	HWND wh = ::GetActiveWindow();
	if (wh && !m_skipUpdateCursorRelativeNextFrame) {
		POINT cursorCoords;
		RECT clientRect;
		::GetCursorPos( &cursorCoords );
		::ScreenToClient( (HWND)wh, &cursorCoords );
		::GetClientRect( (HWND)wh, &clientRect );
		float cursorX = (float)cursorCoords.x / (float)clientRect.right;
		float cursorY = (float)cursorCoords.y / (float)clientRect.bottom;
		Vec2 normalizedCursorPosition = Vec2( cursorX, 1.f - cursorY );
		if (m_cursorState.m_relativeMode) {
			m_cursorState.m_cursorClientDelta = normalizedCursorPosition - m_cursorState.m_cursorClientPosition;
			cursorCoords.x = clientRect.right / 2;
			cursorCoords.y = clientRect.bottom / 2;
			::ClientToScreen( (HWND)wh, &cursorCoords );
			::SetCursorPos( cursorCoords.x, cursorCoords.y );
			::GetCursorPos( &cursorCoords );
			::ScreenToClient( (HWND)wh, &cursorCoords );
			cursorX = (float)cursorCoords.x / (float)clientRect.right;
			cursorY = (float)cursorCoords.y / (float)clientRect.bottom;
			normalizedCursorPosition = Vec2( cursorX, 1.f - cursorY );
			m_cursorState.m_cursorClientPosition = normalizedCursorPosition;
		}
		else {
			m_cursorState.m_cursorClientDelta = Vec2( 0, 0 );
			m_cursorState.m_cursorClientPosition = normalizedCursorPosition;
		}
	}
	else if (wh && m_cursorState.m_relativeMode && m_skipUpdateCursorRelativeNextFrame) {
		m_skipUpdateCursorRelativeNextFrame = false;
		POINT cursorCoords;
		RECT clientRect;
		::GetCursorPos( &cursorCoords );
		::ScreenToClient( (HWND)wh, &cursorCoords );
		::GetClientRect( (HWND)wh, &clientRect );
		float cursorX = (float)cursorCoords.x / (float)clientRect.right;
		float cursorY = (float)cursorCoords.y / (float)clientRect.bottom;
		Vec2 normalizedCursorPosition = Vec2( cursorX, 1.f - cursorY );
		m_cursorState.m_cursorClientDelta = Vec2( 0, 0 );
		m_cursorState.m_cursorClientPosition = normalizedCursorPosition;
	}
	else if (m_skipUpdateCursorRelativeNextFrame && !m_cursorState.m_relativeMode) {
		m_skipUpdateCursorRelativeNextFrame = false;
	}
}

void InputSystem::EndFrame()
{
	for (int i = 0; i < NUM_XBOX_CONTROLLERS; i++) {
		if (m_controllers[i].IsConnected()) {
			m_controllers[i].EndFrame();
		}
	}
	for (int i = 0; i < NUM_KEYCODES; i++) {
		m_keyStates[i].m_previousStste = m_keyStates[i].m_currentState;
	}
}

bool InputSystem::WasKeyJustPressed( unsigned char KeyCode ) const
{
	return !m_keyStates[KeyCode].m_previousStste && m_keyStates[KeyCode].m_currentState;
}

bool InputSystem::WasKeyJustReleased( unsigned char KeyCode ) const
{
	return m_keyStates[KeyCode].m_previousStste && !m_keyStates[KeyCode].m_currentState;
}

bool InputSystem::IsKeyDown( unsigned char KeyCode ) const
{
	return m_keyStates[KeyCode].m_currentState;
}

bool InputSystem::HandleKeyPressed( unsigned char KeyCode )
{
	if (!m_keyStates[KeyCode].m_currentState) {
		m_keyStates[KeyCode].m_currentState = true;
		return true;
	}
	return false;
}

bool InputSystem::HandleKeyReleased( unsigned char KeyCode )
{
	if (m_keyStates[KeyCode].m_currentState) {
		m_keyStates[KeyCode].m_currentState = false;
		return true;
	}
	return false;
}

int InputSystem::GetMouseWheelInput() const
{
	return m_mouseWheelInput;
}

void InputSystem::ConsumeMouseWheelInput()
{
	m_mouseWheelInput = 0;
}

void InputSystem::SetCursorMode( bool hiddenMode, bool relativeMode )
{
	m_cursorState.m_hiddenMode = hiddenMode;
	m_cursorState.m_relativeMode = relativeMode;
}

Vec2 const InputSystem::GetCursorClientDelta() const
{
	if (m_cursorState.m_relativeMode) {
		return m_cursorState.m_cursorClientDelta;
	}
	return Vec2( 0.f, 0.f );
}

Vec2 const InputSystem::GetCursorNormalizedPosition() const
{
	return m_cursorState.m_cursorClientPosition;
}

XboxController& InputSystem::GetController( int controllerId )
{
	GUARANTEE_OR_DIE( controllerId >= 0 && controllerId < 4, "controller id wrong!" );
	return m_controllers[controllerId];
}


bool InputSystem::Event_KeyPressed( EventArgs& args )
{
	if (!g_theInput) {
		return false;
	}
	unsigned char keyCode = (unsigned char)args.GetValue( "KeyCode", (unsigned char)255 );
	g_theInput->HandleKeyPressed( keyCode );
	return true;
}

bool InputSystem::Event_KeyReleased( EventArgs& args )
{
	if (!g_theInput) {
		return false;
	}
	unsigned char keyCode = (unsigned char)args.GetValue( "KeyCode", (unsigned char)255 );
	g_theInput->HandleKeyReleased( keyCode );
	return true;
}

bool InputSystem::Event_MouseWheelInput( EventArgs& args )
{
	if (!g_theInput) {
		return false;
	}
	g_theInput->m_mouseWheelInput = (short)args.GetValue( "MouseWheelValue", (short)0 );
	return true;
}

bool InputSystem::Event_SetCursorMode( EventArgs& args )
{
	if (!g_theInput) {
		return false;
	}
	bool setHiddenMode = args.GetValue( "HiddenMode", false );
	bool setRelativeMode = args.GetValue( "RelativeMode", false );
	g_theInput->SetCursorMode( setHiddenMode, setRelativeMode );
	if (setHiddenMode) {
		while (::ShowCursor( 0 ) >= 0);
	}
	else {
		while (::ShowCursor( 1 ) < 0);
	}
	g_theInput->m_skipUpdateCursorRelativeNextFrame = true;
	return true;
}


bool InputSystem::Event_GetCursorMode( EventArgs& args )
{
	if (!g_theInput) {
		return false;
	}
	if (g_theInput->m_cursorState.m_hiddenMode) {
		args.SetValue( "HiddenMode", true );
	}
	else {
		args.SetValue( "HiddenMode", false);
	}
	if (g_theInput->m_cursorState.m_relativeMode) {
		args.SetValue( "RelativeMode", true );
	}
	else {
		args.SetValue( "RelativeMode", false );
	}

	return true;
}