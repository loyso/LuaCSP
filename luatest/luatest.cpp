#include "luatest.h"

#include <luacpp/luastack.h>
#include <luacpp/luastackvalue.h>

#include <luacsp/csp.h>
#include <luacsp/host.h>

namespace csp
{
	int TestSuite_new( lua_State* luaState );
	int TestSuite_RUN_ALL( lua_State* luaState );

	int TestSuite_checkEqualsInt( lua_State* luaState );

	const csp::FunctionRegistration testSuiteGlobals[] =
	{
		  "new", TestSuite_new
		, "RUN_ALL", TestSuite_RUN_ALL
		, NULL, NULL
	};

	const csp::FunctionRegistration testSuiteFunctions[] =
	{
		"checkEqualsInt", TestSuite_checkEqualsInt
		, NULL, NULL
	};
}

csp::OpTestSuite_RunAll::OpTestSuite_RunAll()
	: m_pClosuresHead(), m_pClosuresTail()
	, m_pCurrentClosure()
{
}

csp::OpTestSuite_RunAll::~OpTestSuite_RunAll()
{
	DeleteClosures( m_pClosuresHead, m_pClosuresTail );

	delete m_pCurrentClosure;
	m_pCurrentClosure = NULL;
}

void csp::OpTestSuite_RunAll::DeleteClosures( TestClosure*& pHead, TestClosure*& pTail )
{
	while( pHead )
	{
		TestClosure* pNext = pHead->pNext;
		delete pHead;
		pHead = pNext;
	}

	pTail = NULL;
}

void csp::OpTestSuite_RunAll::UnrefClosure( TestClosure* pClosure )
{
	if( pClosure->refKey != lua::LUA_NO_REF )
	{
		ThisProcess().LuaThread().GetStack().UnrefInRegistry( pClosure->refKey );
		pClosure->refKey = lua::LUA_NO_REF;
	}
}

void csp::OpTestSuite_RunAll::UnrefClosures( TestClosure* pHead )
{
	for( TestClosure* pClosure = pHead; pClosure; pClosure = pClosure->pNext )
		UnrefClosure( pClosure );
}

void csp::OpTestSuite_RunAll::Terminate( Host& host )
{
	if( m_pCurrentClosure )
		m_pCurrentClosure->process.Terminate( host );

	UnrefClosures( m_pClosuresHead );
	if( m_pCurrentClosure )
		UnrefClosure( m_pCurrentClosure );
}

csp::WorkResult::Enum csp::OpTestSuite_RunAll::Work( Host& host, CspTime_t dt )
{
	if( m_pCurrentClosure )
	{
		if( m_pCurrentClosure->process.IsRunning() )
			m_pCurrentClosure->process.Work( host, dt );
	}

	return IsFinished();
}

csp::WorkResult::Enum csp::OpTestSuite_RunAll::Evaluate( Host& host )
{
	if( m_pCurrentClosure )
	{
		if( m_pCurrentClosure->process.IsRunning() )
			return WorkResult::YIELD;
		else
		{
			if( m_pCurrentClosure->refKey != lua::LUA_NO_REF )
				UnrefClosure( m_pCurrentClosure );

			delete m_pCurrentClosure;
			m_pCurrentClosure = NULL;
		}
	}

	if( m_pCurrentClosure == NULL && m_pClosuresHead )
	{		
		TestClosure* pClosure = ListPopFromHead( m_pClosuresHead, m_pClosuresTail );
		CORE_ASSERT( pClosure );

		lua::Print( "Running test %s.%s...\n", pClosure->suiteName, pClosure->functionName );

		m_pCurrentClosure = pClosure;

		pClosure->process.StartEvaluation( host, 0 );
	}

	return IsFinished();
}

csp::WorkResult::Enum csp::OpTestSuite_RunAll::IsFinished() const
{
	return m_pCurrentClosure == NULL && m_pClosuresHead == NULL ? WorkResult::FINISH : WorkResult::YIELD;
}

void csp::OpTestSuite_RunAll::ListAddToTail( TestClosure*& pHead, TestClosure*& pTail, TestClosure& node )
{
	if( pTail )
		pTail->pNext = &node;
	if( pHead == NULL )
		pHead = &node;

	pTail = &node;
	node.pNext = NULL;
}

