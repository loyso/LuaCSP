
flow = TestSuite:new()

function flow:precedenceAfterTick()
	local steps = {}
	local step = function(n)
		table.insert( steps, n )
	end

	PAR(
		function()
			PAR(
				function()
					step( 0 )
					SLEEP(0)
					step( 4 )
				end
				,
				function()
					step( 1 )
					SLEEP(0)
					step( 5 )
				end
			)
			step( 6 )
			SLEEP(0)
			step( 10 )
		end
		,
		function()
			PAR(
				function()
					step( 2 )
					SLEEP(0)
					step( 7 )
				end
				,
				function()
					step( 3 )
					SLEEP(0)
					step( 8 )
				end
			)
			step( 9 )
			SLEEP(0)
			step( 11 )
		end
	)

	checkEqualsArray( "wrong simulation step order", { 0,1,2,3,4,5,6,7,8,9,10,11 }, steps )
end

function flow:precedenceInOut()
	local t1 = tick()

	local steps = {}
	local step = function(n)
		table.insert( steps, n )
	end

	local ch1 = Channel:new()
	local ch2 = Channel:new()
	PAR(
		function()
			step( 0 )
			SLEEP(0)
			ch1:OUT()
			step( 5 )
		end
		,
		function()
			step( 1 )
			SLEEP(0)
			ch2:IN()
			step( 3 )
			ch1:IN()
			step( 4 )
		end
		,
		function()
			step( 2 )
			SLEEP(0)
			ch2:OUT()
			step( 6 )
		end
	)

	local t2 = tick()
	checkEqualsInt( "simulation tick difference", 1, t2-t1 )

	checkEqualsArray( "wrong simulation step order", { 0,1,2,3,4,5,6 }, steps )
end
