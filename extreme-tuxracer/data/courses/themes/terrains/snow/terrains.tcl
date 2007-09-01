# snow marks
tux_load_texture snow_head buttstart.png 1
tux_load_texture snow_mark buttprint.png 1
tux_load_texture snow_tail buttstop.png 1
tux_bind_texture snow_head snow_head
tux_bind_texture snow_mark snow_mark
tux_bind_texture snow_tail snow_tail

# dirtsnow marks
tux_bind_texture dirtsnow_head snow_head
tux_bind_texture dirtsnow_mark snow_mark
tux_bind_texture dirtsnow_tail snow_tail


# snow Particle
tux_load_texture snow_particle snowparticles.png 0
tux_bind_texture snow_particle snow_particle

# dirtsnow Particle
tux_load_texture dirtsnow_particle dirtsnowparticles.png 0
tux_bind_texture dirtsnow_particle dirtsnow_particle

# icy terrains

# hard/rock terrains

#snowy/soft terrains

tux_terrain_tex -name stsnow1 -texture stsnow1.png \
	-color {176 176 255} \
	-friction 0.35 \
	-compression 0.11 \
	-particles snow_particle \
	-track_head snow_head \
	-track_mark snow_mark \
	-track_tail snow_tail \
	-sound snow_sound \
    -wheight 350

tux_terrain_tex -name stsnow2 -texture stsnow2.png \
	-color {144 144 255} \
	-friction 0.35 \
	-compression 0.11 \
	-particles snow_particle \
	-track_head snow_head \
	-track_mark snow_mark \
	-track_tail snow_tail \
	-sound snow_sound \
    -wheight 351

tux_terrain_tex -name dirtsnow -texture dirtsnow.png \
	-color {192 192 192} \
	-friction 0.35 \
	-compression 0.11 \
	-particles dirtsnow_particle \
	-track_head dirtsnow_head \
	-track_mark dirtsnow_mark \
	-track_tail dirtsnow_tail \
	-sound snow_sound \
    -wheight 352

tux_terrain_tex -name dsnow -texture dsnow.png \
	-color {208 208 160} \
	-friction 0.35 \
	-compression 0.11 \
	-particles dirtsnow_particle \
	-track_head dirtsnow_head \
	-track_mark dirtsnow_mark \
	-track_tail dirtsnow_tail \
	-sound snow_sound \
    -wheight 353

tux_terrain_tex -name dsnow2 -texture dsnow2.png \
	-color {208 160 208} \
	-friction 0.35 \
	-compression 0.11 \
	-particles dirtsnow_particle \
	-sound snow_sound \
    -wheight 354

tux_terrain_tex -name snowygrass -texture snowygrass.png \
	-color {176 255 176} \
	-friction 0.5 \
	-compression 0.06 \
	-particles dirtsnow_particle \
	-track_head dirtsnow_head \
	-track_mark dirtsnow_mark \
	-track_tail dirtsnow_tail \
	-sound snow_sound \
    -wheight 355
