#include "process.h"

#include "operation.h"
#include "host.h"

#include <luacpp/luastackvalue.h>

csp::Process::Process()
	: m_luaThread()
	, m_parentProcess()
	, m_operation()
{
}

csp::Process::~Process()
{
	CORE_ASSERT( m_operation == NULL );
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
	CORE_ASSERT( m_operation );
	return *m_operation;
}

void csp::Process::Work( Host& host, CspTime_t dt )
{
	if( m_operation == NULL )
		return;

	WorkResult::Enum result = m_operation->Work( host, dt );
	
	if( result == WorkResult::FINISH )
	{
		m_operation->SetFinished( true );
		CORE_ASSERT( m_luaThread.Status() == lua::Return::YIELD );
		host.PushEvalStep( *this );
	}
}

csp::WorkResult::Enum csp::Process::StartEvaluation( Host& host, int numArgs )
{
	CORE_ASSERT( m_operation == NULL );
	CORE_ASSERT( LuaThread().Status() == lua::Return::OK );
	CORE_ASSERT( LuaThread().GetStack()[-numArgs-1].IsFunction() );

	return Evaluate( host, numArgs );
}

csp::WorkResult::Enum csp::Process::Evaluate( Host& host, int numArgs )
{
	if( LuaThread().Status() != lua::Return::YIELD && LuaThread().Status() != lua::Return::OK )
		return WorkResult::FINISH;

	if( m_operation )
	{
		WorkResult::Enum result = m_operation->IsFinished()
			? WorkResult::FINISH
			: m_operation->Evaluate( host );

		if( result == WorkResult::FINISH )
		{
			lua::LuaStack luaStack = m_luaThread.GetStack();
			numArgs = m_operation->PushResults( luaStack );
			DeleteOperation( host );
		}
		else
			return WorkResult::YIELD;
	}
	
	WorkResult::Enum result = Resume( numArgs );
	
	if ( result == WorkResult::YIELD && m_operation )
		host.PushEvalStep( *this );
	else if ( result == WorkResult::FINISH && m_parentProcess )
	{
		if( host.GetTopProcess() != m_parentProcess )
			host.PushEvalStep( *m_parentProcess );
	}

	return result;
}


csp::WorkResult::Enum csp::Process::Resume( int numArgs )
{
	lua::Return::Enum retValue = LuaThread().Resume( numArgs, m_parentProcess ? &m_parentProcess->LuaThread() : NULL );
	if( retValue == lua::Return::YIELD )
		return WorkResult::YIELD;

	return WorkResult::FINISH;
}

void csp::Process::DoTerminate( Host& host )
{
	Terminate( host );
}

void csp::Process::Terminate( Host& host )
{
	if( m_operation )
	{
		m_operation->DoTerminate( host );
		DeleteOperation( host );
	}
	
	host.RemoveProcessFromStack( *this );
}

void csp::Process::DeleteOperation( Host& host )
{
	CORE_ASSERT( m_operation );

	m_operation->DebugCheck( host );
	SwitchCurrentOperation( NULL );
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
