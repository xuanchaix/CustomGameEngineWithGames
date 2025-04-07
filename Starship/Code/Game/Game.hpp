#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"

class PlayerShip;
class Entity;
enum class EntityType;
class Asteroid;
class Bullet;
class Debris;
class Renderer;
class RandomNumberGenerator;
struct Wave;
class Level;
class LaserBeam;
class LightSaber;
class BossFirstExplorer;
class BossDestroyer;
class string;
class Game;
class Rocket;
class BossDestroyerTurret;
class Clock;

enum class GameState {
	Playing, Upgrading
};

/*
update options:
faster ammo grow speed - all less ammo
faster speed
shield - shield time
power up time
(bullet attack) - super weapon(cool down)
rocket attack - higher range(more ammo)
(light saber attack) - more light saber(more ammo)

cone attack - larger range(more ammo)
*/
enum class UpgradeType {
	fasterAmmoGrow, lessAmmoCost, fasterSpeed, shield, shieldLessTime, powerUpTime, superBulletWeapon/*ui*/, rocketAttack/*todo*/,
	rocketLargerRange, moreLightSaber, coneAttack, coneAttackLargerRange, NUM, Empty
};


struct UpgradeSystem {
public:
	bool m_isDealt = false;
	bool m_upgradeStatus[(int)UpgradeType::NUM] = { false };
	UpgradeType m_thisUpgrade[3] = {};
	UpgradeType m_canUpgrade[(int)UpgradeType::NUM] = {
		UpgradeType::fasterAmmoGrow,
		UpgradeType::fasterSpeed,
		UpgradeType::shield,
		UpgradeType::powerUpTime,
		UpgradeType::superBulletWeapon,
		UpgradeType::rocketAttack,
		UpgradeType::moreLightSaber,
		UpgradeType::Empty,
		UpgradeType::Empty,
		UpgradeType::Empty,
		UpgradeType::Empty,
		UpgradeType::Empty,
	};
	void DealUpgrade( Game* game );
	void Upgrade( UpgradeType type );
};

constexpr int MAX_BACKGROUND_STARS = 50;
typedef std::vector<Entity*> EntityList;

class Game {
public:
	PlayerShip* m_playerShip = nullptr;		// Just one player ship (for now...)
	Entity* m_entities[MAX_ENTITIES] = {};
	Entity* m_debris[MAX_DEBRIS] = {}; // array stores effects(has no collision: debris, light saber, cone attack etc.)
	//Asteroid* m_asteroids[MAX_ASTEROIDS] = {};	// Fixed number of asteroid “slots”; nullptr if unused.
	//Bullet* m_bullets[MAX_BULLETS] = {};	// The “= {};” syntax initializes the array to zeros.
	RandomNumberGenerator* m_randNumGen;
	Camera m_worldCamera;
	Camera m_screenCamera;

	bool m_healthTutorial = false;
	bool m_healthTutorialCompleted = false;
	bool m_powerUpTutorial = false;
	bool m_powerUpTutorialCompleted = false;
	bool m_AmmoPowerUpTutorial = false;
	bool m_AmmoPowerUpTutorialCompleted = false;

	GameState m_gameState = GameState::Playing;

	float m_screenShakeCountSeconds = 0.f;
	Clock* m_gameClock = nullptr;

public:
	Game();
	~Game();

	void Startup();
	void Update();
	void Render() const;
	Bullet* CreateBullet( Vec2 startPos, Vec2 velocity, float orientationDegrees, bool isEnemyTrigger );
	void CreateCone( Vec2 startPos, float orientationDegrees, bool isEnemyTrigger, Entity const* ship, float radius, float apertureDegrees );
	void CreateLaser( Vec2 startPos, float orientationDegrees, bool isEnemyTrigger, Entity const* ship, float range );
	void CreateSaber( Vec2 startPos, Entity const* ship, float length, float cost=0 );
	void CreateRocket( Vec2 startPos, Vec2 velocity, float orientationDegrees, bool isEnemyTrigger, float range );
	void SpawnDebris( Entity const* deadEntity );
	void SpawnRocketDebris( Rocket const* rocket );
	void SpawnPowerUp( Entity const* deadEntity );
	void SpawnBossDestroyer();
	Entity** BossDestroyerSpawnEnemy();
	BossDestroyerTurret** BossDestroyerSpawnTurret( Entity* boss );
	void BossDestroyerSpawnBeetle();
	void BossDestroyerShootLaser( BossDestroyer* boss );
	void DealLaserDamage( LaserBeam const* laser );
	void DealSaberDamage( LightSaber const* saber );
	void DealRocketDamage( Vec2 const& rocketPos, float range );
	float GetDeltaSeconds() const;

	void ExplorerSpawnNewEnemy( BossFirstExplorer const* explorer );
	void ExplorerHealSelf( BossFirstExplorer* explorer, float range );
	Vec2* GetEnemiesPosInRange( Vec2 const& pos, float range );

	void RegisterRenderText( Text* txt );
	void StopRenderText();

	bool IsUpgraded( UpgradeType type ) const;
	Entity* GetNearestEnemyActor( Vec2 const& refPos ) const;
	Entity* GetNearestPlayerBullet( Vec2 const& refPos ) const;
	Entity* GetRandomEnemyActor() const;

	Entity* SpawnNewEntity( Entity* entity );
	bool SpawnNewDebris( Entity* entity );

	static bool Command_SetTimeScale( EventArgs& args );
private:

	void UpdateEntityArray( Entity** entityArray, int num );
	void RenderEntityArray( Entity* const* entityArray, int num ) const;
	void RenderUI() const;
	void RenderBackground() const;
	void DeleteGarbageInEntityArray( Entity** entityArray, int num );

	void DealCollision();
	bool IsCollide( EntityType type1, EntityType type2 ) const;

	void LoadLevel();
	bool StartNewWave( Wave* wave );
	void StartNewLevel();
	void MakeBackgroundStars();

	Vec2 MakeRandomOffMapPosition( float cosmeticRadius );

	float GetCurBossHealthRatio() const;

	// the array to monitor the entities left in enum class EntityType order
	// do not count entities bullets and debris
	// enum class EntityType { entity, playerShip, bullet, asteroid, beetle, wasp, debris, enemyBullet, boss, powerUp };
	int m_curEnemyAmount = 0;
	Level* m_levels[NUM_OF_LEVELS] = {};
	int m_currentLevel = 0;
	//Wave* m_waves[NUM_WAVES] = {};// save the waves
	//int m_currentWave = 0;

	float m_endGameTimeCount = 0.f;

	Vec2 m_backGroundStarPosition[MAX_BACKGROUND_STARS] = {};
	float m_backGroundStarSize[MAX_BACKGROUND_STARS] = {};

	int m_playerSaberIndex[5] = { -1, -1, -1, -1, -1 };

	bool m_isRenderingText = false;
	Text* m_renderingText = nullptr;
	float m_timeElapsedRenderingText = 0.f;
	float m_waitForUpgradeMode = 0.f;
	float m_powerUpTime = 10.f;
	UpgradeSystem m_upgradeSystem;

	int m_numOfSabers = NUM_OF_SABERS;

	bool m_showKeyTutorial = true;
	bool m_isBossFight = false;
};



