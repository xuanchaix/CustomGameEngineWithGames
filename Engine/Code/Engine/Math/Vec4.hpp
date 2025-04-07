#pragma once

struct Vec3;

struct Vec4 {
public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;

public:
	Vec4() {}
	~Vec4() {}
	Vec4( Vec4 const& copyFrom );
	explicit Vec4( float initialX, float initialY, float initialZ, float initialW );
	explicit Vec4( Vec3 const& vector3, float initialW );
	bool SetFromText( char const* text );

	// Operators
	bool operator==( Vec4 const& compare ) const;
	bool operator!=( Vec4 const& compare ) const;
	Vec4 const operator+( Vec4 const& VecToAdd ) const;
	Vec4 const operator-( Vec4 const& vecToSubtract ) const;
	Vec4 const operator*( float uniformScale ) const;
	Vec4 const operator*( const Vec4& vecToMultiply ) const;
	Vec4 const operator/( float inverseScale ) const;
	Vec4 const operator-() const;

	// Operators(self-mutating)
	void operator+=( Vec4 const& vecToAdd );
	void operator-=( Vec4 const& vecToSubtract );
	void operator*=( float uniformScale );
	void operator/=( float uniformDivisor );
	void operator=( Vec4 const& copyfrom );
	// friend functions
	friend Vec4 const operator*( float uniformScale, Vec4 const& vecToScale );
};