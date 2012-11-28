#include "host.h"

#include <luacpp/luacpp.h>

csp::Host::Host(const lua::LuaState& luaState)
    : m_InternalState(luaState.InternalState())
{
}

csp::Host::~Host()
{
}

lua::LuaState csp::Host::LuaState()
{
    return lua::LuaState(*m_InternalState);
}

void csp::Host::Initialize()
{
}

void csp::Host::Shutdown()
{
}

void csp::Host::Main()
{
}

