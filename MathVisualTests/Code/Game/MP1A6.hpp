#pragma once
#include "Game/VisualTest.hpp"
#include "Engine/Math/EngineMath.hpp"

struct RayCastDiscInfo
{
	Vec2 m_center;
	float m_radius;
};

/// Ray Cast Vs Discs
class MP1A6 : public VisualTest {
public:
	MP1A6();
	virtual ~MP1A6();
	virtual void StartUp() override;
	virtual void RandomizeTest() override;
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
private:
	std::vector<RayCastDiscInfo> discs;

	RayCastResult2D m_thisFrameRes;
	Vec2 m_thisFrameHitCenter;
	float m_moveSpeed = 80.f;
	Vec2 m_rayStartPoint;
	Vec2 m_rayEndPoint;
};