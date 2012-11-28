
#include "host.h"
#include "operation.h"

csp::Host::Host(const lua::LuaState& luaState)
    : m_luaState(luaState)
	, m_mainProcess()
{
}

csp::Host::~Host()
{
	assert( m_mainProcess == NULL );
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

	if( m_mainProcess )
	{
		delete m_mainProcess;
		m_mainProcess = NULL;
	}
}

csp::WorkResult::Enum csp::Host::Main()
{
	m_mainProcess = new Process( m_luaState, NULL );

	lua::LuaStackValue stackValue = m_mainProcess->LuaThread().GetGlobal("main");
	if ( !stackValue.IsFunction() || stackValue.IsCFunction() )
		return WorkResult::ERROR;
	
	lua::Return::Enum retValue = m_mainProcess->LuaThread().Resume(0, NULL);
	switch( retValue )
	{
	case lua::Return::YIELD:
		return WorkResult::YIELD;
	case lua::Return::OK:
		return WorkResult::FINISH;
	default:
		return WorkResult::ERROR;
	}
}

csp::WorkResult::Enum csp::Host::Work( time_t dt )
{
	return WorkResult::FINISH;
}

