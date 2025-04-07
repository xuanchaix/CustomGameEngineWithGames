#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/Convex.hpp"
#include "Game/QuadTree.hpp"
#include "Game/BVH.hpp"

class Entity;
class Renderer;
class Clock;


class Game {
public:
	RandomNumberGenerator* m_randNumGen = nullptr;
	Clock* m_gameClock = nullptr;
	Camera m_worldCamera;
	Camera m_screenCamera;

public:
	Game();
	~Game();

	void Startup();
	void Update();
	void Render() const;

	void HandleKeys();

private:
	void RebuildAllTrees();
	void RebuildAABB2Tree();
	void RebuildSymmetricQuadTree();
	void TestRays();

	Convex2* GenerateRandomConvex( int index ) const;
	void AddVertsForConvexPolyEdges( std::vector<Vertex_PCU>& verts, ConvexPoly2 const& convexPoly2, float thickness, Rgba8 const& color ) const;
	void AddVertsForConvexHullPlanes( std::vector<Vertex_PCU>& verts, ConvexHull2 const& convexHull2, float thickness, Rgba8 const& color ) const;

	void WriteTestBuffer();
	void ReadTestBuffer();

	void SaveCurrentConvexScene( std::string const& path );
	void LoadConvexSceneToCurrent( std::string const& path );

	void ReadSaveFileError( size_t location );

	static bool SaveConvexSceneCommand( EventArgs& args );
	static bool LoadConvexSceneCommand( EventArgs& args );

	void ClearScene();

	std::vector<Convex2*> m_convexArray;
	int m_numOfRandomRays = 8192;
	Convex2* m_hoveringConvex = nullptr;
	Vec2 m_cursorPrevPos;
	bool m_isDragging = false;
	bool m_drawEdgesMode = false;
	bool m_debugDrawMode = false;
	bool m_debugDrawBVHMode = false;
	unsigned int m_seed = 1;
	float m_avgDist = 0.f;
	float m_lastRayTestNormalTime = 0.f;
	float m_lastRayTestDiscRejectionTime = 0.f;
	float m_lastRayTestAABBRejectionTime = 0.f;
	float m_lastRayTestSymmetricTreeTime = 0.f;
	float m_lastRayTestAABBTreeTime = 0.f;

	Vec2 m_rayStart;
	Vec2 m_rayEnd;

	SymmetricQuadTree m_symQuadTree;
	AABB2Tree m_AABB2Tree;
};



