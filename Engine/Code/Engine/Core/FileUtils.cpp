#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/EngineMath.hpp"

int FileReadToBuffer( std::vector<uint8_t>& out_buffer, std::string const& filename )
{
	FILE* file = nullptr;
	errno_t errNo = fopen_s( &file, filename.c_str(), "rb" );
	if (errNo != 0 || file == nullptr) {
		return -1;
		//ERROR_AND_DIE( Stringf( "Cannot open file %s", filename.c_str() ) );
	}

	fseek( file, 0, SEEK_END );
	int size = ftell( file );
	out_buffer.resize( size );
	fseek( file, 0, SEEK_SET );

	/*int count = size;
	int i = 0;
	while (count > 4096) {
		fread( (void*)(out_buffer.data() + i * 4096), sizeof( uint8_t ), 4096, file );
		count -= 4096;
		i++;
	}*/
	fread( out_buffer.data(), sizeof( uint8_t ), size, file );
	/*int readNum = (int)*/ 
	//GUARANTEE_OR_DIE( readNum == size, Stringf( "Read file error!" ) );
	fclose( file );
	return size;
}

int FileReadToString( std::string& out_string, std::string const& filename )
{
	std::vector<uint8_t> buffer;
	int size = FileReadToBuffer( buffer, filename );
	buffer.push_back( '\0' );
	// out_string = std::string( (char*)buffer.data() );
	out_string.assign( (char*)buffer.data(), buffer.size() );
	return size;
}

void StringWriteToFile( std::string const& str, std::string const& filename )
{
	std::vector<uint8_t> buffer;
	buffer.resize( str.length() );
	memcpy( buffer.data(), str.c_str(), str.length() );
	BufferWriteToFile( buffer, filename );
}

void BufferWriteToFile( std::vector<uint8_t> const& buffer, std::string const& filename )
{
	FILE* file = nullptr;
	errno_t errNo = fopen_s( &file, filename.c_str(), "wb" );
	if (errNo != 0 || file == nullptr) {
		ERROR_AND_DIE( Stringf( "Cannot open file %s", filename.c_str() ) );
	}
	fwrite( buffer.data(), sizeof( uint8_t ), buffer.size(), file );
	fclose( file );
}

BufferWriter::BufferWriter( std::vector<uint8_t>& bufferToWrite )
	:m_buffer(bufferToWrite)
{
	m_localEndianMode = GetPlatformLocalEndian();
}

void BufferWriter::AppendByte( uint8_t byteToAppend )
{
	m_buffer.push_back( byteToAppend );
}

void BufferWriter::AppendChar( char charToAppend )
{
	m_buffer.push_back( charToAppend );
}

void BufferWriter::AppendUshort( unsigned short ushortToAppend )
{
	uint8_t* charArray = (uint8_t*)&ushortToAppend;

	if (m_localEndianMode != m_preferredEndianMode) {
		SwitchByte( &charArray[0], &charArray[1] );
	}

	m_buffer.push_back( charArray[0] );
	m_buffer.push_back( charArray[1] );
}

void BufferWriter::AppendShort( signed short shortToAppend )
{
	uint8_t* charArray = (uint8_t*)&shortToAppend;

	if (m_localEndianMode != m_preferredEndianMode) {
		SwitchByte( &charArray[0], &charArray[1] );
	}

	m_buffer.push_back( charArray[0] );
	m_buffer.push_back( charArray[1] );
}

void BufferWriter::AppendUint32( unsigned int uintToAppend )
{
	uint8_t* charArray = (uint8_t*)&uintToAppend;

	if (m_localEndianMode != m_preferredEndianMode) {
		SwitchByte( &charArray[0], &charArray[3] );
		SwitchByte( &charArray[1], &charArray[2] );
	}

	m_buffer.push_back( charArray[0] );
	m_buffer.push_back( charArray[1] );
	m_buffer.push_back( charArray[2] );
	m_buffer.push_back( charArray[3] );
}

void BufferWriter::AppendInt32( signed int intToAppend )
{
	uint8_t* charArray = (uint8_t*)&intToAppend;

	if (m_localEndianMode != m_preferredEndianMode) {
		SwitchByte( &charArray[0], &charArray[3] );
		SwitchByte( &charArray[1], &charArray[2] );
	}

	m_buffer.push_back( charArray[0] );
	m_buffer.push_back( charArray[1] );
	m_buffer.push_back( charArray[2] );
	m_buffer.push_back( charArray[3] );
}

