#pragma once
#include "Game/GameCommon.hpp"
#include <queue>

struct FortuneParabola;

struct FortuneVec2d {
	FortuneVec2d() {};
	FortuneVec2d( double inx, double iny ) :x(inx), y(iny) {};
	bool		operator==( const FortuneVec2d& compare ) const { return compare.x == x && compare.y == y; };
	bool		operator!=( const FortuneVec2d& compare ) const { return compare.x != x || compare.y != y; };
	FortuneVec2d const operator+( const FortuneVec2d& vecToAdd ) const { return FortuneVec2d( vecToAdd.x + x, vecToAdd.y + y ); };
	FortuneVec2d const operator-( const FortuneVec2d& vecToSubtract ) const { return FortuneVec2d( x - vecToSubtract.x, y - vecToSubtract.y ); };
	FortuneVec2d const operator-() const { return FortuneVec2d( -x, -y ); };
	FortuneVec2d const operator*( double uniformScale ) const { return FortuneVec2d( x * uniformScale, y * uniformScale ); };

	double GetLength() const { return sqrt( x * x + y * y ); };
	FortuneVec2d GetNormalized() const { double length = GetLength(); return FortuneVec2d( x / length, y / length ); };
	FortuneVec2d GetRotatedMinus90Degrees() const { return FortuneVec2d( y, -x ); };

	void		operator=( const FortuneVec2d& copyFrom ) { x = copyFrom.x; y = copyFrom.y; };
	void		operator=( const Vec2& copyFrom ) { x = copyFrom.x; y = copyFrom.y; };
	double x = 0.0;
	double y = 0.0;
};

double GetDistance2D( FortuneVec2d const& positionA, FortuneVec2d const& positionB ) {
	return sqrt( (positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y) );
}

struct FortuneSite {

	FortuneVec2d m_pos;
	int m_index;
};

struct FortuneHalfEdge {
	FortuneVec2d m_curStartPos;
	FortuneVec2d m_dir;

	FortuneVec2d m_vertexPos;

	FortuneHalfEdge* m_prev = nullptr;
	FortuneHalfEdge* m_next = nullptr;
	FortuneHalfEdge* m_opposite = nullptr;
	FortuneParabola* m_parabola = nullptr;
};

struct FortuneParabola {
	FortuneParabola() {};
	FortuneParabola( FortuneVec2d const& focus );

	void Reset( double directrixY );
	double Calculate( double x );

	FortuneVec2d m_focus = FortuneVec2d();
	double a = 1.f;
	double b = 0.f;
	double c = 0.f;
	bool m_line = true;
	FortuneParabola* m_left = nullptr;
	FortuneParabola* m_right = nullptr;
	//FortuneHalfEdge* m_halfEdgeLeft = nullptr;
	//FortuneHalfEdge* m_halfEdgeRight = nullptr;
};

FortuneParabola::FortuneParabola( FortuneVec2d const& focus )
	:m_focus(focus)
{
}

void FortuneParabola::Reset( double directrixY )
{
	m_line = false;
	a = 1.f / (m_focus.y - directrixY) * 0.5f;
	b = m_focus.x;
	c = (m_focus.y + directrixY) * 0.5f;
}

double FortuneParabola::Calculate( double x )
{
	double xmb = x - b;
	return a * xmb * xmb + c;
}

bool SolveHalfEdgeIntersection( FortuneHalfEdge* e1, FortuneHalfEdge* e2, FortuneVec2d& out_res )
{
	double d = -(e1->m_dir.x * e2->m_dir.y) + (e1->m_dir.y * e2->m_dir.x);
	double dx = -(e2->m_curStartPos.x - e1->m_curStartPos.x) * e2->m_dir.y + (e2->m_curStartPos.y - e1->m_curStartPos.y) * e2->m_dir.x;
	//float dy = e1->m_dir.x * (e2->m_curStartPos.y - e1->m_curStartPos.y) - e1->m_dir.y * (e2->m_curStartPos.x - e1->m_curStartPos.x);
	if (d == 0.f) {
		return false;
	}
	double t1 = dx / d;
	//float t2 = dy / d;
	out_res = e1->m_curStartPos + e1->m_dir * t1;
	if ((out_res.x - e1->m_curStartPos.x) / e1->m_dir.x < 0) {
		return false;
	}
	else if ((out_res.y - e1->m_curStartPos.y) / e1->m_dir.y < 0) {
		return false;
	}
	else if ((out_res.x - e2->m_curStartPos.x) / e2->m_dir.x < 0) {
		return false;
	}
	else if ((out_res.y - e2->m_curStartPos.y) / e2->m_dir.y < 0) {
		return false;
	}
	else {
		return true;
	}
}

