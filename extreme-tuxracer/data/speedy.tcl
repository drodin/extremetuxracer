# -----------------------------------------------------------------------
# Globals
# -----------------------------------------------------------------------
set xlation_node :t1
set rotation_node :t1
set root :t1

tux_root_node $root

#
# Material Properties
#

tux_material white_penguin \
            {0.58 0.58 0.58 } {0.2 0.2 0.2} 50.0
tux_material green_turtle {0.067 0.776 0.102} {0.5 0.5 0.5} 20.0
tux_material yellow_turtle {1 0.98 0.6} {0.5 0.5 0.5} 20.0
tux_material turtle_shell {0.51 0.31 0.00} {0.5 0.5 0.5} 20.0
tux_material beak_color \
            {0.64 0.54 0.06} {0.4 0.4 0.4} 5
tux_material nostril_color \
            {0.48039 0.318627 0.033725} {0.0 0.0 0.0} 1

tux_material iris_color {0.01 0.01 0.01} {0.4 0.4 0.4} 90.0


#
# Define the geometry
#

# Helper function to make torso parts
proc makeBody { parent } {
    set torso_bl "$parent:b"
    tux_sphere $parent b 1.0
    tux_surfaceproperty $torso_bl yellow_turtle
    tux_scale $torso_bl {0 0 0} { 0.8 1.3 0.3 }
    tux_translate $torso_bl { 0.0 -.1 0.0 }

    set torso_wh1 $parent:w1
    tux_sphere $parent w1 1.0
    tux_surfaceproperty $torso_wh1 turtle_shell
    tux_translate $torso_wh1 { 0.0 -.1 -.3 }
    tux_scale $torso_wh1 {0 0 0} {1.0 1.5 0.6}
    tux_shadow $torso_wh1 off

    set torso_wh2 $parent:w2
    tux_sphere $parent w2 1.0
    tux_surfaceproperty $torso_wh2 turtle_shell
    tux_translate $torso_wh2 { 0.0 -.1 -.5 }
    tux_scale $torso_wh2 {0 0 0} {1.0 1.5 0.25}
    tux_shadow $torso_wh2 on
} 


tux_transform : t1
set root :t1
set xlation_node :t1

tux_transform :t1 r1
set rotation_node :t1:r1
tux_scale $rotation_node {0 0 0} { 0.35 0.35 0.35 }



#
# Create shoulders
#

tux_transform $rotation_node t2
set shoulder_xlate "$rotation_node:t2"
#tux_rotate $shoulder_xlate x 10
#tux_translate $shoulder_xlate { 0 0.4 0.0 }
tux_translate $shoulder_xlate { 0 0.4 0.05 }

tux_transform $shoulder_xlate scl
set shoulder_scale "$shoulder_xlate:scl"
tux_scale $shoulder_scale {0 0 0} { 0.72 0.72 0.72 }

makeBody $shoulder_scale

#
# Create neck
#

tux_transform $rotation_node t3
set neck_xlate "$rotation_node:t3"
tux_translate $neck_xlate { 0 0.9 0.07 }
tux_rotate $neck_xlate y +90

tux_transform $neck_xlate neckjt       ;# neck joint
set neck_joint "$neck_xlate:neckjt"
tux_neck $neck_joint

tux_transform $neck_joint t4
set neck_org "$neck_joint:t4"
tux_rotate $neck_org y -90

# leave rotation point at origin

tux_transform $neck_org scl
set neck_scale "$neck_org:scl"
tux_scale $neck_scale {0 0 0} { 0 0 0 }

tux_sphere $neck_scale neck 0.8
set neck "$neck_scale:neck"
tux_surfaceproperty $neck yellow_turtle

tux_sphere $neck_scale neckw 0.66
set neckw "$neck_scale:neckw"
tux_surfaceproperty $neckw yellow_turtle
tux_translate $neckw {0 -0.08 0.35 }
tux_scale $neckw {0 0 0} {0.8 0.9 0.7}
tux_shadow $neckw off

#
# Create head
#

tux_transform $neck_org t5
set head_xlate "$neck_org:t5"
tux_translate $head_xlate { 0 0.35 0.07 }
tux_rotate $head_xlate y +90
tux_rotate $head_xlate z -20

tux_transform $head_xlate hdjtrot
set head_joint_rot "$head_xlate:hdjtrot"        ;# head joint for rotation

tux_transform $head_joint_rot hdjt
set head_joint "$head_joint_rot:hdjt"           ;# head joint
tux_head $head_joint

tux_transform $head_joint hdorg
set head_org "$head_joint:hdorg"
tux_rotate $head_org y -90
tux_translate $head_org {0 0.20 0}

