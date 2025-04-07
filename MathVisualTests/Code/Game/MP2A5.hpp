#pragma once
#include "Game/VisualTest.hpp"
#include <string>
class CubicBezierCurve2D;
class CatmullRomSpline2D;

/// 2DCurve
class CurveTest2D : public VisualTest {
public:
	CurveTest2D();
	virtual ~CurveTest2D();
	virtual void StartUp() override;
	virtual void RandomizeTest() override;
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
private:
	void RenderEasingCurve( AABB2 const& bounds ) const;
	void RenderBezierCurve( AABB2 const& bounds ) const;
	void RenderSpline( AABB2 const& bounds ) const;
private:
	float m_time = 0.f;
	float m_splineTime = 0.f;
	int m_subdivisions = 64;
	int m_curEasingCurve = 0;
	float(*m_easingFuncs[16])(float) = {};
	std::string m_easingNames[16];
	CubicBezierCurve2D* m_bezierCurve = nullptr;
	CatmullRomSpline2D* m_spline = nullptr;

	bool m_showBounds = false;
};