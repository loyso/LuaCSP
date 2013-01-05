//[ animation_heartbeat
function game( actor, sound_bank )
	PAR(
		function()
			actor:PLAY_ANIMATION("run")
		end
		,
		function()
			local step = actor:Channel("step")
			local heartbeat = actor:Channel("heartbeat")
			while true do
				ALT(
					step, function()
						sound_bank.running.step:play_and_forget()
					end
					,
					heartbeat, function()
						sound_bank.running.heartbeat:play_and_forget()
					end
				)
			end
		end
	)
end
//]