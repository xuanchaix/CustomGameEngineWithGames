#pragma once
#include "Game/GameCommon.hpp"
//---------------------
// abandoned!!!!! Do not use!
struct GridRayCastResult {
public:
	Vec2 fromPoint;
	Vec2 stopPoint;
	float orientationDegrees;
	float length;
	float maxLength;

};

//---------------------
// abandoned!!!!! Do not use!
class RayCast {
public:
	RayCast();
	RayCast( Vec2 const& startPoint, Vec2 const& iBasisNormal, float maxRange = 10.f );
	RayCast( Vec2 const& startPoint, float orientationDegrees, float maxRange = 10.f );
	//RayCast( Vec2 startPoint, Vec2 targetPoint, float maxRange = 10.f );

	float GetOrientationDegrees() const;
	Vec2 const GetiBasisNormal() const;
	float GetYFromX( float x ) const;
	float GetXFromY( float y ) const;

public:
	float m_a;
	float m_b;
	float m_inversedA;
	float m_inversedB;
	float m_c;
	Vec2 m_startPosition;
	float m_maxRange;
private:
	Vec2 m_iBasisNormal;
	float m_orientationDegees;
};
