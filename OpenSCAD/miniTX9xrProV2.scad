// mini transmitter with 9xr pro gimbals
// 1S battery for easy charging
$fn=100;

// CASE (upper wall 3mm, sides 2mm)
case_x=145;
case_y=75;
case_z=40;
case_roundness=10;

// GIMBAL
// distance between holes 47mm (quadrat)
 holedist=47; // distance between mounting holes
 roundcutout=47.5; // diameter of round part that comes out of the case 
 plate=54; // mounting plate size (where holes is)



difference(){
    union(){
        //caseTop();
        //caseMiddle();
        caseBottom();

    translate([case_x/2-11,-case_y/2+3,case_z/2-2])rotate([0,0,90]){
        //rotate([-90,0,0]){antennacase();dipole();}
        //antennamountcoverfull();
    }
    translate([-case_x/2+11,-case_y/2+3,case_z/2-2])rotate([0,0,90]){
        //rotate([90,0,0]){antennacase();dipole();}
        //antennamountcover();
    }

    }
}


//rotate([-180,0,0])rotate([0,90,180])translate([10,23,48.6])rotate([0,0,10])handsupport();


/*
translate([case_x/4-4,5,case_z/2-2.5])rotate([0,0,180])txgimbal();
translate([-case_x/4+4,5,case_z/2-2.5])txgimbal();



translate([0,-11,case_z/2-0.6])microdipswitch();
translate([0,11+10,case_z/2-0.6])microdipswitch();


translate([-case_x/3,-case_y/2+2,0])rotate([90,90,0])switch();
translate([case_x/3,-case_y/2+2,0])rotate([90,90,0])switch();



translate([-case_x/3.2,-case_y/2+13,case_z/2-25])rotate([90,0,0])cc2500();
translate([case_x/4.6,-case_y/2+13,case_z/2-12])rotate([90,0,0])NRF24L01P();
translate([-case_x/8,-case_y/2+10,case_z/2-20])rotate([90,90,0])arduinoMini();
translate([case_x/2-24,-case_y/2+10,case_z/2-29])rotate([90,0,0])arduinoNano();

translate([0,0,5])rotate([90,0,0])battery();
translate([0,0,5-13])rotate([90,0,0])battery();
*/

module caseTop() {
     difference(){
        union(){
            difference(){
                 case_upper(case_x,case_y,case_z,case_roundness);
                 translate([0,0,-1])case_upper(case_x-4,case_y-4,case_z-3,case_roundness-4);
            }
            // Model (M) text
            translate([4.6,-15,case_z/2-0.3])rotate([0,0,180])linear_extrude(height = 1){text("M", $fn = 20, size = 7, font = "ArialBlack");}
            // Config (C) text
            translate([3.8,32,case_z/2-0.3])rotate([0,0,180])linear_extrude(height = 1){text("C", $fn = 20, size = 7, font = "ArialBlack");}

        }
        // antenna cut
        translate([0,-case_y/2+3,case_z/2-3])rotate([0,90,0])antennacutout();
        translate([case_x/2-11,-case_y/2+3,case_z/2-2])rotate([0,0,90]){antennamounthole();antennamounthole_plate();}
        translate([-case_x/2+11,-case_y/2+3,case_z/2-2])rotate([0,0,90]){antennamounthole();antennamounthole_plate();}
        
       
        //cutout for leds
        translate([-case_x/2+7,-case_y/2+17,case_z/2-1])cutoutforled();
        translate([-case_x/3,-case_y/2+11,case_z/2-1])cutoutforled();
        translate([case_x/2-7,-case_y/2+17,case_z/2-1])cutoutforled();
        translate([case_x/3,-case_y/2+11,case_z/2-1])cutoutforled();
        
        //cutout for gimbals
        translate([case_x/4-4,5,case_z/2-2.5]){
            cylinder(d=roundcutout+0.6,h=6,center=true);
            holesforgimbalmount(3.3);
            translate([0,0,9])holesforgimbalmount(6.5);
        }
        translate([-case_x/4+4,5,case_z/2-2.5]){
            cylinder(d=roundcutout+0.6,h=6,center=true);
            holesforgimbalmount(3.3);
            translate([0,0,9])holesforgimbalmount(6.5);
        }
        
        // cutout for powerswitch
        translate([0,5,case_z/2-2.3])powerswitchholdercutout();
        
        //cutout for dipswitch
        translate([0,-11,case_z/2-0.6])hole_microdipswitch();
        translate([0,11+10,case_z/2-0.6])hole_microdipswitch();


        //cutout for hand holder
        translate([-case_x/2,case_y/2-10,0])cube([4,30,100],center=true);
        translate([case_x/2,case_y/2-10,0])cube([4,30,100],center=true);
        

    translate([0,0,-2.50])cube([case_x+1,case_y+1,case_z], center=true);
        
    translate([18,case_y/2-4,0])screwheadcut();
    translate([-18,case_y/2-4,0])screwheadcut();

    translate([case_x/2-6,case_y/2-4.5,0])screwheadcut();
    translate([-case_x/2+6,case_y/2-4.5,0])screwheadcut();
        
    translate([18,-case_y/2+9,0])screwheadcut();
    translate([-18,-case_y/2+9,0])screwheadcut();
        

    //reinforcements for join top/middle parts
    translate([-case_x/2+3,0,case_z/2-5.025])translate([1,0,3])cube([1.4,20.4,2], center=true);

    translate([case_x/2-3,0,case_z/2-5.025])rotate([0,0,180])        translate([1,0,3])cube([1.4,20.4,2], center=true);

    }

}

