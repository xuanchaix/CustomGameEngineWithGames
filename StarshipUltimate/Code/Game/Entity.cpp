#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Weapon.hpp"
#include "Game/Effects.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"

static void* PoisonDamageSource = (void*)2;

//constexpr int ATTACK_EFFECT_POISONOUS = 0b1;
//constexpr int ATTACK_EFFECT_SLOW = 0b10;

std::vector<EntityDefinition> EntityDefinition::s_definitions;
std::map<std::string, std::vector<std::vector<EntityDefinition*>>> EntityDefinition::s_factionLevelMap;

Entity::Entity( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:m_position( startPos )
	,m_def(def)
	,m_orientationDegrees(startOrientation)
	,m_velocity(startVelocity)
{
	m_angularVelocity = 0.f;
	m_cosmeticRadius = m_def.m_cosmeticRadius;
	m_physicsRadius = m_def.m_physicsRadius;
	m_health = m_def.m_maxHealth;
	m_color = Rgba8( 255, 255, 255 );
	m_maxHealth = m_def.m_maxHealth;
	m_clock = new Clock( *GetGameClock() );

	m_shadowLifeTimer = new Timer( 8.f, m_clock );
	m_shadowLifeTimer->Start();

	if (m_def.m_weaponType != "None" && m_def.m_weaponType != "NONE") {
		m_mainWeaponTimer = new Timer( def.m_shootCoolDown, GetGameClock() );
		m_mainWeaponTimer->Start();
		m_mainWeapon = g_theGame->CreateWeaponComponent( WeaponDefinition::GetDefinition( m_def.m_weaponType ), this );
	}
}

Entity::~Entity() {
	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;
	delete m_mainWeapon;
	m_mainWeapon = nullptr;
	delete m_mainWeaponTimer;
	m_mainWeaponTimer = nullptr;
	delete m_clock;
	m_clock = nullptr;
}

void Entity::BeginPlay()
{
	// do nothing now
}

void Entity::Update( float deltaTime )
{
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}

	if (m_isShadowed && m_shadowLifeTimer->HasPeriodElapsed()) {
		m_isDead = true;
		m_color.a = 100;
		ParticleSystem2DAddEmitter( 1000, 0.05f,
			AABB2( m_position - Vec2( m_cosmeticRadius * 5.f, m_cosmeticRadius * 5.f ), m_position + Vec2( m_cosmeticRadius * 5.f, m_cosmeticRadius * 5.f ) ),
			FloatRange( m_cosmeticRadius * 0.4f, m_cosmeticRadius * 0.8f ),
			AABB2( Vec2( m_cosmeticRadius * 5.f, m_cosmeticRadius * 5.f ), Vec2( m_cosmeticRadius * 5.f, m_cosmeticRadius * 5.f ) ),
			FloatRange( 0.3f, 0.6f ), Rgba8( 160, 160, 160, 255 ), Particle2DShape::Asteroid, true, FloatRange( 0.f, 0.f ),
			FloatRange( 0.f, 0.f ), nullptr, Rgba8( 160, 160, 160, 105 ), 0.f, 0.f );
	}

	UpdatePhysics( deltaTime );

	if (m_isPoisoned && !m_immuneToPoison) {
		m_color = Rgba8( 76, 153, 0 );
		m_poisonTimer -= deltaTime;
		if (m_poisonTimer <= 0.f) {
			m_isPoisoned = false;
			m_color = Rgba8::WHITE;
		}
		BeAttackedOnce( 1.f, Vec2(), 1.f, PoisonDamageSource );
	}

	if (m_isSlowed && !m_immuneToSlow) {
		m_slowTimer -= deltaTime;
		if (m_slowTimer <= 0.f) {
			m_isSlowed = false;
			if (m_clock->GetTimeScale() != 0.f) {
				m_clock->SetTimeScale( 1.f );
			}
		}
		if (m_clock->GetTimeScale() != 0.f) {
			m_clock->SetTimeScale( 0.8f );
		}
	}

	if (m_health <= 0.f) {
		if (m_def.m_isEnemy) {
			g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/EnemyDie.wav" ), false, g_theApp->m_soundVolume * 0.1f );
		}
		Die();
	}
}

void Entity::DebugRender() const
{
	DebugDrawLine( m_position, m_orientationDegrees, m_cosmeticRadius, 0.2f, Rgba8( 255, 0, 0, 255 ) );
	DebugDrawLine( m_position, m_orientationDegrees + 90.f, m_cosmeticRadius, 0.2f, Rgba8( 0, 255, 0, 255 ) );
	DebugDrawRing( m_position, m_cosmeticRadius, 0.2f, Rgba8( 255, 0, 255, 255 ) );
	DebugDrawRing( m_position, m_physicsRadius, 0.2f, Rgba8( 0, 255, 255, 255 ) );
	DebugDrawLine( m_position, Atan2Degrees( m_velocity.y, m_velocity.x ), m_velocity.GetLength(), 0.2f, Rgba8( 255, 255, 0, 255 ) );
}