void BufferWriter::AppendUint64( uint64_t uintToAppend )
{
	uint8_t* charArray = (uint8_t*)&uintToAppend;

	if (m_localEndianMode != m_preferredEndianMode) {
		SwitchByte( &charArray[0], &charArray[7] );
		SwitchByte( &charArray[1], &charArray[6] );
		SwitchByte( &charArray[2], &charArray[5] );
		SwitchByte( &charArray[3], &charArray[4] );
	}

	m_buffer.push_back( charArray[0] );
	m_buffer.push_back( charArray[1] );
	m_buffer.push_back( charArray[2] );
	m_buffer.push_back( charArray[3] );
	m_buffer.push_back( charArray[4] );
	m_buffer.push_back( charArray[5] );
	m_buffer.push_back( charArray[6] );
	m_buffer.push_back( charArray[7] );
}

void BufferWriter::AppendInt64( int64_t intToAppend )
{
	uint8_t* charArray = (uint8_t*)&intToAppend;

	if (m_localEndianMode != m_preferredEndianMode) {
		SwitchByte( &charArray[0], &charArray[7] );
		SwitchByte( &charArray[1], &charArray[6] );
		SwitchByte( &charArray[2], &charArray[5] );
		SwitchByte( &charArray[3], &charArray[4] );
	}

	m_buffer.push_back( charArray[0] );
	m_buffer.push_back( charArray[1] );
	m_buffer.push_back( charArray[2] );
	m_buffer.push_back( charArray[3] );
	m_buffer.push_back( charArray[4] );
	m_buffer.push_back( charArray[5] );
	m_buffer.push_back( charArray[6] );
	m_buffer.push_back( charArray[7] );
}

void BufferWriter::AppendFloat( float floatToAppend )
{
	uint8_t* charArray = (uint8_t*)&floatToAppend;

	if (m_localEndianMode != m_preferredEndianMode) {
		SwitchByte( &charArray[0], &charArray[3] );
		SwitchByte( &charArray[1], &charArray[2] );
	}

	m_buffer.push_back( charArray[0] );
	m_buffer.push_back( charArray[1] );
	m_buffer.push_back( charArray[2] );
	m_buffer.push_back( charArray[3] );
}

void BufferWriter::AppendDouble( double doubleToAppend )
{
	uint8_t* charArray = (uint8_t*)&doubleToAppend;

	if (m_localEndianMode != m_preferredEndianMode) {
		SwitchByte( &charArray[0], &charArray[7] );
		SwitchByte( &charArray[1], &charArray[6] );
		SwitchByte( &charArray[2], &charArray[5] );
		SwitchByte( &charArray[3], &charArray[4] );
	}

	m_buffer.push_back( charArray[0] );
	m_buffer.push_back( charArray[1] );
	m_buffer.push_back( charArray[2] );
	m_buffer.push_back( charArray[3] );
	m_buffer.push_back( charArray[4] );
	m_buffer.push_back( charArray[5] );
	m_buffer.push_back( charArray[6] );
	m_buffer.push_back( charArray[7] );
}

void BufferWriter::AppendBool( bool boolToAppend )
{
	m_buffer.push_back( boolToAppend );
}

void BufferWriter::AppendZeroTerminatedString( std::string const& strToAppend )
{
	for (int i = 0; i < (int)strToAppend.size(); ++i) {
		AppendChar( strToAppend[i] );
	}
	AppendChar( '\0' );
}

void BufferWriter::AppendLengthPrecededString( std::string const& strToAppend )
{
	AppendInt32( (int)strToAppend.size() );
	for (int i = 0; i < (int)strToAppend.size(); ++i) {
		AppendChar( strToAppend[i] );
	}
}

void BufferWriter::AppendVec2( Vec2 const& vecToAppend )
{
	AppendFloat( vecToAppend.x );
	AppendFloat( vecToAppend.y );
}

void BufferWriter::AppendVec3( Vec3 const& vecToAppend )
{
	AppendFloat( vecToAppend.x );
	AppendFloat( vecToAppend.y );
	AppendFloat( vecToAppend.z );
}

void BufferWriter::AppendVec4( Vec4 const& vecToAppend )
{
	AppendFloat( vecToAppend.x );
	AppendFloat( vecToAppend.y );
	AppendFloat( vecToAppend.z );
	AppendFloat( vecToAppend.w );
}

