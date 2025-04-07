#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Controller.hpp"
#include "Game/ActorUID.hpp"

class TileHeatMap;
class Map;

class AIController : public Controller {
public:
	virtual ~AIController();
	virtual bool IsPlayer() const override;
	virtual bool IsAI() const override;

	virtual void Damagedby( ActorUID uid, bool isLethal ) override;
	virtual void Update();
	bool IsLoseControl() const;
	void Refresh();

	bool HasLineOfSight( Vec3 const& targetPos ) const;

	TileHeatMap* m_tileHeatMap;
protected:
	void UpdateTileHeatMap( Map* curMap );
	void OptimizeRoute( Map* curMap );
	ActorUID m_targetActorUID;
	Vec2 m_targetLastFramePosition;
	Vec2 m_nextPositionToGo;
	int m_optimizedRouteIndex;
	std::vector<IntVec2> m_routeToTarget;
	bool m_isGarbage;
};

class MeleeAIController :public AIController {
	virtual void Update() override;
};

class RangedAIController :public AIController {
	virtual void Update() override;
};