#pragma once
//-------------Rgba8.hpp .cpp 8.23.2023-----------------
#include "Engine/Math/Vec4.hpp"

// -----------------------------------------------------
// Rgba8 struct is a struct represents a color
// r - red, g - green, b - blue, a - alpha(transparency)
struct Rgba8
{
public:
	unsigned char r = 255;
	unsigned char g = 255;
	unsigned char b = 255;
	unsigned char a = 255;

public:
	Rgba8();
	explicit Rgba8( unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255 );
	explicit Rgba8( Vec4 const& colorAsFloats );
	~Rgba8();
	bool SetFromText( char const* text );
	bool operator==( Rgba8 const& compare ) const;
	bool operator<( Rgba8 const& compare ) const;
	Rgba8 operator*( float floatToMultiply ) const;

	static Rgba8 Interpolate( Rgba8 const& start, Rgba8 const& end, float fraction );
	void GetAsFloats( float* colorAsFloats ) const;
	Vec4 GetAsFloats() const;

	static Rgba8 const WHITE;
	static Rgba8 const BLACK;
	static Rgba8 const RED;
};
