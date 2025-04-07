#include "Engine\Renderer\Window.hpp"
#include "Engine\Input\InputSystem.hpp"
#include "Engine\Core\EventSystem.hpp"
#include "Engine\Core\NamedStrings.hpp"
#include "Engine\Core\ErrorWarningAssert.hpp"
#include "Engine\Core\EngineCommon.hpp"

#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in VERY few places (and .CPPs only)
#include <commdlg.h>

#include "ThirdParty/ImGui/imgui.h"
#include "ThirdParty/ImGui/backends/imgui_impl_win32.h"

class DevConsole;
extern DevConsole* g_devConsole;

Window* Window::s_mainWindow = nullptr;

Window::Window( WindowConfig const& wConfig )
{
	s_mainWindow = this;
	m_config = wConfig;
}

Window::~Window()
{

}

void Window::StartUp()
{
	CreateOSWindow();
}

void Window::BeginFrame()
{
	RunMessagePump();
}

void Window::EndFrame()
{
	//Sleep( 1 );
}

void Window::Shutdown()
{

}

WindowConfig const& Window::GetConfig() const
{
	return m_config;
}

float Window::GetAspect() const
{
	RECT clientRect;
	GetWindowRect( (HWND)m_windowHandle, &clientRect );
	IntVec2 dimensions = GetClientDimensions();
	return (float)dimensions.x / (float)dimensions.y;
}

void* Window::GetWindowHandle() const
{
	return m_windowHandle;
}

void* Window::GetDisplayContext() const
{
	return (void*)m_displayContext;
}

Window* Window::GetWindowContext()
{
	return s_mainWindow;
}

IntVec2 Window::GetClientDimensions() const
{
	RECT clientRect;
	GetWindowRect( (HWND)m_windowHandle, &clientRect );
	return IntVec2( clientRect.right - clientRect.left, clientRect.bottom - clientRect.top );
}

Vec2 const Window::GetNormalizedCursorPos() const
{
	POINT cursorCoords;
	RECT clientRect;
	GetCursorPos( &cursorCoords );
	ScreenToClient( (HWND)m_windowHandle, &cursorCoords );
	GetClientRect( (HWND)m_windowHandle, &clientRect );
	float cursorX = (float)cursorCoords.x / (float)clientRect.right;
	float cursorY = (float)cursorCoords.y / (float)clientRect.bottom;
	return Vec2( cursorX, 1.f - cursorY );
}

bool Window::IsFocus() const
{
	return m_windowHandle == ::GetActiveWindow();
}

