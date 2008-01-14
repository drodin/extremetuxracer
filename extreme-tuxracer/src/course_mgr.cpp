/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
 *
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

#include "course_mgr.h"
#include "tcl_util.h"
#include "textures.h"

#include "game_mgr.h"

static char err_buff[BUFF_LEN];

static bool initialized = false;	/* has module been initialized? */

std::list<CourseData> openCourseList;	/* list of open courses */
std::list<EventData> eventList;		/* list of events */



static char *race_condition_names[RACE_CONDITIONS_NUM_CONDITIONS] =
{
    "sunny",
    "cloudy",
    "night",
	"evening"
};


/*---------------------------------------------------------------------------*/
/*! 
  Initializes the course manager module
  \return  None
  \author  jfpatry
  \date    Created:  2000-09-19
  \date    Modified: 2000-09-19
*/
void init_course_manager() 
{
    check_assertion( initialized == false,
		     "Attempt to initialize course manager twice" );

    initialized = true;
}

/*---------------------------------------------------------------------------*/
/*! 
  Creates an open_course_data_t object from a Tcl string.
  \author  jfpatry
  \date    Created:  2000-09-21
  \date    Modified: 2000-09-21
*/
CourseData* create_open_course_data( Tcl_Interp *ip, CONST84 char *string, 
					     char **err_msg )
{
    CONST84 char **argv = NULL;
    CONST84 char **orig_argv = NULL;
    int argc = 0;
    
    double par_time = 120;

    CourseData *open_course_data = new CourseData();

    if ( Tcl_SplitList( ip, string, &argc, &argv ) == TCL_ERROR ) {
		*err_msg = "open course data is not a list";
		goto bail_open_course_data;
    }

    orig_argv = argv;

    while ( *argv != NULL ) {
	if ( strcmp( *argv, "-course" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -course in open course data";
		goto bail_open_course_data;
	    }

	    open_course_data->course = *argv; //string_copy( *argv );
	} else if ( strcmp( *argv, "-name" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -name in open course data";
		goto bail_open_course_data;
	    }

	    open_course_data->name = *argv; //string_copy( *argv );
	} else if ( strcmp( *argv, "-description" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -description in open course data";
		goto bail_open_course_data;
	    }

	    open_course_data->description = *argv; //string_copy( *argv );
	} else if ( strcmp( *argv, "-contributed" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -contributed in open course data";
		goto bail_open_course_data;
	    }

	    open_course_data->contributed = *argv; //string_copy( *argv );
	}else if ( strcmp( *argv, "-par_time" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		par_time = 120.0;
		print_warning( PEDANTIC_WARNING,
			       "No data supplied for -par_time in open course "
			       "data.  Using %g seconds.", par_time );
	    } else if ( Tcl_GetDouble( ip, *argv, &par_time ) != TCL_OK ) {
		*err_msg = "Invalid value for -par_time in open course data";
		goto bail_open_course_data;
	    }
	} else {
	    sprintf( err_buff, "unrecognized option `%s' in open course data",
		     *argv );
	    *err_msg = err_buff;
	    goto bail_open_course_data;
	}

	NEXT_ARG;
    }

    /* Check mandatory arguments */
    if ( open_course_data->course.empty() ) {
		*err_msg = "No course specified in open course data";
		goto bail_open_course_data;
    }

    if ( open_course_data->name.empty() ) {
		*err_msg = "No name specified in open course data";
		goto bail_open_course_data;
    }

    open_course_data->par_time = par_time;

    Tcl_Free( (char*) orig_argv );

    return open_course_data;

bail_open_course_data:

    if ( orig_argv ) {
	Tcl_Free( (char*) orig_argv );
    }

    if ( open_course_data ) {
		delete open_course_data;
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/
/*! 
  tux_open_courses Tcl callback
  \author  jfpatry
  \date    Created:  2000-09-19
  \date    Modified: 2000-09-19
*/
static int open_courses_cb( ClientData cd, Tcl_Interp *ip,
			    int argc, CONST84 char **argv )
{
    char *err_msg;
    CONST84 char **list = NULL;
    int num_courses;
    std::list<CourseData>::iterator lastElem;
    int i;
	
	check_assertion( initialized,
		     "course_mgr module not initialized" );

    if ( argc != 2 ) {
		err_msg = "Wrong number of arguments";
		goto bail_open_courses;
    }

    if ( Tcl_SplitList( ip, argv[1], &num_courses, &list ) == TCL_ERROR ) {
		err_msg = "Argument is not a list";
		goto bail_open_courses;
    }

    /* Add items to end of list */
    lastElem = openCourseList.end();

    for ( i=0; i<num_courses; i++ ) {
		CourseData *data;
		data = create_open_course_data( ip, list[i], &err_msg );

		if ( data == NULL ) {
	    	goto bail_open_courses;
		}

		openCourseList.push_back(*data);
	}

    Tcl_Free( (char*) list );
    list = NULL;

    return TCL_OK;

bail_open_courses:

    /* We'll leave the data that was successfully added in the list. */

    Tcl_AppendResult(
	ip,
	"Error in call to tux_open_courses: ", 
	err_msg,
	"\n",
	"Usage: tux_open_courses { list of open courses }",
	(NULL) );
    return TCL_ERROR;
}


/*---------------------------------------------------------------------------*/
/*! 
  Creates a race_data_t object from a Tcl string.
  \return  New race_data_t object if successful, or NULL if error
  \author  jfpatry
  \date    Created:  2000-09-19
  \date    Modified: 2000-09-19
*/
static
CourseData* create_race_data ( Tcl_Interp *ip, CONST84 char *string, char **err_msg )
{
	CONST84 char **argv = NULL;
    CONST84 char **orig_argv = NULL;
    int argc = 0;

    bool   herring_req_init = false;
    bool   time_req_init = false;
    bool   score_req_init = false;

    CourseData *race_data = new CourseData;
	
    if ( Tcl_SplitList( ip, string, &argc, &argv ) == TCL_ERROR ) {
		*err_msg = "race data is not a list";
		goto bail_race_data;
    }

    orig_argv = argv;

	
    while ( *argv != NULL ) {
	if ( strcmp( *argv, "-course" ) == 0 ) {
	    NEXT_ARG;
	    if ( *argv == NULL ) {
			*err_msg = "No data supplied for -course in race data";
			goto bail_race_data;
	    }
	    race_data->course = *argv; //string_copy( *argv );

	} else if ( strcmp( *argv, "-name" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -name in race data";
		goto bail_race_data;
	    }

	    race_data->name = *argv; //string_copy( *argv );		

	} else if ( strcmp( *argv, "-description" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -description in race data";
		goto bail_race_data;
	    }

	    race_data->description = *argv; //string_copy( *argv );
		            race_data->description = *argv; //string_copy( *argv );

      } else if ( strcmp( *argv, "-contributed" ) == 0 ) {
           NEXT_ARG;

          if ( *argv == NULL ) {
              *err_msg = "No data supplied for -contributed in race data";
              goto bail_race_data;
           }

           race_data->contributed = *argv; //string_copy( *argv );
	} else if ( strcmp( *argv, "-herring" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -herring in race data";
		goto bail_race_data;
	    }

	    if ( get_tcl_int_tuple( 
		ip, *argv, race_data->herring_req, 
		sizeof(race_data->herring_req)/sizeof(race_data->herring_req[0]) ) == TCL_ERROR )
	    {
		*err_msg = "Value for -herring is not a list or has "
		    "the wrong number of elements";
		goto bail_race_data;
	    }

	    herring_req_init = true;

	} else if ( strcmp( *argv, "-time" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -time in race data" ;
		goto bail_race_data;
	    }

	    if ( get_tcl_tuple( ip, *argv, race_data->time_req, 
				sizeof(race_data->time_req)/sizeof(race_data->time_req[0]) ) 
		 == TCL_ERROR ) 
	    {
		*err_msg = "Value for -time is not a list or hsa the "
		    "wrong number of elements";
		goto bail_race_data;
	    }

	    time_req_init = true;
	} else if ( strcmp( *argv, "-score" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -score in race data";
		goto bail_race_data;
	    }

	    if ( get_tcl_tuple( ip, *argv, race_data->score_req,
				sizeof(race_data->score_req)/sizeof(race_data->score_req[0]) )
		 == TCL_ERROR ) 
	    {
		*err_msg = "Value for -score is not a list or has the "
		    "wrong number of elements";
		goto bail_race_data;
	    }

	    score_req_init = true;
	} else if ( strcmp( *argv, "-mirrored" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -mirrored in race data";
		goto bail_race_data;
	    }

	    if ( strcmp( *argv, "yes" ) == 0 ) {
			race_data->mirrored = true;
	    } else {
			race_data->mirrored = false;
	    }
	} else if ( strcmp( *argv, "-conditions" ) == 0 ) {
	    int i;
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -conditions in race data";
		goto bail_race_data;
	    }

	    for ( i=0; i<RACE_CONDITIONS_NUM_CONDITIONS; i++ ) {
		if ( strcmp( race_condition_names[i],
					 *argv ) == 0 )
		{
		    break;
		}
	    }

	    if ( i == RACE_CONDITIONS_NUM_CONDITIONS ) {
		*err_msg = "Invalid value for -conditions in race data";
		goto bail_race_data;
	    }

	    race_data->condition = (race_conditions_t)i;
	} else if ( strcmp( *argv, "-windy" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -windy in race data";
		goto bail_race_data;
	    }

	    if ( strcmp( *argv, "yes" ) == 0 ) {
			race_data->windy = true;
	    } else {
			race_data->windy = false;
	    }
	} else if ( strcmp( *argv, "-snowing" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -snowing in race data";
		goto bail_race_data;
	    }

	    if ( strcmp( *argv, "yes" ) == 0 ) {
			race_data->snowing = true;
	    } else {
			race_data->snowing = false;
	    }
	} else {
	    sprintf( err_buff, "unrecognized option `%s' in race data",
		     *argv );
	    *err_msg = err_buff;
	    goto bail_race_data;
	}

	NEXT_ARG;
    }

    /* Check mandatory arguments */
    if ( race_data->course.empty() ) {
		*err_msg = "No course specified in race data";
		goto bail_race_data;
    }

    if ( !herring_req_init ||
	 !time_req_init ||
	 !score_req_init ) 
    {
	*err_msg = "Must specify requirement for herring, time, and score.";
	goto bail_race_data;
    }

    Tcl_Free( (char*) orig_argv );

    return race_data;

bail_race_data:
    if ( orig_argv ) {
	Tcl_Free( (char*) orig_argv );
    }

    if ( race_data ) {
	delete( race_data );
    }

    return NULL;
}


/*---------------------------------------------------------------------------*/
/*! 
  Creates a CupData object from a Tcl string.
  \return  New CupData object if successful, or NULL if error
  \author  jfpatry
  \date    Created:  2000-09-19
  \date    Modified: 2000-09-19
*/
static
CupData* create_cup_data( Tcl_Interp *ip, CONST84 char *string, char **err_msg )
{
    CONST84 char **argv = NULL;
    CONST84 char **orig_argv = NULL;
    int argc = 0;

    std::list<CourseData> raceList;
    CONST84 char **races = NULL;
    int num_races = 0;
    int i;

    CupData* cup_data = new CupData();

    if ( Tcl_SplitList( ip, string, &argc, &argv ) == TCL_ERROR ) {
		*err_msg = "cup data is not a list";
		goto bail_cup_data;
    }
    orig_argv = argv;

    while ( *argv != NULL ) {
	if ( strcmp( *argv, "-name" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -name in cup data";
		goto bail_cup_data;
	    }
	    cup_data->name = *argv; //string_copy( *argv );
	} else if ( strcmp( *argv, "-icon" ) == 0 ) {
	    NEXT_ARG;
	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -icon in cup data";
		goto bail_cup_data;
	    }

	    cup_data->icon = *argv; //string_copy( *argv );
	} else if ( strcmp( *argv, "-races" ) == 0 ) {
	    NEXT_ARG;
	    if ( *argv == NULL ) {
		*err_msg= "No data supplied for -races in cup data";
		goto bail_cup_data;
	    }

	    if ( Tcl_SplitList( ip, *argv, &num_races, &races ) == TCL_ERROR ) {
		*err_msg = "Race data is not a list in event data";
		goto bail_cup_data;
	    }
	    for ( i=0; i<num_races; i++) {
		CourseData *raceData;
			raceData = create_race_data( ip, races[i], err_msg );
				
		if ( raceData == NULL ) {
		    goto bail_cup_data;
		}
		raceList.push_back(*raceData);
	    }

	    Tcl_Free( (char*) races );
	    races = NULL;
	} else {
	    sprintf( err_buff, "Unrecognized argument `%s'", *argv );
	    *err_msg = err_buff;
	    goto bail_cup_data;
	}

	NEXT_ARG;
    }

    /* Make sure mandatory fields have been specified */
    if ( cup_data->name.empty() ) {
	*err_msg = "Must specify a name in cup data";
	goto bail_cup_data;
    }

    if ( cup_data->icon.empty() ) {
	*err_msg = "Must specify an icon texture in cup data";
	goto bail_cup_data;
    }

    if ( raceList.empty() ) {
		*err_msg = "Must specify a race list in cup data";
		goto bail_cup_data;
    }

    /* Create a new cup data object */
    check_assertion( cup_data != NULL, "out of memory" );

    cup_data->raceList = raceList;
    bind_texture( cup_data->name.c_str(), cup_data->icon.c_str() );

    Tcl_Free( (char*) orig_argv );
    argv = NULL;

    return cup_data;

bail_cup_data:

    if ( orig_argv ) {
	Tcl_Free( (char*) orig_argv );
    }

/*    if ( name ) {
	free( name );
    }

    if ( icon ) {
	free( icon );
    }
*/
    if ( races ) {
	Tcl_Free( (char*) races );
    }

    /* Clean out race list */
   
    if ( cup_data ) {
		delete( cup_data );
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/
/*! 
  Creates an EventData object from a Tcl string.
  \return  New EventData object if successful, or NULL on error
  \author  jfpatry
  \date    Created:  2000-09-19
  \date    Modified: 2000-09-19
*/
static
EventData* create_event_data( Tcl_Interp *ip, CONST84 char *string, char **err_msg )
{
    CONST84 char **orig_argv = NULL;
    CONST84 char **argv = NULL;
    int argc = 0;

    std::list<CupData> cupList;
    CONST84 char **cups = NULL;
    int num_cups = 0;
    int i;

    EventData* event_data = new EventData();

    if ( Tcl_SplitList( ip, string, &argc, &argv ) == TCL_ERROR ) {
	*err_msg = "event data is not a list";
	goto bail_event_data;
    }

    orig_argv = argv;

    while ( *argv != NULL ) {
	if ( strcmp( *argv, "-name" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -name in event data";
		goto bail_event_data;
	    }

	    event_data->name = *argv;		//string_copy( *argv );
	} else if ( strcmp( *argv, "-icon" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -icon in event data";
		goto bail_event_data;
	    }

	    event_data->icon = *argv; //string_copy( *argv );
	} else if ( strcmp( *argv, "-cups" ) == 0 ) {
	    NEXT_ARG;

	    if ( *argv == NULL ) {
		*err_msg = "No data supplied for -cups in event data";
		goto bail_event_data;
	    }

	    if ( Tcl_SplitList( ip, *argv, &num_cups, &cups ) == TCL_ERROR ) {
		*err_msg = "Cup data is not a list in event data";
		goto bail_event_data;
	    }

	    for ( i=0; i<num_cups; i++ ) {
			CupData *cup_data;

			cup_data = create_cup_data( ip, cups[i], err_msg );
			if ( cup_data == NULL ) {
			    goto bail_event_data;
			}

			cupList.push_back(*cup_data);
	    }

	    Tcl_Free( (char*) cups );
	    cups = NULL;
	} else {
	    sprintf( err_buff, "Unrecognized argument `%s'", *argv );
	    *err_msg = err_buff;
	    goto bail_event_data;
	}

	NEXT_ARG;
    }

    /* Make sure mandatory fields have been specified */
    if ( event_data->name.empty() ) {
		*err_msg = "Must specify a name in event data";
		goto bail_event_data;
    }

    if ( event_data->icon.empty() ) {
		*err_msg = "Must specify an icon texture in event data";
		goto bail_event_data;
    }

    if ( cupList.empty() ) {
		*err_msg = "Must specify a cup list in event data";
		goto bail_event_data;
    }

    /* Create new event data object */

    event_data->cupList = cupList;

    bind_texture( event_data->name.c_str(), event_data->icon.c_str() );

    Tcl_Free( (char*) orig_argv );
    argv = NULL;

    return event_data;

bail_event_data:

    if ( orig_argv ) {
	Tcl_Free( (char*) orig_argv );
    }

/*    if ( name ) {
	free( name );
    }

    if ( icon ) {
	free( name );
    }
*/
    if ( cups ) {
	Tcl_Free( (char*) cups );
    }

    /* Clean out cup list */

    if ( event_data ) {
		delete( event_data );
    }

    return NULL;
}


/*---------------------------------------------------------------------------*/
/*! 
  tux_events Tcl callback
  Here's a sample call to tux_events:

tux_events {
    { 
	-name "Herring Run" -icon noicon -cups {
	    { 
		-name "Cup 1" -icon noicon -races {
		    {
			-course path_of_daggers \
				-description "nice long description" \
				-herring { 15 20 25 30 } \
				-time { 40.0 35.0 30.0 25.0 } \
				-score { 0 0 0 0 } \
				-mirrored yes -conditions cloudy \
				-windy no -snowing no
		    }
		    {
			-course ingos_speedway \
				-description "nice long description" \
				-herring { 15 20 25 30 } \
				-time { 40.0 35.0 30.0 25.0 } \
				-score { 0 0 0 0 } \
				-mirrored yes -conditions cloudy \
				-windy no -snowing no
		    }
		}
		-name "Cup 2" -icon noicon -races {
		    {
			-course penguins_cant_fly \
				-description "nice long description" \
				-herring { 15 20 25 30 } \
				-time { 40.0 35.0 30.0 25.0 } \
				-score { 0 0 0 0 } \
				-mirrored yes -conditions cloudy \
				-windy no -snowing no
		    }
		    {
			-course ingos_speedway \
				-description "nice long description" \
				-herring { 15 20 25 30 } \
				-time { 40.0 35.0 30.0 25.0 } \
				-score { 0 0 0 0 } \
				-mirrored yes -conditions cloudy \
				-windy no -snowing no
		    }
		}
	    }
	}
    }
}

  \return  Tcl error code
  \author  jfpatry
  \date    Created:  2000-09-19
  \date    Modified: 2000-09-19
*/
static int events_cb( ClientData cd, Tcl_Interp *ip,
		      int argc, CONST84 char **argv )
{
    char *err_msg;
    CONST84 char **list = NULL;
    int num_events;
    int i;
    // Make sure module has been initialized
    check_assertion( initialized,
		     "course_mgr module not initialized" );

    if ( argc != 2 ) {
		err_msg = "Incorrect number of arguments";
		goto bail_events;
    }

    if ( Tcl_SplitList( ip, argv[1], &num_events, &list ) == TCL_ERROR ) {
		err_msg = "Argument is not a list";
		goto bail_events;
    }

    //if ( last_event != NULL ) {
	//err_msg = "tux_events has already been called; it can only be called "
	//    "once.";
	//goto bail_events;
    //}

    for (i=0; i<num_events; i++) {
		EventData *data = create_event_data( ip, list[i], &err_msg );

		if ( data == NULL ) {
		    goto bail_events;
		}

		eventList.push_back(*data);
    }
    Tcl_Free( (char*) list );
    list = NULL;

    return TCL_OK;

bail_events:
    if ( list != NULL ) {
		Tcl_Free( (char*) list );
    }

    Tcl_AppendResult(
	ip,
	"Error in call to tux_events: ", 
	err_msg,
	"\n",
	"Usage: tux_events { list of event data }",
	(NULL) );
	return TCL_ERROR;
}


/*---------------------------------------------------------------------------*/
/*! 
  Returns the current race conditions (sunny, cloudy, etc.)
  \author  jfpatry
  \date    Created:  2000-09-25
  \date    Modified: 2000-09-25
*/
static int get_race_conditions_cb( ClientData cd, Tcl_Interp *ip,
				   int argc, CONST84 char **argv )
{
    char *err_msg;
    Tcl_Obj *result;

    if ( argc != 1 ) {
		err_msg = "Incorrect number of arguments";
		goto bail_race_conditions;
    }

	result = Tcl_NewStringObj(
	race_condition_names[ gameMgr->getCurrentRace().condition ],
	strlen( race_condition_names[ gameMgr->getCurrentRace().condition ] ) );

    Tcl_SetObjResult( ip, result );

    return TCL_OK;

bail_race_conditions:

    Tcl_AppendResult(
	ip,
	"Error in call to tux_get_race_conditions: ", 
	err_msg,
	"\n",
	"Usage: tux_get_race_conditions",
	(NULL) );
    return TCL_ERROR;
}

void register_course_manager_callbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "tux_open_courses", open_courses_cb, 0,0);
    Tcl_CreateCommand (ip, "tux_events", events_cb, 0,0);
    Tcl_CreateCommand (ip, "tux_get_race_conditions", 
		       get_race_conditions_cb, 0,0);
}



/* EOF */