module caseMiddle(){
     difference(){
        union(){
            difference(){
                 case_upper(case_x,case_y,case_z,case_roundness);
                 translate([0,0,-1])case_upper(case_x-4,case_y-4,case_z-3,case_roundness-4);
                 cube([case_x-6,case_y-6,case_z], center=true);
            }
            difference(){
                case_lower(case_x,case_y,case_z,case_roundness);
                translate([0,0,1])case_lower(case_x-4,case_y-4,case_z-3,case_roundness-4);
                cube([case_x-6,case_y-6,case_z], center=true);
        }

            translate([0,-case_y/2+case_roundness/2-0.5,case_z/2-case_roundness/2])case_antennacutout();
            // meat for antenna holder (bottom)
            meatforAntennaholder_case();
            mirror([1,0,0])meatforAntennaholder_case();
            // reinforce for cutout handholder
            translate([-case_x/2+2.75,case_y/2-15.5,0])cube([1.5,26,36],center=true);
            translate([case_x/2-2.75,case_y/2-15.5,0])cube([1.5,26,36],center=true);
       
            texts();
        }
        // antenna cut
        translate([0,-case_y/2+3,case_z/2-3])rotate([0,90,0])antennacutout();
        translate([case_x/2-11,-case_y/2+3,case_z/2-2])rotate([0,0,90]){antennamounthole();}
        translate([-case_x/2+11,-case_y/2+3,case_z/2-2])rotate([0,0,90]){antennamounthole();}
        
        // cutouts for switches
        translate([-case_x/3,-case_y/2+2,0])cube([3.5,5,7], center=true);
        translate([case_x/3,-case_y/2+2,0])cube([3.5,5,7], center=true);
        

        //cutout for hand holder
        translate([-case_x/2,case_y/2-10,0])cube([4,30,100],center=true);
        translate([case_x/2,case_y/2-10,0])cube([4,30,100],center=true);

        // cutouts for switches
        translate([-case_x/3,-case_y/2+2,0])cube([3.5,5,7], center=true);
        translate([case_x/3,-case_y/2+2,0])cube([3.5,5,7], center=true);
        
        //cutout for charging 3 pin header
        translate([0,-case_y/2+2,-case_z/2+4])cube([8,6,3], center=true);

        //cutout for serial interface 5 pin header
        translate([0,case_y/2-2,-case_z/2+4])cube([13,6,3], center=true);
       


        translate([0,0,case_z-2.525])cube([case_x+1,case_y+1,case_z], center=true); // temp cut
        translate([0,0,-case_z+2.525])cube([case_x+1,case_y+1,case_z], center=true); // temp cut

    // hole for handsupport mounting
    rotate([-180,0,0])rotate([0,90,180])translate([10,23,0]){
        cylinder(d=11,h=case_x+2,center=true);
        translate([6,0,0])cube([11,11,case_x+2],center=true);
    }

    }
    
