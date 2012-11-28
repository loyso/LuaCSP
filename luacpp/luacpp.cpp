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
    : m_State(&state)
{
}

lua_State* lua::LuaState::InternalState() const
{
    return m_State;
}

lua::LuaState lua::LuaState::NewState()
{
    lua_State* state = lua_newstate(LuaDefaultAlloc, NULL);
    return LuaState(*state);
}

void lua::LuaState::Close()
{
    lua_close(m_State);
    m_State = NULL;
}

lua::Return::Value lua::LuaState::LoadFromMemory(const void* data, size_t size, const char* chunkname)
{
    LuaReader luaReader(data, size);
    int value = lua_load(m_State, LuaReader::Read, &luaReader, chunkname, "bt");
    return (Return::Value)value;
}


void* lua::LuaState::GetUserData()
{
    return *((void**)m_State - 1);
}

void lua::LuaState::SetUserData(void* userData)
{
    *((void**)m_State - 1) = userData;
}
