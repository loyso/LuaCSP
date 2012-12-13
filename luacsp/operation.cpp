#include "operation.h"

#include <luacpp/luacpp.h>
#include <luacpp/luastackvalue.h>

#include "op_alt.h"
#include "op_par.h"

#include "process.h"
#include "host.h"

csp::Operation::Operation()
	: m_pProcess()
	, m_finished( false )
{
}

csp::Operation::~Operation()
{
}

int csp::Operation::PushResults( lua::LuaStack & )
{
	// empty by default
	return 0;
}

csp::Process& csp::Operation::ThisProcess() const
{
	CORE_ASSERT( m_pProcess );
	return *m_pProcess;
}

int csp::Operation::DoInit( lua_State* luaState )
{
	lua::LuaState state( luaState );

	m_pProcess = csp::Process::GetProcess( luaState );
	if( m_pProcess == NULL )
	{
		delete this;
		return state.GetStack().Error("not in CSP process");
	}

	lua::LuaStack args( luaState );
	InitError initError;
	if ( !Init( args, initError ) )
	{
		delete this;
		return state.GetStack().ArgError( initError.errorArg, initError.errorMessage );
	}

	m_pProcess->SwitchCurrentOperation( this );
	return state.Yield( 0 );
}

bool csp::Operation::Init( lua::LuaStack &, InitError& )
{
	// empty by default
	return true;
}

bool csp::Operation::IsFinished() const
{
	return m_finished;
}

void csp::Operation::SetFinished( bool finished )
{
	m_finished = finished;
}

csp::WorkResult::Enum csp::Operation::Evaluate( Host& )
{
	// empty by default
	return WorkResult::YIELD;
}

void csp::Operation::DebugCheck( Host& ) const
{
	// empty by default
}

void csp::Operation::DoTerminate( Host& host )
{
	Terminate( host );
}

void csp::Operation::Terminate( Host& )
{
	// empty by default
}

bool csp::Operation::InitError::ArgError( int arg, const char* message )
{
	errorArg = arg;
	errorMessage = message;
	return false;
}

bool csp::Operation::InitError::Error( const char* message )
{
	errorArg = 0;
	errorMessage = message;
	return false;
}


csp::OpSleep::OpSleep()
	: m_seconds()
{
}

bool csp::OpSleep::Init( lua::LuaStack & args, InitError& initError )
{
	if ( !args[1].IsNumber() )
		return initError.ArgError( 1, "seconds expected" );

	m_seconds = args[1].GetNumber();
	return true;
}

csp::WorkResult::Enum csp::OpSleep::Work( Host&, time_t dt )
{
	m_seconds -= dt;
	return m_seconds > 0 ? WorkResult::YIELD : WorkResult::FINISH;
}


namespace operations
{
	int SLEEP( lua_State* luaState );
	int PAR( lua_State* luaState );
	int PARWHILE( lua_State* luaState );
	int ALT( lua_State* luaState );
}

int operations::SLEEP( lua_State* luaState )
{
	csp::OpSleep* pSleep = CORE_NEW csp::OpSleep();
	return pSleep->DoInit( luaState );
}

int operations::PAR( lua_State* luaState )
{
	csp::OpPar* pPar = CORE_NEW csp::OpPar();
	return pPar->DoInit( luaState );
}

int operations::PARWHILE( lua_State* luaState )
{
	csp::OpParWhile* pPar = CORE_NEW csp::OpParWhile();
	return pPar->DoInit( luaState );
}

int operations::ALT( lua_State* luaState )
{
	csp::OpAlt* pAlt = CORE_NEW csp::OpAlt();
	return pAlt->DoInit( luaState );
}

const csp::FunctionRegistration operationDescriptions[] =
{
	  "SLEEP", operations::SLEEP
	, "PAR", operations::PAR
	, "PARWHILE", operations::PARWHILE
	, "ALT", operations::ALT
	, NULL, NULL
};

void csp::RegisterStandardOperations( lua::LuaState & state, lua::LuaStackValue & value )
{
	RegisterFunctions( state, value, operationDescriptions );
}

void csp::UnregisterStandardOperations( lua::LuaState & state, lua::LuaStackValue & value )
{
	UnregisterFunctions( state, value, operationDescriptions );
}