tux_sphere $head_org hd 1.0
set head "$head_org:hd"
tux_surfaceproperty $head green_turtle
tux_scale $head {0 0 0} {0 0 0}

#
# Create head
#

tux_sphere $head_org bk1 0.8
set beak1 "$head_org:bk1"
tux_surfaceproperty $beak1 green_turtle
tux_translate $beak1 { 0 -0.205 0.3 }
tux_rotate $beak1 x +10
tux_scale $beak1 {0 0 0} { 0.23 0.12 0.4 }

#tux_sphere $head_org bk2
#set beak2 "$head_org:bk2"
#tux_surfaceproperty $beak2 green_turtle
#tux_translate $beak2 { 0 -0.195 0.3 }
#tux_rotate $beak2 x +10
#tux_scale $beak2 {0 0 0} { 0.19 0.15 0.32 }
#tux_shadow $beak2 off

tux_sphere $head_org bk3 0.66
set beak3 "$head_org:bk3"
tux_surfaceproperty $beak3 green_turtle
tux_translate $beak3 { 0 -0.23 0.3 }
tux_rotate $beak3 x +10
tux_scale $beak3 {0 0 0} { 0.21 0.17 0.38 }
tux_shadow $beak3 off

#
# Create eyes
#

tux_sphere $head_org le 0.66
set left_eye "$head_org:le"
tux_surfaceproperty $left_eye white_penguin
tux_translate $left_eye {0.13 -.1 0.38}
tux_rotate $left_eye y -30
tux_rotate $left_eye z -30
tux_rotate $left_eye x 90
tux_scale $left_eye {0 0 0} {0.1 0.13 0.03}
tux_shadow $left_eye off

tux_sphere $head_org re 0.66
set right_eye "$head_org:re"
tux_surfaceproperty $right_eye white_penguin
tux_translate $right_eye {-0.13 -.1 0.38}
tux_rotate $right_eye y 30
tux_rotate $right_eye z 30
tux_rotate $right_eye x 90
tux_scale $right_eye {0 0 0} {0.1 0.13 0.03}
tux_shadow $right_eye off

tux_sphere $head_org li 0.66
set left_iris "$head_org:li"
tux_surfaceproperty $left_iris iris_color
tux_translate $left_iris {0.12 -.08 0.40}
tux_rotate $left_iris y 18
tux_rotate $left_iris z 5
tux_rotate $left_iris x 90
tux_scale $left_iris {0 0 0} {0.055 0.07 0.03}
tux_shadow $left_iris off
tux_eye $left_iris left

tux_sphere $head_org ri 0.66
set right_iris "$head_org:ri"
tux_surfaceproperty $right_iris iris_color
tux_translate $right_iris {-0.12 -.08 0.40}
tux_rotate $right_iris y -18
tux_rotate $right_iris z -5
tux_rotate $right_iris x 90
tux_scale $right_iris {0 0 0} {0.055 0.07 0.03}
tux_shadow $right_iris off
tux_eye $right_iris right

#
# Create Upper Arm
#

# left
tux_transform $shoulder_xlate luarmxlate
set luparm_xlate "$shoulder_xlate:luarmxlate"
tux_rotate $luparm_xlate y 180
tux_translate $luparm_xlate {-0.56 0.3 0}
tux_rotate $luparm_xlate z +45
tux_rotate $luparm_xlate x +180

tux_transform $luparm_xlate lshjt
set lshoulder_joint "$luparm_xlate:lshjt"       ;# left shoulder joint
tux_left_shoulder $lshoulder_joint

tux_transform $lshoulder_joint luarmorg
set luparm_org "$lshoulder_joint:luarmorg"
tux_rotate $luparm_org x -90
tux_translate $luparm_org {-0.22 0 0}

tux_sphere $luparm_org uparm 0.66
set luparm "$luparm_org:uparm"
tux_surfaceproperty $luparm green_turtle
tux_scale $luparm {0 0 0} {0.34 0.1 0.2}

#right
tux_transform $shoulder_xlate ruarmxlate
set ruparm_xlate "$shoulder_xlate:ruarmxlate"
tux_translate $ruparm_xlate {-0.56 0.3 0}
tux_rotate $ruparm_xlate z +45
tux_rotate $ruparm_xlate x -180

tux_transform $ruparm_xlate rshjt                ;# right shoulder joint
set rshoulder_joint "$ruparm_xlate:rshjt"
tux_right_shoulder $rshoulder_joint

tux_transform $rshoulder_joint ruarmorg
set ruparm_org "$rshoulder_joint:ruarmorg"
tux_rotate $ruparm_org x +90
tux_translate $ruparm_org {-0.22 0 0}

