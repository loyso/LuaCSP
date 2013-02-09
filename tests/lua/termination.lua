
termination = TestSuite:new()

function termination:subprocesses()
	startTickCheck( self )

	local flow="f"

	PARWHILE(
		function()
			flow=flow.."1"
			SLEEP(0)
			flow=flow.."7"
		end,
		function()
			flow=flow.."2"
			SLEEP(0)
			flow=flow.."-"
			SLEEP(0)
			flow=flow.."-"
		end,
		function()
			flow=flow.."3"
			local ch = Channel:new()
			ch:IN()
		end,
		function()
			flow=flow.."4"
			local ch = Channel:new()
			ch:OUT()
		end,
		function()
			flow=flow.."5"
			local ch = Channel:new()
			ALT(
				ch, function()
					flow=flow.."-"
				end,
				time() + 30, function()
					flow=flow.."-"
				end
			)
		end,
		function()
			flow=flow.."6"
			PAR(
				function()
					local ch = Channel:new()
					ch:IN()
				end,
				function()
					local ch = Channel:new()
					ch:OUT()
				end
			)
		end
	)
	checkEquals("wrong flow", "f1234567", flow )
	endTickCheck( self, 1)
end

function termination:onInput()
	startTickCheck( self )
	local flow = "f"
	local ch = Channel:new()
	PARWHILE(
		function()
			flow=flow.."1"
			ch:IN()
			flow=flow.."3"
		end,
		function()
			flow=flow.."2"
			ch:OUT()
			flow=flow.."-"
		end
	)
	checkEquals("wrong flow", "f123", flow )
	endTickCheck( self, 0)
end

