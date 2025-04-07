#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"

class Entity;
class Renderer;
class Clock;
class Player;
class Prop;
class Model;

class Game {
public:
	RandomNumberGenerator* m_randNumGen = nullptr;
	Clock* m_gameClock = nullptr;
	Camera m_screenCamera;
	bool m_debugRotation = false;

public:
	Game();
	~Game();

	void Startup();
	void Update();
	void Render() const;

	void HandleKeys();
	void AddEntityToEntityArrays( Entity* entityToAdd );
	void RemoveEntityFromEntityArrays( Entity* entityToRemove );

private:
	void UpdateEntityArrays();
	void UpdateEntityArray( EntityList& entityArray );
	void UpdateDebugRender();

	void RenderUI() const;

	void SetupTestProps();
	void SetupGrids( int minX = -50, int maxX = 50, int minY = -50, int maxY = 50 );
	void ExitScene( int scene );
	void GoToScene( int scene );

private:
	EntityList m_entityArray;
	Player* m_player;
	Timer* m_timer;

	std::vector<Vertex_PCU> m_gridVerts;

	bool m_debugRender = false;
	Model* m_model = nullptr;
	DirectionalLightConstants m_dlc;
	EulerAngles m_sunOrientation;

	Model* m_model0 = nullptr;
	Model* m_model1 = nullptr;
	Model* m_model2 = nullptr;
	Model* m_model3 = nullptr;
	Model* m_model4 = nullptr;
	Prop* m_prop0 = nullptr;
	Prop* m_prop1 = nullptr;
	Prop* m_prop2 = nullptr;
	Prop* m_prop3 = nullptr;
	Model* m_tutorialBox = nullptr;
	Model* m_hadrianTank = nullptr;

	int m_curScene = 0;
};



