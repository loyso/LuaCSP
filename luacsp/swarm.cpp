#include "swarm.h"

#include <luacpp/luacpp.h>
#include <luacpp/luastackvalue.h>

#include "host.h"

namespace csp
{
	int swarm( lua_State* luaState );

	int Swarm_MAIN( lua_State* luaState );
	int Swarm_go( lua_State* luaState );

	const csp::FunctionRegistration swarmGlobals[] =
	{
		"Swarm", csp::swarm
		, NULL, NULL
	};

	const csp::FunctionRegistration swarmFunctions[] =
	{
		"__gc", csp::GcObject_Gc
		, "MAIN", csp::Swarm_MAIN
		, "go", csp::Swarm_go
		, NULL, NULL
	};

	const int SWARM_MAX_CLOSURES = 1024;
}


csp::Swarm::Swarm()
	: m_pOpMain()
{
}

csp::Swarm::~Swarm()
{
}

int csp::Swarm::DoGo( lua::LuaStack& args )
{
	if( m_pOpMain == NULL )
		return args.Error("MAIN isn't started for this Swarm.");	
	return m_pOpMain->Go( args );
}

void csp::Swarm::InitMain( OpSwarmMain& opMain )
{
	m_pOpMain = &opMain;
}

void csp::Swarm::Terminate()
{
	m_pOpMain = NULL;
}


csp::OpSwarmMain::OpSwarmMain()
	: m_pSwarm()
	, m_closures()
	, m_numClosures( 0 )
	, m_closureToRun( 0 )
{
}

csp::OpSwarmMain::~OpSwarmMain()
{
	CORE_ASSERT( m_pSwarm == NULL );

	delete[] m_closures;
	m_closures = NULL;
}

void csp::OpSwarmMain::UnrefClosures()
{
	for( int i = 0; i < m_numClosures; ++i )
	{
		SwarmClosure* pClosure = m_closures[ i ];
		CORE_ASSERT( pClosure );
		if( pClosure->refKey != lua::LUA_NO_REF )
		{
			ThisProcess().LuaThread().GetStack().UnrefInRegistry( pClosure->refKey );
			pClosure->refKey = lua::LUA_NO_REF;

			delete pClosure;
			pClosure = NULL;
		}
	}
}

void csp::OpSwarmMain::Terminate( Host& host )
{
	for( int i = 0; i < m_closureToRun; ++i )
	{
		SwarmClosure* pClosure = m_closures[ i ];
		CORE_ASSERT( pClosure );
		pClosure->process.Terminate( host );
	}
	UnrefClosures();

	CORE_ASSERT( m_pSwarm );
	m_pSwarm->Terminate();
	m_pSwarm = NULL;
}

csp::WorkResult::Enum csp::OpSwarmMain::Work( Host& host, time_t dt )
{
	for( int i = 0; i < m_closureToRun; ++i )
	{
		SwarmClosure* pClosure = m_closures[ i ];
		CORE_ASSERT( pClosure );
		if( pClosure->process.IsRunning() )
			pClosure->process.Work( host, dt );
	}
	return WorkResult::YIELD; // never ends.
}

void csp::OpSwarmMain::CheckFinished()
{
	CORE_ASSERT( m_closureToRun <= m_numClosures );

	for( int i = 0; i < m_closureToRun; ++i )
	{
		SwarmClosure* pClosure = m_closures[ i ];
		CORE_ASSERT( pClosure );

		if( !pClosure->process.IsRunning() && pClosure->refKey != lua::LUA_NO_REF )
		{
			// erase element in unordered vector.
			m_closures[ i ] = m_closures[ m_closureToRun-1 ];
			m_closures[ m_closureToRun-1 ] = m_closures[ m_numClosures-1 ];
			m_closures[ m_numClosures-1 ] = NULL;

			--m_closureToRun;
			--m_numClosures;

			ThisProcess().LuaThread().GetStack().UnrefInRegistry( pClosure->refKey );
			pClosure->refKey = lua::LUA_NO_REF;

			delete pClosure;
			pClosure = NULL;
		}
	}
}

