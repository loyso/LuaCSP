
testalt = TestSuite:new()

testalt.helpers = {}

function testalt.helpers:startTickCheck()
	self.t1 = tick()
end

function testalt.helpers:endTickCheck( expected, t1 )
	local t2 = tick()
	checkEqualsInt( "simulation tick difference", expected, t2-self.t1 )
end

function testalt:startFirst()
	helpers:startTickCheck()
	local ch1 = Channel:new()
	local ch2 = Channel:new()
	local ch3 = Channel:new()
	
	local str
	local bool
	local flow = "f"
	
	PAR(
		function()
			flow=flow.."1"
			ALT(
				ch1, function( arg1, arg2 )
					flow=flow.."3"
					str = arg1; bool = arg2
				end
				,
				ch2, function( arg1, arg2 )
					flow=flow.."4"
					str = arg1; bool = arg2
				end
				,
				ch3, function( arg1, arg2 )
					flow=flow.."5"
					str = arg1; bool = arg2
				end
			)
			flow=flow.."6"
		end,
		function()
			flow=flow.."2"
			ch3:OUT( "hi", true )
			flow=flow.."7"
		end
	)
	checkEquals("no communication", true, bool )
	checkEquals("no communication", "hi", str )
	checkEquals("wrong flow", "f12567", flow )
	helpers:endTickCheck(0)
end

function testalt:startSecond()
	helpers:startTickCheck()
	local ch1 = Channel:new()
	local ch2 = Channel:new()

	local str
	local bool
	local flow="f"

	PAR(
		function()
			flow=flow.."1"
			ch2:OUT( "hi", true )
			flow=flow.."5"
		end,
		function()
			flow=flow.."2"
			ALT(
				ch1, function( arg1, arg2 )
					flow=flow.."-"
					str = arg1; bool = arg2
				end
				,
				ch2, function( arg1, arg2 )
					flow=flow.."3"
					str = arg1; bool = arg2
				end
			)
			flow=flow.."4"
		end
	)
	flow=flow.."6"

	checkEquals("no communication", true, bool )
	checkEquals("no communication", "hi", str )
	checkEquals("wrong flow", "f123456", flow )
	helpers:endTickCheck(0)
end

function testalt:polling()
	helpers:startTickCheck()
	local ch = Channel:new()

	local flow = "f"

	ALT(
		ch, function()
			flow=flow.."-"
		end
		,
		nil, function()
			flow=flow.."1"
		end
	)

	checkEquals("wrong flow", "f1", flow )
	helpers:endTickCheck(0)
end

function testalt:pollingAgainstTime()
	helpers:startTickCheck()

	local flow = "f"

	ALT(
		time()+0.1, function()
			flow=flow.."-"
		end
		,
		nil, function()
			flow=flow.."1"
		end
	)

	checkEquals("wrong flow", "f1", flow )
	helpers:endTickCheck(0)
end

function testalt:timeGuard()
	local t1 = time()
	local ch = Channel:new()
	local flow = "f"
	PAR(
		function()
			flow=flow.."1"
			ALT(
				time() + 0.8, function()
					flow=flow.."7"
				end,
				time() + 0.4, function()
					flow=flow.."5"
				end
			)
			flow=flow.."6"
		end,
		function()
			flow=flow.."2"
			ALT(
				ch, function()
					flow=flow.."8"
				end
				,
				time() + 0.1, function()
					flow=flow.."3"
				end
			)
			flow=flow.."4"
		end
	)
	flow=flow.."9"

	local t2 = time()
	checkEqualsFloat("wrong timing", 0.4, t2-t1, 0.02)
	checkEquals("wrong flow", "f1234569", flow )
end

