#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "io.h"
#include "movement.h"
#include "lib/open_interface.h"
#include "lib/util.h"
#include "lib/lcd.h"
#include "scan.h"
#include "bluetooth.h"
#include "ui.h"
#include "sound.h"

void ui_control(void);
void autonomous(void);
void evasive_action(int dist, char left_evasive);
int path_blocked_w_data(int target_dist, obj_t* objects, int count);
int find_goal(obj_t* objects, int count, int* dist, int* final_angle);

oi_t* sensor_data;
const char WIDTH_THRESHOLD = 3;

/// Main function
/**
 * This is the main function for the robot.  It handles the main user input loop.
 */
int main(void)
{
	int initialzed = 0;
		
	// Init hardware
	init_UART();
	lcd_init();
	sensor_data = init_iRobot();
	
	lprintf("Welcome to\nBlastOffToMars.c!");
	
    while(1)
	{
		menu_option user_choice = main_menu();
		if (user_choice == INIT){
			send_msg("Welcome to IRobot Mars Robot interface\r\n");
			send_msg("The servo and IR sensor are being initialized\r\n");
			lprintf("The hardware has been initialized.");
			//init hardware methods
			init_servo();
			init_ir();
			songs(MARIO);
			initialzed = 1;
		}
		else if (user_choice==STANDBY){
			send_msg("The robot has reached the retrieval zone\r\n");
			songs(DARTHVADER);
			lprintf("The robot has reached the retrieval zone");
		}
		else if (user_choice==ZONE_IDENTIFY){
			send_msg("The robot has identified the retrieval zone\r\n");
			lprintf("The robot has identified the retrieval zone");
		}
		else if (user_choice==MOVEMENT){
			if (!initialzed) {
				send_msg("Please initialize the robot first.\r\n");
			} else {
				move_menu(sensor_data, 0);
			}
		} else if (user_choice==MOVEMENT_NO_SENSOR) {
			move_menu(sensor_data, 1);
		}
		else if (user_choice==SCAN){
			if (!initialzed) {
				send_msg("Please initialize the robot first.\r\n");
			} else {
				lprintf("Scanning the area");
								
				show_sensors(sensor_data);
				send_msg("Scanning the area...\r\n");
				show_objects();
			}
		} else if (user_choice == AUTO) {
			// AUTO
			autonomous();
		} else if (user_choice == PROGRAM_UI) {
			ui_control();
		}
	}
}

/// Hidden mode for GUI frontend interface
/**
 * This is a hidden menu that is designed for our GUI frontend to communicate with.  It has simplified commands to reduce frontend parsing and USART transmission sizes.
 */
void ui_control(void) {
	in_program_ui = 1;
	stop_reason reason;
	int result;
	char ignore_sensors;
	char msg[80];
	init_servo();
	init_ir();
	
	char user_input[20];
	while (1) {
		ignore_sensors = 0;
		read_line(user_input, 20);
		switch (user_input[0]) {
		case 'a':
			autonomous();
		case 'e':
			// Send sensor data
			show_sensors(sensor_data);
			break;
		case 'i':
			// Move ignoring sensors
			ignore_sensors = 1;
		case 'm':
			// Move
			result = move_result(atoi(user_input + 2), sensor_data, ignore_sensors, ignore_sensors, &reason);
			sprintf(msg, "m,%d,%s.", result, stop_reason_descrip[reason]);
			send_msg(msg);
			break;
		case 'r':
			// Rotate
			result = rotate_deg(atoi(user_input + 2), sensor_data);
			sprintf(msg, "r,%d.", result);
			send_msg(msg);
			break;
		case 'c':
			// Scan
			show_objects();
			break;
		case  'o':
			songs(DARTHVADER);
			break;
		}
	}
}

/// Autonomous mode
/**
 * This is the autonomous loop.  The robot will search for the end goal endlessly until it thinks it finds the goal.
 */
