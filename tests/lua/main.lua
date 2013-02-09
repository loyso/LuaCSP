
function main()
	TestSuite.RUN_ALL()
end


function startTickCheck( self )
	self.t1 = tick()
end

function endTickCheck( self, expected, t1 )
	local t2 = tick()
	self.checkEqualsInt( "simulation tick difference", expected, t2-self.t1 )
end

function sleepTicks( ticks )
	for i=1,ticks do
		SLEEP(0)
	end
end


