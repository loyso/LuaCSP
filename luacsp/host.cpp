
#include "host.h"
#include "operation.h"

namespace csp
{
	static const int CHANNEL_STACK_SIZE = 256;
}

csp::Host::Host(const lua::LuaState& luaState)
    : m_luaState(luaState)
	, m_mainProcess()
	, m_evalStepsStack()
	, m_evalStepsStackTop( -1 )
{
	m_evalStepsStack = CORE_NEW Process*[ CHANNEL_STACK_SIZE ];
	
	for( int i = 0; i < CHANNEL_STACK_SIZE; ++i )
		m_evalStepsStack[i] = NULL;
}

csp::Host::~Host()
{
	delete[] m_evalStepsStack;
	m_evalStepsStack = NULL;
}

lua::LuaState& csp::Host::LuaState()
{
    return m_luaState;
}

void csp::Host::Initialize()
{
	lua::LuaStackValue globals = m_luaState.GetStack().PushGlobalTable();
	RegisterStandardOperations( m_luaState, globals );
	m_luaState.GetStack().Pop(1);
}

void csp::Host::Shutdown()
{
	lua::LuaStackValue globals = m_luaState.GetStack().PushGlobalTable();
	UnregisterStandardOperations( m_luaState, globals );
	m_luaState.GetStack().Pop(1);
}

csp::WorkResult::Enum csp::Host::Main()
{
	m_mainProcess.SetLuaThread( m_luaState );

	lua::LuaStackValue stackValue = m_mainProcess.LuaThread().GetStack().PushGlobalValue("main");
	if ( !stackValue.IsFunction() || stackValue.IsCFunction() )
		return WorkResult::FINISH;
	
	PushEvalStep( m_mainProcess );
	Evaluate();
	return WorkResult::YIELD;
}

void csp::Host::Evaluate()
{
	while( !IsEvalsStackEmpty() )
	{
		csp::Process& process = PopEvalStep();
		process.Evaluate( *this );
	}
}

csp::WorkResult::Enum csp::Host::Work( time_t dt )
{
	if( !m_mainProcess.IsRunning() )
		return WorkResult::FINISH;

	m_mainProcess.Work( *this, dt );
	Evaluate();
	return m_mainProcess.IsRunning() ? WorkResult::YIELD : WorkResult::FINISH;
}

void csp::Host::PushEvalStep( Process& process )
{
	CORE_ASSERT( m_evalStepsStackTop >= -1 && m_evalStepsStackTop < CHANNEL_STACK_SIZE-1 );

	++m_evalStepsStackTop;
	m_evalStepsStack[ m_evalStepsStackTop ] = &process;
}

csp::Process& csp::Host::PopEvalStep()
{
	CORE_ASSERT( m_evalStepsStackTop >= 0 && m_evalStepsStackTop < CHANNEL_STACK_SIZE );

	Process* pProcess = m_evalStepsStack[ m_evalStepsStackTop ];
	CORE_ASSERT( pProcess );
	m_evalStepsStack[ m_evalStepsStackTop ] = NULL;

	--m_evalStepsStackTop;
	return *pProcess;
}

bool csp::Host::IsEvalsStackEmpty() const
{
	return m_evalStepsStackTop < 0;
}

