


tux_open_courses { \
        { \
            -course events/mountain_mania/volcanoes -name "Volcanoes" \
                    -description "This gigantic crater was full of water, and herring, but now it's all frozen, and yours for the taking." \
                    -par_time 80.0 \
        } \
        
        { \
            -course events/mountain_mania/chinese_wall -name "Chinese Wall" \
                    -description "The Great Wall of China in winter. Try to stay on top of the wall - that's where the herrings are. If you fall off you can try to jump back up, there are 7 hidden ramps. You'll get a lot of exercise." \
                    -par_time 80.0 \
        } \
        { \
            -course events/mountain_mania/slippy_slidey -name "Slippy-Slidey" \
                    -description "Slide on the slippery frozen river in a hilly forest, slipperily." \
                    -par_time 80.0 \
        } \
        { \
            -course events/mountain_mania/frozen_lakes -name "Frozen Lakes" \
                    -description "To reach the finish you must pass several frozen lakes. Watch your speed on the ice - there are some dangerous corners." \
                    -par_time 80.0 \
        } \
        { \
            -course events/mountain_mania/explore_mountains -name "Explore the Mountains" \
                    -description "Can you find your way through the icy, dangerous mountains. I doubt it." \
                    -par_time 80.0 \
        } \
        { \
            -course events/mountain_mania/candy_lane -name "Candy Lane" \
                    -description "Mostly ice, with very few obstacles in your path, this is like a stroll down Candy Lane." \
                    -par_time 80.0 \
        } \
        { \
            -course events/mountain_mania/bobsled_ride -name "Bobsled Ride" \
                    -description "Just like a real bobsled ride, only more dangerous." \
                    -par_time 80.0 \
        } \
        { \
            -course events/mountain_mania/nature_stroll -name "Nature Stroll" \
                    -description "The fresh air, the pine scent, who doesn't like a nice refreshing walk through the mountains?" \
                    -par_time 80.0 \
        } \
        { \
            -course events/mountain_mania/merry_go_round -name "Merry-Go-Round" \
                    -description "Try to build up enough speed to jump from one roundabout to the next." \
                    -par_time 80.0 \
        } \


    } \


tux_events {
    {
        -name "Mountain Mania" -icon herring_run_icon -cups {
            {
                -name "Icy Peril" -icon cup_icon -races {
                    {
                        -course events/mountain_mania/explore_mountains \
                                -name "Explore the Mountains" \
                                -description "Can you find your way through the icy, dangerous mountains. I doubt it. " \
                                -herring { 36 36 36 36 } \
                                -time { 0 160 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions evening \
                                -windy no -snowing no
                    }
                    {
                        -course events/mountain_mania/candy_lane \
                                -name "Candy Lane" \
                                -description "Mostly ice, with very few obstacles in your path, this is like a stroll down Candy Lane." \
                                -herring { 35 35 35 35 } \
                                -time { 0 185 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
                 
               }    
      
            }
            {
                -name "Slippery Defeat" -icon cup_icon -races {
                    {
                        -course events/mountain_mania/volcanoes \
                                -name "Volcanoes" \
                                -description "This gigantic crater was full of water, and herring, but now it's all frozen, and yours for the taking." \
                                -herring { 55 55 55 55 } \
                                -time { 0 50 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions cloudy \
                                -windy no -snowing no
                    }
                    {
                        -course events/mountain_mania/chinese_wall \
                                -name "Chinese Wall" \
                                -description "The Great Wall of China in winter. Try to stay on top of the wall - that's where the herrings are. If you fall off you can try to jump back up, there are 7 hidden ramps. You'll get a lot of exercise." \
                                -herring { 50 50 50 50 } \
                                -time { 0 160 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
                    {
                        -course events/mountain_mania/slippy_slidey \
                                -name "Slippy-Slidey" \
                                -description "Slide on the slippery frozen river in a hilly forest, slipperily." \
                                -herring { 40 40 40 40 } \
                                -time { 0 180 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions night \
                                -windy no -snowing no
                    }
                    {
                        -course events/mountain_mania/frozen_lakes \
                                -name "Frozen Lakes" \
                                -description "To reach the finish you must pass several frozen lakes. Watch your speed on the ice - there are some dangerous corners." \
                                -herring { 30 30 30 30 } \
                                -time { 0 120 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions evening \
                                -windy no -snowing no
                    }
                 
               } 
              }
            {
                -name "The Final Three" -icon cup_icon -races {
                    {
                        -course events/mountain_mania/bobsled_ride \
                                -name "Bobsled Ride" \
                                -description "Just like a real bobsled ride, only more dangerous." \
                                -herring { 70 70 70 70 } \
                                -time { 0 120 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions cloudy \
                                -windy no -snowing no
                    }
                    {
                        -course events/mountain_mania/merry_go_round \
                                -name "Merry Go Round" \
                                -description "Try to build up enough speed to jump from one roundabout to the next." \
                                -herring { 07 07 07 07 } \
                                -time { 0 75 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions evening \
                                -windy no -snowing no
                    }
                    {
                        -course events/mountain_mania/nature_stroll \
                                -name "Nature Stroll" \
                                -description "The fresh air, the pine scent, who doesn't like a nice refreshing walk through the mountains?" \
                                -herring { 30 30 30 30 } \
                                -time { 0 125 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions night \
                                -windy no -snowing no 
 				}
			}
		}
	}
}
}

