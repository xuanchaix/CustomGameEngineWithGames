#include "Game/Weapon.hpp"
#include "Game/Game.hpp"
#include "Game/Controller.hpp"
#include "Game/DiamondFraction.hpp"

std::vector<WeaponDefinition> WeaponDefinition::s_definitions;

Weapon::Weapon( WeaponDefinition const& def, Entity* owner )
	:m_weaponDef(def)
	,m_owner(owner)
{
}

Weapon::~Weapon()
{
}

ProjectileDefinition const& Weapon::GetProjectileDef() const
{
	return *m_weaponDef.m_projectileDef;
}

BulletGun::BulletGun( WeaponDefinition const& def, Entity* owner )
	:Weapon( def, owner )
{

}

BulletGun::~BulletGun()
{

}

Projectile* BulletGun::Fire( Vec2 const& forwardVec, Vec2 const& startPos )
{
	//float actualSpeed = DotProduct2D( m_owner->m_velocity, normal ) + m_projectileDef.m_speed;
	Projectile* projectile = g_theGame->SpawnProjectileToGame( GetProjectileDef(), startPos, forwardVec.GetOrientationDegrees(), forwardVec * GetProjectileDef().m_speed );
	projectile->m_faction = m_owner->m_def.m_faction;
	projectile->m_damage = m_owner->GetMainWeaponDamage();
	return projectile;
}

RayShooter::RayShooter( WeaponDefinition const& def, Entity* owner )
	:Weapon( def, owner )
{

}

RayShooter::~RayShooter()
{

}

Projectile* RayShooter::Fire( Vec2 const& forwardVec, Vec2 const& startPos )
{
	Vec2 endPos;
	StarshipRayCastResult res1 = g_theGame->RayCastVsEntities( startPos + forwardVec.GetRotated90Degrees() * m_owner->m_physicsRadius * 0.5f, forwardVec, 50.f, m_owner, "Player", m_weaponDef.m_canTriggerReflection );
	StarshipRayCastResult res2 = g_theGame->RayCastVsEntities( startPos + forwardVec.GetRotatedMinus90Degrees() * m_owner->m_physicsRadius * 0.5f, forwardVec, 50.f, m_owner, "Player", m_weaponDef.m_canTriggerReflection );
	if (res1.m_didImpact && res2.m_didImpact) {
		if (res1.m_impactDist < res2.m_impactDist) {
			if (res1.m_entityHit->m_controller && res1.m_entityHit->m_controller->IsPlayer()) {
				res1.m_entityHit->BeAttacked( m_owner->GetMainWeaponDamage(), res1.m_impactNormal );
			}
			else if (m_weaponDef.m_canTriggerReflection && res1.m_entityHit->m_def.m_aiBehavior == "DiamondReflector") {
				((DiamondReflector*)res1.m_entityHit)->TriggerShootRays( res1.m_rayForwardNormal );
			}
			endPos = res1.m_impactDist * res1.m_rayForwardNormal + startPos;
		}
		else {
			if (res2.m_entityHit->m_controller && res2.m_entityHit->m_controller->IsPlayer()) {
				res2.m_entityHit->BeAttacked( m_owner->GetMainWeaponDamage(), res2.m_impactNormal );
			}
			else if (m_weaponDef.m_canTriggerReflection && res2.m_entityHit->m_def.m_aiBehavior == "DiamondReflector") {
				((DiamondReflector*)res2.m_entityHit)->TriggerShootRays( res2.m_rayForwardNormal );
			}
			endPos = res2.m_impactDist * res2.m_rayForwardNormal + startPos;
		}
	}
	else if (res1.m_didImpact) {
		if (res1.m_entityHit->m_controller && res1.m_entityHit->m_controller->IsPlayer()) {
			res1.m_entityHit->BeAttacked( m_owner->GetMainWeaponDamage(), res1.m_impactNormal );
		}
		else if (m_weaponDef.m_canTriggerReflection && res1.m_entityHit->m_def.m_aiBehavior == "DiamondReflector") {
			((DiamondReflector*)res1.m_entityHit)->TriggerShootRays( res1.m_rayForwardNormal );
		}
		endPos = res1.m_impactDist * res1.m_rayForwardNormal + startPos;
	}
	else if (res2.m_didImpact) {
		if (res2.m_entityHit->m_controller && res2.m_entityHit->m_controller->IsPlayer()) {
			res2.m_entityHit->BeAttacked( m_owner->GetMainWeaponDamage(), res2.m_impactNormal );
		}
		else if (m_weaponDef.m_canTriggerReflection && res2.m_entityHit->m_def.m_aiBehavior == "DiamondReflector") {
			((DiamondReflector*)res2.m_entityHit)->TriggerShootRays( res2.m_rayForwardNormal );
		}
		endPos = res2.m_impactDist * res2.m_rayForwardNormal + startPos;
	}
	else {
		endPos = startPos + forwardVec * 50.f;
	}
	StarshipEffect* rayEffect = g_theGame->SpawnEffectToGame( EffectType::Laser, startPos, m_owner->m_orientationDegrees );
	((RayLaser*)rayEffect)->m_owner = m_owner;
	((RayLaser*)rayEffect)->m_rayEndPos = endPos;
	rayEffect->m_physicsRadius = m_owner->m_physicsRadius;
	rayEffect->BeginPlay();
	return nullptr;
}

