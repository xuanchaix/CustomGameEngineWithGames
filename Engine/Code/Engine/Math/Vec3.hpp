#pragma once

//-----------------------------------------------------------------------------------------------
struct Vec2;
struct EulerAngles;
struct Mat44;
struct IntVec3;

struct Vec3 
{
public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

public:
	Vec3() {}
	explicit Vec3( const float x, const float y, const float z );
	Vec3( Vec3 const& copyFrom );
	Vec3( Vec2 const& copyFrom, float z = 0.f );
	Vec3( IntVec3 const& copyFrom );
	~Vec3() {}
	bool SetFromText( char const* text );

	// static methods
	static Vec3 const MakeFromPolarRadians( float latitudeRadians, float longitudeRadians, float length = 1.f );
	static Vec3 const MakeFromPolarDegrees( float latitudeDegrees, float longitudeDegrees, float length = 1.f );

	// Accessors
	float GetLength() const;
	float GetLengthXY() const;
	float GetLengthSquared() const;
	float GetLengthXYSquared() const;
	float GetAngleAboutZRadians() const;
	float GetAngleAboutZDegrees() const;
	Vec3 const GetRotatedAboutZRadians( float deltaRadians ) const;
	Vec3 const GetRotatedAboutZDegrees( float deltaDegrees ) const;
	Vec3 const GetClamped( float maxLength ) const;
	Vec3 const GetNormalized() const;
	EulerAngles GetOrientationEulerAngles() const;
	Mat44 GetOrientationMatrix() const;

	void Normalize();
	float NormalizeAndGetPreviousLength();

	// Operators
	bool operator==( Vec3 const& compare ) const;
	bool operator!=( Vec3 const& compare ) const;
	Vec3 const operator+( Vec3 const& VecToAdd ) const;
	Vec3 const operator-( Vec3 const& vecToSubtract ) const;
	Vec3 const operator*( float uniformScale ) const;
	Vec3 const operator*( const Vec3& vecToMultiply ) const;
	Vec3 const operator/( float inverseScale ) const;
	Vec3 const operator-() const;

	// Operators(self-mutating)
	void operator+=( Vec3 const& vecToAdd );
	void operator-=( Vec3 const& vecToSubtract );
	void operator*=( float uniformScale );
	void operator/=( float uniformDivisor );
	void operator=( Vec3 const& copyfrom );
	void operator=( Vec2 const& copyfrom );
	// friend functions
	friend Vec3 const operator*( float uniformScale, Vec3 const& vecToScale );

	static Vec3 const Zero;
	static Vec3 const One;
	static Vec3 const XAxis;
	static Vec3 const YAxis;
	static Vec3 const ZAxis;
	static Vec3 const Forward;
	static Vec3 const Backward;
	static Vec3 const Left;
	static Vec3 const Right;
	static Vec3 const Up;
	static Vec3 const Down;
};
