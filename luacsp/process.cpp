
#include "process.h"

csp::Process::Process( lua::LuaState & luaThread, Process * parentProcess )
	: m_parentProcess( parentProcess )
	, m_luaThread( luaThread )
	, m_operation()
{
}

csp::Process::~Process()
{
	assert( m_operation == NULL );
}

lua::LuaState & csp::Process::LuaThread()
{
	return m_luaThread;
}