void BufferWriter::AppendIntVec2( IntVec2 const& vecToAppend )
{
	AppendInt32( vecToAppend.x );
	AppendInt32( vecToAppend.y );
}

void BufferWriter::AppendIntVec3( IntVec3 const& vecToAppend )
{
	AppendInt32( vecToAppend.x );
	AppendInt32( vecToAppend.y );
	AppendInt32( vecToAppend.z );
}

void BufferWriter::AppendIntVec4( IntVec4 const& vecToAppend )
{
	AppendInt32( vecToAppend.x );
	AppendInt32( vecToAppend.y );
	AppendInt32( vecToAppend.z );
	AppendInt32( vecToAppend.w );
}

void BufferWriter::AppendRgba8( Rgba8 const colorToAppend )
{
	AppendByte( colorToAppend.r );
	AppendByte( colorToAppend.g );
	AppendByte( colorToAppend.b );
	AppendByte( colorToAppend.a );
}

void BufferWriter::AppendRgb8( Rgba8 const colorToAppend )
{
	AppendByte( colorToAppend.r );
	AppendByte( colorToAppend.g );
	AppendByte( colorToAppend.b );
}

void BufferWriter::AppendAABB2( AABB2 const& aabb2ToAppend )
{
	AppendVec2( aabb2ToAppend.m_mins );
	AppendVec2( aabb2ToAppend.m_maxs );
}

void BufferWriter::AppendAABB3( AABB3 const& aabb3ToAppend )
{
	AppendVec3( aabb3ToAppend.m_mins );
	AppendVec3( aabb3ToAppend.m_maxs );
}

void BufferWriter::AppendOBB2( OBB2 const& obb2ToAppend )
{
	AppendVec2( obb2ToAppend.m_center );
	AppendVec2( obb2ToAppend.m_halfDimensions );
	AppendVec2( obb2ToAppend.m_iBasisNormal );
}

void BufferWriter::AppendOBB3( OBB3 const& obb3ToAppend )
{
	AppendVec3( obb3ToAppend.m_center );
	AppendVec3( obb3ToAppend.m_halfDimensions );
	AppendVec3( obb3ToAppend.m_iBasis );
	AppendVec3( obb3ToAppend.m_jBasis );
	AppendVec3( obb3ToAppend.m_kBasis );
}

void BufferWriter::AppendPlane2( Plane2 const& plane2ToAppend )
{
	AppendVec2( plane2ToAppend.m_normal );
	AppendFloat( plane2ToAppend.m_distanceFromOrigin );
}

void BufferWriter::AppendPlane3( Plane3 const& plane3ToAppend )
{
	AppendVec3( plane3ToAppend.m_normal );
	AppendFloat( plane3ToAppend.m_distanceFromOrigin );
}

void BufferWriter::AppendVertexPCU( Vertex_PCU const& vertToAppend )
{
	AppendVec3( vertToAppend.m_position );
	AppendRgba8( vertToAppend.m_color );
	AppendVec2( vertToAppend.m_uvTexCoords );
}

void BufferWriter::WriteIntToPos( size_t startPos, unsigned int intToWrite )
{
	if (startPos + sizeof(unsigned int) <= m_buffer.size()) {
		if (m_localEndianMode == m_preferredEndianMode) {
			m_buffer[startPos] = ((uint8_t*)&intToWrite)[0];
			m_buffer[startPos + 1] = ((uint8_t*)&intToWrite)[1];
			m_buffer[startPos + 2] = ((uint8_t*)&intToWrite)[2];
			m_buffer[startPos + 3] = ((uint8_t*)&intToWrite)[3];
		}
		else {
			m_buffer[startPos] = ((uint8_t*)&intToWrite)[3];
			m_buffer[startPos + 1] = ((uint8_t*)&intToWrite)[2];
			m_buffer[startPos + 2] = ((uint8_t*)&intToWrite)[1];
			m_buffer[startPos + 3] = ((uint8_t*)&intToWrite)[0];
		}
	}
}

void BufferWriter::SetPreferredEndianMode( EndianMode mode )
{
	if (mode != EndianMode::Big && mode != EndianMode::Little) {
		ERROR_AND_DIE( "Cannot Set Endian Mode" );
	}
	m_preferredEndianMode = mode;
}

