#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in VERY few places (and .CPPs only)
#include "Game/App.hpp"
#include "Engine/Core/EngineCommon.hpp"

extern App* g_theApp;

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( _In_ HINSTANCE applicationInstanceHandle, _In_opt_ HINSTANCE, _In_ LPSTR commandLineString, _In_ int )
{
	UNUSED( applicationInstanceHandle );
	UNUSED( commandLineString );
	g_theApp = new App();
	g_theApp->Startup();

	// Program main loop; keep running frames until it's time to quit
	g_theApp->Run();
	
	g_theApp->Shutdown();
	delete g_theApp;
	g_theApp = nullptr;

	return 0;
}


