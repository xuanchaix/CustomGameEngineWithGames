#include "Game/Enemy.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"

EnemyBase::EnemyBase( Vec2 const& startPos, EnemyDefinition const& def )
	:Entity(startPos, g_theGame)
	,m_def(def)
	,m_clock(*g_theGame->m_gameClock)
	,m_movementAnimTimer(1.f / m_def.m_movingSpeed, &m_clock)
	,m_attackAnimTimer(m_def.m_attackCooldown, &m_clock)
	,m_deathTimer(1.f, &m_clock)
{
	m_entityType = EntityType::Enemy;
	m_speed = m_def.m_movingSpeed;
	m_maxHealth = m_def.m_maxHealth;
	m_health = m_maxHealth;
	m_physicsBounds = AABB2( Vec2( 0.f, 0.f ), Vec2( m_def.m_physicsBounds ) );
	m_visualBounds = AABB2( Vec2( 0.f, 0.f ), Vec2( m_def.m_visualBounds ) );
	m_enemyState = EnemyState::Idle;
	m_damage = m_def.m_damage;
	m_physicsDiscRadius = m_def.m_physicsBounds.GetLength() * 0.2f;
}

EnemyBase::~EnemyBase()
{

}

void EnemyBase::Update( float deltaTime )
{
	if (m_isDead && m_deathTimer.HasStartedAndNotPeriodElapsed()) {

		return;
	}
	else if (m_isDead && m_deathTimer.HasPeriodElapsed()) {
		m_isGarbage = true;
		return;
	}
	if (m_health <= 0.f) {
		m_isDead = true;
		m_enemyState = EnemyState::Dying;
		m_deathTimer.Start();
		return;
	}

	if (m_enemyState == EnemyState::Running) {
		// no target or target is dead
		if (m_attackingTarget == nullptr || m_attackingTarget->m_isDead) {
			if (ChooseTarget()) { // if find a new target
				// if near the target and rotation is right
				if (DoAABB2sOverlap2D( GetWorldPhysicsBounds(), m_attackingTarget->GetWorldPhysicsBounds() ) && m_orientationDegrees == (m_attackingTarget->m_position - m_position).GetOrientationDegrees()) {
					m_enemyState = EnemyState::Attacking;
					m_attackAnimTimer.Start();
					m_movementAnimTimer.Stop();
				}
				else { // if need to reach the target
					m_enemyState = EnemyState::Running;
					m_movementAnimTimer.Start();
				}
			}
			else { // not find a new target? idle
				m_attackingTarget = nullptr;
				m_movementAnimTimer.Stop();
				m_enemyState = EnemyState::Idle;
			}
		} // reach target, start attack!
		else if (DoAABB2sOverlap2D( GetWorldPhysicsBounds(), m_attackingTarget->GetWorldPhysicsBounds() ) && std::abs(GetShortestAngularDispDegrees( m_orientationDegrees, (m_attackingTarget->m_position - m_position).GetOrientationDegrees())) < 5.f) {
			m_enemyState = EnemyState::Attacking;
			m_attackAnimTimer.Start();
			m_movementAnimTimer.Stop();
		}
		else {
			m_movementAnimTimer.DecrementPeriodIfElapsed();
			m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, (m_attackingTarget->m_position - m_position).GetOrientationDegrees(), deltaTime * m_def.m_turnSpeed );
			if (!DoAABB2sOverlap2D( GetWorldPhysicsBounds(), m_attackingTarget->GetWorldPhysicsBounds() )) {
				m_position += GetForwardNormal() * deltaTime * m_speed;
			}
		}
	}
	else if (m_enemyState == EnemyState::Attacking) {
		// no target or target is dead
		if (m_attackingTarget == nullptr || m_attackingTarget->m_isDead) {
			if (ChooseTarget()) { // if find a new target
				// if near the target and rotation is right
				if (DoAABB2sOverlap2D( GetWorldPhysicsBounds(), m_attackingTarget->GetWorldPhysicsBounds() ) && m_orientationDegrees == (m_attackingTarget->m_position - m_position).GetOrientationDegrees() ) {
					m_enemyState = EnemyState::Attacking;
					m_attackAnimTimer.Start();
				}
				else { // if need to reach the target
					m_enemyState = EnemyState::Running;
					m_attackAnimTimer.Stop();
					m_movementAnimTimer.Start();
				}
			}
			else { // not find a new target? idle
				m_attackingTarget = nullptr;
				m_attackAnimTimer.Stop();
				m_enemyState = EnemyState::Idle;
			}
			return;
		}
		else {
			// attack!
			if (m_attackAnimTimer.DecrementPeriodIfElapsed()) {
				AttackTarget( m_attackingTarget );
			}
		}
	}
	else if (m_enemyState == EnemyState::Idle) {
		if (ChooseTarget()) { // if find a new target
			// if near the target and rotation is right
			if (DoAABB2sOverlap2D( GetWorldPhysicsBounds(), m_attackingTarget->GetWorldPhysicsBounds() ) && m_orientationDegrees == (m_attackingTarget->m_position - m_position).GetOrientationDegrees()) {
				m_enemyState = EnemyState::Attacking;
				m_attackAnimTimer.Start();
			}
			else { // if need to reach the target
				m_enemyState = EnemyState::Running;
				m_attackAnimTimer.Stop();
				m_movementAnimTimer.Start();
			}
		}
		else { // not find a new target? idle

		}
	}
}

