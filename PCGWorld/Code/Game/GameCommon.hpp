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
#include <source_location>
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
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/DX11/DX11Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/FileUtils.hpp"

#include "ThirdParty/Squirrel/SmoothNoise.hpp"

#include "ThirdParty/ImGui/imgui.h"
#include "ThirdParty/ImGui/backends/imgui_impl_win32.h"
#include "ThirdParty/ImGui/backends/imgui_impl_dx11.h"
#include "ThirdParty/ImGui/misc/cpp/imgui_stdlib.h"
#include "ThirdParty/zip/zip.h"

constexpr char const* APP_NAME = "PCG World";

#ifdef _DEBUG
#define DEBUG_MODE
#endif

//#define DEBUG_COMPARE_HISTORY

class App;
class Game;
class Entity;
class Map;
typedef std::vector<Entity*> EntityList;
// global variables
extern App* g_theApp;
extern Game* g_theGame;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_window;
extern BitmapFont* g_ASCIIFont;

// constant variables
constexpr float UI_SIZE_X = 1600.f;
constexpr float UI_SIZE_Y = 800.f;
constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float EDGE_GUARD_X = 600.f;
constexpr float EDGE_GUARD_Y = 600.f;
constexpr float MAX_SIZE_X = EDGE_GUARD_X * 0.5f;
constexpr float MAX_SIZE_Y = EDGE_GUARD_X * 0.5f;
constexpr float CITY_ICON_SIDE_LENGTH = 0.15f;
constexpr float TOWN_ICON_SIDE_LENGTH = 0.1f;
constexpr float TAX_PERSON_SAME_CULTURE = 0.05f;
constexpr float TAX_PERSON_DIFF_CULTURE = 0.005f;
constexpr float TAX_PERSON_COMMERCIAL_CITY = 0.1f;
constexpr float TAX_PERSON_NORMAL_CITY = 0.05f;
constexpr float PRODUCTION_VALUE_PER_PERSON = 0.1f;
constexpr int ARMY_SOLDIER_COST = 6;
constexpr int ARMY_COST = 4000;
constexpr int BUILD_ARMY_COST = 100;
constexpr int RECRUIT_SOLDIER_COST = 100;
constexpr int MAX_SAVE_MONTH = 96;
constexpr int DEV_COST = 10000;
constexpr int ASSIMILATE_COST = 10000;
constexpr int CONVERT_COST = 10000;
constexpr int BUILD_FRIENDLY_RELATION_COAT = 20000;
constexpr float HEIGHT_FACTOR = 0.0008f;

enum class AudioName { AttractMode, NUM };
enum class AnimationName { PlaceHolder, NUM };

// debug drawing functions
void DebugDrawRing( Vec2 const& center, float radius, float thickness, Rgba8 const& color );
void DebugDrawLine( Vec2 const& startPos, float orientation, float length, float thickness, Rgba8 const& color );
void DebugDrawLine( Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color );

struct StarEdge;
class MapPolygonUnit;

void PCGWorld_Log( std::string const& message, const std::source_location& location = std::source_location::current() );

struct MapGenerationSettings;
Map* GetCurMap();
MapGenerationSettings const& GetCurGenerationSettings();

/// return one of the two shared edges, use opposite variable to fine the other one
StarEdge* GetSharedEdge( MapPolygonUnit* unit1, MapPolygonUnit* unit2 );

enum class Direction {
	North,
	South,
	West,
	East,
	NorthEast,
	NorthWest,
	SouthEast,
	SouthWest,
	Middle,
	NUM
};

enum class CountryGovernmentType {
	Autocracy, Parliamentarism, Nomad, Separatism, Oligarchy, None,
};

extern const char* g_governmentTypeDict[];

extern std::vector<std::string> g_productNameMap;
extern std::vector<std::string> g_climateNameMap;
extern std::vector<std::string> g_landformNameMap;
extern std::vector<std::string> g_cultureOriginNameMap;
extern std::string g_monthShortNameMap[12];
extern std::string g_monthFullNameMap[12];
extern const char* g_productNameMapC[];
extern const char* g_landformNameMapC[];
extern const char* g_climateNameMapC[];
extern const char* g_cityAttributeNameMapC[];
extern const char* g_cultureTraitsName[];
extern const char* g_cultureTraitsDesc[];

class Country;
struct CountryEndWarTreaty {
	Country* m_country1 = nullptr;
	Country* m_country2 = nullptr;
	int m_year = 0;
	int m_month = 0;
};

constexpr WorkerThreadType Loading_Job = 0x2;