tux_sphere $ruparm_org uparm 0.66
set ruparm "$ruparm_org:uparm"
tux_surfaceproperty $ruparm green_turtle
tux_scale $ruparm {0 0 0} {0.34 0.1 0.2}

#
# Create forearm
#

# left
tux_transform $luparm_org llarmxlate
set llowarm_xlate "$luparm_org:llarmxlate"
tux_translate $llowarm_xlate {-0.23 0 0}
tux_rotate $llowarm_xlate z 20
tux_rotate $llowarm_xlate x +90

tux_transform $llowarm_xlate lelbjt
set lelbow_joint "$llowarm_xlate:lelbjt"        ;# left elbow joint

tux_transform $lelbow_joint llarmorg
set llowarm_org "$lelbow_joint:llarmorg"
tux_rotate $llowarm_org x -90
tux_translate $llowarm_org {-0.19 0 0}

tux_sphere $llowarm_org llowarm 0.66
set llowarm "$llowarm_org:llowarm"
tux_surfaceproperty $llowarm green_turtle
tux_scale $llowarm {0 0 0} {0.30 0.07 0.15}

# right
tux_transform $ruparm_org rlarmxlate
set rlowarm_xlate "$ruparm_org:rlarmxlate"
tux_translate $rlowarm_xlate {-0.23 0 0}
tux_rotate $rlowarm_xlate z 20
tux_rotate $rlowarm_xlate x -90

tux_transform $rlowarm_xlate relbjt              ;# right elbow joint
set relbow_joint "$rlowarm_xlate:relbjt"

tux_transform $relbow_joint rlarmorg
set rlowarm_org "$relbow_joint:rlarmorg"
tux_rotate $rlowarm_org x +90
tux_translate $rlowarm_org {-0.19 0 0}

tux_sphere $rlowarm_org rlowarm 0.66
set rlowarm "$rlowarm_org:rlowarm"
tux_surfaceproperty $rlowarm green_turtle
tux_scale $rlowarm {0 0 0} {0.30 0.07 0.15}

#
# Create hands
#

# left
tux_transform $llowarm_org lhandxlate
set lhand_xlate "$llowarm_org:lhandxlate"
tux_translate $lhand_xlate {-0.24 0 0}
tux_rotate $lhand_xlate z 20
tux_rotate $lhand_xlate x +90

tux_transform $lhand_xlate lwrtjt
set lwrist_joint "$lhand_xlate:lwrtjt"          ;# left wrist joint

tux_transform $lwrist_joint lhndorg
set lhand_org "$lwrist_joint:lhndorg"
tux_rotate $lhand_org x -90
tux_translate $lhand_org {-0.10 0 0}

tux_sphere $lhand_org lhand 0.5
set lhand "$lhand_org:lhand"
tux_surfaceproperty $lhand green_turtle
tux_scale $lhand {0 0 0} {0.12 0.05 0.12}

# right
tux_transform $rlowarm_org rhandxlate
set rhand_xlate "$rlowarm_org:rhandxlate"
tux_translate $rhand_xlate {-0.24 0 0}
tux_rotate $rhand_xlate z 20
tux_rotate $rhand_xlate x -90

tux_transform $rhand_xlate rwrtjt
set rwrist_joint "$rhand_xlate:rwrtjt"          ;# right wrist joint

tux_transform $rwrist_joint rhndorg
set rhand_org "$rwrist_joint:rhndorg"
tux_rotate $rhand_org x +90
tux_translate $rhand_org {-0.10 0 0}

tux_sphere $rhand_org rhand 0.5
set rhand "$rhand_org:rhand"
tux_surfaceproperty $rhand green_turtle
tux_scale $rhand {0 0 0} {0.12 0.05 0.12}

#
# Make legs
# 

#left
tux_transform $rotation_node lthgxlate
set lthigh_xlate "$rotation_node:lthgxlate"
tux_rotate $lthigh_xlate y +180
tux_rotate $lthigh_xlate z -20
tux_translate $lthigh_xlate {-0.28 -0.7 0.24}
tux_rotate $lthigh_xlate x +20
tux_rotate $lthigh_xlate y +1200

tux_transform $lthigh_xlate lhipjt               ;# left hip joint
set lhip_joint "$lthigh_xlate:lhipjt"
tux_left_hip $lhip_joint

tux_transform $lhip_joint lthgorg
set lthigh_org "$lhip_joint:lthgorg"
tux_rotate $lthigh_org y -110
tux_translate $lthigh_org { 0 -0.1 0 } 