void EnemyBase::Render() const
{
	std::vector<Vertex_PCU> verts;
	if (m_def.m_id == 0) {
		if (m_enemyState == EnemyState::Running || m_enemyState == EnemyState::Idle) {
			constexpr float runningStep = 360.f / 16.f;
			constexpr float runningHalfStep = 360.f / 16.f * 0.5f;
			float normalizedDegrees = NormalizeDegrees180( m_orientationDegrees );
			int index = 0;
			if (normalizedDegrees < -180.f + runningHalfStep || normalizedDegrees >= 180.f - runningHalfStep) {
				index = 15;
			}
			else {
				index = RoundDownToInt( (normalizedDegrees + 180.f - runningHalfStep) / runningStep );
			}
			int xIndex = 0;
			if (m_enemyState == EnemyState::Running) {
				xIndex = RoundDownToInt( m_movementAnimTimer.GetElapsedFraction() * 16.f );
			}

			if (index >= 0 && index < 4) {
				AddVertsForAABB2D( verts, AABB2( -m_visualBounds.m_maxs * 0.5f, m_visualBounds.m_maxs * 0.5f ), Rgba8::WHITE, m_def.m_runSprite2.GetSpriteUVs( (3 - index) * 16 + xIndex ) );
				g_theRenderer->BindTexture( &m_def.m_runSprite2.GetTexture() );
			}
			else if (index >= 4 && index < 8) {
				AddVertsForAABB2D( verts, AABB2(-m_visualBounds.m_maxs * 0.5f, m_visualBounds.m_maxs * 0.5f), Rgba8::WHITE, m_def.m_runSprite1.GetSpriteUVs( (7 - index) * 16 + xIndex ) );
				g_theRenderer->BindTexture( &m_def.m_runSprite1.GetTexture() );
			}
			else if (index >= 8 && index < 12) {
				AddVertsForAABB2D( verts, AABB2(-m_visualBounds.m_maxs * 0.5f, m_visualBounds.m_maxs * 0.5f), Rgba8::WHITE, m_def.m_runSprite0.GetSpriteUVs( (11 - index) * 16 + xIndex ) );
				g_theRenderer->BindTexture( &m_def.m_runSprite0.GetTexture() );
			}
			else if (index >= 12 && index < 16) {
				AddVertsForAABB2D( verts, AABB2(-m_visualBounds.m_maxs * 0.5f, m_visualBounds.m_maxs * 0.5f), Rgba8::WHITE, m_def.m_runSprite3.GetSpriteUVs( (15 - index) * 16 + xIndex ) );
				g_theRenderer->BindTexture( &m_def.m_runSprite3.GetTexture() );
			}
		}
		else if (m_enemyState == EnemyState::Attacking) {
			constexpr float attackingStep = 360.f / 16.f;
			constexpr float attackingHalfStep = 360.f / 16.f * 0.5f;
			float normalizedDegrees = NormalizeDegrees180( m_orientationDegrees );
			int index = 0;
			if (normalizedDegrees < -180.f + attackingHalfStep || normalizedDegrees >= 180.f - attackingHalfStep) {
				index = 15;
			}
			else {
				index = RoundDownToInt( (normalizedDegrees + 180.f - attackingHalfStep) / attackingStep );
			}
			int xIndex = RoundDownToInt( m_attackAnimTimer.GetElapsedFraction() * 11.f );

			if (index >= 0 && index < 4) {
				AddVertsForAABB2D( verts, AABB2( -m_visualBounds.m_maxs * 0.5f, m_visualBounds.m_maxs * 0.5f ), Rgba8::WHITE, m_def.m_attackSprite2.GetSpriteUVs( (3 - index) * 11 + xIndex ) );
				g_theRenderer->BindTexture( &m_def.m_attackSprite2.GetTexture() );
			}
			else if (index >= 4 && index < 8) {
				AddVertsForAABB2D( verts, AABB2( -m_visualBounds.m_maxs * 0.5f, m_visualBounds.m_maxs * 0.5f ), Rgba8::WHITE, m_def.m_attackSprite1.GetSpriteUVs( (7 - index) * 11 + xIndex ) );
				g_theRenderer->BindTexture( &m_def.m_attackSprite1.GetTexture() );
			}
			else if (index >= 8 && index < 12) {
				AddVertsForAABB2D( verts, AABB2( -m_visualBounds.m_maxs * 0.5f, m_visualBounds.m_maxs * 0.5f ), Rgba8::WHITE, m_def.m_attackSprite0.GetSpriteUVs( (11 - index) * 11 + xIndex ) );
				g_theRenderer->BindTexture( &m_def.m_attackSprite0.GetTexture() );
			}
			else if (index >= 12 && index < 16) {
				AddVertsForAABB2D( verts, AABB2( -m_visualBounds.m_maxs * 0.5f, m_visualBounds.m_maxs * 0.5f ), Rgba8::WHITE, m_def.m_attackSprite3.GetSpriteUVs( (15 - index) * 11 + xIndex ) );
				g_theRenderer->BindTexture( &m_def.m_attackSprite3.GetTexture() );
			}
		}
		else if (m_enemyState == EnemyState::Dying) {
			constexpr float deathStep = 360.f / 16.f;
			constexpr float deathHalfStep = 360.f / 16.f * 0.5f;
			float normalizedDegrees = NormalizeDegrees180( m_orientationDegrees );
			int index = 0;
			if (normalizedDegrees < -180.f + deathHalfStep || normalizedDegrees >= 180.f - deathHalfStep) {
				index = 15;
			}
			else {
				index = RoundDownToInt( (normalizedDegrees + 180.f - deathHalfStep) / deathStep );
			}
			int xIndex = RoundDownToInt( m_deathTimer.GetElapsedFraction() * 11.f );
			int uvIndex = -1;
			if (index >= 0 && index < 4) {
				uvIndex = (11 - index) * 17 + xIndex;
			}
			else if (index >= 4 && index < 8) {
				uvIndex = (11 - index) * 17 + xIndex;
			}
			else if (index >= 8 && index < 12) {
				uvIndex = (11 - index) * 17 + xIndex;
			}
			else if (index >= 12 && index < 16) {
				uvIndex = (27 - index) * 17 + xIndex;
			}
			if (uvIndex < 72) {
				AddVertsForAABB2D( verts, AABB2( -m_visualBounds.m_maxs * 0.5f, m_visualBounds.m_maxs * 0.5f ), Rgba8::WHITE, m_def.m_dieSprite0.GetSpriteUVs( uvIndex ) );
				g_theRenderer->BindTexture( &m_def.m_dieSprite0.GetTexture() );
			}
			else if (uvIndex < 144) {
				AddVertsForAABB2D( verts, AABB2( -m_visualBounds.m_maxs * 0.5f, m_visualBounds.m_maxs * 0.5f ), Rgba8::WHITE, m_def.m_dieSprite1.GetSpriteUVs( uvIndex - 72 ) );
				g_theRenderer->BindTexture( &m_def.m_dieSprite1.GetTexture() );
			}
			else if (uvIndex < 216) {
				AddVertsForAABB2D( verts, AABB2( -m_visualBounds.m_maxs * 0.5f, m_visualBounds.m_maxs * 0.5f ), Rgba8::WHITE, m_def.m_dieSprite2.GetSpriteUVs( uvIndex - 144 ) );
				g_theRenderer->BindTexture( &m_def.m_dieSprite2.GetTexture() );
			}
			else {
				AddVertsForAABB2D( verts, AABB2( -m_visualBounds.m_maxs * 0.5f, m_visualBounds.m_maxs * 0.5f ), Rgba8::WHITE, m_def.m_dieSprite3.GetSpriteUVs( uvIndex - 216 ) );
				g_theRenderer->BindTexture( &m_def.m_dieSprite3.GetTexture() );
			}
		}
	}

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetModelConstants( GetPositionModelConstants(), m_color );
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );

	// draw normal
