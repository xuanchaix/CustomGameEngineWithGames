#include "Game/Weapon.hpp"
#include "Game/Game.hpp"
#include "Game/Controller.hpp"
#include "Game/App.hpp"

std::vector<WeaponDefinition> WeaponDefinition::s_definitions;

WeaponDefinition::WeaponDefinition( XmlElement* xmlIter )
{
	m_name = ParseXmlAttribute( *xmlIter, "name", m_name );
	m_refireTime = ParseXmlAttribute( *xmlIter, "refireTime", m_refireTime );
	m_rayCount = ParseXmlAttribute( *xmlIter, "rayCount", m_rayCount );
	m_rayCone = ParseXmlAttribute( *xmlIter, "rayCone", m_rayCone );
	m_rayRange = ParseXmlAttribute( *xmlIter, "rayRange", m_rayRange );
	m_rayDamage = ParseXmlAttribute( *xmlIter, "rayDamage", m_rayDamage );
	m_rayImpulse = ParseXmlAttribute( *xmlIter, "rayImpulse", m_rayImpulse );
	m_rayHitWallEffectActor = ParseXmlAttribute( *xmlIter, "rayHitWallEffectActor", m_rayHitWallEffectActor );
	m_rayHitActorEffectActor = ParseXmlAttribute( *xmlIter, "rayHitActorEffectActor", m_rayHitActorEffectActor );
	m_projectileCount = ParseXmlAttribute( *xmlIter, "projectileCount", m_projectileCount );
	m_projectileCone = ParseXmlAttribute( *xmlIter, "projectileCone", m_projectileCone );
	m_projectileSpeed = ParseXmlAttribute( *xmlIter, "projectileSpeed", m_projectileSpeed );
	m_projectileActor = ParseXmlAttribute( *xmlIter, "projectileActor", m_projectileActor );
	m_meleeCount = ParseXmlAttribute( *xmlIter, "meleeCount", m_meleeCount );
	m_meleeDamage = ParseXmlAttribute( *xmlIter, "meleeDamage", m_meleeDamage );
	m_meleeArc = ParseXmlAttribute( *xmlIter, "meleeArc", m_meleeArc );
	m_meleeRange = ParseXmlAttribute( *xmlIter, "meleeRange", m_meleeRange );
	m_meleeImpulse = ParseXmlAttribute( *xmlIter, "meleeImpulse", m_meleeImpulse );
	m_rangeDamage = ParseXmlAttribute( *xmlIter, "rangeDamage", m_rangeDamage );
	m_rangeRadius = ParseXmlAttribute( *xmlIter, "rangeRadius", m_rangeRadius );
	m_hasRangeDamage = ParseXmlAttribute( *xmlIter, "hasRangeDamage", m_hasRangeDamage );
	/*
	<Sounds>
	  <Sound sound="Fire" name="Data/Audio/PlasmaFire.wav"/>
	</Sounds>
	*/
	XmlElement* HUDElement = xmlIter->FirstChildElement( "HUD" );
	if (HUDElement) {
		std::string HUDShaderStr = ParseXmlAttribute( *HUDElement, "shader", "Default" );
		m_HUDshader = g_theRenderer->CreateShader( HUDShaderStr.c_str(), VertexType::PCU );
		std::string baseTextureStr = ParseXmlAttribute( *HUDElement, "baseTexture", "Default" );
		m_baseTexture = g_theRenderer->CreateOrGetTextureFromFile( baseTextureStr.c_str() );
		std::string reticleTextureStr = ParseXmlAttribute( *HUDElement, "reticleTexture", "Default" );
		m_reticleTexture = g_theRenderer->CreateOrGetTextureFromFile( reticleTextureStr.c_str() );
		m_reticleSize = ParseXmlAttribute( *HUDElement, "reticleSize", m_reticleSize );
		m_spriteSize = ParseXmlAttribute( *HUDElement, "spriteSize", m_spriteSize );
		m_spritePivot = ParseXmlAttribute( *HUDElement, "spritePivot", m_spritePivot );
		XmlElement* animElement = HUDElement->FirstChildElement();
		while (animElement != nullptr) {
			std::string name = ParseXmlAttribute( *animElement, "name", "Default" );
			std::string shaderName = ParseXmlAttribute( *animElement, "shader", "Default" );
			std::string textureName = ParseXmlAttribute( *animElement, "spriteSheet", "Default" );
			IntVec2 cellCount = ParseXmlAttribute( *animElement, "cellCount", IntVec2( 1, 1 ) );
			int startIndex = ParseXmlAttribute( *animElement, "startFrame", 0 );
			int endIndex = ParseXmlAttribute( *animElement, "endFrame", 0 );
			float secondsPerFrame = ParseXmlAttribute( *animElement, "secondsPerFrame", 0.f );
			float totalSeconds = (float)(endIndex - startIndex + 1) * secondsPerFrame;

			Texture* texture = g_theRenderer->CreateOrGetTextureFromFile( textureName.c_str() );
			m_animationSpriteSheet = new SpriteSheet( *texture, cellCount );
			m_animations.insert( std::pair<std::string, SpriteAnimDefinition>( name, SpriteAnimDefinition( *m_animationSpriteSheet, startIndex, endIndex, totalSeconds, SpriteAnimPlaybackType::ONCE ) ) );
			animElement = animElement->NextSiblingElement();
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
}

WeaponDefinition::WeaponDefinition()
{

}

WeaponDefinition const& WeaponDefinition::GetWeaponDefinition( std::string const& name )
{
	for (auto const& weaponDef : WeaponDefinition::s_definitions) {
		if (weaponDef.m_name == name) {
			return weaponDef;
		}
	}
	ERROR_AND_DIE( Stringf( "Cannot find weapon definition named %s!", name.c_str() ) );
}

void WeaponDefinition::SetUpWeaponDefinitions()
{
	WeaponDefinition::s_definitions.reserve( 64 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Definitions/WeaponDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document WeaponDefinitions.xml error" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "Definitions" ), "Syntax Error! Name of the root of WeaponDefinitions.xml should be \"Definitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "WeaponDefinition" ), "Syntax Error! Names of the elements of WeaponDefinitions.xml should be \"WeaponDefinition\" " );
		WeaponDefinition::s_definitions.emplace_back( xmlIter );
		xmlIter = xmlIter->NextSiblingElement();
	}
}

Weapon::Weapon( WeaponDefinition const& def, ActorUID owner )
	:m_owner(owner)
	,m_def(def)
{
	m_shootCoolDownTimer = new Timer( def.m_refireTime, g_theGame->m_gameClock );
	m_shootCoolDownTimer->Start();
	m_animationTimer = new Timer( 100.f, g_theGame->m_gameClock );
	m_animationTimer->Start();
}

void Weapon::Fire()
{
	if (m_shootCoolDownTimer->DecrementPeriodIfElapsed()) {
		Vec3 const& actorPosition = m_owner->m_position + Vec3( 0.f, 0.f, m_owner->m_def.m_eyeHeight );
		EulerAngles const& actorOrientation = m_owner->m_orientation;
		ActorFaction targetFaction = ActorFaction::NEUTRAL;
		if (m_owner->m_def.m_faction == ActorFaction::ALLY) {
			targetFaction = ActorFaction::ENEMY;
		}
		else if (m_owner->m_def.m_faction == ActorFaction::ENEMY) {
			targetFaction = ActorFaction::ALLY;
		}
		// can shoot
		// ray shoot
		for (int i = 0; i < m_def.m_rayCount; i++) {
			Vec3 dir = GetRandomDirectionInCone( actorOrientation, m_def.m_rayCone );
			RayCastResultDoomenstein rayRes = g_theGame->m_curMap->RayCastAll( actorPosition, dir, m_def.m_rayRange, m_owner->m_map->GetActorByUID( m_owner ), ActorFaction::PROJECTILE );
#ifdef DEBUG_SHOW_HIT
			DebugAddWorldLine( actorPosition, actorPosition + dir * 10.f, 0.01f, 5.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY );
#endif // DEBUG_SHOW_HIT
			if (rayRes.m_didImpact) {
#ifdef DEBUG_SHOW_HIT
				DebugAddWorldWireSphere( rayRes.m_impactPos, 0.06f, 5.f, Rgba8::WHITE, Rgba8::WHITE );
				DebugAddWorldArrow( rayRes.m_impactPos, rayRes.m_impactPos + rayRes.m_impactNormal * 0.1f, 0.01f, 5.f, Rgba8( 0, 0, 255 ), Rgba8( 0, 0, 255 ) );
#endif // DEBUG_SHOW_HIT
				if (rayRes.m_didHitActor) {
					bool isLethal = false;
					rayRes.m_uid->BeAttacked( g_theGame->m_randNumGen->RollRandomFloatInRange( m_def.m_rayDamage ), isLethal );
					if (rayRes.m_uid->m_controller) {
						rayRes.m_uid->m_controller->Damagedby( m_owner, isLethal );
					}
					rayRes.m_uid->AddImpulse( m_def.m_rayImpulse * dir );
					m_owner->m_map->SpawnActorToMap( ActorDefinition::GetActorDefinition( m_def.m_rayHitActorEffectActor ), rayRes.m_impactPos );
				}
				else {
					m_owner->m_map->SpawnActorToMap( ActorDefinition::GetActorDefinition( m_def.m_rayHitWallEffectActor ), rayRes.m_impactPos );
				}
				if (m_def.m_hasRangeDamage) {
					g_theGame->m_curMap->DealRangeDamage( rayRes.m_impactPos, m_def.m_rangeRadius, m_def.m_rangeDamage, m_owner );
				}
			}
		}
		// projectile
		for (int i = 0; i < m_def.m_projectileCount; i++) {
			float newYaw = actorOrientation.m_yawDegrees + g_theGame->m_randNumGen->RollRandomFloatInRange( -m_def.m_projectileCone, m_def.m_projectileCone );
			float newPitch = actorOrientation.m_pitchDegrees + g_theGame->m_randNumGen->RollRandomFloatInRange( -m_def.m_projectileCone, m_def.m_projectileCone );
			EulerAngles newOrientation = EulerAngles( newYaw, newPitch, actorOrientation.m_rollDegrees );
			ActorDefinition const& projDef = ActorDefinition::GetActorDefinition( m_def.m_projectileActor );
			ActorUID projUID = m_owner->m_map->SpawnActorToMap( projDef, actorPosition - Vec3( 0.f, 0.f, projDef.m_physicsHeight * 0.5f ), newOrientation, newOrientation.GetIFwd() * m_def.m_projectileSpeed );
			projUID->m_owner = m_owner;
		}
		// melee attack
		for (int i = 0; i < m_def.m_meleeCount; i++) {
			Actor* targetActor = m_owner->m_map->GetNearestEnemyInSector( targetFaction, m_owner->m_position, m_owner->GetForwardNormal(), m_def.m_meleeArc, m_def.m_meleeRange );
			if (targetActor) {
				bool isLethal = false;
				targetActor->BeAttacked( g_theGame->m_randNumGen->RollRandomFloatInRange( m_def.m_meleeDamage ), isLethal );
				if (targetActor->m_controller) {
					targetActor->m_controller->Damagedby( m_owner, isLethal );
				}
				targetActor->AddImpulse( m_def.m_meleeImpulse * (targetActor->m_position - m_owner->m_position).GetNormalized() );
			}
		}
		m_shootCoolDownTimer->Start();
		m_owner->SetAnimationState( "Attack" );
		m_owner->m_animationTimer->Start();
		m_curAnimationState = "Attack";
		m_animationTimer->Start();
		auto iter = m_def.m_sounds.find( "Fire" );
		if (iter != m_def.m_sounds.end()) {
			g_theGame->PlaySound3D( iter->second, m_owner->m_position, m_owner->m_uid, true, 0.01f, false, 1.f, 0.f, 1.f );
		}
	}
}

void Weapon::Update()
{
	auto iter = m_def.m_animations.find( m_curAnimationState );
	if (iter != m_def.m_animations.end()) {
		SpriteAnimDefinition const& anim = iter->second;
		if (m_curAnimationState != "Idle" && anim.IsCompleted(m_animationTimer->GetElapsedTime())) {
			m_curAnimationState = "Idle";
		}
	}
	//ERROR_AND_DIE( Stringf( "No such Weapon Animation name: %s", m_curAnimationState.c_str() ) );
}

SpriteDefinition const& Weapon::GetCurrentSpriteDef() const
{
	auto iter = m_def.m_animations.find( m_curAnimationState );
	if (iter != m_def.m_animations.end()) {
		SpriteAnimDefinition const& anim = iter->second;
		return anim.GetSpriteDefAtTime( m_animationTimer->GetElapsedTime() );
	}
	ERROR_AND_DIE( Stringf( "No such Weapon Animition name: %s", m_curAnimationState.c_str() ) );
}

Vec3 const Weapon::GetRandomDirectionInCone( EulerAngles const& orientation, float maxDegrees )
{
	float newYawDegrees = orientation.m_yawDegrees + g_theGame->m_randNumGen->RollRandomFloatInRange( -maxDegrees, maxDegrees );
	float newPitchDegrees = orientation.m_pitchDegrees + g_theGame->m_randNumGen->RollRandomFloatInRange( -maxDegrees, maxDegrees );
	return Vec3::MakeFromPolarDegrees( newPitchDegrees, newYawDegrees );
}
