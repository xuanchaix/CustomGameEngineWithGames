#pragma once

struct IntVec3 {
public:
	int x = 0;
	int y = 0;
	int z = 0;

public:
	IntVec3() {}
	IntVec3( IntVec3 const& copyFrom );
	explicit IntVec3( int initialX, int initialY, int initialZ );
	~IntVec3() {}

	// bool SetFromText( char const* text );

	// Accessors
	float GetLength() const;
	int GetTaxicabLength() const;
	int GetLengthSquared() const;

	// Operators (const)
	bool operator==( IntVec3 const& compare ) const;
	IntVec3 const operator+( IntVec3 const& vecToAdd ) const;
	IntVec3 const operator-( IntVec3 const& vecToSub ) const;
	bool operator!=( IntVec3 const& compare ) const;
	IntVec3 const operator-() const;
	IntVec3 const operator*( int uniformScale ) const;
	IntVec3 const operator*( IntVec3 const& vecToMultiply ) const;
	IntVec3 const operator/( int inverseScale ) const;

	// Operators (self-mutating / non-const)
	void operator+=( IntVec3 const& vecToAdd );
	void operator-=( IntVec3 const& vecToSubtract );
	void operator*=( int const uniformScale );
	void operator/=( int const uniformDivisor );
	void operator=( IntVec3 const& copyFrom );


	// Standalone "friend" functions that are conceptually, but not actually, part of Vec2::
	friend IntVec3 const operator*( int uniformScale, IntVec3 const& vecToScale );


	static IntVec3 const ZERO;
};
