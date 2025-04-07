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
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Math/EngineMath.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Timer.hpp"

constexpr char const* APP_NAME = "Shape Factory";

#ifdef _DEBUG
#define DEBUG_MODE
#endif

class App;
class Game;
class Entity;
class Map;
class Selector;
class SFUISystem;
class SFWidget;
class Building;
class PowerNetwork;
typedef std::vector<Entity*> EntityList;
// global variables
extern App* g_theApp;
extern Game* g_theGame;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_window;
extern BitmapFont* g_ASCIIFont;
extern SFUISystem* g_uiSystem;
extern Building* g_curChosenBuilding;
extern SpriteSheet* g_conveyorBeltSprite;
class ProductDefinition;
class Blender;
class PowerBuilding;
class Conveyer;

// constant variables
constexpr int NumOfProductTypes = 20;
constexpr float UI_SIZE_X = 1600.f;
constexpr float UI_SIZE_Y = 800.f;
constexpr float WORLD_SIZE_X = 36.f;
constexpr float WORLD_SIZE_Y = 18.f;

constexpr int TILE_MAP_SIZE_X = 1000;
constexpr int TILE_MAP_SIZE_Y = 1000;

constexpr float RESOURCE_RADIUS = 0.245f;
constexpr float CONVEY_SPEED = 0.75f;

enum class AudioName { AttractMode, NUM };
enum class AnimationName { PlaceHolder, NUM };
enum class Direction {
	Left, Right, Up, Down, None
};

enum class ProcessDirection {
	Forward, Left, Right, Backward,
};

extern Vec2 DirectionUnitVec[];
extern IntVec2 DirectionUnitIntVec[];
extern Vec2 OutputVec[];
extern Vec2 InputVec[];

enum class BuildingType {
	Building, Conveyer, Base, Drill, Selector, Router, OverflowGate, WareHouse, Exporter, Junction, Bridge, PowerPlant, PowerNode, Blender, Refinery, Wall, StraightArcher, GuidedMissile, ThreeDirectionsPike, Mortar, Laser, None,
};

enum class EventCallBackFuncType {
	Selector,
};

// debug drawing functions
void DebugDrawRing( Vec2 const& center, float radius, float thickness, Rgba8 const& color );
void DebugDrawLine( Vec2 const& startPos, float orientation, float length, float thickness, Rgba8 const& color );
void DebugDrawLine( Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color );

void AddVertsForPowerLink( std::vector<Vertex_PCU>& verts, PowerBuilding const* powerBuilding );
void AddVertForCursor( std::vector<Vertex_PCU>& verts, IntVec2 const& cursorCoords );

void DrawConveyer( Conveyer const* conveyor, IntVec2 const& LBPos, Direction dir, Rgba8 const& tint = Rgba8::WHITE, unsigned char alpha = 255 );
void AddVertsForDrill( std::vector<Vertex_PCU>& verts, IntVec2 const& LBPos, Direction dir, Rgba8 const& tint = Rgba8::WHITE, unsigned char alpha = 255 );
void AddVertsForSelector( std::vector<Vertex_PCU>& verts, Selector const* selector, Rgba8 const& tint = Rgba8::WHITE, unsigned char alpha = 255 );
void AddVertsForSelector( std::vector<Vertex_PCU>& verts, IntVec2 const& LBPos, Direction dir, Rgba8 const& tint = Rgba8::WHITE, unsigned char alpha = 255 );
void AddVertsForRouter( std::vector<Vertex_PCU>& verts, IntVec2 const& LBPos, Rgba8 const& tint = Rgba8::WHITE, unsigned char alpha = 255 );
void AddVertsForOverflowGate( std::vector<Vertex_PCU>& verts, IntVec2 const& LBPos, Direction dir, Rgba8 const& tint = Rgba8::WHITE, unsigned char alpha = 255 );
void DrawWareHouse( IntVec2 const& LBPos, Rgba8 const& tint = Rgba8::WHITE, unsigned char alpha = 255 );
void AddVertsForExporter( std::vector<Vertex_PCU>& verts, IntVec2 const& LBPos, Direction dir, Rgba8 const& tint = Rgba8::WHITE, unsigned char alpha = 255 );
void AddVertsForJunction( std::vector<Vertex_PCU>& verts, IntVec2 const& LBPos, Rgba8 const& tint = Rgba8::WHITE, unsigned char alpha = 255 );
void DrawBridge( IntVec2 const& LBPos, Direction dir, bool isInput, Rgba8 const& tint = Rgba8::WHITE, unsigned char alpha = 255 );
void AddVertsForPowerPlant( std::vector<Vertex_PCU>& verts, IntVec2 const& LBPos, Rgba8 const& tint = Rgba8::WHITE, unsigned char alpha = 255 );
void DrawPowerNode( IntVec2 const& LBPos, Rgba8 const& tint = Rgba8::WHITE, unsigned char alpha = 255 );
void DrawRefinery( IntVec2 const& LBPos, Rgba8 const& tint = Rgba8::WHITE, unsigned char alpha = 255 );
void DrawBlender( Blender const* blender, IntVec2 const& LBPos, Rgba8 const& tint = Rgba8::WHITE, unsigned char alpha = 255 );
void DrawStraightArcher( IntVec2 const& LBPos, Direction dir, Rgba8 const& tint = Rgba8::WHITE, unsigned char alpha = 255 );
void DrawThreeDirectionPike( IntVec2 const& LBPos, Direction dir, Rgba8 const& tint = Rgba8::WHITE, unsigned char alpha = 255 );

void AddVertsForHealthBar( std::vector<Vertex_PCU>& verts, Building const* building );

void AddVertsForResource( std::vector<Vertex_PCU>& verts, Vec2 const& centerPos, ProductDefinition const& type, float radius = RESOURCE_RADIUS );
void DrawResource(Vec2 const& centerPos, ProductDefinition const& type, float radius = RESOURCE_RADIUS );

void AddVertsForResourcePanel( std::vector<Vertex_PCU>& verts, std::vector<Vertex_PCU>& textVerts, Vec2 const& middleLeftPos, int* resourcesAmount, Rgba8 const& backgroundColor = Rgba8::WHITE, Rgba8 const& textColor = Rgba8::BLACK);
void AddVertsForElectricityPanel( std::vector<Vertex_PCU>& verts, std::vector<Vertex_PCU>& textVerts, Vec2 const& middleLeftPos, PowerNetwork* network, Rgba8 const& backgroundColor = Rgba8::WHITE, Rgba8 const& textColor = Rgba8::BLACK );

Map* GetCurMap();

Direction GetInversedDir( Direction dir );
Direction GetTurnLeftDir( Direction dir );
Direction GetTurnRightDir( Direction dir );
Direction GetDir( IntVec2 const& unitIntVec2 );
float GetOrientationDegreesFromDir( Direction dir );

extern Timer* g_conveyorAnimTimer;

// global ui widgets

extern std::vector<SFWidget*> g_filterUIs;
extern std::vector<SFWidget*> g_refineryUIs;
extern std::vector<SFWidget*> g_blenderUIs;
extern std::vector<SFWidget*> g_constructionPanelUI;