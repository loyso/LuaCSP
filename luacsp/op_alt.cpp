#include "op_alt.h"

#include <luacpp/luastackvalue.h>

#include "process.h"
#include "operation.h"
#include "host.h"
#include "channel.h"

csp::OpAlt::OpAlt()
	: m_cases()
	, m_numCases( 0 )
	, m_pCaseTriggered()
	, m_pNilCase()
	, m_arguments()
	, m_numArguments( 0 )
	, m_argumentsMoved( false )
	, m_processRefKey( lua::LUA_NO_REF )
{
}

csp::OpAlt::~OpAlt()
{
	CORE_ASSERT( m_processRefKey == lua::LUA_NO_REF );

	delete[] m_cases;
	m_cases = NULL;
	m_numCases = 0;

	delete[] m_arguments;
	m_arguments = NULL;
	m_numArguments = 0;
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

		Channel* pChannel = NULL;
		if( IsChannelArg(guard) && ( pChannel = GetChannelArg(guard) ) != NULL ) 
		{
			if( pChannel->InAttached() )
				return initError.ArgError( i, "channel is in input operation already" );
		}
		else if( guard.IsNumber() )
		{
		}
		else if( guard.IsNil() )
		{
			if( nilCase )
				return initError.ArgError( i, "there must be just one nil case" );
			nilCase = true;
		}
		else
			return initError.ArgError( i, "channel, number or nil required as a guard" );

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
			Channel* pChannel = GetChannelArg( guard );
			CORE_ASSERT( pChannel != NULL );
			pChannel->SetAttachmentIn( this );

			guard.PushValue();
			m_cases[ initCase ].m_channelRefKey = args.RefInRegistry();
			m_cases[ initCase ].m_pChannel = pChannel;
		}
		else if( guard.IsNumber() )
		{
			m_cases[ initCase ].m_time = guard.GetNumber();
		}
		else if( guard.IsNil() )
		{
			CORE_ASSERT( m_pNilCase == NULL );
			m_pNilCase = m_cases + initCase;
		}

		++initCase;
	}
}

void csp::OpAlt::UnrefArguments( lua::LuaStack const& stack )
{
	for( int i = 0; i < m_numArguments; ++i )
	{
		stack.UnrefInRegistry( m_arguments[i].refKey );
		m_arguments[i].refKey = lua::LUA_NO_REF;
	}
}

void csp::OpAlt::UnrefChannels( lua::LuaStack const& stack )
{
	for( int i = 0; i < m_numCases; ++i )
	{
		if( m_cases[i].m_pChannel != NULL )
		{
			m_cases[i].m_pChannel = NULL;
			stack.UnrefInRegistry( m_cases[i].m_channelRefKey );
			m_cases[i].m_channelRefKey = lua::LUA_NO_REF;
		}
	}
}

void csp::OpAlt::UnrefClosures( lua::LuaStack const& stack )
{
	for( int i = 0; i < m_numCases; ++i )
	{
		AltCase& altCase = m_cases[ i ];
		if( altCase.m_closureRefKey != lua::LUA_NO_REF )
		{
			stack.UnrefInRegistry( altCase.m_closureRefKey );
			altCase.m_closureRefKey = lua::LUA_NO_REF;
		}
	}
}

void csp::OpAlt::UnrefProcess( lua::LuaStack const& stack )
{
	if( m_processRefKey != lua::LUA_NO_REF )
	{
		stack.UnrefInRegistry( m_processRefKey );
		m_processRefKey = lua::LUA_NO_REF;
	}
}

void csp::OpAlt::SelectChannelProcessToTrigger( Host& host )
{
	CORE_ASSERT( m_pCaseTriggered == NULL );

	for( int i = 0; i < m_numCases; ++i )
	{
		AltCase& altCase = m_cases[ i ];
		if( m_pNilCase == &altCase )
		{
			m_pCaseTriggered = m_pNilCase;

			m_argumentsMoved = true;
			DetachChannels();

			host.PushEvalStep( ThisProcess() );
			break;
		}

		Channel* pChannel = altCase.m_pChannel;
		if( pChannel && pChannel->OutAttached() )
		{
			ChannelAttachmentOut_i& out = pChannel->OutAttachment();
			out.MoveChannelArguments();

			host.PushEvalStep( out.ProcessToEvaluate() );
			host.PushEvalStep( ThisProcess() );
			break;
		}
	}
}