    //switch holders
    translate([-case_x/3,-case_y/2+2,0])rotate([90,90,0])switchholder();
    translate([case_x/3,-case_y/2+2,0])rotate([90,90,0])switchholder();
    // Battery holder
    translate([0,-case_y/2+4,0])batteryholder();
    translate([0,case_y/2-4,0])batteryholder();
    
    // case screws holders
    translate([18,case_y/2-4,9.975])case_mountinghole();
    translate([-18,case_y/2-4,9.975])case_mountinghole();
    
    
    translate([18,-case_y/2+9,9.975])case_mountingholefront();
    translate([-18,-case_y/2+9,9.975])case_mountingholefront();
    
    translate([case_x/2-6,case_y/2-4.5,9.975])case_mountinghole_corner();
    translate([-case_x/2+6,case_y/2-4.5,9.975])rotate([0,0,90])case_mountinghole_corner();
    

    // bottom screws sockets
    translate([18,case_y/2-4,-9.975])rotate([0,180,0])case_mountinghole();
    translate([-18,case_y/2-4,-9.975])rotate([0,180,0])case_mountinghole();

    translate([18,-case_y/2+4,-9.975])rotate([0,180,180])case_mountinghole();
    translate([-18,-case_y/2+4,-9.975])rotate([0,180,180])case_mountinghole();

    translate([-case_x/2+6,case_y/2-4.5,-9.975])rotate([0,180,0])case_mountinghole_corner();
    translate([case_x/2-6,case_y/2-4.5,-9.975])rotate([0,180,-90])case_mountinghole_corner();

    translate([case_x/2-5,-case_y/2+5,-9.975])rotate([0,180,180])case_mountinghole_corner();
    translate([-case_x/2+5,-case_y/2+5,-9.975])rotate([0,180,90])case_mountinghole_corner();

    //reinforcements for join top/middle parts
    translate([-case_x/2+3,0,case_z/2-5.025])reinforcement();
    translate([case_x/2-3,0,case_z/2-5.025])rotate([0,0,180])reinforcement();

}

module caseBottom(){
    difference(){
         case_lower(case_x,case_y,case_z,case_roundness);
         translate([0,0,1])case_lower(case_x-4,case_y-4,case_z-3,case_roundness-4);
         // cutout for bind dipswitch 
         translate([0,5,-case_z/2+1])rotate([180,0,0])hole_microdipswitch(); 
        
        //cutout for hand holder
        translate([-case_x/2,case_y/2-10,0])cube([4,30,100],center=true);
        translate([case_x/2,case_y/2-10,0])cube([4,30,100],center=true);

        // main cut
        translate([0,0,2.5])cube([case_x+1,case_y+1,case_z], center=true);
      
                // BIND text
        translate([-23,case_y/3,-case_z/2+0.6])rotate([180,0,0])linear_extrude(height = 1){text("BIND", $fn = 20, size = 12, font = "ArialBlack");}


        // holes for screws
        translate([18,case_y/2-4,0])rotate([0,180,0])screwheadcut();
        translate([-18,case_y/2-4,0])rotate([0,180,0])screwheadcut();

        translate([18,-case_y/2+4,0])rotate([0,180,180])screwheadcut();
        translate([-18,-case_y/2+4,0])rotate([0,180,180])screwheadcut();

        translate([-case_x/2+6,case_y/2-4.5,0])rotate([0,180,0])screwheadcut();
        translate([case_x/2-6,case_y/2-4.5,0])rotate([0,180,-90])screwheadcut();

        translate([case_x/2-5,-case_y/2+5,0])rotate([0,180,180])screwheadcut();
        translate([-case_x/2+5,-case_y/2+5,0])rotate([0,180,90])screwheadcut();
            

    }

}



