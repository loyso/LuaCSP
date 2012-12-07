#include <luacpp/luacpp.h>

#include "process.h"
#include "operation.h"
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


csp::OpSleep::OpSleep()
	: m_seconds()
{
}

bool csp::OpSleep::Init( lua::LuaStack & args, InitError& initError )
{
	if ( !args[1].IsNumber() )
		return initError.ArgError( 1, "seconds expected" );

	m_seconds = args[1].GetNumber();
	SetFinished( m_seconds <= 0 );
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

const csp::FunctionRegistration operationDescriptions[] =
{
	  "log", helpers::log
	, "SLEEP", operations::SLEEP
	, "PAR", operations::PAR
	, "ALT", operations::ALT
	, NULL, NULL
};

void csp::RegisterStandardOperations( lua::LuaState & state, lua::LuaStackValue & value )
{
	RegisterFunctions( state, value, operationDescriptions );
	InitializeChannels( state );
}

void csp::UnregisterStandardOperations( lua::LuaState & state, lua::LuaStackValue & value )
{
	ShutdownChannels( state );
	UnregisterFunctions( state, value, operationDescriptions );
}

csp::OpPar::OpPar()
	: m_closures()
	, m_numClosures()
	, m_closureToRun()
{
}

csp::OpPar::~OpPar()
{
	for( int i = 0; i < m_numClosures; ++i )
	{
		CORE_ASSERT( m_closures[i].refKey == lua::LUA_NO_REF );
	}

	delete[] m_closures;
	m_closures = NULL;
}

bool csp::OpPar::Init( lua::LuaStack& args, InitError& initError )
{
	m_numClosures = 0;
	for( int i = 1; i <= args.NumArgs(); ++i )
	{
		lua::LuaStackValue arg = args[i];
		if ( arg.IsFunction() )
			++m_numClosures;
		else
			return initError.ArgError( i, "function closure expected" );
	}

	m_closures = CORE_NEW ParClosure[ m_numClosures ];

	for( int i = 1; i <= args.NumArgs(); ++i )
	{
		lua::LuaStackValue arg = args[i];
		ParClosure& closure = m_closures[ i-1 ];

		lua::LuaState thread = args.NewThread();
		arg.PushValue();
		args.XMove( thread.GetStack(), 1 );

		closure.process.SetLuaThread( thread );
		closure.process.SetParentProcess( ThisProcess() );
		closure.refKey = args.RefInRegistry();
	}

	return true;
}

csp::WorkResult::Enum csp::OpPar::Evaluate( Host& host )
{
	bool finished = true;

	if( m_closureToRun < m_numClosures )
	{
		ParClosure& closure = m_closures[ m_closureToRun++ ];

		if( m_closureToRun < m_numClosures )
		{
			host.PushEvalStep( ThisProcess() );
			finished = false;
		}

		closure.process.Evaluate( host );
	}

	if ( !CheckFinished() )
		finished = false;

	if (finished && !IsFinished())
		SetFinished( true );

	return IsFinished() ? WorkResult::FINISH : WorkResult::YIELD;
}

csp::WorkResult::Enum csp::OpPar::Work( Host& host, time_t dt )
{
	for( int i = 0; i < m_closureToRun; ++i )
	{
		Process& process = m_closures[ i ].process;
		if( process.IsRunning() )
			process.Work( host, dt );
	}

	return IsFinished() ? WorkResult::FINISH : WorkResult::YIELD;
}

bool csp::OpPar::CheckFinished()
{
	bool finished = true;
	for( int i = 0; i < m_closureToRun; ++i )
	{
		Process& process = m_closures[ i ].process;
		if( process.IsRunning() )
		{
			finished = false;
		}
		else if( m_closures[ i ].refKey != lua::LUA_NO_REF )
		{
			ThisProcess().LuaThread().GetStack().UnrefInRegistry( m_closures[ i ].refKey );
			m_closures[ i ].refKey = lua::LUA_NO_REF;
		}
	}
	return finished;
}


csp::OpAlt::OpAlt()
	: m_cases()
	, m_numCases( 0 )
	, m_caseTriggered( -1 )
	, m_nilCase( -1 )
	, m_processRefKey( lua::LUA_NO_REF )
{
}

csp::OpAlt::~OpAlt()
{
	CORE_ASSERT( m_processRefKey == lua::LUA_NO_REF );

	delete m_cases;
	m_cases = NULL;
	m_numCases = 0;
}

bool csp::OpAlt::Init( lua::LuaStack& args, InitError& initError )
{
	if( !CheckArgs( args, initError ) )
		return false;

	InitCases( args );
	return true;
}

bool csp::OpAlt::CheckArgs( lua::LuaStack& args, InitError& initError ) const
{
	if( (args.NumArgs() & 1) != 0 )
		return initError.Error( "even number of arguments required. (guard+closure) pairs required" );

	bool nilCase = false;
	for( int i = 1; i <= args.NumArgs(); i += 2 )
	{
		lua::LuaStackValue guard = args[i];
		lua::LuaStackValue closure = args[i+1];

		if( !(IsChannelArg(guard) && GetChannelArg(guard )!= NULL) 
			&& !guard.IsNumber() 
			&& !guard.IsNil() )
		{
			return initError.ArgError( i, "channel, number or nil required as a guard" );
		}

		if( guard.IsNil() )
		{
			if( nilCase )
				return initError.ArgError( i, "there must be just one nil case" );
			nilCase = true;
		}

		if( !closure.IsFunction() )
			return initError.ArgError( i+1, "closure required" );
	}

	return true;
}

void csp::OpAlt::InitCases( lua::LuaStack& args )
{
	m_numCases = args.NumArgs()/2;
	m_cases = CORE_NEW AltCase [ m_numCases ];

	int initCase = 0;
	for( int i = 1; i <= args.NumArgs(); i += 2 )
	{
		lua::LuaStackValue guard = args[i];
		lua::LuaStackValue closure = args[i+1];

		closure.PushValue();
		m_cases[ initCase ].m_closureRefKey = args.RefInRegistry();

		if( IsChannelArg(guard) )
		{
			m_cases[ initCase ].m_pChannel = GetChannelArg( guard );
			CORE_ASSERT( m_cases[ initCase ].m_pChannel != NULL );
			guard.PushValue();
			m_cases[ initCase ].m_channelRefKey = args.RefInRegistry();
		}
		else if( guard.IsNumber() )
		{
			m_cases[ initCase ].m_time = guard.GetNumber();
		}
		else if( guard.IsNil() )
		{
			CORE_ASSERT( m_nilCase == -1 );
			m_nilCase = initCase;
		}

		++initCase;
	}
}

csp::WorkResult::Enum csp::OpAlt::Evaluate( Host& host )
{
	if( m_caseTriggered >= 0 && m_caseTriggered < m_numCases )
	{
		if( m_process.IsRunning() )
		{
			return WorkResult::YIELD;
		}
		else if( m_processRefKey != lua::LUA_NO_REF )
		{
			ThisProcess().LuaThread().GetStack().UnrefInRegistry( m_processRefKey );
			m_processRefKey = lua::LUA_NO_REF;
			return WorkResult::FINISH;
		}
	}

	//TODO: work here.

	return WorkResult::YIELD;
}

csp::WorkResult::Enum csp::OpAlt::Work( Host& host, time_t dt )
{
	if( m_caseTriggered >= 0 && m_caseTriggered < m_numCases )
	{
		if( m_process.IsRunning() )
			m_process.Work( host, dt );
	}

	return IsFinished() ? WorkResult::FINISH : WorkResult::YIELD;
}

csp::OpAlt::AltCase::AltCase()
	: m_closureRefKey( lua::LUA_NO_REF )
	, m_pChannel()
	, m_channelRefKey( lua::LUA_NO_REF )
	, m_time( -1 )
{
}
