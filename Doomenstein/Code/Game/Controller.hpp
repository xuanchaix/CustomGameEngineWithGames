#pragma once
#include "Game/ActorUID.hpp"

class Controller {
public:
	virtual ~Controller();
	virtual bool IsPlayer() const = 0;
	virtual bool IsAI() const = 0;
	virtual void Damagedby( ActorUID uid, bool isLethal ) = 0;

	virtual void Possess( ActorUID uid );
	Actor* GetActor() const;

	ActorUID m_controlledActorUID;
};