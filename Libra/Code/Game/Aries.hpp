#pragma once
#include "Game/Entity.hpp"

class Bullet;
//Enemy logic
// state 1: wondering - See player - goto 2
// state 2: go for player - cannot see player or player dies - goto 3 
// state 3: lost player track - go to destination cannot see player goto 1 - once see player goto 2

//class Aries: cannot shoot but bouncing off player bullets
class Aries : public Entity {
public:
	Aries( Vec2 const& startPos, Map* map, EntityFaction faction );
	virtual ~Aries();

	virtual void Update( float deltaTime ) override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void DebugRender() const override;

	virtual void GetAttackedByBullet( Bullet* b ) override;
	virtual void BeAttacked( float hit ) override;
private:
	Texture* m_tankTexture;
	float m_wonderingCoolDown;
	float m_wonderingTime;
};