module case_upper(x,y,z,r){
    len_x=x/2-r/2;
    len_y=y/2-r/2;
    len_z=z/2-r/2;
    hull(){
        translate([len_x,len_y,len_z])sphere(d=r,$fn=$fn/2);
        translate([-len_x,len_y,len_z])sphere(d=r,$fn=$fn/2);
        translate([len_x,-len_y,len_z])sphere(d=r,$fn=$fn/2);
        translate([-len_x,-len_y,len_z])sphere(d=r,$fn=$fn/2);
        translate([len_x,len_y,0])cylinder(d=r,h=1,$fn=$fn/2);
        translate([-len_x,len_y,0])cylinder(d=r,h=1,$fn=$fn/2);
        translate([len_x,-len_y,0])cylinder(d=r,h=1,$fn=$fn/2);
        translate([-len_x,-len_y,0])cylinder(d=r,h=1,$fn=$fn/2);
    }

}

module case_lower(x,y,z,r){
    len_x=x/2-r/2;
    len_y=y/2-r/2;
    len_z=z/2-r/2;
    hull(){
        translate([len_x,len_y,-len_z])sphere(d=r,$fn=$fn/2);
        translate([-len_x,len_y,-len_z])sphere(d=r,$fn=$fn/2);
        translate([len_x,-len_y,-len_z])sphere(d=r,$fn=$fn/2);
        translate([-len_x,-len_y,-len_z])sphere(d=r,$fn=$fn/2);
        translate([len_x,len_y,-1])cylinder(d=r,h=1,$fn=$fn/2);
        translate([-len_x,len_y,-1])cylinder(d=r,h=1,$fn=$fn/2);
        translate([len_x,-len_y,-1])cylinder(d=r,h=1,$fn=$fn/2);
        translate([-len_x,-len_y,-1])cylinder(d=r,h=1,$fn=$fn/2);
    }
    //cube([case_x,case_y,case_z], center=true);
}

module txgimbal(){
// 9xr Pro gimbal dimensions

 color("LIGHTGRAY"){
    difference(){
         union(){
             translate([0,0,1.5])cylinder(d=roundcutout,h=3, center=true);
             translate([0,0,-35/2])cube([plate,plate,35], center=true);
             translate([0,0,-13])cube([38,38,20], center=true);
             translate([-plate/2-2.5,-35/2,-25])cube([2.5,35,25]);
             translate([-plate/2-10,-35/2+10,-25])cube([10,25,15]);
         }
         holesforgimbalmount(3);
         hdh=holedist/2;
         translate([hdh,hdh,0])cube([8,8,8], center=true);
         translate([hdh,-hdh,0])cube([8,8,8], center=true);
         translate([-hdh,hdh,0])cube([8,8,8], center=true);
         translate([-hdh,-hdh,0])cube([8,8,8], center=true);
         
         translate([plate/2-1.5,-plate/2-1,-35-1])cube([2,plate+2,35-5]);
     }
     
 }
 // stick
 color("WHITE")cylinder(d1=3,d2=7,h=30,$fn=10);
}

module holesforgimbalmount(diam){
     hdh=holedist/2;
     translate([hdh,hdh,0])cylinder(d=diam,h=15, center=true);
     translate([hdh,-hdh,0])cylinder(d=diam,h=15, center=true);
     translate([-hdh,hdh,0])cylinder(d=diam,h=15, center=true);
     translate([-hdh,-hdh,0])cylinder(d=diam,h=15, center=true);

}

module cc2500(){
color("BLUE")cube([35,25,5], center=true);
}

module NRF24L01P(){
color("GRAY")cube([46,17,5], center=true);
}

