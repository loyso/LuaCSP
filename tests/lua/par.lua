
testpar = TestSuite:new()

function testpar:elementary()
	startTickCheck( self )
	local flow = "f"
	PAR(
		function()
			flow = flow.."1"
			SLEEP(0)
			flow = flow.."5"
		end,
		function()
			flow = flow.."2"
			PAR(
				function()
					flow = flow.."3"
					SLEEP(0)
					flow = flow.."6"
				end,
				function()
					flow = flow.."4"
					SLEEP(0)
					flow = flow.."7"
				end
			)
			flow = flow.."8"
		end
	)
	flow = flow.."9"
	checkEquals("wrong flow", "f123456789", flow )
	SLEEP(0)
	endTickCheck( self, 2)
end

function testpar:emptyPar()
	startTickCheck( self )
	local flow = "f"
	PAR(
		function()
			flow = flow.."1"
		end,
		function()
			flow = flow.."2"
		end
	)
	checkEquals("wrong flow", "f12", flow )
	endTickCheck( self, 0)
end

function testpar:emptyNestedPar()
	startTickCheck( self )
	local flow = "f"
	PAR(
		function()
			flow = flow.."1"
		end,
		function()
			flow = flow.."2"
			PAR(
				function()
					flow = flow.."3"
				end,
				function()
					flow = flow.."4"
				end
			)
			flow = flow.."5"
		end
	)
	checkEquals("wrong flow", "f12345", flow )
	endTickCheck( self, 0)
end

function testpar:parNested()
	startTickCheck( self )
	local ch = Channel:new()
	local flow = "f"
	PAR(
		function()
			flow = flow.."1"
			PAR(
				function()
					flow = flow.."2"
				end,
				function()
					flow = flow.."3"
					ch:IN()
					flow = flow.."5"
				end
			)
			flow = flow.."6"
		end,
		function()
			flow = flow.."4"
			ch:OUT()
			flow = flow.."7"
		end,
		function()
			flow = flow.."8"
		end,
		function()
			flow = flow.."9"
		end,
		function()
			flow = flow.."a"
		end,
		function()
			flow = flow.."b"
		end
	)
	checkEquals("wrong flow", "f123456789ab", flow )
	endTickCheck( self, 0)
end

