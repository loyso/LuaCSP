#include <luacpp/luacpp.h>

#include "process.h"
#include "operation.h"

csp::Operation::Operation()
	: m_pProcess()
{

}

csp::Operation::~Operation()
{
	assert( m_pProcess == NULL );
}

void csp::Operation::SetProcess( Process & process )
{
	m_pProcess = &process;
}

csp::OpSleep::OpSleep( float seconds )
	: m_seconds( seconds )
{
}

csp::WorkResult::Enum csp::OpSleep::Work( time_t dt, int & )
{
	m_seconds -= dt;
	return m_seconds > 0 ? WorkResult::YIELD : WorkResult::FINISH;
}

struct lua_State;

int OperationSleep( lua_State * L )
{
	lua::LuaStack args(L);

	csp::Process * pProcess = csp::Process::GetProcess(L);
	if( pProcess == NULL )
		return args.State().Error("not in CSP process");

	if ( !args[1].IsNumber() )
		return args[1].ArgError("seconds expected"); 

	csp::OpSleep* pSleep = new csp::OpSleep( args[1].GetNumber() );
	return pProcess->Operate( *pSleep );
}

const csp::OperationDescription operationDescriptions[] =
{
	  OperationSleep, "sleep"
	, NULL, NULL
};

void csp::RegisterOperations( lua::LuaState & state, lua::LuaStackValue & value, const OperationDescription descriptions[] )
{
	for( int i = 0; descriptions[i].function; ++i)
	{
		state.PushCFunction( descriptions[i].function );
		state.SetField( value, descriptions[i].name );
	}
}

void csp::UnregisterOperations( lua::LuaState & state, lua::LuaStackValue & value, const OperationDescription descriptions[] )
{
	for( int i = 0; descriptions[i].function; ++i)
	{
		state.PushNil();
		state.SetField( value, descriptions[i].name );
	}
}

void csp::RegisterStandardOperations( lua::LuaState & state, lua::LuaStackValue & value )
{
	RegisterOperations( state, value, operationDescriptions );
}

void csp::UnregisterStandardOperations( lua::LuaState & state, lua::LuaStackValue & value )
{
	UnregisterOperations( state, value, operationDescriptions );
}
