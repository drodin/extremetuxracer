## Course configuration#
tux_course_name "Crazy Path"
tux_course_author "Crazywater <Crazytofire@web.de>"
tux_course_dim 60 2500 60 2490       ;# width, length of course in m
tux_start_pt 30 3.5           ;# start position, measured from left rear corner
tux_angle  30                  ;# angle of course
tux_elev_scale 25             ;# amount by which to scale elevation data
tux_base_height_value 255     ;# greyscale value corresponding to height                               ;#     offset of 0 (integer from 0 - 255)
tux_elev elev.png              ;# bitmap specifying course elevations
tux_terrain terrain.png   
tux_theme_init ppracer ;    
tux_course_init