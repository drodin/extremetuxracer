## Course configuration#
tux_course_name "Merry-Go-Round"
tux_course_author "Ascii Monster <asciimonster@myrealbox.com>"
tux_course_dim 150 650 120 600        ;# width, length of course in m
tux_start_pt 75 3.5           ;# start position, measured from left rear corner
tux_angle 24                   ;# angle of course
tux_elev_scale 7.0             ;# amount by which to scale elevation data
tux_base_height_value 0        ;# greyscale value corresponding to height
                               ;#     offset of 0 (integer from 0 - 255)
tux_elev elev.png              ;# bitmap specifying course elevations
tux_terrain terrain.png        ;# bitmap specifying terrains type
tux_course_init