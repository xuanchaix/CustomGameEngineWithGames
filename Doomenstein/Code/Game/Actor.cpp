#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerController.hpp"
#include "Game/AIController.hpp"
#include "Game/Weapon.hpp"
#include "Game/App.hpp"

std::vector<ActorDefinition> ActorDefinition::s_definitions;

Actor::Actor( Map* map, ActorDefinition const& def )
	:m_map(map)
	,m_def(def)
	,m_color(def.m_tintColor)
	,m_height(def.m_physicsHeight)
	,m_physicsRadius(def.m_physicsRadius)
	,m_health(def.m_maxHealth)
	,m_maxHealth(def.m_maxHealth)
	,m_destroyTime(def.m_corpseLiveSeconds)
{
	m_animationClock = new Clock( *g_theGame->m_gameClock );
	m_animationTimer = new Timer( 100.f, m_animationClock );
}

Actor::~Actor()
{
	delete m_destroyTimer;
	delete m_animationClock;
	delete m_animationTimer;
	//delete m_AIController;
	for (auto weapon : m_weapons) {
		delete weapon;
	}
}

void Actor::BeginPlay()
{
	if (m_def.m_AIEnabled) {
		m_AIController = nullptr;
	}

	if ((int)m_def.m_weaponNames.size() > 0) {
		for (auto weaponDef : m_def.m_weaponNames) {
			m_weapons.push_back( new Weapon( *weaponDef, m_uid ) );
		}
		m_curWeapon = m_weapons[0];
	}

}

void Actor::Update()
{
	if (m_isDead) {
		if (m_def.m_updatePositionEvenDie) {
			float deltaSeconds = g_theGame->m_gameClock->GetDeltaSeconds();
			m_position += m_velocity * deltaSeconds;
		}
		if (m_destroyTimer->HasPeriodElapsed()) {
			if (m_map->m_mapDef->m_gameMode == "Survival") {
				if (m_def.m_faction == ActorFaction::ENEMY) {
					ActorUID uid = g_theGame->m_curMap->SpawnActorToMap( ActorDefinition::GetActorDefinition( "Dog" ), m_position, m_orientation );
					AIController* thisAIController = g_theGame->CreateNewAIController( "MeleeAI" );
					thisAIController->Possess( uid );
				}
			}
			m_isGarbage = true;
		}
		return;
	}

	UpdatePhysics();

	if ((int)m_def.m_animGroup.size() > 0) {
		SpriteAnimGroupDefinition const& curGroupDef = m_def.GetGroupAnimDef( m_animationState );
		if (m_animationState != "Default" && curGroupDef.m_scaleBySpeed) {
			m_animationClock->SetTimeScale( m_velocity.GetLength() / m_def.m_walkSpeed );
		}
		else {
			m_animationClock->SetTimeScale( 1.f );
		}
		if (m_animationState != "Default" && curGroupDef.m_playbackType == SpriteAnimPlaybackType::ONCE && m_animationTimer->GetElapsedTime() > curGroupDef.m_totalTimeSeconds) {
			//m_animationTimer->Start();
			SetAnimationStateToDefault();
		}
	}

	if (m_curWeapon) {
		m_curWeapon->Update();
	}
}

