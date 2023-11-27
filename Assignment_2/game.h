/*
 * game.h
 *
 * Author: Jarrod Bennett, Cody Burnett
 *
 * Function prototypes for game functions available externally. You may wish
 * to add extra function prototypes here to make other functions available to
 * other files.
 */


#ifndef GAME_H_
#define GAME_H_

#include <stdint.h>

#define TRACK_LENGTH 129

// Initialise the game by resetting the grid and beat
void initialise_game(void);

// Play a note in the given lane
void play_note(uint8_t lane);

// Advance the notes one row down the display
void advance_note(void);

// Returns 1 if the game is over, 0 otherwise.
uint8_t is_game_over(void);

//Finds the next valid note
uint8_t find_next_note(uint8_t indx);
void update_terminal_score(int score);


uint8_t game_over_flag;
uint8_t note_played[TRACK_LENGTH];
int score; 

uint8_t combo;

float duty_cycle;

uint16_t pulsewidth;

uint8_t current_track;

void update_combo(void);




#endif
