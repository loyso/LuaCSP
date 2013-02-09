
csp_operation = TestSuite:new()

function csp_operation:sleep()
	startTickCheck( self )

	local t1 = time() 
	local dt, str, b = LUA_SLEEP(10)
	local t2 = time()

	checkEqualsFloat( "deltaTime calc error", t2-t1, dt, 0.001 )
	checkEquals( "no return args", "hi", str )
	checkEquals( "no return args", true, b )

	endTickCheck( self, 10 )
end

function csp_operation:terminate()
	startTickCheck( self )

	local t = {}
	PARWHILE(
		function()
			SLEEP(0)
		end
		,
		function()
			LUA_ONTERMINATE( t )
		end
	)

	checkEquals( "no Terminate handler called", true, t.terminated )
	endTickCheck( self, 1 )
end


OpLuaSleep = CspOperation:table()

function OpLuaSleep:Work( deltaTime )
	self.time = self.time + deltaTime
	self.ticks = self.ticks - 1
	if self.ticks > 0 then
		return self.Yield
	else
		return self.Finish
	end
end

function OpLuaSleep:Terminate()
end

function OpLuaSleep:PushResults()
	return self.time, "hi", true
end

function LUA_SLEEP( ticks )
	local op = OpLuaSleep:new()
	op.ticks = ticks
	op.time = 0
	return op:Init()
end


OpOnTerminate = CspOperation:table()

function OpOnTerminate:Work( deltaTime )
	return self.Yield
end

function OpOnTerminate:Terminate()
	self.table.terminated = true
end

function LUA_ONTERMINATE( table )
	local op = OpOnTerminate:new()
	op.table = table
	return op:Init()
end
