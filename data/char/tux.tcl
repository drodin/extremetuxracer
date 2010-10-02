
set root :t1
set rot_node :t1:r1
set torso_scl "$rot_node:tors_scl"
set shoulder_xlate "$rot_node:t2"
set shoulder_scale "$shoulder_xlate:scl"
set neck_xlate "$rot_node:t3"
set neck_joint "$neck_xlate:neckjt"
set neck_org "$neck_joint:t4"
set neck_scale "$neck_org:scl"
set neck "$neck_scale:neck"
set neckw "$neck_scale:neckw"
set head_xlate "$neck_org:t5"
set head_joint_rot "$head_xlate:hdjtrot"      
set head_joint "$head_joint_rot:hdjt"         
set head_org "$head_joint:hdorg"
set head "$head_org:hd"
set beak1 "$head_org:bk1"
set beak3 "$head_org:bk3"
set left_eye "$head_org:le"
set right_eye "$head_org:re"
set left_iris "$head_org:li"
set right_iris "$head_org:ri"
set luparm_xlate "$shoulder_xlate:luarmxlate"
set lshoulder_joint "$luparm_xlate:lshjt"     
set luparm_org "$lshoulder_joint:luarmorg"
set luparm "$luparm_org:uparm"
set ruparm_xlate "$shoulder_xlate:ruarmxlate"
set rshoulder_joint "$ruparm_xlate:rshjt"
set ruparm_org "$rshoulder_joint:ruarmorg"
set ruparm "$ruparm_org:uparm"
set llowarm_xlate "$luparm_org:llarmxlate"
set lelbow_joint "$llowarm_xlate:lelbjt"    
set llowarm_org "$lelbow_joint:llarmorg"
set llowarm "$llowarm_org:llowarm"
set rlowarm_xlate "$ruparm_org:rlarmxlate"
set relbow_joint "$rlowarm_xlate:relbjt"
set rlowarm_org "$relbow_joint:rlarmorg"
set rlowarm "$rlowarm_org:rlowarm"
set lhand_xlate "$llowarm_org:lhandxlate"
set lwrist_joint "$lhand_xlate:lwrtjt"     
set lhand_org "$lwrist_joint:lhndorg"
set lhand "$lhand_org:lhand"
set rhand_xlate "$rlowarm_org:rhandxlate"
set rwrist_joint "$rhand_xlate:rwrtjt" 
set rhand_org "$rwrist_joint:rhndorg"
set rhand "$rhand_org:rhand"
set lthigh_xlate "$rot_node:lthgxlate"
set lhip_joint "$lthigh_xlate:lhipjt"
set lthigh_org "$lhip_joint:lthgorg"
set lthigh "$lthigh_org:lthigh"
set lhipball "$lthigh_org:lhipball"
set rthigh_xlate "$rot_node:rthgxlate"
set rhip_joint "$rthigh_xlate:rhipjt"
set rthigh_org "$rhip_joint:rthgorg"
set rthigh "$rthigh_org:rthigh"
set rhipball "$rthigh_org:rhipball"
set lcalf_xlate "$lthigh_org:lclfxlate"
set lknee_joint "$lcalf_xlate:lkneejt"      
set lcalf_org "$lknee_joint:lclforg"
set lcalf "$lcalf_org:lcalf"
set rcalf_xlate "$rthigh_org:rclfxlate"
set rknee_joint "$rcalf_xlate:rkneejt"
set rcalf_org "$rknee_joint:rclforg"
set rcalf "$rcalf_org:rcalf"
set lfoot_xlate "$lcalf_org:lftxlate"
set lankle_joint "$lfoot_xlate:lankjt"        
set rfoot_xlate "$rcalf_org:rftxlate"
set rankle_joint "$rfoot_xlate:rankjt"        
set tail_xlate "$rot_node:tailxlate"
set tail_joint "$tail_xlate:tail_joint"       
set tail_orig "$tail_joint:tail_orig"
set tail "$tail_orig:tail"

    set torso_bl1 "$torso_scl:b"
    set torso_wh1 "$torso_scl:w1"
    set torso_bl2 "$shoulder_scale:b"
    set torso_wh2 "$shoulder_scale:w1"

    set foot_org1 "$lankle_joint:footorg"
    set foot_base1 "$foot_org1:base"
    set toe11 "$foot_org1:toe1"
    set toe12 "$foot_org1:toe2"
    set toe13 "$foot_org1:toe3"

    set foot_org2 "$rankle_joint:footorg"
    set foot_base2 "$foot_org2:base"
    set toe21 "$foot_org2:toe1"
    set toe22 "$foot_org2:toe2"
    set toe23 "$foot_org2:toe3"


