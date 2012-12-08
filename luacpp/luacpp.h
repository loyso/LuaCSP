#pragma once

struct lua_State;

namespace lua
{
	typedef float LuaNumber_t; // must be correspondent to lua_Number in luaconf.h

	typedef int LuaRef_t;
	const LuaRef_t LUA_NO_REF = -2; // LUA_NOREF

    namespace Return
    {
        enum Enum
        {
              OK = 0
            , YIELD = 1
            , ERRRUN = 2
            , ERRSYNTAX = 3
            , ERRMEM = 4
            , ERRGCMM = 5
            , ERRERR = 6
        };
    };

	int Print(const char* fmt, ...);

	class LuaState;
	class LuaStack;
	class LuaStackValue;

	class LuaStackValue
	{
	public:
		LuaStackValue( lua_State* state, int index);

		bool IsNil() const;
		bool IsFunction() const;
		bool IsCFunction() const;
		bool IsNumber() const;
		bool IsString() const;
		bool IsBoolean() const;
		bool IsUserData() const;
		bool IsTable() const;

		LuaNumber_t GetNumber() const;
		LuaNumber_t CheckNumber() const;
		LuaNumber_t OptNumber( LuaNumber_t default ) const;
		
		const char* GetString() const; 
		const char* CheckString() const; 
		const char* OptString( const char* default ) const; 

		bool GetBoolean() const;
		void* GetUserData() const;

		int ArgError( const char* errMsg );

		int Index() const;
		void PushValue();

	private:
		lua_State* m_state;
		int m_index;
	};

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
		void PushCFunction( int (*function)(lua_State* L) );
		void PushLightUserData( void* userData );
		LuaStackValue PushTable( int narr=0, int nrec=0 );
		void* PushUserData( size_t size );

		LuaStackValue PushGlobalValue(const char * var) const;
		LuaStackValue PushGlobalTable() const;

		LuaRef_t RefInRegistry() const;
		void UnrefInRegistry( LuaRef_t key ) const;
		void PushRegistryReferenced( LuaRef_t key ) const;

		void RegistrySet();
		lua::LuaStackValue RegistryGet();

		LuaStackValue GetField(LuaStackValue & value, const char * key) const;
		void SetField(LuaStackValue & value, const char * key);

		LuaState NewThread();

		void XMove( const LuaStack& toStack, int numValues );
		void SetMetaTable( const LuaStackValue& value );

		void Pop(int numValues);

	private:
		lua_State* m_state;
	};

    class LuaState
    {
    public:
        static LuaState NewState();
		static void CloseState( LuaState & luaState );
        
        void Close();

		LuaState();
        explicit LuaState(lua_State* luaState);
        lua_State* InternalState() const;

        Return::Enum LoadFromMemory( const void* data, size_t size, const char* chunkname );
		Return::Enum Call( int numArgs, int numResults );
		Return::Enum Resume( int numArgs, LuaState * pStateFrom );

		void CheckStack() const;		

		int GetTop() const;
		LuaStackValue GetTopValue() const;

		LuaStack& GetStack();
		
        // Per-thread user data requires the Lua interpreter to be compiled with LUAI_EXTRASPACE=sizeof(void*)
        static void* GetUserData( lua_State* luaState );
        static void SetUserData( lua_State* luaState, void* userData );

		int Error( const char* format, ... );
		int ArgError( int arg, const char* format, ... );
		Return::Enum PrintError( Return::Enum result );

		int Yield( int numArgs );

		Return::Enum Status() const;

		void ReportRefLeaks() const;
    private:
        LuaStack m_stack;
    };
}