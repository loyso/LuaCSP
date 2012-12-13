#include "host.h"

#include <luacpp/luastackvalue.h>

#include "operation.h"
#include "helpers.h"
#include "channel.h"
#include "swarm.h"

namespace csp
{
	static const int CHANNEL_STACK_SIZE = 256;
	static const char HOST_IDENTITY_KEY = 0;
}

csp::Host::Host(const lua::LuaState& luaState)
    : m_luaState(luaState)
	, m_mainProcess()
	, m_evalStepsStack()
	, m_evalStepsStackTop( 0 )
	, m_time( 0 )
	, m_tick( 0 )
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

csp::Host& csp::Host::GetHost( lua_State* luaState )
{
	lua::LuaStack stack( luaState );

	lua::LuaStackValue value = stack.RegistryPtrGet( &HOST_IDENTITY_KEY );
	CORE_ASSERT( value.IsLightUserData() );

	Host* pHost = static_cast< Host* >( value.GetLightUserData() );
	CORE_ASSERT( pHost );

	stack.Pop(1);
	return *pHost;
}

void csp::Host::Initialize()
{
	lua::LuaStackValue globals = m_luaState.GetStack().PushGlobalTable();	
	
	RegisterStandardHelpers( m_luaState, globals );
	RegisterStandardOperations( m_luaState, globals );
	InitializeChannels( m_luaState );
	InitializeSwarms( m_luaState );
	
	m_luaState.GetStack().Pop(1);

	m_luaState.GetStack().PushLightUserData( this );
	m_luaState.GetStack().RegistryPtrSet( &HOST_IDENTITY_KEY );
}

void csp::Host::Shutdown()
{
	m_luaState.ReportRefLeaks();

	m_luaState.GetStack().PushNil();
	m_luaState.GetStack().RegistryPtrSet( &HOST_IDENTITY_KEY );

	lua::LuaStackValue globals = m_luaState.GetStack().PushGlobalTable();

	ShutdownSwarms( m_luaState );
	ShutdownChannels( m_luaState );
	UnregisterStandardOperations( m_luaState, globals );	
	UnregisterStandardHelpers( m_luaState, globals );	
	
	m_luaState.GetStack().Pop(1);
}

csp::WorkResult::Enum csp::Host::Main()
{
	m_mainProcess.SetLuaThread( m_luaState );

	lua::LuaStackValue stackValue = m_mainProcess.LuaThread().GetStack().PushGlobalValue("main");
	if ( !stackValue.IsFunction() || stackValue.IsCFunction() )
		return WorkResult::FINISH;

	m_mainProcess.StartEvaluation( *this, 0 );
	Evaluate();
	return m_mainProcess.IsRunning() ? WorkResult::YIELD : WorkResult::FINISH;
}

void csp::Host::Evaluate()
{
	while( !IsEvalsStackEmpty() )
	{
		csp::Process& process = PopEvalStep();
		process.Evaluate( *this, 0 );
	}
}

csp::WorkResult::Enum csp::Host::Work( time_t dt )
{
	if( !m_mainProcess.IsRunning() )
		return WorkResult::FINISH;

	m_time += dt;
	m_tick++;

	m_mainProcess.Work( *this, dt );
	Evaluate();

	return m_mainProcess.IsRunning() ? WorkResult::YIELD : WorkResult::FINISH;
}

void csp::Host::PushEvalStep( Process& process )
{
	CORE_ASSERT( m_evalStepsStackTop >= 0 && m_evalStepsStackTop < CHANNEL_STACK_SIZE );

	m_evalStepsStack[ m_evalStepsStackTop ] = &process;
	++m_evalStepsStackTop;
}

csp::Process& csp::Host::PopEvalStep()
{
	CORE_ASSERT( m_evalStepsStackTop > 0 && m_evalStepsStackTop <= CHANNEL_STACK_SIZE );
	--m_evalStepsStackTop;

	Process* pProcess = m_evalStepsStack[ m_evalStepsStackTop ];
	CORE_ASSERT( pProcess );
	m_evalStepsStack[ m_evalStepsStackTop ] = NULL;

	return *pProcess;
}

void csp::Host::RemoveProcessFromStack( const Process& process )
{
	int writePos = 0;
	for( int i = 0; i < m_evalStepsStackTop; ++i )
	{		
		Process* pProcess = m_evalStepsStack[ i ];
		if( writePos < i )
		{
			m_evalStepsStack[ writePos ] = pProcess;
			m_evalStepsStack[ i ] = NULL;
		}

		if( pProcess != &process )
			++writePos;
	}

	m_evalStepsStack[ writePos ] = NULL;
	m_evalStepsStackTop = writePos;
}

bool csp::Host::IsEvalsStackEmpty() const
{
	return m_evalStepsStackTop <= 0;
}

csp::Process* csp::Host::GetTopProcess() const
{
	CORE_ASSERT( m_evalStepsStackTop >= 0 && m_evalStepsStackTop < CHANNEL_STACK_SIZE );

	return m_evalStepsStackTop > 0 ? m_evalStepsStack[ m_evalStepsStackTop-1 ] : NULL;
}

void csp::Host::DebugCheckDeletion( const Process& process ) const
{
	for( int i = 0; i < m_evalStepsStackTop; ++i )
	{
		CORE_ASSERT( &process != m_evalStepsStack[i] );
	}
}

csp::time_t csp::Host::Time() const
{
	return m_time;
}

unsigned int csp::Host::Tick() const
{
	return m_tick;
}

void csp::Host::TerminateMain()
{
	if( m_mainProcess.IsRunning() )
		m_mainProcess.DoTerminate( *this );
}
