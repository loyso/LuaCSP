#include "luastate.h"

#include "luastackvalue.h"

#include <stdint.h>

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
