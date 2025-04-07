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
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/EngineMath.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Window.hpp"

// advanced functions definition here
// #define DUAL_PLAYERS // uncompleted
// Enemy shoot macro: beetles can shoot, player is stronger
// #define ENEMY_SHOOT
// debug mode: has debug keys
//#ifdef _DEBUG
#define DEBUG_MODE
//#endif

constexpr char const* APP_NAME = "SD1A4: StarShip Gold";

class App;
class Game;
struct Text;
// global variables
extern App* g_theApp;
extern Game* g_theGame;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_window;
extern BitmapFont* g_ASCIIFont;

// constant variables

// level and wave information
constexpr int NUM_OF_LEVELS = 9;
constexpr int NUM_WAVES = 5;
constexpr int NUM_ENEMY_TYPES = 7;
constexpr int MAX_WAVES_IN_LEVEL = 30;
//constexpr int MAX_TEXTS_IN_LEVEL = 30;
// hard code wave information: maybe need to change later
constexpr int NUM_OF_WAVES_IN_LEVEL[NUM_OF_LEVELS] = { 4, 3, 4, 5, 5, 5, 5, 5, 1 };
// wave load time / asteroids / beetles / wasps / boss1 / boss2 / carrier / boss3
constexpr int LEVEL_INFO[NUM_OF_LEVELS][MAX_WAVES_IN_LEVEL][NUM_ENEMY_TYPES + 1] = {
	{
		{42, 2, 0, 0, 0, 0, 0, 0},
		{5, 2, 0, 0, 0, 0, 0, 0},
		{9, 2, 1, 0, 0, 0, 0, 0},
		{9, 2, 0, 1, 0, 0, 0, 0},
	},
	{
		{5, 2, 0, 0, 0, 0, 0, 0},
		{9, 2, 1, 0, 0, 0, 0, 0},
		{9, 3, 2, 1, 0, 0, 0, 0}
	},
	{
		{5, 2, 1, 1, 0, 0, 0, 0},
		{9, 2, 1, 1, 0, 0, 0, 0},
		{9, 3, 1, 1, 0, 0, 0, 0},
		{9, 3, 2, 1, 0, 0, 0, 0}
	},
	{
		{5, 2, 1, 1, 0, 0, 0, 0},
		{10, 2, 1, 1, 0, 0, 0, 0},
		{10, 3, 2, 1, 1, 0, 0, 0},//boss
		{10, 3, 2, 1, 0, 0, 0, 0},
		{10, 3, 3, 1, 0, 0, 0, 0}
	},
	{
		{5, 2, 0, 0, 0, 0, 2, 0},
		{10, 3, 3, 3, 0, 0, 1, 0},
		{10, 3, 4, 4, 0, 0, 1, 0},
		{10, 4, 5, 5, 0, 0, 2, 0},
		{10, 4, 6, 6, 0, 0, 1, 0}
	},
	{
		{5, 2, 1, 1, 0, 0, 2, 0},
		{10, 3, 2, 2, 0, 0, 2, 1},//boss
		{10, 3, 3, 3, 0, 0, 2, 0},
		{10, 4, 4, 4, 0, 0, 2, 0},
		{10, 4, 5, 5, 0, 0, 2, 0}
	},
	{
		{5, 4, 2, 1, 0, 0, 1, 0},
		{10, 6, 4, 2, 0, 0, 1, 0},
		{10, 6, 6, 3, 0, 0, 1, 0},
		{10, 6, 8, 4, 0, 0, 1, 0},
		{10, 6, 12, 5, 0, 0, 1, 0}
	},
	{
		{5, 6, 2, 2, 0, 0, 1, 0},
		{10, 8, 3, 3, 0, 0, 1, 0},
		{10, 8, 4, 6, 0, 0, 1, 0},
		{10, 8, 5, 9, 0, 0, 1, 0},
		{10, 8, 6, 12, 0, 0, 1, 0}
	},
	{
		{5, 25, 0, 0, 0, 1, 2, 0},//boss
	},
};

constexpr int NUM_OF_TEXTS_IN_LEVEL[NUM_OF_LEVELS] = { 30, 30, 30, 30, 30, 30 };

// 
// asteroids / beetles / wasps / boss1 / boss2
/*constexpr int WAVE_INFO[NUM_WAVES][NUM_ENEMY_TYPES] = {{3, 1, 0},
														{5, 3, 1},
														{7, 5, 2},
														{9, 6, 3},
														{11, 7, 4},
													   };*/
//constexpr int WAVE_INFO[NUM_WAVES][NUM_ENEMY_TYPES] = { {0, 1, 0},
//														{0, 1, 0},
//														{0, 1, 0},
//														{0, 1, 0},
//														{0, 1, 0},
//													   };

//constexpr int NUM_STARTING_ASTEROIDS = 6;
//constexpr int MAX_ASTEROIDS = 12;
//constexpr int MAX_BULLETS = 20;
constexpr int MAX_ENTITIES = 1000;
constexpr int MAX_DEBRIS = 2000;
//constexpr int PLAYER_HEALTH = 3;

constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;
constexpr float UI_SIZE_X = 1600.f;
constexpr float UI_SIZE_Y = 800.f;

constexpr float ASTEROID_SPEED = 10.f;
constexpr float ASTEROID_PHYSICS_RADIUS = 1.6f;
constexpr float ASTEROID_COSMETIC_RADIUS = 2.0f;
constexpr int ASTEROID_HEALTH = 4;

