/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#include "luatest.h"

#include <luacpp/luastack.h>
#include <luacpp/luastackvalue.h>

#include <luacsp/csp.h>
#include <luacsp/host.h>

namespace csp
{
	int TestSuite_new( lua_State* luaState );
	int TestSuite_RUN_ALL( lua_State* luaState );

	int TestSuite_checkEquals( lua_State* luaState );
	int TestSuite_checkEqualsInt( lua_State* luaState );
	int TestSuite_checkEqualsFloat( lua_State* luaState );
	int TestSuite_checkEqualsArray( lua_State* luaState );

	const csp::FunctionRegistration testSuiteGlobals[] =
	{
		  "new", TestSuite_new
		, "RUN_ALL", TestSuite_RUN_ALL
		, NULL, NULL
	};

	const csp::FunctionRegistration testSuiteFunctions[] =
	{
		  "checkEquals", TestSuite_checkEquals
		, "checkEqualsInt", TestSuite_checkEqualsInt
		, "checkEqualsFloat", TestSuite_checkEqualsFloat
		, "checkEqualsArray", TestSuite_checkEqualsArray
		, NULL, NULL
	};

	struct ArgsForCheck
	{
		ArgsForCheck()
			: pTestContext()
		{
		}

		TestRunContext* pTestContext;
		lua::LuaStackValue errMsg;
		lua::LuaStackValue expected;
		lua::LuaStackValue actual;
	};

	int GetArgsForCheck( lua::LuaStack& args, ArgsForCheck& argsForCheck );
	void PrintTestError( ArgsForCheck const& argsForCheck );
}


csp::OpTestSuite_RunAll::OpTestSuite_RunAll()
	: m_pClosuresHead(), m_pClosuresTail()
	, m_pCurrentClosure()
	, m_numTests()
	, m_numTestsFailed()
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

			if( m_pCurrentClosure->numChecksFailed > 0 )
				lua::Print( "FAILED!\n" );
			else
				lua::Print( "OK!\n" );

			delete m_pCurrentClosure;
			m_pCurrentClosure = NULL;
		}
	}

	if( m_pCurrentClosure == NULL && m_pClosuresHead )
	{		
		TestClosure* pClosure = ListPopFromHead( m_pClosuresHead, m_pClosuresTail );
		CORE_ASSERT( pClosure );

		lua::Print( "Running test %s.%s... ", pClosure->suiteName, pClosure->functionName );

		m_pCurrentClosure = pClosure;

		pClosure->process.LuaThread().GetTopValue().PushClosureEnv();
		pClosure->process.StartEvaluation( host, 1 );
	}

	WorkResult::Enum isFinished = IsFinished();
	if( isFinished == WorkResult::FINISH )
	{
		if( m_numTestsFailed == 0 )
			lua::Print( "ALL TESTS PASSED! [%d/%d]\n", m_numTests, m_numTests );
		else
			lua::Print( "SOME TESTS FAILED! [%d/%d]\n", m_numTestsFailed, m_numTests );
	}

	return isFinished;
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
	pClosure->numChecksFailed = 0;

	++m_numTests;

	ListAddToTail( m_pClosuresHead, m_pClosuresTail, *pClosure );
}

