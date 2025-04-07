#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include <string>

std::vector<std::string> g_productNameMap;
std::vector<std::string> g_climateNameMap;
std::vector<std::string> g_landformNameMap;
std::vector<std::string> g_cultureOriginNameMap;

void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color ) {
	constexpr int NUM_SIDES = 16;
	constexpr int NUM_TRIANGLES = NUM_SIDES * 2;
	constexpr int NUM_VERTS = 3 * NUM_TRIANGLES;
	constexpr float DEGREES_PER_SIDE = 360.f / (float)NUM_SIDES;
	constexpr float RADIANS_PER_SIDE = DEGREES_PER_SIDE * PI / 180.f;
	Vertex_PCU ringVertsArray[NUM_VERTS];
	for (int i = 0; i < NUM_SIDES; i++) {
		ringVertsArray[6 * i] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * radius, SinRadians( RADIANS_PER_SIDE * i ) * radius, 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 2] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 1] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 3] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 4] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 5] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness), 0 ), color, Vec2( 0, 0 ) );
	}
	for (int i = 0; i < NUM_VERTS; i++) {
		ringVertsArray[i].m_position += Vec3( center.x, center.y, 0 );
	}
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( NUM_VERTS, ringVertsArray );
}

void DebugDrawLine(Vec2 const& startPos, float orientation, float length, float thickness, Rgba8 const& color) {
	Vertex_PCU lineVertsArray[6];
	Vec3 p1 = Vec3( -thickness * 0.5f, -thickness * 0.5f, 0 );
	Vec3 p2 = Vec3( -thickness * 0.5f, thickness * 0.5f, 0 );
	Vec3 p3 = Vec3( length + thickness * 0.5f, -thickness * 0.5f, 0 );
	Vec3 p4 = Vec3( length + thickness * 0.5f, thickness * 0.5f, 0 );
	lineVertsArray[0] = Vertex_PCU( p1, color, Vec2( 0, 0 ) );
	lineVertsArray[1] = Vertex_PCU( p2, color, Vec2( 0, 0 ) );
	lineVertsArray[2] = Vertex_PCU( p3, color, Vec2( 0, 0 ) );
	lineVertsArray[3] = Vertex_PCU( p2, color, Vec2( 0, 0 ) );
	lineVertsArray[4] = Vertex_PCU( p3, color, Vec2( 0, 0 ) );
	lineVertsArray[5] = Vertex_PCU( p4, color, Vec2( 0, 0 ) );

	TransformVertexArrayXY3D( 6, lineVertsArray, 1.f, orientation, startPos );
	g_theRenderer->DrawVertexArray( 6, lineVertsArray );
}

void DebugDrawLine( Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color )
{
	Vec2 dForward = (endPos - startPos).GetNormalized();
	Vec2 forward = dForward * thickness * 0.5f;
	Vec2 left = dForward.GetRotated90Degrees() * thickness * 0.5f;
	Vertex_PCU lineVertsArray[6];
	Vec3 p1 = Vec3( (startPos - forward + left).x, (startPos - forward + left).y, 0 );
	Vec3 p2 = Vec3( (startPos - forward - left).x, (startPos - forward - left).y, 0 );
	Vec3 p3 = Vec3( (endPos + forward + left).x, (endPos + forward + left).y, 0 );
	Vec3 p4 = Vec3( (endPos + forward - left).x, (endPos + forward - left).y, 0 );
	lineVertsArray[0] = Vertex_PCU( p1, color, Vec2( 0, 0 ) );
	lineVertsArray[1] = Vertex_PCU( p2, color, Vec2( 0, 0 ) );
	lineVertsArray[2] = Vertex_PCU( p3, color, Vec2( 0, 0 ) );
	lineVertsArray[3] = Vertex_PCU( p2, color, Vec2( 0, 0 ) );
	lineVertsArray[4] = Vertex_PCU( p3, color, Vec2( 0, 0 ) );
	lineVertsArray[5] = Vertex_PCU( p4, color, Vec2( 0, 0 ) );

	g_theRenderer->DrawVertexArray( 6, lineVertsArray );
}

void PCGWorld_Log( std::string const& message, const std::source_location& location /*= std::source_location::current() */ )
{
	//DebuggerPrintf( "Debug Msg: %s line %d: %s\n", location.file_name(), location.line(), message.c_str() );
	UNUSED( location );
	DebuggerPrintf( "Debug Msg: %s\n", message.c_str() );
	GetCurMap()->m_generationLog.append( message + "\n" );
}

Map* GetCurMap()
{
	return g_theGame->m_map;
}

MapGenerationSettings const& GetCurGenerationSettings()
{
	return g_theGame->m_map->m_generationSettings;
}

StarEdge* GetSharedEdge( MapPolygonUnit* unit1, MapPolygonUnit* unit2 )
{
	std::vector<StarEdge*>* edges = nullptr;
	MapPolygonUnit* testUnit = nullptr;
	if (unit1->m_edges.size() < unit2->m_edges.size()) {
		edges = &unit1->m_edges;
		testUnit = unit2;
	}
	else {
		edges = &unit2->m_edges;
		testUnit = unit1;
	}

	for (int i = 0; i < (int)edges->size(); i++) {
		if ((*edges)[i]->m_opposite->m_owner == testUnit) {
			return (*edges)[i];
		}
	}
	return nullptr;
}

