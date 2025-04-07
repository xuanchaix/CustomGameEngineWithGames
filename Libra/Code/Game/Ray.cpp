#include "Game/Ray.hpp"

RayCast::RayCast()
{

}

RayCast::RayCast( Vec2 const& startPoint, Vec2 const& iBasisNormal, float maxRange /*= 10.f */ )
	:m_startPosition(startPoint),
	m_iBasisNormal(iBasisNormal),
	m_maxRange(maxRange)
{
	m_orientationDegees = m_iBasisNormal.GetOrientationDegrees();
	m_a = m_iBasisNormal.y;
	m_b = -m_iBasisNormal.x;
	m_inversedA = 1.f / m_a;
	m_inversedB = 1.f / m_b;
	m_c = m_iBasisNormal.x * startPoint.y - m_iBasisNormal.y * startPoint.x;
}

RayCast::RayCast( Vec2 const& startPoint, float orientationDegrees, float maxRange /*= 10.f */ )
	:m_startPosition( startPoint ),
	m_orientationDegees( orientationDegrees ),
	m_maxRange( maxRange )
{
	while (m_orientationDegees >= 180.f) {
		m_orientationDegees -= 360.f;
	}
	while (m_orientationDegees <= -180.f) {
		m_orientationDegees += 360.f;
	}
	m_iBasisNormal = Vec2::MakeFromPolarDegrees( m_orientationDegees );
	m_a = m_iBasisNormal.y;
	m_b = -m_iBasisNormal.x;
	m_inversedA = 1.f / m_a;
	m_inversedB = 1.f / m_b;
	m_c = m_iBasisNormal.x * startPoint.y - m_iBasisNormal.y * startPoint.x;
}

float RayCast::GetOrientationDegrees() const
{
	return m_orientationDegees;
}

Vec2 const RayCast::GetiBasisNormal() const
{
	return m_iBasisNormal;
}

float RayCast::GetYFromX(float x) const
{
	if (m_b == 0.f) {
		return -1;
	}
	return -(m_c + m_a * x) * m_inversedB;
}

float RayCast::GetXFromY(float y) const
{
	if (m_a == 0.f) {
		return -1;
	}
	return -(m_c + m_b * y) * m_inversedA;
}