void Actor::Render( Camera const* renderCamera, bool isShadow ) const
{
	if (m_isGarbage) {
		return;
	}
	if (m_controller && m_controller->IsPlayer() && &((PlayerController*)m_controller)->m_worldCamera == renderCamera && ((PlayerController*)m_controller)->m_cameraMode != PlayerControlMode::FREE_FLY){
		return;
	}
	if (!m_def.m_visible) {
		return;
	}	
	Vec2 viewDir = Vec2( m_position ) - Vec2( renderCamera->m_position );
	viewDir = GetModelMatrix().GetOrthonormalInverse().TransformVectorQuantity2D( viewDir ).GetNormalized();
	SpriteDefinition const* spriteDef;
	if (m_animationState == "Default") {
		spriteDef = &m_def.GetDefultAnimSprite( viewDir );
	}
	else {
		spriteDef = &m_def.GetAnimSprite( m_animationState, viewDir, m_animationTimer->GetElapsedTime() );
	}
	Vec3 pivotTranslation = Vec3( 0.f, -m_def.m_pivot.x * m_def.m_visualSize.x, -m_def.m_pivot.y * m_def.m_visualSize.y );

	if (m_def.m_blendAdditive) {
		g_theRenderer->SetBlendMode( BlendMode::ADDITIVE );
	}
	else {
		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	}

	if (m_def.m_renderLit) {
		std::vector<Vertex_PCUTBN> verts;
		verts.reserve( 48 );
		if (m_def.m_renderRounded) {
			AddVertsForQuad3D( verts, Vec3( 0.f, 0.f, 0.f ) + pivotTranslation,
				Vec3( 0.f, m_def.m_visualSize.x, 0.f ) + pivotTranslation,
				Vec3( 0.f, m_def.m_visualSize.x, m_def.m_visualSize.y ) + pivotTranslation,
				Vec3( 0.f, 0.f, m_def.m_visualSize.y ) + pivotTranslation,
				Rgba8::WHITE, spriteDef->GetUVs() );
		}
		else {
			AddVertsForQuad3D( verts, Vec3( 0.f, 0.f, 0.f ) + pivotTranslation,
				Vec3( 0.f, m_def.m_visualSize.x, 0.f ) + pivotTranslation,
				Vec3( 0.f, m_def.m_visualSize.x, m_def.m_visualSize.y ) + pivotTranslation,
				Vec3( 0.f, 0.f, m_def.m_visualSize.y ) + pivotTranslation,
				Rgba8::WHITE, spriteDef->GetUVs() );
		}
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
		g_theRenderer->SetDirectionalLightConstants( m_map->m_dirLightConsts );
		g_theRenderer->BindShader( g_theRenderer->CreateShader( m_def.m_shader.c_str(), VertexType::PCUTBN ) );
		g_theRenderer->BindTexture( &spriteDef->GetTexture() );
		//if (isShadow) {
		//	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
		//}
		//else {
			g_theRenderer->SetModelConstants( GetBillboardMatrix( m_def.m_billboardType, renderCamera->GetTransformMatrix(), m_position ), m_color );
		//}
		//Mat44 viewMatrix = Mat44::CreateTranslation3D( directionalLight.m_position );
		//viewMatrix.Append( directionalLight.m_orientation.GetAsMatrix_IFwd_JLeft_KUp() );
		//g_theRenderer->SetLightConstants( directionalLight.m_position, 0.8f, directionalLight.GetViewMatrix(), directionalLight.GetProjectionMatrix() );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		if (isShadow) {
			g_theRenderer->SetShadowMode( ShadowMode::POINT_SHADOW );
			g_theRenderer->RenderShadowMap( verts );
		}
		else {
			g_theRenderer->DrawVertexArray( verts );
		}
	}
	else if(!isShadow) {
		std::vector<Vertex_PCU> verts;
		verts.reserve( 24 );
		AddVertsForQuad3D( verts, Vec3( 0.f, 0.f, 0.f ) + pivotTranslation,
			Vec3( 0.f, m_def.m_visualSize.x, 0.f ) + pivotTranslation,
			Vec3( 0.f, m_def.m_visualSize.x, m_def.m_visualSize.y ) + pivotTranslation,
			Vec3( 0.f, 0.f, m_def.m_visualSize.y ) + pivotTranslation,
			Rgba8::WHITE, spriteDef->GetUVs() );

		g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->SetDepthMode( DepthMode::ENABLED );
		g_theRenderer->BindShader( g_theRenderer->CreateShader( m_def.m_shader.c_str(), VertexType::PCU ) );
		g_theRenderer->BindTexture( &spriteDef->GetTexture() );
		g_theRenderer->SetModelConstants( GetBillboardMatrix( m_def.m_billboardType, renderCamera->GetTransformMatrix(), m_position ), m_color );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->DrawVertexArray( verts );
	}
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );
}

Mat44 const Actor::GetModelMatrix() const
{
	Mat44 retMat = Mat44::CreateTranslation3D( m_position );
	retMat.Append( Mat44::CreateZRotationDegrees( m_orientation.m_yawDegrees ) );
	return retMat;
}


bool Actor::IsAlive() const
{
	return !m_isDead;
}

void Actor::UpdatePhysics()
{
	if (!m_def.m_simulated) {
		return;
	}
	float deltaSeconds = g_theGame->m_gameClock->GetDeltaSeconds();
	AddForce( -m_velocity );
	m_velocity += m_acceleration * deltaSeconds;
	m_position += m_velocity * deltaSeconds;
	if (!m_def.m_canFly) {
		m_position.z = 0.f;
	}
	m_acceleration = Vec3();
}

void Actor::AddForce( Vec3 const& force )
{
	m_acceleration += force * m_def.m_drag;
}

void Actor::AddImpulse( Vec3 const& impulse )
{
	m_velocity += impulse;
}

void Actor::OnPossessed()
{

}

void Actor::OnUnpossessed()
{
	if (m_controller && m_controller->IsPlayer() && m_AIController) {
		m_controller = m_AIController;
		m_controller->m_controlledActorUID = m_uid;
	}
	else {
		m_controller = nullptr;
	}
}

