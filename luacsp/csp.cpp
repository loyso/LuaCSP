
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
