/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#pragma once

struct lua_State;

namespace lua
{
	class LuaState;
	class LuaStackValue;
}

namespace csp
{
	class Host;
	class Process;
	class Channel;

	struct ChannelArgument;
}

namespace csp
{
	typedef float CspTime_t;

	namespace WorkResult
	{
		enum Enum
		{
			  FINISH = 0
			, YIELD
		};
	}

	class GcObject
	{
	public:
		GcObject();
		virtual ~GcObject();
	};

	struct ChannelAttachment_i
	{
		virtual Process& ProcessToEvaluate() = 0;
		virtual void CloseChannel( Host& host, Channel& channel ) = 0;
	};

	struct ChannelAttachmentIn_i : ChannelAttachment_i
	{
		virtual void MoveChannelArguments( Channel& channel, ChannelArgument* arguments, int numArguments ) = 0;
	};

	struct ChannelAttachmentOut_i : ChannelAttachment_i
	{
		virtual void Communicate( Host& host, Process& inputProcess ) = 0;
	};

	struct FunctionRegistration
	{
		const char* name;
		int (*function)( lua_State* L );
	};

	void RegisterFunctions( lua::LuaState& state, lua::LuaStackValue& value, const FunctionRegistration registrations[] );
	void UnregisterFunctions( lua::LuaState& state, lua::LuaStackValue& value, const FunctionRegistration registrations[] );


	void InitializeCspObject( lua::LuaState& state, const char* scopeName, const FunctionRegistration globalFunctions[]
		, const FunctionRegistration memberFunctions[] );
	
	void InitializeCspObjectEnv( lua::LuaState& state, const char* scopeName, const FunctionRegistration globalFunctions[]
		, const FunctionRegistration memberFunctions[], lua::LuaStackValue& env );
	
	void InitilaizeCspObjectGlobals( lua::LuaState& state, const FunctionRegistration globalFunctions[]
		, const char* scopeName );

	void ShutdownCspObject( lua::LuaState& state, const char* scopeName, const FunctionRegistration globalFunctions[]
		, const FunctionRegistration memberFunctions[] );


	void PushGcObject( lua_State* luaState, GcObject& gcObject, const FunctionRegistration memberFunctions[] );
	int GcObject_Gc( lua_State* luaState );

	lua::LuaStackValue PushCspMetatable( lua_State* luaState, const FunctionRegistration memberFunctions[] );
	void CspSetMetatable( lua_State* luaState, const lua::LuaStackValue& value, const FunctionRegistration memberFunctions[] );
	bool CspHasMetatable( lua_State* luaState, const lua::LuaStackValue& value, const FunctionRegistration memberFunctions[] );

    Host& Initialize();
    void Shutdown(Host& host);
}