WeaponDefinition::WeaponDefinition()
{

}

WeaponDefinition::WeaponDefinition( XmlElement* xmlIter )
{
	m_name = ParseXmlAttribute( *xmlIter, "name", m_name );
	m_behavior = ParseXmlAttribute( *xmlIter, "behavior", m_behavior );
	m_canTriggerReflection = ParseXmlAttribute( *xmlIter, "canTriggerReflection", m_canTriggerReflection );
	std::string projectileName = ParseXmlAttribute( *xmlIter, "projectileName", "None" );
	if (projectileName != "None" && projectileName != "NONE") {
		m_projectileDef = &ProjectileDefinition::GetDefinition( projectileName );
	}
}

void WeaponDefinition::SetUpWeaponDefinitions()
{
	WeaponDefinition::s_definitions.reserve( 64 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Definitions/WeaponDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document WeaponDefinitions.xml error" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "WeaponDefinitions" ), "Syntax Error! Name of the root of WeaponDefinitions.xml should be \"WeaponDefinitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "WeaponDefinition" ), "Syntax Error! Names of the elements of WeaponDefinitions.xml should be \"WeaponDefinition\" " );
		WeaponDefinition::s_definitions.emplace_back( xmlIter );
		xmlIter = xmlIter->NextSiblingElement();
	}
}

WeaponDefinition const& WeaponDefinition::GetDefinition( std::string const& name )
{
	for (auto const& def : s_definitions) {
		if (def.m_name == name) {
			return def;
		}
	}
	ERROR_AND_DIE( Stringf( "No Such Weapon Definition %s", name.c_str() ) );
}

Machete::Machete( WeaponDefinition const& def, Entity* owner )
	:Weapon( def, owner )
{

}

Machete::~Machete()
{

}

Projectile* Machete::Fire( Vec2 const& forwardVec, Vec2 const& startPos )
{
	UNUSED( forwardVec );
	UNUSED( startPos );
	StarshipEffect* effect = g_theGame->SpawnEffectToGame( EffectType::Slash, Vec2() );
	((SlashEffect*)effect)->m_owner = m_owner;
	((SlashEffect*)effect)->m_curve.m_startPos = Vec2( 0.1f, -3.8f ) * m_owner->m_cosmeticRadius;
	((SlashEffect*)effect)->m_curve.m_startVel = Vec2( 8.f, 6.f ) * m_owner->m_cosmeticRadius;
	((SlashEffect*)effect)->m_curve.m_endPos = Vec2( 0.4f, 3.4f ) * m_owner->m_cosmeticRadius;
	((SlashEffect*)effect)->m_curve.m_endVel = Vec2( 8.f, -6.f ) * m_owner->m_cosmeticRadius;
	if (m_fromRight) {
		Vec2 temp = ((SlashEffect*)effect)->m_curve.m_startPos;
		((SlashEffect*)effect)->m_curve.m_startPos = ((SlashEffect*)effect)->m_curve.m_endPos;
		((SlashEffect*)effect)->m_curve.m_endPos = temp;
		temp = ((SlashEffect*)effect)->m_curve.m_startVel;
		((SlashEffect*)effect)->m_curve.m_startVel = ((SlashEffect*)effect)->m_curve.m_endVel;
		((SlashEffect*)effect)->m_curve.m_endVel = temp;
	}
	effect->BeginPlay();
	m_fromRight = !m_fromRight;
	return nullptr;
}

