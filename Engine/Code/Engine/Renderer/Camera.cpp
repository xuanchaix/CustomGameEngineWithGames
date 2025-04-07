#define WIN32_LEAN_AND_MEAN	
#include <windows.h>
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"

#undef far
#undef near

Camera::Camera()
{
	HWND wh = ::GetActiveWindow();
	RECT clientRect;
	::GetWindowRect( (HWND)wh, &clientRect );
	float xLength = (float)clientRect.right - (float)clientRect.left;
	float yLength = (float)clientRect.bottom - (float)clientRect.top;
	SetViewPort( AABB2( Vec2( 0.f, 0.f ), Vec2( xLength, yLength ) ) );
}

void Camera::SetPerspectiveView( float aspect, float fov, float near, float far )
{
	m_perspectiveAspect = aspect;
	m_perspectiveFar = far;
	m_perspectiveFov = fov;
	m_perspectiveNear = near;
	m_mode = CameraMode::Perspective;
}

void Camera::SetOrthoView( Vec2 const& inBottomLeft, Vec2 const& inTopRight, float near /*= 0.f*/, float far /*= 1.f */ )
{
	m_cameraBox.m_mins = inBottomLeft;
	m_cameraBox.m_maxs = inTopRight;
	m_orthoFar = far;
	m_orthoNear = near;
	m_mode = CameraMode::Orthographic;
}

Vec2 Camera::GetOrthoBottomLeft() const
{
	return m_cameraBox.m_mins;
}

Vec2 Camera::GetOrthoTopRight() const
{
	return m_cameraBox.m_maxs;
}

void Camera::Translate2D( Vec2 const& translation2D )
{
	m_cameraBox.m_mins += translation2D;
	m_cameraBox.m_maxs += translation2D;
}

void Camera::SetTransform( Vec3 const& position, EulerAngles const& orientation )
{
	m_position = position;
	m_orientation = orientation;
}

Mat44 Camera::GetViewMatrix() const
{
	//Mat44 retMat = Mat44( Vec3( 0.f, 0.f, 1.f ), Vec3( -1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 0.f ) );
	Mat44 renderMat = GetRenderMatrix();
	Mat44 viewMatrix = Mat44::CreateTranslation3D( Vec3( m_position.x, m_position.y, m_position.z ) );
	viewMatrix.Append( m_orientation.GetAsMatrix_IFwd_JLeft_KUp() );
	renderMat.Append( viewMatrix.GetOrthonormalInverse() );
	return renderMat;
}

void Camera::SetCenter( Vec2 const& newCenter )
{
	m_cameraBox.SetCenter( newCenter );
}

Vec2 const Camera::GetCursorWorldPosition( const Vec2& NormalizedCursorPos ) const
{
	return m_cameraBox.GetPointAtUV( NormalizedCursorPos );
}

void Camera::Scale2D( float xScale, float yScale )
{
	float xSize = m_cameraBox.m_maxs.x - m_cameraBox.m_mins.x;
	float ySize = m_cameraBox.m_maxs.y - m_cameraBox.m_mins.y;
	Vec2 center = GetCenter();
	m_cameraBox.m_maxs = center + Vec2( xSize * 0.5f * xScale, ySize * 0.5f * yScale );
	m_cameraBox.m_mins = center - Vec2( xSize * 0.5f * xScale, ySize * 0.5f * yScale );
}

Vec2 const Camera::GetCenter() const
{
	return (m_cameraBox.m_maxs + m_cameraBox.m_mins) * 0.5f;
}

Mat44 Camera::GetOrthoMatrix() const
{
	/*Mat44 retMat;
	retMat.m_values[Mat44::Ix] = 2.f / (m_cameraBox.m_maxs.x - m_cameraBox.m_mins.x);
	retMat.m_values[Mat44::Jy] = 2.f / (m_cameraBox.m_maxs.y - m_cameraBox.m_mins.y);
	retMat.m_values[Mat44::Kz] = 2.f / (m_orthoNear - m_orthoFar);
	retMat.m_values[Mat44::Tx] = -(m_cameraBox.m_maxs.x + m_cameraBox.m_mins.x) / (m_cameraBox.m_maxs.x - m_cameraBox.m_mins.x);
	retMat.m_values[Mat44::Ty] = -(m_cameraBox.m_maxs.y + m_cameraBox.m_mins.y) / (m_cameraBox.m_maxs.y - m_cameraBox.m_mins.y);
	retMat.m_values[Mat44::Tz] = -(m_orthoFar + m_orthoNear) / (m_orthoFar - m_orthoNear);
	return retMat;*/
	return Mat44::CreateOrthoProjection( m_cameraBox.m_mins.x, m_cameraBox.m_maxs.x, m_cameraBox.m_mins.y, m_cameraBox.m_maxs.y, m_orthoNear, m_orthoFar );
}