module arduinoNano(){
color("NAVY")cube([44,16,5], center=true);
}

module arduinoMini(){
color("NAVY")cube([34,18,5], center=true);
}

module battery(){
    // 3.7v round Lipo
    color("RED")cylinder(d=12,h=70,$fn=20,center=true);
}

module dipole(){
    translate([0,0,5])color("WHITE"){
        cylinder(d=3,h=26,$fn=20);
        translate([0,0,26])cylinder(d=1,h=26,$fn=20);
    }
}


module case_antennacutout(){
    difference(){ 
        cube([case_x-case_roundness,case_roundness/1.6,case_roundness/1.6], center=true);
        translate([0,case_roundness/2,-case_roundness/2])rotate([45,0,0])cube([case_x,case_roundness,case_roundness], center=true);
    }
}

module antennacase(){
    // mounting ring lenght = 3mm (diam 6.5mm). 
    difference(){
        union(){
            rotate([0,90,0])cylinder(d=8,h=5,center=true);
            translate([4,0,0])rotate([0,90,0])cylinder(d=6.5,h=3,center=true,$fn=$fn/2);
            translate([4+1.75,0,0])rotate([0,90,0])cylinder(d1=6.5,d2=8,h=1,center=true,$fn=$fn/2);
            translate([4+3.25,0,0])rotate([0,90,0])cylinder(d=8,h=2,center=true,$fn=$fn/2);
            difference(){
            translate([-0.5,0,0])cylinder(d=6,h=60,$fn=$fn/2);
            translate([-3.5,-5,-0.5])cube([1,10,61]);
            }
        }
        translate([0.7,0,0])sphere(d=5.5,$fn=20);
        cylinder(d=4,h=61);
        rotate([0,90,0])cylinder(d1=4,d2=5,h=10,$fn=$fn/2);
    }
}



module antennacutout(){
     cylinder(d=6,h=case_x-20,center=true);
     translate([1,-5,0])cube([4,10,case_x-20],center=true);
     translate([-5,-3.5,0])cube([10,13,case_x-20],center=true);
}


module switch(){
color([0.2,0.2,0.2]){
    translate([0,0,-2.5])cube([11,5.5,5], center=true);
    translate([1.5,0,2])cube([3,3,4], center=true);
    difference(){
        translate([0,0,-0.1])cube([11+8,5,0.2], center=true);
        translate([15/2,0,0])cylinder(d=2.2,h=1, center=true, $fn=15);
        translate([-15/2,0,0])cylinder(d=2.2,h=1, center=true, $fn=15);
    }
    translate([0,0,-7])cube([0.3,1.8,4], center=true);
    translate([4,0,-7])cube([0.3,1.8,4], center=true);
    translate([-4,0,-7])cube([0.3,1.8,4], center=true);
}
}

module switchholder(){
    difference(){ 
        translate([-3.25,0,-2])cube([13/2,7,4],center=true);
        cube([11.5,5.7,10],center=true);
        cube([20,5.5,0.8],center=true);
    }
    translate([2,-3.35,-2])cube([31-0.05,1,4],center=true);
    translate([2,3.35,-2])cube([31-0.05,1,4],center=true);
}


module powerswitchholdercutout(){
     hull(){
         cube([5.7,7+2.2*2,1.5],center=true);
         translate([0,0,1.5])cube([3.3,7,1],center=true);
     }
     translate([0,0,1.5])cube([3.3,7,3],center=true);
     cube([5.7,19.6,1.3],center=true);
}

module batteryholder(){
    translate([7.5,0,-2.5])cube([1.5,4,case_z-10.05],center=true);
    translate([-7.5,0,-2.5])cube([1.5,4,case_z-10.05],center=true);
    translate([0,0,12.8])cube([16.5,4,2],center=true);
}


module cutoutforled(){
     translate([0,0,-0])cylinder(d=3.3,h=5,$fn=30, center=true);
     translate([0,0,-1.5])cylinder(d2=3.3,d1=5,h=1.2,$fn=40);
     translate([0,0,-3.5])cylinder(d=5,h=2,$fn=40);
}

