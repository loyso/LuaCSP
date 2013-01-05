//[ swarm_example
function foo( ch )
	local swarm = Swarm:new()

	PARWHILE(
		function()
			ch:IN()
		end,
		function()
			swarm:MAIN()
		end,
		function()
			local ch2 = Channel:new()
			swarm:go(
				function()
					SLEEP(10)
				end,
				function()
					SLEEP(20)
					ch2:IN()
				end
			)
			swarm:go(
				function()
					SLEEP(10)
				end,
				function()
					ch2:OUT()
				end
			)
			log("finished!\n")
		end
	)
end
//]