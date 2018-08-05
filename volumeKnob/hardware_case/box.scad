/* box + lid */

ep = 3;
$fn=60;

// screw supports
module screw() {
	difference() {
		cylinder (d=5, h=30, center=true);
		cylinder (d=3, h=32, center=true);
	}
}

// main cube
module bcube() {
	minkowski() {
		cube ([30+ep,40+ep,25+ep], center=true);
		sphere (d=10);
	}
}

module box() {
	difference() {
		bcube();

		// remove lid
		translate ([0,0,29.5])
			cube ([55,55,35], center=true);

		// remove interior
		cube ([35,45,35+1], center=true);

		// usb opening
		translate ([23,-6,-6.5])
			cube ([15,13,12], center=true);
	}

	translate ([ 16, 21,-3]) screw();
	translate ([-16, 21,-3]) screw();
	translate ([ 16,-21,-3]) screw();
	translate ([-16,-21,-3]) screw();

}

// speeker 2D drawing
module hp() {
	square(size=[0.5,1]);
	polygon ( [ [0.5,1], [1,1.5], [1,-0.5], [0.5,0]  ] );
}

module lid() {
	difference() {
		bcube();
		// keep lid
		translate ([0,0,-4])
			cube ([55,55,35], center=true);
		// screw holes
		translate ([ 16, 21,15]) cylinder (d=3.5, h=10, center=true);
		translate ([-16, 21,15]) cylinder (d=3.5, h=10, center=true);
		translate ([ 16,-21,15]) cylinder (d=3.5, h=10, center=true);
		translate ([-16,-21,15]) cylinder (d=3.5, h=10, center=true);
		// rotary encoder shaft
		translate ([ 0,11,15]) cylinder (d=8, h=10, center=true);
		// rotary encoder mounting holes
		translate ([-8,2.5,15]) cylinder (d=4, h=10, center=true); // screw
		translate ([-8,2.5,17.5]) cylinder (d=5.2, h=3, center=true); // head
		translate ([-8,16,15]) cylinder (d=4, h=10, center=true);
		translate ([-8,16,17.5]) cylinder (d=5.2, h=3, center=true); // head
		// speeker drawing
		translate([5,-18,16]) linear_extrude(height=3) scale(10) rotate([0,0,90])  hp();
	}
	// texte
	color ("red") {
		translate ([-1,16,17]) {
			linear_extrude(3) {
				rotate ([0,0,90])
					text ("11", size=4);
			}
		}
		translate ([-1,2,17]) {
			linear_extrude(3) {
				rotate ([0,0,90])
					text ("0", size=4);
			}
		}
	}
}

box();
//lid();
