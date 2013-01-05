//[ contract_stages
Stages = Contract:table()
Stages.stage1 = Channel
Stages.stage2 = Channel
Stages.stage3 = Channel -- 3 messages

function process1( contract )
	contract.stage1:IN()
	contract.stage2:IN()
	for i=1,3 do
		contract.stage3:IN()
	end
end

function process2( contract )
	contract.stage1:OUT()
	contract.stage2:OUT()
	for i=1,3 do
		contract.stage3:OUT()
	end
end

function start()
	local contract = Stages:new() -- spawn channels
	PAR(
		function()
			process1( contract )
		end,
		function()
			process2( contract )
		end
	)
end
//]

//[ contract_input
UserInput = Contract:table()
UserInput.axis 		= Gamepad.Channels.axis
UserInput.buttons 	= Gamepad.Channels.buttons
UserInput.dpad 		= Gamepad.Channels.dpad

function processInput()
	local ui = UserInput:new()
	local processInput = true
	while processInput do
		ALT(
			ui.axis, function( code, filtered_value, raw_value )
				log( "axis:", code, filtered_value, raw_value, "\n" )
			end
			,
			ui.buttons, function( code, state )
				log( "buttons:", code, state, "\n" )
			end
			,
			ui.dpad, function( code, state )
				log( "dpad:", code, state, "\n" )
			end
		)
	end
end
//]
