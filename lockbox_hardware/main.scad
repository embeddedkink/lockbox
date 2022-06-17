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

// Other constants

wall_thickness = 0.8;

FN = 32;

$fn=FN;
make();

module make() {
    box();
    lid();
    cam();
}

module box() //make me
{
    box_size_x = wall_thickness*2+board_width;
    box_size_z = wall_thickness*2+full_pin_height;
    box_size_y = wall_thickness+servo_height+board_length;

    // Main body
    difference()
    {
        // Main chunk
        translate([-0.5*box_size_x, 0, 0])
            cube([box_size_x, box_size_y, box_size_z]);
        // Slits for pins
        for (i = [-1, 1])
            translate([i*((box_size_x-(wall_thickness*2))/2 - pins_width/2),0,0])
            translate([-0.5*pins_width, 0, wall_thickness])
                cube([pins_width, box_size_y-wall_thickness, box_size_z-2*wall_thickness]);
        // Room for key
        translate([-(box_size_x-(pins_width*2+wall_thickness*4))/2, 0, wall_thickness*2+servo_thickness])
            cube([box_size_x-(pins_width*2+wall_thickness*4), box_size_y-wall_thickness, box_size_z-wall_thickness*3-servo_thickness]);
        translate([-(box_size_x-(wall_thickness*4))/2, 0, wall_thickness*2+servo_thickness])
            cube([box_size_x-(wall_thickness*4), box_size_y-wall_thickness-board_length, box_size_z-wall_thickness*3-servo_thickness]);
        // Room for board, servo
        translate([-(box_size_x-(wall_thickness*2))/2, 0, wall_thickness])
            cube([box_size_x-(wall_thickness*2), box_size_y-wall_thickness, servo_thickness]);
        // Slits for servo wings
        non_wings_len = wall_thickness+board_length+servo_wings_offset;
        wings_len = box_size_y - non_wings_len;
        translate([-(box_size_x)/2, 0, wall_thickness])
            cube([box_size_x, wings_len, servo_thickness]);
    }
}

module lid() //make me
{
}

module cam() //make me
{
}
