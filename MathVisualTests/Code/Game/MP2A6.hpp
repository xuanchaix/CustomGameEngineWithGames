#pragma once
#include "Game/VisualTest.hpp"
#include "Game/GameCommon.hpp"

class PachinkoMachine2DTest;

struct PachinkoBilliard {
	virtual ~PachinkoBilliard();
	virtual void Startup();
	virtual void UpdatePhysics( float deltaSeconds );
	virtual void AddVerts( std::vector<Vertex_PCU>& verts );
	Vec2 m_position;
	float m_radius;
	float m_elasticity;
	Vec2 m_velocity;
	PachinkoMachine2DTest* m_test = nullptr;
	Rgba8 m_color;
	int m_inBoxIndex = -1;
};


struct PachinkoShape {
	virtual ~PachinkoShape();
	virtual void BounceOff( PachinkoBilliard* mobileBilliard ) = 0;
	virtual void AddVerts( std::vector<Vertex_PCU>& verts ) = 0;
	float m_elasticity;
};

struct PachinkoShapeDisc : public PachinkoShape {
	virtual void BounceOff( PachinkoBilliard* mobileBilliard ) override;
	virtual void AddVerts( std::vector<Vertex_PCU>& verts ) override;
	Vec2 m_center;
	float m_radius;
};

struct PachinkoShapeOBB : public PachinkoShape {
	virtual void BounceOff( PachinkoBilliard* mobileBilliard ) override;
	virtual void AddVerts( std::vector<Vertex_PCU>& verts ) override;
	OBB2 m_box;
};

struct PachinkoShapeCapsule : public PachinkoShape {
	virtual void BounceOff( PachinkoBilliard* mobileBilliard ) override;
	virtual void AddVerts( std::vector<Vertex_PCU>& verts ) override;
	Vec2 m_startPos;
	Vec2 m_endPos;
	float m_radius;
};

class PachinkoMachine2DTest : public VisualTest {
public:
	PachinkoMachine2DTest();
	virtual ~PachinkoMachine2DTest();
	virtual void StartUp() override;
	virtual void RandomizeTest() override;
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
public:
	std::vector<PachinkoShape*> m_blockShapes;
	std::vector<PachinkoBilliard*> m_billiards;
private:
	void UpdatePhysics( float deltaSeconds );
private:
	float m_physicsSimulationTime = 0.005f;
	bool m_isFloorExist = true;
	bool m_isUsingNormalUpdate = false;
	Vec2 m_rayStart;
	Vec2 m_rayEnd;
	float m_maxRadius = 10.f;
	float m_minRadius = 3.f;
	float m_minElasticity = 0.1f;
	float m_maxElasticity = 0.9f;
	float m_physicsSecondsDebt = 0.f;
	float m_deltaSecondsThisFrame = 0.f;

	IntVec2 m_dimensionOfBoundingBoxes;
	std::vector<AABB2> m_boundingBoxes;
	std::vector<Vertex_PCU> m_verts;
};