std::pair<double, double> SolveParabolaIntersection ( FortuneParabola const* p1, FortuneParabola const* p2 )
{
	double a = (p1->a - p2->a);
	double b = -2.f * (p1->a * p1->b - p2->a * p2->b);
	double c = p1->a * p1->b * p1->b + p1->c - p2->a * p2->b * p2->b - p2->c;
	double sqrDelta = b * b - 4 * a * c;
	if (sqrDelta < 0.f) {
		return std::pair<double, double>( FLT_MAX, FLT_MAX );
	}
	double inv2a = 0.5f / a;
	double x1 = inv2a * (-b + sqrt( sqrDelta ));
	double x2 = inv2a * (-b - sqrt( sqrDelta ));
	return std::pair<double, double>( x1, x2 );
}

struct FortuneIntersectionPoint {
	FortuneIntersectionPoint() {};
	FortuneIntersectionPoint( FortuneHalfEdge* leftHalfEdge, FortuneHalfEdge* rightHalfEdge, FortuneParabola* leftParabola, FortuneParabola* rightParabola ) 
		:m_leftHalfEdge(leftHalfEdge), m_rightHalfEdge(rightHalfEdge), m_leftParabola(leftParabola), m_rightParabola(rightParabola) {}
	FortuneHalfEdge* m_leftHalfEdge = nullptr;
	FortuneHalfEdge* m_rightHalfEdge = nullptr;
	FortuneParabola* m_leftParabola = nullptr;
	FortuneParabola* m_rightParabola = nullptr;
	bool m_isActive = true;
	double GetX() const;
	double GetY() const;
};

double FortuneIntersectionPoint::GetX() const
{
	if (m_leftParabola != nullptr && m_rightParabola != nullptr) {
		if (m_leftParabola->m_line) {
			return m_leftParabola->m_focus.x;
		}
		else if (m_rightParabola->m_line) {
			return m_rightParabola->m_focus.x;
		}
		std::pair<double, double> res = SolveParabolaIntersection( m_leftParabola, m_rightParabola );
		if (m_leftHalfEdge->m_dir.x < 0.f) {
			return std::min( res.first, res.second );
		}
		else {
			return std::max( res.first, res.second );
		}
	}
	else if (m_leftParabola == nullptr && m_rightParabola != nullptr) {
		return -10000000000.f;
	}
	else if (m_rightParabola == nullptr) {
		return 10000000000.f;
	}
	else {
		return 0.f;
	}
}

double FortuneIntersectionPoint::GetY() const
{
	double x = GetX();
	if (m_leftParabola) {
		return m_leftParabola->Calculate( x );
	}
	else if (m_rightParabola) {
		return m_rightParabola->Calculate( x );
	}
	return 0.f;
}

struct FortuneEvent {
	FortuneEvent( int type, double triggerY, FortuneSite* site = nullptr );
	FortuneEvent( int type, double triggerY, FortuneIntersectionPoint* pointMeetLeft, FortuneIntersectionPoint* pointMeetRight, FortuneVec2d intersectionPos );
	// 0 site event 1 edge intersection event
	int m_type = 0;
	double m_triggerY = 0.f;
	FortuneSite* m_site = nullptr;
	// edge intersection event
	FortuneVec2d m_intersectionPos;
	FortuneIntersectionPoint* m_pointMeetLeft = nullptr;
	FortuneIntersectionPoint* m_pointMeetRight = nullptr;
};

FortuneEvent::FortuneEvent( int type, double triggerY, FortuneSite* site /*= nullptr */ )
	:m_type(type), m_triggerY(triggerY), m_site(site)
{

}

FortuneEvent::FortuneEvent( int type, double triggerY, FortuneIntersectionPoint* pointMeetLeft, FortuneIntersectionPoint* pointMeetRight, FortuneVec2d intersectionPos )
	:m_type(type), m_triggerY(triggerY), m_pointMeetLeft(pointMeetLeft), m_pointMeetRight(pointMeetRight), m_intersectionPos(intersectionPos)
{

}

