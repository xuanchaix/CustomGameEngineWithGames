#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

Rgba8 const Rgba8::WHITE = Rgba8( 255, 255, 255, 255 );
Rgba8 const Rgba8::BLACK = Rgba8( 0, 0, 0, 255 );
Rgba8 const Rgba8::RED = Rgba8( 255, 0, 0, 255 );

Rgba8::Rgba8(){}

Rgba8::Rgba8( unsigned char r, unsigned char g, unsigned char b, unsigned char a )
	:r(r)
	,g(g)
	,b(b)
	,a(a)
{
}

Rgba8::Rgba8( Vec4 const& colorAsFloats )
	:r( (unsigned char)(colorAsFloats.x * 255.f) )
	,g( (unsigned char)(colorAsFloats.y * 255.f) )
	,b( (unsigned char)(colorAsFloats.z * 255.f) )
	,a( (unsigned char)(colorAsFloats.w * 255.f) )
{

}

Rgba8::~Rgba8(){}

bool Rgba8::SetFromText( char const* text )
{
	Strings strs;
	strs.reserve( 4 );
	int numOfStrings = SplitStringOnDelimiter( strs, text, ',' );
	if (numOfStrings == 3) {
		r = (unsigned char)stoi( strs[0] );
		g = (unsigned char)stoi( strs[1] );
		b = (unsigned char)stoi( strs[2] );
		a = 255;
	}
	else if (numOfStrings == 4) {
		r = (unsigned char)stoi( strs[0] );
		g = (unsigned char)stoi( strs[1] );
		b = (unsigned char)stoi( strs[2] );
		a = (unsigned char)stoi( strs[3] );
	}
	else {
		return false;
	}

	return true;
}

bool Rgba8::operator==( const Rgba8& compare ) const
{
	return compare.r == r && compare.g == g && compare.b == b && compare.a == a;
}

bool Rgba8::operator<( const Rgba8& compare ) const
{
	unsigned int thisValue = r * 16777216 + g * 65536 + b * 256 + a;
	unsigned int compareValue = compare.r * 16777216 + compare.g * 65536 + compare.b * 256 + compare.a;
	return thisValue < compareValue;
}

Rgba8 Rgba8::operator*( float floatToMultiply ) const
{
	return Rgba8( (unsigned char)(r * floatToMultiply), (unsigned char)(g * floatToMultiply), (unsigned char)(b * floatToMultiply), (unsigned char)(a * floatToMultiply) );
}

Rgba8 Rgba8::Interpolate( Rgba8 const& start, Rgba8 const& end, float fraction )
{
	unsigned char r = (unsigned char)::Interpolate( (float)start.r, (float)end.r, fraction );
	unsigned char g = (unsigned char)::Interpolate( (float)start.g, (float)end.g, fraction );
	unsigned char b = (unsigned char)::Interpolate( (float)start.b, (float)end.b, fraction );
	unsigned char a = (unsigned char)::Interpolate( (float)start.a, (float)end.a, fraction );
	return Rgba8( r, g, b, a );
}

void Rgba8::GetAsFloats( float* colorAsFloats ) const
{
	colorAsFloats[0] = (float)r / 255.f;
	colorAsFloats[1] = (float)g / 255.f;
	colorAsFloats[2] = (float)b / 255.f;
	colorAsFloats[3] = (float)a / 255.f;
}

Vec4 Rgba8::GetAsFloats() const
{
	return Vec4( (float)r / 255.f, (float)g / 255.f, (float)b / 255.f, (float)a / 255.f );
}