EndianMode BufferWriter::GetPreferredEndianMode() const
{
	return m_preferredEndianMode;
}

EndianMode BufferWriter::GetLocalEndianMode() const
{
	return m_localEndianMode;
}

size_t BufferWriter::GetCurBufferSize() const
{
	return m_buffer.size();
}

void BufferWriter::SwitchByte( uint8_t* a, uint8_t* b ) const
{
	uint8_t temp = *a;
	*a = *b;
	*b = temp;
}

BufferReader::BufferReader( std::vector<uint8_t> const& bufferToRead )
	:m_buffer(bufferToRead.data())
	,m_size(bufferToRead.size())
{
	int n = 1;
	if (*(char*)&n == 1) {
		m_localEndianMode = EndianMode::Little;
	}
	else {
		m_localEndianMode = EndianMode::Big;
	}
}

BufferReader::BufferReader( uint8_t const* const& buffer, size_t size )
	:m_buffer( buffer )
	, m_size( size )
{
	m_localEndianMode = GetPlatformLocalEndian();
}

uint8_t BufferReader::ParseByte()
{
	GUARANTEE_OR_DIE( SafetyCheck( sizeof( uint8_t ) ), Stringf( "Cannot Parse a Byte in position %d, position out of buffer range", m_curHeader ) );
	uint8_t value = m_buffer[m_curHeader];
	m_curHeader += sizeof(uint8_t);
	return value;
}

char BufferReader::ParseChar()
{
	GUARANTEE_OR_DIE( SafetyCheck( sizeof( char ) ), Stringf( "Cannot Parse a Char in position %d, position out of buffer range", m_curHeader ) );
	char value = m_buffer[m_curHeader];
	m_curHeader += sizeof(char);
	return value;
}

unsigned short BufferReader::ParseUshort()
{
	GUARANTEE_OR_DIE( SafetyCheck( sizeof( unsigned short ) ), Stringf( "Cannot Parse a unsigned short in position %d, position out of buffer range", m_curHeader ) );
	unsigned short value = *(unsigned short*)(&m_buffer[m_curHeader]);

	if (m_localEndianMode != m_bufferEndianMode) {
		SwitchByte( &(((uint8_t*)&value)[0]), &(((uint8_t*)&value)[1]) );
	}

	m_curHeader += sizeof( unsigned short );
	return value;
}

signed short BufferReader::ParseShort()
{
	GUARANTEE_OR_DIE( SafetyCheck( sizeof( signed short ) ), Stringf( "Cannot Parse a signed short in position %d, position out of buffer range", m_curHeader ) );
	signed short value = *(signed short*)(&m_buffer[m_curHeader]);

	if (m_localEndianMode != m_bufferEndianMode) {
		SwitchByte( &(((uint8_t*)&value)[0]), &(((uint8_t*)&value)[1]) );
	}

	m_curHeader += sizeof( signed short );
	return value;
}

unsigned int BufferReader::ParseUint32()
{
	GUARANTEE_OR_DIE( SafetyCheck( sizeof( unsigned int ) ), Stringf( "Cannot Parse a unsigned int in position %d, position out of buffer range", m_curHeader ) );
	unsigned int value = *(unsigned int*)(&m_buffer[m_curHeader]);

	if (m_localEndianMode != m_bufferEndianMode) {
		SwitchByte( &(((uint8_t*)&value)[0]), &(((uint8_t*)&value)[3]) );
		SwitchByte( &(((uint8_t*)&value)[1]), &(((uint8_t*)&value)[2]) );
	}

	m_curHeader += sizeof( unsigned int );
	return value;
}

signed int BufferReader::ParseInt32()
{
	GUARANTEE_OR_DIE( SafetyCheck( sizeof( signed int ) ), Stringf( "Cannot Parse a signed int in position %d, position out of buffer range", m_curHeader ) );
	signed int value = *(signed int*)(&m_buffer[m_curHeader]);

	if (m_localEndianMode != m_bufferEndianMode) {
		SwitchByte( &(((uint8_t*)&value)[0]), &(((uint8_t*)&value)[3]) );
		SwitchByte( &(((uint8_t*)&value)[1]), &(((uint8_t*)&value)[2]) );
	}

	m_curHeader += sizeof( signed int );
	return value;
}

