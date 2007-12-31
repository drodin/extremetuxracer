#
# Course configuration (original course.tcl by Julien Canet <jools@free.fr>: "Jool's Big Mountain")
#
tux_course_name "Hey, Tux"
tux_course_author "Klaus Krechan <klaus@krechan.de>"
tux_course_dim 60 1200        ;# width, length of course in m
tux_start_pt 30 3.5           ;# start position, measured from left rear corner
tux_angle 24                   ;# angle of course
tux_elev_scale 7.0             ;# amount by which to scale elevation data
tux_base_height_value 0        ;# greyscale value corresponding to height
                               ;#     offset of 0 (integer from 0 - 255)
tux_elev elev.rgb              ;# bitmap specifying course elevations
tux_terrain terrain.rgb        ;# bitmap specifying terrains type

tux_course_init
