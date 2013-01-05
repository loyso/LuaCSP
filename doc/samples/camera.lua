//[ camera_main
function camera_main( actor, actor_bone, tollerance_settings, relative_dof_settings )
	local viewport = Viewport:new( 0, 0, 1, 1 )	

	local camera = Camera:new()	
	viewport.set_camera( camera )

	local ctrl1 = CameraController:new()
	local ctrl2 = CameraController:new()
	local blender = CameraLinearBlend:new( ctrl1, ctrl2 )

	camera:set_controller( blender )

	blender:set_blend( 0 ) -- 0 means ctrl1 at full strength, 1 - ctrl2 at full strength.
	
	PAR(						
		function()
			local track1 = CameraTracks.cinematics_spline1:new()
			camera_play_track_lookat( ctrl1, track1, actor, actor_bone, tollerance_settings, relative_dof_settings )
		end,
		function()
			local track2 = CameraTracks.cinematics_maintrack:new()
			camera_play_track( ctrl2, track2 )
		end,
		function()
			local transition_seconds1 = 5
			local transition_seconds2 = 3
			blender:LINEAR_FADE_TO( 1.0, transition_seconds1 ) -- fade to 1.0 in 5 seconds
			blender:EXPONENTIAL_FADE_TO( 0.0, transition_seconds2 ) -- fade back to 0.0 in 3 seconds 
		end		
	)
end
//]

//[ camera_play_track
function camera_play_track( ctrl, track )
	PAR(
		function()
			ctrl:PLAY_TRACK( track )
		end,
		function()
			ctrl:TRANSLATION_TRACK( 1, track )
		end,
		function()
			ctrl:ROTATION_TRACK( 1, track )
		end,
		function()
			ctrl:DOF_TRACK( track )
		end,
		function()
			ctrl:FOV_TRACK( track )
		end
	)
end
//]

//[ camera_play_track_lookat
function camera_play_track_lookat( ctrl, track, actor, actor_bone, tollerance_settings, relative_dof_settings )
	PAR(						
		function()
			ctrl:PLAY_TRACK( track )
		end,
		function()				
			ctrl:TRANSLATION_TRACK( 1, track )
		end,
		function()
			ctrl:ROTATION_TO_ACTOR( 1, actor, actor_bone, tollerance_settings )
		end,
		function()
			ctrl:DOF_TO_ACTOR( relative_dof_settings, actor, actor_bone, tollerance_settings )
		end,
		function()
			ctrl:FOV_TRACK( track )
		end
	)
end
//]
