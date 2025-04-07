#include "Game/Unit.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Effect.hpp"

std::vector<float> Unit::s_unitDirYawMap = { 30.f, 90.f, 330.f, 210.f, 270.f, 150.f, 180.f, 0.f, 340.f };

Unit::Unit( Game* game, UnitDefinition const& def )
	:Model(game, def.m_modelFileName)
	,m_def(def)
{
	m_health = m_def.m_health;
	m_movingAnimTimer = new Timer( 1.f / UNIT_MOVE_SPEED, Clock::GetSystemClock() );
	if (m_def.m_type == UnitType::Artillery) {
		m_attackingAnimTimer = new Timer( 1.5f, Clock::GetSystemClock() );
	}
	else if (m_def.m_type == UnitType::Tank) {
		m_attackingAnimTimer = new Timer( 0.2f, Clock::GetSystemClock() );
	}
}

Unit::~Unit()
{
	delete m_attackingAnimTimer;
	delete m_movingAnimTimer;
}

void Unit::Update()
{
	if (m_health <= 0 && m_attackingAnimTimer->IsStopped()) {
		Die();
		return;
	}
	float deltaSeconds = Clock::GetSystemClock()->GetDeltaSeconds();
	if (m_faction == 1) {
		if (this == g_theGame->m_map->m_selectingUnit) {
			m_color = Rgba8( 128, 128, 255 );
		}
		else if (m_performedAction) {
			m_color = Rgba8( 0, 0, 120 );
		}
		else {
			m_color = Rgba8( 50, 78, 200 );
		}
	}
	else if (m_faction == 2) {
		if (this == g_theGame->m_map->m_selectingUnit) {
			m_color = Rgba8( 255, 128, 128 );
		}
		else if (m_performedAction) {
			m_color = Rgba8( 120, 0, 0 );
		}
		else {
			m_color = Rgba8( 200, 78, 50 );
		}
	}

	if (!ReachTargetDir()) {
		//if (!m_movingAnimTimer->IsStopped()) {
		//	m_movingAnimTimer->Pause();
		//}
		if (!m_attackingAnimTimer->IsStopped()) {
			m_attackingAnimTimer->Pause();
		}
		if (m_targetDir != UnitDirection::Custom) {
			float targetYaw = s_unitDirYawMap[(int)m_targetDir];
			if (!m_movingAnimTimer->IsStopped()) {
				m_orientation.m_yawDegrees = GetTurnedTowardDegrees( m_orientation.m_yawDegrees, targetYaw, UNIT_TURN_SPEED * deltaSeconds * 3.f );
			}
			else {
				m_orientation.m_yawDegrees = GetTurnedTowardDegrees( m_orientation.m_yawDegrees, targetYaw, UNIT_TURN_SPEED * deltaSeconds );
			}
			if (m_orientation.m_yawDegrees == targetYaw) {
				m_dir = m_targetDir;
			}
		}
		else {
			if (!m_movingAnimTimer->IsStopped()) {
				m_orientation.m_yawDegrees = GetTurnedTowardDegrees( m_orientation.m_yawDegrees, m_customYawDegrees, UNIT_TURN_SPEED * deltaSeconds * 3.f );
			}
			else {
				m_orientation.m_yawDegrees = GetTurnedTowardDegrees( m_orientation.m_yawDegrees, m_customYawDegrees, UNIT_TURN_SPEED * deltaSeconds );
			}
			if (m_orientation.m_yawDegrees == m_customYawDegrees) {
				m_dir = m_targetDir;
			}
		}
	}
	else {
		if (!m_attackingAnimTimer->IsStopped()) {
			if (m_attackingAnimTimer->IsPaused()) {
				Effect* effect = nullptr;
				for (int i = 0; i < 10; i++) {
					effect = GetCurMap()->SpawnEffect( EffectType::Cone_Muzzle_Particle, GetTurretPosition() );
					((ConeMuzzleParticle*)effect)->m_direction = m_targetUnit->m_position - m_position;
					((ConeMuzzleParticle*)effect)->m_timePeriod = 0.3f;
					((ConeMuzzleParticle*)effect)->m_speed = 0.f;
					((ConeMuzzleParticle*)effect)->m_maxHalfDegrees = 10.f;
					((ConeMuzzleParticle*)effect)->m_size = Vec2( 0.3f, 0.3f );
					effect->BeginPlay();
				}
				for (int i = 0; i < 30; i++) {
					effect = GetCurMap()->SpawnEffect( EffectType::Cone_Smoke_Particle, GetTurretPosition() );
					((ConeSmokeParticle*)effect)->m_direction = m_targetUnit->m_position - m_position;
					((ConeSmokeParticle*)effect)->m_timePeriod = 3.f;
					((ConeSmokeParticle*)effect)->m_speed = 0.3f;
					((ConeSmokeParticle*)effect)->m_maxHalfDegrees = 30.f;
					((ConeSmokeParticle*)effect)->m_size = Vec2( 0.3f, 0.3f );
					effect->BeginPlay();
				}
				if (m_def.m_type == UnitType::Artillery) {
					g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Audio/RocketShot.wav" ) );
				}
				else if (m_def.m_type == UnitType::Tank) {
					g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Audio/TankShot.wav" ) );
				}
				if (m_def.m_type == UnitType::Artillery) {
					effect = GetCurMap()->SpawnEffect( EffectType::Art_Rocket );
					((Rocket*)effect)->m_timePeriod = 1.5f;
					((Rocket*)effect)->m_startPos = GetTurretPosition();
					((Rocket*)effect)->m_endPos = m_targetUnit->m_position;
					effect->BeginPlay();
				}
				m_attackingAnimTimer->Restart();
			}
		}
	}

	if (m_targetTile && m_position != m_targetTile->GetUnitPosition()) {
		if (m_movingAnimTimer->GetElapsedTime() / m_movingAnimTimer->GetPeriodSeconds() > (float)m_spline.GetNumOfCurves()) {
			m_position = m_targetTile->GetUnitPosition();
			m_movingAnimTimer->Stop();
		}
		else {
			m_targetDir = UnitDirection::Custom;
			m_position = m_spline.EvaluateAtParametric( GetClamped( m_movingAnimTimer->GetElapsedTime() / m_movingAnimTimer->GetPeriodSeconds(), 0.f, (float)m_spline.GetNumOfCurves() ) );
			if (m_movingAnimTimer->GetElapsedTime() / m_movingAnimTimer->GetPeriodSeconds() - 0.01f < 0.f) {
				m_customYawDegrees = (m_spline.EvaluateAtParametric( GetClamped( m_movingAnimTimer->GetElapsedTime() / m_movingAnimTimer->GetPeriodSeconds() + 0.01f, 0.f, (float)m_spline.GetNumOfCurves() ) ) - Vec2( m_position )).GetOrientationDegrees();
			}
			else {
				m_customYawDegrees = (Vec2( m_position ) - m_spline.EvaluateAtParametric( GetClamped( m_movingAnimTimer->GetElapsedTime() / m_movingAnimTimer->GetPeriodSeconds() - 0.01f, 0.f, (float)m_spline.GetNumOfCurves() ) )).GetOrientationDegrees();
			}
		}
		//m_position = Interpolate( m_currentRouteToTargetTile[m_currentRouteToTargetTile.size() - 1]->GetUnitPosition(), m_currentRouteToTargetTile[m_currentRouteToTargetTile.size() - 2]->GetUnitPosition(), GetClamped( m_movingAnimTimer->GetElapsedFraction(), 0.f, 1.f ) );
	}

	if (m_targetUnit && !m_endedAttackAnim && m_attackingAnimTimer->HasPeriodElapsed()) {
		m_attackingAnimTimer->Stop();
		m_endedAttackAnim = true;
		m_targetUnit->ResolveDamage( m_def.m_groundAttackDamage, this );
	}
}