RocketShooter::RocketShooter( WeaponDefinition const& def, Entity* owner )
	:Weapon(def, owner)
{

}

RocketShooter::~RocketShooter()
{

}

Projectile* RocketShooter::Fire( Vec2 const& forwardVec, Vec2 const& startPos )
{
	Projectile* projectile = g_theGame->SpawnProjectileToGame( GetProjectileDef(), startPos, m_owner->m_orientationDegrees, forwardVec * GetProjectileDef().m_speed );
	projectile->m_faction = m_owner->m_def.m_faction;
	projectile->m_damage = m_owner->GetMainWeaponDamage();
	projectile->BeginPlay();
	return projectile;
}

EnemyBulletGun::EnemyBulletGun( WeaponDefinition const& def, Entity* owner )
	:Weapon( def, owner )
{

}

EnemyBulletGun::~EnemyBulletGun()
{

}

Projectile* EnemyBulletGun::Fire( Vec2 const& forwardVec, Vec2 const& startPos )
{
	//float actualSpeed = DotProduct2D( m_owner->m_velocity, normal ) + m_projectileDef.m_speed;
	Projectile* projectile = g_theGame->SpawnProjectileToGame( GetProjectileDef(), startPos, forwardVec.GetOrientationDegrees(), forwardVec * GetProjectileDef().m_speed );
	projectile->m_faction = m_owner->m_def.m_faction;
	projectile->m_damage = m_owner->GetMainWeaponDamage();
	return projectile;
}

MissileGun::MissileGun( WeaponDefinition const& def, Entity* owner )
	:Weapon( def, owner )
{

}

MissileGun::~MissileGun()
{

}

Projectile* MissileGun::Fire( Vec2 const& forwardVec, Vec2 const& startPos )
{
	UNUSED( forwardVec );
	Projectile* projectile = g_theGame->SpawnProjectileToGame( GetProjectileDef(), startPos, 0.f );
	projectile->m_faction = m_owner->m_def.m_faction;
	projectile->m_damage = m_owner->GetMainWeaponDamage();
	return projectile;
}

Spray::Spray( WeaponDefinition const& def, Entity* owner )
	:Weapon(def, owner)
{
}

Spray::~Spray()
{

}

Projectile* Spray::Fire( Vec2 const& forwardVec, Vec2 const& startPos )
{
	if (m_isSector) {
		SectorSprayAttack* effect = (SectorSprayAttack*)g_theGame->SpawnEffectToGame( EffectType::SectorSprayAttack, startPos );
		effect->m_forwardDegrees = forwardVec.GetOrientationDegrees();
		effect->m_rangeDegrees = m_sectorRangeDegrees;
		effect->m_faction = m_owner->m_def.m_faction;
		effect->m_damage = m_owner->GetMainWeaponDamage();
		effect->m_dist = m_length;
		effect->m_isPoisonous = m_isPoisonous;
		effect->BeginPlay();
	}
	else {
		SprayAttack* effect = (SprayAttack*)g_theGame->SpawnEffectToGame( EffectType::SprayAttack, startPos );
		effect->m_boundingBox.m_center = startPos;
		effect->m_boundingBox.m_iBasisNormal = forwardVec;
		effect->m_faction = m_owner->m_def.m_faction;
		effect->m_damage = m_owner->GetMainWeaponDamage();
		effect->m_width = m_owner->m_cosmeticRadius * 2.f;
		effect->m_dist = m_length;
		effect->m_isPoisonous = m_isPoisonous;
		effect->BeginPlay();
	}

	return nullptr;
}

CoinGun::CoinGun( WeaponDefinition const& def, Entity* owner )
	:Weapon( def, owner )
{

}

CoinGun::~CoinGun()
{

}

Projectile* CoinGun::Fire( Vec2 const& forwardVec, Vec2 const& startPos )
{
	Projectile* projectile = g_theGame->SpawnProjectileToGame( GetProjectileDef(), startPos, forwardVec.GetOrientationDegrees(), forwardVec * GetProjectileDef().m_speed );
	projectile->m_faction = m_owner->m_def.m_faction;
	projectile->m_damage = m_owner->GetMainWeaponDamage();
	return projectile;
}
