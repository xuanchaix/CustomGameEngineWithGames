#pragma once
#include "Engine/Math/Mat44.hpp"

struct Vec3;

struct EulerAngles {
public:
	EulerAngles() = default;
	EulerAngles( float yawDegrees, float pitchDegrees, float rollDegrees );
	bool SetFromText( char const* text );

	void GetAsVectors_IFwd_JLeft_KUp( Vec3& out_IVector, Vec3& out_JVector, Vec3& out_KVector ) const;
	Mat44 GetAsMatrix_IFwd_JLeft_KUp() const;
	Mat44 GetAsInversedMatrix_IFwd_JLeft_KUp() const;
	Vec3 const GetIFwd() const;
	Vec3 const GetJLeft() const;
	Vec3 const GetKUp() const;
public:
	float m_yawDegrees = 0.f;
	float m_pitchDegrees = 0.f;
	float m_rollDegrees = 0.f;
};