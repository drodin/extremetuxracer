
# snow types -- see snow.txt for explanations on how it works
# use type 0 and 1 for "decorative" purposes, type 2 for snowfalls (if you want snow to disorientate the player)
# tux_snow_type <index> <speed> <minSize> <maxSize> <MAXPART> <MAXNEAR>
#tux_snow_type   0       8.0     0.15      0.45      30        20       #light snow
#tux_snow_type   1       8.0     0.15      0.45      64        30       #medium snow
#tux_snow_type   2       10.0    0.35      0.65      100       60       #hard snow

#light snow -- "decorative" (when the weather is sunny :-) )
tux_snow_type    0       8.0     0.15      0.45      64        30
#medium snow - ambient snow, adds a nice visual touch to a sunset level
tux_snow_type    1       13.0    0.35      0.65      250       150
#hard snow -- "snowfall" mode, will make the race more difficult ! Ex. twisty slope in foggy weather :-P
tux_snow_type    2       18.0    0.35      0.65      700       700


# snow marks
tux_load_texture c_snow_head buttstart.png 1
tux_load_texture c_snow_mark buttprint.png 1
tux_load_texture c_snow_tail buttstop.png 1
tux_bind_texture c_snow_head c_snow_head
tux_bind_texture c_snow_mark c_snow_mark
tux_bind_texture c_snow_tail c_snow_tail

#cracks
tux_load_texture crack crack2.png 1
tux_bind_texture crack crack

# snow Particle
tux_load_texture c_snow_particle snowparticles.png 0
tux_bind_texture c_snow_particle c_snow_particle

#snow flakes
tux_bind_texture c_snow_flake0 c_snow_particle
tux_load_texture c_snow_flake1 snowflake1.png 0
tux_bind_texture c_snow_flake1 c_snow_flake1
tux_load_texture c_snow_flake2 snowflake2.png 0
tux_bind_texture c_snow_flake2 c_snow_flake2
tux_load_texture c_snow_flake3 snowflake3.png 0
tux_bind_texture c_snow_flake3 c_snow_flake3


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
