#include "luastack.h"

#include "luastackvalue.h"
#include "luastate.h"

extern "C"
{
#include <lua-5.2.1/src/lua.h>
#include <lua-5.2.1/src/lauxlib.h>
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

void lua::LuaStack::PushNumber( lua::LuaNumber_t number )
{
	lua_pushnumber( m_state, number );
}

void lua::LuaStack::PushInteger( int number )
{
	lua_pushinteger( m_state, number );
}

void lua::LuaStack::PushBoolean( bool value )
{
	lua_pushboolean( m_state, value );
}

void lua::LuaStack::PushString( const char* str )
{
	lua_pushstring( m_state, str );
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

void lua::LuaStack::PushLightUserData( const void* userData )
{
	lua_pushlightuserdata( m_state, const_cast< void* >( userData ) );
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

void lua::LuaStack::RegistryPtrSet( const void* ptr )
{
	lua_rawsetp( m_state, LUA_REGISTRYINDEX, ptr );
}

lua::LuaStackValue lua::LuaStack::RegistryPtrGet( const void* ptr )
{
	lua_rawgetp( m_state, LUA_REGISTRYINDEX, ptr );
	return GetTopValue();
}

int lua::LuaStack::Error( const char* format, ... )
{
	char buffer[1024];

	va_list args;
	va_start( args, format );
	_vsnprintf( buffer, sizeof(buffer), format, args );
	va_end( args );

	return luaL_error( m_state, "%s", buffer );
}

int lua::LuaStack::ArgError( int arg, const char* format, ... )
{
	char buffer[1024];

	va_list args;
	va_start( args, format );
	_vsnprintf( buffer, sizeof(buffer), format, args );
	va_end( args );

	return luaL_argerror( m_state, arg, buffer );
}

lua::LuaStackValue lua::LuaStack::RawGet( LuaStackValue & value ) const
{
	lua_rawget( m_state, value.Index() );
	return GetTopValue();
}

void lua::LuaStack::RawSet( LuaStackValue & value )
{
	lua_rawset( m_state, value.Index() );
}
