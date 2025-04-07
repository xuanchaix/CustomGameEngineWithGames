#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

EulerAngles::EulerAngles( float yawDegrees, float pitchDegrees, float rollDegrees )
	:m_yawDegrees(yawDegrees), m_pitchDegrees(pitchDegrees), m_rollDegrees(rollDegrees)
{

}

bool EulerAngles::SetFromText( char const* text )
{
	Strings strs;
	strs.reserve( 4 );
	int numOfStrings = SplitStringOnDelimiter( strs, text, ',' );
	if (numOfStrings != 3) {
		return false;
	}
	try
	{
		m_yawDegrees = stof( strs[0] );
		m_pitchDegrees = stof( strs[1] );
		m_rollDegrees = stof( strs[2] );
	}
	catch (std::exception const& e)
	{
		UNUSED( e );
		return false;
	}
	return true;
}

void EulerAngles::GetAsVectors_IFwd_JLeft_KUp( Vec3& out_IVector, Vec3& out_JVector, Vec3& out_KVector ) const
{
	float sy = SinDegrees( m_yawDegrees );
	float cy = CosDegrees( m_yawDegrees );
	float sp = SinDegrees( m_pitchDegrees );
	float cp = CosDegrees( m_pitchDegrees );
	float sr = SinDegrees( m_rollDegrees );
	float cr = CosDegrees( m_rollDegrees );

	out_IVector.x = cy * cp;
	out_IVector.y = sy * cp;
	out_IVector.z = -sp;

	out_JVector.x = -sy * cr + cy * sp * sr;
	out_JVector.y = cy * cr + sr * sy * sp;
	out_JVector.z = cp * sr;

	out_KVector.x = sr * sy + cy * sp * cr;
	out_KVector.y = -cy * sr + cr * sy * sp;
	out_KVector.z = cp * cr;
}

Mat44 EulerAngles::GetAsMatrix_IFwd_JLeft_KUp() const
{
	float sy = SinDegrees( m_yawDegrees );
	float cy = CosDegrees( m_yawDegrees );
	float sp = SinDegrees( m_pitchDegrees );
	float cp = CosDegrees( m_pitchDegrees );
	float sr = SinDegrees( m_rollDegrees );
	float cr = CosDegrees( m_rollDegrees );

	Mat44 mat = Mat44();
	mat.m_values[Mat44::Ix] = cy * cp;
	mat.m_values[Mat44::Iy] = sy * cp;
	mat.m_values[Mat44::Iz] = -sp;

	mat.m_values[Mat44::Jx] = -sy * cr + cy * sp * sr;
	mat.m_values[Mat44::Jy] = cy * cr + sr * sy * sp;
	mat.m_values[Mat44::Jz] = cp * sr;

	mat.m_values[Mat44::Kx] = sr * sy + cy * sp * cr;
	mat.m_values[Mat44::Ky] = -cy * sr + cr * sy * sp;
	mat.m_values[Mat44::Kz] = cp * cr;
	return mat;
}

Mat44 EulerAngles::GetAsInversedMatrix_IFwd_JLeft_KUp() const
{
	float sy = SinDegrees( m_yawDegrees );
	float cy = CosDegrees( m_yawDegrees );
	float sp = SinDegrees( m_pitchDegrees );
	float cp = CosDegrees( m_pitchDegrees );
	float sr = SinDegrees( m_rollDegrees );
	float cr = CosDegrees( m_rollDegrees );

	Mat44 mat = Mat44();
	mat.m_values[Mat44::Ix] = cy * cp;
	mat.m_values[Mat44::Jx] = sy * cp;
	mat.m_values[Mat44::Kx] = -sp;

	mat.m_values[Mat44::Iy] = -sy * cr + cy * sp * sr;
	mat.m_values[Mat44::Jy] = cy * cr + sr * sy * sp;
	mat.m_values[Mat44::Ky] = cp * sr;

	mat.m_values[Mat44::Iz] = sr * sy + cy * sp * cr;
	mat.m_values[Mat44::Jz] = -cy * sr + cr * sy * sp;
	mat.m_values[Mat44::Kz] = cp * cr;
	return mat;
}

Vec3 const EulerAngles::GetIFwd() const
{
	Vec3 retVec;
	float sy = SinDegrees( m_yawDegrees );
	float cy = CosDegrees( m_yawDegrees );
	float sp = SinDegrees( m_pitchDegrees );
	float cp = CosDegrees( m_pitchDegrees );

	retVec.x = cy * cp;
	retVec.y = sy * cp;
	retVec.z = -sp;
	return retVec;
}

Vec3 const EulerAngles::GetJLeft() const
{
	Vec3 retVec;
	float sy = SinDegrees( m_yawDegrees );
	float cy = CosDegrees( m_yawDegrees );
	float sp = SinDegrees( m_pitchDegrees );
	float cp = CosDegrees( m_pitchDegrees );
	float sr = SinDegrees( m_rollDegrees );
	float cr = CosDegrees( m_rollDegrees );

	retVec.x = -sy * cr + cy * sp * sr;
	retVec.y = cy * cr + sr * sy * sp;
	retVec.z = cp * sr;

	return retVec;
}

Vec3 const EulerAngles::GetKUp() const
{
	Vec3 retVec;
	float sy = SinDegrees( m_yawDegrees );
	float cy = CosDegrees( m_yawDegrees );
	float sp = SinDegrees( m_pitchDegrees );
	float cp = CosDegrees( m_pitchDegrees );
	float sr = SinDegrees( m_rollDegrees );
	float cr = CosDegrees( m_rollDegrees );

	retVec.x = sr * sy + cy * sp * cr;
	retVec.y = -cy * sr + cr * sy * sp;
	retVec.z = cp * cr;

	return retVec;
}
