#

# Course configuration

#

tux_course_name "Hippo Run"
tux_course_author "Adam Martin  <zopheus@yahoo.com>"
tux_course_dim 30 3500 30 3495        ;# width, length of course in m
tux_start_pt 16 3.5            ;# start position, measured from left rear corner
tux_angle 25                   ;# angle of course
tux_elev_scale 13.5             ;# amount by which to scale elevation data
tux_elev elev.png             ;# bitmap specifying course elevations
tux_terrain terrain.png        ;# bitmap specifying terrains type

tux_theme_init ppracer ;

tux_course_init

