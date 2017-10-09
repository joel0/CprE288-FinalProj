#ifndef IO_H_
#define IO_H_

#include "lib/open_interface.h"

void init_UART(void);
oi_t* init_iRobot(void);
void init_servo(void);
void init_ir(void);

#endif /* IO_H_ */