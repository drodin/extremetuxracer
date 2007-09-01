


tux_open_courses { \
        { \
            -course events/herring_run/frozen_river -name "Frozen River" \
                    -description "Keep your speed down to collect herring!" \
                    -par_time 80.0 \
        } \
        { \
            -course events/herring_run/path_of_daggers -name "Path of Daggers" \
                    -description "Big spikes may prove to be the least of your worries on this course. Muliple paths allow you to take low or high roads, each with their own challenges." \
                    -par_time 70.0 \
        } \
        { \
            -course events/herring_run/bunny_hill -name "Bunny Hill" \
                    -description "Use clever turnings to conquer the Bunny Hill." \
                    -par_time 40.0 \
        } \
        { \
            -course events/herring_run/twisty_slope -name "Twisty Slope" \
                    -description "Tight twists make grabbing herring difficult.  Hard turns will lead you to victory." \
                    -par_time 40.0 \
        } \
        { \
            -course events/herring_run/bumpy_ride -name "Bumpy Ride" \
                    -description "This hill has a series of ramps tackle.  Make sure to line yourself up before getting airborne." \
                    -par_time 40.0 \
        } \
        { \
	    -course events/herring_run/penguins_cant_fly \
            -name "Who Says Penguins Can't Fly?" \
            -description "Keep your speed up and try to collect a few herings." \
            -par_time 85.0 \
        } \
        { \
            -course events/herring_run/ski_jump -name "Ski Jump" \
                    -description "Like real ski jump, but there are no extra points for jumped distance." \
                    -par_time 20.0 \
        } \
        { \
            -course events/herring_run/tux-toboggan_run -name "Tux Toboggan Run" \
                    -description "Collect herrings in an ice channel with various paths and many high and low speed passages." \
                    -par_time 270.0 \
        } \
        { \
            -course events/herring_run/slalom -name "Slalom" \
                    -description "You have to do many hard turnings to collect all herrings in a slalom course." \
                    -par_time 120.0 \
        } \
        { \
            -course events/herring_run/high_road -name "High Road" \
                    -description "Stay In The Air Or Defeat The Trees." \
                    -par_time 59.0 \
        } \
        { \
            -course events/herring_run/keep_it_up -name "Keep It Up" \
                    -description "Stay In The Air Or Die." \
                    -par_time 59.0 \
        } \
        { \
            -course events/herring_run/ive_got_a_woody -name "I've Got A Woody" \
                    -description "Stay In The Air Or Eat The Trees." \
                    -par_time 39.0 \
        } \
        { \
            -course events/herring_run/ice_labyrinth -name "Ice Labyrinth" \
                    -description "Many paths are available to you, but some have more herrings than others!" \
                    -par_time 95.0 \
        } \
        { \
            -course events/herring_run/mount_herring -name "Mt. Herring" \
                    -description "Mt. Herring is relatively fast but long, somewhat bumpy in places, and has some wide open areas and branching paths. Lots of herrings to collect though it can be difficult unless you watch your speed." \
                    -par_time 230.0 \
        } \
        { \
            -course events/herring_run/hamburger_hill -name "Hamburger Hill" \
                    -description "Hamburger Hill is steep, narrow, twisty, lumpy, icy, and rocky, with a few trees right in your way!" \
                    -par_time 220.0 \
        } \
        { \
            -course events/herring_run/deadman -name "Dead Man's Drop" \
                    -description "Pretty tough hill. It's fast, steep, lumpy, twisty, and full of trees. Hard to get herrings. Contains a *BIG* surprise!" \
                    -par_time 60.0 \
        } \
        { \
            -course events/herring_run/mount_satan -name "Mt. Satan" \
                    -description "Mount Satan is a sister mountain to Dead Man's Drop. No surprises here, just a steep, bumpy, twisty path full of trees to get through." \
                    -par_time 120.0 \
        } \
        { \
            -course events/herring_run/snow_valley -name "Snow Valley" \
                    -description "Snow Valley is a relatively flat, and very fast course. You'll find a few herrings very difficult to get if you're going for a perfect score! Watch your speed in certain areas to get the most herrings!" \
                    -par_time 39.0 \
        } \
        { \
            -course events/herring_run/hazzard_valley -name "Hazzard Valley" \
                    -description "Hazzard Valley is full of hazards, like trees, deep caverns, spikes, and huge areas of rocky plains, not to mention your speed being your enemy in trying to get the herrings and stay on course!" \
                    -par_time 90.0 \
        } \
        { \
            -course events/herring_run/skull_mountain -name "Skull Mountain" \
                    -description "Skull mountain is a steep, twisty drop around treefilled inner peaks. Expect to miss a lot of herrings and crash a lot." \
                    -par_time 110.0 \
        } \
        { \
            -course events/herring_run/the_narrow_way -name "The Narrow Way" \
                     -description "To put it simply, stay on the path or you lose! It's hard to see where you're going, there's trees in your way, and the herrings are hard to get. Good luck, you'll need it!" \
	             -par_time 65.0 \
        } \

    } 


