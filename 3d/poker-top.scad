
// Outer ring: 39mm outer dia (19.5mm r), 33mm inner dia (16.5mm r), 3.4mm thick
// Split into 16 alternating black and white sections
for (i = [0:7]) {
    rotate([0, 0, i * 45]) 
    rotate_extrude(angle = 18, $fn = 256)
    difference() {
        union() {
            translate([16.5,0,0])
            square([2.6, 2.2]);
            translate([16.5,1.8,0])
            circle(0.4);
            translate([19.1,1.8,0])
            circle(0.4);
            translate([19,0,0])
            square([0.5,1.8]);
            translate([16,0,0])square([1,0.5]);
        }
        translate([16.5,0.95,0])circle(0.45);
    }
}

rotate_extrude(angle = 360, $fn = 256)
union() {
    translate([12.5,0,0])
    square([3.5, 0.5]);
}

translate([40, 0, 0])
for (i = [0:7]) {
    rotate([0, 0, i * 45]) 
    rotate_extrude(angle = 18, $fn = 256)
    union() {
        translate([16,0,0])
        square([3.5,0.5]);
        translate([16.5,0,0])
        square([3,0.7]);
        translate([19.1,0.7,0])
        circle(0.4);
        translate([16.5,0,0])
        square([2.6,1.1]);
    }
}

translate([40, 0, 0])
rotate_extrude(angle = 360, $fn = 256)
union() {
    translate([12.5,0,0])
    square([3.5, 0.5]);
}

