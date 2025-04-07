#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/DevConsole.hpp"

EventSystem* g_theEventSystem = nullptr;

EventSystem::EventSystem( EventSystemConfig const& config )
	:m_config(config)
{

}

EventSystem::~EventSystem()
{
	for (auto& pair : m_subscriptionListsByEventName) {
		for (auto eventSubBase : pair.second) {
			delete eventSubBase;
		}
	}
}

void EventSystem::Startup()
{
	SubscribeEventCallbackFunction( "LoadGameConfig", Event_LoadGameConfig );
}

void EventSystem::Shutdown()
{

}

void EventSystem::BeginFrame()
{

}

void EventSystem::EndFrame()
{

}

void EventSystem::SubscribeEventCallbackFunction( std::string const& eventName, EventCallbackStandAloneFunction functionPtr )
{
	m_mutex.lock();
	auto found = m_subscriptionListsByEventName.find( eventName );
	// if find push back
	if (found != m_subscriptionListsByEventName.end()) {
		found->second.push_back( new EventSubscriptionStandaloneFunction( functionPtr ) );
	}
	// if do not find, make a new event name and reserve
	else {
		m_subscriptionListsByEventName[eventName].reserve( 8 );
		m_subscriptionListsByEventName[eventName].push_back( new EventSubscriptionStandaloneFunction( functionPtr ) );
	}
	m_mutex.unlock();
}

void EventSystem::UnsubscribeEventCallbackFunction( std::string const& eventName, EventCallbackStandAloneFunction functionPtr )
{
	m_mutex.lock();
	auto found = m_subscriptionListsByEventName.find( eventName );
	if (found != m_subscriptionListsByEventName.end()) {
		// unsubscribe all pointer of the same value
		for (int i = 0; i < (int)found->second.size(); i++) {
			EventSubscriptionStandaloneFunction* asStandaloneFunc = dynamic_cast<EventSubscriptionStandaloneFunction*>(found->second[i]);
			if (asStandaloneFunc && asStandaloneFunc->m_callbackFuncPtr == functionPtr) {
				delete found->second[i];
				found->second.erase( found->second.begin() + i );
				--i;
			}
		}
	}
	m_mutex.unlock();
	//else {
	//	ERROR_RECOVERABLE( Stringf( "Cannot find eventName %s when unsubscribing event call back function!", eventName.c_str() ) );
	//}
}

void EventSystem::UnsubscribeAllEventCallbackFunctionByName( std::string const& eventName )
{
	m_mutex.lock();
	auto found = m_subscriptionListsByEventName.find( eventName );
	if (found != m_subscriptionListsByEventName.end()) {
		for (int i = 0; i < (int)found->second.size(); i++) {
			delete found->second[i];
		}
		found->second.clear();
	}
	m_mutex.unlock();
}

void EventSystem::FireEvent( std::string const& eventName, EventArgs& args )
{
	m_mutex.lock();
	auto found = m_subscriptionListsByEventName.find( eventName );
	if (found != m_subscriptionListsByEventName.end()) {
		SubscriptionList copyedList = found->second;
		m_mutex.unlock();
		for (int i = 0; i < (int)copyedList.size(); i++) {
			// if is consumed by this call back function, stop calling others
			if (copyedList[i]->FireEvent( args )) {
				return;
			}
		}
	}
	else {
		m_mutex.unlock();
		g_devConsole->AddLine( DevConsole::INFO_ERROR, Stringf( " Error! Cannot Fire Event Name %s", eventName.c_str() ) );
	}
	//m_mutex.unlock();
	//else {
	//	ERROR_AND_DIE( Stringf( "Cannot find eventName %s when fire event!", eventName.c_str() ) );
	//}
}

void EventSystem::FireEvent( std::string const& eventName )
{
	EventArgs args;
	FireEvent( eventName, args );
}


std::string EventSystem::GetNamesOfAllRegisteredCommands() const
{
	std::string retStr = "";
	for (auto p : m_subscriptionListsByEventName) {
		if (p.first.GetString().find("Command") == 0) {
			//std::string strToAdd = SplitStringOnDelimiter( p.first, ' ' )[1];
			retStr += (p.first.GetString().substr(8) + " ");
		}
	}
	return retStr;
}

void EventSystem::GetNamesOfAllRegisteredCommands( Strings& out_strs ) const
{
	for (auto p : m_subscriptionListsByEventName) {
		if (p.first.GetString().find( "Command" ) == 0) {
			//std::string strToAdd = SplitStringOnDelimiter( p.first, ' ' )[1];
			out_strs.push_back( p.first.GetString().substr( 8 ) + " " );
		}
	}
}

void SubscribeEventCallbackFunction( std::string const& eventName, EventCallbackStandAloneFunction functionPtr )
{
	if (g_theEventSystem) {
		g_theEventSystem->SubscribeEventCallbackFunction( eventName, functionPtr );
	}
}

void UnsubscribeEventCallbackFunction( std::string const& eventName, EventCallbackStandAloneFunction functionPtr )
{
	if (g_theEventSystem) {
		g_theEventSystem->UnsubscribeEventCallbackFunction( eventName, functionPtr );
	}
}

void UnsubscribeAllEventCallbackFunctionByName( std::string const& eventName )
{
	if (g_theEventSystem) {
		g_theEventSystem->UnsubscribeAllEventCallbackFunctionByName( eventName );
	}
}

void FireEvent( std::string const& eventName, EventArgs& args )
{
	if (g_theEventSystem) {
		g_theEventSystem->FireEvent( eventName, args );
	}
}

void FireEvent( std::string const& eventName )
{
	if (g_theEventSystem) {
		g_theEventSystem->FireEvent( eventName );
	}
}

EventSubscriptionStandaloneFunction::EventSubscriptionStandaloneFunction( EventCallbackStandAloneFunction callbackFuncPtr )
	:m_callbackFuncPtr(callbackFuncPtr)
{
}

bool EventSubscriptionStandaloneFunction::FireEvent( EventArgs& args )
{
	if (m_callbackFuncPtr) {
		return m_callbackFuncPtr( args );
	}
	return false;
}