tux_material white_penguin {0.58 0.58 0.58 } {0.2 0.2 0.2} 50
tux_material black_penguin {0.1 0.1 0.1} {0.5 0.5 0.5} 20
tux_material beak_colour {0.64 0.54 0.06} {0.4 0.4 0.4} 5
tux_material nostril_colour {0.48 0.32 0.034} {0 0 0} 1
tux_material iris_colour {0.01 0.01 0.01} {0.4 0.4 0.4} 90

tux_root_node $root
tux_transform : t1
tux_transform :t1 r1

tux_scale $rot_node {0 0 0} { 0.35 0.35 0.35 }
tux_transform $rot_node tors_scl
tux_scale $torso_scl {0 0 0} { 0.90 0.9 0.9 }

    tux_sphere $torso_scl b 1.0
    tux_surfaceproperty $torso_bl1 black_penguin
    tux_scale $torso_bl1 {0 0 0} { 0.95 1.0 0.8 }

    tux_sphere $torso_scl w1 1.0
    tux_surfaceproperty $torso_wh1 white_penguin
    tux_translate $torso_wh1 { 0.0 0 0.17 }
    tux_scale $torso_wh1 {0 0 0} {0.8 0.9 0.7}
    tux_shadow $torso_wh1 off

tux_transform $rot_node t2
tux_translate $shoulder_xlate { 0 0.4 0.05 }
tux_transform $shoulder_xlate scl
tux_scale $shoulder_scale {0 0 0} { 0.72 0.72 0.72 }

    tux_sphere $shoulder_scale b 1.0
    tux_surfaceproperty $torso_bl2 black_penguin
    tux_scale $torso_bl2 {0 0 0} { 0.95 1.0 0.8 }

    tux_sphere $shoulder_scale w1 1.0
    tux_surfaceproperty $torso_wh2 white_penguin
    tux_translate $torso_wh2 { 0.0 0 0.17 }
    tux_scale $torso_wh2 {0 0 0} {0.8 0.9 0.7}
    tux_shadow $torso_wh2 off

tux_transform $rot_node t3
tux_translate $neck_xlate { 0 0.9 0.07 }
tux_rotate $neck_xlate y +90
tux_transform $neck_xlate neckjt   
tux_neck $neck_joint
tux_transform $neck_joint t4
tux_rotate $neck_org y -90
tux_transform $neck_org scl
tux_scale $neck_scale {0 0 0} { 0.45 0.5 0.45 }
tux_sphere $neck_scale neck 0.8
tux_surfaceproperty $neck black_penguin
tux_sphere $neck_scale neckw 0.66
tux_surfaceproperty $neckw white_penguin
tux_translate $neckw {0 -0.08 0.35 }
tux_scale $neckw {0 0 0} {0.8 0.9 0.7}
tux_shadow $neckw off
tux_transform $neck_org t5

tux_translate $head_xlate { 0 0.3 0.07 }
tux_rotate $head_xlate y +90
tux_transform $head_xlate hdjtrot
tux_transform $head_joint_rot hdjt
tux_head $head_joint
tux_transform $head_joint hdorg
tux_rotate $head_org y -90
tux_translate $head_org {0 0.20 0}
tux_sphere $head_org hd 1.0
tux_surfaceproperty $head black_penguin
tux_scale $head {0 0 0} {0.42 0.5 0.42}
tux_sphere $head_org bk1 0.8

