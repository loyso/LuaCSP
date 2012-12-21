#pragma once
#include "luacpp.h"

namespace lua
{
	class LuaStack
	{
	public:
		LuaStack( lua_State* state );

		LuaStackValue operator[]( int index ) const;
		int GetTop() const;
		LuaStackValue GetTopValue() const;

		int NumArgs() const;

		LuaState State() const;
		lua_State* InternalState() const;
		void SetInternalState( lua_State* state );

		void PushNil();
		void PushNumber( lua::LuaNumber_t number );
		void PushInteger( int number );
		void PushBoolean( bool value );
		void PushString( const char* str );

		void PushCFunction( int (*function)(lua_State* L) );
		void PushLightUserData( const void* userData );
		LuaStackValue PushTable( int narr=0, int nrec=0 );
		void* PushUserData( size_t size );

		LuaStackValue PushGlobalValue(const char * var) const;
		LuaStackValue PushGlobalTable() const;

		LuaRef_t RefInRegistry() const;
		void UnrefInRegistry( LuaRef_t key ) const;
		void PushRegistryReferenced( LuaRef_t key ) const;

		void RegistrySet();
		lua::LuaStackValue RegistryGet();

		void RegistryPtrSet( const void* ptr );
		lua::LuaStackValue RegistryPtrGet( const void* ptr );

		LuaStackValue GetField(LuaStackValue & value, const char * key) const;
		void SetField(LuaStackValue & value, const char * key);

		LuaStackValue RawGet(LuaStackValue & value) const;
		void RawSet(LuaStackValue & value);

		LuaState NewThread();

		void XMove( const LuaStack& toStack, int numValues );
		void SetMetaTable( const LuaStackValue& value );

		void Pop(int numValues);

		int Error( const char* format, ... );
		int ArgError( int arg, const char* format, ... );

	private:
		lua_State* m_state;
	};
}
