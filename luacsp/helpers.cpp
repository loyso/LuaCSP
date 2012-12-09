#include "helpers.h"

#include <luacpp/luacpp.h>
#include <luacpp/luastackvalue.h>

#include "host.h"

namespace helpers
{
	int log( lua_State* luaState );
	int time( lua_State* luaState );
	int tick( lua_State* luaState );
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

int helpers::time( lua_State* luaState )
{
	lua::LuaStack stack( luaState );

	csp::Host& host = csp::Host::GetHost( luaState );
	stack.PushNumber( host.Time() );
	return 1;
}

int helpers::tick( lua_State* luaState )
{
	lua::LuaStack stack( luaState );

	csp::Host& host = csp::Host::GetHost( luaState );
	stack.PushInteger( host.Tick() );
	return 1;
}


const csp::FunctionRegistration helpersDescriptions[] =
{
  	  "log", helpers::log
	, "time", helpers::time
	, "tick", helpers::tick
	, NULL, NULL
};


void csp::RegisterStandardHelpers( lua::LuaState & state, lua::LuaStackValue & value )
{
	RegisterFunctions( state, value, helpersDescriptions );
}

void csp::UnregisterStandardHelpers( lua::LuaState & state, lua::LuaStackValue & value )
{
	UnregisterFunctions( state, value, helpersDescriptions );
}
