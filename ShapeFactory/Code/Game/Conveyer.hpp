#pragma once
#include "Game/Building.hpp"
#include "Game/GameCommon.hpp"


class Resource;

class Conveyer : public Building {
public:
	Conveyer( IntVec2 const& LBCoords );
	virtual ~Conveyer();
	virtual void Render() const;
	bool HasMoreThanOneEntrance() const;

	Vec2 GetExitPos() const;

	Direction m_dir = Direction::None;
	Conveyer* m_next = nullptr;
	Conveyer* m_rear = nullptr;
	Conveyer* m_left = nullptr;
	Conveyer* m_right = nullptr;
	//Resource* m_frontResource = nullptr;
	//Resource* m_backResource = nullptr;
	float m_conveySpeed = CONVEY_SPEED;
};