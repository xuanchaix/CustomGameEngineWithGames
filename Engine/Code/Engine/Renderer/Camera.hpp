#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"

enum class CameraMode {
	Orthographic,
	Perspective,
	NUM
};

class Camera {
public:
	Camera();
	void SetPerspectiveView( float aspect, float fov, float near, float far );
	void SetOrthoView( Vec2 const& inBottomLeft, Vec2 const& inTopRight, float near = 0.f, float far = 1.f );
	Vec2 GetOrthoBottomLeft() const;
	Vec2 GetOrthoTopRight() const;
	void Translate2D( Vec2 const& translation2D );
	void SetTransform( Vec3 const& position, EulerAngles const& orientation );
	Mat44 GetViewMatrix() const;
	void SetCenter( Vec2 const& newCenter );
	Vec2 const GetCursorWorldPosition( const Vec2& NormalizedCursorPos ) const;
	void Scale2D( float xScale, float yScale );
	Vec2 const GetCenter() const;

	Mat44 GetOrthoMatrix() const;
	Mat44 GetPerspectiveMatrix() const;
	Mat44 GetProjectionMatrix() const;

	void SetRenderBasis( Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis );
	Mat44 GetRenderMatrix() const;
	Mat44 GetTransformMatrix() const;

	//Vec2 PerspectiveWorldPosToScreen( Vec3 const& worldPos ) const;
	Vec3 PerspectiveScreenPosToWorld( Vec2 const& screenPos ) const;


	void SetViewPort( AABB2 const& viewPort );
public:
	CameraMode m_mode = CameraMode::Perspective;
	AABB2 m_viewPort; /* The range which renderer shows on screen. */
	AABB2 m_cameraBox;

	float m_orthoNear;
	float m_orthoFar;

	float m_perspectiveAspect;
	float m_perspectiveFov;
	float m_perspectiveNear;
	float m_perspectiveFar;

	Vec3 m_renderIBasis = Vec3( 1.f, 0.f, 0.f );
	Vec3 m_renderJBasis = Vec3( 0.f, 1.f, 0.f );
	Vec3 m_renderKBasis = Vec3( 0.f, 0.f, 1.f );
	Vec3 m_position;
	EulerAngles m_orientation;
};