#pragma once

#include <luacsp/cppchannel.h>

class OpMonitorKeyState : public csp::OpCppChannelOut
{
public:
	OpMonitorKeyState();
	virtual ~OpMonitorKeyState();

private:
	virtual bool Init( lua::LuaStack& args, InitError& initError );

	virtual csp::WorkResult::Enum Update( csp::CspTime_t dt );
	virtual bool IsOutputReady() const;
	virtual int PushOutputArguments( lua::LuaStack& luaStack );

	char m_charToMonitor;
	bool m_pressed;
	int m_count;
};

void InitializeMyChannels( lua::LuaState& state );
void ShutdownMyChannels( lua::LuaState& state );
