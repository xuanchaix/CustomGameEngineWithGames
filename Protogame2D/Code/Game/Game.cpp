#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Entity.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"

class DummyJob :public Job {
public:
	DummyJob( int sleepMilliseconds ) { m_sleepMilliseconds = sleepMilliseconds; };
	virtual ~DummyJob() {};
	virtual void Execute() override;

	int m_sleepMilliseconds = 0;
};

void DummyJob::Execute()
{
	std::this_thread::sleep_for( std::chrono::milliseconds( m_sleepMilliseconds ) );
}

Game::Game()
{
	// load random number generator
	m_randNumGen = new RandomNumberGenerator();
	m_gameClock = new Clock();
}

Game::~Game()
{
	// delete all pointers managed by Game
	delete m_randNumGen;
	m_randNumGen = nullptr;

}

void Game::Startup()
{
	// set up camera
	m_screenCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( UI_SIZE_X, UI_SIZE_Y ), 1.f, -1.f );
	m_screenCamera.m_mode = CameraMode::Orthographic;
	m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), 1.f, -1.f );
	m_worldCamera.m_mode = CameraMode::Orthographic;
}

void Game::Update()
{
	HandleKeys();
}

static std::vector<std::string> g_instructionStrs;

void Game::Render() const
{
	// Game Camera
	g_theRenderer->BeginCamera( m_worldCamera );

	g_theRenderer->EndCamera( m_worldCamera );
	// UI Camera
	g_theRenderer->BeginCamera( m_screenCamera );

	// render threading test game
	std::vector<Vertex_PCU> verts;
	Vec2 LeftTopPos( 25.f, 787.5f );
	constexpr float totalWidth = 1550.f;
	constexpr float totalHeight = 775.f;
	constexpr float widthPerBlock = totalWidth / 40.f;
	constexpr float heightPerBlock = totalHeight / 20.f;
	constexpr float margin = 0.1f;

	for (int i = (int)m_jobList.size() > 800 ? (int)m_jobList.size() - 800 : 0; i < (int)m_jobList.size(); i++) {
		int k = i;
		if ((int)m_jobList.size() > 800) {
			k = k + 800 - (int)m_jobList.size();
		}
		int y = k / 40;
		int x = k - y * 40;
		Rgba8 color;
		if (m_jobList[i]->m_status == JobStatus::Queued) {
			color = Rgba8( 255, 0, 0 );
		}
		else if (m_jobList[i]->m_status == JobStatus::Executing) {
			color = Rgba8( 255, 255, 0 );
		}
		else if (m_jobList[i]->m_status == JobStatus::Completed) {
			color = Rgba8( 0, 255, 0 );
		}
		else if (m_jobList[i]->m_status == JobStatus::Retrieved) {
			color = Rgba8( 0, 0, 255 );
		}
		else if (m_jobList[i]->m_status == JobStatus::NoRecord) {
			color = Rgba8( 255, 0, 255 );
		}
		Vec2 leftBottomPos = LeftTopPos + Vec2( widthPerBlock * (float)x, -heightPerBlock * (float)(y + 1) );
		AddVertsForAABB2D( verts, AABB2( leftBottomPos + Vec2( widthPerBlock, heightPerBlock ) * margin, leftBottomPos + Vec2( widthPerBlock, heightPerBlock ) * (1.f - margin) ), color );
	}

	// info
	Vec2 leftBottomPos =  LeftTopPos + Vec2( widthPerBlock * 29.f, -heightPerBlock * 20.f );
	Rgba8 color = Rgba8( 255, 0, 0 );
	AddVertsForAABB2D( verts, AABB2( leftBottomPos + Vec2( widthPerBlock, heightPerBlock ) * margin, leftBottomPos + Vec2( widthPerBlock, heightPerBlock ) * (1.f - margin) ), color );
	leftBottomPos = LeftTopPos + Vec2( widthPerBlock * 32.f, -heightPerBlock * 20.f );
	color = Rgba8( 255, 255, 0 );
	AddVertsForAABB2D( verts, AABB2( leftBottomPos + Vec2( widthPerBlock, heightPerBlock ) * margin, leftBottomPos + Vec2( widthPerBlock, heightPerBlock ) * (1.f - margin) ), color );
	leftBottomPos = LeftTopPos + Vec2( widthPerBlock * 35.f, -heightPerBlock * 20.f );
	color = Rgba8( 0, 255, 0 );
	AddVertsForAABB2D( verts, AABB2( leftBottomPos + Vec2( widthPerBlock, heightPerBlock ) * margin, leftBottomPos + Vec2( widthPerBlock, heightPerBlock ) * (1.f - margin) ), color );
	leftBottomPos = LeftTopPos + Vec2( widthPerBlock * 38.f, -heightPerBlock * 20.f );
	color = Rgba8( 0, 0, 255 );
	AddVertsForAABB2D( verts, AABB2( leftBottomPos + Vec2( widthPerBlock, heightPerBlock ) * margin, leftBottomPos + Vec2( widthPerBlock, heightPerBlock ) * (1.f - margin) ), color );

	std::vector<Vertex_PCU> textVerts;
	leftBottomPos = LeftTopPos + Vec2( widthPerBlock * 28.f, -heightPerBlock * 19.3f );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( leftBottomPos,
		leftBottomPos + Vec2( widthPerBlock * 3.f, heightPerBlock ) ), 40.f, "Queued" );
	leftBottomPos = LeftTopPos + Vec2( widthPerBlock * 31.f, -heightPerBlock * 19.3f );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( leftBottomPos + Vec2( widthPerBlock * 2.f, heightPerBlock ) * margin,
		leftBottomPos + Vec2( widthPerBlock * 3.f, heightPerBlock ) ), 40.f, "Executing" );
	leftBottomPos = LeftTopPos + Vec2( widthPerBlock * 34.f, -heightPerBlock * 19.3f );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( leftBottomPos + Vec2( widthPerBlock * 2.f, heightPerBlock ) * margin,
		leftBottomPos + Vec2( widthPerBlock * 3.f, heightPerBlock) ), 40.f, "Completed" );
	leftBottomPos = LeftTopPos + Vec2( widthPerBlock * 37.f, -heightPerBlock * 19.3f );
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( leftBottomPos + Vec2( widthPerBlock * 2.f, heightPerBlock ) * margin,
		leftBottomPos + Vec2( widthPerBlock * 3.f, heightPerBlock ) ), 40.f, "Retrieved" );

	int k = 0;
	if (!g_instructionStrs.empty()) {
		for (int i = (int)g_instructionStrs.size() - 1; i > (int)g_instructionStrs.size() - 6 && i >= 0; --i) {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 0.f, 25.f * k ), Vec2( 1600.f, 25.f * (k + 1) ) ), 40.f, g_instructionStrs[i], Rgba8( 0, 255, 0 ) );
			++k;
		}
	}


	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( );
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );

	g_theRenderer->EndCamera( m_screenCamera );
}