struct FortuneEventGreater
{
	bool operator()( FortuneEvent const* a, FortuneEvent const* b ) const {
		return a->m_triggerY > b->m_triggerY;
	}
};

class FortuneAlgorithmSolverClass {
public:
	void FortuneAlgorithmSolver( std::vector<Vec2> const& sitesPos, std::vector<FortuneHalfEdge*>& out_edges );
	void FortuneAlgorithmStepInit( std::vector<Vec2> const& sitesPos );
	void FortuneAlgorithmSingleStep();
	void AddDebugVerts( std::vector<Vertex_PCU>& verts );

	std::priority_queue<FortuneEvent*, std::vector<FortuneEvent*>, FortuneEventGreater> eventList;
	std::vector<FortuneSite> sites;
	std::vector<FortuneParabola*> parabolas;
	std::deque<FortuneIntersectionPoint*> beachline;
	std::vector<FortuneEvent*> debugEventList;
	std::vector<FortuneHalfEdge*> allEdges;
	bool m_stepTest = true;
	bool m_doNextStep = true;
	double directrixY = 0.f;
};

void FortuneAlgorithmSolverClass::FortuneAlgorithmSolver( std::vector<Vec2> const& sitesPos, std::vector<FortuneHalfEdge*>& out_edges ) 
{
	FortuneAlgorithmStepInit( sitesPos );
	while (!eventList.empty()) {
		FortuneAlgorithmSingleStep();
	}

	out_edges = allEdges;

}

void FortuneAlgorithmSolverClass::FortuneAlgorithmStepInit( std::vector<Vec2> const& sitesPos )
{
	for (int i = 0; i < (int)sitesPos.size(); i++) {
		FortuneSite site;
		site.m_index = i;
		site.m_pos = sitesPos[i];
		double xNoise = (double)g_engineRNG->RollRandomFloatInRange( -0.0001f, 0.0001f ) * (double)g_engineRNG->RollRandomFloatInRange( -0.0001f, 0.0001f );
		double yNoise = (double)g_engineRNG->RollRandomFloatInRange( -0.0001f, 0.0001f ) * (double)g_engineRNG->RollRandomFloatInRange( -0.0001f, 0.0001f );
		site.m_pos = site.m_pos + FortuneVec2d(xNoise, yNoise);
		sites.push_back( site );
	}

	for (int i = 0; i < (int)sites.size(); i++) {
		FortuneEvent* event = new FortuneEvent( 0, sites[i].m_pos.y, &sites[i] );
		eventList.emplace( event );
		debugEventList.push_back( event );
	}
}

