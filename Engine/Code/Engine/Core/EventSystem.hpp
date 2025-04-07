#pragma once
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <mutex>
#include "Engine/Core/EngineCommon.hpp"
class NamedProperties;
class NamedStrings;
class InputSystem;
typedef std::vector<std::string> Strings;

//typedef void(*EventCallbackFunction)();
struct EventSystemConfig {
};


using EventCallbackStandAloneFunction = bool(*)(EventArgs&);

struct EventSubscriptionBase {
	virtual ~EventSubscriptionBase() = default;
	virtual bool FireEvent( EventArgs& args ) = 0;
};

struct EventSubscriptionStandaloneFunction : public EventSubscriptionBase {
	virtual ~EventSubscriptionStandaloneFunction() = default;
	EventSubscriptionStandaloneFunction( EventCallbackStandAloneFunction callbackFuncPtr );
	EventCallbackStandAloneFunction m_callbackFuncPtr = nullptr;

	virtual bool FireEvent( EventArgs& args ) override;
};

template<typename T_Object>
struct EventSubscriptionMemberFunction : public EventSubscriptionBase {
	virtual ~EventSubscriptionMemberFunction() = default;
	using EventCallbackMemberFunction = bool(T_Object::*)(EventArgs&);
	EventSubscriptionMemberFunction( EventCallbackMemberFunction callbackFuncPtr, T_Object* objectPtr ) 
		:m_callbackFuncPtr(callbackFuncPtr)
		,m_objectPtr(objectPtr)
	{
	};
	virtual bool FireEvent( EventArgs& args ) override {
		return (m_objectPtr->*m_callbackFuncPtr)( args );
	}
	EventCallbackMemberFunction m_callbackFuncPtr = nullptr;
	T_Object* m_objectPtr = nullptr;
};


typedef std::vector<EventSubscriptionBase*> SubscriptionList;

class EventSystem {
public:
	EventSystem( EventSystemConfig const& config );
	~EventSystem();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void SubscribeEventCallbackFunction( std::string const& eventName, EventCallbackStandAloneFunction functionPtr );
	void UnsubscribeEventCallbackFunction( std::string const& eventName, EventCallbackStandAloneFunction functionPtr );
	template<typename T_Object>
	void SubscribeEventCallbackFunction( std::string const& eventName, T_Object* objectPtr, bool(T_Object::* functionPtr)(EventArgs&) );
	template<typename T_Object>
	void UnsubscribeEventCallbackFunction( std::string const& eventName, T_Object* objectPtr, bool(T_Object::* functionPtr)(EventArgs&) );
	template<typename T_Object>
	void UnsubscribeAllEventCallbackFunctionForObject( T_Object* objectPtr );
	void UnsubscribeAllEventCallbackFunctionByName( std::string const& eventName );
	void FireEvent( std::string const& eventName, EventArgs& args );
	void FireEvent( std::string const& eventName );
	
	std::string GetNamesOfAllRegisteredCommands() const;
	void GetNamesOfAllRegisteredCommands(Strings& out_strs) const;

protected:
	std::map < HashedCaseInsensitiveString, SubscriptionList > m_subscriptionListsByEventName;
	EventSystemConfig m_config;

	std::mutex m_mutex;
};

template<typename T_Object>
void EventSystem::SubscribeEventCallbackFunction( std::string const& eventName, T_Object* objectPtr, bool(T_Object::* functionPtr)(EventArgs&) )
{
	m_mutex.lock();
	auto found = m_subscriptionListsByEventName.find( eventName );
	// if find push back
	if (found != m_subscriptionListsByEventName.end()) {
		found->second.push_back( new EventSubscriptionMemberFunction( functionPtr, objectPtr ) );
	}
	// if do not find, make a new event name and reserve
	else {
		m_subscriptionListsByEventName[eventName].reserve( 8 );
		m_subscriptionListsByEventName[eventName].push_back( new EventSubscriptionMemberFunction( functionPtr, objectPtr ) );
	}
	m_mutex.unlock();
}

template<typename T_Object>
void EventSystem::UnsubscribeEventCallbackFunction( std::string const& eventName, T_Object* objectPtr, bool(T_Object::* functionPtr)(EventArgs&) )
{
	m_mutex.lock();
	auto found = m_subscriptionListsByEventName.find( eventName );
	if (found != m_subscriptionListsByEventName.end()) {
		// unsubscribe all pointer of the same value
		for (int i = 0; i < (int)found->second.size(); i++) {
			EventSubscriptionMemberFunction<T_Object>* asMemberFunc = dynamic_cast<EventSubscriptionMemberFunction<T_Object>*>(found->second[i]);
			if (asMemberFunc && asMemberFunc->m_callbackFuncPtr == functionPtr && asMemberFunc->m_objectPtr == objectPtr) {
				delete found->second[i];
				found->second.erase( found->second.begin() + i );
				--i;
			}
		}
	}
	m_mutex.unlock();
}

template<typename T_Object>
void EventSystem::UnsubscribeAllEventCallbackFunctionForObject( T_Object* objectPtr )
{
	m_mutex.lock();
	for(auto& pair: m_subscriptionListsByEventName){
		// unsubscribe all pointer of the same value
		for (int i = 0; i < (int)pair.second.size(); i++) {
			EventSubscriptionMemberFunction<T_Object>* asMemberFunc = dynamic_cast<EventSubscriptionMemberFunction<T_Object>*>(pair.second[i]);
			if (asMemberFunc && asMemberFunc->m_objectPtr == objectPtr) {
				delete pair.second[i];
				pair.second.erase( pair.second.begin() + i );
				--i;
			}
		}
	}
	m_mutex.unlock();
}

void SubscribeEventCallbackFunction( std::string const& eventName, EventCallbackStandAloneFunction functionPtr );
void UnsubscribeEventCallbackFunction( std::string const& eventName, EventCallbackStandAloneFunction functionPtr );
void UnsubscribeAllEventCallbackFunctionByName( std::string const& eventName );
void FireEvent( std::string const& eventName, EventArgs& args );
void FireEvent( std::string const& eventName );

template<typename T_Object>
void SubscribeEventCallbackFunction( std::string const& eventName, T_Object* objectPtr, bool(T_Object::* functionPtr)(EventArgs&) );
template<typename T_Object>
void UnsubscribeEventCallbackFunction( std::string const& eventName, T_Object* objectPtr, bool(T_Object::* functionPtr)(EventArgs&) );
template<typename T_Object>
void UnsubscribeAllEventCallbackFunctionForObject( T_Object* objectPtr );

template<typename T_Object>
void UnsubscribeAllEventCallbackFunctionForObject( T_Object* objectPtr )
{
	if (g_theEventSystem) {
		g_theEventSystem->UnsubscribeAllEventCallbackFunctionForObject( objectPtr );
	}
}

template<typename T_Object>
void SubscribeEventCallbackFunction( std::string const& eventName, T_Object* objectPtr, bool(T_Object::* functionPtr)(EventArgs&) )
{
	if (g_theEventSystem) {
		g_theEventSystem->SubscribeEventCallbackFunction( eventName, objectPtr, functionPtr );
	}
}

template<typename T_Object>
void UnsubscribeEventCallbackFunction( std::string const& eventName, T_Object* objectPtr, bool(T_Object::* functionPtr)(EventArgs&) )
{
	if (g_theEventSystem) {
		g_theEventSystem->UnsubscribeEventCallbackFunction( eventName, objectPtr, functionPtr );
	}
}

class EventRecipient {
	virtual ~EventRecipient() = 0 {
		UnsubscribeAllEventCallbackFunctionForObject( this );
	}
};