constexpr float BULLET_LIFETIME_SECONDS = 2.0f;
constexpr float BULLET_SPEED = 50.f;
constexpr float BULLET_PHYSICS_RADIUS = 0.75f;
constexpr float BULLET_COSMETIC_RADIUS = 2.0f;

constexpr float PLAYER_SHIP_ACCELERATION = 300.f;
constexpr int NUM_OF_PLAYER_SHIP_VERTS = 15;
constexpr float PLAYER_SHIP_TURN_SPEED = 300.f;
constexpr float PLAYER_SHIP_PHYSICS_RADIUS = 1.75f;
constexpr float PLAYER_SHIP_COSMETIC_RADIUS = 2.25f;
constexpr int PLAYER_SHIP_HEALTH = 5;

constexpr float BEETLE_SPEED = 10.0f;
constexpr float BEETLE_PHYSICS_RADIUS = 1.6f;
constexpr float BEETLE_COSMETIC_RADIUS = 2.5f;
constexpr int BEETLE_HEALTH = 3;

constexpr float WASP_MAX_SPEED = 30.0f;
constexpr float WASP_ACCELERATE_SPEED = 5.0f;
constexpr float WASP_PHYSICS_RADIUS = 1.25f;
constexpr float WASP_COSMETIC_RADIUS = 2.f;
constexpr int WASP_HEALTH = 2;

constexpr float CARRIER_SPEED = 6.0f;
constexpr float CARRIER_PHYSICS_RADIUS = 2.5f;
constexpr float CARRIER_COSMETIC_RADIUS = 3.f;
constexpr int CARRIER_HEALTH = 5;

constexpr float FIGHTER_SPEED = 20.0f;
constexpr float FIGHTER_PHYSICS_RADIUS = 1.5f;
constexpr float FIGHTER_COSMETIC_RADIUS = 2.f;
constexpr int FIGHTER_HEALTH = 1;

constexpr float CONE_LIFETIME_SECONDS = 0.3f;
constexpr float CONE_APERTURE_DEGREES = 100.f;
constexpr float CONE_RADIUS = 25.f;

constexpr float LASER_LIFETIME_SECONDS = 0.5f;
//constexpr int LASER_DAMAGE_STAGE = 3;
constexpr float LASER_COOLDOWN_SECONDS = 0.6f;

constexpr float ROCKET_LIFETIME_SECONDS = 8.f;
constexpr float ROCKET_PHYSICS_RADIUS = 2.f;
constexpr float ROCKET_COSMETIC_RADIUS = 2.5f;

constexpr int NUM_OF_SABERS = 1;

constexpr float POWER_UP_PHYSICS_RADIUS = 3.f;
constexpr float POWER_UP_COSMETIC_RADIUS = 1.5f;

constexpr float BOSS_EXPLORER_SPEED = 18.f;
constexpr float BOSS_EXPLORER_ACCELERATE_SPEED = 5.f;
constexpr float BOSS_EXPLORER_PHYSICS_RADIUS = 3.f;
constexpr float BOSS_EXPLORER_COSMETIC_RADIUS = 4.f;
constexpr int BOSS_EXPLORER_HEALTH = 60;
constexpr int BOSS_SECOND_STAGE_HEALTH = 50;
constexpr int BOSS_THIRD_STAGE_HEALTH = 30;

constexpr float BOSS_MOTHER_PHYSICS_RADIUS = 4.f;
constexpr float BOSS_MOTHER_COSMETIC_RADIUS = 5.f;
constexpr float BOSS_MOTHER_SPEED = 10.f;
constexpr int BOSS_MOTHER_HEALTH = 120;

constexpr int DESTROYER_HEALTH = 300;

constexpr float TURRET_COSMETIC_RADIUS = 3.f;
constexpr float TURRET_PHYSICS_RADIUS = 2.5f;
constexpr int TURRET_HEALTH = 18;

constexpr float ENEMY_TURN_SPEED = 50.f;

constexpr float DEBRIS_LIFETIME_SECONDS = 2.0f;

constexpr float SCREEN_SHAKE_SECONDS = 0.8f;

constexpr float POWER_UP_LIFETIME_SECONDS = 10.f;

enum class AudioName{shoot, playerDie, playerReborn, enemyDie, newLevel, victory, defeat, laser, powerUp,
	health, coneAttack, rocket, lightSaberOn, lightSaberRotate, attractMode, battle, boss, NUM};

// debug drawing functions
void DebugDrawRing( Vec2 const& center, float radius, float thickness, Rgba8 const& color );
void DebugDrawLine( Vec2 const& startPos, float orientation, float length, float thickness, Rgba8 const& color );
void DebugDrawLine( Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color );

void DrawXBoxButton( Vec2 const& centerPos, float radius, XboxButtonID buttonName, Rgba8 const& mainColor );
void DrawJoyStick( Vec2 const& centerPos, float radius, Rgba8 const& mainColor );
void DrawKeyBoardButtom( AABB2 const& bounds, unsigned char keyValue, Rgba8 const& mainColor );

bool DoDiscOverlapOrientedSector2D( Vec2 const& discCenter, float discRadius, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius );
bool DoDiscOverlapAABB2D( Vec2 const& discCenter, float discRadius, AABB2 const& aabbBox );
bool DoDiscOverlapOBB2D( Vec2 const& discCenter, float discRadius, OBB2 const& obbBox );
bool DoDiscOverlapCapsule2D( Vec2 const& discCenter, float discRadius, Vec2 const& boneStartPoint, Vec2 const& boneEndPoint, float capsuleRadius );