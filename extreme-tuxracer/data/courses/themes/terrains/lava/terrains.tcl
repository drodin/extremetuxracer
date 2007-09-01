
# icy terrains

# hard/rock terrains

tux_terrain_tex -name lavastone -texture lavastone.png \
	-color {160 0 0} \
	-friction 0.9 \
	-compression 0.01 \
	-sound rock_sound \
    -wheight 160

tux_terrain_tex -name lava -texture lava.png \
	-color {255 121 23} \
	-friction 0.9 \
	-compression 0.01 \
	-envmap_texture terrain_envmap \
	-sound ice_sound \
    -wheight 161

#snowy/soft terrains