module microdipswitch(){
    color([0.2,0.2,0.2]){ 
     translate([0,0,-3.5/2])cube([6,6,3.5], center=true);
     cylinder(d=4, h=2,center=true);
    }
}

module hole_microdipswitch(){
    
     hull(){ 
         translate([0,0,-3/2])cube([6.3,6.3,3], center=true);
         translate([0,0,-4])cube([6.3,11,3], center=true);
     }
     cylinder(d=5, h=2,center=true);
     translate([0,0,1.3])cylinder(d1=5, d2=12,h=2,center=true);
    
}


module handsupport() {
// extendable hand support / sticks cover
    difference(){
        union(){
            hull(){
                cylinder(d=20,h=50,center=true);
                translate([25,0,0])cylinder(d=20,h=50,center=true);
            }
            hull(){
                difference(){
                    translate([20,-45,0])cylinder(d=30,h=50,center=true);
                    translate([-0,-40,0])cube([30,40,55],center=true);
                }
                
                translate([25,0,0])cylinder(d=20,h=50,center=true);
                translate([15,-55,0])cylinder(d=10,h=50,center=true);
                translate([20,0,0])cylinder(d=20,h=50,center=true);
            }
        }
        translate([0,0,-5])cube([60,20,45],center=true);
        translate([0,0,-0.4])cube([60,110,45],center=true);
        
        translate([0,-18,-17])rotate([0,90,0])cylinder(d=10,h=110);
    }
    translate([0,0,20])cylinder(d=13,h=6,center=true);

}


module meatforAntennaholder_case(){
    translate([case_x/2-11,-case_y/2+6.5,case_z/2-case_roundness/2]){
        difference(){
            hull(){
                cube([17,9.4,5],center=true);
                translate([0,-5,-10])cube([5,1,1],center=true);
            }
            translate([5.7,2.1,4])cylinder(d=1.5, h=15, center=true);
            translate([-5.7,2.1,4])cylinder(d=1.5, h=15, center=true);
            
        }
    }
}

module antennamountcoverfull(){
    difference(){antennamountcover();antennamounthole();}
}

module antennamountcover(){
    translate([9.1,0,1.3]){
        difference(){
            union(){
                hull(){
                    cylinder(d=4.4,h=1.5, center=true);
                    translate([-3,0,0])cube([6.5,14.7,1.5], center=true);
                }
                translate([-3,0,-1]){
                    difference(){
                        hull(){
                            rotate([0,90,0])cylinder(d=12,h=6.5, center=true);
                            translate([4,0,0])rotate([0,90,0])cylinder(d=4,h=1, center=true);
                        }
                        translate([0,0,-5])cube([15,15,11], center=true);
                    }
                }
            }
            translate([-3.5,5.7,4]){
                cylinder(d=3, h=6.5, center=true);
                cylinder(d=1.8, h=10, center=true);
            }
            translate([-3.5,-5.7,4]){
                cylinder(d=3, h=6.5, center=true);
                cylinder(d=1.8, h=10, center=true);
            }
            
        }
    }
}

module antennamounthole(){
    translate([-1,0,0])rotate([0,90,0])cylinder(d=8.6,h=7.2,center=true);
    translate([4,0,0])rotate([0,90,0])cylinder(d=6.8,h=3.5,center=true,$fn=$fn/2);
    translate([4+1.75,0,0])rotate([0,90,0])cylinder(d1=6.8,d2=8.3,h=1,center=true,$fn=$fn/2);
    translate([4+3.25,0,0])rotate([0,90,0])cylinder(d=8.3,h=2,center=true,$fn=$fn/2);
    translate([0,0,-0.4])cube([21,5,2], center=true);

}

