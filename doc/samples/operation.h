//[ operation_h_workresult
namespace WorkResult
{
	enum Enum
	{
		  FINISH = 0 // return from operation, resume coroutine
		, YIELD	// do not return for now, continue to work
	};
}
//]

//[ operation_h_decl
class Operation
{
public:
	virtual WorkResult::Enum Work( Host& host, CspTime_t dt ) = 0;	
	virtual int PushResults( lua::LuaStack& luaStack );
	void SetFinished( bool finished );
...

private:
	virtual bool Init( lua::LuaStack& args, InitError& initError );
	virtual void Terminate( Host& host );
...
};
//]

//[ operation_h_sleep_decl
class OpSleep : public Operation
{
public:
	OpSleep();

private:
	virtual bool Init( lua::LuaStack& args, InitError& initError );
	virtual WorkResult::Enum Work( Host& host, CspTime_t dt );

	float m_seconds;
};
//]
