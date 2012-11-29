
#include "host.h"
#include "operation.h"

csp::Host::Host(const lua::LuaState& luaState)
    : m_luaState(luaState)
	, m_mainProcess()
{
}

csp::Host::~Host()
{
}

lua::LuaState& csp::Host::LuaState()
{
    return m_luaState;
}

void csp::Host::Initialize()
{
	lua::LuaStackValue globals = m_luaState.GetGlobals();
	RegisterStandardOperations( m_luaState, globals );
}

void csp::Host::Shutdown()
{
	lua::LuaStackValue globals = m_luaState.GetGlobals();
	UnregisterStandardOperations( m_luaState, globals );
}

csp::WorkResult::Enum csp::Host::Main()
{
	m_mainProcess.SetLuaThread( m_luaState );

	lua::LuaStackValue stackValue = m_mainProcess.LuaThread().GetGlobal("main");
	if ( !stackValue.IsFunction() || stackValue.IsCFunction() )
		return WorkResult::FINISH;
	
	return m_mainProcess.Resume(0);
}

csp::WorkResult::Enum csp::Host::Work( time_t dt )
{
	if( !m_mainProcess.IsRunning() )
		return WorkResult::FINISH;

	return m_mainProcess.Work( dt );
}

