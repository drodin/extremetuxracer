tux_wind_velocity -velocity {90 10 10} -scale 1.0

set conditions [tux_get_race_conditions]
if { $conditions == "sunny" } {
    source sunny_light.tcl
} elseif { $conditions == "cloudy" } {
    source foggy_light.tcl
} elseif { $conditions == "night" } {
    source night_light.tcl
} elseif { $conditions == "evening" } {
    source evening_light.tcl
} 
