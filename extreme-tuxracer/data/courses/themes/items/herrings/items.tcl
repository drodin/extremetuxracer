
tux_load_texture fish2 herring_red.png 0
tux_item_spec -name herringred -type herring -diameter 1.0 -height 1.0 \
      -texture fish2 -color {0 255 0} -above_ground 0.2 


tux_load_texture fish3 herring_green.png 0
tux_item_spec -name herringgreen -type herring -diameter 1.0 -height 1.0 \
      -texture fish3 -color {255 170 0} -above_ground 0.2 

tux_load_texture star star.png 0
tux_item_spec -name star -type herring -diameter 1.0 -height 1.0 \
      -texture star -color {188 79 105} -above_ground 0.2 -score 5

tux_load_texture deadfish herring_dead.png 0
tux_item_spec -name deadfish -type herring -diameter 1.0 -height 2.0 \
      -texture deadfish -color {192 192 0} -above_ground 0.2 -score -10