tux_sphere $lthigh_org lthigh 0.5
set lthigh "$lthigh_org:lthigh"
tux_surfaceproperty $lthigh green_turtle
tux_scale $lthigh {0 0 0} {0.2 0.5 0.07}

#right
tux_transform $rotation_node rthgxlate
set rthigh_xlate "$rotation_node:rthgxlate"
tux_rotate $rthigh_xlate z -20
tux_translate $rthigh_xlate {-0.28 -0.7 -.24}
tux_rotate $rthigh_xlate x -20
tux_rotate $rthigh_xlate y -120

tux_transform $rthigh_xlate rhipjt               ;# right hip joint
set rhip_joint "$rthigh_xlate:rhipjt"
tux_right_hip $rhip_joint

tux_transform $rhip_joint rthgorg
set rthigh_org "$rhip_joint:rthgorg"
tux_rotate $rthigh_org y +110
tux_translate $rthigh_org { 0 -0.1 0 } 

tux_sphere $rthigh_org rthigh 0.5
set rthigh "$rthigh_org:rthigh"
tux_surfaceproperty $rthigh green_turtle
tux_scale $rthigh {0 0 0} {0.2 0.5 0.07}

#
# Make calves (no longer used)
#

#left
tux_transform $lthigh_org lclfxlate
set lcalf_xlate "$lthigh_org:lclfxlate"
tux_translate $lcalf_xlate {0 -0.21 0}
tux_rotate $lcalf_xlate y +90

tux_transform $lcalf_xlate lkneejt               ;# left knee joint
set lknee_joint "$lcalf_xlate:lkneejt"      
tux_left_knee $lknee_joint

tux_transform $lknee_joint lclforg
set lcalf_org "$lknee_joint:lclforg"
tux_rotate $lcalf_org y -90
tux_translate $lcalf_org { 0 -0.13 0 } 

tux_sphere $lcalf_org lcalf 0.5
set lcalf "$lcalf_org:lcalf"
tux_surfaceproperty $lcalf green_turtle
tux_scale $lcalf {0 0 0} { 0 0 0}

#right
tux_transform $rthigh_org rclfxlate
set rcalf_xlate "$rthigh_org:rclfxlate"
tux_translate $rcalf_xlate {0 -0.21 0}
tux_rotate $rcalf_xlate y -90

tux_transform $rcalf_xlate rkneejt               ;# right knee joint
set rknee_joint "$rcalf_xlate:rkneejt"
tux_right_knee $rknee_joint

tux_transform $rknee_joint rclforg
set rcalf_org "$rknee_joint:rclforg"
tux_rotate $rcalf_org y +90
tux_translate $rcalf_org { 0 -0.13 0 } 

tux_sphere $rcalf_org rcalf 0.5
set rcalf "$rcalf_org:rcalf"
tux_surfaceproperty $rcalf green_turtle
tux_scale $rcalf {0 0 0} { 0 0 0}


#
# Create feet (scaled down to 0, not needed for turtle - needed to prevent 'segmentation faults')
#

# helper function to make feet
proc makeFoot { parent } {
 
} 

# left foot
tux_transform $lcalf_org lftxlate
set lfoot_xlate "$lcalf_org:lftxlate"
tux_translate $lfoot_xlate { 0 -0.18 0 }
tux_rotate $lfoot_xlate y -50

tux_transform $lfoot_xlate lankjt
set lankle_joint "$lfoot_xlate:lankjt"          ;# left ankle joint
tux_left_ankle $lankle_joint

makeFoot $lankle_joint

# right foot
tux_transform $rcalf_org rftxlate
set rfoot_xlate "$rcalf_org:rftxlate"
tux_translate $rfoot_xlate { 0 -0.18 0 }
tux_rotate $rfoot_xlate y +50

tux_transform $rfoot_xlate rankjt
set rankle_joint "$rfoot_xlate:rankjt"          ;# right ankle joint
tux_right_ankle $rankle_joint

makeFoot $rankle_joint

#
# Create Tail
#
tux_transform $rotation_node tailxlate
set tail_xlate "$rotation_node:tailxlate"
tux_translate $tail_xlate {0 -1 -.1}
tux_rotate $tail_xlate x 0

tux_transform $tail_xlate tail_joint
set tail_joint "$tail_xlate:tail_joint"         ;# tail joint
tux_tail $tail_joint

tux_transform $tail_joint tail_orig
set tail_orig "$tail_joint:tail_orig"
tux_translate $tail_orig {0 0.15 0}

tux_sphere $tail_orig tail 0.5
set tail "$tail_orig:tail"
tux_surfaceproperty $tail green_turtle
tux_scale $tail {0 0 0} {0.2 0.3 0.1}