tux_surfaceproperty $beak1 beak_colour
tux_translate $beak1 { 0 -0.205 0.3 }
tux_rotate $beak1 x +10
tux_scale $beak1 {0 0 0} { 0.23 0.12 0.4 }

tux_sphere $head_org bk3 0.66
tux_surfaceproperty $beak3 beak_colour
tux_translate $beak3 { 0 -0.23 0.3 }
tux_rotate $beak3 x +10
tux_scale $beak3 {0 0 0} { 0.21 0.17 0.38 }
tux_shadow $beak3 off

tux_sphere $head_org le 0.66
tux_surfaceproperty $left_eye white_penguin
tux_translate $left_eye {0.13 -0.03 0.38}
tux_rotate $left_eye y 18
tux_rotate $left_eye z 5
tux_rotate $left_eye x 5
tux_scale $left_eye {0 0 0} {0.1 0.13 0.03}
tux_shadow $left_eye off

tux_sphere $head_org re 0.66
tux_surfaceproperty $right_eye white_penguin
tux_translate $right_eye {-0.13 -0.03 0.38}
tux_rotate $right_eye y -18
tux_rotate $right_eye z -5
tux_rotate $right_eye x 5
tux_scale $right_eye {0 0 0} {0.1 0.13 0.03}
tux_shadow $right_eye off

tux_sphere $head_org li 0.66

tux_surfaceproperty $left_iris iris_colour
tux_translate $left_iris {0.12 -0.045 0.40}
tux_rotate $left_iris y 18
tux_rotate $left_iris z 5
tux_rotate $left_iris x 5
tux_scale $left_iris {0 0 0} {0.055 0.07 0.03}
tux_shadow $left_iris off
tux_eye $left_iris left

tux_sphere $head_org ri 0.66
tux_surfaceproperty $right_iris iris_colour
tux_translate $right_iris {-0.12 -0.045 0.40}
tux_rotate $right_iris y -18
tux_rotate $right_iris z -5
tux_rotate $right_iris x 5
tux_scale $right_iris {0 0 0} {0.055 0.07 0.03}
tux_shadow $right_iris off
tux_eye $right_iris right

tux_transform $shoulder_xlate luarmxlate
tux_rotate $luparm_xlate y 180
tux_translate $luparm_xlate {-0.56 0.3 0}
tux_rotate $luparm_xlate z +45
tux_rotate $luparm_xlate x +90

tux_transform $luparm_xlate lshjt
tux_left_shoulder $lshoulder_joint
tux_transform $lshoulder_joint luarmorg

tux_rotate $luparm_org x -90
tux_translate $luparm_org {-0.22 0 0}
tux_sphere $luparm_org uparm 0.66
tux_surfaceproperty $luparm black_penguin
tux_scale $luparm {0 0 0} {0.34 0.1 0.2}

tux_transform $shoulder_xlate ruarmxlate
tux_translate $ruparm_xlate {-0.56 0.3 0}
tux_rotate $ruparm_xlate z +45
tux_rotate $ruparm_xlate x -90
tux_transform $ruparm_xlate rshjt    
tux_right_shoulder $rshoulder_joint

tux_transform $rshoulder_joint ruarmorg
tux_rotate $ruparm_org x +90
tux_translate $ruparm_org {-0.22 0 0}
tux_sphere $ruparm_org uparm 0.66
tux_surfaceproperty $ruparm black_penguin
tux_scale $ruparm {0 0 0} {0.34 0.1 0.2}

tux_transform $luparm_org llarmxlate
tux_translate $llowarm_xlate {-0.23 0 0}
tux_rotate $llowarm_xlate z 20
tux_rotate $llowarm_xlate x +90
tux_transform $llowarm_xlate lelbjt
tux_transform $lelbow_joint llarmorg
tux_rotate $llowarm_org x -90
tux_translate $llowarm_org {-0.19 0 0}
tux_sphere $llowarm_org llowarm 0.66
tux_surfaceproperty $llowarm black_penguin
tux_scale $llowarm {0 0 0} {0.30 0.07 0.15}

tux_transform $ruparm_org rlarmxlate