void Entity::RenderUI() const
{
}

Vec2 Entity::GetForwardNormal() const
{
	return Vec2::MakeFromPolarDegrees( m_orientationDegrees );
}

bool Entity::IsAlive() const
{
	return !m_isDead;
}

void Entity::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage, Vec2 const& projectileVelocity )
{
	if (m_isDead) {
		return;
	}
	UNUSED( directDamage );
	UNUSED( projectileVelocity );
	if (hit > 0.f && m_def.m_isShielded && hitNormal != Vec2(0.f, 0.f) && DotProduct2D(hitNormal, GetForwardNormal()) > 0.1f) {
		StarshipEffect* shieldEffect = g_theGame->SpawnEffectToGame( EffectType::Shield, m_position );
		((StarshipShield*)shieldEffect)->m_owner = this;
		shieldEffect->BeginPlay();
		return;
	}
	else {
		g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/EnemyHit.wav" ), false, g_theApp->m_soundVolume * 0.1f );
		m_health -= hit;
	}
	if (m_health > m_maxHealth) {
		m_health = m_maxHealth;
	}
	if (m_health <= 0.f) {
		if (m_def.m_isEnemy) {
			g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/EnemyDie.wav" ), false, g_theApp->m_soundVolume * 0.1f );
		}
		Die();
	}
}

void Entity::BeAttackedOnce( float hit, Vec2 const& hitNormal, float coolDownSeconds, void* damageSource, bool directDamage, Vec2 const& projectileVelocity )
{
	auto iter = m_damageSourceMap.find( damageSource );
	if (iter != m_damageSourceMap.end()) {
		if (iter->second + coolDownSeconds > GetGameClock()->GetTotalSeconds()) {
			return;
		}
		// can deal damage
		iter->second += coolDownSeconds;
	}
	else {
		// can deal damage
		m_damageSourceMap[damageSource] = GetGameClock()->GetTotalSeconds();
	}

	BeAttacked( hit, hitNormal, directDamage, projectileVelocity );
}

void Entity::AddForce( Vec2 const& force, bool isAffectedByMass /*= true */ )
{
	if (isAffectedByMass) {
		m_accelerateVelocity += force / m_mass;
	}
	else {
		m_accelerateVelocity += force;
	}
}

void Entity::AddImpulse( Vec2 const& impulse, bool isAffectedByMass /*= false */ )
{
	if (isAffectedByMass) {
		m_velocity += impulse / m_mass;
	}
	else {
		m_velocity += impulse;
	}
}

void Entity::SetOrientationDegrees( float newOrientation )
{
	m_orientationDegrees = newOrientation;
}

bool Entity::Fire( Vec2 const& forwardVec, Vec2 const& startPos, bool forceToFire )
{
	if (m_mainWeaponTimer->DecrementPeriodIfElapsed() || forceToFire) {
		m_mainWeapon->Fire( forwardVec, startPos );
		m_mainWeaponTimer->Start();
		return true;
	}
	return false;
}

float Entity::GetMainWeaponDamage() const
{
	return m_def.m_weaponDamage;
}

bool Entity::IsInvincible()
{
	return m_isInvincible;
}

void Entity::UpdatePhysics( float deltaSeconds )
{
	if (!m_disableFriction) {
		AddForce( -m_velocity * m_mass * 9.8f );
	}
	m_velocity += m_accelerateVelocity * deltaSeconds;
	m_position += m_velocity * deltaSeconds;
	m_accelerateVelocity = Vec2( 0.f, 0.f );
	m_orientationDegrees += m_angularVelocity * deltaSeconds;
}

Mat44 Entity::GetModelMatrix() const
{
	Mat44 translationMat = Mat44::CreateTranslation2D( m_position );
	translationMat.Append( Mat44::CreateZRotationDegrees( m_orientationDegrees ) );
	return translationMat;
}


void Entity::SpawnReward() const
{
	for (int i = 0; i < m_def.m_killReward; i++) {
		g_theGame->SpawnEffectToGame( EffectType::Reward, m_position );
	}
}

