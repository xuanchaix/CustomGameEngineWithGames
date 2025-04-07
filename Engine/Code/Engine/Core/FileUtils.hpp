#pragma once
#include <vector>
#include <string>
#include "Engine/Core/EngineCommon.hpp"

struct Vec2;
struct Vec3;
struct Vec4;
struct IntVec2;
struct IntVec3;
struct IntVec4;
struct Rgba8;
struct AABB2;
struct AABB3;
struct OBB2;
struct OBB3;
struct Plane2;
struct Plane3;
struct Vertex_PCU;

int FileReadToBuffer( std::vector<uint8_t>& out_buffer, std::string const& filename );
int FileReadToString( std::string& out_string, std::string const& filename );

void StringWriteToFile( std::string const& str, std::string const& filename );
void BufferWriteToFile( std::vector<uint8_t> const& buffer, std::string const& filename );


class BufferWriter {
public:
	BufferWriter( std::vector<uint8_t>& bufferToWrite );

	void AppendByte( uint8_t byteToAppend );
	void AppendChar( char charToAppend );
	void AppendUshort( unsigned short ushortToAppend );
	void AppendShort( signed short shortToAppend );
	void AppendUint32( unsigned int uintToAppend );
	void AppendInt32( signed int intToAppend );
	void AppendUint64( uint64_t uintToAppend );
	void AppendInt64( int64_t intToAppend );
	void AppendFloat( float floatToAppend );
	void AppendDouble( double doubleToAppend );
	void AppendBool( bool boolToAppend );

	void AppendZeroTerminatedString( std::string const& strToAppend );
	void AppendLengthPrecededString( std::string const& strToAppend );

	void AppendVec2( Vec2 const& vecToAppend );
	void AppendVec3( Vec3 const& vecToAppend );
	void AppendVec4( Vec4 const& vecToAppend );

	void AppendIntVec2( IntVec2 const& vecToAppend );
	void AppendIntVec3( IntVec3 const& vecToAppend );
	void AppendIntVec4( IntVec4 const& vecToAppend );

	void AppendRgba8( Rgba8 const colorToAppend );
	void AppendRgb8( Rgba8 const colorToAppend );

	void AppendAABB2( AABB2 const& aabb2ToAppend );
	void AppendAABB3( AABB3 const& aabb3ToAppend );

	void AppendOBB2( OBB2 const& obb2ToAppend );
	void AppendOBB3( OBB3 const& obb3ToAppend );

	void AppendPlane2( Plane2 const& plane2ToAppend );
	void AppendPlane3( Plane3 const& plane3ToAppend );

	void AppendVertexPCU( Vertex_PCU const& vertToAppend );

	void WriteIntToPos( size_t startPos, unsigned int intToWrite );

	void SetPreferredEndianMode( EndianMode mode );
	EndianMode GetPreferredEndianMode() const;
	EndianMode GetLocalEndianMode() const;

	size_t GetCurBufferSize() const;
protected:
	void SwitchByte( uint8_t* a, uint8_t* b ) const;

	std::vector<uint8_t>& m_buffer;
	EndianMode m_localEndianMode;
	EndianMode m_preferredEndianMode = EndianMode::Little;
};

class BufferReader {
public:
	BufferReader( uint8_t const* const& buffer, size_t size );
	BufferReader( std::vector<uint8_t> const& bufferToRead );
	uint8_t ParseByte();
	char ParseChar();
	unsigned short ParseUshort();
	signed short ParseShort();
	unsigned int ParseUint32();
	signed int ParseInt32();
	uint64_t ParseUint64();
	int64_t ParseInt64();
	float ParseFloat();
	double ParseDouble();
	bool ParseBool();

	void ParseZeroTerminatedString( std::string& str );
	void ParseLengthPrecededString( std::string& str );

	Vec2 ParseVec2();
	Vec3 ParseVec3();
	Vec4 ParseVec4();

	IntVec2 ParseIntVec2();
	IntVec3 ParseIntVec3();
	IntVec4 ParseIntVec4();

	Rgba8 ParseRgba8();
	Rgba8 ParseRgb8();

	AABB2 ParseAABB2();
	AABB3 ParseAABB3();

	OBB2 ParseOBB2();
	OBB3 ParseOBB3();

	Plane2 ParsePlane2();
	Plane3 ParsePlane3();

	Vertex_PCU ParseVertexPCU();

	size_t GetCurReadPosition() const;
	void SetCurReadPosition( size_t readPosition );

	void SetBufferEndianMode( EndianMode mode );
	EndianMode GetBufferEndianMode() const;
	EndianMode GetLocalEndianMode() const;
protected:
	void SwitchByte( uint8_t* a, uint8_t* b ) const;
	bool SafetyCheck( size_t steps ) const;
protected:
	uint8_t const* m_buffer = nullptr;
	size_t m_size = 0;
	size_t m_curHeader = 0;
	EndianMode m_localEndianMode;
	EndianMode m_bufferEndianMode = EndianMode::Little;
};