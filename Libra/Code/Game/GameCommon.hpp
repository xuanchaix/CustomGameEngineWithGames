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
#include "Engine/Math/EngineMath.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Clock.hpp"


constexpr char const* APP_NAME = "SD1-A8: Libra Epilogue";

#define DEBUG_MODE

class App;
class Game;
// global variables
extern App* g_theApp;
extern Game* g_theGame;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_window;
extern BitmapFont* g_ASCIIFont;

extern float g_enemyVisibleRange;

// constant variables
constexpr float UI_SIZE_X = 1600.f;
constexpr float UI_SIZE_Y = 800.f;
constexpr float WORLD_SIZE_X = 20.f;
constexpr float WORLD_SIZE_Y = 10.f;

//constexpr float TILE_SIDE_LENGTH = 12.f;
constexpr int NUM_TILES_IN_X_AXIS = 20;

//------------------------------------------
// player staffs
constexpr float PLAYER_PHYSICS_RADIUS = 0.25f;
constexpr float PLAYER_COSMETIC_RADIUS = 0.4f;
constexpr float PLAYER_START_ROTATION = 45.f;

//------------------------------------------
// Scorpio staffs
constexpr float SCORPIO_PHYSICS_RADIUS = 0.4f;
constexpr float SCORPIO_COSMETIC_RADIUS = 0.5f;

//------------------------------------------
// Leo staffs
constexpr float LEO_PHYSICS_RADIUS = 0.2f;
constexpr float LEO_COSMETIC_RADIUS = 0.4f;

//------------------------------------------
// Cancer staffs
constexpr float CANCER_PHYSICS_RADIUS = 0.2f;
constexpr float CANCER_COSMETIC_RADIUS = 0.4f;

//------------------------------------------
// Aries staffs
constexpr float ARIES_PHYSICS_RADIUS = 0.2f;
constexpr float ARIES_COSMETIC_RADIUS = 0.4f;

//------------------------------------------
// Capricorn staffs
constexpr float CAPRICORN_PHYSICS_RADIUS = 0.2f;
constexpr float CAPRICORN_COSMETIC_RADIUS = 0.4f;

//------------------------------------------
// Bullet staffs
constexpr float BULLET_PHYSICS_RADIUS = 0.03f;
constexpr float BULLET_COSMETIC_RADIUS = 0.1f;

//------------------------------------------
// Explosion staffs
constexpr float EXPLOSION_COSMETIC_RADIUS = 0.5f;

enum class AudioName{AttractMode, GameMode, StartGame, Click, Pause, UnPause, BulletBounce, BulletRicochet, BulletRicochet2, EnemyDied, EnemyHit,
	EnemyShoot, ExitMap, GameOver, PlayerHit, PlayerShootNormal, Victory, EnemyAlert, NUM};

enum class AnimationName{EntityExplosion, BulletExplosion, FlameBullet, NUM};

// debug drawing functions
// 
//void DebugDrawRing( Vec2 const& center, float radius, float thickness, Rgba8 const& color );
//void DebugDrawLine( Vec2 const& startPos, float orientation, float length, float thickness, Rgba8 const& color );
//void DebugDrawLine( Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color );

void AddVertsForDebugDrawRing( std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness, Rgba8 const& color );
void AddVertsForDebugDrawLine( std::vector<Vertex_PCU>& verts, Vec2 const& startPos, float orientation, float length, float thickness, Rgba8 const& color );
void AddVertsForDebugDrawLine( std::vector<Vertex_PCU>& verts, Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color );