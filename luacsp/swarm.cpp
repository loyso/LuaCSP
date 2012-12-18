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
	, m_pClosuresHead(), m_pClosuresTail()
	, m_pClosuresToRunHead(), m_pClosuresToRunTail()
{
}

csp::OpSwarmMain::~OpSwarmMain()
{
	CORE_ASSERT( m_pSwarm == NULL );

	DeleteClosures( m_pClosuresHead, m_pClosuresTail );
	DeleteClosures( m_pClosuresToRunHead, m_pClosuresToRunTail );
}

void csp::OpSwarmMain::DeleteClosures( SwarmClosure*& pHead, SwarmClosure*& pTail )
{
	while( pHead )
	{
		SwarmClosure* pNext = pHead->pNext;
		delete pHead;
		pHead = pNext;
	}

	pTail = NULL;
}

void csp::OpSwarmMain::UnrefClosures( SwarmClosure* pHead )
{
	for( SwarmClosure* pClosure = pHead; pClosure; pClosure = pClosure->pNext )
	{
		if( pClosure->refKey != lua::LUA_NO_REF )
		{
			ThisProcess().LuaThread().GetStack().UnrefInRegistry( pClosure->refKey );
			pClosure->refKey = lua::LUA_NO_REF;
		}
	}
}

void csp::OpSwarmMain::Terminate( Host& host )
{
	for( SwarmClosure* pClosure = m_pClosuresHead; pClosure; pClosure = pClosure->pNext )
		pClosure->process.Terminate( host );

	UnrefClosures( m_pClosuresHead );
	UnrefClosures( m_pClosuresToRunHead );

	CORE_ASSERT( m_pSwarm );
	m_pSwarm->Terminate();
	m_pSwarm = NULL;
}

csp::WorkResult::Enum csp::OpSwarmMain::Work( Host& host, CspTime_t dt )
{
	for( SwarmClosure* pClosure = m_pClosuresHead; pClosure; pClosure = pClosure->pNext )
	{
		if( pClosure->process.IsRunning() )
			pClosure->process.Work( host, dt );
	}
	return WorkResult::YIELD; // never ends.
}

void csp::OpSwarmMain::CheckFinished()
{
	for( SwarmClosure* pClosure = m_pClosuresHead, *pPrev = NULL; pClosure; )
	{
		SwarmClosure* pNext = pClosure->pNext;

		if( !pClosure->process.IsRunning() && pClosure->refKey != lua::LUA_NO_REF )
		{
			ThisProcess().LuaThread().GetStack().UnrefInRegistry( pClosure->refKey );
			pClosure->refKey = lua::LUA_NO_REF;

			if( pClosure == m_pClosuresHead )
			{
				m_pClosuresHead = pNext;
			}
			else
			{
				CORE_ASSERT( pPrev );
				pPrev->pNext = pNext;
			}

			if( pClosure == m_pClosuresTail )
				m_pClosuresTail = pPrev;

			delete pClosure;
		}
		else
			pPrev = pClosure;

		pClosure = pNext;
	}
}

int csp::OpSwarmMain::Go( lua::LuaStack& args )
{
	for( int i = 2; i <= args.NumArgs(); ++i )
	{
		lua::LuaStackValue arg = args[i];
		if ( !arg.IsFunction() )
			return args.ArgError( i, "function closure expected" );
	}

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

		ListAddToTail( m_pClosuresToRunHead, m_pClosuresToRunTail, *pClosure );
	}

	Host& host = Host::GetHost( args.InternalState() );
	host.PushEvalStep( ThisProcess() );

	return 0;
}

csp::WorkResult::Enum csp::OpSwarmMain::Evaluate( Host& host )
{
	if( m_pClosuresToRunHead )
	{		
		SwarmClosure* pClosure = ListPopFromHead( m_pClosuresToRunHead, m_pClosuresToRunTail );
		CORE_ASSERT( pClosure );

		ListAddToTail( m_pClosuresHead, m_pClosuresTail, *pClosure );

		if( m_pClosuresToRunHead )
			host.PushEvalStep( ThisProcess() );

		pClosure->process.StartEvaluation( host, 0 );
	}

	CheckFinished();
	return WorkResult::YIELD;
}

void csp::OpSwarmMain::ListAddToTail( SwarmClosure*& pHead, SwarmClosure*& pTail, SwarmClosure& node )
{
	if( pTail )
		pTail->pNext = &node;
	if( pHead == NULL )
		pHead = &node;
	
	pTail = &node;
	node.pNext = NULL;
}

csp::OpSwarmMain::SwarmClosure* csp::OpSwarmMain::ListPopFromHead( SwarmClosure*& pHead, SwarmClosure*& pTail )
{
	if( pHead == NULL )
		return NULL;

	SwarmClosure* pNode = pHead;
	pHead = pHead->pNext;
	if( pNode == pTail )
		pTail = NULL;

	pNode->pNext = NULL;
	return pNode;
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
