#include "Game/ActorUID.hpp"
#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"

unsigned int const ActorUID::INVALID = 0;

ActorUID::ActorUID( unsigned int index, unsigned int salt )
{
	m_data = (salt << 16) | (index & (unsigned int)(0x0000ffff));
}

ActorUID::ActorUID()
{
}

bool ActorUID::IsValid() const
{
	Actor* pActor = g_theGame->m_curMap->m_actors[GetIndex()];
	return m_data != INVALID && pActor != nullptr && m_data == pActor->m_uid.m_data;
}

int ActorUID::GetIndex() const
{
	return m_data & 0x0000ffff;
}

Actor* ActorUID::GetActor() const
{
	Actor* pActor = g_theGame->m_curMap->m_actors[GetIndex()];
	if (m_data != INVALID && pActor != nullptr && m_data == pActor->m_uid.m_data) {
		return pActor;
	}
	return nullptr;
}

/*void Destroy(ActorUID& actorUID)
{
	Actor* pActor = g_theGame->m_curMap->m_actors[actorUID.GetIndex()];
	if (actorUID.m_data != ActorUID::INVALID && pActor != nullptr && actorUID.m_data == pActor->m_uid.m_data) {
		delete pActor;
		g_theGame->m_curMap->m_actors[actorUID.GetIndex()] = nullptr;
	}
}*/

Actor* ActorUID::operator->() const
{
	Actor* pActor = g_theGame->m_curMap->m_actors[GetIndex()];
	if (m_data != INVALID && pActor != nullptr && m_data == pActor->m_uid.m_data) {
		return pActor;
	}
	//ERROR_RECOVERABLE( Stringf( "Trying to Access NULL of ActorUID %h", m_data ) );
	return nullptr;
}

bool ActorUID::operator==( ActorUID other ) const
{
	return m_data == other.m_data;
}

/*
Actor& ActorUID::operator*() const
{
	Actor* pActor = g_theGame->m_curMap->m_actors[GetIndex()];
	if (m_data != INVALID && pActor != nullptr && m_data == pActor->m_uid.m_data) {
		return *pActor;
	}
	ERROR_AND_DIE( Stringf( "Trying to Access NULL of ActorUID %h", m_data ) );
}
*/
