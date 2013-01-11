
flow = TestSuite:new()

function flow:precedenceInOut()
	local t1 = tick()
	SLEEP(0)
	local t2 = tick()
	checkEqualsInt( 1, t2-t1, "simulation tick difference" )
end