void Actor::MoveInDirection( Vec3 const& movement )
{
	m_position += movement;
}

void Actor::TurnInDirection( EulerAngles const& turnDegrees )
{
	m_orientation.m_yawDegrees += turnDegrees.m_yawDegrees;
	m_orientation.m_pitchDegrees += turnDegrees.m_pitchDegrees;
	m_orientation.m_rollDegrees += turnDegrees.m_rollDegrees;
}

void Actor::SetOrientation( EulerAngles const& orientation )
{
	m_orientation = orientation;
}

void Actor::SetYawDegrees( float yawDegrees )
{
	m_orientation.m_yawDegrees = yawDegrees;
}

void Actor::SetAnimationState( std::string const& newState )
{
	m_animationState = newState;
}

void Actor::SetAnimationStateToDefault()
{
	m_animationState = "Default";
}

void Actor::Attack()
{
	if (m_curWeapon) {
		m_curWeapon->Fire();
	}
}

void Actor::EquipWeapon( int index )
{
	if (index >= 0 && index < (int)m_weapons.size()) {
		m_curWeapon = m_weapons[index];
	}
}

void Actor::EquipNextWeapon()
{
	int index = -1;
	for (int i = 0; i < (int)m_weapons.size(); i++) {
		if (m_curWeapon == m_weapons[i]) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		return;
	}
	index = (index + 1) % ((int)m_weapons.size());
	m_curWeapon = m_weapons[index];
}

void Actor::EquipPrevWeapon()
{
	int index = -1;
	for (int i = 0; i < (int)m_weapons.size(); i++) {
		if (m_curWeapon == m_weapons[i]) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		return;
	}
	index = (index - 1 + (int)m_weapons.size()) % ((int)m_weapons.size());
	m_curWeapon = m_weapons[index];
}

void Actor::BeAttacked( float hit, bool& isLethal )
{
	if (m_health <= 0.f) {
		return;
	}
	SetAnimationState( "Hurt" );
	m_animationTimer->Start();
	auto iter = m_def.m_sounds.find( "Hurt" );
	if (iter != m_def.m_sounds.end()) {
		g_theGame->PlaySound3D( iter->second, m_position, m_uid, true, 0.01f, false, 1.f, 0.f, 1.f );
	}
	m_health -= hit;
	m_health = GetClamped( m_health, 0.f, m_maxHealth );
	if (m_health <= 0.f) {
		Die();
		isLethal = true;
	}
	else {
		isLethal = false;
	}
}

void Actor::Die()
{
	if (m_def.m_actorName == "SpawnPoint" || m_def.m_actorName == "EnemySpawnPoint") {
		return;
	}
	m_isDead = true;
	m_destroyTimer = new Timer( m_destroyTime, g_theGame->m_gameClock );
	m_destroyTimer->Start();
	SetAnimationState( "Death" );
	m_animationTimer->Start();
	auto iter = m_def.m_sounds.find( "Death" );

	if (iter != m_def.m_sounds.end()) {
		g_theGame->PlaySound3D( iter->second, m_position, m_uid, true, 0.01f, false, 1.f, 0.f, 1.f );
	}

	if (m_def.m_spawnExplosionParticle) {
		for (int i = 0; i < 100; i++) {
			m_map->SpawnActorToMap( ActorDefinition::GetActorDefinition(m_def.m_particleName), m_position, m_orientation, GetRandomDirection3D() * 0.5f );
		}
	}

	if (!m_def.m_dieOnSpawn) {
		m_color.r = m_color.r - 100 > 0 ? m_color.r - 100 : 0;
		m_color.g = m_color.g - 100 > 0 ? m_color.g - 100 : 0;
		m_color.b = m_color.b - 100 > 0 ? m_color.b - 100 : 0;
	}
}

Vec3 Actor::GetForwardNormal() const
{
	return m_orientation.GetIFwd();
}

