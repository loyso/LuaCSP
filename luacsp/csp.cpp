
#include "csp.h"

#include <luacpp/luacpp.h>

#include "host.h"

csp::Host& csp::Initialize()
{
    lua::LuaState luaState = lua::LuaState::NewState();
    
	csp::Host& host = *new csp::Host(luaState);
    host.Initialize();
    
	return host;
}

void csp::Shutdown(csp::Host& host)
{
	lua::LuaState luaState = host.LuaState();
    
	host.Shutdown();
    delete &host;

	lua::LuaState::CloseState( luaState );
}

void csp::RegisterFunctions( lua::LuaState & state, lua::LuaStackValue & value, const FunctionRegistration descriptions[] )
{
	lua::LuaStack stack = state.GetStack();
	for( int i = 0; descriptions[i].function; ++i)
	{
		stack.PushCFunction( descriptions[i].function );
		stack.SetField( value, descriptions[i].name );
	}
}

void csp::UnregisterFunctions( lua::LuaState & state, lua::LuaStackValue & value, const FunctionRegistration descriptions[] )
{
	lua::LuaStack stack = state.GetStack();
	for( int i = 0; descriptions[i].function; ++i)
	{
		stack.PushNil();
		stack.SetField( value, descriptions[i].name );
	}
}


