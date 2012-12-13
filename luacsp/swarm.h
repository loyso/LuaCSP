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
		virtual WorkResult::Enum Work( Host& host, time_t dt );
		virtual void Terminate( Host& host );
		
		void CheckFinished();
		void UnrefClosures();

		Swarm* m_pSwarm;

		struct SwarmClosure
		{
			Process process;
			lua::LuaRef_t refKey;
		};
		SwarmClosure** m_closures;
		int m_numClosures;
		int m_closureToRun;
	};

	void PushSwarm( lua_State* luaState, Swarm& swarm );
	bool IsSwarmArg( lua::LuaStackValue const& value );
	Swarm* GetSwarmArg( lua::LuaStackValue const& value );

	void InitializeSwarms( lua::LuaState& state );
	void ShutdownSwarms( lua::LuaState& state );
}
