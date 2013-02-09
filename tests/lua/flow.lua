
flow = TestSuite:new()

function flow:precedenceAfterTick()
	startTickCheck( self )

	local flow = "f"

	PAR(
		function()
			PAR(
				function()
					flow=flow.."0"
					SLEEP(0)
					flow=flow.."4"
				end
				,
				function()
					flow=flow.."1"
					SLEEP(0)
					flow=flow.."5"
				end
			)
			flow=flow.."6"
			SLEEP(0)
			flow=flow.."a"
		end
		,
		function()
			PAR(
				function()
					flow=flow.."2"
					SLEEP(0)
					flow=flow.."7"
				end
				,
				function()
					flow=flow.."3"
					SLEEP(0)
					flow=flow.."8"
				end
			)
			flow=flow.."9"
			SLEEP(0)
			flow=flow.."b"
		end
	)

	endTickCheck( self, 2 )
	checkEquals( "wrong simulation step order", "f0123456789ab", flow )
end

function flow:precedenceInOut()
	startTickCheck( self )

	local flow="f"

	local ch1 = Channel:new()
	local ch2 = Channel:new()
	PAR(
		function()
			flow=flow.."0"
			SLEEP(0)
			ch1:OUT()
			flow=flow.."5"
		end
		,
		function()
			flow=flow.."1"
			SLEEP(0)
			ch2:IN()
			flow=flow.."3"
			ch1:IN()
			flow=flow.."4"
		end
		,
		function()
			flow=flow.."2"
			SLEEP(0)
			ch2:OUT()
			flow=flow.."6"
		end
	)

	endTickCheck( self, 1 )

	checkEquals( "wrong simulation step order", "f0123456", flow )
end
