/*
 * @file sound.h
 * @brief this header file contains all the basic definition and the
 * required function for loading the song on the rover.
 * @author: Vaibhav Malhotra, Erika Clarke, Joel May, Fahmida Jyoti 
 * @date 4/13/2016
 */ 

#ifndef _SONGS_H
#define _SONGS_H

#include <stdint.h>

//Defined the variable mario with the int id 1
#define MARIO	1
//Defined the variable star wars with the int id 2
#define DARTHVADER     2
//Defined the variable star wars with the int id 3
#define RICKROLLED 3
/*
* This method loads the song onto the rover and then plays it according to the song id assigned to it.
* @author Vaibhav Malhotra
* @param id The song id to load the song associated with it.
* @date 4/17/2016
*/
void songs(uint8_t);

#endif /* _SONGS_H */