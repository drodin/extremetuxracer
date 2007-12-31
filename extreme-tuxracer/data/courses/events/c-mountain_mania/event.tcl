tux_open_courses { \
        { \
            -course events/c-mountain_mania/crazy_path -name "Crazy Path" \
                    -description "You'll have trouble slowing down on this one. There's lots of herring, but be careful not to fly too much or you won't get any." \
                    -par_time 80.0 \
        } \
		{ \
            -course events/c-mountain_mania/in_search_of_vodka -name "In Search of Vodka" \
                    -description "Tux needs some needs some vodka to warm up his cold belly. Alas, his liquor cabinet has been pillaged. Join Tux on the quest for vodka.Pick up herring for dinner along the way!" \
                    -par_time 80.0 \
        } \
		{ \
            -course events/c-mountain_mania/hey_tux -name "Hey, Tux!" \
                    -description "This course offers true excitement for your race-loving penguin buddy." \
                    -par_time 80.0 \
        } \
		{ \
            -course events/c-mountain_mania/hippo_run -name "Hippo Run" \
                    -description "In Hippo Run, you slide down an icy slope as a penguin, I honestly don't know what this has to do with hippos or running..." \
                    -par_time 80.0 \
        } \
		{ \
            -course events/c-mountain_mania/ice_pipeline -name "Ice Pipeline" \
                    -description "A short, slippery ice-trough with herring scattered throughout." \
                    -par_time 80.0 \
        } \
		{ \
            -course events/c-mountain_mania/volcanoes -name "Volcanoes" \
                    -description "This gigantic crater was full of water, and herring, but now it's all frozen, and yours for the taking." \
                    -par_time 80.0 \
        } \
        { \
            -course events/c-mountain_mania/slippy_slidey -name "Slippy-Slidey" \
                    -description "Slide on the slippery frozen river in a hilly forest, slipperily." \
                    -par_time 80.0 \
        } \
        { \
            -course events/c-mountain_mania/candy_lane -name "Candy Lane" \
                    -description "Mostly ice, with very few obstacles in your path, this is like a stroll down Candy Lane." \
                    -par_time 80.0 \
        } \
        { \
            -course events/c-mountain_mania/bobsled_ride -name "Bobsled Ride" \
                    -description "Just like a real bobsled ride, only more dangerous." \
                    -par_time 80.0 \
        } \
        { \
            -course events/c-mountain_mania/nature_stroll -name "Nature Stroll" \
                    -description "The fresh air, the pine scent, who doesn't like a nice refreshing walk through the mountains?" \
                    -par_time 80.0 \
        } \
        { \
            -course events/c-mountain_mania/merry_go_round -name "Merry-Go-Round" \
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
                        -course events/c-mountain_mania/crazy_path \
                                -name "Crazy Path" \
                                -description "You'll have trouble slowing down on this one. There's lots of herring, but be careful not to fly too much or you won't get any." \
                                -herring { 200 200 200 200 } \
                                -time { 0 110 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
					{
                        -course events/c-mountain_mania/candy_lane \
                                -name "Candy Lane" \
                                -description "Mostly ice, with very few obstacles in your path, this is like a stroll down Candy Lane." \
                                -herring { 35 35 35 35 } \
                                -time { 0 185 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
					{
                        -course events/c-mountain_mania/in_search_of_vodka \
                                -name "In Search of Vodka" \
                                -description "Tux needs some needs some vodka to warm up his cold belly. Alas, his liquor cabinet has been pillaged. Join Tux on the quest for vodka.Pick up herring for dinner along the way!" \
                                -herring { 60 60 60 60 } \
                                -time { 0 185 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions evening \
                                -windy no -snowing no
                    }
                 
               }    
      
            }
			{
                -name "Death Cup" -icon cup_icon -races {
                    {
                        -course events/c-mountain_mania/hey_tux \
                                -name "Hey, Tux!" \
                                -description "This course offers true excitement for your race-loving penguin buddy." \
                                -herring { 30 30 30 30 } \
                                -time { 0 90 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions night \
                                -windy no -snowing no
                    }
					{
                        -course events/c-mountain_mania/hippo_run \
                                -name "Hippo Run" \
                                -description "In Hippo Run, you slide down an icy slope as a penguin, I honestly don't know what this has to do with hippos or running..." \
                                -herring { 75 75 75 75 } \
                                -time { 0 180 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions cloudy \
                                -windy no -snowing no
                    }
					{
                        -course events/c-mountain_mania/ice_pipeline \
                                -name "Ice Pipeline" \
                                -description "A short, slippery ice-trough with herring scattered throughout." \
                                -herring { 60 60 60 60 } \
                                -time { 0 15 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
                 
               }    
      
            }
            {
                -name "Slippery Defeat" -icon cup_icon -races {
                    {
                        -course events/c-mountain_mania/volcanoes \
                                -name "Volcanoes" \
                                -description "This gigantic crater was full of water, and herring, but now it's all frozen, and yours for the taking." \
                                -herring { 55 55 55 55 } \
                                -time { 0 50 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions cloudy \
                                -windy no -snowing no
                    }
                    {
                        -course events/c-mountain_mania/slippy_slidey \
                                -name "Slippy-Slidey" \
                                -description "Slide on the slippery frozen river in a hilly forest, slipperily." \
                                -herring { 40 40 40 40 } \
                                -time { 0 180 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions night \
                                -windy no -snowing no
                    }
               } 
              }
            {
                -name "The Final Three" -icon cup_icon -races {
                    {
                        -course events/c-mountain_mania/bobsled_ride \
                                -name "Bobsled Ride" \
                                -description "Just like a real bobsled ride, only more dangerous." \
                                -herring { 70 70 70 70 } \
                                -time { 0 120 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions cloudy \
                                -windy no -snowing no
                    }
                    {
                        -course events/c-mountain_mania/merry_go_round \
                                -name "Merry Go Round" \
                                -description "Try to build up enough speed to jump from one roundabout to the next." \
                                -herring { 07 07 07 07 } \
                                -time { 0 75 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions evening \
                                -windy no -snowing no
                    }
                    {
                        -course events/c-mountain_mania/nature_stroll \
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