void Unit::Render() const
{
	Model::Render();
}

void Unit::Die()
{
	m_isGarbage = true;
	m_tileOn->m_unitOnTile = nullptr;
	if (GetCurMap()->m_selectingUnit == this) {
		GetCurMap()->EndCurUnitTurn();
		GetCurMap()->m_gameState = GameState::ViewingMap;
	}

	Effect* effect = nullptr;
	for (int i = 0; i < 30; i++) {
		effect = GetCurMap()->SpawnEffect( EffectType::Cone_Smoke_Particle, m_position );
		((ConeSmokeParticle*)effect)->m_direction = Vec3( 0.f, 0.f, -1.f );
		((ConeSmokeParticle*)effect)->m_timePeriod = 6.f;
		((ConeSmokeParticle*)effect)->m_speed = 0.3f;
		((ConeSmokeParticle*)effect)->m_maxHalfDegrees = 45.f;
		((ConeSmokeParticle*)effect)->m_size = Vec2( 1.f, 1.f );
		effect->BeginPlay();
	}
	for (int i = 0; i < 10; i++) {
		effect = GetCurMap()->SpawnEffect( EffectType::Sphere_Fire_Particle, m_position );
		((SphereFireParticle*)effect)->m_timePeriod = 3.f;
		((SphereFireParticle*)effect)->m_speed = 0.1f;
		((SphereFireParticle*)effect)->m_size = Vec2( 1.f, 1.f );
		effect->BeginPlay();
	}
	g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Audio/Explosion.wav" ) );
}

