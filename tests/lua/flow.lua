
flow = TestSuite:new()

function flow:precedenceAfterTick()
	local steps = {}
	PAR(
		function()
			PAR(
				function()
					table.insert( steps, 0 )
					SLEEP(0)
					table.insert( steps, 4 )
				end
				,
				function()
					table.insert( steps, 1 )
					SLEEP(0)
					table.insert( steps, 5 )
				end
			)
			table.insert( steps, 6 )
			SLEEP(0)
			table.insert( steps, 10 )
		end
		,
		function()
			PAR(
				function()
					table.insert( steps, 2 )
					SLEEP(0)
					table.insert( steps, 7 )
				end
				,
				function()
					table.insert( steps, 3 )
					SLEEP(0)
					table.insert( steps, 8 )
				end
			)
			table.insert( steps, 9 )
			SLEEP(0)
			table.insert( steps, 11 )
		end
	)

	checkEqualsArray( { 0,1,2,3,4,5,6,7,8,9,10,11 }, steps, "wrong simulation step order" )
end

function flow:precedenceInOut()
	local t1 = tick()
	local steps = {}

	local ch1 = Channel:new()
	local ch2 = Channel:new()
	PAR(
		function()
			table.insert( steps, 0 )
			SLEEP(0)
			ch1:OUT()
			table.insert( steps, 5 )
		end
		,
		function()
			table.insert( steps, 1 )
			SLEEP(0)
			ch2:IN()
			table.insert( steps, 3 )
			ch1:IN()
			table.insert( steps, 4 )
		end
		,
		function()
			table.insert( steps, 2 )
			SLEEP(0)
			ch2:OUT()
			table.insert( steps, 6 )
		end
	)

	local t2 = tick()
	checkEqualsInt( 1, t2-t1, "simulation tick difference" )

	checkEqualsArray( { 0,1,2,3,4,6,5 }, steps, "wrong simulation step order" )
end