void Window::SetFullScreen( bool toFullScreen )
{
	if (toFullScreen) {
		const DWORD windowStyleFlags = WS_POPUP;
		const DWORD windowStyleExFlags = WS_EX_TOPMOST;
		// Get desktop rectangular, dimensions, aspect
		RECT desktopRect;
		HWND desktopWindowHandle = GetDesktopWindow();
		GetClientRect( desktopWindowHandle, &desktopRect );
		float desktopWidth = (float)(desktopRect.right - desktopRect.left);
		float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
		float desktopAspect = desktopWidth / desktopHeight;

		// Calculate maximum client size (as some % of desktop size)
		constexpr float maxClientFractionOfDesktop = 1.f;
		float clientWidth = desktopWidth * maxClientFractionOfDesktop;
		float clientHeight = desktopHeight * maxClientFractionOfDesktop;

		if (m_config.m_clientAspect > desktopAspect)
		{
			// Client window has a wider aspect than desktop; shrink client height to match its width
			clientHeight = clientWidth / m_config.m_clientAspect;
		}
		else
		{
			// Client window has a taller aspect than desktop; shrink client width to match its height
			clientWidth = clientHeight * m_config.m_clientAspect;
		}

		// Calculate client rectangular bounds by centering the client area
		float clientMarginX = 0.5f * (desktopWidth - clientWidth);
		float clientMarginY = 0.5f * (desktopHeight - clientHeight);
		RECT clientRect;
		clientRect.left = (int)clientMarginX;
		clientRect.right = clientRect.left + (int)clientWidth;
		clientRect.top = (int)clientMarginY;
		clientRect.bottom = clientRect.top + (int)clientHeight;

		// Calculate the outer dimensions of the physical window, including frame et. al.
		RECT windowRect = clientRect;
		AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

		SetWindowLong( (HWND)m_windowHandle, GWL_STYLE, windowStyleFlags );
		SetWindowLong( (HWND)m_windowHandle, GWL_EXSTYLE, windowStyleExFlags );
		SetWindowPos( (HWND)m_windowHandle, 0, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_SHOWWINDOW );
		
		
		//ShowWindow( (HWND)m_windowHandle, SW_MAXIMIZE );
	}
	else {
		//ShowWindow( (HWND)m_windowHandle, SW_SHOWDEFAULT );
		const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
		const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

		// Get desktop rectangular, dimensions, aspect
		RECT desktopRect;
		HWND desktopWindowHandle = GetDesktopWindow();
		GetClientRect( desktopWindowHandle, &desktopRect );
		float desktopWidth = (float)(desktopRect.right - desktopRect.left);
		float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
		float desktopAspect = desktopWidth / desktopHeight;

		// Calculate maximum client size (as some % of desktop size)
		constexpr float maxClientFractionOfDesktop = 0.90f;
		float clientWidth = desktopWidth * maxClientFractionOfDesktop;
		float clientHeight = desktopHeight * maxClientFractionOfDesktop;
		if (m_config.m_clientAspect > desktopAspect)
		{
			// Client window has a wider aspect than desktop; shrink client height to match its width
			clientHeight = clientWidth / m_config.m_clientAspect;
		}
		else
		{
			// Client window has a taller aspect than desktop; shrink client width to match its height
			clientWidth = clientHeight * m_config.m_clientAspect;
		}

		// Calculate client rectangular bounds by centering the client area
		float clientMarginX = 0.5f * (desktopWidth - clientWidth);
		float clientMarginY = 0.5f * (desktopHeight - clientHeight);
		RECT clientRect;
		clientRect.left = (int)clientMarginX;
		clientRect.right = clientRect.left + (int)clientWidth;
		clientRect.top = (int)clientMarginY;
		clientRect.bottom = clientRect.top + (int)clientHeight;

		// Calculate the outer dimensions of the physical window, including frame et. al.
		RECT windowRect = clientRect;
		AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

		SetWindowLong( (HWND)m_windowHandle, GWL_STYLE, windowStyleFlags );
		SetWindowLong( (HWND)m_windowHandle, GWL_EXSTYLE, windowStyleExFlags );
		SetWindowPos( (HWND)m_windowHandle, 0, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_SHOWWINDOW );
		
	}
}

bool Window::ChooseFileFromBroser( std::string& filePath )
{
	char fileName[MAX_PATH];
	fileName[0] = '\0';
	char dictionary[MAX_PATH];

	GetCurrentDirectoryA( MAX_PATH, dictionary );

	EventArgs args1;
	args1.SetValue( "HiddenMode", "false" );
	args1.SetValue( "RelativeMode", "false" );
	FireEvent( "SetCursorMode", args1 );

	EventArgs args2;
	FireEvent( "GetCursorMode", args2 );

	OPENFILENAMEA fileData = { };
	fileData.lStructSize = sizeof( fileData );
	fileData.lpstrFile = fileName;
	fileData.nMaxFile = sizeof( fileName );
	fileData.lpstrFilter = "All\0*.*\0";
	fileData.nFilterIndex = 1;
	fileData.lpstrInitialDir = dictionary;
	fileData.hwndOwner = (HWND)m_windowHandle;
	fileData.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	bool res = GetOpenFileNameA( &fileData );
	int errCode = GetLastError();
	if (!res && errCode) {
		ERROR_AND_DIE( Stringf( "Error opening files! Error Code: %d", errCode ) );
	}

	SetCurrentDirectoryA( dictionary );

	filePath.assign( fileName );

	FireEvent( "SetCursorMode", args2 );

	return true;
}

