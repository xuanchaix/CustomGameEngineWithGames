#pragma once
class Entity;

class Controller {
public:
	virtual ~Controller();
	virtual bool IsPlayer() const = 0;
	virtual bool IsAI() const = 0;
	virtual void Update( float deltaSeconds ) = 0;
	virtual void Render();

	virtual void Possess( Entity* entity );
	Entity* GetControlledEntity() const;

protected:
	Entity* m_controlledEntity;
};