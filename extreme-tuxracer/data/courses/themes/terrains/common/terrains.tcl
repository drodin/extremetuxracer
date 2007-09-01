
# snow marks
tux_load_texture c_snow_head buttstart.png 1
tux_load_texture c_snow_mark buttprint.png 1
tux_load_texture c_snow_tail buttstop.png 1
tux_bind_texture c_snow_head c_snow_head
tux_bind_texture c_snow_mark c_snow_mark
tux_bind_texture c_snow_tail c_snow_tail

# snow Particle
tux_load_texture c_snow_particle snowparticles.png 0
tux_bind_texture c_snow_particle c_snow_particle

# icy terrains

tux_terrain_tex -name ice -texture ice.png \
	-color {0 0 0} \
	-friction 0.22 \
	-compression 0.03 \
	-envmap_texture terrain_envmap \
	-sound ice_sound \
    -wheight 0

# hard/rock terrains

tux_terrain_tex -name rock -texture rock.png \
	-color {128 128 128} \
	-friction 0.9 \
	-compression 0.01 \
	-sound rock_sound \
    -wheight 150

#snowy/soft terrains

tux_terrain_tex -name snow -texture snow.png \
	-color {255 255 255} \
	-friction 0.35 \
	-compression 0.11 \
	-particles c_snow_particle \
	-track_head c_snow_head \
	-track_mark c_snow_mark \
	-track_tail c_snow_tail \
	-sound snow_sound \
    -wheight 300
