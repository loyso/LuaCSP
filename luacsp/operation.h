#pragma once

#include "csp.h"
#include "process.h"

namespace lua
{
	class LuaStackValue;
}

namespace csp
{
	class Process;
	class Channel;
	class Operation;
}

namespace csp
{
    class Operation
    {
	public:
		Operation();
		virtual ~Operation();

		int Initialize( lua_State* luaState );

		virtual WorkResult::Enum Evaluate( Host& host );
		virtual WorkResult::Enum Work( Host& host, time_t dt ) = 0;
		
		virtual int PushResults( lua::LuaStack & luaStack );

		bool IsFinished() const;
		void SetFinished( bool finished );

		virtual void DebugCheck( Host& host );

	protected:
		Process& ThisProcess() const;

		struct InitError
		{
			bool ArgError( int arg, const char* message );
			bool Error( const char* message );

			int errorArg;
			const char* errorMessage;
		};

    private:
		virtual bool Init( lua::LuaStack & args, InitError& initError );

		Process* m_pProcess;
		bool m_finished;
    };

	class OpSleep : public Operation
	{
	public:
		OpSleep();

	private:
		virtual bool Init( lua::LuaStack & args, InitError& initError );
		virtual WorkResult::Enum Work( Host& host, time_t dt );

		float m_seconds;
	};

	class OpPar : public Operation
	{
	public:
		OpPar();
		virtual ~OpPar();

		virtual WorkResult::Enum Evaluate( Host& host );
		virtual WorkResult::Enum Work( Host& host, time_t dt );

	private:
		virtual bool Init( lua::LuaStack& args, InitError& initError );
		bool CheckFinished();

		struct ParClosure
		{
			Process process;
			lua::LuaRef_t refKey;
		};
		ParClosure* m_closures;
		int m_numClosures;

		int m_closureToRun;
	};

	class OpAlt : public Operation, ChannelAttachmentIn_i
	{
	public:
		OpAlt();
		virtual ~OpAlt();

		virtual WorkResult::Enum Evaluate( Host& host );
		virtual WorkResult::Enum Work( Host& host, time_t dt );

	private:
		virtual bool Init( lua::LuaStack& args, InitError& initError );
		bool CheckArgs( lua::LuaStack& args, InitError& initError ) const;
		void InitCases( lua::LuaStack& args );
		
		void UnrefChannels( lua::LuaStack const& stack );
		void UnrefArguments( lua::LuaStack const& stack );
		void UnrefClosures( lua::LuaStack const& stack );
		void DetachChannels() const;

		WorkResult::Enum StartTriggeredProcess( Host& host );
		void SelectProcessToTrigger( Host& host );

		virtual void MoveChannelArguments( Channel& channel, ChannelArgument* arguments, int numArguments );
		virtual Process& ProcessToEvaluate();

		struct AltCase
		{
			AltCase();

			lua::LuaRef_t m_closureRefKey;

			Channel* m_pChannel;
			lua::LuaRef_t m_channelRefKey;
			time_t m_time;				
		};
		AltCase* m_cases;
		int m_numCases;
		
		AltCase* m_pCaseTriggered;
		AltCase* m_pNilCase;

		Process m_process;
		lua::LuaRef_t m_processRefKey;

		ChannelArgument* m_arguments;
		int m_numArguments;
		bool m_argumentsMoved;
	};


	void RegisterStandardOperations( lua::LuaState & state, lua::LuaStackValue & value );
	void UnregisterStandardOperations( lua::LuaState & state, lua::LuaStackValue & value );
}