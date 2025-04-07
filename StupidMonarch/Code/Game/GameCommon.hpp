#pragma once
// ----------------------game common things puts in here-------------------------
// frequently include headers
// should not include Game.hpp
// 
// following .hpps are included in Vertex_PCU.hpp
//#include "Engine/Math/Vec2.hpp"
//#include "Engine/Math/Vec3.hpp"
//#include "Engine/Core/Rgba8.hpp"
#include <string>
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/HeatMaps.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Math/EngineMath.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"

constexpr char const* APP_NAME = "Stupid Monarch";

#ifdef _DEBUG
#define DEBUG_MODE
#endif

class App;
class Game;
class Entity;
class SM_BitmapFont;
enum class GameMode;
typedef std::vector<Entity*> EntityList;

enum class SM_GameLanguage {
	ENGLISH,
	ZH,
	ZH_CN,
	DEFAULT,
};
// global variables
extern SM_GameLanguage g_gameLanguage;

extern App* g_theApp;
extern Game* g_theGame;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_window;
extern BitmapFont* g_ASCIIFont;
extern SM_BitmapFont* g_chineseFont;

extern GameMode g_gameMode;


// constant variables
constexpr float UI_SIZE_X = 1600.f;
constexpr float UI_SIZE_Y = 800.f;
constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;

constexpr int developmentCost = 5;
constexpr int defenseCost = 5;
constexpr int attractCost = 5;
constexpr int buildArmyCost = 5;
constexpr int addLegalProgressCost = 5;

enum class AudioName { AttractMode, NUM };
enum class AnimationName { PlaceHolder, NUM };

// debug drawing functions
void DebugDrawRing( Vec2 const& center, float radius, float thickness, Rgba8 const& color );
void DebugDrawLine( Vec2 const& startPos, float orientation, float length, float thickness, Rgba8 const& color );
void DebugDrawLine( Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color );

// buttons
void AddVertsForExitButton( std::vector<Vertex_PCU>& verts, Vec2 const& pos, float radius );
void AddVertsForTriangle2D( std::vector<Vertex_PCU>& verts, Vec2 const& p1, Vec2 const& p2, Vec2 const& p3, Rgba8 const& color );