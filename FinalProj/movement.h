#include <avr/io.h>
#include "lib/open_interface.h"

#ifndef MOVEMENT_H_
#define MOVEMENT_H_


/**
 * Enum describing why the robot stopped moving.
 */
typedef enum {BUMP_L = 0, BUMP_R = 1, CLIFF_L = 2, CLIFF_R = 3, COLOR = 4, NONE = 5} stop_reason;
extern const char* stop_reason_descrip[];

int rotate_deg(int deg, oi_t* sensor_data);
int move_result(int units, oi_t* sensor_data, char ignore_cliffbump, char ignore_color, stop_reason* reason);
int read_bumps(oi_t* sensor_data, stop_reason* reason);
int read_cliffs(oi_t* sensor_data, stop_reason* reason);
int read_cliff_signals( oi_t* sensor_data);


#endif /* MOVEMENT_H_ */