EntityDefinition::EntityDefinition( XmlElement* xmlIter )
{
	XmlElement* basicElement = xmlIter->FirstChildElement( "Basic" );
	if (basicElement) {
		m_name = ParseXmlAttribute( *basicElement, "name", "Default" );
		m_faction = ParseXmlAttribute( *basicElement, "faction", "Default" );
		m_physicsRadius = ParseXmlAttribute( *basicElement, "physicsRadius", m_physicsRadius );
		m_cosmeticRadius = ParseXmlAttribute( *basicElement, "cosmeticRadius", m_cosmeticRadius );
		m_turnSpeed = ParseXmlAttribute( *basicElement, "turnSpeed", m_turnSpeed );
		m_flySpeed = ParseXmlAttribute( *basicElement, "flySpeed", m_flySpeed );
		m_maxHealth = ParseXmlAttribute( *basicElement, "maxHealth", m_maxHealth );
		m_killReward = ParseXmlAttribute( *basicElement, "killReward", m_killReward );
		m_dealDamageOnCollide = ParseXmlAttribute( *basicElement, "dealDamageOnCollide", m_dealDamageOnCollide );
		m_isReflector = ParseXmlAttribute( *basicElement, "isReflector", m_isReflector );
		m_isEnemy = ParseXmlAttribute( *basicElement, "isEnemy", m_isEnemy );
		m_enemyLevel = ParseXmlAttribute( *basicElement, "enemyLevel", m_enemyLevel );
		m_enableCollision = ParseXmlAttribute( *basicElement, "enableCollision", m_enableCollision );
		m_deathParticleColor = ParseXmlAttribute( *basicElement, "deathParticleColor", m_deathParticleColor );
	}


	XmlElement* AIElement = xmlIter->FirstChildElement( "AI" );
	if (AIElement) {
		m_isAIEnabled = ParseXmlAttribute( *AIElement, "isEnabled", m_isAIEnabled );
		m_aiBehavior = ParseXmlAttribute( *AIElement, "aiBehavior", m_aiBehavior );
		m_isShielded = ParseXmlAttribute( *AIElement, "isShielded", m_isShielded );
	}

	XmlElement* renderElement = xmlIter->FirstChildElement( "Render" );
	if (renderElement) {
		m_texturePath = ParseXmlAttribute( *renderElement, "texturePath", m_texturePath );
		//m_aiBehavior = ParseXmlAttribute( *renderElement, "aiBehavior", m_aiBehavior );
		//m_isShielded = ParseXmlAttribute( *renderElement, "isShielded", m_isShielded );
	}

	XmlElement* weaponElement = xmlIter->FirstChildElement( "Weapon" );
	if (weaponElement) {
		m_weaponType = ParseXmlAttribute( *weaponElement, "weaponType", m_weaponType );
		m_shootCoolDown = ParseXmlAttribute( *weaponElement, "shootCoolDown", m_shootCoolDown );
		m_weaponDamage = ParseXmlAttribute( *weaponElement, "weaponDamage", m_weaponDamage );
	}
}

EntityDefinition::EntityDefinition()
{

}

void EntityDefinition::SetUpEntityDefinitions()
{
	EntityDefinition::s_definitions.reserve( 64 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Definitions/EntityDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document EntityDefinitions.xml error" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "EntityDefinitions" ), "Syntax Error! Name of the root of EntityDefinitions.xml should be \"EntityDefinitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "EntityDefinition" ), "Syntax Error! Names of the elements of EntityDefinitions.xml should be \"EntityDefinition\" " );
		EntityDefinition::s_definitions.emplace_back( xmlIter );
		xmlIter = xmlIter->NextSiblingElement();
	}

	for (auto& def : s_definitions) {
		auto iter = s_factionLevelMap.find( def.m_faction );
		if (iter != s_factionLevelMap.end()) {
			iter->second[def.m_enemyLevel].push_back( &def );
		}
		else {
			s_factionLevelMap[def.m_faction] = std::vector<std::vector<EntityDefinition*>>( 6 );
			s_factionLevelMap[def.m_faction][def.m_enemyLevel].push_back( &def );
		}
	}
}

EntityDefinition const& EntityDefinition::GetDefinition( std::string const& name )
{
	for (auto const& def : s_definitions) {
		if (def.m_name == name) {
			return def;
		}
	}
	ERROR_AND_DIE( Stringf( "No Such Entity Definition %s", name.c_str() ) );
}

EntityDefinition const& EntityDefinition::GetRandomDefinition( std::string const& faction, int level /*= -1 */ )
{
	auto iter = s_factionLevelMap.find( faction );
	if (iter != s_factionLevelMap.end()) {
		if (level != -1) {
			level = GetClamped( level, 0, 5 );
			if ((int)iter->second[level].size() > 0) {
				int rnd = g_theGame->m_randNumGen->RollRandomIntLessThan( (int)iter->second[level].size() );
				return *(iter->second[level][rnd]);
			}
			else {
				ERROR_AND_DIE( Stringf( "No level %d of enemy in faction %s!", level, faction.c_str() ) );
			}
		}
		else {
			int count = 0;
			int rndLevel = 0;
			do {
				if (count > 30) {
					ERROR_AND_DIE( Stringf( "Cannot find a random enemy in faction %s! Possible infinite loop!", faction.c_str() ) );
				}
				rndLevel = g_theGame->m_randNumGen->RollRandomIntLessThan( 4 );
				count++;
			} while ((int)iter->second[rndLevel].size() == 0);
			int rnd = g_theGame->m_randNumGen->RollRandomIntLessThan( (int)iter->second[rndLevel].size() );
			return *(iter->second[rndLevel][rnd]);
		}
	}
	else {
		ERROR_AND_DIE( Stringf( "No faction %s!", faction.c_str()) );
	}
}
