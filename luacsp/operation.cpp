#include <luacpp/luacpp.h>

#include "operation.h"
#include "op_alt.h"
#include "op_par.h"

#include "process.h"
#include "host.h"
#include "channel.h"

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
	return 0;
}

csp::Process& csp::Operation::ThisProcess() const
{
	CORE_ASSERT( m_pProcess );
	return *m_pProcess;
}

int csp::Operation::Initialize( lua_State* luaState )
{
	lua::LuaState state( luaState );

	m_pProcess = csp::Process::GetProcess( luaState );
	if( m_pProcess == NULL )
	{
		delete this;
		return state.Error("not in CSP process");
	}

	lua::LuaStack args( luaState );
	InitError initError;
	if ( !Init( args, initError ) )
	{
		delete this;
		return state.ArgError( initError.errorArg, initError.errorMessage );
	}

	m_pProcess->SwitchCurrentOperation( this );
	return state.Yield( 0 );
}

bool csp::Operation::Init( lua::LuaStack &, InitError& )
{
	return true;
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
	return WorkResult::YIELD;
}

void csp::Operation::DebugCheck( Host& )
{
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
	int ALT( lua_State* luaState );
}

namespace helpers
{
	int log( lua_State* luaState );
	int time( lua_State* luaState );
	int tick( lua_State* luaState );
}

int helpers::log( lua_State* luaState )
{
	lua::LuaStack args( luaState );

	for( int i = 1; i <= args.NumArgs(); ++i )
	{
		lua::LuaStackValue arg = args[i];
		if( arg.IsNil() )
			lua::Print( "nil" );
		else if( arg.IsBoolean() )
			lua::Print( arg.GetBoolean() ? "true" : "false" );
		else if( arg.IsNumber() )
			lua::Print( "%.5f", arg.GetNumber() );
		else if( arg.IsString() )
			lua::Print( arg.GetString() );

		if( i < args.NumArgs() )
			lua::Print(" ");
	}

	return 0;
}

int helpers::time( lua_State* luaState )
{
	lua::LuaStack stack( luaState );

	csp::Host& host = csp::Host::GetHost( luaState );
	stack.PushNumber( host.Time() );
	return 1;
}

int helpers::tick( lua_State* luaState )
{
	lua::LuaStack stack( luaState );

	csp::Host& host = csp::Host::GetHost( luaState );
	stack.PushInteger( host.Tick() );
	return 1;
}

int operations::SLEEP( lua_State* luaState )
{
	csp::OpSleep* pSleep = CORE_NEW csp::OpSleep();
	return pSleep->Initialize( luaState );
}

int operations::PAR( lua_State* luaState )
{
	csp::OpPar* pPar = CORE_NEW csp::OpPar();
	return pPar->Initialize( luaState );
}

int operations::ALT( lua_State* luaState )
{
	csp::OpAlt* pAlt = CORE_NEW csp::OpAlt();
	return pAlt->Initialize( luaState );
}

const csp::FunctionRegistration helpersDescriptions[] =
{
	  "log", helpers::log
	, "time", helpers::time
	, "tick", helpers::tick
	, NULL, NULL
};

const csp::FunctionRegistration operationDescriptions[] =
{
	  "SLEEP", operations::SLEEP
	, "PAR", operations::PAR
	, "ALT", operations::ALT
	, NULL, NULL
};

void csp::RegisterStandardOperations( lua::LuaState & state, lua::LuaStackValue & value )
{
	RegisterFunctions( state, value, helpersDescriptions );
	RegisterFunctions( state, value, operationDescriptions );
	InitializeChannels( state );
}

void csp::UnregisterStandardOperations( lua::LuaState & state, lua::LuaStackValue & value )
{
	ShutdownChannels( state );
	UnregisterFunctions( state, value, operationDescriptions );
	UnregisterFunctions( state, value, helpersDescriptions );
}