Mat44 Camera::GetPerspectiveMatrix() const
{
	return Mat44::CreatePerspectiveProjection( m_perspectiveFov, m_perspectiveAspect, m_perspectiveNear, m_perspectiveFar );
	/*Mat44 retMat;
	float s = CosDegrees( m_perspectiveFov * 0.5f ) / SinDegrees( m_perspectiveFov * 0.5f );
	retMat.m_values[Mat44::Ix] = s / m_perspectiveAspect;
	retMat.m_values[Mat44::Jy] = s;
	retMat.m_values[Mat44::Kz] = m_perspectiveFar / (m_perspectiveFar - m_perspectiveNear);
	retMat.m_values[Mat44::Kw] = 1;
	retMat.m_values[Mat44::Tz] = -m_perspectiveFar * m_perspectiveNear / (m_perspectiveFar - m_perspectiveNear);
	retMat.m_values[Mat44::Tw] = 0;
	return retMat;*/
}

Mat44 Camera::GetProjectionMatrix() const
{
	if (m_mode == CameraMode::Orthographic) {
		return GetOrthoMatrix();
	}
	else if (m_mode == CameraMode::Perspective) {
		//Mat44 retMat = GetPerspectiveMatrix();
		//retMat.Append( GetOrthoMatrix() );
		//return retMat;
		return GetPerspectiveMatrix();
	}
	return GetOrthoMatrix();
}

void Camera::SetRenderBasis( Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis )
{
	m_renderIBasis = iBasis;
	m_renderJBasis = jBasis;
	m_renderKBasis = kBasis;
}

Mat44 Camera::GetRenderMatrix() const
{
	return Mat44( m_renderIBasis, m_renderJBasis, m_renderKBasis, Vec3( 0, 0, 0 ) );
}

Mat44 Camera::GetTransformMatrix() const
{
	Mat44 retMat = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	retMat.SetTranslation3D( m_position );
	return retMat;
}

/*Vec2 Camera::PerspectiveWorldPosToScreen(Vec3 const& worldPos) const
{
	Vec3 iBasis, jBasis, kBasis;
	m_orientation.GetAsVectors_IFwd_JLeft_KUp( iBasis, jBasis, kBasis );
	float h = 2.f * m_perspectiveFar * SinDegrees( m_perspectiveFov * 0.5f ) / CosDegrees( m_perspectiveFov * 0.5f );
	float w = h * m_perspectiveAspect;
	float dist = GetDistance3D( worldPos, m_position );
	Vec3 worldPosCenter = m_position + iBasis * dist;

	//worldPos = worldPosCenter - jBasis * (screenPos.x - 0.5f) * w + kBasis * (screenPos.y - 0.5f) * h;
	//Vec2 screenPos;
	return worldPos;
}*/

Vec3 Camera::PerspectiveScreenPosToWorld( Vec2 const& screenPos ) const
{
	// make sure this is normalized to 0-1
	Vec3 iBasis, jBasis, kBasis;
	m_orientation.GetAsVectors_IFwd_JLeft_KUp( iBasis, jBasis, kBasis );
	Vec3 worldPos = m_position + iBasis * m_perspectiveNear;
	float h = 2.f * m_perspectiveNear * SinDegrees( m_perspectiveFov * 0.5f ) / CosDegrees( m_perspectiveFov * 0.5f );
	float w = h * m_perspectiveAspect;
	worldPos -= jBasis * (screenPos.x - 0.5f) * w;
	worldPos += kBasis * (screenPos.y - 0.5f) * h;
	return worldPos;
}

void Camera::SetViewPort( AABB2 const& viewPort )
{
	float prevXWidth = m_viewPort.m_maxs.x - m_viewPort.m_mins.x;
	float prevYHeight = m_viewPort.m_maxs.y - m_viewPort.m_mins.y;
	float prevAspect = prevXWidth / prevYHeight;

	float newXWidth = viewPort.m_maxs.x - viewPort.m_mins.x;
	float newYHeight = viewPort.m_maxs.y - viewPort.m_mins.y;
	float newAspect = newXWidth / newYHeight;
	m_perspectiveAspect = m_perspectiveAspect / prevAspect * newAspect;
	m_viewPort = viewPort;
}

