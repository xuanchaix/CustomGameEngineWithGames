#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Room.hpp"

std::string g_starshipKeyStringMapping[256] = {
	"", "Left mouse button", "Right mouse button", "Control-break processing", "Middle mouse button", "X1 mouse button", "X2 mouse button", "", "Backspace", "TAB", "", "", "CLEAR", "ENTER", "", "",
	"SHIFT", "CTRL", "ALT", "PAUSE", "CAPS LOCK", "", "", "", "", "", "", "ESC", "", "", "", "",
	"SPACE", "PAGE UP", "PAGE DOWN", "END", "HOME", "LEFT ARROW", "UP ARROW", "RIGHT ARROW", "DOWN ARROW", "SELECT", "PRINT", "EXECUTE", "PRINT SCREEN", "INS", "DEL", "HELP",
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "", "", "", "", "", "",
	"", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
	"P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Left Windows", "Right Windows", "Applications" "", "Computer Sleep",
	"Numeric keypad 0", "Numeric keypad 1", "Numeric keypad 2", "Numeric keypad 3", "Numeric keypad 4", "Numeric keypad 5", "Numeric keypad 6", "Numeric keypad 7", "Numeric keypad 8", "Numeric keypad 9", "Multiply", "ADD", "Separator", "Subtract", "Decimal", "Divide",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16",
	"F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", "", "", "", "", "", "", "", "",
	"NUM LOCK", "SCROLL LOCK", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"Left SHIFT", "Right SHIFT", "Left CONTROL", "Right CONTROL", "Left ALT", "Right ALT", "Browser Back", "Browser Forward", "Browser Refresh", "Browser Stop", "Browser Search", "Browser Favorites", "Browser Start and Home", "Volume Mute", "Volume Down", "Volume Up",
	"Next Track", "Previous Track", "Stop Media", "Play/Pause Media","Start Mail", "Select Media", "Start Application 1", "Start Application 2","", "", ";", "+",",", "-", ".", "/",
	"`", "", "","", "", "", "","", "", "", "", "", "[", "\\", "]", "\'",
	"", "", "", "", "", "", "","", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "Attn","CrSel", "ExSel", "Erase EOF", "Play", "Zoom", "", "PA1", "Clear", "",
};

/*
  0  NUL (null)                      32  SPACE     64  @         96  `
  1  SOH (start of heading)          33  !         65  A         97  a
  2  STX (start of text)             34  "         66  B         98  b
  3  ETX (end of text)               35  #         67  C         99  c
  4  EOT (end of transmission)       36  $         68  D        100  d
  5  ENQ (enquiry)                   37  %         69  E        101  e
  6  ACK (acknowledge)               38  &         70  F        102  f
  7  BEL (bell)                      39  '         71  G        103  g
  8  BS  (backspace)                 40  (         72  H        104  h
  9  TAB (horizontal tab)            41  )         73  I        105  i
 10  LF  (NL line feed, new line)    42  *         74  J        106  j
 11  VT  (vertical tab)              43  +         75  K        107  k
 12  FF  (NP form feed, new page)    44  ,         76  L        108  l
 13  CR  (carriage return)           45  -         77  M        109  m
 14  SO  (shift out)                 46  .         78  N        110  n
 15  SI  (shift in)                  47  /         79  O        111  o
 16  DLE (data link escape)          48  0         80  P        112  p
 17  DC1 (device control 1)          49  1         81  Q        113  q
 18  DC2 (device control 2)          50  2         82  R        114  r
 19  DC3 (device control 3)          51  3         83  S        115  s
 20  DC4 (device control 4)          52  4         84  T        116  t
 21  NAK (negative acknowledge)      53  5         85  U        117  u
 22  SYN (synchronous idle)          54  6         86  V        118  v
 23  ETB (end of trans. block)       55  7         87  W        119  w
 24  CAN (cancel)                    56  8         88  X        120  x
 25  EM  (end of medium)             57  9         89  Y        121  y
 26  SUB (substitute)                58  :         90  Z        122  z
 27  ESC (escape)                    59  ;         91  [        123  {
 28  FS  (file separator)            60  <         92  \        124  |
 29  GS  (group separator)           61  =         93  ]        125  }
 30  RS  (record separator)          62  >         94  ^        126  ~
 31  US  (unit separator)            63  ?         95  _        127  DEL

*/



