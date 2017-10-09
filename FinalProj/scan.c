#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "lib/util.h"
#include "scan.h"

static obj_t scanner[15];
void set_servo_OCR(int ticks);
int calc_servo_OCR_ticks(float deg);
int ir_distance_cm(void);
int ir_ADC_to_cm(int reading);
int side_angle_side(int angle, int side_len);

/// Scans the 180 degrees to find objects
/**
 * Scans from 0 to 180 degrees looking for objects.  It returns an array of objects along with a count.
 * @param obj_count the number of objects in the array
 */
obj_t* do_scan(int* obj_count)
{
	int ir_dist = 0, start_angle = 0, end_angle = 0;
	char is_measuring = 0;
	*obj_count = 0;
	
	set_servo_pos(0);
	wait_ms(1000);
	
	for(int angle = 0; angle < 181 && *obj_count <= sizeof(scanner); angle += 1)
	{
		ir_dist = dist_at_angle(angle);
				
		if(is_measuring == 0)
		{
			if(ir_dist < 60)
			{
				start_angle = angle;
				is_measuring = 1;
				scanner[*obj_count].dist = ir_dist;
			}
		}
		else
		{
			if(ir_dist > 60)
			{
				end_angle = angle;
				is_measuring = 0;
				scanner[*obj_count].angular_width = end_angle - start_angle;
				if (scanner[*obj_count].angular_width > 1) {
					scanner[*obj_count].width = side_angle_side(scanner[*obj_count].angular_width, scanner[*obj_count].dist);
					scanner[*obj_count].angular_location = (start_angle + end_angle) / 2;
					(*obj_count)++;
				}
			}
			else
			{
				scanner[*obj_count].dist = (scanner[*obj_count].dist + ir_dist) / 2;
			}
		}
	}
	return scanner;
}

/// Side-angle-side calculation on a symmetric triangle to find the far side
/**
 * Applies the side-angle-side formula to calculate the far side.  The two sides are assumed to have the same length.
 * @param angle the angle between the two given sides
 * @param side_len the length of both known sides
 * @return the length of the unknown side
 */
int side_angle_side(int angle, int side_len) {
	int a = side_len * side_len * 2 - 2 * side_len * side_len * cos(angle * M_PI / 180);
	return sqrt(a);
}

/// Side-angle-side calculation on a triangle of arbitrary shape
/**
 * Applies the side-angle-side formula to calculate the far side.
 * @param angle the angle between the two sides
 * @param side1_len the length of the side to the left of the angle
 * @param side2_len the length of the side to the right of the angle
 * @return the length of the unknown side
 */
int side_angle_side2(int angle, int side1_len, int side2_len) {
	int a = side1_len * side1_len + side2_len * side2_len - (2 * side1_len * side2_len) * cos(angle * M_PI / 180);
	return sqrt(a);
}

/// Applies the side-side-side calculation to determine an angle of a symmetric triangle
/**
 * Applies the side-side-side calculation to find the angle between two sides of the same length
 * @param far_side the length of the far side of the triangle
 * @param adjascent_sides the length of the two adjacent sides of the triangle
 * @return the angle between the two adjacent sides
 */
int side_side_side(int far_side, int adjascent_sides) {
	float a = acos((2 * (adjascent_sides * adjascent_sides) - far_side * far_side) / (float) (2 * adjascent_sides * adjascent_sides)) * 180.0 / M_PI;
	return a;
}

/// Rotates the servo and measures the distance at that angle
/**
 * Rotates the servo to the specified angle, waits a little bit for the hardware to move, then reads
 * the value from the ADC and converts it to cm.
 * @param angle the angle in degrees to rotate the servo to
 * @return the distance in cm of an object
 */
int dist_at_angle(int angle)
{
	set_servo_pos(angle);
	return ir_distance_cm();
}

/// Rotates the servo to the specified angle in degrees
/**
 * Sets the servo to the specified angle in degrees.  It waits 100 ms after setting the output.
 * @param deg the angle in degrees to rotate to
 */
void set_servo_pos(int deg) {
	set_servo_OCR(calc_servo_OCR_ticks(deg));
}

/// Sets timer 3's OCR to the specified number of ticks for a PWM wave
/**
 * Sets the OCR of timer 3 to the specified number of ticks to create a PWM wave for rotating the
 * servo to the correct angle.
 * @param ticks the number of ticks that the signal is high
 */
void set_servo_OCR(int ticks)
{
	OCR3B = ticks;
	wait_ms(50);
}

/// Calculates the number of ticks required to rotate the servo to the desired angle
/**
 * Calculates the number of ticks required to rotate the servo to the desired angle.  The PWM
 * wave has a certain duty cycle to rotate the servo.  It has been calibrated to the servo, so the
 * values do not have much meaning outside of being calibrated.
 * @param deg the angle in degrees to rotate the servo to
 */
int calc_servo_OCR_ticks(float deg)
{
	float percent = (float) deg / 180;
	return 800 + 3400 * percent;
}

/// Reads a value from the ADC and converts it to cm
/**
 * Reads a value from the ADC and uses the appropriate conversion to convert the value to cm.
 * @return the measured distance in cm
 */
int ir_distance_cm(void)
{
	int val = ADC_read();
	return ir_ADC_to_cm(val);
}

/// Gets a reading from the ADC
/**
 * Starts an ADC conversion, waits for it to complete, then returns the raw value.
 * @return the raw value from the ADC
 */
int ADC_read()
{
	// Start the ADC conversion
	ADCSRA |= _BV(ADSC);
	// Wait for the result
	while (!(ADCSRA & _BV(ADIF)))
		{}
	return ADC;
}

/// Converts a raw ADC reading to cm
/**
 * Using a magical calibrated conversion, converts a raw reading to cm.
 * @param reading the raw reading from the ADC
 * @return the conversion in cm
 */
int ir_ADC_to_cm(int reading)
{
	// Magical formula created by taking samples and creating an equation in Mathematica. ;)
	return 133.987 -
			0.69158 * reading +
			0.00176938 * pow(reading, 2) -
			2.25827 * pow(10, -6) * pow(reading, 3) +
			1.14087 * pow(10, -9) * pow(reading, 4) +
			1.59493 * pow(10, -13) * pow(reading, 5) -
			2.46348 * pow(10, -16) * pow(reading, 6);
}