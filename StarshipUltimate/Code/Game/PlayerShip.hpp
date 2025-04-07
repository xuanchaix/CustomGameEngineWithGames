#pragma once
#include "Game/Entity.hpp"
#include "Game/Projectile.hpp"
#include "Game/Weapon.hpp"

class Weapon;
class PlayerShield;
class PlayerAsteroid;
class PersistentRay;
class DiagonalRetinue;
class LaserWingPlane;
class WingPlane;

constexpr float REVENGE_BULLET_TIME = 0.8f;
constexpr float SELF_DAMAGE_BULLET_TIME = 1.0f;


class PlayerShip : public Entity {
public:
	PlayerShip( EntityDefinition const& def, Vec2 const& startPos, float startOrientation = 0.f, Vec2 const& startVelocity = Vec2() );
	virtual ~PlayerShip();

	virtual void BeginPlay() override;
	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void RenderUI() const;

	virtual void BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage = false, Vec2 const& projectileVelocity = Vec2() ) override;
	virtual bool IsInvincible() override;
	virtual bool Fire( Vec2 const& forwardVec, Vec2 const& startPos, bool forceToFire = false ) override;

	void DoDash( Vec2 const& direction );
	bool IsDashing() const;
	void RecoverHealth();
	void PerformSkill();
	bool FireMainWeapon();
	void FireSubWeapon();
	void Interact();

	void GainMaxHealth( float maxHealth );
	void GainMaxArmor( float maxArmor );
	void GainHealth( float health );
	void GainArmor( float armor );

	void GainItem( int itemID );
	void LoseItem( int itemID );
	bool HasItem( int itemID ) const;

	virtual float GetMainWeaponDamage() const override;
	float GetAttackCoolDown() const;
	float GetBulletPerSecond() const;
	float GetMovingSpeed() const;
	float GetBulletSpeed() const;
	float GetBulletLifeTime() const;
	float GetDashingCoolDown() const;
	float GetDashingDist() const;

	void GoToNextLevel();
	void CorrectAllFollowers();
public:
	std::vector<int> m_itemList;

	float m_damageModifier = 0.f; // multiply -
	float m_attackSpeedModifier = 0.f; // multiply -
	float m_movingSpeedModifier = 0.f; // multiply -
	float m_bulletSpeedModifier = 0.f; // multiply -
	float m_bulletLifeTimeModifier = 0.f; // multiply -
	float m_maxHealthModifier = 0.f; // add -
	float m_maxArmorModifier = 0.f; // add -
	float m_dashingCoolDownModifier = 0.f; // multiply -
	float m_dashingDistanceModifier = 0.f; // multiply -

	float m_curArmor;
	float m_maxArmor = 2.f;

	int m_skillItemID = -1;
	int m_subWeaponItemID = -1;
	float m_mainWeaponDamage = 0.f;

	float m_luckiness = 0.f;

	Timer* m_deathTimer = nullptr;

	// item effects
	bool m_upgradeItem = false;
	bool m_subWeaponShieldDashOn = false;
	Timer* m_subWeaponCoolDownTimer = nullptr;
	bool m_blindWithItem = false;
	bool m_bloodThirst = false;
	bool m_customerCard = false;
	bool m_discountCard = false;
	bool m_moreMoneyShopBonus = false;
	bool m_bribe = false;
	bool m_blindWithRoom = false;
	bool m_becomeLighter = false;
	bool m_collisionDamage = false;
	Timer* m_timeStopTimer = nullptr;
	bool m_healthBlind = false;
	bool m_richCanDoAnything = false;
	bool m_itemAutoRefresh = false;

protected:
	void SetBulletAttributes( Projectile* proj );
	void AddItemToItemList( int id );
	void RemoveItemFromItemList( int id );
	void SetItemEffect( std::string const& type, bool isGetItem );
	void RerandomizeAllItems( int excludeItem = -1 );

	void StartRespawnParticle();
	void FireBulletWeapon( Vec2 const& fwdVec, Vec2 const& bulletPos );
	void FireRayWeapon( Vec2 const& fwdVec, Vec2 const& startPos );
	void FireSprayerWeapon( Vec2 const& fwdVec, Vec2 const& startPos, bool sectorAttack=false );

	static bool Event_GetItem( EventArgs& args );
	static bool Event_LoseItem( EventArgs& args );
	static bool Event_RandItem( EventArgs& args );

private:
	float m_dashingCooldown = 1.5f;
	Timer* m_invincibleTimer = nullptr;
	Timer* m_dashTimer = nullptr;

	// item effects
	bool m_threeBullets = false;
	bool m_bulletBias = false;
	bool m_spearBullet = false;
	bool m_rangeDamage = false;
	bool m_revengeBullet = false;
	Timer* m_revengeBulletTimer = nullptr;
	Timer* m_revengeBulletShootTimer = nullptr;
	bool m_respawn1Health = false;
	bool m_respawned1HealthBefore = false;
	bool m_respawnHalfHealth = false;
	bool m_respawnedHalfHealthBefore = false;
	bool m_loseHealthGainArmor = false;
	bool m_loseArmorGainHealth = false;
	bool m_revengeSpeed = false;
	Timer* m_revengeSpeedTimer = nullptr;
	bool m_lowerHealthAdvantage = false;
	PlayerAsteroid* m_asteroid = nullptr;
	bool m_poisonousBullet = false;
	bool m_demonContract = false;
	bool m_medicalInsurance = false;
	bool m_toilet = false;
	bool m_biggerBullet = false;
	bool m_moreMoneyMakesMeBetter = false;
	float m_vengefulAttack = 0.f;
	bool m_moreDamageMoreAttack = false;
	bool m_shootLaserInstead = false;
	bool m_movementDamage = false;
	bool m_biggerBody = false;
	bool m_bacteriaSprayer = false;
	Weapon* m_sprayer = nullptr;
	bool m_healthToCharge = false;
	bool m_shotGun = false;
	bool m_missile = false;
	bool m_bulletAddDamageByLifeTime = false;
	bool m_iceBullet = false;
	bool m_damageByEmptyHealth = false;
	bool m_chanceShootSpray = false;
	bool m_rerandomizeItemsWhenDamaged = false;
	int m_rerandomizeKeepItemID = -1;
	bool m_armorHealthRandomLose = false;
	DiagonalRetinue* m_diagonalRetinue = nullptr;
	LaserWingPlane* m_laserWingPlane = nullptr;
	WingPlane * m_wingPlane = nullptr;

	// skills
	bool m_skillSelfDamage = false;
	Timer* m_selfDamageBulletTimer = nullptr;
	Timer* m_selfDamageShootTimer = nullptr;
	bool m_shiningShield = false;
	bool m_hasShield = false;
	PlayerShield* m_shield = nullptr;
	bool m_itemChooseAdd1Health = false;
	bool m_selfHurtMachine = false;
	bool m_attackEnhancement = false;
	bool m_armorGenerator = false;
	bool m_timeStop = false;
	bool m_healthToMaxArmor = false;
	bool m_damageTrap = false;
	bool m_teleport = false;
	bool m_itemDice = false;
	bool m_generateShop = false;

	// sub-weapon
	Clock* m_subWeaponClock = nullptr;
	bool m_subWeaponShieldDash = false;
	bool m_subWeaponLaser = false;
	bool m_subWeaponRocket = false;
	Weapon* m_subWeaponRocketPtr = nullptr;
	bool m_subWeaponExplosive = false;
	bool m_subWeaponCoinGun = false;
	Weapon* m_subWeaponCoinGunPtr = nullptr;
	bool m_subWeaponElectricChain = false;
};