module antennamounthole_plate(){
    translate([9.09,0,1.5]){
        hull(){
            cylinder(d=5,h=2, center=true);
            translate([-3,0,0])cube([7,15,2], center=true);
        }
        translate([-3.5,5.7,4])cylinder(d=1.8, h=15, center=true);
        translate([-3.5,-5.7,4])cylinder(d=1.8, h=15, center=true);

     }
}


module case_mountinghole(){
    rotate([0,0,180])difference(){
        hull(){
            cylinder(d=6, h=15, center=true);
            translate([0,-2,0])cube([6,2,15], center=true);
        }
        translate([0,0,3])cylinder(d=1.9, h=9, center=true);
        translate([0,2,-8])rotate([45,0,0])cube([15,15,10], center=true);
    }
}

module case_mountingholefront(){
    difference(){
        hull(){
            cylinder(d=6, h=15, center=true);
            translate([0,-6,0])cube([6,2,15], center=true);
        }
        translate([0,0,3])cylinder(d=1.9, h=10, center=true);
        translate([0,4,-8])rotate([45,0,0])cube([20,20,20], center=true);
        translate([0,-7,8])rotate([0,90,0])cylinder(d=9,h=10,center=true);
    }
}

module case_mountinghole_corner(){
    rotate([0,0,180])difference(){
        hull(){
            cylinder(d=6, h=15, center=true);
            translate([0,-2,0])cube([6,2,15], center=true);
            translate([-2,0,0])cube([2,6,15], center=true);
        }
        translate([0,0,3])cylinder(d=1.9, h=9, center=true);
        translate([0,2,-9])rotate([45,0,-45])cube([20,20,10], center=true);
    }
}

module screwheadcut(){
    translate([0,0,case_z/2]){
        cylinder(d=3.8,h=3,center=true);
        cylinder(d=2.3,h=6,center=true);
    }
}

module reinforcement() {
difference(){
    union(){
        translate([0,0,0])cube([3,20,5], center=true);
        translate([1,0,3])cube([1,20,2], center=true);
    }
    translate([3,0,-5])rotate([0,45,0])cube([10,22,10], center=true);
}
}

module texts(){
    // ON text
    translate([case_x/3-6,-case_y/2+0.6,7])rotate([90,0,0])linear_extrude(height = 1){text("ON", $fn = 20, size = 5, font = "ArialBlack");}
    translate([-case_x/3-6,-case_y/2+0.6,7])rotate([90,0,0])linear_extrude(height = 1){text("ON", $fn = 20, size = 5, font = "ArialBlack");}

    // OFF text
    translate([case_x/3-7.5,-case_y/2+0.6,-12])rotate([90,0,0])linear_extrude(height = 1){text("OFF", $fn = 20, size = 5, font = "ArialBlack");}
    translate([-case_x/3-7.5,-case_y/2+0.6,-12])rotate([90,0,0])linear_extrude(height = 1){text("OFF", $fn = 20, size = 5, font = "ArialBlack");}

    // polarity (-+-) text
    translate([-4.7,-case_y/2+0.6,-13])rotate([90,0,0])linear_extrude(height = 1){text("-+-", $fn = 20, size = 5, font = "ArialBlack");}
    // Charge (CHARGE 1S) text
    translate([-17,-case_y/2+0.6,-7])rotate([90,0,0])linear_extrude(height = 1){text("CHARGE 1S", $fn = 20, size = 4, font = "ArialBlack");}

    // Serial (Rx Tx GND Tx Rx) Text
    translate([32,case_y/2-0.6,-12])rotate([90,0,180])linear_extrude(height = 1){text("Rx Tx GND Tx Rx", $fn = 20, size = 5, font = "ArialBlack");}

    // 3.3v text
    translate([-50,case_y/2-0.6,-12])rotate([90,0,180])linear_extrude(height = 1){text("3.3v", $fn = 20, size = 5, font = "ArialBlack");}

    translate([53,case_y/2-0.6,-4])rotate([90,0,180])linear_extrude(height = 1){text("mini-TX", $fn = 20, size = 18, font = "ArialBlack");}
}