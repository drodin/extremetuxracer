tux_course_light 0 -on -position { 1 1 0 0 } -diffuse { 0.39 0.51 0.88 1 } \
     -specular { 0.0 0.0 0.0 1 } -ambient { 0.0 0.09 0.34 1.0 }
     
tux_course_light 1 -on -position { 1 1 1 0 } -specular { 0.8 0.8 0.8 1 } 

tux_fog -on -mode linear -density 0.005 -color { 0.0 0.09 0.34 1 } -start 0 \
    -end [tux_get_param forward_clip_distance]

  
#
# Environmental sphere map
    
tux_load_texture alpine1-sphere nightenv.png 0
tux_bind_texture terrain_envmap alpine1-sphere

tux_particle_color { 0.39 0.51 0.88 1.0 }

if ![tux_get_param disable_background] {
tux_load_texture alpine1-front nightfront.png 0
tux_load_texture alpine1-right nightright.png 0
tux_load_texture alpine1-left nightleft.png 0
tux_load_texture alpine1-back nightback.png 0
tux_load_texture alpine1-top nighttop.png 0
tux_load_texture alpine1-bottom nightbottom.png 0

tux_bind_texture sky_front alpine1-front
tux_bind_texture sky_right alpine1-right
tux_bind_texture sky_left alpine1-left
tux_bind_texture sky_back alpine1-back
tux_bind_texture sky_top alpine1-top
tux_bind_texture sky_bottom alpine1-bottom
}