void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color ) {
	constexpr int NUM_SIDES = 16;
	constexpr int NUM_TRIANGLES = NUM_SIDES * 2;
	constexpr int NUM_VERTS = 3 * NUM_TRIANGLES;
	constexpr float DEGREES_PER_SIDE = 360.f / (float)NUM_SIDES;
	constexpr float RADIANS_PER_SIDE = DEGREES_PER_SIDE * PI / 180.f;
	Vertex_PCU ringVertsArray[NUM_VERTS];
	for (int i = 0; i < NUM_SIDES; i++) {
		ringVertsArray[6 * i] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * radius, SinRadians( RADIANS_PER_SIDE * i ) * radius, 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 2] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 1] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 3] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 5] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness), 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 4] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), 0 ), color, Vec2( 0, 0 ) );
	}
	for (int i = 0; i < NUM_VERTS; i++) {
		ringVertsArray[i].m_position += Vec3( center.x, center.y, 0 );
	}
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( NUM_VERTS, ringVertsArray );
}

void DebugDrawLine(Vec2 const& startPos, float orientation, float length, float thickness, Rgba8 const& color) {
	Vertex_PCU lineVertsArray[6];
	Vec3 p1 = Vec3( -thickness * 0.5f, -thickness * 0.5f, 0 );
	Vec3 p2 = Vec3( -thickness * 0.5f, thickness * 0.5f, 0 );
	Vec3 p3 = Vec3( length + thickness * 0.5f, -thickness * 0.5f, 0 );
	Vec3 p4 = Vec3( length + thickness * 0.5f, thickness * 0.5f, 0 );
	lineVertsArray[0] = Vertex_PCU( p1, color, Vec2( 0, 0 ) );
	lineVertsArray[2] = Vertex_PCU( p2, color, Vec2( 0, 0 ) );
	lineVertsArray[1] = Vertex_PCU( p3, color, Vec2( 0, 0 ) );
	lineVertsArray[3] = Vertex_PCU( p2, color, Vec2( 0, 0 ) );
	lineVertsArray[4] = Vertex_PCU( p3, color, Vec2( 0, 0 ) );
	lineVertsArray[5] = Vertex_PCU( p4, color, Vec2( 0, 0 ) );

	TransformVertexArrayXY3D( 6, lineVertsArray, 1.f, orientation, startPos );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 6, lineVertsArray );
}

void DebugDrawLine( Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color )
{
	Vec2 dForward = (endPos - startPos).GetNormalized();
	Vec2 forward = dForward * thickness * 0.5f;
	Vec2 left = dForward.GetRotated90Degrees() * thickness * 0.5f;
	Vertex_PCU lineVertsArray[6];
	Vec3 p1 = Vec3( (startPos - forward + left).x, (startPos - forward + left).y, 0 );
	Vec3 p2 = Vec3( (startPos - forward - left).x, (startPos - forward - left).y, 0 );
	Vec3 p3 = Vec3( (endPos + forward + left).x, (endPos + forward + left).y, 0 );
	Vec3 p4 = Vec3( (endPos + forward - left).x, (endPos + forward - left).y, 0 );
	lineVertsArray[0] = Vertex_PCU( p1, color, Vec2( 0, 0 ) );
	lineVertsArray[1] = Vertex_PCU( p2, color, Vec2( 0, 0 ) );
	lineVertsArray[2] = Vertex_PCU( p3, color, Vec2( 0, 0 ) );
	lineVertsArray[3] = Vertex_PCU( p2, color, Vec2( 0, 0 ) );
	lineVertsArray[5] = Vertex_PCU( p3, color, Vec2( 0, 0 ) );
	lineVertsArray[4] = Vertex_PCU( p4, color, Vec2( 0, 0 ) );

	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 6, lineVertsArray );
}

bool Starship_IsVec2NearlyZero( Vec2 const& vectorToCompare )
{
	return vectorToCompare.x < 0.001f && vectorToCompare.x > -0.001f && vectorToCompare.y < 0.001f && vectorToCompare.y > -0.001f;
}

bool Starship_IsNearlyZero( float floatToCompare )
{
	return abs( floatToCompare ) < 0.001f;
}

void Starship_AddVertsForArch( std::vector<Vertex_PCU>& verts, Vec2 const& bottomPos, Vec2 const& topPos, float radius, Rgba8 const& color )
{
	Vec2 forwardVector = topPos - bottomPos;
	float length = forwardVector.GetLength();
	Vec2 iBasisNormal = forwardVector / length;
	Vec2 jBasisNormal = Vec2( -iBasisNormal.y, iBasisNormal.x );
	float startRadian = forwardVector.GetOrientationRadians() - PI * 0.5f;
	constexpr int NUM_OF_VERTS = 90;
	for (int i = 0; i < NUM_OF_VERTS / 6; i++) {
		verts.emplace_back( topPos, color );
		verts.emplace_back( topPos + Vec2( CosRadians( 6 * PI / NUM_OF_VERTS * i + startRadian ) * radius, SinRadians( 6 * PI / NUM_OF_VERTS * i + startRadian ) * radius ), color );
		verts.emplace_back( topPos + Vec2( CosRadians( 6 * PI / NUM_OF_VERTS * (i + 1) + startRadian ) * radius, SinRadians( 6 * PI / NUM_OF_VERTS * (i + 1) + startRadian ) * radius ), color );
	}
	verts.emplace_back( bottomPos - jBasisNormal * radius, color );
	verts.emplace_back( topPos - jBasisNormal * radius, color );
	verts.emplace_back( bottomPos + jBasisNormal * radius, color );
	verts.emplace_back( bottomPos + jBasisNormal * radius, color );
	verts.emplace_back( topPos - jBasisNormal * radius, color );
	verts.emplace_back( topPos + jBasisNormal * radius, color );
}

