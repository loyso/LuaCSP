/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#include "op_par.h"

#include <luacpp/luastackvalue.h>

#include "host.h"

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
		finished = false;

		ParClosure& closure = m_closures[ m_closureToRun++ ];

		if( m_closureToRun < m_numClosures )
			host.PushEvalStep( ThisProcess() );

		closure.process.StartEvaluation( host, 0 );
	}

	if ( !CheckFinished() )
		finished = false;

	if ( finished )
		SetFinished( true );

	return IsFinished() ? WorkResult::FINISH : WorkResult::YIELD;
}

csp::WorkResult::Enum csp::OpPar::Work( Host& host, CspTime_t dt )
{
	for( int i = m_closureToRun-1; i >= 0; --i )
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

void csp::OpPar::DebugCheck( Host& host ) const
{
#ifdef _DEBUG
	for( int i = 0; i < m_closureToRun; ++i )
	{
		Process& process = m_closures[ i ].process;
		CORE_ASSERT( !host.DebugIsProcessOnStack( process ) );
	}
#endif
}

void csp::OpPar::UnrefClosures()
{
	for( int i = 0; i < m_numClosures; ++i )
	{
		if( m_closures[ i ].refKey != lua::LUA_NO_REF )
		{
			ThisProcess().LuaThread().GetStack().UnrefInRegistry( m_closures[ i ].refKey );
			m_closures[ i ].refKey = lua::LUA_NO_REF;
		}
	}
}

void csp::OpPar::Terminate( Host& host )
{
	for( int i = 0; i < m_numClosures; ++i )
	{
		m_closures[ i ].process.Terminate( host );
	}
	UnrefClosures();
}


csp::WorkResult::Enum csp::OpParWhile::Evaluate( Host& host )
{
	if( m_numClosures < 1 )
		return WorkResult::FINISH;

	if( m_closureToRun > 0 )
	{
		if( !m_closures[ 0 ].process.IsRunning() )
		{
			DoTerminate( host );
			return WorkResult::FINISH;
		}
	}

	return OpPar::Evaluate( host );
}

void csp::OpParWhile::DoTerminate( Host& host )
{
	Terminate( host );
}
