# mud marks
tux_load_texture mud_head mudstart.png 1
tux_load_texture mud_mark mudprint.png 1
tux_load_texture mud_tail mudstop.png 1
tux_bind_texture mud_head mud_head
tux_bind_texture mud_mark mud_mark
tux_bind_texture mud_tail mud_tail

# mud particle
tux_load_texture mud_particle mudparticles.png 0
tux_bind_texture mud_particle mud_particle


# icy terrains

# hard/rock terrains

#snowy/soft terrains

tux_terrain_tex -name mud -texture mud.png \
	-color {176 128 80} \
	-friction 0.35 \
	-compression 0.11 \
	-particles mud_particle \
	-track_head mud_head \
	-track_mark mud_mark \
	-track_tail mud_tail \
	-sound snow_sound \
    -wheight 301

