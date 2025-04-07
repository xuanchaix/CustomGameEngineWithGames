#pragma once
#include "Game/VisualTest.hpp"
#include "Engine/Math/EngineMath.hpp"

/// Get Nearest Point/ Is Point Inside
class MP1A5 : public VisualTest {
public:
	MP1A5();
	virtual ~MP1A5();
	virtual void StartUp() override;
	virtual void RandomizeTest() override;
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
private:
	float m_moveSpeed = 80.f;
	Vec2 m_testPoint;
	Vec2 m_discCenter;
	float m_discRadius;
	AABB2 m_aabbBox;
	OBB2 m_obbBox;
	Vec2 m_capsuleBoneStartPoint;
	Vec2 m_capsuleBoneEndPoint;
	float m_capsuleRadius;
	Vec2 m_lineSegmentStartPoint;
	Vec2 m_lineSegmentEndPoint;
	Vec2 m_infiniteLineStartPoint;
	Vec2 m_infiniteLineEndPoint;
	Vec2 m_sectorCenter;
	float m_sectorForwardDegrees;
	float m_sectorApertureDegrees;
	float m_sectorRadius;
};