void autonomous(void) {
	const int EXPLORE_DIST = 500;
	int count;
	obj_t* objects;
	int dist;
	stop_reason reason;
	int index;
	int angle;
	char msg[80];
	char left_evasive;
	
	while (1) {
		lprintf("Scanning area");
		send_msg("Scanning area\r\n");
		left_evasive = 1;
		
		objects = do_scan(&count);
		if (find_goal(objects, count, &dist, &angle) != -1) {
			if ((index = path_blocked_w_data(EXPLORE_DIST, objects, count)) != -1) {
				send_msg("Path blocked\r\n");
				// Path is blocked
				int offset_angle = side_side_side(10 + objects[index].width / 2, objects[index].dist);
				sprintf(msg, "Path blocked in exploration, rotating %d to avoid object\r\n", offset_angle);
				send_msg(msg);
				rotate_deg(offset_angle, sensor_data);
			} else {
				// Found a small object
				send_msg("Small objects found\r\n");
				int offset_angle = find_goal(objects, count, &dist, &angle);
				rotate_deg(offset_angle - 90, sensor_data);
				dist = move_result(dist, sensor_data, 0, 0, &reason);
				if (reason == COLOR) {
					lprintf("WE WIN!");
					send_msg("WE WIN1!\r\n");
					songs(DARTHVADER);
					while (1) {}
				}
				rotate_deg(angle, sensor_data);
				dist = move_result(400, sensor_data, 0, 0, &reason);
				if (reason == COLOR) {
					lprintf("WE WIN!");
					send_msg("WE WIN2!\r\n");
					songs(DARTHVADER);
					while (1) {}
				}
			}
		} else {
			// Did not find a small object.  Explore
			if ((index = path_blocked_w_data(EXPLORE_DIST, objects, count)) != -1) {
				send_msg("Path blocked\r\n");
				// Path is blocked
				int offset_angle = side_side_side(10 + objects[index].width / 2, objects[index].dist);
				sprintf(msg, "Path blocked in exploration, rotating %d to avoid object\r\n", offset_angle);
				send_msg(msg);
				rotate_deg(offset_angle, sensor_data);
			}
			// Path is not blocked.  Continue
			dist = move_result(EXPLORE_DIST, sensor_data, 0, 0, &reason);
			sprintf(msg, "Reason: %s\r\n", stop_reason_descrip[reason]);
			send_msg(msg);
		}
		static char rred = 0;
		switch (reason) {
		case BUMP_L:
			left_evasive = 0;
		case BUMP_R:
			// Evasive action
			send_msg("That's an object.\r\n");
			lprintf("Ouch!");
			if (rred == 0) { songs(RICKROLLED); }
			rred = 1;
			evasive_action(200, left_evasive);
			break;
		case CLIFF_L:
			left_evasive = 0;
		case CLIFF_R:
			// Evasive action
			send_msg("That's a cliff.\r\n");
			lprintf("That's a cliff");
			if (rred == 0) { songs(RICKROLLED); }
			rred = 1;
			evasive_action(500, left_evasive);
			break;
		case COLOR:
			send_msg("Found an edge.\r\n");
			char obj_in_range = 0;
			objects = do_scan(&count);
			for (int i = 0; i < count; i++) {
				if (objects[i].dist < 50 && objects[i].angular_location < 90) {
					obj_in_range = 1;
				}
				if (obj_in_range && objects[i].dist < 50 && objects[i].angular_location > 90) {
					lprintf("WE WIN!");
					send_msg("WE WIN\r\n");
					move_result(150, sensor_data, 0, 1, &reason);
					songs(DARTHVADER);
					while (1) {}
				}
			}
			lprintf("I don't want to go there");
			if (rand() % 1) {
				rotate_deg(100, sensor_data);
			} else {
				rotate_deg(-100, sensor_data);
			}
			break;
		default: break;
		}
	}
}

/// Searches the surrounding area for objects that look like the goal
/**
 * This function searches the surrounding area for small objects that appear to be about 60 cm apart.
 * @param objects the list of objects from a scan (so we don't need to repeat the scan)
 * @param count the number of objects in the objects array
 * @param dist (return) the estimated distance between the robot and the goal posts
 * @param final_angle (return) the angle to rotate the robot to face the final goal after moving toward it
 * @return the angle to the center of the goal posts, or -1 if the goals are not found
 */
int find_goal(obj_t* objects, int count, int* dist, int* final_angle) {
	char msg[80];
	for (int i = 0; i < count; i++) {
		if (objects[i].width < WIDTH_THRESHOLD) {
			for (int j = i + 1; j < count; j++) {
				if (objects[j].width < WIDTH_THRESHOLD) {
					int angle = objects[j].angular_location - objects[i].angular_location;
					int separation = side_angle_side2(angle, objects[j].dist, objects[i].dist);
					if (separation > 40 && separation < 80) {
						sprintf(msg, "Separation between two small objects: %d (%d - %d) center %d\r\n", separation, objects[j].angular_location, objects[i].angular_location, objects[i].angular_location + angle / 2);
						send_msg(msg);
						*final_angle = sin((objects[i].dist - objects[j].dist) / (double) separation);
						*dist = (objects[i].dist + objects[j].dist) / 2;
						return objects[i].angular_location + angle / 2;
					}
				}
			}
		}
	}
	return -1;
}

/// Rotates to avoid an object
/**
 * Rotates the robot to avoid hitting an object that has been found.
 */
void evasive_action(int dist, char left_evasive) {
	if (left_evasive) {
		send_msg("Evasive action, to the left\r\n");
		rotate_deg(90, sensor_data);
	} else {
		send_msg("Evasive action, to the left\r\n");
		rotate_deg(-90, sensor_data);
	}
}

int path_blocked_w_data(int target_dist, obj_t* objects, int count) {
	int dist;
	int horiz_dist;
	char msg[80];
	for (int i = 0; i < count; i++) {
		dist = objects[i].dist * sin(objects[i].angular_location / 180.0 * M_PI);
		horiz_dist = objects[i].dist * cos(objects[i].angular_location / 180.0 * M_PI);
		if (abs(horiz_dist) < 10 && abs(dist) < target_dist) {
			sprintf(msg, "Path blocked at %d deg, %d dist.  %d to the right, %d ahead.\r\n", objects[i].angular_location, objects[i].dist, horiz_dist, dist);
			send_msg(msg);
			return i;
		}
	}
	return -1;
}