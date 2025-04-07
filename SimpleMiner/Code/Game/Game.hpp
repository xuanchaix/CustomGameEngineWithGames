#pragma once
#include "Engine/Renderer/Camera.hpp"
#include <vector>

class Entity;
class Renderer;
class Clock;
class Player;
class Prop;
class RandomNumberGenerator;
class World;
typedef std::vector<Entity*> EntityList;

class Game {
public:
	RandomNumberGenerator* m_randNumGen = nullptr;
	Clock* m_gameClock = nullptr;
	Camera m_screenCamera;
	World* m_world;

public:
	Game();
	~Game();

	void Startup();
	void Update();
	void Render() const;

	void HandleKeys() const;
	void AddEntityToEntityArries( Entity* entityToAdd );
private:
	void UpdateEntityArrays();
	void UpdateEntityArray( EntityList& entityArray );
	void UpdateDebugRender();

	void SetupDefinitions();

private:
	EntityList m_entityArray;
};



