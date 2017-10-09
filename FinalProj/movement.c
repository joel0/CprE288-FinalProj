#include <avr/io.h>
#include "lib/open_interface.h"
#include "movement.h"
#include "lib/lcd.h"
#include "bluetooth.h"
#include <stdlib.h>
#include <stdio.h>

const char* stop_reason_descrip[] = {"LeftBump", "RightBump", "CliffLeft", "CliffRight", "Color", "None"};

///Rotates the given number of degrees
/**
 * Takes in an int specifying an angle in degrees and the robot will turn that many degrees.
 * Positive angles are counter clockwise; negative angles are clockwise.
 * @param deg angle in degrees for the roboto to rotate
 * @param sensor_data the oi_t struct containing all the robots data
 */
int rotate_deg(int deg, oi_t* sensor_data)
{
	int degree = 0;
    if(deg < 0)
    {
        //For turning cw
		oi_set_wheels(-200, 200);
        while (degree > deg)
        {
            oi_update(sensor_data);
            degree += sensor_data->angle;

        }
    }
    else{
        //For turning ccw
		oi_set_wheels(200, -200);
        while (degree < deg)
        {
            oi_update(sensor_data);
            degree += sensor_data->angle;

        }
    }
    oi_set_wheels(0, 0);
	return deg;
}

/**
 * Moves the robot forward or backward the given number of units and tracks the number of units that
 * the robot has moved. The units may need to be in time.  200 speed will probably not knock over any
 * obstacles, so it will be the default.  It will also optionally scan for color and the cliff/bump sensor can
 * be turned off.
 * @param units distance in mm for the roboto to move
 * @param sensor_data the oi_t struct containing all the robots data
 * @param ignore_cliffbump 1 to ignore, 0 else
 * @param ignore_color 1 to ignore, 0 else
 */
int move_result(int units, oi_t* sensor_data, char ignore_cliffbump, char ignore_color, stop_reason* reason)
{
	stop_reason dummy;
	if (reason == NULL) {
		reason = &dummy;
	}
    int16_t sum = 0;
	int temp_result = 0;
    if(units < 0)
    {
		units = -units;
        oi_set_wheels(-200, -200);
    }else{
        oi_set_wheels(200, 200);
    }
	
    while (abs(sum) < units) {
		oi_update(sensor_data);
		if(!ignore_cliffbump) {
			temp_result = read_cliffs(sensor_data, reason);
			if(!temp_result) {
				temp_result += read_bumps(sensor_data, reason);
			}
			if(temp_result) {
				return sum + temp_result;
			}
		}
        if(!ignore_color)
        {
			temp_result += read_cliff_signals(sensor_data);
			if(temp_result) {
				*reason = COLOR;
				return sum + temp_result;
			}
        }
        sum += sensor_data->distance;
    }
    oi_set_wheels(0, 0);
	*reason = NONE;
	return sum;
}

///Reads the bumper sensors and backs up the robot.
/**
 * If something has been bumped into, the robot will stop and back up a little. When the robot is backing up, the robot
 * will not look for color, bumps or cliffs.
 * @param sensor_data the oi_t struct containing all the robots data
 */
int read_bumps(oi_t* sensor_data, stop_reason* reason)
{
    if (sensor_data->bumper_left) {
		*reason = BUMP_L;
        return move_result(-100,sensor_data,1,1,NULL); //should move ten centimeters.
    }
    if (sensor_data->bumper_right) {
		*reason = BUMP_R;
        return move_result(-100,sensor_data,1,1,NULL); //should move ten centimeters.
    }
    return 0;
}

///Reads the register values for the cliff sensors and backs up the robot if cliff
/**
 * When a cliff is sensed to the front right/left or the right/left, the robot will stop and back up a little. When the
 * robot is backing up, the robot will not look for color, bumps or cliffs.
 * @param sensor_data the oi_t struct containing all the robots data
 */
int read_cliffs(oi_t* sensor_data, stop_reason* reason)
{
    if (sensor_data->cliff_left) {
		*reason = CLIFF_L;
        return move_result(-100,sensor_data,1,1,NULL); //should move ten centimeters.
    }
    if (sensor_data->cliff_frontleft) {
		*reason = CLIFF_L;
        return move_result(-100,sensor_data,1,1,NULL); //should move ten centimeters.
    }
    if (sensor_data->cliff_right) {
		*reason = CLIFF_R;
        return move_result(-100,sensor_data,1,1,NULL); //should move ten centimeters.
    }
    if (sensor_data->cliff_frontright) {
		*reason = CLIFF_R;
        return move_result(-100,sensor_data,1,1,NULL); //should move ten centimeters.
    }
    return 0;
}

///Reads the ground color.  If abnormal, backs up
/**
 * When a color is sensed in the cliff signals, and the value is greater than 500, the robot stops and backs up,
 * ignoring color and cliffs and bumps.
 * @param sensor_data the oi_t struct containing all the robots data
 */
int read_cliff_signals( oi_t* sensor_data)
{
	int result = 0;
	static int i=0;
	static char initialized = 0;
	static uint16_t average_left_signal=0;
	static uint16_t average_right_signal=0;
	static uint16_t average_fleft_signal=0;
	static uint16_t average_fright_signal=0;
	static uint16_t last_left_signal[5];
	static uint16_t last_fleft_signal[5];
	static uint16_t last_fright_signal[5];
	static uint16_t last_right_signal[5];
	lprintf("%d, %d, %d, %d",average_left_signal,average_fleft_signal,average_right_signal,average_fright_signal);

	if (initialized) {
		if (sensor_data->cliff_left_signal > average_left_signal * 2) {
			result = move_result(-100,sensor_data,1,1,NULL); //should move ten centimeters.
			} else if (sensor_data->cliff_frontleft_signal > average_fleft_signal * 2) {
			result = move_result(-100,sensor_data,1,1,NULL); //should move ten centimeters.
			} else if (sensor_data->cliff_right_signal > average_right_signal * 2) {
			result = move_result(-100,sensor_data,1,1,NULL); //should move ten centimeters.
			} else if (sensor_data->cliff_frontright_signal > average_fright_signal * 2) {
			result = move_result(-100,sensor_data,1,1,NULL); //should move ten centimeters.
		}
	}
	else
	{
		for(int k=0;k<5;k++)
		{
			last_left_signal[k]=sensor_data->cliff_left_signal;
			last_fleft_signal[k]=sensor_data->cliff_frontleft_signal;
			last_right_signal[k]=sensor_data->cliff_right_signal;
			last_fright_signal[k]=sensor_data->cliff_frontright_signal;
			
		}
		
	}
	last_left_signal[i] = sensor_data->cliff_left_signal;
	last_fleft_signal[i] = sensor_data->cliff_frontleft_signal;
	last_fright_signal[i] = sensor_data->cliff_frontright_signal;
	last_right_signal[i] = sensor_data->cliff_right_signal;
	average_left_signal = 0;
	average_fleft_signal = 0;
	average_right_signal = 0;
	average_fright_signal = 0;
	for(int j=0;j<5;j++)
	{
		average_left_signal+=last_left_signal[j];
		average_fleft_signal+=last_fleft_signal[j];
		average_right_signal+=last_right_signal[j];
		average_fright_signal+=last_fright_signal[j];
	}
	average_left_signal=average_left_signal/5;
	average_fleft_signal=average_fleft_signal/5;
	average_right_signal=average_right_signal/5;
	average_fright_signal=average_fright_signal/5;
	i=(i+1)%5;
	initialized = 1;
	return result;
}
