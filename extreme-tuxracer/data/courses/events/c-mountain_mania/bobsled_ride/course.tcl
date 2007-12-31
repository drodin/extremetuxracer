## Course configuration
#tux_course_name "Bobsled Ride"
tux_course_author "Ascii Monster <asciimonster@myrealbox.com>"
tux_course_dim 100 1900 60 1800 ;# width, length of course in m
tux_start_pt 45 3.5            ;# start position, measured from left rear corner
tux_angle 30                   ;# angle of course (was 23)
tux_elev_scale 8.0             ;# amount by which to scale elevation data
tux_base_height_value 202      ;# greyscale value corresponding to height 
tux_elev elev.png              ;# bitmap specifying course elevations
tux_terrain terrain.png        ;# bitmap specifying terrains type                              ;#     offset of 0 (integer from 0 - 255)tux_elev elev.rgb              ;# bitmap specifying course elevationstux_terrain terrain.rgb        ;# bitmap specifying terrains type
tux_course_init