Vec2 Starship_GetStrongestPart( Vec2 const& input )
{
	constexpr float sqrt1of2 = 0.707f;
	int choose = -1;
	float res = 0.f;
	Vec2 east = Vec2( 1.f, 0.f );
	if (DotProduct2D( input, east ) > res) {
		res = DotProduct2D( input, east );
		choose = 0;
	}
	Vec2 west = Vec2( -1.f, 0.f );
	if (DotProduct2D( input, west ) > res) {
		res = DotProduct2D( input, west );
		choose = 1;
	}
	Vec2 north = Vec2( 0.f, 1.f );
	if (DotProduct2D( input, north ) > res) {
		res = DotProduct2D( input, north );
		choose = 2;
	}
	Vec2 south = Vec2( 0.f, -1.f );
	if (DotProduct2D( input, south ) > res) {
		res = DotProduct2D( input, south );
		choose = 3;
	}
	Vec2 northEast = Vec2( sqrt1of2, sqrt1of2 );
	if (DotProduct2D( input, northEast ) > res) {
		res = DotProduct2D( input, northEast );
		choose = 4;
	}
	Vec2 northWest = Vec2( -sqrt1of2, sqrt1of2 );
	if (DotProduct2D( input, northWest ) > res) {
		res = DotProduct2D( input, northWest );
		choose = 5;
	}
	Vec2 southEast = Vec2( sqrt1of2, -sqrt1of2 );
	if (DotProduct2D( input, southEast ) > res) {
		res = DotProduct2D( input, southEast );
		choose = 6;
	}
	Vec2 southWest = Vec2( -sqrt1of2, -sqrt1of2 );
	if (DotProduct2D( input, southWest ) > res) {
		res = DotProduct2D( input, southWest );
		choose = 7;
	}

	if (choose == -1) {
		return Vec2();
	}
	if (choose == 0) {
		return east;
	}
	if (choose == 1) {
		return west;
	}
	if (choose == 2) {
		return north;
	}
	if (choose == 3) {
		return south;
	}
	if (choose == 4) {
		return northEast;
	}
	if (choose == 5) {
		return northWest;
	}
	if (choose == 6) {
		return southEast;
	}
	if (choose == 7) {
		return southWest;
	}
	return Vec2();
}

Vec2 Starship_GetRandomPosInCapsule( Vec2 const& startPos, Vec2 const& endPos, float radius )
{
	float rnd1 = GetRandGen()->RollRandomFloatZeroToOne();
	Vec2 refPos = Interpolate( startPos, endPos, rnd1 );
	return GetRandomPointInDisc2D( radius, refPos );
}

Clock* GetGameClock()
{
	return g_theGame->m_gameClock;
}

RandomNumberGenerator* GetRandGen()
{
	return g_theGame->m_randNumGen;
}

RoomDirection GetOppositeRoomDir( RoomDirection dir )
{
	if (dir == RoomDirection::UP) {
		return RoomDirection::DOWN;
	}
	if (dir == RoomDirection::DOWN) {
		return RoomDirection::UP;
	}
	if (dir == RoomDirection::LEFT) {
		return RoomDirection::RIGHT;
	}
	if (dir == RoomDirection::RIGHT) {
		return RoomDirection::LEFT;
	}
	return RoomDirection::CENTER;
}

void ClampIntoRoom( Vec2& pos, float radius, Room* room )
{
	AABB2 const& bounds = room->m_bounds;
	if (pos.x < radius + bounds.m_mins.x) {
		pos.x = radius + bounds.m_mins.x;
	}
	if (pos.x > bounds.m_maxs.x - radius) {
		pos.x = bounds.m_maxs.x - radius;
	}
	if (pos.y < radius + bounds.m_mins.y) {
		pos.y = radius + bounds.m_mins.y;
	}
	if (pos.y > bounds.m_maxs.y - radius) {
		pos.y = bounds.m_maxs.y - radius;
	}
}

