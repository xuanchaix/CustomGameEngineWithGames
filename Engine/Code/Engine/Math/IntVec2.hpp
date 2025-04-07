#pragma once

struct IntVec3;

struct IntVec2 {
public:
	int x = 0;
	int y = 0;

public:
	IntVec2() {}
	IntVec2( IntVec2 const& copyFrom );
	IntVec2( IntVec3 const& copyFrom );
	explicit IntVec2( int initialX, int initialY );
	~IntVec2() {}

	bool SetFromText( char const* text );

	// Accessors
	float GetLength() const;
	int GetTaxicabLength() const;
	int GetLengthSquared() const;
	float GetOrientationRadians() const;
	float GetOrientationDegrees() const;
	IntVec2 const GetRotated90Degrees() const;
	IntVec2 const GetRotatedMinus90Degrees() const;
	
	// Mutators
	void Rotate90Degrees();
	void RotateMinus90Degrees();

	// Operators (const)
	bool operator==( IntVec2 const& compare ) const;
	IntVec2 const operator+( IntVec2 const& vecToAdd ) const;
	IntVec2 const operator-( IntVec2 const& vecToSub ) const;
	bool operator!=( IntVec2 const& compare ) const;
	IntVec2 const operator-() const;
	IntVec2 const operator*( int uniformScale ) const;
	IntVec2 const operator*( IntVec2 const& vecToMultiply ) const;
	IntVec2 const operator/( int inverseScale ) const;

	// Operators (self-mutating / non-const)
	void operator+=( IntVec2 const& vecToAdd );
	void operator-=( IntVec2 const& vecToSubtract );
	void operator*=( int const uniformScale );
	void operator/=( int const uniformDivisor );
	void operator=( IntVec2 const& copyFrom );


	// Standalone "friend" functions that are conceptually, but not actually, part of Vec2::
	friend IntVec2 const operator*( int uniformScale, IntVec2 const& vecToScale );


	static IntVec2 const ZERO;
};
