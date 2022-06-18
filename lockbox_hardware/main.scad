// Measurements in mm

board_width = 26;
board_length = 50;
pins_width = 3;
full_pin_height = 25;
cable_management_height = 10;

servo_width = 23;
servo_height = 30.5;
servo_thickness = 12.5;
servo_wings_width = 32.5;
servo_wings_offset = 16.5;

cam_height = 3;

// Other constants

inner_wall_thickness = 1.2;
outer_wall_thickness = 1.8;
layer_height = 0.32;

cam_bridge_height = 5*layer_height;

// inter-component calculations

box_size_x = outer_wall_thickness*2+board_width;
box_size_z = outer_wall_thickness*2+full_pin_height;
box_size_y = outer_wall_thickness+servo_height+board_length;

// todo: calc openign for cam
cam_slot_size = 16;

// Generation constants

MODELVER = "OOB";

FN = 32;

// Start of code

$fn=FN;
model_version = MODELVER;
make();

module make() {
    box();
    lid();
    cam();
}

module box() //make me
{
    // Main body
    difference()
    {
        
        union()
        {
            // Main chunk
            translate([-0.5*box_size_x, 0, 0])
                cube([box_size_x, box_size_y, box_size_z]);
            // Cam bridge
            translate([-0.5*box_size_x, -cam_bridge_height, 0])
                cube([box_size_x, cam_bridge_height, outer_wall_thickness]);
        }
        // Slits for pins
        // todo: fix to proper width for if box becomes larger than board width
        for (i = [-1, 1])
            translate([i*((box_size_x-(outer_wall_thickness*2))/2 - pins_width/2),0,0])
            translate([-0.5*pins_width, 0, outer_wall_thickness])
                cube([pins_width, box_size_y-outer_wall_thickness, box_size_z-2*outer_wall_thickness]);
        // Room for key
        translate([-(box_size_x-(pins_width*2+outer_wall_thickness*2+inner_wall_thickness*2))/2, 0, outer_wall_thickness+inner_wall_thickness+servo_thickness])
            cube([box_size_x-(pins_width*2+outer_wall_thickness*2+inner_wall_thickness*2), box_size_y-outer_wall_thickness, box_size_z-outer_wall_thickness*2-inner_wall_thickness-servo_thickness]);
        // Allow for bigger key
        //translate([-(box_size_x-(wall_thickness*4))/2, 0, wall_thickness*2+servo_thickness])
        //    cube([box_size_x-(wall_thickness*4), box_size_y-wall_thickness-board_length, box_size_z-wall_thickness*3-servo_thickness]);
        // Room for board, servo
        translate([-(box_size_x-(outer_wall_thickness*2))/2, 0, outer_wall_thickness])
            cube([box_size_x-(outer_wall_thickness*2), box_size_y-outer_wall_thickness, servo_thickness]);
        // Slits for servo wings
        non_wings_len = outer_wall_thickness+board_length+servo_wings_offset;
        wings_len = box_size_y - non_wings_len;
        translate([-(box_size_x)/2, 0, outer_wall_thickness])
            cube([box_size_x, wings_len, servo_thickness]);
        // Cam opening
        translate([-cam_slot_size/2, 0, 0])
            #cube([cam_slot_size, cam_height, outer_wall_thickness]);
        // Power wires opening
        translate([(box_size_x/2)-pins_width-outer_wall_thickness,box_size_y-outer_wall_thickness,box_size_z-outer_wall_thickness-pins_width*2])
            cube([pins_width, outer_wall_thickness, pins_width*2]);
        // Version text
        translate([0,box_size_y-layer_height,0])
        rotate([90,0,180])
        linear_extrude(layer_height)
        translate([-6,6,0])
        union()
        {
            translate([-6,0,0])
                text(text="V: ", size = 4);
            text(text=model_version, size = 4);
        }
        // Logo
        logo_width = 12;
        translate([2,box_size_y,14])
        rotate([90,0,180])
            scale([logo_width/64,logo_width/64,layer_height/255])
            surface(file="eki-logo-simple.png", invert = true);
    }
}

module lid() //make me
{
}

module cam() //make me
{
}