tux_translate $rlowarm_xlate {-0.23 0 0}
tux_rotate $rlowarm_xlate z 20
tux_rotate $rlowarm_xlate x -90
tux_transform $rlowarm_xlate relbjt  
tux_transform $relbow_joint rlarmorg
tux_rotate $rlowarm_org x +90
tux_translate $rlowarm_org {-0.19 0 0}
tux_sphere $rlowarm_org rlowarm 0.66
tux_surfaceproperty $rlowarm black_penguin
tux_scale $rlowarm {0 0 0} {0.30 0.07 0.15}

tux_transform $llowarm_org lhandxlate
tux_translate $lhand_xlate {-0.24 0 0}
tux_rotate $lhand_xlate z 20
tux_rotate $lhand_xlate x +90
tux_transform $lhand_xlate lwrtjt
tux_transform $lwrist_joint lhndorg
tux_rotate $lhand_org x -90
tux_translate $lhand_org {-0.10 0 0}

tux_sphere $lhand_org lhand 0.5
tux_surfaceproperty $lhand black_penguin
tux_scale $lhand {0 0 0} {0.12 0.05 0.12}
tux_transform $rlowarm_org rhandxlate
tux_translate $rhand_xlate {-0.24 0 0}
tux_rotate $rhand_xlate z 20
tux_rotate $rhand_xlate x -90
tux_transform $rhand_xlate rwrtjt
tux_transform $rwrist_joint rhndorg
tux_rotate $rhand_org x +90
tux_translate $rhand_org {-0.10 0 0}
tux_sphere $rhand_org rhand 0.5
tux_surfaceproperty $rhand black_penguin
tux_scale $rhand {0 0 0} {0.12 0.05 0.12}

tux_transform $rot_node lthgxlate
tux_rotate $lthigh_xlate y +180
tux_translate $lthigh_xlate {-0.28 -0.8 0}
tux_rotate $lthigh_xlate y +110

tux_transform $lthigh_xlate lhipjt   
tux_left_hip $lhip_joint
tux_transform $lhip_joint lthgorg
tux_rotate $lthigh_org y -110
tux_translate $lthigh_org { 0 -0.1 0 } 
tux_sphere $lthigh_org lthigh 0.5

tux_surfaceproperty $lthigh beak_colour
tux_scale $lthigh {0 0 0} {0.07 0.30 0.07}
tux_sphere $lthigh_org lhipball 0.5
tux_surfaceproperty $lhipball black_penguin
tux_translate $lhipball {0.0 0.05 0.0}
tux_scale $lhipball {0 0 0} {0.09 0.18 0.09}
tux_shadow $lhipball off

tux_transform $rot_node rthgxlate
tux_translate $rthigh_xlate {-0.28 -0.8 0}
tux_rotate $rthigh_xlate y -110

tux_transform $rthigh_xlate rhipjt        
tux_right_hip $rhip_joint
tux_transform $rhip_joint rthgorg
tux_rotate $rthigh_org y +110
tux_translate $rthigh_org { 0 -0.1 0 } 
tux_sphere $rthigh_org rthigh 0.5
tux_surfaceproperty $rthigh beak_colour
tux_scale $rthigh {0 0 0} {0.07 0.30 0.07}
tux_sphere $rthigh_org rhipball 0.5
tux_surfaceproperty $rhipball black_penguin
tux_translate $rhipball {0.0 0.05 0.0}
tux_scale $rhipball {0 0 0} {0.09 0.18 0.09}
tux_shadow $rhipball off

tux_transform $lthigh_org lclfxlate

tux_translate $lcalf_xlate {0 -0.21 0}
tux_rotate $lcalf_xlate y +90
tux_transform $lcalf_xlate lkneejt          
tux_left_knee $lknee_joint
tux_transform $lknee_joint lclforg
tux_rotate $lcalf_org y -90
tux_translate $lcalf_org { 0 -0.13 0 } 
tux_sphere $lcalf_org lcalf 0.5
tux_surfaceproperty $lcalf beak_colour
tux_scale $lcalf {0 0 0} {0.06 0.18 0.06}