tux_events {
    {
        -name "Herring Run" -icon herring_run_icon -cups {
            {
                -name "Canadian Cup" -icon cup_icon -races {
                    {
                        -course events/herring_run/bunny_hill \
                                -name "Bunny Hill" \
                                -description "Use clever turning to conquer the Bunny Hill." \
                                -herring { 23 23 23 23 } \
                                -time { 37 35 32 30 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
                    {
                       -course events/herring_run/twisty_slope \
                                -name "Twisty Slope" \
                                -description "Tight twists make grabbing herring difficult.  Hard turns will lead you to victory." \
                                -herring { 24 24 24 24 } \
                                -time { 43 40 34 31.5 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions night \
                                -windy no -snowing no
                    }
                    {
                        -course events/herring_run/bumpy_ride \
                                -name "Bumpy Ride" \
                                -description "This hill has a series of ramps tackle.  Make sure to line yourself up before getting airborne." \
                                -herring { 17 17 17 17 } \
                                -time { 36 34 32 30 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions cloudy\
                                -windy no -snowing no
                    }
                }
             }            
        
              {
               -name "Mountain Man" -icon cup_icon -races {
                    {
                        -course events/herring_run/ice_labyrinth \
                                -name "Ice Labyrinth" \
                                -description "Many paths are available to you, but some have more herrings than others!" \
                                -herring { 18 18 18 18 } \
                                -time { 107 106 105 104 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions evening \
                                -windy no -snowing no
                    }
                    {
                        -course events/herring_run/mount_herring \
                                -name "Mt. Herring" \
                                -description "Mt. Herring is relatively fast but long, somewhat bumpy in places, and has some wide open areas and branching paths. Lots of herrings to collect though it can be difficult unless you watch your speed." \
                                -herring { 95 95 95 95 } \
                                -time { 240 235 230 225 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
                    {
                        -course events/herring_run/hamburger_hill \
                                -name "Hamburger Hill" \
                                -description "Hamburger Hill is steep, narrow, twisty, lumpy, icy, and rocky, with a few trees right in your way!" \
                                -herring { 80 80 80 80 } \
                                -time { 220 215 210 205 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions cloudy \
                                -windy no -snowing no
                    }
                    {
                        -course events/herring_run/deadman \
                                -name "Dead Man's Drop" \
                                -description "Pretty tough hill. It's fast, steep, lumpy, twisty, and full of trees. Hard to get herrings. Contains a *BIG* surprise!" \
                                -herring { 100 100 100 100 } \
                                -time { 125 120 115 110 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
                    {
                        -course events/herring_run/mount_satan \
                                -name "Mt. Satan" \
                                -description "Mount Satan is a sister mountain to Dead Man's Drop. No surprises here, just a steep, bumpy, twisty path full of trees to get through." \
                                -herring { 69 69 69 69 } \
                                -time { 135 130 125 120 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions night \
                                -windy no -snowing no
                    }
              }
	      
	     }

              {
               -name "Valley Cup" -icon cup_icon -races {
                    {
                        -course events/herring_run/snow_valley \
                                -name "Snow Valley" \
                                -description "Snow Valley is a relatively flat, and very fast course. You'll find a few herrings very difficult to get if you're going for a perfect score! Watch your speed in certain areas to get the most herrings!" \
                                -herring { 42 42 42 42 } \
                                -time { 45 43 41 39 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions evening \
                                -windy no -snowing no
                    }
                    {
                        -course events/herring_run/hazzard_valley \
                                -name "Hazzard Valley" \
                                -description "Hazzard Valley is full of hazards, like trees, deep caverns, spikes, and huge areas of rocky plains, not to mention your speed being your enemy in trying to get the herrings and stay on course!" \
                                -herring { 68 68 68 68 } \
                                -time { 105 100 95 90 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
                    {
                        -course events/herring_run/skull_mountain \
                                -name "Skull Mountain" \
                                -description "Skull mountain is a steep, twisty drop around tree-filled inner peaks. Expect to miss a lot of herrings and crash a lot." \
                                -herring { 25 25 25 25 } \
                                -time { 116 114 112 110 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }

{
                        -course events/herring_run/the_narrow_way \
                                -name "The Narrow Way" \
                                -description "To put it simply, stay on the path or you lose! It's hard to see where you're going, there's trees in your way, and the herrings are hard to get. Good luck, you'll need it!" \
                                -herring { 23 23 23 23 } \
                                -time { 68 67 66 65 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions cloudy \
                                -windy no -snowing no
                    }
              }
	      
	     }
        
             {
            
                -name "Tux the winter sportsman" -icon cup_icon -races {
                    {
                        -course events/herring_run/penguins_cant_fly \
                                -name "Who Says Penguins Can't Fly?" \
                                -description "Keep your speed up and try to collect a few herings." \
                                -herring { 8 8 8 8 } \
                                -time { 88 86 84 8 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
                    {
                                -course events/herring_run/ski_jump \
                                -name "Ski Jump" \
                                -description "Like real ski jump, but there are no extra points for jumped distance." \
                                -herring { 14 14 14 14 } \
                                -time { 26 25 25 25 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions cloudy \
                                -windy no -snowing no
                    }
                    {
                                -course events/herring_run/tux-toboggan_run \
                                -name "Tux Toboggan Run" \
                                -description "Collect herrings in an ice channel with various paths and many high and low speed passages." \
                                -herring { 140 145 150 155 } \
                                -time { 275 265 265 265 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
                    {
                                -course events/herring_run/slalom \
                                -name "Slalom" \
                                -description "You have to do many hard turnings to collect all herrings in a slalomcourse." \
                                -herring { 65 65 65 65 } \
                                -time { 80 79 78 77 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions evening \
                                -windy no -snowing no
                    }
                  
                }
              
              }
                     {  -name "Highway To Hell" -icon cup_icon -races {
                    {
                        -course events/herring_run/high_road \
                                -name "High Road" \
                                -description "Stay In The Air Or Defeat The Trees." \
                                -herring { 15 15 15 15 } \
                                -time { 60 59 58 47 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions night \
                                -windy no -snowing no
                    }
                    {
                        -course events/herring_run/keep_it_up \
                                -name "Keep It Up" \
                                -description "Stay In The Air Or Fall To Your Death." \
                                -herring { 25 25 25 25 } \
                                -time { 60 59 58 47 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions night \
                                -windy no -snowing no
                    }
                    {
                        -course events/herring_run/ive_got_a_woody \
                                -name "I've Got A Woody" \
                                -description "Stay In The Air Or Eat The Trees." \
                                -herring { 13 13 13 13 } \
                                -time { 40 39 38 37 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions night \
                                -windy no -snowing no
                    }


               }    
      
            }
        }        
    }
}
