
#include "process.h"
#include "operation.h"

csp::Process::Process( lua::LuaState & luaThread, Process * parentProcess )
	: m_parentProcess( parentProcess )
	, m_luaThread( luaThread )
	, m_operation()
{
	SetProcess( m_luaThread.InternalState(), this );
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
	operation.SetProcess( *this );
	
	SwitchCurrentOperation( &operation );

	int numRets = 0;
	WorkResult::Enum res = CurrentOperation().Work( 0, numRets );
	switch( res )
	{
	case WorkResult::FINISH:
		SwitchCurrentOperation( NULL );
		return numRets;
	case WorkResult::YIELD:
		return m_luaThread.Yield( 0 );
	case WorkResult::ERROR:
	default:
		SwitchCurrentOperation( NULL );
		return m_luaThread.Error("error in operation"); // TODO: API for OPs to report errors.
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
