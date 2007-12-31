tux_open_courses { \
        { \
            -course events/d-niehoff_experience/challenge_one -name "Challenge One" \
                    -description "A very dangerous path along the mountains. Try to catch more than 60 herrings. Warning: The course is very difficult and there are some situations where you can get stuck." \
                    -par_time 80.0 \
        } \
		{ \
            -course events/d-niehoff_experience/chinese_wall -name "Chinese Wall" \
                    -description "The Great Wall of China in winter. Try to stay on top of the wall - that's where the herrings are. If you fall off you can try to jump back up, there are 7 hidden ramps. You'll get a lot of exercise." \
                    -par_time 80.0 \
        } \
		{ \
            -course events/d-niehoff_experience/frozen_lakes -name "Frozen Lakes" \
                    -description "To reach the finish you must pass several frozen lakes. Watch your speed on the ice - there are some dangerous corners." \
                    -par_time 80.0 \
        } \
		{ \
            -course events/d-niehoff_experience/explore_mountains -name "Explore the Mountains" \
                    -description "Can you find your way through the icy, dangerous mountains? I doubt it." \
                    -par_time 80.0 \
        } \
		{ \
            -course events/d-niehoff_experience/in_search_of_the_holy_grail -name "In Search of the Holy Grail" \
                    -description "An adventure course with a simple task: Find the grail! Warning: It is very difficult to find the way." \
                    -par_time 80.0 \
        } \
		{ \
            -course events/d-niehoff_experience/tux_at_home -name "Secret Valleys" \
                    -description "Your task: Find the hidden valleys and the ten groups of dead trees. Look around, the ways are not easy to find." \
                    -par_time 80.0 \
        } \
		{ \
            -course events/d-niehoff_experience/tux_at_home -name "Tux at Home" \
                    -description "Here lives Tux. You can take the corridore on the right - it's the shortest and fastest way. But if you want to catch some herrings you have to enter the rooms on the left." \
                    -par_time 80.0 \
        } \
		{ \
            -course events/d-niehoff_experience/wild_mountains -name "Wild Mountains" \
                    -description "A wild landscape of ice and snow. It isn't easy to keep the right way. Not for beginners." \
                    -par_time 80.0 \
        } \

    } 


tux_events {
    {
        -name "The Niehoff Experience" -icon herring_run_icon -cups {
            {
                -name "Search, Search, Search" -icon cup_icon -races {

					{
                        -course events/d-niehoff_experience/secret_valleys \
                                -name "Secret Valleys" \
                                -description "Your task: Find the hidden valleys and the ten groups of dead trees. Look around, the ways are not easy to find." \
                                -herring { 50 50 50 50 } \
                                -time { 0 165 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions night \
                                -windy no -snowing no
                    }
					{
                        -course events/d-niehoff_experience/in_search_of_the_holy_grail \
                                -name "In Search of the Holy Grail" \
                                -description "An adventure course with a simple task: Find the grail! Warning: It is very difficult to find the way." \
                                -herring { 20 20 20 20 } \
                                -time { 0 110 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions evening \
                                -windy no -snowing no
                    }
					{
                        -course events/d-niehoff_experience/tux_at_home \
                                -name "Tux at Home" \
                                -description "Here lives Tux. You can take the corridore on the right - it's the shortest and fastest way. But if you want to catch some herrings you have to enter the rooms on the left." \
                                -herring { 20 20 20 20 } \
                                -time { 0 180 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
                }
             }            
        
              {
               -name "Mountains and Hills" -icon cup_icon -races {
                    {
                        -course events/d-niehoff_experience/explore_mountains \
                                -name "Explore the Mountains" \
                                -description "Can you find your way through the icy, dangerous mountains. I doubt it. " \
                                -herring { 36 36 36 36 } \
                                -time { 0 160 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions cloudy \
                                -windy no -snowing no
                    }
                    {
                        -course events/d-niehoff_experience/chinese_wall \
                                -name "Chinese Wall" \
                                -description "The Great Wall of China in winter. Try to stay on top of the wall - that's where the herrings are. If you fall off you can try to jump back up, there are 7 hidden ramps. You'll get a lot of exercise." \
                                -herring { 50 50 50 50 } \
                                -time { 0 160 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions evening \
                                -windy no -snowing no
                    }
					{
                        -course events/d-niehoff_experience/frozen_lakes \
                                -name "Frozen Lakes" \
                                -description "To reach the finish you must pass several frozen lakes. Watch your speed on the ice - there are some dangerous corners." \
                                -herring { 30 30 30 30 } \
                                -time { 0 120 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions night \
                                -windy no -snowing no
                    }
              }
			  }
			  {
               -name "The Final Challenge" -icon cup_icon -races {
                    {
                        -course events/a-tux_racer/challenge_one \
                                -name "Challenge One" \
                                -description "A very dangerous path along the mountains. Try to catch more than 60 herrings. Warning: The course is very difficult and there are some situations where you can get stuck." \
                                -herring { 5 5 5 5 } \
                                -time { 0 180 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
                    {
                        -course events/a-tux_racer/wild_mountains \
                                -name "Wild Mountains" \
                                -description "A wild landscape of ice and snow. It isn't easy to keep the right way. Not for beginners." \
                                -herring { 55 55 55 55 } \
                                -time { 0 140 0 0 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
              }
	      
	     }
		}
	}	
}