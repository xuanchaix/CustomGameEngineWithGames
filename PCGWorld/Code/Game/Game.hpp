#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/Map.hpp"

class Entity;
class Renderer;
class Clock;
class Map;
struct MapGenerationSettings;

class Game {
public:
	RandomNumberGenerator* m_randNumGen = nullptr;
	Clock* m_gameClock = nullptr;
	Camera m_worldCamera;
	Camera m_screenCamera;
	Map* m_map;
	Vec2 m_cameraCenter;

	float m_worldCameraScale = 3.f;
	float m_2DMaxWorldCameraHeight = 100.f;
	float m_2DMinWorldCameraHeight = 10.f;
	float m_2DScalingSpeed = 150.f;
	MapViewMode m_viewMode = MapViewMode::ViewMode2D;

	Timer* m_loadingTimer = nullptr;
public:
	Game();
	~Game();

	void Startup();
	void Update();
	void Render() const;

	void HandleKeys();

protected:
	void UpdateMouseInput();
	void UpdateUI();
	void GenerateNewMap( MapGenerationSettings const& settings );

protected:
	void Update2DPopupUI();
	void UpdateSpherePopupUI();
	void Update3DPopupUI();
	void UpdateControlPanelUI();
	void UpdateHistorySimulationUI();
	void UpdateHistoryLogUI();
	void UpdateLegendsUI();
	void UpdateClimateLegend();
	void UpdateLandformLegend();
	void UpdateCultureLegend();
	void UpdateReligionLegend();
	void UpdateProductLegend();
	void MakeLinearValueLegend( std::string const& title, float minValue, float maxValue, Rgba8 minColor, Rgba8 maxColor, std::string const& unitName, int numOfSteps = 10 );
	void UpdateRelationLegend();

	void UpdateProvinceUI();
	void UpdateCityUI();
	void UpdateArmyUI();
	void UpdateSaveHistoryPopupUI();

	void SaveCurrentGenerationSettings();
	void LoadGenerationSettings();
	void ClearSaves();

	void RenderAttractScreen() const;

	EntityList m_entityArray;

	std::deque<Job*> m_jobList;
	int m_jobListFirst = 0;
	float m_scrollSpeed = 10.f;
	MapGenerationSettings m_generationSettings;
	IntVec2 m_simulateToTime;
	IntVec2 m_simulateAmountTime;
	bool m_isLabelDirty = false;
};



