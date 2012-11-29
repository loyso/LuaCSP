
#include "process.h"
#include "operation.h"

csp::Process::Process()
	: m_luaThread()
	, m_parentProcess()
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

csp::Process* csp::Process::GetProcess( lua_State* luaState )
{
	void* pUserData = lua::LuaState::GetUserData( luaState );
	return static_cast< Process* >( pUserData );
}

void csp::Process::SetProcess( lua_State* luaState, Process* process )
{
	lua::LuaState::SetUserData( luaState, process );
}

int csp::Process::Operate( Operation& operation )
{
	SwitchCurrentOperation( &operation );

	WorkResult::Enum res = CurrentOperation().Work( 0 );
	switch( res )
	{
	case WorkResult::FINISH:
	default:
		{
			lua::LuaStack luaStack = m_luaThread.GetStack();
			int numRets = CurrentOperation().PushResults( luaStack );
			SwitchCurrentOperation( NULL );
			return numRets;
		}
	case WorkResult::YIELD:
		return m_luaThread.Yield( 0 );
	}
}

void csp::Process::SwitchCurrentOperation( Operation* pOperation )
{
	if( m_operation )
	{
		delete m_operation;
		m_operation = NULL;
	}

	m_operation = pOperation;
}

csp::Operation& csp::Process::CurrentOperation()
{
	assert( m_operation );
	return *m_operation;
}

csp::WorkResult::Enum csp::Process::Work( time_t dt )
{
	if( m_operation )
	{
		WorkResult::Enum result = m_operation->Work( dt );
		if( result == WorkResult::YIELD )
			return WorkResult::YIELD;
	}
	
	if( m_luaThread.Status() == lua::Return::YIELD )
	{
		int numArgs = 0;
		if( m_operation )
		{
			lua::LuaStack luaStack = m_luaThread.GetStack();
			numArgs = m_operation->PushResults( luaStack );
			SwitchCurrentOperation( NULL );
		}
		return Resume( numArgs );
	}

	SwitchCurrentOperation( NULL );
	return WorkResult::FINISH;
}

csp::WorkResult::Enum csp::Process::Resume( int numArgs )
{
	lua::Return::Enum retValue = LuaThread().Resume( numArgs, m_parentProcess ? &m_parentProcess->LuaThread() : NULL );
	if( retValue == lua::Return::YIELD )
		return WorkResult::YIELD;

	return WorkResult::FINISH;
}

bool csp::Process::IsRunning() const
{
	return m_operation != NULL || m_luaThread.InternalState() == NULL || m_luaThread.Status() == lua::Return::YIELD;
}

void csp::Process::SetLuaThread( const lua::LuaState& luaThread )
{
	m_luaThread = luaThread;
	Process::SetProcess( m_luaThread.InternalState(), this );
}

void csp::Process::SetParentProcess( Process& parentProcess )
{
	m_parentProcess = &parentProcess;
}
