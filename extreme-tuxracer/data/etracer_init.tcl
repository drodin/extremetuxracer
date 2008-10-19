# Tux Racer initialization script

#
# Save data directory
# 
set tux_data_dir [pwd]

set tux_terrain_file "none"

#
# Create a settings directory
# Solves segfault bug
#

if {![file isdir "$env(HOME)/.etracer/"]} {
    file mkdir $env(HOME)/.etracer
}


#
# Useful subroutines
#
proc tux_goto_data_dir {} {
    global tux_data_dir
    cd $tux_data_dir
}

proc tux_theme_init {theme} {
    global tux_data_dir
    global env

    set cwd [pwd]

    if [file exists "$env(HOME)/.etracer/themes/$theme/courseinit.tcl"] {
	cd "$env(HOME)/.etracer/themes/$theme/"
	source courseinit.tcl
    } else {
        cd "$tux_data_dir/courses/themes/"
	source "$theme.tcl"
    }

    source "$tux_data_dir/tux_walk.tcl";

    if [file exists "$cwd/trees.rgb"] {
        tux_trees "$cwd/trees.rgb"
    } elseif [file exists "$cwd/trees.png"] {
        tux_trees "$cwd/trees.png"
    }

    global tux_terrain_file
    tux_load_terrain $tux_terrain_file

    cd $cwd
} 

proc tux_theme_pack_init {type name} {

    global tux_data_dir
    set cwd [pwd]

    cd "$tux_data_dir/courses/themes/$type/$name/";
    source "$type.tcl"

    cd $cwd
}

proc tux_course_init {} {
    tux_theme_init common
} 

proc tux_terrain {terrain} {
    global tux_terrain_file
    set tux_terrain_file [pwd]/$terrain
} 

#
# Read course index
#
source courses/course_idx.tcl

#
# Set Up Music
#

# Splash & Start screens
if { [tux_load_music start_screen music/start1-jt.ogg] } {
    tux_bind_music splash_screen start_screen -1
    tux_bind_music start_screen start_screen -1
}

# Credits screen
if { [tux_load_music credits_screen music/credits1-cp.ogg] } {
    tux_bind_music credits_screen credits_screen -1
}

# Options screen
if { [tux_load_music options_screen music/options1-jt.ogg] } {
    tux_bind_music options_screen options_screen -1
}

# Music played during race
if { [tux_load_music racing music/race1-jt.ogg] } {
    tux_bind_music intro racing -1
    tux_bind_music racing racing -1
    tux_bind_music paused racing -1
}

# Game Over screen
if { [tux_load_music game_over music/wonrace1-jt.ogg] } {
    tux_bind_music game_over game_over 1
}

# Event music theme
source music/themes.tcl

#
# Set Up Sounds
#

# Tree Hit
if { [tux_load_sound tree_hit1 sounds/tux_hit_tree1.wav] } {
    tux_bind_sounds tree_hit tree_hit1 
}

# Fish Pickup
if { [tux_load_sound fish_pickup_sound_1 sounds/fish_pickup1.wav]&& 
     [tux_load_sound fish_pickup_sound_2 sounds/fish_pickup2.wav]&& 
     [tux_load_sound fish_pickup_sound_3 sounds/fish_pickup3.wav] } \
{
    tux_bind_sounds item_collect fish_pickup_sound_1 \
	                         fish_pickup_sound_2 \
				 fish_pickup_sound_3
}

# Snow Sliding
if { [tux_load_sound snow_sound sounds/tux_on_snow1.wav] } {
    tux_bind_sounds snow_sound snow_sound
    tux_bind_sounds flying_sound snow_sound
}

# Rock Sliding
if { [tux_load_sound rock_sound sounds/tux_on_rock1.wav] } {
    tux_bind_sounds rock_sound rock_sound
}

# Ice Sliding
if { [tux_load_sound ice_sound sounds/tux_on_ice1.wav] } {
    tux_bind_sounds ice_sound ice_sound
}

# Splash screen
tux_load_texture splash_screen textures/splash.png 0
tux_bind_texture splash_screen splash_screen

tux_load_texture splash_screen_small textures/splash_small.png 0
tux_bind_texture splash_screen_small splash_screen_small

# UI Snow Particle
tux_load_texture ui_snow_particle textures/snowparticles.png 0
tux_bind_texture ui_snow_particle ui_snow_particle

