


tux_open_courses { \
        { \
            -course events/a-tux_racer/bunny_hill -name "Bunny Hill" \
                    -description "Let's start out easy, just collect the herring in the time limit and you'll be fine. The best part is that you don't have to use those stupid T-lifts." \
                    -par_time 40.0 \
        } \
        { \
            -course events/a-tux_racer/twisty_slope -name "Twisty Slope" \
                    -description "Tight twists make grabbing herring difficult.  Hard turns and not too much braking will lead you to victory." \
                    -par_time 40.0 \
        } \
        { \
            -course events/a-tux_racer/bumpy_ride -name "Bumpy Ride" \
                    -description "This hill has a series of ramps to tackle.  Make sure to line yourself up before getting airborne." \
                    -par_time 40.0 \
        } \
		        { \
            -course events/a-tux_racer/frozen_river -name "Frozen River" \
                    -description "Keep your speed down to collect herring. You have plenty of time, so don't rush things." \
                    -par_time 80.0 \
        } \
        { \
            -course events/a-tux_racer/path_of_daggers -name "Path of Daggers" \
                    -description "Big spikes may prove to be the least of your worries on this course. Muliple paths allow you to take low or high roads, each with their own challenges." \
                    -par_time 70.0 \
        } \
    } 


tux_events {
    {
        -name "Tux Racer Classics" -icon herring_run_icon -cups {
            {
                -name "Canadian Cup" -icon cup_icon -races {
                    {
                        -course events/a-tux_racer/bunny_hill \
                                -name "Bunny Hill" \
                                -description "Let's start out easy, just collect the herring in the time limit and you'll be fine. The best part is that you don't have to use those stupid T-lifts." \
                                -herring { 23 23 23 23 } \
                                -time { 37 35 32 30 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
                    {
                       -course events/a-tux_racer/twisty_slope \
                                -name "Twisty Slope" \
                                -description "Tight twists make grabbing herring difficult.  Hard turns and not too much braking will lead you to victory." \
                                -herring { 24 24 24 24 } \
                                -time { 43 40 34 31.5 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions night \
                                -windy no -snowing no
                    }
                    {
                        -course events/a-tux_racer/bumpy_ride \
                                -name "Bumpy Ride" \
                                -description "This hill has a series of ramps to tackle.  Be sure to line yourself up before getting airborne." \
                                -herring { 17 17 17 17 } \
                                -time { 36 34 32 30 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions evening\
                                -windy no -snowing no
                    }
                }
             }            
        
              {
               -name "Swiss Cup" -icon cup_icon -races {
                    {
                        -course events/a-tux_racer/frozen_river \
                                -name "Frozen River" \
                                -description "Keep your speed down to collect herring. You have plenty of time, so don't rush things." \
                                -herring { 40 40 40 40 } \
                                -time { 107 120 105 104 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions cloudy \
                                -windy no -snowing no
                    }
                    {
                        -course events/a-tux_racer/path_of_daggers \
                                -name "Path of Daggers" \
                                -description "Big spikes may prove to be the least of your worries on this course. Muliple paths allow you to take low or high roads, each with their own challenges." \
                                -herring { 18 18 18 18 } \
                                -time { 240 60 230 225 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
              }
	      
	     }
		}
	}	
}