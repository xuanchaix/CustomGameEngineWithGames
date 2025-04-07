#pragma once
#include "Game/Entity.hpp"
//Enemy logic
// state 1: wondering - See player - goto 2
// state 2: go for player - cannot see player - goto 3 ->(I think is better)- if target dies - goto 1
// state 3: lost player track - go to destination cannot see player goto 1 - once see player goto 2

//class Leo: common shooting tank
class Leo : public Entity {
public:
	Leo( Vec2 const& startPos, Map* map, EntityFaction faction );
	virtual ~Leo();

	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void DebugRender() const override;

	virtual void BeAttacked( float hit ) override;
private:
	Texture* m_tankTexture;
	float m_wonderingCoolDown;
	float m_wonderingTime;
	float m_shootCoolDown;
};