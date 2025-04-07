#pragma once
#include "Game/VisualTest.hpp"
#include "Engine/Math/EngineMath.hpp"

struct RayCastLineInfo
{
	Vec2 m_startPos;
	Vec2 m_endPos;
};

/// Ray Cast Vs Line Segments
class RayVsLineSeg2DTest : public VisualTest {
public:
	RayVsLineSeg2DTest();
	virtual ~RayVsLineSeg2DTest();
	virtual void StartUp() override;
	virtual void RandomizeTest() override;
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
private:
	std::vector<RayCastLineInfo> m_lineSegs;

	RayCastResult2D m_thisFrameRes;
	RayCastLineInfo* m_thisFrameHitInfo = nullptr;
	float m_moveSpeed = 80.f;
	Vec2 m_rayStartPoint;
	Vec2 m_rayEndPoint;
};

/// Ray Cast Vs AABB2s
class RayVsAABB2DTest : public VisualTest {
public:
	RayVsAABB2DTest();
	virtual ~RayVsAABB2DTest();
	virtual void StartUp() override;
	virtual void RandomizeTest() override;
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
private:
	std::vector<AABB2> m_aabb2s;

	RayCastResult2D m_thisFrameRes;
	AABB2* m_thisFrameHitInfo = nullptr;
	float m_moveSpeed = 80.f;
	Vec2 m_rayStartPoint;
	Vec2 m_rayEndPoint;
};