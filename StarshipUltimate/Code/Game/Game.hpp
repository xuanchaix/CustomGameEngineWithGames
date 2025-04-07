#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/Projectile.hpp"
#include "Game/Effects.hpp"
#include "Game/Weapon.hpp"
#include "Engine/Renderer/Camera.hpp"

class Renderer;
class Clock;
class PlayerController;
class DiamondReflector;
class Room;
class PlayerShip;

struct StarshipRayCastResult : public RayCastResult2D {
	StarshipRayCastResult();
	StarshipRayCastResult( RayCastResult2D const& res );
	bool m_isEntityHit = false;
	Entity* m_entityHit = nullptr;
};

enum class GameState {
	IN_ROOM,
	GO_TO_NEXT_ROOM
};

/// Go to next level: do not forget to set items in room back to pool
class Game {
public:
	RandomNumberGenerator* m_randNumGen = nullptr;
	Clock* m_gameClock = nullptr;
	Camera m_worldCamera;
	Camera m_screenCamera;
	EntityList m_entityArray;
	Room* m_curRoom = nullptr;
	std::vector<Room*> m_roomMap;
	GameState m_state = GameState::IN_ROOM;
	Vec2 m_goToNextRoomCameraTargetCenter = Vec2();
	Timer* m_goToNextRoomTimer = nullptr;

	bool m_isChoosingItems = false;
	int m_numOfItemsCanChoose = 0;
	std::vector<int> m_showingItems;
	std::vector<int> m_hidingShopItems;

	PlayerController* m_playerController;
	std::vector<Projectile*> m_projectileArray;

	int m_curLevel = 0;
	std::vector<std::string> m_levelSequence;
	std::vector<std::string> m_levelMusicSequence;
	std::string m_bossMusic = "Default";

	int m_savedCoins = 0;
	bool m_renderMapScreen = false;
	bool m_renderItemScreen = false;
	int m_insepctingItem = 0;
	bool m_renderOptionsMenu = false;
	bool m_isQuitting = false;
	bool m_goToNextFloorNextFrame = false;

	SoundPlaybackID m_backgroundMusicID = (SoundPlaybackID)-1;
public:
	Game();
	~Game();

	void Startup();
	void Update();
	void Render() const;

	Entity* GetPlayerEntity() const;
	PlayerShip* GetPlayerObject() const;

	void SpawnPlayerToGame();
	Entity* SpawnEntityToGame( EntityDefinition const& def, Vec2 const& position, float orientationDegrees, Vec2 const& initialVelocity = Vec2( 0.f, 0.f ) );
	Projectile* SpawnProjectileToGame( ProjectileDefinition const& def, Vec2 const& position, float orientationDegrees, Vec2 const& initialVelocity = Vec2( 0.f, 0.f ) );
	StarshipEffect* SpawnEffectToGame( EffectType type, Vec2 const& position, float orientationDegrees = 0.f, Vec2 const& initialVelocity = Vec2( 0.f, 0.f ), bool pushBack = false );

	void AddEntityToGame( Entity* entityToAdd );
	void RemoveEntityFromGame( Entity* entityToRemove );

	StarshipRayCastResult RayCastVsEntities( Vec2 position, Vec2 direction, float maxDist, Entity* ignoreEntity, std::string const& targetFaction, bool targetReflector = false );
	
	std::vector<DiamondReflector*> GetAllDiamondReflectors() const;

	void StartCameraShake();
	void SetOneDirCameraShake( Vec2 const& displacement, float seconds );

	Weapon* CreateWeaponComponent( WeaponDefinition const& def, Entity* owner ) const;

	Room* GetRoomInDirectionByCurRoom( RoomDirection dir ) const;
	Room* GetRoomAtCoords( IntVec2 const& coords ) const;
	bool CanLeaveCurrentRoom() const;
	void EnterRoom( Room* roomToEnter, RoomDirection dirFrom );

	void DebugKillEverything();

	bool IsPlayerBulletInRange( Vec2 const& pos, float rangeRadius ) const;

	void DealRangeDamage( Projectile* proj, bool dealOnce = false, float coolDownTime = 0.f );
	void DealRangeDamage( float damage, Vec2 const& position, float range, std::string const& sourceFaction="Default", bool dealOnce = false, float coolDownTime = 0.f, void* source = nullptr);

	void PickUpItem( Vec2 const& pos );
	void TransferItemToMaxHealth( Vec2 const& pos );

	bool IsInShop() const;

	void EnterChooseItemMode();
	void AddItemToChoose( int itemID );
	void ExitChooseItemMode();

	void RoomClearCallBack( Room* room );

	Entity* GetNearestEnemy( Vec2 const& position, float maxDist, Entity* excludeEnemy = nullptr ) const;

	void GoToNextFloor();

	void SetTimeScaleOfAllEntity( std::string const& friendlyFaction, float newTimeScale );

	void RerandomizeAllItemsInRoom( bool onlyShop = false);

	void GenerateNewShopToLevel( int levelOfShop = 1 );

	void GetAllEntityByDef( std::vector<Entity*>& out_entityArray, EntityDefinition const& def ) const;
private:
	void HandleKeys();
	void BeginGame();
	void SetUpRooms( std::string const& faction, int level = 0 );
	void SetUpLastFloor( int level = 0 );
	void UpdateAllGameObjects( float deltaSeconds );
	void RemoveGarbageGameObjects();

	void RenderAllGameObjects() const;
	void RenderItemsInRoom() const;
	void DebugRenderAllGameObjects() const;
	void AddProjectileToGame( Projectile* projectile );
	void RenderUI() const;

	void UpdateCollisions();
	void CollideTwoEntities( Entity* a, Entity* b );
	void CollideEntityAndProjectile( Entity* entity, Projectile* projectile );

	bool IsRoomCoordsInBounds( IntVec2 const& coords ) const;
	bool IsRoomAdjToOther( IntVec2 const& coords ) const;

	void RandomSetRoom( std::string const& faction, int num, int level );

	void RenderMapScreen() const;
	void RenderItemScreen() const;
	void RenderOptionsScreen() const;

	void GetTheNearestItemIDAndDist( int& id, float& dist, Vec2 const& refPos ) const;

	void GenerateHealthOrArmorPickup( Vec2 const& pos );

	void CleanUpCurLevel();

	std::string GetRandomFaction() const;

	void SaveGameBetweenRuns() const;
	void ReadGameSaveData();
private:
	IntVec2 m_roomDimensions = IntVec2( 9, 9 );
	int m_numOfRooms = 0;
	Vec2 m_cameraCenter;
	std::vector<StarshipEffect*> m_effectArray;
	bool m_debugMode = false;
	std::vector<Controller*> m_controllers;
	PlayerShip* m_playerShip;
	Timer* m_cameraShakeTimer = nullptr;
	Timer* m_cameraPerShakeTimer = nullptr;
	Timer* m_cameraOneShakeTimer = nullptr;
	bool m_cameraShakeThisFrame = false;
	Vec2 m_cameraFrameShakeDisplacement = Vec2();

	bool m_clearTheRoomFisrtTime = true;


	int m_numOfHealthPickupGenerated = 0;
	int m_maxNumOfHealthPickupThisLevel = 6;

	NextFloorPortal* m_portal = nullptr;

	int m_curHoveringButtom = 0;
	int m_numOfButtons = 4;
	int m_bossKilledThisFloor = 0;
};



