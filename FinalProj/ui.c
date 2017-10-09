#include "bluetooth.h"
#include "ui.h"
#include "scan.h"
#include "lib/lcd.h"
#include "movement.h"
#include "scan.h"
#include "lib/util.h"
#include <stdio.h>
#include <stdlib.h>

char in_program_ui = 0;

/// Displays the main menu to the user over UART and waits for the choice
/**
 * Sends the main menu over UART and waits for the user to make a selection from the menu.  Invalid input results in repeating the menu and prompt.
 * @return the menu option the user selected
 */
menu_option main_menu()
{
	char user_input[2];
	while (1) {
		send_msg("\r\n");
		send_msg("a) autonomous\r\n");
		send_msg("b) begin the test\r\n");
		send_msg("e) reached the retrieval zone\r\n");
		send_msg("r) retrieval zone has been identified\r\n");
		send_msg("m) move the robot\r\n");
		send_msg("i) move the robot ignoring sensors\r\n");
		send_msg("s) scan the area\r\n");
		send_msg("Your choice: ");
		read_line(user_input, 2);
		send_msg("\r\n\r\n");
	
		switch (user_input[0])
		{
		case 'a':
			return AUTO;
		case 'b':
			return INIT;
		case 'e':
			return STANDBY;
		case 'r':
			return ZONE_IDENTIFY;
		case 'm':
			return MOVEMENT;
		case 'i':
			return MOVEMENT_NO_SENSOR;
		case 's':
			return SCAN;
		case 'p':
			// hidden option for our UI to use
			return PROGRAM_UI;
		default:
			break;
			// do nothing, continue the loop
		}
	}
}

/// Scans the surrounding area and prints the objects that are found.
/**
 * Initiates an IR scan of the surrounding area to find all objects of >1 degree width.  The distance of the object, the angular location, and the calculated width of the object are sent over UART
 * If in_program_ui is set, the output is in a machine readable format.
 */
void show_objects()
{
	int obj_count;
	char msg[100];
	obj_t* objs;
	
	objs = do_scan(&obj_count);
	if (!in_program_ui) {
		sprintf(msg, "Objects found: %d\r\n", obj_count);
		send_msg(msg);
	}
	
	for (int i = 0; i < obj_count; i++) {
		if (in_program_ui) {
			sprintf(msg, "c,%d,%d,%d.", objs[i].dist, objs[i].angular_location, objs[i].width);
		} else {
			sprintf(msg, "%d: angular location: %3d    distance: %3d    width: %3d    angular width: %3d\r\n", i + 1, objs[i].angular_location, objs[i].dist, objs[i].width, objs[i].angular_width);
		}
		send_msg(msg);
	}
}

/// Reads all the relevant sensors and sends them over UART
/**
 * Rotates the servo to 90 degrees, then reads the distance sensor, bump sensors, cliff sensors, and cliff signal sensors and sends the values over UART.
 * If in_program_ui is set, the values are in a machine readable format.
 * @param sensor_data the sensor data structure to work with
 */
void show_sensors(oi_t* sensor_data)
{
	char msg[80];
	oi_update(sensor_data);

	// Distance
	set_servo_pos(90);
	wait_ms(500);
	int val;
	int dist;
	val = ADC_read();
	dist = dist_at_angle(90);
	
	if (in_program_ui) {
		sprintf(msg, "e,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d.", sensor_data->bumper_left, sensor_data->bumper_right, sensor_data->cliff_left, sensor_data->cliff_frontleft, sensor_data->cliff_frontright,
			sensor_data->cliff_right, sensor_data->cliff_left_signal, sensor_data->cliff_frontleft_signal, sensor_data->cliff_frontright_signal, sensor_data->cliff_right_signal, dist);
		send_msg(msg);
	} else {
		sprintf(msg, "IR distance: %3d val %3d\r\n", dist, val);
		send_msg(msg);
		// Cliff
		send_msg("Cliff Sensors\r\n");
		sprintf(msg, "  Left: %4d\tF-Left: %4d\tF-Right: %4d\tRight: %4d\r\n", sensor_data->cliff_left, sensor_data->cliff_frontleft, sensor_data->cliff_frontright, sensor_data->cliff_right);
		send_msg(msg);
		// Cliff brightness
		send_msg("Cliff Sensor Signals\r\n");
		sprintf(msg, "  Left: %4d\tF-Left: %4d\tF-Right: %4d\tRight: %4d\r\n", sensor_data->cliff_left_signal, sensor_data->cliff_frontleft_signal, sensor_data->cliff_frontright_signal, sensor_data->cliff_right_signal);
		send_msg(msg);
		// Bump
		send_msg("Bump:\r\n");
		sprintf(msg, "  Left: %d\tRight: %d\r\n", sensor_data->bumper_left, sensor_data->bumper_right);
		send_msg(msg);
	}
}

/// A sub-menu allowing the user to choose between rotation and linear movement
/**
 * Displays a menu over UART with choices for linear movement and rotation.  Moves or rotates the robot and tells the user the result of the action.
 * @param sensor_data the sensor data structure to work with
 * @param ignore_sensors allows the robot to move despite color and cliff sensors.
 */
void move_menu(oi_t* sensor_data, char ignore_sensors)
{
	stop_reason s_reason;
	send_msg("r [-]#) rotate\r\n");
	send_msg("m [-]#) move\r\n");
	char user_input[8];
	int val;
	char msg[80];
	read_line(user_input, sizeof(user_input));
	send_msg("\r\n");
	switch (user_input[0])
	{
	case 'r':
		val = atoi(user_input + 2);
		val = rotate_deg(val, sensor_data);
		sprintf(msg, "Degrees rotated: %d\r\n", val);
		send_msg(msg);
		break;
	case 'm':
		val = atoi(user_input + 2);
		val = move_result(val, sensor_data, ignore_sensors, ignore_sensors, &s_reason);
		sprintf(msg, "Moved %d\r\nStop reason: %s\r\n", val, stop_reason_descrip[s_reason]);
		send_msg(msg);
		break;
	default:
		send_msg("Invalid input\r\n");
	}
}