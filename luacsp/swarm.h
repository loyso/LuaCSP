/**
 * This file is a part of LuaCSP library.
 * Copyright (c) 2012-2013 Alexey Baskakov
 * Project page: http://github.com/loyso/LuaCSP
 * This library is distributed under the GNU General Public License (GPL), version 2.
 * The above copyright notice shall be included in all copies or substantial portions of the Software.
 */
#pragma once

#include "csp.h"

#include "operation.h"

namespace lua
{
	class LuaState;
}

namespace csp
{
	class Swarm;
	class OpSwarmMain;
}

namespace csp
{
	class Swarm : public GcObject
	{
	public:
		Swarm();
		virtual ~Swarm();

		int DoGo( lua::LuaStack& args );
		
		void InitMain( OpSwarmMain& opMain );
		void Terminate();
	
	private:
		OpSwarmMain* m_pOpMain;
	};

	class OpSwarmMain : public Operation
	{
	public:
		OpSwarmMain();
		virtual ~OpSwarmMain();

		int Go( lua::LuaStack& args );

	private:
		virtual bool Init( lua::LuaStack& args, InitError& initError );
		virtual WorkResult::Enum Evaluate( Host& host );
		virtual WorkResult::Enum Work( Host& host, CspTime_t dt );
		virtual void Terminate( Host& host );
		
		void CheckFinished();

		Swarm* m_pSwarm;

		struct SwarmClosure
		{
			SwarmClosure* pNext;
			Process process;
			lua::LuaRef_t refKey;
		};

		void UnrefClosures( SwarmClosure* pHead );
		static void DeleteClosures( SwarmClosure*& pHead, SwarmClosure*& pTail );

		static SwarmClosure* ListPopFromHead( SwarmClosure*& pHead, SwarmClosure*& pTail );
		static void ListAddToTail( SwarmClosure*& pHead, SwarmClosure*& pTail, SwarmClosure& node );

		SwarmClosure *m_pClosuresHead, *m_pClosuresTail;
		SwarmClosure *m_pClosuresToRunHead, *m_pClosuresToRunTail;
	};

	void PushSwarm( lua_State* luaState, Swarm& swarm );
	bool IsSwarmArg( lua::LuaStackValue const& value );
	Swarm* GetSwarmArg( lua::LuaStackValue const& value );

	void InitializeSwarms( lua::LuaState& state );
	void ShutdownSwarms( lua::LuaState& state );
}