# UI Snow Particle for original Tuxracer
tux_load_texture snow_particle textures/snowparticles.png 0
tux_bind_texture snow_particle snow_particle


# Load truetype fonts
# button label
pp_load_font -binding button_label \
             -font fonts/PaperCuts20.ttf \
             -size 30

pp_bind_font -binding button_label_hilit \
             -font button_label \
             -color {1.00 0.89 0.01 1.0}

pp_bind_font -binding loading \
             -font button_label_hilit \
             -color {1.00 0.89 0.01 1.0}

pp_bind_font -binding button_label_disabled \
             -font button_label \
             -color { 1.0 1.0 1.0 0.5 }

pp_bind_font -binding menu_label \
             -font button_label


# listbox
pp_load_font -binding listbox_item \
             -font fonts/PaperCuts20.ttf \
             -size 22 \
             -color { 1.00 0.89 0.01 1.0 }

pp_bind_font -binding listbox_item_insensitive \
             -font listbox_item \
             -color { 1.00 0.89 0.01 0.5 }

# race requirements
pp_load_font -binding race_requirements \
             -font fonts/std.ttf \
             -size 13

pp_bind_font -binding race_requirements_label \
             -font race_requirements \
             -color { 1.00 0.95 0.01 1.0 }

# heading
pp_load_font -binding heading \
             -font fonts/PaperCuts20.ttf \
             -size 40 \
             -color { 1.00 0.89 0.01 1.0 }

pp_load_font -binding heading_outline \
             -outline \
             -width 1.0 \
             -font fonts/PaperCuts20.ttf \
             -size 40 \
             -color { 0 0 0 1.0 }

pp_bind_font -binding paused \
             -font heading

pp_bind_font -binding race_over \
             -font heading \

pp_bind_font -binding race_over_outline \
             -font heading_outline \


# several stuff
pp_load_font -binding race_description \
             -font fonts/PaperCuts20.ttf \
             -size 16

pp_bind_font -binding cup_status \
             -font race_description \
             -color { 1.00 0.89 0.01 1.0 }

pp_load_font -binding race_stats \
             -font fonts/PaperCuts20.ttf \
             -size 27 \
             -color {1.00 0.89 0.01 1.0}

pp_load_font -binding race_stats_outline \
             -font fonts/PaperCuts20.ttf \
		-outline \
		-width 0.2 \
             -size 27 \
             -color {0 0 0 1.0}

#pp_load_font -binding race_results -font fonts/PaperCuts20.ttf -size 35 -color {1.00 0.89 0.01 1.0}
pp_load_font -binding race_results \
             -font fonts/PaperCuts20.ttf \
             -size 35 \
             -color {0.0 1.0 0.0 1.0}

pp_load_font -binding race_results_outline \
             -font fonts/PaperCuts20.ttf \
		-outline \
		-width 0.2 \
             -size 35 \
             -color {0 0 0 1.0}

pp_load_font -binding race_results_fail \
             -font fonts/PaperCuts20.ttf \
             -size 35 \
             -color {1.00 0 0.00 1.0}

pp_load_font -binding event_and_cup_label \
             -font fonts/PaperCuts20.ttf \
             -size 22

# credits
pp_load_font -binding credits_text -font fonts/PaperCuts20.ttf -size 20
pp_load_font -binding credits_text_big -font fonts/PaperCuts20.ttf -size 40
pp_load_font -binding credits_text_small -font fonts/PaperCuts20.ttf -size 14
pp_bind_font -binding credits_h1 -font button_label_hilit
pp_bind_font -binding credits_h2 -font credits_text -color {1.00 0.89 0.01 1.0}
pp_bind_font -binding credits_text_big -font credits_text_big -color {1.00 0.89 0.01 1.0}


#we don't be this because hud fonts need to be fast
pp_load_font -binding herring_count -font fonts/PaperCuts20.ttf -size 40 -color {1.00 0.89 0.01 1.0}
pp_load_font -binding herring_count_outline -font fonts/PaperCuts20.ttf -outline -size 40 -color {0 0 0 1.0} -width 1.5
pp_load_font -binding fps -font fonts/std.ttf -size 20 -color {0 0 0 1.0}
pp_load_font -binding time_value -font fonts/PaperCuts20.ttf -size 40 -color {1.00 0.89 0.01 1.0}
pp_load_font -binding speed_digits -font fonts/PaperCuts20.ttf -size 40 -color {1.00 0.89 0.01 1.0}
pp_load_font -binding speed_units -font fonts/PaperCuts20.ttf -size 25 -color {1.00 0.89 0.01 1.0}

