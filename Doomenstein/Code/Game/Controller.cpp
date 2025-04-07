#include "Game/Controller.hpp"
#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"

Controller::~Controller()
{

}

void Controller::Possess( ActorUID uid )
{
	//Actor* a = uid.GetActor();
	if (m_controlledActorUID.IsValid()) {
		m_controlledActorUID->OnUnpossessed();
	}
	if (uid.IsValid()) {
		uid->OnPossessed();
		if (uid->m_controller) {
			uid->m_controller->m_controlledActorUID = ActorUID();
		}
		if (uid->m_controller && uid->m_controller->IsAI()) {
			uid->m_AIController = uid->m_controller;
		}
		uid->m_controller = this;
		m_controlledActorUID = uid;
	}
}

Actor* Controller::GetActor() const
{
	return g_theGame->m_curMap->GetActorByUID( m_controlledActorUID );
}