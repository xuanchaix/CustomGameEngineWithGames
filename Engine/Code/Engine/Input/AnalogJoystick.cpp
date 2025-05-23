#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Math/MathUtils.hpp"

Vec2 AnalogJoystick::GetPosition() const
{
	return m_correctPosition;
}

float AnalogJoystick::GetMagnitude() const
{
	return m_correctPosition.GetLength();
}

float AnalogJoystick::GetRawMagnitude() const
{
	return m_rawPosition.GetLength();
}

float AnalogJoystick::GetOrientationDegrees() const
{
	return Atan2Degrees( m_rawPosition.y, m_rawPosition.x );
}

Vec2 AnalogJoystick::GetRawUncorrectedPosition() const
{
	return m_rawPosition;
}

float AnalogJoystick::GetInnerDeadZoneFraction() const
{
	return m_innerDeadZoneFraction;
}

float AnalogJoystick::GetOuterDeadZoneFraction() const
{
	return m_outerDeadZoneFraction;
}

void AnalogJoystick::Reset()
{
	m_rawPosition = Vec2( 0.f, 0.f );
	m_correctPosition = Vec2( 0.f, 0.f );
}

void AnalogJoystick::SetDeadZoneThresholds( float normalizedInnerDeadZoneThreshold, float normalizedOuterDeadZoneThreshold )
{
	m_innerDeadZoneFraction = normalizedInnerDeadZoneThreshold;
	m_outerDeadZoneFraction = normalizedOuterDeadZoneThreshold;
}

void AnalogJoystick::UpdatePosition( float rawNormalizedX, float rawNormalizedY )
{
	m_rawPosition = Vec2( rawNormalizedX, rawNormalizedY );
	float orientationDegree = GetOrientationDegrees();
	float magnitude = GetRawMagnitude();
	magnitude = RangeMapClamped( magnitude, m_innerDeadZoneFraction, m_outerDeadZoneFraction, 0.f, 1.f );
	m_correctPosition = Vec2::MakeFromPolarDegrees( orientationDegree, magnitude );

}