int csp::OpSwarmMain::Go( lua::LuaStack& args )
{
	int numClosures = 0;
	for( int i = 2; i <= args.NumArgs(); ++i )
	{
		lua::LuaStackValue arg = args[i];
		if ( arg.IsFunction() )
			++numClosures;
		else
			return args.ArgError( i, "function closure expected" );
	}

	if( m_numClosures + numClosures > SWARM_MAX_CLOSURES )
		return args.Error( "max swarm closure limit achieved." );

	for( int i = 2; i <= args.NumArgs(); ++i )
	{
		lua::LuaStackValue arg = args[i];
		SwarmClosure* pClosure = CORE_NEW SwarmClosure();

		lua::LuaState thread = args.NewThread();
		arg.PushValue();
		args.XMove( thread.GetStack(), 1 );

		pClosure->process.SetLuaThread( thread );
		pClosure->process.SetParentProcess( ThisProcess() );
		pClosure->refKey = args.RefInRegistry();

		m_closures[ m_numClosures++ ] = pClosure;
	}

	Host& host = Host::GetHost( args.InternalState() );
	host.PushEvalStep( ThisProcess() );

	return 0;
}

csp::WorkResult::Enum csp::OpSwarmMain::Evaluate( Host& host )
{
	if( m_closureToRun < m_numClosures )
	{
		SwarmClosure* pClosure = m_closures[ m_closureToRun++ ];
		CORE_ASSERT( pClosure );

		if( m_closureToRun < m_numClosures )
			host.PushEvalStep( ThisProcess() );

		pClosure->process.StartEvaluation( host, 0 );
	}

	CheckFinished();
	return WorkResult::YIELD;
}

bool csp::OpSwarmMain::Init( lua::LuaStack& args, InitError& initError )
{
	lua::LuaStackValue swarmArg = args[1];
	if( !IsSwarmArg( swarmArg ) )
		return initError.ArgError( 1, "Swarm object expected" );

	Swarm* pSwarm = GetSwarmArg( swarmArg );
	if( pSwarm == NULL )
		return initError.ArgError( 1, "Swarm pointer expected" );

	m_pSwarm = pSwarm;
	m_pSwarm->InitMain( *this );

	m_closures = CORE_NEW SwarmClosure*[ SWARM_MAX_CLOSURES ];
	for( int i = 0; i < SWARM_MAX_CLOSURES; ++i )
		m_closures[ i ] = NULL;

	return true;
}


void csp::PushSwarm( lua_State* luaState, Swarm& swarm )
{
	PushGcObject( luaState, swarm, swarmFunctions );
}

bool csp::IsSwarmArg( lua::LuaStackValue const& value )
{
	return value.IsUserData(); // TODO: add more checks.
}

csp::Swarm* csp::GetSwarmArg( lua::LuaStackValue const& value )
{
	if( !value.IsUserData() )
		return NULL;

	GcObject** pGcObject = (GcObject**)value.GetUserData();
	Swarm* pSwarm = static_cast< Swarm* >( *pGcObject ); // TODO: add more type checks.
	return pSwarm;
}

int csp::swarm( lua_State* luaState )
{
	csp::Swarm* pSwarm = CORE_NEW csp::Swarm();
	csp::PushSwarm( luaState, *pSwarm );
	return 1;
}

int csp::Swarm_MAIN( lua_State* luaState )
{
	OpSwarmMain* pMain = CORE_NEW OpSwarmMain();
	return pMain->DoInit( luaState );
}

int csp::Swarm_go( lua_State* luaState )
{
	lua::LuaStack args( luaState );
	lua::LuaStackValue swarmArg = args[1];
	if( !IsSwarmArg( swarmArg ) )
		return swarmArg.ArgError( "Swarm object expected" );

	Swarm* pSwarm = GetSwarmArg( swarmArg );
	if( pSwarm == NULL )
		return swarmArg.ArgError( "Swarm pointer expected" );

	return pSwarm->DoGo( args );
}

void csp::InitializeSwarms( lua::LuaState& state )
{
	InitializeCspObject( state, swarmFunctions, swarmGlobals );
}

void csp::ShutdownSwarms( lua::LuaState& state )
{
	ShutdownCspObject( state, swarmFunctions, swarmGlobals );
}
