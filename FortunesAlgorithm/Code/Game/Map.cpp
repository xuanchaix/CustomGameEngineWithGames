#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/VoronoiHelper.hpp"

static FortuneAlgorithmSolverClass fortuneSolver;
static std::vector<Vec2> randomPoints;
Map::Map()
{

}

Map::~Map()
{

}

void Map::Startup()
{
	g_engineRNG->SetSeed( 23 );
	PopulateMapWithPolygons();
}

void Map::Update()
{
	static int count = 1;
	if (count > 0) {
		for (int i = 0; i < 10; i++) {
			fortuneSolver.FortuneAlgorithmSingleStep();
			fortuneSolver.m_doNextStep = false;
			--count;
			if (count <= 0) {
				break;
			}
		}
	}
	if (g_theInput->WasKeyJustPressed( 'I' )) {
		fortuneSolver.m_doNextStep = true;
		count += 100000;
	}
}

void Map::Render() const
{
	std::vector<Vertex_PCU> verts;
	for (int i = 0; i < (int)randomPoints.size(); i++) {
		AddVertsForAABB2D( verts, AABB2( randomPoints[i] - Vec2( 0.5f, 0.5f ), randomPoints[i] + Vec2( 0.5f, 0.5f ) ), Rgba8( 255, 0, 0 ) );
	}
	fortuneSolver.AddDebugVerts( verts );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void Map::PopulateMapWithPolygons()
{
	//std::vector<Vec2> randomPoints;
	AABB2 m_bounds = g_theGame->m_worldCamera.m_cameraBox;
	int numOfPolygons = 1000;

	// first generate some random points and make them more average
	/*int numOfRestrictBox = 100;
	std::vector<AABB2> bounds;
	int counter = 0;
	Vec2 size = (m_bounds.m_maxs - m_bounds.m_mins) / (float)numOfRestrictBox;
	for (int i = 0; i < numOfRestrictBox; i++) {
		for (int j = 0; j < numOfRestrictBox; j++) {
			bounds.push_back( AABB2( m_bounds.m_mins + Vec2( i * size.x, j * size.y ), m_bounds.m_mins + Vec2( (i + 1) * size.x, (j + 1) * size.y ) ) );
		}
	}
	while (1) {
		for (int i = 0; i < numOfRestrictBox * numOfRestrictBox; i++) {
			randomPoints.push_back( bounds[i].GetRandomPointInside() );
			counter++;
			if (counter >= numOfPolygons) {
				goto ExitLoop;
			}
		}
	}
ExitLoop:*/
	// first generate some random points
	std::vector<Vec2> outEdges;
	for (int i = 0; i < numOfPolygons; i++) {
		randomPoints.push_back( m_bounds.GetRandomPointInside() );
	}
	randomPoints.push_back( Vec2( -999.f, 50.f ) );
	randomPoints.push_back( Vec2( 999.f, 50.f ) );
	randomPoints.push_back( Vec2( 100.f, -1000.f ) );
	randomPoints.push_back( Vec2( 100.f, 1000.f ) );
	
	fortuneSolver.FortuneAlgorithmStepInit( randomPoints );
	//FortuneAlgorithmSolver( randomPoints, bounds, outEdges );
}
