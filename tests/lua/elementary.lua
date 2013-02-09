
elementary = TestSuite:new()

function elementary:sleep1tick()
	startTickCheck( self )
	SLEEP(0)
	endTickCheck( self, 1)
end

function elementary:inAndOut()
	startTickCheck( self )

	local ch = Channel:new()
	local errMsg = "communication error"
	PAR(
		function()
			ch:OUT("hi", 42, true)
			SLEEP(0)
			ch:OUT()
		end,
		function()
			local str, num, bool = ch:IN()
			checkEquals( errMsg, "hi", str )
			checkEquals( errMsg, 42, num )
			checkEquals( errMsg, true, bool )

			local n = ch:IN()
			checkEquals( errMsg, nil, n )
		end
	)

	endTickCheck( self, 1)
end


function elementary:swarm()
	startTickCheck( self )

	local swarm = Swarm:new()
	local ch = Channel:new()
	local neverFalse = true
	local num = 0

	PARWHILE(
		function()
			ch:IN()
		end,
		function()
			swarm:MAIN()
			neverFalse = false
		end,
		function()
			local ch2 = Channel:new()
			swarm:go(
				function()
					sleepTicks(4)
					neverFalse = false
				end,
				function()
					SLEEP(0)
					num = ch2:IN()
				end
			)
			swarm:go(
				function()
					sleepTicks(5)
					neverFalse = false
				end,
				function()
					ch2:OUT(42)
				end
			)
			sleepTicks(3)
			ch:OUT()
			neverFalse = false
		end
	)

	checkEquals( "no swarms communication", 42, num )
	checkEquals( "code was not terminated", true, neverFalse )
	endTickCheck( self, 3)
end



Stages = Contract:table()
Stages.stage1 = Channel
Stages.stage2 = Channel
Stages.stage3 = Channel

function elementary:contract()
	startTickCheck( self )
	
	local c = Stages:new()
	local v = 0
	PAR(
		function()
			c.stage1:OUT(1)
			c.stage2:OUT(2)
			c.stage3:OUT(3)
		end,
		function()
			v = v + c.stage1:IN()
			v = v + c.stage2:IN()
			v = v + c.stage3:IN()
		end
	)
	
	checkEqualsInt( "no communication", 6, v )
	endTickCheck( self, 0)
end

function elementary:range()
	startTickCheck( self )

	local ch = Channel:new()
	local v = 0
	PAR(
		function()
			for status, value1, value2 in ch:RANGE() do
				v = v + value1
				v = v + value2
			end
		end,
		function()
			ch:OUT( 1, 2 )
			ch:OUT( 3, 4 )
			ch:OUT( 5, 6 )
			ch:close()
		end
	)
	
	checkEqualsInt( "no communication", 21, v )
	endTickCheck( self, 0)
end

function elementary:surviveGc()
	startTickCheck( self )

	local flow = "f"
	local ch = Channel:new()
	
	local str 
	local tab 
	local ch2
	
	PAR(
		function()
			flow=flow.."1"
			ch:OUT( "hi", {1,2,3}, Channel:new() )
			flow=flow.."5"
		end,
		function()
			flow=flow.."2"
			collectgarbage()
			flow=flow.."3"
			str, tab, ch2 = ch:IN()
			flow=flow.."4"
			ch2:close()
		end
	)
	
	checkEquals( "no communication", "hi", str )
	checkEqualsArray( "no communication", {1,2,3}, tab )
	checkEquals( "wrong flow", "f12345", flow )

	endTickCheck( self, 0)
end