uint64_t BufferReader::ParseUint64()
{
	GUARANTEE_OR_DIE( SafetyCheck( sizeof( uint64_t ) ), Stringf( "Cannot Parse a unsigned int in position %d, position out of buffer range", m_curHeader ) );
	uint64_t value = *(uint64_t*)(&m_buffer[m_curHeader]);

	if (m_localEndianMode != m_bufferEndianMode) {
		SwitchByte( &(((uint8_t*)&value)[0]), &(((uint8_t*)&value)[7]) );
		SwitchByte( &(((uint8_t*)&value)[1]), &(((uint8_t*)&value)[6]) );
		SwitchByte( &(((uint8_t*)&value)[2]), &(((uint8_t*)&value)[5]) );
		SwitchByte( &(((uint8_t*)&value)[3]), &(((uint8_t*)&value)[4]) );
	}

	m_curHeader += sizeof( uint64_t );
	return value;
}

int64_t BufferReader::ParseInt64()
{
	GUARANTEE_OR_DIE( SafetyCheck( sizeof( int64_t ) ), Stringf( "Cannot Parse a signed int64 in position %d, position out of buffer range", m_curHeader ) );
	int64_t value = *(int64_t*)(&m_buffer[m_curHeader]);

	if (m_localEndianMode != m_bufferEndianMode) {
		SwitchByte( &(((uint8_t*)&value)[0]), &(((uint8_t*)&value)[7]) );
		SwitchByte( &(((uint8_t*)&value)[1]), &(((uint8_t*)&value)[6]) );
		SwitchByte( &(((uint8_t*)&value)[2]), &(((uint8_t*)&value)[5]) );
		SwitchByte( &(((uint8_t*)&value)[3]), &(((uint8_t*)&value)[4]) );
	}

	m_curHeader += sizeof( int64_t );
	return value;
}

float BufferReader::ParseFloat()
{
	GUARANTEE_OR_DIE( SafetyCheck( sizeof( float ) ), Stringf( "Cannot Parse a float in position %d, position out of buffer range", m_curHeader ) );
	float value = *(float*)(&m_buffer[m_curHeader]);

	if (m_localEndianMode != m_bufferEndianMode) {
		SwitchByte( &(((uint8_t*)&value)[0]), &(((uint8_t*)&value)[3]) );
		SwitchByte( &(((uint8_t*)&value)[1]), &(((uint8_t*)&value)[2]) );
	}

	m_curHeader += sizeof( float );
	return value;
}

double BufferReader::ParseDouble()
{
	GUARANTEE_OR_DIE( SafetyCheck( sizeof( double ) ), Stringf( "Cannot Parse a double in position %d, position out of buffer range", m_curHeader ) );
	double value = *(double*)(&m_buffer[m_curHeader]);

	if (m_localEndianMode != m_bufferEndianMode) {
		SwitchByte( &(((uint8_t*)&value)[0]), &(((uint8_t*)&value)[7]) );
		SwitchByte( &(((uint8_t*)&value)[1]), &(((uint8_t*)&value)[6]) );
		SwitchByte( &(((uint8_t*)&value)[2]), &(((uint8_t*)&value)[5]) );
		SwitchByte( &(((uint8_t*)&value)[3]), &(((uint8_t*)&value)[4]) );
	}

	m_curHeader += sizeof( double );
	return value;
}

bool BufferReader::ParseBool()
{
	GUARANTEE_OR_DIE( SafetyCheck( sizeof( bool ) ), Stringf( "Cannot Parse a bool in position %d, position out of buffer range", m_curHeader ) );

	bool value = *(bool*)(&m_buffer[m_curHeader]);
	m_curHeader += sizeof( bool );
	return value;
}

void BufferReader::ParseZeroTerminatedString( std::string& out_str )
{
	char* str = (char*)&m_buffer[m_curHeader];
	// ToDo: a safer version needed! There may be infinite loop
	while (m_buffer[m_curHeader++] != '\0');
	out_str = std::string( str );
}

void BufferReader::ParseLengthPrecededString( std::string& out_str )
{
	unsigned int length = ParseUint32();

	GUARANTEE_OR_DIE( SafetyCheck( (size_t)length ), Stringf( "Cannot Parse a string in position %d, position out of buffer range", m_curHeader ) );

	out_str.clear();
	out_str.reserve( length );
	for (unsigned int i = 0; i < length; ++i) {
		out_str.push_back( m_buffer[m_curHeader + i] );
	}
	//out_str.assign( m_buffer[m_curHeader], length );
	m_curHeader += (size_t)length;
}

