/*
 * @file sound.c
 * @brief this file contains all the notes for the songs that are to played by the rover when it reaches it's base.
 * @author: Vaibhav Malhotra, Erika Clarke, Joel May, Fahmida Jyoti 
 * @date 4/13/2016 
 *  
 */ 
#include "lib/open_interface.h"
#include "sound.h"

static uint8_t StarwarsNumNotes = 19;
static uint8_t StarwarsNotes[19] = {55, 55, 55, 51, 58, 55, 51, 58, 55, 0,  62, 62, 62, 63, 58, 54, 51, 58, 55};
static uint8_t StarwarsDurations[19] = {32, 32, 32, 20, 12, 32, 20, 12, 32, 32, 32, 32, 32, 20, 12, 32, 20, 12, 32};

static uint8_t marioNumNotes = 49;
static uint8_t marioNotes[49] =
	{48, 60, 45, 57, 46, 58,  0, 48, 60, 45, 57, 46, 58,  0, 41, 53, 38, 50,
	 39, 51,  0, 41, 53, 38, 50, 39, 51,  0, 51, 50, 49, 48, 51, 50, 44, 43,
	 49, 48, 54, 53, 52, 58, 57, 56, 51, 47, 46, 45, 44 };
static uint8_t marioDuration[49] =
	{12, 12, 12, 12, 12, 12, 62, 12, 12, 12, 12, 12, 12, 62, 12, 12, 12, 12,
	 12, 12, 62, 12, 12, 12, 12, 12, 12, 48,  8,  8,  8, 24, 24, 24, 24, 24,
	 24,  8,  8,  8,  8,  8,  8, 16, 16, 16, 16, 16, 16 };
	 
static uint8_t rickrollNumNotes = 11;
static uint8_t rickrollNotes[11] = {53, 55, 48, 55, 57, 60, 58, 57, 53, 55, 48};
static uint8_t rickrollDurations[11] = {48, 64, 16, 48, 48, 8,  8,  8,  48, 64, 64};


/*
* This method loads the song onto the rover and then plays it according to the song id assigned to it.
* @param id The song id to load the song associated with it.
*/
void songs(uint8_t id)
{
		// This will load the song associated with the id.
		switch (id) 
		{
		case MARIO: //This will play the Mario song.
			oi_load_song(MARIO, marioNumNotes, marioNotes, marioDuration);
			oi_play_song(MARIO);
			break;
		case DARTHVADER: //This will play the Star wars song.
			oi_load_song(DARTHVADER, StarwarsNumNotes,StarwarsNotes, StarwarsDurations);
			oi_play_song(DARTHVADER);
			break;
		case RICKROLLED: //This will play the Rick Roll song.
			oi_load_song(RICKROLLED, rickrollNumNotes, rickrollNotes, rickrollDurations);
			oi_play_song(RICKROLLED);
			break;
		
		default: //This will be executed if no proper id is provided.
				break;
		}
}