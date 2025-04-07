#pragma once
class Actor;

typedef unsigned UID;

class ActorUID {
public:
	ActorUID();
	ActorUID( unsigned int index, unsigned int salt );

	bool IsValid() const;
	int GetIndex() const;
	Actor* GetActor() const;
	Actor* operator->() const;
	bool operator==( ActorUID other ) const;

	//friend void Destroy( ActorUID& actorUID );
	//Actor& operator*() const;

	static const UID INVALID;
private:
	UID m_data = INVALID;
};