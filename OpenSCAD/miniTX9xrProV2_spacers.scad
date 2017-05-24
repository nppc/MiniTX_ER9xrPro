$fn=90;
for(ix=[0:10:20])for(iy=[0:10:20])translate([ix,iy,0])spacer();
    
module spacer(){
    difference(){
        union(){
            cylinder(d=5.5, h=3.7, center=true);
            translate([0,0,-2])cylinder(d2=5.5, d1=5.2, h=0.3, center=true);
        }
        cylinder(d=3.5,h=5,center=true);
        translate([0,0,-2.05])cylinder(d1=3.8,d2=3.5, h=0.3,center=true);
    }
}