pp_load_font -binding speed_digits_outline -font fonts/PaperCuts20.ttf -outline -size 40 -color {0 0 0 1.0} -width 1.5
pp_load_font -binding speed_units_outline -font fonts/PaperCuts20.ttf -outline -size 25 -color {0 0 0 1.0} -width 1.0

pp_load_font -binding objectives_time -font fonts/PaperCuts20.ttf -size 25 -color {1.00 0.89 0.01 1.0}
pp_load_font -binding objectives_herring -font fonts/PaperCuts20.ttf -size 25 -color {1.00 0.89 0.01 1.0}

pp_load_font -binding objectives_time_outline -outline -font fonts/PaperCuts20.ttf -size 25 -color {0 0 0 1.0} -width 0.8
pp_load_font -binding objectives_herring_outline -outline -font fonts/PaperCuts20.ttf -size 25 -color {0 0 0 1.0} -width 0.8



# not used
#pp_load_font -binding time_hundredths -font fonts/PaperCuts20.ttf -size 20 -color {1.00 0.89 0.01 1.0}


# HUD
tux_load_texture herring_icon textures/herringicon.png 0
tux_bind_texture herring_icon herring_icon

tux_load_texture herring_icon-ok textures/herringicon-ok.png 0
tux_bind_texture herring_icon-ok herring_icon-ok

tux_load_texture time_icon textures/timeicon.png 0
tux_bind_texture time_icon time_icon

tux_load_texture time_icon-no textures/timeicon-no.png 0
tux_bind_texture time_icon-no time_icon-no

tux_load_texture gauge_outline textures/gaugeoutline.png 0
tux_bind_texture gauge_outline gauge_outline

tux_load_texture gauge_energy_mask textures/gaugeenergymask.png 0
tux_bind_texture gauge_energy_mask gauge_energy_mask

tux_load_texture gauge_speed_mask textures/gaugespeedmask.png 0
tux_bind_texture gauge_speed_mask gauge_speed_mask


#Energy und Speedmask für die Demobalken
tux_load_texture energy_mask textures/energymask.png 0
tux_bind_texture energy_mask energy_mask
tux_load_texture speed_mask textures/speedmask.png 0
tux_bind_texture speed_mask speed_mask
tux_load_texture mask_outline textures/mask_outline.png 0
tux_bind_texture mask_outline mask_outline
tux_load_texture mask_outline2 textures/mask_outline2.png 0
tux_bind_texture mask_outline2 mask_outline2


# UI widgets
tux_load_texture listbox_arrows textures/listbox_arrows.png 0
tux_bind_texture listbox_arrows listbox_arrows
tux_bind_texture textarea_arrows listbox_arrows

tux_load_texture mirror_button textures/mirror_button.png 0
tux_bind_texture mirror_button mirror_button

tux_load_texture conditions_button textures/conditions_button.png 0
tux_bind_texture conditions_button conditions_button

tux_load_texture snow_button textures/snow_button.png 0
tux_bind_texture snow_button snow_button

tux_load_texture wind_button textures/wind_button.png 0
tux_bind_texture wind_button wind_button


tux_load_texture checkmark textures/checkmark.png 0
tux_bind_texture checkmark checkmark

# Menu decorations
tux_load_texture menu_bottom_left textures/menu_bottom_left.png 0
tux_bind_texture menu_bottom_left menu_bottom_left

tux_load_texture menu_bottom_right textures/menu_bottom_right.png 0
tux_bind_texture menu_bottom_right menu_bottom_right

tux_load_texture menu_top_left textures/menu_top_left.png 0
tux_bind_texture menu_top_left menu_top_left

tux_load_texture menu_top_right textures/menu_top_right.png 0
tux_bind_texture menu_top_right menu_top_right

tux_load_texture menu_title textures/menu_title.png 0
tux_bind_texture menu_title menu_title

tux_load_texture menu_title_small textures/menu_title_small.png 0
tux_bind_texture menu_title_small menu_title_small

# Tux life icon
tux_load_texture tux_life textures/tuxlife.png 0
tux_bind_texture tux_life tux_life

# Mouse cursor
tux_load_texture mouse_cursor textures/mouse_cursor.png 0
tux_bind_texture mouse_cursor mouse_cursor



#stubs/unused functions
proc tux_course_description {description} {}