void csp::OpTestSuite_RunAll::SetCheckFailed()
{
	CORE_ASSERT( m_pCurrentClosure );

	if( m_pCurrentClosure->numChecksFailed == 0 )
		m_numTestsFailed++;

	m_pCurrentClosure->numChecksFailed++;
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


csp::TestRunContext* csp::TestRunContext::GetTestContext( lua_State* luaState )
{
	lua::LuaStack stack( luaState );
	lua::LuaStackValue mainThread = stack.PushMainThread();
	if( !mainThread.IsThread() )
		return NULL;

	lua_State* pThread = mainThread.GetThread();
	CORE_ASSERT( pThread );

	Process* pProcess = Process::GetProcess( pThread );
	if( pProcess == NULL )
		return NULL;

	if( !pProcess->IsInOperation() )
		return NULL;

	return static_cast< TestRunContext* >( &pProcess->CurrentOperation() ); //TODO: perform some runtime check.
}

int csp::GetArgsForCheck( lua::LuaStack& args, ArgsForCheck& argsForCheck )
{
	argsForCheck.pTestContext = TestRunContext::GetTestContext( args.InternalState() );
	if( argsForCheck.pTestContext == NULL )
		return args.Error( "Not in test run context.");

	argsForCheck.errMsg = args[1];
	argsForCheck.expected = args[2];
	argsForCheck.actual = args[3];

	if( !argsForCheck.errMsg.IsString() )
		return argsForCheck.errMsg.ArgError( "error message string expected" );

	return 0;
}

void csp::PrintTestError( ArgsForCheck const& argsForCheck )
{
	lua::Print( "ERROR! %s: expected=", argsForCheck.errMsg.GetString() );
	lua::PrintStackValue( argsForCheck.expected );
	lua::Print( " actual=" );
	lua::PrintStackValue( argsForCheck.actual );
	lua::Print( "\n" );
	
	CORE_ASSERT( argsForCheck.pTestContext );
	argsForCheck.pTestContext->SetCheckFailed();
}

int csp::TestSuite_checkEquals( lua_State* luaState )
{
	lua::LuaStack args( luaState );

	ArgsForCheck argsForCheck;
	if( GetArgsForCheck( args, argsForCheck ) )
		return 0;

	if( !argsForCheck.expected.IsRawEqual( argsForCheck.actual ) )
		PrintTestError( argsForCheck );

	return 0;
}

int csp::TestSuite_checkEqualsInt( lua_State* luaState )
{
	lua::LuaStack args( luaState );

	ArgsForCheck argsForCheck;
	if( GetArgsForCheck( args, argsForCheck ) )
		return 0;

	if( !argsForCheck.expected.IsNumber() )
		return argsForCheck.expected.ArgError( "number expected" );

	if( !argsForCheck.actual.IsNumber() )
		return argsForCheck.actual.ArgError( "number expected" );

	int expectedInt = argsForCheck.expected.GetInteger();
	int valueInt = argsForCheck.actual.GetInteger();
	if( expectedInt != valueInt )
	{
		lua::Print( "ERROR! %s: expected=%d value=%d\n", argsForCheck.errMsg.GetString(), expectedInt, valueInt );
		argsForCheck.pTestContext->SetCheckFailed();
	}

	return 0;
}

int csp::TestSuite_checkEqualsFloat( lua_State* luaState )
{
	lua::LuaStack args( luaState );

	ArgsForCheck argsForCheck;
	if( GetArgsForCheck( args, argsForCheck ) )
		return 0;

	lua::LuaStackValue delta = args[4];

	if( !argsForCheck.expected.IsNumber() )
		return argsForCheck.expected.ArgError( "number expected" );

	if( !argsForCheck.actual.IsNumber() )
		return argsForCheck.actual.ArgError( "number expected" );

	if( !delta.IsNumber() )
		return delta.ArgError( "number expected" );

	lua::LuaNumber_t expectedFloat = argsForCheck.expected.GetNumber();
	lua::LuaNumber_t valueFloat = argsForCheck.actual.GetNumber();
	lua::LuaNumber_t deltaFloat = delta.GetNumber();
	
	lua::LuaNumber_t d = expectedFloat - valueFloat;
	if( d < 0 )
		d = -d;

	if( d > deltaFloat )
	{
		lua::Print( "ERROR! %s: expected=%g value=%g delta=%g\n", argsForCheck.errMsg.GetString(), expectedFloat, valueFloat, deltaFloat );
		argsForCheck.pTestContext->SetCheckFailed();
	}

	return 0;
}

int csp::TestSuite_checkEqualsArray( lua_State* luaState )
{
	lua::LuaStack args( luaState );

	ArgsForCheck argsForCheck;
	if( GetArgsForCheck( args, argsForCheck ) )
		return 0;
	
	if( !argsForCheck.expected.IsTable() )
		return argsForCheck.expected.ArgError( "table expected" );

	if( !argsForCheck.actual.IsTable() )
		return argsForCheck.actual.ArgError( "table expected" );

	const int expectedLen = argsForCheck.expected.RawLength();
	const int checkLen = argsForCheck.actual.RawLength();

	bool err = false;
	if( expectedLen != checkLen )
	{
		lua::Print( "ERROR! %s: expected array length=%d actual array length=%d\n", argsForCheck.errMsg.GetString(), expectedLen, checkLen );
		err = true;
	}

	const int len = expectedLen > checkLen ? checkLen : expectedLen;
	for( int i = 1; i <= len; ++i )
	{
		ArgsForCheck element( argsForCheck );
		element.expected = argsForCheck.expected.PushRawGetIndex( i );
		element.actual = argsForCheck.actual.PushRawGetIndex( i );
		
		if( !element.expected.IsRawEqual( element.actual ) )
		{
			PrintTestError( element );
			err = true;
		}

		args.Pop( 2 );
	}

	if( err )
	{
		lua::Print( "expected array = " );
		lua::PrintStackArray( argsForCheck.expected );
		lua::Print( "\nactual array   = " );
		lua::PrintStackArray( argsForCheck.actual );
		lua::Print( "\n" );
		argsForCheck.pTestContext->SetCheckFailed();
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