void FortuneAlgorithmSolverClass::FortuneAlgorithmSingleStep()
{
	// deal with events
	if (eventList.empty()) {
		return;
	}
	FortuneEvent* event = eventList.top();
	eventList.pop();
	directrixY = event->m_triggerY;
	// update parabola
	for (int i = 0; i < (int)parabolas.size(); i++) {
		parabolas[i]->Reset( directrixY );
	}
	if (event->m_type == 0) {
		FortuneVec2d const& thisSitePos = event->m_site->m_pos;
		// deal with site event
		if (!beachline.empty()) {
			FortuneVec2d hitPos;
			// binary search which parabola will hit

			// use simple loop now
			// loop
			for (int i = 0; i < (int)beachline.size() - 1; i++) {
				if (beachline[i]->GetX() <= thisSitePos.x && beachline[i + 1]->GetX() > thisSitePos.x) {
					double y = beachline[i]->m_rightParabola->Calculate( thisSitePos.x );

					// create new parabola
					FortuneParabola* p = new FortuneParabola( thisSitePos );
					parabolas.push_back( p );
					// create new half edges
					FortuneHalfEdge* leftHalfEdge = new FortuneHalfEdge();
					leftHalfEdge->m_parabola = p;
					leftHalfEdge->m_dir = (beachline[i]->m_rightParabola->m_focus - thisSitePos).GetNormalized().GetRotatedMinus90Degrees();
					leftHalfEdge->m_curStartPos = FortuneVec2d( thisSitePos.x, y );
					FortuneHalfEdge* rightHalfEdge = new FortuneHalfEdge();
					rightHalfEdge->m_parabola = p;
					rightHalfEdge->m_dir = -leftHalfEdge->m_dir;
					rightHalfEdge->m_curStartPos = FortuneVec2d( thisSitePos.x, y );

					allEdges.push_back( leftHalfEdge );
					allEdges.push_back( rightHalfEdge );

					// opposite
					leftHalfEdge->m_opposite = rightHalfEdge;
					rightHalfEdge->m_opposite = leftHalfEdge;


					FortuneIntersectionPoint* IP1 = new FortuneIntersectionPoint( leftHalfEdge, rightHalfEdge, beachline[i]->m_rightParabola, p );
					FortuneIntersectionPoint* IP2 = new FortuneIntersectionPoint( rightHalfEdge, leftHalfEdge, p, beachline[i]->m_rightParabola );

					FortuneVec2d intrPos;
					bool res;
					if (beachline[i]->m_leftHalfEdge) {
						res = SolveHalfEdgeIntersection( beachline[i]->m_leftHalfEdge, leftHalfEdge, intrPos );
						if (res) {
							double dist = GetDistance2D( intrPos, IP1->m_leftParabola->m_focus );
							if (intrPos.y + dist > directrixY) {
								FortuneEvent* newEvent = new FortuneEvent( 1, intrPos.y + dist, beachline[i], IP1, intrPos );
								eventList.emplace( newEvent );
								debugEventList.push_back( newEvent );
							}
						}
					}
					/*if (beachline[i]->m_leftHalfEdge) {
						res = SolveHalfEdgeIntersection( beachline[i]->m_leftHalfEdge, rightHalfEdge, intrPos );
						if (res) {
							double dist = GetDistance2D( intrPos, IP1->m_leftParabola->m_focus );
							if (intrPos.y + dist > directrixY) {
								FortuneEvent* newEvent = new FortuneEvent( 1, intrPos.y + dist, beachline[i], IP1, intrPos );
								eventList.emplace( newEvent );
								debugEventList.push_back( newEvent );
							}
						}
					}*/
					if (beachline[i + 1]->m_leftHalfEdge) {
						res = SolveHalfEdgeIntersection( beachline[i + 1]->m_leftHalfEdge, rightHalfEdge, intrPos );
						if (res) {
							double dist = GetDistance2D( intrPos, IP2->m_rightParabola->m_focus );
							if (intrPos.y + dist > directrixY) {
								FortuneEvent* newEvent = new FortuneEvent( 1, intrPos.y + dist, IP2, beachline[i + 1], intrPos );
								eventList.emplace( newEvent );
								debugEventList.push_back( newEvent );
							}
						}
					}
					/*if (beachline[i + 1]->m_leftHalfEdge) {
						res = SolveHalfEdgeIntersection( beachline[i + 1]->m_leftHalfEdge, leftHalfEdge, intrPos );
						if (res) {
							double dist = GetDistance2D( intrPos, IP2->m_rightParabola->m_focus );
							if (intrPos.y + dist > directrixY) {
								FortuneEvent* newEvent = new FortuneEvent( 1, intrPos.y + dist, IP2, beachline[i + 1], intrPos );
								eventList.emplace( newEvent );
								debugEventList.push_back( newEvent );
							}
						}
					}
					*/
					beachline.insert( beachline.begin() + i + 1, IP2 );
					beachline.insert( beachline.begin() + i + 1, IP1 );
					break;
				}
			}
			for (int i = 0; i < (int)beachline.size() - 1; i++) {
				double x1 = beachline[i]->GetX();
				double x2 = beachline[i + 1]->GetX();
				if (x1 > x2) {
					int u = 0;
					double x1 = beachline[i]->GetX();
				}
			}
		}
		else {
			FortuneParabola* p = new FortuneParabola( thisSitePos );
			parabolas.push_back( p );
			FortuneIntersectionPoint* IP1 = new FortuneIntersectionPoint();
			IP1->m_rightParabola = p;
			beachline.push_back( IP1 );
			FortuneIntersectionPoint* IP2 = new FortuneIntersectionPoint();
			IP2->m_leftParabola = p;
			beachline.push_back( IP2 );
		}

	}
	else if (event->m_type == 1) {
		// deal with edge event
		// remove two intersection point
		FortuneIntersectionPoint* IPLeft = event->m_pointMeetLeft;
		FortuneIntersectionPoint* IPRight = event->m_pointMeetRight;
		if (IPLeft->m_isActive && IPRight->m_isActive) {
			FortuneVec2d const& intrPos = event->m_intersectionPos;
			IPLeft->m_rightHalfEdge->m_vertexPos = intrPos;
			IPRight->m_rightHalfEdge->m_vertexPos = intrPos;
			auto iter = std::find( beachline.begin(), beachline.end(), IPLeft );
			FortuneIntersectionPoint* IPLeftLeft = *(iter - 1);
			FortuneIntersectionPoint* IPRightRight = *(iter + 2);
			// insert the new one in front of them
			FortuneHalfEdge* newLeftHalfEdge = new FortuneHalfEdge();
			newLeftHalfEdge->m_curStartPos = intrPos;
			FortuneHalfEdge* newRightHalfEdge = new FortuneHalfEdge();
			newRightHalfEdge->m_curStartPos = intrPos;
			// opposite
			newLeftHalfEdge->m_opposite = newRightHalfEdge;
			newRightHalfEdge->m_opposite = newLeftHalfEdge;
			newLeftHalfEdge->m_dir = (IPLeft->m_leftParabola->m_focus - IPRight->m_rightParabola->m_focus).GetNormalized().GetRotatedMinus90Degrees();
			newRightHalfEdge->m_dir = -newLeftHalfEdge->m_dir;

			// set the vertex pos
			newLeftHalfEdge->m_vertexPos = intrPos;

			// prev and next
			newLeftHalfEdge->m_prev = IPLeft->m_leftHalfEdge;
			IPLeft->m_leftHalfEdge->m_next = newLeftHalfEdge;
			newRightHalfEdge->m_next = IPRight->m_rightHalfEdge;
			IPRight->m_rightHalfEdge->m_prev = newRightHalfEdge;
			IPRight->m_leftHalfEdge->m_next = IPLeft->m_rightHalfEdge;
			IPLeft->m_rightHalfEdge->m_prev = IPRight->m_leftHalfEdge;

			allEdges.push_back( newLeftHalfEdge );
			allEdges.push_back( newRightHalfEdge );

			FortuneIntersectionPoint* IPNew = new FortuneIntersectionPoint( newLeftHalfEdge, newRightHalfEdge, IPLeft->m_leftParabola, IPRight->m_rightParabola );
			beachline.insert( iter, IPNew );
			// erase these two
			iter = std::find( beachline.begin(), beachline.end(), IPLeft );
			beachline.erase( iter, iter + 2 );

			IPLeft->m_isActive = false;
			IPRight->m_isActive = false;
			//delete IPLeft;
			//delete IPRight;
			
			// generate new events
			FortuneVec2d intrCheckPos;
			bool res = false;
			if (IPLeftLeft->m_leftHalfEdge) {
				res = SolveHalfEdgeIntersection( IPLeftLeft->m_leftHalfEdge, newLeftHalfEdge, intrCheckPos );
				if (res) {
					double dist = GetDistance2D( intrCheckPos, IPLeftLeft->m_rightParabola->m_focus );
					if (intrCheckPos.y + dist > directrixY) {
						FortuneEvent* newEvent = new FortuneEvent( 1, intrCheckPos.y + dist, IPLeftLeft, IPNew, intrCheckPos );
						eventList.emplace( newEvent );
						debugEventList.push_back( newEvent );
					}
				}
			}
			if (IPLeftLeft->m_leftHalfEdge) {
				res = SolveHalfEdgeIntersection( IPLeftLeft->m_leftHalfEdge, newRightHalfEdge, intrCheckPos );
				if (res) {
					double dist = GetDistance2D( intrCheckPos, IPLeftLeft->m_rightParabola->m_focus );
					if (intrCheckPos.y + dist > directrixY) {
						FortuneEvent* newEvent = new FortuneEvent( 1, intrCheckPos.y + dist, IPLeftLeft, IPNew, intrCheckPos );
						eventList.emplace( newEvent );
						debugEventList.push_back( newEvent );
					}
				}
			}

			if (IPRightRight->m_leftHalfEdge) {
				res = SolveHalfEdgeIntersection( IPRightRight->m_leftHalfEdge, newLeftHalfEdge, intrCheckPos );
				if (res) {
					double dist = GetDistance2D( intrCheckPos, IPRightRight->m_leftParabola->m_focus );
					if (intrCheckPos.y + dist > directrixY) {
						FortuneEvent* newEvent = new FortuneEvent( 1, intrCheckPos.y + dist, IPNew, IPRightRight, intrCheckPos );
						eventList.emplace( newEvent );
						debugEventList.push_back( newEvent );
					}
				}
			}

			if (IPRightRight->m_leftHalfEdge) {
				res = SolveHalfEdgeIntersection( IPRightRight->m_leftHalfEdge, newRightHalfEdge, intrCheckPos );
				if (res) {
					double dist = GetDistance2D( intrCheckPos, IPRightRight->m_leftParabola->m_focus );
					if (intrCheckPos.y + dist > directrixY) {
						FortuneEvent* newEvent = new FortuneEvent( 1, intrCheckPos.y + dist, IPNew, IPRightRight, intrCheckPos );
						eventList.emplace( newEvent );
						debugEventList.push_back( newEvent );
					}
				}
			}
		}
	}


}
void FortuneDebugDrawAddVertsForParabola( std::vector<Vertex_PCU>& verts, FortuneIntersectionPoint* leftPoint, FortuneIntersectionPoint* rightPoint );

