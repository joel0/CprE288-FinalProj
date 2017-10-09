#ifndef SCAN_H_
#define SCAN_H_


typedef struct
{
	int width;
	int dist;
	int angular_location;
	int angular_width;
} obj_t;

obj_t* do_scan(int* obj_count);
void set_servo_pos(int deg);
int dist_at_angle(int angle);
int ADC_read(void);
int side_side_side(int far_side, int adjascent_sides);
int side_angle_side2(int angle, int side1_len, int side2_len);

#endif /* SCAN_H_ */