
testpar = TestSuite:new()

testpar.helpers = {}

function testpar.helpers:startTickCheck()
	self.t1 = tick()
end

function testpar.helpers:endTickCheck( expected, t1 )
	local t2 = tick()
	checkEqualsInt( "simulation tick difference", expected, t2-self.t1 )
end


function testpar:elementary()
	helpers:startTickCheck()
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
	helpers:endTickCheck(2)
end

function testpar:emptyPar()
	helpers:startTickCheck()
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
	helpers:endTickCheck(0)
end

function testpar:emptyNestedPar()
	helpers:startTickCheck()
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
	helpers:endTickCheck(0)
end

function testpar:parNested()
	helpers:startTickCheck()
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
			flow = flow.."b"
		end,
		function()
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
		end
	)
	checkEquals("wrong flow", "f123456789ab", flow )
	helpers:endTickCheck(0)
end

