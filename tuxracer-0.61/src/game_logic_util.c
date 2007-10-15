/* 
 * Tux Racer 
 * Copyright (C) 1999-2001 Jasmin F. Patry
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "tuxracer.h"
#include "game_logic_util.h"
#include "multiplayer.h"
#include "list.h"
#include "course_mgr.h"
#include "save.h"

/*---------------------------------------------------------------------------*/
/*! 
  Returns true iff race was won
  \author  jfpatry
  \date    Created:  2000-09-24
  \date    Modified: 2000-09-24
*/
bool_t was_current_race_won()
{
    difficulty_level_t d;
    race_data_t *race;
    player_data_t *plyr;

    check_assertion( g_game.practicing == False,
		     "was_current_race_won() called in practice mode" );

    if ( g_game.race_aborted ) {
	return False;
    }

    d = g_game.difficulty;
    race = &g_game.race;
    plyr = get_player_data( local_player() );

    if ( g_game.time <= race->time_req[d] &&
	 plyr->herring >= race->herring_req[d] &&
	 plyr->score >= race->score_req[d] ) 
    {
	print_debug( DEBUG_GAME_LOGIC, "won race" );
	return True;
    } else {
	print_debug( DEBUG_GAME_LOGIC, "lost race" );
	return False;
    }
}


/*---------------------------------------------------------------------------*/
/*! 
  Returns true if the current race is the last race in the current cup
  \author  jfpatry
  \date    Created:  2000-09-26
  \date    Modified: 2000-09-26
*/
bool_t is_current_race_last_race_in_cup( void )
{
    race_data_t *race_data;
    list_elem_t elem;
    list_t race_list;

    check_assertion( 
	!g_game.practicing,
	"is_current_race_last_race_in_cup called in practice mode" );

    check_assertion( g_game.current_event != NULL,
		     "current event is null" );
    check_assertion( g_game.current_cup != NULL,
		     "current cup is null" );

    elem = get_event_by_name( g_game.current_event );
    check_assertion( elem != NULL, "couldn't find event" );
    
    elem = get_event_cup_by_name( (event_data_t*) get_list_elem_data( elem ),
				  g_game.current_cup );
    check_assertion( elem != NULL, "couldn't find cup" );

    race_list = get_cup_race_list( (cup_data_t*) get_list_elem_data( elem ) );

    check_assertion( race_list != NULL, "race list is null" );

    elem = get_list_tail( race_list );

    check_assertion( elem != NULL, "race list is empty" );

    race_data = (race_data_t*) get_list_elem_data( elem );

    if ( strcmp( race_data->name, g_game.race.name ) == 0 ) {
	return True;
    } else {
	return False;
    }
}


/*---------------------------------------------------------------------------*/
/*! 
  Returns true if latest race result beat best result in race
  \author  jfpatry
  \date    Created:  2000-09-26
  \date    Modified: 2000-09-26
*/
bool_t did_player_beat_best_results( void )
{
    player_data_t *plyr = get_player_data( local_player() );
    scalar_t time;
    int herring;
    int score;

    if ( !get_saved_race_results( plyr->name,
				  g_game.current_event,
				  g_game.current_cup,
				  g_game.race.name,
				  g_game.difficulty,
				  &time,
				  &herring,
				  &score ) )
    {
	/* No previous result, so we didn't "beat" anything */
	return False;
    }

    if ( plyr->score > score ) {
	return True;
    } else {
	return False;
    }
}


/*---------------------------------------------------------------------------*/
/*! 
  Returns true if current cup is complete
  \author  jfpatry
  \date    Created:  2000-09-26
  \date    Modified: 2000-09-26
*/
bool_t is_current_cup_complete( void )
{
    event_data_t *event_data;
    list_elem_t elem;

    check_assertion( 
	!g_game.practicing,
	"is_current_cup_complete called in practice mode" );

    check_assertion( g_game.current_event != NULL,
		     "current event is null" );
    check_assertion( g_game.current_cup != NULL,
		     "current cup is null" );

    elem = get_event_by_name( g_game.current_event );
    check_assertion( elem != NULL, "couldn't find event" );

    event_data = (event_data_t*) get_list_elem_data( elem );
    
    elem = get_event_cup_by_name( event_data, g_game.current_cup );
    check_assertion( elem != NULL, "couldn't find cup" );

    return is_cup_complete( event_data, elem );
}



/*---------------------------------------------------------------------------*/
/*! 
  Updates a player's score; to be called after a race is complete
  \author  jfpatry
  \date    Created:  2000-09-24
  \date    Modified: 2000-09-24
*/
void update_player_score( player_data_t *plyr )
{
    scalar_t par_time;

    /* use easy time as par score */
    par_time = g_game.race.time_req[DIFFICULTY_LEVEL_EASY];
    plyr->score = max( 0, (int) (
	100*(par_time-g_game.time) + 200*plyr->herring ) );
}


/*---------------------------------------------------------------------------*/
/*! 
  Splits a floating point time value in minutes, seconds, and hundredths.
  \author  jfpatry
  \date    Created:  2000-09-24
  \date    Modified: 2000-09-24
*/
void get_time_components( scalar_t time, int *minutes, int *seconds,
			  int *hundredths )
{
    *minutes = (int) (time / 60);
    *seconds = ((int) time) % 60;
    *hundredths = ((int) (time * 100 + 0.5) ) % 100;
}

/* EOF */
