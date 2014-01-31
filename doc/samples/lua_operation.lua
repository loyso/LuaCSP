//[ lua_operation_sleep
OpLuaSleep = CspOperation:table()

function OpLuaSleep:Work( deltaTime )
	self.seconds = self.seconds - deltaTime
	if self.seconds > 0 then
		return self.Yield
	else
		return self.Finish
	end
end

function LUA_SLEEP( seconds )
	local op = OpLuaSleep:new()
	op.seconds = seconds
	return op:Init()
end
//]

//[ lua_operation_terminate
function OpSoundPlayback:Terminate()
	self.sound:Stop()
end
//]

//[ lua_operation_pushresults
function OpMyComputation:PushResults()
	return self.result, "hi", true
end
//]