ActorDefinition::ActorDefinition( XmlElement* xmlIter )
{
	m_actorName = ParseXmlAttribute( *xmlIter, "name", "Default" );
	std::string faction = ParseXmlAttribute( *xmlIter, "faction", "Default" );
	if (faction == "Marine") {
		m_faction = ActorFaction::ALLY;
	}
	else if (faction == "Demon") {
		m_faction = ActorFaction::ENEMY;
	}
	else if (faction == "Default") {
		m_faction = ActorFaction::PROJECTILE;
	}
	else {
		m_faction = ActorFaction::NEUTRAL;
	}
	m_visible = ParseXmlAttribute( *xmlIter, "visible", m_visible );
	m_maxHealth = ParseXmlAttribute( *xmlIter, "health", m_maxHealth );
	m_corpseLiveSeconds = ParseXmlAttribute( *xmlIter, "corpseLifetime", m_corpseLiveSeconds );
	m_canBePossessed = ParseXmlAttribute( *xmlIter, "canBePossessed", m_canBePossessed );
	m_dieOnSpawn = ParseXmlAttribute( *xmlIter, "dieOnSpawn", m_dieOnSpawn );
	m_tintColor = ParseXmlAttribute( *xmlIter, "tintColor", m_tintColor );
	m_spawnExplosionParticle = ParseXmlAttribute( *xmlIter, "spawnExplosionParticle", m_spawnExplosionParticle );
	m_particleName = ParseXmlAttribute( *xmlIter, "particleName", m_particleName );
	m_updatePositionEvenDie = ParseXmlAttribute( *xmlIter, "updatePositionEvenDie", m_updatePositionEvenDie );

	XmlElement* collisionElement = xmlIter->FirstChildElement( "Collision" );
	if (collisionElement) {
		m_physicsRadius = ParseXmlAttribute( *collisionElement, "radius", m_physicsRadius );
		m_physicsHeight = ParseXmlAttribute( *collisionElement, "height", m_physicsHeight );
		m_collidesWithWorld = ParseXmlAttribute( *collisionElement, "collidesWithWorld", m_collidesWithWorld );
		m_collidesWithActors = ParseXmlAttribute( *collisionElement, "collidesWithActors", m_collidesWithActors );
		m_dieOnCollide = ParseXmlAttribute( *collisionElement, "dieOnCollide", m_dieOnCollide );
		m_damageOnColiide = ParseXmlAttribute( *collisionElement, "damageOnCollide", m_damageOnColiide );
		m_impulseOnCollide = ParseXmlAttribute( *collisionElement, "impulseOnCollide", m_impulseOnCollide );
	}

	XmlElement* physicsElement = xmlIter->FirstChildElement( "Physics" );
	if (physicsElement) {
		m_simulated = ParseXmlAttribute( *physicsElement, "simulated", m_simulated );
		m_canFly = ParseXmlAttribute( *physicsElement, "flying", m_canFly );
		m_walkSpeed = ParseXmlAttribute( *physicsElement, "walkSpeed", m_walkSpeed );
		m_runSpeed = ParseXmlAttribute( *physicsElement, "runSpeed", m_runSpeed );
		m_drag = ParseXmlAttribute( *physicsElement, "drag", m_drag );
		m_turnSpeedDegrees = ParseXmlAttribute( *physicsElement, "turnSpeed", m_turnSpeedDegrees );
	}

	XmlElement* cameraElement = xmlIter->FirstChildElement( "Camera" );
	if( cameraElement ) {
		m_eyeHeight = ParseXmlAttribute( *cameraElement, "eyeHeight", m_eyeHeight );
		m_cameraFOVDegrees = ParseXmlAttribute( *cameraElement, "cameraFOV", m_cameraFOVDegrees );
	}

	XmlElement* visualElement = xmlIter->FirstChildElement( "Visuals" );
	if (visualElement) {
		m_visualSize = ParseXmlAttribute( *visualElement, "size", m_visualSize );
		m_pivot = ParseXmlAttribute( *visualElement, "pivot", m_pivot );
		std::string billboardTypeStr = ParseXmlAttribute( *visualElement, "billboardType", "Default" );
		if (billboardTypeStr == "FullFacing") {
			m_billboardType = BillboardType::FULL_CAMERA_FACING;
		}
		else if (billboardTypeStr == "WorldUpFacing") {
			m_billboardType = BillboardType::WORLD_UP_CAMERA_FACING;
		}
		else if (billboardTypeStr == "WorldUpOpposing") {
			m_billboardType = BillboardType::WORLD_UP_CAMERA_OPPOSING;
		}
		else if (billboardTypeStr == "FullOpposing") {
			m_billboardType = BillboardType::FULL_CAMERA_OPPOSING;
		}
		else {
			m_billboardType = BillboardType::NONE;
		}
		m_renderLit = ParseXmlAttribute( *visualElement, "renderLit", m_renderLit );
		m_renderRounded = ParseXmlAttribute( *visualElement, "renderRounded", m_renderRounded );
		m_shader = ParseXmlAttribute( *visualElement, "shader", m_shader );
		m_spriteSheet = ParseXmlAttribute( *visualElement, "spriteSheet", m_spriteSheet );
		m_cellCount = ParseXmlAttribute( *visualElement, "cellCount", m_cellCount );
		m_animSprite = new SpriteSheet( *g_theRenderer->CreateOrGetTextureFromFile( m_spriteSheet.c_str() ), m_cellCount );
		m_blendAdditive = ParseXmlAttribute( *visualElement, "blendAdditive", m_blendAdditive );
		XmlElement* iter = visualElement->FirstChildElement();
		while (iter != nullptr) {
			m_animGroup.push_back( SpriteAnimGroupDefinition( iter, *m_animSprite ) );
			iter = iter->NextSiblingElement();
		}
	}

	XmlElement* soundElement = xmlIter->FirstChildElement( "Sounds" );
	if (soundElement) {
		XmlElement* iter = soundElement->FirstChildElement();
		while (iter != nullptr) {
			std::string soundName = ParseXmlAttribute( *iter, "sound", "Default" );
			std::string path = ParseXmlAttribute( *iter, "name", "Default" );
			m_sounds[soundName] = path;
			iter = iter->NextSiblingElement();
		}
	}

	XmlElement* AIElement = xmlIter->FirstChildElement( "AI" );
	if (AIElement) {
		m_AIEnabled = ParseXmlAttribute( *AIElement, "aiEnabled", m_AIEnabled );
		m_sightRadius = ParseXmlAttribute( *AIElement, "sightRadius", m_sightRadius );
		m_sightAngle = ParseXmlAttribute( *AIElement, "sightAngle", m_sightAngle );
		m_aiBehavior = ParseXmlAttribute( *AIElement, "aiBehavior", m_aiBehavior );
	}

	XmlElement* inventoryElement = xmlIter->FirstChildElement( "Inventory" );
	if (inventoryElement) {
		XmlElement* weaponIter = inventoryElement->FirstChildElement();
		while (weaponIter != nullptr) {
			std::string name = ParseXmlAttribute( *weaponIter, "name", "Default" );
			m_weaponNames.push_back( &WeaponDefinition::GetWeaponDefinition( name ) );
			weaponIter = weaponIter->NextSiblingElement();
		}
	}
}