Vec2 BufferReader::ParseVec2()
{
	Vec2 value;
	value.x = ParseFloat();
	value.y = ParseFloat();
	return value;
}

Vec3 BufferReader::ParseVec3()
{
	Vec3 value;
	value.x = ParseFloat();
	value.y = ParseFloat();
	value.z = ParseFloat();
	return value;
}

Vec4 BufferReader::ParseVec4()
{
	Vec4 value;
	value.x = ParseFloat();
	value.y = ParseFloat();
	value.z = ParseFloat();
	value.w = ParseFloat();
	return value;
}

IntVec2 BufferReader::ParseIntVec2()
{
	IntVec2 value;
	value.x = ParseInt32();
	value.y = ParseInt32();
	return value;
}

IntVec3 BufferReader::ParseIntVec3()
{
	IntVec3 value;
	value.x = ParseInt32();
	value.y = ParseInt32();
	value.z = ParseInt32();
	return value;
}

IntVec4 BufferReader::ParseIntVec4()
{
	IntVec4 value;
	value.x = ParseInt32();
	value.y = ParseInt32();
	value.z = ParseInt32();
	value.w = ParseInt32();
	return value;
}

Rgba8 BufferReader::ParseRgba8()
{
	Rgba8 value;
	value.r = ParseByte();
	value.g = ParseByte();
	value.b = ParseByte();
	value.a = ParseByte();
	return value;
}

Rgba8 BufferReader::ParseRgb8()
{
	Rgba8 value;
	value.r = ParseByte();
	value.g = ParseByte();
	value.b = ParseByte();
	value.a = 255;
	return value;
}

AABB2 BufferReader::ParseAABB2()
{
	AABB2 value;
	value.m_mins = ParseVec2();
	value.m_maxs = ParseVec2();
	return value;
}

AABB3 BufferReader::ParseAABB3()
{
	AABB3 value;
	value.m_mins = ParseVec3();
	value.m_maxs = ParseVec3();
	return value;
}

OBB2 BufferReader::ParseOBB2()
{
	OBB2 value;
	value.m_center = ParseVec2();
	value.m_halfDimensions = ParseVec2();
	value.m_iBasisNormal = ParseVec2();
	return value;
}

OBB3 BufferReader::ParseOBB3()
{
	OBB3 value;
	value.m_center = ParseVec3();
	value.m_halfDimensions = ParseVec3();
	value.m_iBasis = ParseVec3();
	value.m_jBasis = ParseVec3();
	value.m_kBasis = ParseVec3();
	return value;
}

Plane2 BufferReader::ParsePlane2()
{
	Plane2 value;
	value.m_normal = ParseVec2();
	value.m_distanceFromOrigin = ParseFloat();
	return value;
}

Plane3 BufferReader::ParsePlane3()
{
	Plane3 value;
	value.m_normal = ParseVec3();
	value.m_distanceFromOrigin = ParseFloat();
	return value;
}

Vertex_PCU BufferReader::ParseVertexPCU()
{
	Vertex_PCU vert;
	vert.m_position = ParseVec3();
	vert.m_color = ParseRgba8();
	vert.m_uvTexCoords = ParseVec2();
	return vert;
}

size_t BufferReader::GetCurReadPosition() const
{
	return m_curHeader;
}

void BufferReader::SetCurReadPosition( size_t readPosition )
{
	m_curHeader = readPosition;
}

void BufferReader::SetBufferEndianMode( EndianMode mode )
{
	if (mode != EndianMode::Big && mode != EndianMode::Little) {
		ERROR_AND_DIE( "Cannot Set Endian Mode" );
	}
	m_bufferEndianMode = mode;
}

EndianMode BufferReader::GetBufferEndianMode() const
{
	return m_bufferEndianMode;
}

EndianMode BufferReader::GetLocalEndianMode() const
{
	return m_localEndianMode;
}

void BufferReader::SwitchByte( uint8_t* a, uint8_t* b ) const
{
	uint8_t temp = *a;
	*a = *b;
	*b = temp;
}

bool BufferReader::SafetyCheck( size_t steps ) const
{
	if (m_curHeader + steps > m_size) {
		return false;
	}
	return true;
}