//-----------------------------------------------------------------------------------------------
// Handles Windows (Win32) messages/events; i.e. the OS is trying to tell us something happened.
// This function is called back by Windows whenever we tell it to (by calling DispatchMessage).
LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam )
{
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	// block keys????
	if (ImGui_ImplWin32_WndProcHandler( windowHandle, wmMessageCode, wParam, lParam ))
		return true;
	// block mouse cursor input
	if (ImGui::GetCurrentContext()) {
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse && (wmMessageCode == WM_LBUTTONDOWN || wmMessageCode == WM_LBUTTONUP || wmMessageCode == WM_RBUTTONDOWN || wmMessageCode == WM_RBUTTONUP || wmMessageCode == WM_MBUTTONDOWN || wmMessageCode == WM_MBUTTONUP || wmMessageCode == WM_MOUSEWHEEL || wmMessageCode == WM_MOUSEMOVE))
		{
			return TRUE;
		}
	}

	//Window* thisWindow = Window::GetWindowContext();
	//InputSystem* inputSystem = thisWindow->GetConfig().m_inputSystem;
	switch (wmMessageCode)
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
	case WM_CLOSE:
	{
		FireEvent( "quit" );
		//g_theApp->HandleQuitRequested();
		return 0; // "Consumes" this message (tells Windows "okay, we handled it")
	}

	// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
	case WM_KEYDOWN:
	{
		EventArgs args;
		args.SetValue( "KeyCode", (unsigned char)wParam );
		FireEvent( "KeyPressed", args );
		return 0;
		/*
		unsigned char asKey = (unsigned char)wParam;
		if (inputSystem->HandleKeyPressed( asKey )) {
			return 0;
		}
		break;
		*/
	}

	// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
	case WM_KEYUP:
	{		
		EventArgs args;
		args.SetValue( "KeyCode", (unsigned char)wParam );
		FireEvent( "KeyReleased", args );
		return 0;
		/*
		unsigned char asKey = (unsigned char)wParam;

		if (inputSystem->HandleKeyReleased( asKey )) {
			return 0;
		}
		break;
		*/
	}
	case WM_LBUTTONDOWN:
	{
		EventArgs args;
		args.SetValue( "KeyCode", KEYCODE_LEFTMOUSE );
		FireEvent( "KeyPressed", args );
		return 0;
		/*
		unsigned char asKey = KEYCODE_LEFTMOUSE;
		if (inputSystem->HandleKeyPressed( asKey )) {
			return 0;
		}
		break;*/
	}
	case WM_LBUTTONUP:
	{
		EventArgs args;
		args.SetValue( "KeyCode", KEYCODE_LEFTMOUSE );
		FireEvent( "KeyReleased", args );
		return 0;
		/*
		unsigned char asKey = KEYCODE_LEFTMOUSE;
		if (inputSystem->HandleKeyReleased( asKey )) {
			return 0;
		}
		break;*/
	}
	case WM_RBUTTONDOWN:
	{
		EventArgs args;
		args.SetValue( "KeyCode", KEYCODE_RIGHTMOUSE );
		FireEvent( "KeyPressed", args );
		return 0;
		/*
		unsigned char asKey = KEYCODE_RIGHTMOUSE;
		if (inputSystem->HandleKeyPressed( asKey )) {
			return 0;
		}
		break;
		*/
	}
	case WM_RBUTTONUP:
	{
		EventArgs args;
		args.SetValue( "KeyCode", KEYCODE_RIGHTMOUSE );
		FireEvent( "KeyReleased", args );
		return 0;
		/*
		unsigned char asKey = KEYCODE_RIGHTMOUSE;
		if (inputSystem->HandleKeyReleased( asKey )) {
			return 0;
		}
		break;*/
	}
	case WM_CHAR:
	{
		if (g_devConsole) {
			EventArgs args;
			args.SetValue( "KeyCode", (unsigned char)wParam );
			FireEvent( "CharInput", args );
		}
		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		if (IsMouseWheelPresent()) {
			EventArgs args;
			short delta = GET_WHEEL_DELTA_WPARAM( wParam );
			args.SetValue( "MouseWheelValue", (short)delta );
			FireEvent( "MouseWheelInput", args );
			return 0;
		}
		return 0;
	}
	// set the cursor back to arrow
	case WM_SETCURSOR:
	{
		return TRUE;
	}
	}

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc( windowHandle, wmMessageCode, wParam, lParam );
}

bool IsMouseWheelPresent()
{
	return (GetSystemMetrics( SM_MOUSEWHEELPRESENT ) != 0);
}

