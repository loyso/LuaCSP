
function main()
	log("main begin\n")
	testCppChannel()
	log("main end\n")
end

function testCppChannel()
	log("testCppChannel start\n")
	local ch = Channel:new()
	PARWHILE(
		function()
			SLEEP(10)
		end,
		function()
			KEYSTATE( ch, "a" )
		end,
		function()
			SLEEP(5)
			while true do
				local char = ch:IN()
				log(char)
			end
		end
	)
	log("\ntestCppChannel end\n")
end
