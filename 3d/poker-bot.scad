
// Outer ring: 39mm outer dia (19.5mm r), 33mm inner dia (16.5mm r), 3.4mm thick
// Split into 16 alternating black and white sections
for (i = [0:7]) {
    rotate([0, 0, i * 45]) 
    rotate_extrude(angle = 27, $fn = 256)
    difference() {
        union() {
            translate([16.5,0,0])
            square([2.6, 3.4]);
            translate([16.5,3,0])
            circle(0.4);
            translate([19.1,3,0])
            circle(0.4);
            translate([19.1,0.4,0])
            circle(0.4);
            translate([19,0.4,0])
            square([0.5,2.6]);
            translate([16.0,0,0])
            square([1.0,0.9]);
            translate([16,0,0])square([1,1.7]);
        }
        translate([16.5,2.15,0])circle(0.45);
    }
}

rotate_extrude(angle = 360, $fn = 256)
difference() {
    union() {
        translate([13,0,0])
        square([3, 0.5]);
        translate([12.75,0.5,0])
        circle(0.5);   
        translate([16,0,0])
        circle(0.5);   
    }
    translate([0,0.5,0])square([17,0.5]);
    translate([0,-0.5,0])square([17,0.5]);
}