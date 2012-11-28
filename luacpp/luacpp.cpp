#include "luacpp.h"

#include <stdlib.h>
#include <stdint.h>

extern "C"
{
#include <lua-5.2.1/src/lua.h>
}

void* LuaDefaultAlloc (void* ud, void* ptr, size_t osize, size_t nsize) {
    (void)ud;  (void)osize;  /* not used */
    if (nsize == 0) {
        free(ptr);
        return NULL;
    }
    else
        return realloc(ptr, nsize);
}

class LuaReader
{
public:
    LuaReader(const void* data, size_t size)
        : m_Data((const uint8_t*)data)
        , m_Size(size)
    {
    }

    static const char* Read(lua_State *L, void *data, size_t *size)
    {
        (void)L;
        LuaReader* pThis = (LuaReader*)data;
        
        const char* result = NULL;
        *size = 0;
        
        if(pThis->m_Size > 0)
        {
            *size = pThis->m_Size;
            result = (const char*)pThis->m_Data;
        
            pThis->m_Data += pThis->m_Size;
            pThis->m_Size -= *size;
        }

        return result;
    }

private:
    const uint8_t* m_Data;
    size_t m_Size;
};


lua::LuaState::LuaState(lua_State& state)
    : m_state(&state)
{
}

lua_State* lua::LuaState::InternalState() const
{
    return m_state;
}

lua::LuaState lua::LuaState::NewState()
{
    lua_State* state = lua_newstate(LuaDefaultAlloc, NULL);
    return LuaState(*state);
}

void lua::LuaState::Close()
{
    lua_close(m_state);
    m_state = NULL;
}

lua::LuaState lua::LuaState::NewThread()
{
	return LuaState( *lua_newthread(m_state) );
}

lua::Return::Value lua::LuaState::LoadFromMemory(const void* data, size_t size, const char* chunkname)
{
    LuaReader luaReader(data, size);
    int value = lua_load(m_state, LuaReader::Read, &luaReader, chunkname, "bt");
    return (Return::Value)value;
}


void* lua::LuaState::GetUserData() const
{
    return *((void**)m_state - 1);
}

void lua::LuaState::SetUserData(void* userData)
{
    *((void**)m_state - 1) = userData;
}

lua::LuaStackValue lua::LuaState::GetGlobal(const char * var) const
{
	CheckStack();
	lua_getglobal(m_state, var);
	return LuaStackValue( *this, GetTop() );
}

void lua::LuaState::CheckStack() const
{
	lua_checkstack(m_state, LUA_MINSTACK);
}

int lua::LuaState::GetTop() const
{
	return lua_gettop(m_state);
}

void lua::LuaState::Pop( int numValues )
{
	lua_pop(m_state, numValues);
}

lua::Return::Value lua::LuaState::Call(int numArgs, int numResults)
{
	if (!GetTopValue().IsFunction())
		return Return::ERRRUN;

	return (Return::Value)lua_pcall(m_state, numArgs, numResults, 0); // TODO: stack traceback in msgh.
}

lua::LuaStackValue lua::LuaState::GetTopValue() const
{
	return LuaStackValue( *this, GetTop() );
}

lua::LuaStackValue::LuaStackValue( LuaState const & luaState, int index )
	: m_state( luaState.InternalState() )
	, m_index( index )
{
}

bool lua::LuaStackValue::IsNil() const
{
	return lua_isnil(m_state, m_index);
}

bool lua::LuaStackValue::IsFunction() const
{
	return lua_isfunction(m_state, m_index);
}

bool lua::LuaStackValue::IsCFunction() const
{
	return !!lua_iscfunction(m_state, m_index);
}
