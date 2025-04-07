#pragma once
#include "Vec2.hpp"
/*a.	Member variables are precisely: Vec2 m_center, Vec2 m_iBasisNormal, Vec2 m_halfDimensions
b.	Consider also adding the following methods (not required for MP1-A5):
i.	void GetCornerPoints( Vec2* out_fourCornerWorldPositions ) const; // for drawing, etc.!
ii.	Vec2 GetLocalPosForWorldPos( Vec2 worldPos ) const;
iii.	Vec2 GetWorldPosForLocalPos( Vec2 localPos ) const;
iv.	RotateAboutCenter( float rotationDeltaDegrees );
*/

struct OBB2 {
public:
	OBB2();
	OBB2( Vec2 const& center, Vec2 const& iBasisNormal, Vec2 const& halfDimensions );
	~OBB2();
	Vec2 m_center;
	Vec2 m_iBasisNormal;
	Vec2 m_halfDimensions;

	// Accessors
	Vec2 const GetLocalPosFromWorldPos( Vec2 const& worldPos ) const;
	bool IsPointInside( Vec2 const& point ) const;
	Vec2 const GetNearestPoint( Vec2 const& referencePosition ) const;
	//Vec2 const GetPointAtUV( Vec2 const& uv ) const; // uv = (0, 0) is at mins; uv = (1, 1) is at maxs
	//Vec2 const GetUVForPoint( Vec2 const& point ) const;
	Vec2 GetRandomPointInside() const;

	// Mutators
	void Translate( Vec2 const& translationToApply );
	void SetCenter( Vec2 const& newCenter );
	void SetDimensions( Vec2 const& newHalfDimensions );
	void RotateAboutCenter( float rotationDeltaDegrees );
	//void StretchToIncludePoint( Vec2 const& point );
};