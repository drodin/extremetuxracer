
#
# Procedure to get course name, author, description and par time from course.tcl file.
#
proc get_course_info { } {
    if [catch {open course.tcl r} fileId] {
	puts stderr "Couldn't open course.tcl in [pwd]"
	return {}
    }

    set name ""
    set author ""
    set description ""
    set par_time ""

    while {[gets $fileId line] >= 0} {
	regexp {tux_course_name *([^;]*)} $line match name
	regexp {tux_course_author *([^;]*)} $line match author
	regexp {tux_course_description *([^;]*)} $line match description
	regexp {tux_par_time *([^;]*)} $line match par_time

	if { $author != "" && $name != "" && $par_time != "" } {
	    break;
	}
    }

    if { $author == "" } {
	#set author "Unknown"
    }

    if { $name == "" } {
	set name "Unknown"
    }

    if { $par_time == "" } {
        set par_time 120.0
    } 

    # Remove quotes around strings, etc.; e.g. "Jasmin Patry" -> Jasmin Patry
    eval "set name $name"
    eval "set author $author"
    eval "set description $description"
    eval "set par_time $par_time"

    return [list $name $author $par_time $description];
}

set cwd [pwd]


#
# Build list of contributed courses in ~/.ppracer/contrib
#
set ucontrib "$env(HOME)/.ppracer/contrib/"
if [file exists "$ucontrib"] {
	cd $ucontrib

	set contrib_course_list {}
	foreach course [glob -nocomplain *] {
 	   if { $course == "CVS" } {
		continue;
 	   }

	   if [catch {cd $course}] {
		puts stderr "Couldn't change directory to $course"
		continue;
 	   }

 	   set course_info [get_course_info]

 	   cd ..

 	   set description "[lindex $course_info 3]"
 	   set contributed "[lindex $course_info 1]"

 	   lappend contrib_course_list \
		    [list -course "$ucontrib$course" -name [lindex $course_info 0] \
		    -description $description -contributed $contributed \
		    -par_time [lindex $course_info 2] ] 
	}

	foreach course [glob -nocomplain *] {
   	 if [file exists "$course/preview.rgb"] {
		tux_load_texture "$ucontrib$course" "$course/preview.rgb"
		tux_bind_texture "$ucontrib$course" "$ucontrib$course"
  	  }
	    if [file exists "$course/preview.png"] {
		tux_load_texture "$ucontrib$course" "$course/preview.png"
		tux_bind_texture "$ucontrib$course" "$ucontrib$course"
	    }
	}
}

tux_goto_data_dir

tux_load_texture noicon textures/noicon.png

cd courses

tux_load_texture herring_run_icon events/b-herring_run/herringrunicon.png 0
tux_load_texture cup_icon events/b-herring_run/cupicon.png 0

tux_load_texture no_preview ../textures/nopreview.png
tux_bind_texture no_preview no_preview

cd contrib;

foreach course [glob -nocomplain *] {
    if [file exists "$course/preview.rgb"] {
	tux_load_texture "contrib/$course" "$course/preview.rgb"
	tux_bind_texture "contrib/$course" "contrib/$course"
    }
    if [file exists "$course/preview.png"] {
	tux_load_texture "contrib/$course" "$course/preview.png"
	tux_bind_texture "contrib/$course" "contrib/$course"
    }
}

cd ..

#
# Build list of contributed courses in contrib
#
cd contrib
foreach course [glob -nocomplain *] {
    if { $course == "CVS" } {
	continue;
    }

    if [catch {cd $course}] {
	puts stderr "Couldn't change directory to $course"
	continue;
    }

    set course_info [get_course_info]

    cd ..

    set description "[lindex $course_info 3]"
    set contributed "[lindex $course_info 1]"

    
    lappend contrib_course_list \
	    [list -course "contrib/$course" -name [lindex $course_info 0] \
	    -description $description -contributed $contributed \
	    -par_time [lindex $course_info 2] ] 
}
cd ..

tux_open_courses $contrib_course_list

#
# Load events and bind preview textures if they exist
#
cd events
foreach event [glob -nocomplain *] {
    
    cd $event
    foreach course [glob -nocomplain *] {
       if [file exists "$course/preview.rgb"] {
	   tux_load_texture $course "$course/preview.rgb"
	   tux_bind_texture events/$event/$course $course
       }
       if [file exists "$course/preview.png"] {
	   tux_load_texture $course "$course/preview.png"
	   tux_bind_texture events/$event/$course $course
       }
    }
    source event.tcl
    cd ..
}

if [file exists "$env(HOME)/.etracer/events/"] {
    cd "$env(HOME)/.etracer/events/"
    foreach event [glob -nocomplain *] {
    
        cd $event
        foreach course [glob -nocomplain *] {
            if [file exists "$course/preview.rgb"] {
	        tux_load_texture $course "$course/preview.rgb"
	        tux_bind_texture events/$event/$course $course
            }
            if [file exists "$course/preview.png"] {
	        tux_load_texture $course "$course/preview.png"
	        tux_bind_texture events/$event/$course $course
            }
        }
        source event.tcl
        cd ..
    }
}

cd $cwd