void FortuneAlgorithmSolverClass::AddDebugVerts( std::vector<Vertex_PCU>& verts )
{
	/*for (int i = 0; i < (int)debugEventList.size(); i++) {
		if (debugEventList[i]->m_type == 0) {
			AddVertsForLineSegment2D( verts, Vec2( -100.f, debugEventList[i]->m_triggerY ), Vec2( 1000.f, debugEventList[i]->m_triggerY ), 0.3f, Rgba8( 0, 255, 0 ) );
		}
		else {
			AddVertsForLineSegment2D( verts, Vec2( -100.f, debugEventList[i]->m_triggerY ), Vec2( 1000.f, debugEventList[i]->m_triggerY ), 0.3f, Rgba8( 0, 255, 255 ) );
		}
	}*/
	AddVertsForLineSegment2D( verts, Vec2( -100.f, (float)directrixY ), Vec2( 1000.f, (float)directrixY ), 0.3f, Rgba8( 0, 0, 0 ) );
	for (int i = 0; i < (int)beachline.size() - 1; i++) {
		FortuneDebugDrawAddVertsForParabola( verts, beachline[i], beachline[i + 1] );
	}
	for (int i = 1; i < (int)beachline.size() - 1; i++) {
		if (beachline[i]->m_leftParabola) {
			Vec2 center = Vec2( (float)beachline[i]->GetX(), (float)beachline[i]->m_leftParabola->Calculate( beachline[i]->GetX() ) );
			AddVertsForAABB2D( verts, AABB2( center - Vec2( 0.4f, 0.4f ), center + Vec2( 0.4f, 0.4f ) ), Rgba8( 0, 255, 0 ) );
		}

	}

	for (int i = 0; i < (int)allEdges.size(); i++) {
		if (allEdges[i]->m_next && allEdges[i]->m_prev) {
			AddVertsForLineSegment2D( verts, Vec2((float)allEdges[i]->m_vertexPos.x, (float)allEdges[i]->m_vertexPos.y), 
				Vec2((float)allEdges[i]->m_opposite->m_vertexPos.x, (float)allEdges[i]->m_opposite->m_vertexPos.y),
				0.3f, Rgba8( 96, 96, 96 ) );
		}
	}
}

void FortuneDebugDrawAddVertsForParabola( std::vector<Vertex_PCU>& verts, FortuneIntersectionPoint* leftPoint, FortuneIntersectionPoint* rightPoint )
{
	float leftX = (float)leftPoint->GetX();
	float rightX = (float)rightPoint->GetX();
	FortuneParabola* p = leftPoint->m_rightParabola;
	if (leftX < rightX && p) {
		float step = (rightX - leftX) / 32.f;
		for (int i = 0; i < 32; i++) {
			AddVertsForLineSegment2D( verts, Vec2( (float)leftX + i * step, (float)p->Calculate( leftX + i * step ) ), Vec2( leftX + (i + 1) * step, (float)p->Calculate( leftX + (i + 1) * step ) ), 0.5f, Rgba8( 0, 0, 255 ) );
		}
	}
}