std::string g_monthShortNameMap[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
std::string g_monthFullNameMap[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

const char* g_productNameMapC[]{ "Fish", "Grain", "Fruit", "Sugar", "Salt", "Livestock", "Wax", "Fur", "Wood", "Ivory", "Cotton", "Wool",
"Iron", "Gold", "Copper", "Silver", "Wine", "Spice", "Tea", "Coffee", "Tobacco", "Glass", "Silk", "Jade", "Porcelain", "Cloth", "Gem",
"WarHorse", "Sword", "Nothing" };

const char* g_landformNameMapC[] = {
	"Ocean", "Land", "Lake", "Island", "Lowland Plain", "Highland Plain", "Grassland", "Savanna", "Marsh", "Rainforest", "Temperate Forest",
	"Subarctic Forest", "Subtropical Forest", "Lowland Hill", "Highland Hill", "Mountain", "Icefield", "Tundra", "Desert",
};

const char* g_climateNameMapC[] = {
	"Tropical Rainforest", "Tropical Monsoon", "Tropical Savanna", "Hot Desert", "Cold Desert", "Mediterranean Climate", "Humid Subtropical",
	"Oceanic", "Humid Monsoon", "Humid Continental", "Subarctic", "Tundra", "Ice Cap", "Water", "None",
};

const char* g_cityAttributeNameMapC[] = {
	"Commercial", "Fort", "Capital", "Port", "AdjToRiver", "AdjToSea",
};

const char* g_governmentTypeDict[] = { "Autocracy", "Parliamentarism", "Nomad", "Separatism", "Oligarchy", "None" };

const char* g_cultureTraitsName[] = {
	"Industrious",
	"Commercial", /*+10% city tax*/
	"Agrarian", /*+10% population growth*/
	//"Ingenious", /**/
	"Thrifty", /*-1% cost as country culture*/
	"Traditional", /*+5% tax, -5% city tax*/
	"Docile", /*-50% culture conflict monthly progress as province culture*/
	"Quarrelsome", /*-5% combat damage as country culture*/
	"Irritable", /*+10% combat damage +10% damage sustained as country culture*/
	"Filial", /*(+5% population growth -30% civil war monthly progress) as country culture*/
	"Free", /*+50% culture conflict monthly progress as province culture*/
	"Alarmist", /*-10% production*/
	"Unruly", /*-20% assimilation rate to this culture*/
	"Nomadic", /*(-10% population growth +10% combat damage) as country culture*/
	"Sedentary", /*+5% population growth as province culture -10% assimilation ability as country culture*/
	"Communal", /*+5% population growth -5% damage sustained as country culture*/
	//"Conformists", /**/
	//"Deviants", /**/
	"Vulnerable", /*+10% damage sustained as country culture*/
	//"Decadent", /**/
	"Resilient", /*+20% population growth in province with less population density*/
	"Miserly", /*(+5% tax, +50% relation instruction cost) as country culture*/
	"Matrilineal", /*+5% tax as province culture, +5% combat damage as country culture*/
	"Conservationist", /*-5% production, +5% tax as country culture*/
	"Jinxed", /*-5% city tax as province culture*/
	"Egalitarian", /*-10% tax +10% production as province culture (-5% damage sustained as country culture)*/
	"Xenophile", /*-50% relation instruction cost, +20% recruit cost as country culture*/
	"Xenophobe", /*+100% relation instruction cost, more aggressive, -10% recruit cost, -10% army maintenance cost, +20% culture conflict monthly progress as country culture*/
	"Pacifistic", /*-50% relation instruction cost, +50% recruit cost, -20% army maintenance cost as country culture*/
	"Militaristic", /*more aggressive, -10% army maintenance cost as country culture*/
	"Tolerant", /*-20% culture conflict monthly progress -20% religion conflict monthly progress as country culture*/
	"Patriarchy", /*+5% population growth as province culture, +5% production as country culture*/
	"NUM"
	"None",
};

const char* g_cultureTraitsDesc[] = {
	"+10% production income",
	"+10% city tax income",
	"+10% province population growth",
	//Ingenious, /**/
	"-1% cost as country culture",
	"+5% country tax as country culture, -5% city tax as province culture",
	"-50% culture conflict monthly progress as province culture",
	"-5% combat damage as country culture",
	"+10% combat damage, +10% damage sustained as country culture",
	"+5% population growth, -30% civil war monthly progress as country culture",
	"+50% culture conflict monthly progress as province culture",
	"-10% province production",
	"-25% assimilation rate to this culture",
	"-10% population growth, +10% combat damage as country culture",
	"+5% population growth as province culture, -20% assimilation ability as country culture",
	"+5% population growth in cities and towns, -5% damage sustained as country culture",
	//Conformists, /**/
	//Deviants, /**/
	"+10% damage sustained as country culture",
	//Decadent, /**/
	"+20% population growth in province with less population density as province culture",
	"+5% tax, +50% relation instruction cost as country culture",
	"+5% tax as province culture, +5% combat damage as country culture",
	"-5% production, +5% tax as country culture",
	"-5% city tax as province culture",
	"+10% production as province culture, -10% tax, -5% damage sustained as country culture",
	"-50% relation instruction cost, +25% recruit cost as country culture",
	"+100% relation instruction cost, -10% recruit cost, -5% army maintenance cost, +20% culture conflict monthly progress as country culture",
	"-20% relation instruction cost, +50% recruit cost, -10% army maintenance cost as country culture",
	"-5% army maintenance cost as country culture",
	"-20% culture conflict monthly progress, -20% religion conflict monthly progress as country culture",
	"+5% population growth as province culture, +5% production as country culture",
	"Oops, You should not read this text :(",
	"Oops, You should not read this text :(",
};