//-----------------------------------------------------------------------------------------------
void Window::CreateOSWindow()
{
	// Define a window style/class
	HINSTANCE handleInstance = GetModuleHandle( NULL );
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = handleInstance;
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT( "Simple Window Class" );
	RegisterClassEx( &windowClassDescription );

	// #SD1ToDo: Add support for full screen mode (requires different window style flags than windowed mode)
	DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	RECT windowRect = { 0 };
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect( desktopWindowHandle, &desktopRect );
	if (m_config.m_isFullScreen) {
		windowRect = desktopRect;
		windowStyleFlags = WS_POPUP;
		windowStyleExFlags = WS_EX_TOPMOST;
	}
	else{
		// if window size and window position is not set
		if (m_config.m_windowSize == IntVec2( -1, -1 ) && m_config.m_windowPosition == IntVec2( -1, -1 )) {
			// Get desktop rectangular, dimensions, aspect
			float desktopWidth = (float)(desktopRect.right - desktopRect.left);
			float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
			float desktopAspect = desktopWidth / desktopHeight;

			// Calculate maximum client size (as some % of desktop size)
			constexpr float maxClientFractionOfDesktop = 0.9f;
			float clientWidth = desktopWidth * maxClientFractionOfDesktop;
			float clientHeight = desktopHeight * maxClientFractionOfDesktop;
			if (m_config.m_clientAspect > desktopAspect)
			{
				// Client window has a wider aspect than desktop; shrink client height to match its width
				clientHeight = clientWidth / m_config.m_clientAspect;
			}
			else
			{
				// Client window has a taller aspect than desktop; shrink client width to match its height
				clientWidth = clientHeight * m_config.m_clientAspect;
			}

			// Calculate client rectangular bounds by centering the client area
			float clientMarginX = 0.5f * (desktopWidth - clientWidth);
			float clientMarginY = 0.5f * (desktopHeight - clientHeight);
			RECT clientRect;
			clientRect.left = (int)clientMarginX;
			clientRect.right = clientRect.left + (int)clientWidth;
			clientRect.top = (int)clientMarginY;
			clientRect.bottom = clientRect.top + (int)clientHeight;
			windowRect = clientRect;
		}
		else {
			float clientLeftX, clientTopY;
			float clientWidth, clientHeight;

			float desktopWidth = (float)(desktopRect.right - desktopRect.left);
			float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
			if (m_config.m_windowSize == IntVec2( -1, -1 )) {
				float desktopAspect = desktopWidth / desktopHeight;

				// Calculate maximum client size (as some % of desktop size)
				constexpr float maxClientFractionOfDesktop = 0.9f;
				clientWidth = desktopWidth * maxClientFractionOfDesktop;
				clientHeight = desktopHeight * maxClientFractionOfDesktop;
				if (m_config.m_clientAspect > desktopAspect)
				{
					// Client window has a wider aspect than desktop; shrink client height to match its width
					clientHeight = clientWidth / m_config.m_clientAspect;
				}
				else
				{
					// Client window has a taller aspect than desktop; shrink client width to match its height
					clientWidth = clientHeight * m_config.m_clientAspect;
				}
			}
			else {
				clientWidth = (float)m_config.m_windowSize.x;
				clientHeight = (float)m_config.m_windowSize.y;
			}

			if (m_config.m_windowPosition == IntVec2( -1, -1 )) {
				clientLeftX = 0.5f * (desktopWidth - clientWidth);;
				clientTopY = 0.5f * (desktopHeight - clientHeight);
			}
			else {
				clientLeftX = (float)m_config.m_windowPosition.x;
				clientTopY = (float)m_config.m_windowPosition.y;
			}

			RECT clientRect;
			clientRect.left = (int)(clientLeftX);
			clientRect.right = (int)(clientLeftX + clientWidth);
			clientRect.top = (int)(clientTopY);
			clientRect.bottom = (int)(clientTopY + clientHeight);
			windowRect = clientRect;
		}
	}
	

	// Calculate the outer dimensions of the physical window, including frame et. al.
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	WCHAR windowTitle[1024];
	MultiByteToWideChar( GetACP(), 0, m_config.m_windowTitle.c_str(), -1, windowTitle, sizeof( windowTitle ) / sizeof( windowTitle[0] ) );
	m_windowHandle = (void*)CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		//(HINSTANCE)applicationInstanceHandle,
		handleInstance,
		NULL );

	ShowWindow( (HWND)m_windowHandle, SW_SHOW );
	SetForegroundWindow( (HWND)m_windowHandle );
	SetFocus( (HWND)m_windowHandle );

	m_displayContext = (void*)GetDC( (HWND)m_windowHandle );

	HCURSOR cursor = LoadCursor( NULL, IDC_ARROW );
	SetCursor( cursor );
}


//-----------------------------------------------------------------------------------------------
// Processes all Windows messages (WM_xxx) for this app that have queued up since last frame.
// For each message in the queue, our WindowsMessageHandlingProcedure (or "WinProc") function
//	is called, telling us what happened (key up/down, minimized/restored, gained/lost focus, etc.)
void Window::RunMessagePump()
{
	MSG queuedMessage;
	for (;; )
	{
		const BOOL wasMessagePresent = PeekMessage( &queuedMessage, NULL, 0, 0, PM_REMOVE );
		if (!wasMessagePresent)
		{
			break;
		}

		TranslateMessage( &queuedMessage );
		DispatchMessage( &queuedMessage ); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}



