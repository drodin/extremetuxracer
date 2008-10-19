#
# Procedure to get course information from info.tcl file and append it to the list.
#
proc append_course_info {course path {mode "open"}} {
	#Initialization of optional variables
	set course_name "Unknown"
	set course_author {}
	set course_description {}
	set par_time 40.0
	
	if { [catch {source $course/info.tcl}] } {
		#That sounds better than "error evalating etracer_init.tcl"
		puts stderr "Could not open $course/info.tcl"
		puts stderr "Please see http://extremetuxracer.com/wiki/index.php?title=Creating_Courses."
		if {$mode == "event"} {exit 1}
	}
	
	global open_course_list
	global races_list
	
	lappend open_course_list \
		[list -course $path$course -name $course_name \
		-description $course_description -contributed $course_author \
		-par_time $par_time ] 
	if {$mode == "event"} {	   
	lappend races_list \
		[list -course $path$course -name $course_name \
		-description $course_description -contributed $course_author \
		-herring $herring -time $times -score $score -mirrored $mirrored \
		-conditions $conditions -windy $windy -snowing $snowing -snowtype $snowtype ] 
	}
}

#
# Procedure to get event information form event.tcl and append it to the list. [This also executes append_course_info and load_preview!]
#
proc append_event_info {event {path events/}} {
	global events_list
	global races_list
	set cups_list {}
	# A default value for compatibility
	set event_music tuxracer
	
	source event.tcl	
	
	foreach cup $cups {	
		#We're doing all of this mess with arrays to preserve course order in event mode.
		array set tmp [array get $cup]			
		#Clear out races_list for each new cup		
		set races_list {} 		

		foreach race $tmp(races) {					
			load_preview $race $path$event/
			append_course_info $race $path$event/ event
		}
		
		lappend cups_list \
			[list -name $tmp(name) -icon $tmp(icon) -races $races_list]
	}	
	
	lappend events_list \
		[list -name $event_name -icon $event_icon -music $event_music \
		-cups $cups_list]
}

#
# Procedure to load course preview
#
proc load_preview {course path} {
	if [file exists "$course/preview.rgb"] {
		tux_load_texture "$path$course" "$course/preview.rgb"
		tux_bind_texture "$path$course" "$path$course"
	}
	if [file exists "$course/preview.png"] {
		tux_load_texture "$path$course" "$course/preview.png"
		tux_bind_texture "$path$course" "$path$course"
	}
	
}

#
# Initializing variables
#
set cwd [pwd]
set open_course_list {}
set events_list {}
set races_list {}


#
# Build list of contributed courses in ~/.etracer/contrib
#
set ucontrib "$env(HOME)/.etracer/contrib/"
if [file exists "$ucontrib"] {
	cd $ucontrib
	foreach course [lsort [glob -nocomplain *]] {
		append_course_info $course contrib/
		load_preview $course $ucontrib
	}
}

#
# Loading default icons
#
tux_goto_data_dir

tux_load_texture noicon textures/noicon.png

cd courses

tux_load_texture herring_run_icon events/b-herring_run/herringrunicon.png 0
tux_load_texture cup_icon events/b-herring_run/cupicon.png 0

tux_load_texture no_preview ../textures/nopreview.png
tux_bind_texture no_preview no_preview


#
# Build list of contributed courses in contrib
#
cd contrib
foreach course [lsort [glob -nocomplain *]] {
	load_preview $course contrib/	
	append_course_info $course contrib/
}
cd ..


#
# Load events and bind preview textures if they exist
#
cd events
foreach event [lsort [glob -nocomplain *]] {
	cd $event    
	append_event_info $event 
	cd ..
}

if [file exists "$env(HOME)/.etracer/events/"] {
	set path "$env(HOME)/.etracer/events/"
	cd $path
	foreach event [lsort [glob -nocomplain *]] {
		cd $event
		append_event_info $event $path
		cd ..
	}
}

tux_open_courses $open_course_list
tux_events $events_list

cd $cwd