void csp::OpAlt::SelectTimeProcessToTrigger( Host& host )
{
	CORE_ASSERT( m_pCaseTriggered == NULL );

	time_t time = host.Time();

	for( int i = 0; i < m_numCases; ++i )
	{
		AltCase& altCase = m_cases[ i ];
		if( m_pNilCase == &altCase )
			continue;

		if( altCase.m_pChannel == NULL && altCase.m_time > time )
		{
			if( m_pCaseTriggered )
			{
				if( altCase.m_time < m_pCaseTriggered->m_time )
					m_pCaseTriggered = &altCase;
			}
			else
				m_pCaseTriggered = &altCase;
		}
	}

	if( m_pCaseTriggered )
	{
		m_argumentsMoved = true;
		DetachChannels();

		host.PushEvalStep( ThisProcess() );
	}
}

csp::WorkResult::Enum csp::OpAlt::StartTriggeredProcess( Host& host )
{
	CORE_ASSERT( m_pCaseTriggered );

	lua::LuaStack& stack = host.LuaState().GetStack();

	lua::LuaState thread = stack.NewThread();

	stack.GetTopValue().PushValue();
	m_processRefKey = stack.RefInRegistry();

	m_process.SetLuaThread( thread );
	m_process.SetParentProcess( ThisProcess() );

	lua::LuaStack threadStack = thread.GetStack();
	threadStack.PushRegistryReferenced( m_pCaseTriggered->m_closureRefKey );
	for( int i = 0; i < m_numArguments; ++i )
		threadStack.PushRegistryReferenced( m_arguments[i].refKey );

	WorkResult::Enum result = m_process.StartEvaluation( host, m_numArguments );
	UnrefClosures( stack );
	return result;
}


csp::WorkResult::Enum csp::OpAlt::Evaluate( Host& host )
{
	if( m_pCaseTriggered == NULL )
	{
		SelectChannelProcessToTrigger( host );
	}
	else
	{
		lua::LuaStack& stack = host.LuaState().GetStack();

		if( m_argumentsMoved )
		{
			m_argumentsMoved = false;

			StartTriggeredProcess( host );

			UnrefArguments( stack );
			UnrefChannels( stack );			
		}
		else if( m_process.IsRunning() )
		{
		}
		else if( m_processRefKey != lua::LUA_NO_REF )
		{
			UnrefProcess( stack );
			return WorkResult::FINISH;
		}
	}

	return WorkResult::YIELD;
}

csp::WorkResult::Enum csp::OpAlt::Work( Host& host, time_t dt )
{
	if( m_pCaseTriggered )
	{
		if( m_process.IsRunning() )
			m_process.Work( host, dt );
	}
	else
		SelectTimeProcessToTrigger( host );

	return IsFinished() ? WorkResult::FINISH : WorkResult::YIELD;
}

void csp::OpAlt::MoveChannelArguments( Channel& channel, ChannelArgument* arguments, int numArguments )
{
	CORE_ASSERT( m_arguments == NULL );
	CORE_ASSERT( m_numArguments == 0 );

	AltCase* pCaseTriggered = NULL;
	for( int i = 0; i < m_numCases && pCaseTriggered == NULL; ++i )
	{
		if( m_cases[ i ].m_pChannel == &channel )
			pCaseTriggered = m_cases + i;
	}

	CORE_ASSERT( pCaseTriggered );
	CORE_ASSERT( m_pCaseTriggered == NULL );
	m_pCaseTriggered = pCaseTriggered;

	m_arguments = arguments;
	m_numArguments = numArguments;
	
	m_argumentsMoved = true;
	DetachChannels();
}

csp::Process& csp::OpAlt::ProcessToEvaluate()
{
	return ThisProcess();
}

void csp::OpAlt::DetachChannels() const
{
	for( int i = 0; i < m_numCases; ++i )
	{
		if( m_cases[i].m_pChannel )
			m_cases[i].m_pChannel->SetAttachmentIn( NULL );
	}
}

void csp::OpAlt::DebugCheck( Host& host ) const
{
	host.DebugCheckDeletion( m_process );
}

void csp::OpAlt::Terminate( Host& host )
{
	lua::LuaStack stack = host.LuaState().GetStack();

	m_process.Terminate( host );

	DetachChannels();
	UnrefChannels( stack );
	UnrefArguments( stack );
	UnrefClosures( stack );
	UnrefProcess( stack );
}

csp::OpAlt::AltCase::AltCase()
	: m_closureRefKey( lua::LUA_NO_REF )
	, m_pChannel()
	, m_channelRefKey( lua::LUA_NO_REF )
	, m_time( -1 )
{
}