// 	verts.clear();
// 
// 	AddVertsForArrow2D( verts, m_position, m_position + GetForwardNormal() * 2.f, 0.2f, 0.2f, Rgba8( 0, 255, 0 ) );
// 	g_theRenderer->BindTexture( nullptr );
// 	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
// 	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
// 	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
// 	g_theRenderer->BindShader( nullptr );
// 	g_theRenderer->SetModelConstants( );
// 	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
// 
// 	g_theRenderer->DrawVertexArray( verts );
}

void EnemyBase::Die()
{
	m_isDead = true;
}

bool EnemyBase::ChooseTarget()
{
	m_attackingTarget = nullptr;
	// choose the nearest target
	Map* map = GetCurMap();
	float minDistance = FLT_MAX;
	for (auto entity : map->m_entities) {
		if (entity && entity->IsAlive() && entity->m_entityType == EntityType::Building) {
			float distSquared = GetDistanceSquared2D( m_position, entity->m_position );
			if (distSquared < minDistance) {
				minDistance = distSquared;
				m_attackingTarget = entity;
			}
		}
	}
	if (m_attackingTarget) {
		m_movingTargetPos = m_attackingTarget->m_position;
		return true;
	}
	return false;
}

void EnemyBase::AttackTarget( Entity* target )
{
	target->BeAttacked( m_def.m_damage );
	UNUSED( target );
}

