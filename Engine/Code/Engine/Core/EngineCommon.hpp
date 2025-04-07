#pragma once
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Game/EngineBuildPreferences.hpp"
class EventSystem;
class DevConsole;
class RandomNumberGenerator;
class JobSystem;
class NetSystem;
//---------------------some core const or macro in engine------------

//-----------------------------------------------------------------------------------------------
// if a variable is not used in a place, put a UNUSED can prevent compiler warning and remind programmer
#define UNUSED(x) (void)(x);

//-----------------------------------------------------------------------------------------------
//
#ifndef ENGINE_DX12_RENDERER_INTERFACE
#define ENGINE_DX11_RENDERER_INTERFACE
#else
#undef ENGINE_DX11_RENDERER_INTERFACE
#endif // !DX11RendererInterface

class NamedProperties;
typedef NamedProperties EventArgs;

enum class EndianMode {
	Native, Little, Big, Num
};

bool Event_LoadGameConfig( EventArgs& args );

EndianMode GetPlatformLocalEndian();

extern NamedStrings g_gameConfigBlackboard;
extern EventSystem* g_theEventSystem;
extern DevConsole* g_devConsole;
extern JobSystem* g_theJobSystem;
extern NetSystem* g_theNetSystem;
extern RandomNumberGenerator* g_engineRNG;