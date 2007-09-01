tux_item_spec -name float -type none -color {255 128 255} -reset_point

tux_load_texture fish herring_standard.png 0
tux_item_spec -name herring -type herring -diameter 1.0 -height 1.0 \
      -texture fish -color {28 185 204} -above_ground 0.2


tux_load_texture flag1 flag.png 0
tux_item_spec -name flag -type none -diameter 1.0 -height 1.0 \
      -texture flag1 -color {194 40 40}


tux_load_texture start start.png 0
tux_item_spec -name start -type none -diameter 9.0 -height 6.0 \
		-texture start -color {128 128 0} \
                -normal {0 0 1}
      

tux_load_texture finish finish.png 0
tux_item_spec -name finish -type none -diameter 9.0 -height 6.0 \
		-texture finish -color {255 255 0} \
                -normal {0 0 1}