csp::OpTestSuite_RunAll::TestClosure* csp::OpTestSuite_RunAll::ListPopFromHead( TestClosure*& pHead, TestClosure*& pTail )
{
	if( pHead == NULL )
		return NULL;

	TestClosure* pNode = pHead;
	pHead = pHead->pNext;
	if( pNode == pTail )
		pTail = NULL;

	pNode->pNext = NULL;
	return pNode;
}

bool csp::OpTestSuite_RunAll::Init( lua::LuaStack& args, InitError& initError )
{
	lua::LuaStackValue globals = args.PushGlobalTable();

	for ( lua::LuaStackTableIterator it( globals ); it; it.Next() )
	{
		lua::LuaStackValue name = it.Key();
		lua::LuaStackValue value = it.Value();

		if ( name.IsString() && value.IsTable() && CspHasMetatable( args.InternalState(), value, testSuiteFunctions ) )
		{
			InitTestSuite( args, initError, name.GetString(), value );
		}
	}

	return true;
}

void csp::OpTestSuite_RunAll::InitTestSuite( lua::LuaStack& stack, InitError& initError, const char* suiteName, lua::LuaStackValue& suiteEnv )
{
	for ( lua::LuaStackTableIterator it( suiteEnv ); it; it.Next() )
	{
		lua::LuaStackValue name = it.Key();
		lua::LuaStackValue value = it.Value();

		if ( name.IsString() && value.IsFunction() )
			InitTest( stack, initError, suiteName, name.GetString(), value );
	}

	return InitTestSuiteEnv( stack, suiteName, suiteEnv, suiteEnv );
}

void csp::OpTestSuite_RunAll::InitTestSuiteEnv( lua::LuaStack& stack, const char* suiteName, lua::LuaStackValue& suiteEnv, lua::LuaStackValue& table )
{
	for ( lua::LuaStackTableIterator it( table ); it; it.Next() )
	{
		lua::LuaStackValue name = it.Key();
		lua::LuaStackValue value = it.Value();

		if ( name.IsString() )
		{
			if( value.IsFunction() )
				value.SetClosureEnv( suiteEnv );
			else if( value.IsTable() )
				InitTestSuiteEnv( stack, suiteName, suiteEnv, value );
		}
	}
}

void csp::OpTestSuite_RunAll::InitTest( lua::LuaStack& stack, InitError&, const char* suiteName, const char* functionName, lua::LuaStackValue& function )
{
	TestClosure* pClosure = CORE_NEW TestClosure();

	lua::LuaState thread = stack.NewThread();
	function.PushValue();
	stack.XMove( thread.GetStack(), 1 );

	pClosure->process.SetLuaThread( thread );
	pClosure->process.SetParentProcess( ThisProcess() );
	pClosure->refKey = stack.RefInRegistry();
	pClosure->suiteName = suiteName;
	pClosure->functionName = functionName;

	ListAddToTail( m_pClosuresHead, m_pClosuresTail, *pClosure );
}


int csp::TestSuite_new( lua_State* luaState )
{
	lua::LuaStack stack( luaState );
	stack.PushTable();
	CspSetMetatable( luaState, stack.GetTopValue(), testSuiteFunctions );
	return 1;
}

int csp::TestSuite_RUN_ALL( lua_State* luaState )
{
	OpTestSuite_RunAll* pRunAll = CORE_NEW OpTestSuite_RunAll();
	return pRunAll->DoInit( luaState );
}

int csp::TestSuite_checkEqualsInt( lua_State* luaState )
{
	lua::LuaStack args( luaState );
	if( !args[1].IsNumber() )
		return args[1].ArgError( "number expected" );

	if( !args[2].IsNumber() )
		return args[2].ArgError( "number expected" );

	if( !args[3].IsString() )
		return args[3].ArgError( "error message string expected" );

	int expected = args[1].GetInteger();
	int value = args[2].GetInteger();
	if( expected != value )
	{
		lua::Print( "ERROR! %s: expected=%d value=%d\n", args[3].GetString(), expected, value );
	}

	return 0;
}

void csp::InitTests( lua::LuaState& state )
{
	lua::LuaStack& stack = state.GetStack();
	lua::LuaStackValue globals = stack.PushGlobalTable();

	InitializeCspObjectEnv( state, "TestSuite", testSuiteGlobals, testSuiteFunctions, globals );

	stack.Pop(1);
}

void csp::ShutdownTests( lua::LuaState& state )
{
	ShutdownCspObject( state, "TestSuite", testSuiteGlobals, testSuiteFunctions );
}