tux_transform $rthigh_org rclfxlate
tux_translate $rcalf_xlate {0 -0.21 0}
tux_rotate $rcalf_xlate y -90

tux_transform $rcalf_xlate rkneejt          
tux_right_knee $rknee_joint
tux_transform $rknee_joint rclforg
tux_rotate $rcalf_org y +90
tux_translate $rcalf_org { 0 -0.13 0 } 

tux_sphere $rcalf_org rcalf 0.5
tux_surfaceproperty $rcalf beak_colour
tux_scale $rcalf {0 0 0} {0.06 0.18 0.06}

tux_transform $lcalf_org lftxlate
tux_translate $lfoot_xlate { 0 -0.18 0 }
tux_rotate $lfoot_xlate y -50
tux_transform $lfoot_xlate lankjt

tux_left_ankle $lankle_joint

    tux_transform $lankle_joint footorg
    tux_translate $foot_org1 { -0.13 0 0 }
    tux_surfaceproperty $foot_org1 beak_colour
    tux_scale $foot_org1 { 0 0 0 } { 1.1 1.0 1.3 }

    tux_sphere $foot_org1 base 0.66
    tux_scale $foot_base1 {0 0 0} {0.25 0.08 0.18}

    tux_sphere $foot_org1 toe1 0.66
    tux_surfaceproperty $toe11 beak_colour
    tux_translate $toe11 {-0.07 0 0.1}
    tux_rotate $toe11 y 30
    tux_scale $toe11 {0 0 0} {0.27 0.07 0.11}

    tux_sphere $foot_org1 toe2 0.66
    tux_surfaceproperty $toe12 beak_colour
    tux_translate $toe12 {-0.07 0 -0.1}
    tux_rotate $toe12 y -30
    tux_scale $toe12 {0 0 0} {0.27 0.07 0.11}

    tux_sphere $foot_org1 toe3 0.66
    tux_surfaceproperty $toe13 beak_colour
    tux_translate $toe13 {-0.08 0 0}
    tux_scale $toe13 {0 0 0} {0.27 0.07 0.10}

tux_transform $rcalf_org rftxlate
tux_translate $rfoot_xlate { 0 -0.18 0 }
tux_rotate $rfoot_xlate y +50

tux_transform $rfoot_xlate rankjt
tux_right_ankle $rankle_joint

    tux_transform $rankle_joint footorg
    tux_translate $foot_org2 { -0.13 0 0 }
    tux_surfaceproperty $foot_org2 beak_colour
    tux_scale $foot_org2 { 0 0 0 } { 1.1 1.0 1.3 }

    tux_sphere $foot_org2 base 0.66
    tux_scale $foot_base2 {0 0 0} {0.25 0.08 0.18}

    tux_sphere $foot_org2 toe1 0.66
    tux_surfaceproperty $toe21 beak_colour
    tux_translate $toe21 {-0.07 0 0.1}
    tux_rotate $toe21 y 30
    tux_scale $toe21 {0 0 0} {0.27 0.07 0.11}

    tux_sphere $foot_org2 toe2 0.66
    tux_surfaceproperty $toe22 beak_colour
    tux_translate $toe22 {-0.07 0 -0.1}
    tux_rotate $toe22 y -30
    tux_scale $toe22 {0 0 0} {0.27 0.07 0.11}

    tux_sphere $foot_org2 toe3 0.66
    tux_surfaceproperty $toe23 beak_colour
    tux_translate $toe23 {-0.08 0 0}
    tux_scale $toe23 {0 0 0} {0.27 0.07 0.10}

tux_transform $rot_node tailxlate
tux_translate $tail_xlate {0 -0.4 -0.5}
tux_rotate $tail_xlate x -60
tux_transform $tail_xlate tail_joint
tux_tail $tail_joint
tux_transform $tail_joint tail_orig
tux_translate $tail_orig {0 0.15 0}
tux_sphere $tail_orig tail 0.5
tux_surfaceproperty $tail black_penguin
tux_scale $tail {0 0 0} {0.2 0.3 0.1}