void Game::HandleKeys()
{
	if (g_theInput->WasKeyJustPressed( 'J' )) {
		int sleepSeconds = g_engineRNG->RollRandomIntInRange( 50, 3000 );
		Job* job = new DummyJob( sleepSeconds );
		m_jobList.push_back( job );
		g_theJobSystem->AddJob( job );
		g_instructionStrs.push_back( "Add 1 job" );
	}
	if (g_theInput->WasKeyJustPressed( 'N' )) {
		for (int i = 0; i < 100; i++) {
			int sleepSeconds = g_engineRNG->RollRandomIntInRange( 50, 3000 );
			Job* job = new DummyJob( sleepSeconds );
			m_jobList.push_back( job );
			g_theJobSystem->AddJob( job );
		}
		g_instructionStrs.push_back( "Add 100 jobs" );
	}
	if (g_theInput->WasKeyJustPressed( 'R' )) {
		bool m_canRetrieve = g_theJobSystem->RetrieveJob( m_jobList[m_jobListFirst] );
		if (m_canRetrieve) {
			m_jobListFirst++;
			//delete m_jobList.front();
			//m_jobList.pop_front();
			g_instructionStrs.push_back( "Retrieve 1 job" );
		}
	}
	if (g_theInput->WasKeyJustPressed( 'A' )) {
		bool m_canRetrieve = false;
		do {
			m_canRetrieve = g_theJobSystem->RetrieveJob( m_jobList[m_jobListFirst] );
			if (m_canRetrieve) {
				m_jobListFirst++;
				//delete m_jobList.front();
				//m_jobList.pop_front();
				g_instructionStrs.push_back( "Retrieve all finished jobs from head" );
			}
		} while (m_canRetrieve);
	}


#ifdef DEBUG_MODE
	m_gameClock->SetTimeScale( 1.f );

	if (g_theInput->WasKeyJustPressed( 0x4F )) {// O key run a single frame and pauses
		m_gameClock->StepSingleFrame();
	}

	if (g_theInput->IsKeyDown( 0x54 )) // T key slow the game
	{
		m_gameClock->SetTimeScale( 0.1f );
	}

	if (g_theInput->WasKeyJustPressed( 0x50 )) // P key pause the game; handle the pause problem
	{
		m_gameClock->TogglePause();
	}
#endif // DEBUG_MODE
}
