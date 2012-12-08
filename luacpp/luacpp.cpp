#include "luacpp.h"

#include <stdint.h>

#include <stdio.h>
#include <stdarg.h>

extern "C"
{
#include <lua-5.2.1/src/lua.h>
#include <lua-5.2.1/src/lauxlib.h>
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

lua::LuaState::LuaState()
	: m_stack( NULL )
{
}

lua::LuaState::LuaState(lua_State* state)
    : m_stack( state )
{
	CORE_ASSERT( state );
}

lua_State* lua::LuaState::InternalState() const
{
    return m_stack.InternalState();
}

lua::LuaState lua::LuaState::NewState()
{
    lua_State* state = lua_newstate(LuaDefaultAlloc, NULL);
    return LuaState(state);
}

void lua::LuaState::Close()
{
    lua_close(m_stack.InternalState());
    m_stack.SetInternalState( NULL );
}

lua::Return::Enum lua::LuaState::LoadFromMemory(const void* data, size_t size, const char* chunkname)
{
    LuaReader luaReader( data, size );
	Return::Enum retValue = (Return::Enum)lua_load( m_stack.InternalState(), LuaReader::Read, &luaReader, chunkname, "bt" );
	if( retValue == LUA_OK || retValue == LUA_YIELD )
		return retValue;

	return PrintError( retValue );
}


void* lua::LuaState::GetUserData( lua_State* luaState )
{
	CORE_ASSERT( luaState );
    return *((void**)luaState - 1);
}

void lua::LuaState::SetUserData( lua_State* luaState, void* userData )
{
	CORE_ASSERT( luaState );
    *((void**)luaState - 1) = userData;
}

void lua::LuaState::CheckStack() const
{
	luaL_checkstack( m_stack.InternalState(), LUA_MINSTACK, NULL );
}

int lua::LuaState::GetTop() const
{
	return lua_gettop( m_stack.InternalState() );
}

lua::Return::Enum lua::LuaState::Call( int numArgs, int numResults )
{
	if (!GetTopValue().IsFunction())
		return Return::ERRRUN;

	return (Return::Enum)lua_pcall( m_stack.InternalState(), numArgs, numResults, 0 ); // TODO: use msgh to trace stack.
}


int lua::Print(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int retval = vprintf(fmt, ap);
	va_end(ap);
	return retval;
}


lua::Return::Enum lua::LuaState::PrintError( Return::Enum result )
{
	const char * errCode = "[no error code]";

	switch( result )
	{
	case LUA_ERRRUN:
		errCode = "LuaState: runtime error";
		break;
	case LUA_ERRSYNTAX:
		errCode = "LuaState: syntax error";
		break;
	case LUA_ERRMEM:
		errCode = "LuaState: memory allocation error";
		break;
	case LUA_ERRERR:
		errCode = "LuaState: error while running the error handler function";
		break;
	case LUA_ERRGCMM: 
		errCode = "LuaState: error while running a gc metamethod";
		break;
	case LUA_OK:
	case LUA_YIELD:
		return result;
	}

	const char * errMessage = GetTopValue().OptString( "[no error message]" );
	Print("%s\n%s\n", errCode, errMessage);

	luaL_traceback( m_stack.InternalState(), m_stack.InternalState(), NULL, 0 );
	const char * stackInfo = GetTopValue().OptString( "[no stack]" );
	Print( "%s\n", stackInfo );

	GetStack().Pop(2);
	return result;
}

lua::Return::Enum lua::LuaState::Resume(int numArgs, LuaState * pStateFrom)
{
	Return::Enum retValue = (Return::Enum)lua_resume( m_stack.InternalState(), pStateFrom ? pStateFrom->InternalState() : NULL, numArgs );
	if( retValue == LUA_OK || retValue == LUA_YIELD )
		return retValue;

	return PrintError( retValue );
}

int lua::LuaState::Yield( int numArgs )
{
	return lua_yield( m_stack.InternalState(), numArgs );
};

lua::LuaStackValue lua::LuaState::GetTopValue() const
{
	return LuaStackValue( m_stack.InternalState(), GetTop() );
}

int lua::LuaState::Error( const char* format, ... )
{
	char buffer[1024];
	
	va_list args;
	va_start( args, format );
	_vsnprintf( buffer, sizeof(buffer), format, args );
	va_end( args );

	return luaL_error( m_stack.InternalState(), "%s", buffer );
}

int lua::LuaState::ArgError( int arg, const char* format, ... )
{
	char buffer[1024];

	va_list args;
	va_start( args, format );
	_vsnprintf( buffer, sizeof(buffer), format, args );
	va_end( args );

	return luaL_argerror( m_stack.InternalState(), arg, buffer );
}

void lua::LuaState::CloseState( LuaState & luaState )
{
	luaState.Close();
}

lua::LuaStack& lua::LuaState::GetStack()
{
	return m_stack;
}

lua::Return::Enum lua::LuaState::Status() const
{
	return (Return::Enum)lua_status( m_stack.InternalState() );
}

void lua::LuaState::ReportRefLeaks() const
{
	lua_State* state = m_stack.InternalState();

	int len = (int)lua_rawlen( state, LUA_REGISTRYINDEX );
	for( int i = 1; i <= len; ++i )
	{
		if( i == LUA_RIDX_MAINTHREAD || i == LUA_RIDX_GLOBALS )
			continue;

		lua_rawgeti( state, LUA_REGISTRYINDEX, i );
		if( !lua_isnil( state, -1 ) && !lua_isnumber( state, -1 ) )
		{
			int luaType = lua_type( state, -1 );
			Print( "Lua ref leak: %d -> (%s)\n", i, lua_typename( state, luaType ) );
		}
		lua_pop( state, 1 );
	}
}


lua::LuaStackValue::LuaStackValue( lua_State* luaState, int index )
	: m_state( luaState )
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

bool lua::LuaStackValue::IsNumber() const
{
	return !!lua_isnumber( m_state, m_index );
}

int lua::LuaStackValue::Index() const
{
	return m_index;
}

lua::LuaNumber_t lua::LuaStackValue::GetNumber() const
{
	return lua_tonumber( m_state, m_index );
}

lua::LuaNumber_t lua::LuaStackValue::CheckNumber() const
{
	return luaL_checknumber( m_state, m_index );
}

lua::LuaNumber_t lua::LuaStackValue::OptNumber( LuaNumber_t default ) const
{
	return luaL_optnumber( m_state, m_index, default );
}

int lua::LuaStackValue::ArgError( const char * errMsg )
{
	return luaL_argerror( m_state, m_index, errMsg );
}

const char* lua::LuaStackValue::OptString( const char* default ) const
{
	return luaL_optstring( m_state, m_index, default );
}

bool lua::LuaStackValue::GetBoolean() const
{
	return !!lua_toboolean( m_state, m_index );
}

bool lua::LuaStackValue::IsBoolean() const
{
	return !!lua_isboolean( m_state, m_index );
}

const char* lua::LuaStackValue::GetString() const
{
	return lua_tostring( m_state, m_index );
}

bool lua::LuaStackValue::IsString() const
{
	return lua_type( m_state, m_index ) == LUA_TSTRING;
}

void lua::LuaStackValue::PushValue()
{
	lua_pushvalue( m_state, m_index );
}

void* lua::LuaStackValue::GetUserData() const
{
	return lua_touserdata( m_state, m_index );
}

bool lua::LuaStackValue::IsUserData() const
{
	return !!lua_isuserdata( m_state, m_index );
}

bool lua::LuaStackValue::IsTable() const
{
	return !!lua_istable( m_state, m_index );
}

lua::LuaStack::LuaStack( lua_State* luaState )
	: m_state( luaState )
{
}

lua::LuaStackValue lua::LuaStack::operator[]( int index ) const
{
	return LuaStackValue( m_state, index );
}

int lua::LuaStack::NumArgs() const
{
	return lua_gettop( m_state );
}

lua::LuaState lua::LuaStack::State() const
{
	return LuaState( m_state );
}

void lua::LuaStack::PushNil()
{
	lua_pushnil(m_state);
}

void lua::LuaStack::PushCFunction( int (*function)(lua_State* L) )
{
	lua_pushcfunction( m_state, function );
}


void lua::LuaStack::PushRegistryReferenced( LuaRef_t key ) const
{
	lua_rawgeti( m_state, LUA_REGISTRYINDEX, key );
}

lua::LuaRef_t lua::LuaStack::RefInRegistry() const
{
	lua::LuaRef_t key = luaL_ref( m_state, LUA_REGISTRYINDEX );
	return key;
}

void lua::LuaStack::UnrefInRegistry( LuaRef_t key ) const
{
	luaL_unref( m_state, LUA_REGISTRYINDEX, key );
}

lua::LuaState lua::LuaStack::NewThread()
{
	return LuaState( lua_newthread(m_state) );
}

void lua::LuaStack::XMove( const LuaStack& toStack, int numValues )
{
	lua_xmove( m_state, toStack.State().InternalState(), numValues );
}

void lua::LuaStack::Pop( int numValues )
{
	lua_pop(m_state, numValues);
}

void lua::LuaStack::PushLightUserData( void* userData )
{
	lua_pushlightuserdata( m_state, userData );
}

void* lua::LuaStack::PushUserData( size_t size )
{
	return lua_newuserdata( m_state, size );
}

lua::LuaStackValue lua::LuaStack::PushTable( int narr, int nrec )
{
	lua_createtable( m_state, narr, nrec );
	return GetTopValue();
}

void lua::LuaStack::SetMetaTable( const LuaStackValue& value )
{
	lua_setmetatable( m_state, value.Index() );
}

lua::LuaStackValue lua::LuaStack::GetField(LuaStackValue & value, const char * key) const
{
	lua_getfield(m_state, value.Index(), key);
	return GetTopValue();
}

void lua::LuaStack::SetField( LuaStackValue & value, const char * key )
{
	lua_setfield(m_state, value.Index(), key);
}

lua::LuaStackValue lua::LuaStack::GetTopValue() const
{
	return LuaStackValue( m_state, GetTop() );
}

void lua::LuaStack::RegistrySet()
{
	lua_rawset( m_state, LUA_REGISTRYINDEX );
}

lua::LuaStackValue lua::LuaStack::RegistryGet()
{
	lua_rawget( m_state, LUA_REGISTRYINDEX );
	return GetTopValue();
}

lua::LuaStackValue lua::LuaStack::PushGlobalValue(const char * var) const
{
	lua_getglobal(m_state, var);
	return LuaStackValue( m_state, GetTop() );
}

lua::LuaStackValue lua::LuaStack::PushGlobalTable() const
{
	lua_pushinteger( m_state, LUA_RIDX_GLOBALS );
	lua_rawget( m_state, LUA_REGISTRYINDEX ); 
	return GetTopValue();
}

int lua::LuaStack::GetTop() const
{
	return lua_gettop( m_state );
}

lua_State* lua::LuaStack::InternalState() const
{
	return m_state;
}

void lua::LuaStack::SetInternalState( lua_State* state )
{
	m_state = state;
}

