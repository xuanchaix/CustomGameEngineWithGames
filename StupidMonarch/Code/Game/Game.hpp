#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Button.hpp"
#include "Engine/Renderer/Camera.hpp"

class Entity;
class Renderer;
class Map;
class Province;
class Force;
class Army;
class BattleReport;
class StupidMonarchAI;

enum class GameMode {
	CHOOSE_FORCES,
	VIEW_MAP,
	CLICK_ARMY,
	ARMY_FIGHT_INSPECT,
};

struct Scenario {
	int m_year;
	std::string m_name;
	std::string m_description;
};

class Game {
	friend class StupidMonarchAI;
public:
public:
	Game();
	~Game();

	void Startup();
	void Update( float deltaTime );
	void Render() const;

	void SetCurrentForce( Force* playerForce );
	Force* GetCurrentForce() const;

	AABB2 const& GetCameraRangeOnMap() const;
	Army const* GetChoosingArmy() const;
	void NextRound();

	void AddBattleReport( BattleReport* battleReport );
	void SetButtonVisable( std::string const& buttonName, bool isVisible );
	void SetButtonDisable( std::string const& buttonName, bool isDisable );

	void GoToGameMode( GameMode gameModeToGo );

	bool IsForceChosenByPlayer( Force* force ) const;
	void LoadScenarioHistory( Scenario const& scenario );
private:
	void NextTurn();
	bool CanGameStart();
	void LoadScenarios();
public:
	RandomNumberGenerator* m_randNumGen;
	Camera m_worldCamera;
	Camera m_screenCamera;
	Province* m_onInspectProv = nullptr;
	Map* m_map = nullptr;
	bool m_isShowingRank = false;

	//EntityList m_entityArray;
	Vec2 m_cameraCenter = Vec2( 0.f, 0.f );
	Force* m_curForce = nullptr;
	Army* m_choosingArmy = nullptr;
	int m_roundCount = 1;
	std::vector<BattleReport*> m_battleReports;
	float m_worldCameraScale = 1.f;
	float m_scrollSpeed = 2.f;
	std::vector<RectButton> m_buttons;

	// choosing force variables
	int m_numOfPlayers = 1;
	int m_curChoosingPlayer = 0;
	std::vector<int> m_playerChooseForceVector;
	Force* m_onInspectForce = nullptr;

	// Scenarios
	std::vector<Scenario> m_scenarios;
	int m_curScenario = 0;

	// save variables
	bool m_loadFromSave = false;
	Force* m_startForceFromSave = nullptr;

	SoundPlaybackID m_backgroundMusic = (SoundPlaybackID)-1;
};

bool OnAttractButtonClicked( EventArgs& args );
bool OnDefenseButtonClicked( EventArgs& args );
bool OnDevelopmentButtonClicked( EventArgs& args );
bool OnValidButtonClicked( EventArgs& args );
bool OnBuildArmyButtonClicked( EventArgs& args );
bool OnRankButtonClicked( EventArgs& args );
bool OnSaveButtonClicked( EventArgs& args );

bool OnRecruitButtonClicked( EventArgs& args );
bool OnRecruit5ButtonClicked( EventArgs& args );

bool OnYearButtonClicked( EventArgs& args );
bool OnNumOfPlayerButtonClicked( EventArgs& args );
bool OnGotoNextPlayerButtonClicked( EventArgs& args );
bool OnPlayGameButtonClicked( EventArgs& args );