ActorDefinition::ActorDefinition()
{

}

ActorDefinition const& ActorDefinition::GetActorDefinition( std::string const& name )
{
	for (auto& actorDef : s_definitions) {
		if (actorDef.m_actorName == name) {
			return actorDef;
		}
	}
	ERROR_AND_DIE( Stringf( "Cannot find actor definition named %s!", name.c_str() ) );
}

void ActorDefinition::SetUpActorDefinitions()
{
	ActorDefinition::s_definitions.reserve( 64 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Definitions/ActorDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document ActorDefinitions.xml error" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "Definitions" ), "Syntax Error! Name of the root of ActorDefinitions.xml should be \"Definitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "ActorDefinition" ), "Syntax Error! Names of the elements of ActorDefinitions.xml should be \"ActorDefinition\" " );
		ActorDefinition::s_definitions.emplace_back( xmlIter );
		xmlIter = xmlIter->NextSiblingElement();
	}
	
	errorCode = xmlDocument.LoadFile( "Data/Definitions/ProjectileActorDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document ProjectileActorDefinitions.xml error" );
	root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "Definitions" ), "Syntax Error! Name of the root of ProjectileActorDefinitions.xml should be \"Definitions\" " );
	xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "ActorDefinition" ), "Syntax Error! Names of the elements of ProjectileActorDefinitions.xml should be \"ActorDefinition\" " );
		ActorDefinition::s_definitions.emplace_back( xmlIter );
		xmlIter = xmlIter->NextSiblingElement();
	}
}

ActorDefinition::~ActorDefinition()
{
}

SpriteAnimGroupDefinition const& ActorDefinition::GetGroupAnimDef( std::string name ) const
{
	for (auto& def : m_animGroup) {
		if (name == def.m_name) {
			return def;
		}
	}
	return m_animGroup[0];
}

SpriteDefinition const& ActorDefinition::GetAnimSprite( std::string const& curAnimState, Vec3 const viewDirNormal, float seconds ) const
{
	for (auto& def : m_animGroup) {
		if (curAnimState == def.m_name) {
			return def.GetSpriteAnimDefByDirectionAndTime( viewDirNormal, seconds );
		}
	}
	ERROR_RECOVERABLE( Stringf( "Error! Cannot find an animation called %s", curAnimState.c_str() ) );
	return GetDefultAnimSprite( viewDirNormal );
}

SpriteDefinition const& ActorDefinition::GetDefultAnimSprite( Vec3 const viewDirNormal ) const
{
	return m_animGroup[0].GetSpriteAnimDefByDirectionAndTime( viewDirNormal, 0.f );
}
