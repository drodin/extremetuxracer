
tux_hud -hud 1 -type fsb \
	-position {12 12} \
	-font fps \
	-string "FPS: %.1f"



tux_hud -hud 1 -type herring \
	-position {-12 -21} \
	-font herring_count \
	-string "%03d"

tux_hud -hud 1 -type herring \
	-position {-12 -20} \
	-font herring_count_outline \
	-string "%03d"

tux_hud -hud 1 -type image \
   -position {12 -12} \
   -texture time_icon \
   -width 50 \
   -height 50 \
   -size 50

tux_hud -hud 1 -type image \
   -position {-70 -16} \
   -texture herring_icon \
   -width 50 \
   -height 50 \
   -size 50

tux_hud -hud 1 -type time \
	-position {60 -21} \
	-font time_value \
	-string "%02d:%02d:%02d"

tux_hud -hud 1 -type time \
	-position {60 -20} \
	-font herring_count_outline \
	-string "%02d:%02d:%02d"

tux_hud -hud1 -type gauge \
	-position {-256 12} \
	-width 127 \
	-height 103 \
	-size 128

tux_hud -hud 1 -type text \
	-position {-32 30} \
	-font speed_units \
	-string "km/h"

tux_hud -hud 1 -type speed \
	-position {-36 52} \
	-font speed_digits \
	-string "%.0f"

tux_hud -hud 1 -type text \
	-position {-32 29} \
	-font speed_units_outline \
	-string "km/h"

tux_hud -hud 1 -type speed \
	-position {-36 51} \
	-font speed_digits_outline \
	-string "%.0f"

tux_hud -hud 1 -type image \
	-position {-12 150} \
	-texture mask_outline2 \
	-width 33 \
	-height 128 \
	-size 128

tux_hud -hud 1 -type percentagebar \
	-position {-14 152} \
	-texture energy_mask \
	-width 29 \
	-height 124 \
	-angle 180
    
tux_hud -hud 1 -type objectives_time \
	-position {95 -54} \
    -font objectives_time \
    -string "%d:%d:%d"
    
tux_hud -hud 1 -type objectives_time_outline \
	-position {94 -54} \
    -font objectives_time_outline \
    -string "%d:%d:%d"    
    
tux_hud -hud 1 -type objectives_herring \
	-position {-13 -54} \
    -font objectives_herring \
    -string "%d"
    
tux_hud -hud 1 -type objectives_herring_outline \
	-position {-14 -54} \
    -font objectives_herring_outline \
    -string "%d"

