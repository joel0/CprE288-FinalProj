#ifndef HI_H_
#define HI_H_

#include "lib/open_interface.h"

extern char in_program_ui;

typedef enum 
{
	INIT,
	STANDBY,
	ZONE_IDENTIFY,
	MOVEMENT,
	MOVEMENT_NO_SENSOR,
	SCAN,
	AUTO,
	PROGRAM_UI
} menu_option;

menu_option main_menu(void);
menu_option mymenu_option;
void show_objects(void);
void show_sensors(oi_t* sensor_data);
void move_menu(oi_t* sensor_data, char ignore_sensors);

#endif /* HI_H_ */