void Unit::DebugRender() const
{

}

void Unit::CalculateLegitimateMovingDest()
{
	// bfs
	m_legitimateMovingDest.clear();
	std::deque<std::pair<Tile*, int>> queue;
	std::map<IntVec2, Tile*> dirtyMap;
	queue.push_back( std::pair<Tile*, int>( m_tileOn, 0 ) );
	dirtyMap[m_tileOn->m_coords] = m_tileOn;
	while (!queue.empty()) {
		std::pair<Tile*, int> const& thisTile = queue.front();
		m_legitimateMovingDest.push_back( thisTile.first );
		if (thisTile.second < m_def.m_movementRange) {
			std::vector<Tile*> neighbors;
			thisTile.first->GetAllNeighbors( neighbors );
			for (auto adjTile : neighbors) {
				if (!adjTile->m_def.m_isBlocked && dirtyMap.find( adjTile->m_coords ) == dirtyMap.end()) {
					dirtyMap[adjTile->m_coords] = adjTile;
					queue.push_back( std::pair<Tile*, int>( adjTile, thisTile.second + 1 ) );
				}
			}
		}
		queue.pop_front();
	}
}

void Unit::MoveTo( Tile* tile, bool skipAnimation )
{
	if (!skipAnimation) {
		m_targetTile = tile;
		GetCurMap()->GetRouteBetweenTwoTiles( m_currentRouteToTargetTile, m_tileOn, m_targetTile );
		m_movingAnimTimer->Start();

		std::vector<Vec2> vecList;
		for (int i = (int)m_currentRouteToTargetTile.size() - 1; i >= 0; --i) {
			vecList.push_back( m_currentRouteToTargetTile[i]->GetUnitPosition());
		}
		m_spline = CatmullRomSpline2D( vecList );
	}
	m_prevTile = m_tileOn;
	m_tileOn->m_unitOnTile = nullptr;
	m_tileOn = tile;
	tile->m_unitOnTile = this;
	if (skipAnimation) {
		m_position = m_tileOn->GetUnitPosition();
	}
}

bool Unit::IsUnitInAttackRange( Unit* unit ) const
{
	int dist = g_theGame->m_map->GetDistBetweenTwoTiles( m_tileOn, unit->m_tileOn );
	return dist >= m_def.m_groundAttackRangeMin && dist <= m_def.m_groundAttackRangeMax && m_faction != unit->m_faction;
}

void Unit::Fire( Unit* target )
{
	m_attackingAnimTimer->Start();
	m_endedAttackAnim = false;
	m_targetUnit = target;
	m_attackingAnimTimer->Pause();
	//target->ResolveDamage( m_def.m_groundAttackDamage, this );
}

