
elementary = TestSuite:new()


elementary.helpers = {}

function elementary.helpers:startTickCheck()
	self.t1 = tick()
end

function elementary.helpers:endTickCheck( expected, t1 )
	local t2 = tick()
	checkEqualsInt( "simulation tick difference", expected, t2-self.t1 )
end

function elementary.helpers:sleepTicks( ticks )
	for i=1,ticks do
		SLEEP(0)
	end
end


function elementary:sleep1tick()
	helpers:startTickCheck()
	SLEEP(0)
	helpers:endTickCheck(1)
end

function elementary:inAndOut()
	helpers:startTickCheck()

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

	helpers:endTickCheck(1)
end


function elementary:swarm()
	helpers:startTickCheck()

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
					helpers:sleepTicks(4)
					neverFalse = false
				end,
				function()
					SLEEP(0)
					num = ch2:IN()
				end
			)
			swarm:go(
				function()
					helpers:sleepTicks(5)
					neverFalse = false
				end,
				function()
					ch2:OUT(42)
				end
			)
			helpers:sleepTicks(3)
			ch:OUT()
			neverFalse = false
		end
	)

	checkEquals( "no swarms communication", 42, num )
	checkEquals( "code was not terminated", true, neverFalse )
	helpers:endTickCheck(3)
end



Stages = Contract:table()
Stages.stage1 = Channel
Stages.stage2 = Channel
Stages.stage3 = Channel

function elementary:contract()
	helpers:startTickCheck()
	
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
	helpers:endTickCheck(0)
end

function elementary:range()
	helpers:startTickCheck()

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
	helpers:endTickCheck(0)
end
