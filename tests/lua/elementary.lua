
elementary = TestSuite:new()

function elementary:sleep1tick()
	local t1 = tick()
	SLEEP(0)
	local t2 = tick()
	checkEqualsInt( "simulation tick difference", 1, t2-t1 )
end
