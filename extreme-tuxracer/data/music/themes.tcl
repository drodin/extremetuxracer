# themes.tcl: Here we specify music themes

#
# Procedures to create music themes
#

# Used when we have different sounds
proc create_music_theme { name intro racing paused game_over } {
	tux_load_music ${name}_racing music/${racing}
	tux_load_music ${name}_paused music/${paused}
	tux_load_music ${name}_game_over music/${game_over}


	tux_bind_music ${name}_intro ${name}_intro 1
	tux_bind_music ${name}_racing ${name}_racing -1
	tux_bind_music ${name}_paused ${name}_paused -1
	tux_bind_music ${name}_game_over ${name}_game_over 1
}

# And this one when we have only two sounds; This looks so weird and is to preserve old Tux Racer music behaviour.
proc create_music_theme_simple { name intro game_over } {
	tux_load_music ${name}_intro music/${intro}
	tux_load_music ${name}_game_over music/${game_over}

	tux_bind_music ${name}_intro ${name}_intro -1
	tux_bind_music ${name}_game_over ${name}_game_over 1
}


#
# And here themes go
#

create_music_theme_simple tuxracer race1-jt.ogg wonrace1-jt.ogg

