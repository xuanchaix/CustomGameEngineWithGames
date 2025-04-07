#include "Game/Controller.hpp"
#include "Game/Entity.hpp"

Controller::~Controller()
{

}

void Controller::Render()
{

}

void Controller::Possess( Entity* entity )
{
	m_controlledEntity = entity;
	entity->m_controller = this;
}

Entity* Controller::GetControlledEntity() const
{
	return m_controlledEntity;
}