EnemyDefinition::EnemyDefinition( XmlElement* xmlIter )
	:m_runSprite0( *g_theRenderer->CreateOrGetTextureFromFile( ParseXmlAttribute( *xmlIter, "runSprite0Path", "" ).c_str() ), ParseXmlAttribute( *xmlIter, "runSprite0Layout", IntVec2( 1, 1 ) ) )
	,m_runSprite1( *g_theRenderer->CreateOrGetTextureFromFile( ParseXmlAttribute( *xmlIter, "runSprite1Path", "" ).c_str() ), ParseXmlAttribute( *xmlIter, "runSprite1Layout", IntVec2( 1, 1 ) ) )
	,m_runSprite2( *g_theRenderer->CreateOrGetTextureFromFile( ParseXmlAttribute( *xmlIter, "runSprite2Path", "" ).c_str() ), ParseXmlAttribute( *xmlIter, "runSprite2Layout", IntVec2( 1, 1 ) ) )
	,m_runSprite3( *g_theRenderer->CreateOrGetTextureFromFile( ParseXmlAttribute( *xmlIter, "runSprite3Path", "" ).c_str() ), ParseXmlAttribute( *xmlIter, "runSprite3Layout", IntVec2( 1, 1 ) ) )
	,m_attackSprite0( *g_theRenderer->CreateOrGetTextureFromFile( ParseXmlAttribute( *xmlIter, "attackSprite0Path", "" ).c_str() ), ParseXmlAttribute( *xmlIter, "attackSprite0Layout", IntVec2( 1, 1 ) ) )
	,m_attackSprite1( *g_theRenderer->CreateOrGetTextureFromFile( ParseXmlAttribute( *xmlIter, "attackSprite1Path", "" ).c_str() ), ParseXmlAttribute( *xmlIter, "attackSprite1Layout", IntVec2( 1, 1 ) ) )
	,m_attackSprite2( *g_theRenderer->CreateOrGetTextureFromFile( ParseXmlAttribute( *xmlIter, "attackSprite2Path", "" ).c_str() ), ParseXmlAttribute( *xmlIter, "attackSprite2Layout", IntVec2( 1, 1 ) ) )
	,m_attackSprite3( *g_theRenderer->CreateOrGetTextureFromFile( ParseXmlAttribute( *xmlIter, "attackSprite3Path", "" ).c_str() ), ParseXmlAttribute( *xmlIter, "attackSprite3Layout", IntVec2( 1, 1 ) ) )
	,m_dieSprite0( *g_theRenderer->CreateOrGetTextureFromFile( ParseXmlAttribute( *xmlIter, "dieSprite0Path", "" ).c_str() ), ParseXmlAttribute( *xmlIter, "dieSprite0Layout", IntVec2( 1, 1 ) ) )
	,m_dieSprite1( *g_theRenderer->CreateOrGetTextureFromFile( ParseXmlAttribute( *xmlIter, "dieSprite1Path", "" ).c_str() ), ParseXmlAttribute( *xmlIter, "dieSprite1Layout", IntVec2( 1, 1 ) ) )
	,m_dieSprite2( *g_theRenderer->CreateOrGetTextureFromFile( ParseXmlAttribute( *xmlIter, "dieSprite2Path", "" ).c_str() ), ParseXmlAttribute( *xmlIter, "dieSprite2Layout", IntVec2( 1, 1 ) ) )
	,m_dieSprite3( *g_theRenderer->CreateOrGetTextureFromFile( ParseXmlAttribute( *xmlIter, "dieSprite3Path", "" ).c_str() ), ParseXmlAttribute( *xmlIter, "dieSprite3Layout", IntVec2( 1, 1 ) ) )
{
	m_name = ParseXmlAttribute( *xmlIter, "name", m_name );
	m_id = ParseXmlAttribute( *xmlIter, "id", m_id );
	m_damage = ParseXmlAttribute( *xmlIter, "damage", m_damage );
	m_maxHealth = ParseXmlAttribute( *xmlIter, "maxHealth", m_maxHealth );
	m_movingSpeed = ParseXmlAttribute( *xmlIter, "movingSpeed", m_movingSpeed );
	m_turnSpeed = ParseXmlAttribute( *xmlIter, "turnSpeed", m_turnSpeed );
	m_attackCooldown = ParseXmlAttribute( *xmlIter, "attackCooldown", m_attackCooldown );
	m_range = ParseXmlAttribute( *xmlIter, "range", m_range );
	std::string AIName = ParseXmlAttribute( *xmlIter, "AIName", "None" );
	if (AIName != "None") {
		if (AIName == "NearestTarget") {
			m_AI = 0;
		}
	}
	m_physicsBounds = ParseXmlAttribute( *xmlIter, "physicsBounds", m_physicsBounds );
	m_visualBounds = ParseXmlAttribute( *xmlIter, "visualBounds", m_visualBounds );
	m_color = ParseXmlAttribute( *xmlIter, "tintColor", m_color );

}

void EnemyDefinition::SetUpEnemyDefinitions()
{
	EnemyDefinition::s_definitions.reserve( 64 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Definitions/EnemyDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document EnemyDefinitions.xml error" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "EnemyDefinitions" ), "Syntax Error! Name of the root of EnemyDefinitions.xml should be \"EnemyDefinitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "EnemyDefinition" ), "Syntax Error! Names of the elements of EnemyDefinitions.xml should be \"EnemyDefinition\" " );
		EnemyDefinition::s_definitions.emplace_back( xmlIter );
		xmlIter = xmlIter->NextSiblingElement();
	}
}

std::vector<EnemyDefinition> EnemyDefinition::s_definitions;

EnemyDefinition const& EnemyDefinition::GetDefinition( std::string const& name )
{
	for (auto const& def : s_definitions) {
		if (def.m_name == name) {
			return def;
		}
	}
	ERROR_AND_DIE( "Cannot find enemy definition" );
}

EnemyDefinition const& EnemyDefinition::GetDefinition( int id )
{
	return s_definitions[id];
}