void Unit::ResolveDamage( int damage, Unit* source )
{
	m_health -= 2 * damage / m_def.m_defense;
	Effect* damageNumberEffect = GetCurMap()->SpawnEffect( EffectType::Damage_Number, m_position + Vec3( 0.f, 0.f, 0.3f ) );
	((DamageNumber*)damageNumberEffect)->m_damageValue = 2 * damage / m_def.m_defense;
	if (source) {
		for (int i = 0; i < 20; i++) {
			Effect* hitEffect = GetCurMap()->SpawnEffect( EffectType::Cone_Smoke_Particle, m_position );
			((ConeSmokeParticle*)hitEffect)->m_direction = (m_position - source->m_position);
			hitEffect->BeginPlay();
		}
	}
	g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Audio/Hit.wav" ) );
}

void Unit::GetAllEnemyUnitsInRange( std::vector<Unit*>& out_units ) const
{
	out_units.clear();
	Map* map = g_theGame->m_map;
	for (auto unit : map->m_units) {
		if (unit && unit->m_faction != m_faction && IsUnitInAttackRange( unit )) {
			out_units.push_back( unit );
		}
	}
}

bool Unit::ReachTargetDir() const
{
	if (m_targetDir == UnitDirection::Custom) {
		if (m_orientation.m_yawDegrees == m_customYawDegrees) {
			return true;
		}
		return false;
	}
	else {
		if (m_orientation.m_yawDegrees == s_unitDirYawMap[(int)m_targetDir]) {
			return true;
		}
		return false;
	}
}

Vec3 Unit::GetTurretPosition() const
{
	return m_position + Vec2::MakeFromPolarDegrees( m_orientation.m_yawDegrees, 0.4f ) + Vec3( 0.f, 0.f, 0.3f );
}

std::vector<UnitDefinition> UnitDefinition::s_definitions;

UnitDefinition::UnitDefinition( XmlElement* elem )
{
	m_symbol = ParseXmlAttribute( *elem, "symbol", m_symbol );
	m_name = ParseXmlAttribute( *elem, "name", m_name );
	m_imageFileName = ParseXmlAttribute( *elem, "imageFilename", m_imageFileName );
	m_modelFileName = ParseXmlAttribute( *elem, "modelFilename", m_modelFileName );
	std::string typeName = ParseXmlAttribute( *elem, "type", "Tank" );
	if (typeName == "Tank") {
		m_type = UnitType::Tank;
	}
	else if (typeName == "Artillery") {
		m_type = UnitType::Artillery;
	}
	else {
		m_type = UnitType::None;
	}
	m_groundAttackDamage = ParseXmlAttribute( *elem, "groundAttackDamage", m_groundAttackDamage );
	m_groundAttackRangeMin = ParseXmlAttribute( *elem, "groundAttackRangeMin", m_groundAttackRangeMin );
	m_groundAttackRangeMax = ParseXmlAttribute( *elem, "groundAttackRangeMax", m_groundAttackRangeMax );
	m_movementRange = ParseXmlAttribute( *elem, "movementRange", m_movementRange );
	m_defense = ParseXmlAttribute( *elem, "defense", m_defense );
	m_health = ParseXmlAttribute( *elem, "health", m_health );
	m_imageFile = g_theRenderer->CreateOrGetTextureFromFile( m_imageFileName.c_str() );
}

UnitDefinition const& UnitDefinition::GetDefinition( std::string const& defName )
{
	for (auto const& def : s_definitions) {
		if (def.m_name == defName) {
			return def;
		}
	}
	ERROR_AND_DIE( Stringf( "Do not have unit name %s", defName.c_str() ) );
	//g_devConsole->AddLine( DevConsole::INFO_ERROR, Stringf( "Do not have unit name %s", defName.c_str() ) );
	//return *((UnitDefinition*)(nullptr));
}

UnitDefinition const& UnitDefinition::GetDefinition( char symbol )
{
	for (auto const& def : s_definitions) {
		if (def.m_symbol == symbol) {
			return def;
		}
	}
	ERROR_AND_DIE( Stringf( "Do not have unit name %c", symbol ) );
	//g_devConsole->AddLine( DevConsole::INFO_ERROR, Stringf( "Do not have unit name %c", symbol ) );
	//return *((UnitDefinition*)(nullptr));
}
