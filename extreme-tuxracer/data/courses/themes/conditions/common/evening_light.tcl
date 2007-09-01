tux_course_light 0 -on -position { 1 1 0 0 } \
	-diffuse {0.7 0.6 0.5 1.0} \
	-ambient {0.4 0.3 0.3 1.0} 
  
tux_course_light 1 -on -position { 1 1 2 0 } \
	-specular { 0.9 0.6 0.3 1 } 


tux_load_texture envmap eveningenv.png 0
tux_bind_texture terrain_envmap envmap

tux_fog -on -mode linear -density 0.005 -color { 0.45 0.3 0.15 1 } -start 0 \
    -end [tux_get_param forward_clip_distance]

tux_particle_color { 0.9 0.7 0.35 1.0 }

tux_load_texture sky_front eveningfront.png 0
tux_load_texture sky_right eveningright.png 0
tux_load_texture sky_left eveningleft.png 0
tux_load_texture sky_back eveningback.png 0
tux_load_texture sky_top eveningtop.png 0
tux_load_texture sky_bottom eveningbottom.png 0

tux_bind_texture sky_front sky_front
tux_bind_texture sky_right sky_right
tux_bind_texture sky_left sky_left
tux_bind_texture sky_back sky_back
tux_bind_texture sky_top sky_top
tux_bind_texture sky_bottom sky_bottom
