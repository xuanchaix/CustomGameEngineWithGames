#pragma once
#include "Game/GameCommon.hpp"

enum class TestShapeType {
	DEFAULT,
	Sphere,
	ZCylinder,
	AABB3,
	Plane,
	OBB3,
};

struct TestShape {
	virtual ~TestShape();
	virtual void Update( float deltaSeconds );
	virtual void Render() const = 0;
	virtual Vec3 GetNearestPointOnShape( Vec3 const& refPos ) const = 0;
	virtual bool RayCastVsShape( RayCastResult3D& out_result, Vec3 const& startPos, Vec3 const& forwardVec, float maxDist ) = 0;
	virtual void Translate( Vec3 const& translation ) = 0;
	void CheckOverlap( TestShape* other );
	float CalculateOverlapTimerFraction() const;
	bool m_isWired = false;
	TestShapeType m_type = TestShapeType::DEFAULT;
	bool m_isoverlapped = false;
	float m_overlapTimer = 0.f;
	bool m_isHitByRay = false;
	bool m_isGrabbedByUser = false;
};

struct TestShapeSphere : public TestShape {
	TestShapeSphere();
	Vec3 m_center;
	float m_radius;
	int m_numOfLatitudeSlices;
	int m_numOfLongitudeSlices;

	virtual void Translate( Vec3 const& translation ) override;
	virtual void Render() const override;
	virtual Vec3 GetNearestPointOnShape( Vec3 const& refPos ) const override;
	virtual bool RayCastVsShape( RayCastResult3D& out_result, Vec3 const& startPos, Vec3 const& forwardVec, float maxDist ) override;
};

struct TestShapeAABB3 : public TestShape {
	TestShapeAABB3();
	AABB3 m_box;

	virtual void Translate( Vec3 const& translation ) override;
	virtual void Render() const override;
	virtual Vec3 GetNearestPointOnShape( Vec3 const& refPos ) const override;
	virtual bool RayCastVsShape( RayCastResult3D& out_result, Vec3 const& startPos, Vec3 const& forwardVec, float maxDist ) override;
};

struct TestShapeZCylinder : public TestShape {
	TestShapeZCylinder();
	Vec2 m_center;
	float m_radius;
	float m_minZ;
	float m_maxZ;
	int m_slices;

	virtual void Translate( Vec3 const& translation ) override;
	virtual void Render() const override;
	virtual Vec3 GetNearestPointOnShape( Vec3 const& refPos ) const override;
	virtual bool RayCastVsShape( RayCastResult3D& out_result, Vec3 const& startPos, Vec3 const& forwardVec, float maxDist ) override;
};

struct TestShapePlane : public TestShape {
	TestShapePlane();
	Plane3 m_plane;

	virtual void Translate( Vec3 const& translation ) override;
	virtual void Render() const override;
	virtual Vec3 GetNearestPointOnShape( Vec3 const& refPos ) const override;
	virtual bool RayCastVsShape( RayCastResult3D& out_result, Vec3 const& startPos, Vec3 const& forwardVec, float maxDist ) override;

};

struct TestShapeOBB3 : public TestShape {
	TestShapeOBB3();
	OBB3 m_obb3;

	virtual void Translate( Vec3 const& translation ) override;
	virtual void Render() const override;
	virtual Vec3 GetNearestPointOnShape( Vec3 const& refPos ) const override;
	virtual bool RayCastVsShape( RayCastResult3D& out_result, Vec3 const& startPos, Vec3 const& forwardVec, float maxDist ) override;

};