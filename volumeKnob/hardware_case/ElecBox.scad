/** Box fort electronic pojects
 * Licence: CC-BY-SA
 */

/* [Parameters] */

// Wich object to create ?
Object = "Lid"; // [Box, Lid, Both]
// Inner box length (mm)
InnerBoxLength = 45;
// Inner box width (mm)
InnerBoxWidth = 40;
// Inner box height (mm)
InnerBoxHeight = 25;
// Walls thickness (mm)
BoxThick = 3;
// Screw diameter (mm)
ScrewSize = 3;

// Resolution
$fn = 30;

/* [Hidden] */
 // wall size around screw
ScrewWall = 2;
// internal variables
BoxLength = InnerBoxLength + ScrewSize + ScrewWall;
BoxWidth = InnerBoxWidth + ScrewSize + ScrewWall;
BoxHeight = InnerBoxHeight;

// speeker 2D drawing
module hp() {
	square(size=[0.5,1]);
	polygon ( [ [0.5,1], [1,1.5], [1,-0.5], [0.5,0]  ] );
}

/** Customize the Lid
 * Starting point is bottom left, when lid is seen from top
 */
module CustomLid () {
	// rotary encoder
	translate ([30, InnerBoxWidth/2, 0]) cylinder (d=8, h=BoxThick); // shaft
	translate ([30-10, InnerBoxWidth/2+8, 0]) cylinder (d=5, h=BoxThick); // screw
	translate ([30-10, InnerBoxWidth/2+8, BoxThick/2]) cylinder (d=6, h=BoxThick/2); // screw head
	translate ([30+7, InnerBoxWidth/2+8, 0]) cylinder (d=5, h=BoxThick); // screw
	translate ([30+7, InnerBoxWidth/2+8, BoxThick/2]) cylinder (d=6, h=BoxThick/2); // screw head
	// speeker drawing
	translate([0,7,BoxThick/2]) linear_extrude(height=BoxThick/2) scale(10) rotate([0,0,0])  hp();
	/** 10x10 mm square removed from the top for half the thickness of the lid */
// 	translate ([0,0,BoxThick/2])
// 		cube ([10,10,BoxThick/2]);
	// texte
	color ("red") {
		translate ([35,InnerBoxWidth/2-5,BoxThick/2]) {
			linear_extrude(BoxThick/2) {
					text ("11", font="Liberation Sans:style=Bold", size=4);
			}
		}
		translate ([21,InnerBoxWidth/2-5,BoxThick/2]) {
			linear_extrude(BoxThick/2) {
					text ("0", font="Liberation Sans:style=Bold", size=4);
			}
		}
	}
}

/** Customize the front face of the Box
 * Starint point is bottom left when Box is seen from front.
 */
module CustomFace () {
	/** 5mm hole in the center of the face */
// 	translate ([InnerBoxLength/2, InnerBoxHeight/2, 0])
// 		cylinder (d=5, h=BoxThick);
	// LED hole
	translate ([2.5,5,0])
		cylinder (d=5, h=BoxThick);
	// USB B hole
	translate ([25,4,0])
		cube ([13,12,BoxThick]);
	// uUSB hole
	translate ([11,3,0])
		cube ([10,5,BoxThick]);
	/** 10x10 mm square removed for half the thickness of the face */
// 	translate ([0,0,BoxThick/2])
// 		cube ([10,10,BoxThick/2]);
	/** Fully opened Box */
// 	cube ([InnerBoxLength, InnerBoxHeight, BoxThick]);
}


/** Internal Code */

/* Box */
module FullBox () {
	difference () {
		union () {
			difference () {
				minkowski () { // main body
					cube ([BoxLength, BoxWidth, BoxHeight], center=true);
					sphere (r = BoxThick);
				}
				// remove inside hole
				cube ([BoxLength, BoxWidth, BoxHeight], center=true);
			}
			// screw terminals
			for ( x = [-1,1] ) {
				for ( y = [-1,1] ) {
					translate ([x * (BoxLength-BoxThick)/2, y * (BoxWidth-BoxThick)/2, 0]) {
						cylinder (d=ScrewSize + ScrewWall, h=BoxHeight, center=true);
					}
				}
			}
		}
		// screw holes
		for ( x = [-1,1] ) {
			for ( y = [-1,1] ) {
				translate ([x * (BoxLength-BoxThick)/2, y * (BoxWidth-BoxThick)/2, BoxThick/2]) {
					cylinder (d=ScrewSize, h=BoxHeight+BoxThick, center=true);
				}
			}
		}
	}
}

module Box () {
	difference () {
		FullBox();
		// remove Lid
		translate ([0,0,BoxHeight])
			cube ([BoxLength+2*BoxThick, BoxWidth+2*BoxThick, BoxHeight], center=true);
		// add (ie:remove) customiztion
		translate ([-InnerBoxLength/2, -(BoxWidth)/2, -BoxHeight/2])
			rotate ([90,0,0])
				CustomFace();
	}
}

module Lid () {
	difference () {
		FullBox();
		// remove Box
		translate ([0,0,-BoxThick/2])
			cube ([BoxLength+2*BoxThick, BoxWidth+2*BoxThick, BoxHeight+BoxThick], center=true);
		// add (ie: remove) customization
		translate ([-(InnerBoxLength)/2,-InnerBoxWidth/2,BoxHeight/2]) {
			CustomLid();
		}
	}
}

module Both() {
	translate ([-(BoxLength+3*BoxThick)/2, 0, 0]) Box();
	translate ([ (BoxLength+3*BoxThick)/2, 0, -(BoxHeight+BoxThick)]) Lid();
}

if (Object == "Box") {
	Box();
} else if (Object == "Lid") {
	Lid();
} else {
	Both();
}
