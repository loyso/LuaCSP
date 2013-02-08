
OpLuaSleep = CspOperation:table()

function OpLuaSleep:Work( deltaTime )
	-- log(self.seconds, deltaTime, "\n")
	self.seconds = self.seconds - deltaTime
	if self.seconds > 0 then
		return 1 --CspOperation.Yield
	else
		return 0 --CspOperation.Finish
	end
end

function OpLuaSleep:Terminate()
end

function OpLuaSleep:PushResults()
	return "hi", 42, true
end

function LUA_SLEEP( seconds )
	local op = OpLuaSleep:new()
	op.seconds = seconds
	return op:Init()
end


csp_operation = TestSuite:new()

function csp_operation:sleep()
	LUA_SLEEP(1)
end
