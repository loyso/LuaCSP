
#include "host.h"

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
}

void csp::Host::Shutdown()
{
	if( m_mainProcess )
	{
		delete m_mainProcess;
		m_mainProcess = NULL;
	}
}

lua::Return::Value csp::Host::Main()
{
	m_mainProcess = new Process( m_luaState, NULL );

	lua::LuaStackValue stackValue = m_mainProcess->LuaThread().GetGlobal("MAIN");
	if ( !stackValue.IsFunction() || stackValue.IsCFunction() )
		return lua::Return::ERRRUN;
	
	lua::Return::Value retValue = m_mainProcess->LuaThread().Call(0, 0);

	return retValue;
}

