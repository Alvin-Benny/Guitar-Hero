/*
 * game.c
 *
 * Functionality related to the game state and features.
 *
 * Author: Jarrod Bennett, Cody Burnett
 */ 

#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "display.h"
#include "ledmatrix.h"
#include "terminalio.h"

static const uint8_t *track; //pointer to the array




static const uint8_t track0[TRACK_LENGTH] = {0x00,
	0x00, 0x00, 0x08, 0x08, 0x08, 0x80, 0x04, 0x02,
	0x04, 0x40, 0x08, 0x80, 0x00, 0x00, 0x04, 0x02,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x02, 0x20, 0x01,
	0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x80, 0x04, 0x40, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x40, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x02, 0x20, 0x01,
	0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x08, 0x08, 0x08, 0x80, 0x04, 0x02,
	0x04, 0x40, 0x02, 0x08, 0x80, 0x00, 0x02, 0x01,
	0x04, 0x40, 0x08, 0x80, 0x04, 0x02, 0x20, 0x01,
	0x10, 0x10, 0x12, 0x20, 0x00, 0x00, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x40, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x40, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x40, 0x02, 0x20,
	0x01, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00};
	
static const uint8_t track1[TRACK_LENGTH] = {
	0x00,0x00, 0x00,
	0x2,0x8,0x20,0x0,0x2,0x4,0x4,0x10,0x20,
	0x4,0x10,0x40,0x40,0x40,0x80,0x0,0x20,
	0x0,0x1,0x2,0x4,0x4,0x20,0x20,0x2,0x0,
	0x40,0x20,0x40,0x8,0x10,0x4,0x4,0x10,0x8
	,0x8,0x40,0x20,0x40,0x4,0x2,0x8,0x10,0x20,
	0x4,0x4,0x4,0x80,0x8,0x8,0x8,0x80,0x40,0x40,
	0x20,0x8,0x40,0x40,0x2,0x8,0x40,0x2,0x80,0x8,
	0x40,0x4,0x4,0x2,0x20,0x10,0x40,0x4,0x0,0x8,0x8,
	0x2,0x10,0x8,0x4,0x2,0x2,0x20,0x0,0x0,0x4,0x2,0x4,
	0x10,0x2,0x1,0x40,0x1,0x10,0x12,0x80,0x4,0x4,0x4,
	0x8,0x2,0x80,0x10,0x8,0x4,0x40,0x40,0x8,0x4,0x1,
	0x1,0x10,0x40,0x40,0x40,0x10,0x2,0x40,0x0,
	0x0,0x10,0x2,0x2,0x00, 0x00, 0x00, 0x00};
	
static const uint8_t track2[TRACK_LENGTH] = {0x00,
	0x00, 0x00, 0x8,0x1,0x40,0x20,0x8,0x4,0x4,0x10,
	0x8,0x2,0x20,0x8,0x8,0x8,0x40,0x2,0x40,0x10,0x4,
	0x40,0x4,0x2,0x10,0x10,0x8,0x8,0x40,0x20,0x10,0x4,
	0x0,0x2,0x10,0x1,0x0,0x1,0x20,0x2,0x4,0x80,0x40,
	0x80,0x20,0x4,0x10,0x4,0x2,0x8,0x8,0x4,0x80,0x8,0x10,
	0x40,0x4,0x1,0x0,0x0,0x40,0x4,0x80,0x2,0x20,0x40,0x10,
	0x0,0x2,0x2,0x4,0x10,0x8,0x2,0x4,0x2,0x20,0x0,0x0,0x4,
	0x4,0x8,0x10,0x8,0x40,0x8,0x4,0x1,0x2,0x40,0x20,0x2,0x40,
	0x2,0x2,0x2,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x20,
	0x4,0x8,0x20,0x4,0x8,0x20,0x40,0x2,0x10,0x4,0x0,0x4,
	0x80,0x0,0x12,0x4,0x80,0x10,0x4, 0x00, 0x00, 0x00, 0x00};


uint16_t beat;

uint16_t beat_when_sounded;


void update_combo(void){
	move_terminal_cursor(10, 12);
	clear_to_end_of_line();
	printf("Combo: %d", combo);
	
	if (combo == 3){
		PORTD |= (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7);
		move_terminal_cursor(10, 15);
		printf("  ______    ______   __       __  _______    ______  ");
		move_terminal_cursor(10, 16);
		printf(" /      \\  /      \\ |  \\     /  \\|       \\  /      \\");
		move_terminal_cursor(10, 17);
		printf("|  $$$$$$\\|  $$$$$$\\| $$\\   /  $$| $$$$$$$\\|  $$$$$$");
		move_terminal_cursor(10, 18);
		printf("| $$   \\$$| $$  | $$| $$$\\ /  $$$| $$__/ $$| $$  | $$");
		move_terminal_cursor(10, 19);
		printf("| $$      | $$  | $$| $$$$\\  $$$$| $$    $$| $$  | $$");
		move_terminal_cursor(10, 19);
		printf("| $$   __ | $$  | $$| $$\\$$ $$ $$| $$$$$$$\\| $$  | $$");
		move_terminal_cursor(10, 20);
		printf("| $$__/  \\| $$__/ $$| $$ \\$$$| $$| $$__/ $$| $$__/ $$");
		move_terminal_cursor(10, 21);
		printf("\\$$    $$ \\$$    $$| $$  \\$ | $$| $$    $$ \\$$    $$");
		move_terminal_cursor(10, 22);
		printf(" \\$$$$$$   \\$$$$$$  \\$$      \\$$ \\$$$$$$$   \\$$$$$$ ");
		
	}
	else if (combo == 2) {
		PORTD |= (1 << PORTD5) | (1 << PORTD6);
	}
	else if (combo == 1) {
		PORTD |= (1 << PORTD5);
	}
	else if (combo < 1){
		move_terminal_cursor(10, 15);
		clear_to_end_of_line();
		move_terminal_cursor(10, 16);
		clear_to_end_of_line();
		move_terminal_cursor(10, 17);
		clear_to_end_of_line();
		move_terminal_cursor(10, 18);
		clear_to_end_of_line();
		move_terminal_cursor(10, 19);
		clear_to_end_of_line();
		move_terminal_cursor(10, 20);
		clear_to_end_of_line();
		move_terminal_cursor(10, 21);
		clear_to_end_of_line();
		move_terminal_cursor(10, 22);
		clear_to_end_of_line();
		PORTD &= ~(1 << PORTD5);
		PORTD &= ~(1 << PORTD6);
		PORTD &= ~(1 << PORTD7);

	}
}

void update_terminal_score(int score) {
	move_terminal_cursor(10,4);
	clear_to_end_of_line();
	
	uint8_t num_of_digits = 1;
	if (abs(score) >= 10) {
		num_of_digits = 2;
	}
	if (abs(score) >= 100) {
		num_of_digits = 3;
	}
	
	if (score < 0) {
		num_of_digits = num_of_digits + 1; //cater for neg sign
	}
	
	//'5' based on examples provided
	uint8_t num_of_spaces= 5 - num_of_digits;
	
	printf("Game Score");
	for (uint8_t x=0; x < num_of_spaces; x++){
		printf(" "); //empty space
	}
	printf("%d", score);
}

uint8_t find_next_note(uint8_t indx)
{
	for (uint8_t f_note = indx + 1; f_note < TRACK_LENGTH; f_note++)
	{
		if (track[f_note] & 0x0F) //Check if next note exists
		{
			return f_note;
		}
	}
	return 0;
}


//For audio
uint16_t b0_note =  523.2511;
uint16_t b1_note =   622.254;
uint16_t b2_note =   698.4565;
uint16_t b3_note =   783.9909;

uint16_t freq;

uint16_t freq_to_clock_period(uint16_t freq) {
	return (1000000UL / freq);
}

uint16_t duty_cycle_to_pulse_width(float dutycycle, uint16_t clockperiod) {
	return (dutycycle * clockperiod) / 100;
}



// Initialise the game by resetting the grid and beat
void initialise_game(void)
{
	//display_digit(score, digit);
	// initialise the display we are using.
	default_grid();
	beat = 0;
	combo = 0; //reset the combo
	beat_when_sounded = 0;
	//An array that keeps track of all the notes that have been played
	// default value 0
	for(uint8_t i=0; i < TRACK_LENGTH; i++){
		note_played[i] = 0;
		
	if (current_track == 0){
		track = track0; 
	}
	else if (current_track == 1){
		track = track1;
	}
	else if (current_track == 2){
		track = track2;
	}
	}
	
}


// Play a note in the given lane
void play_note(uint8_t lane)
{
	//scoring area starts from 11
	for (uint8_t col=11; col<MATRIX_NUM_COLUMNS; col++)
	{
		uint8_t future = MATRIX_NUM_COLUMNS-1-col;
		// notes are only drawn every five columns
		if ((future+beat)%5)
		{
			continue;
		}
		// index of which note in the track to play
		uint8_t index = (future+beat)/5;
		// if the index is beyond the end of the track,
		// no note can be played
		if (index >= TRACK_LENGTH)
		{
			continue;
		}
		
		//led lane is left to right
		
		// check if there's a note in the specific path
		if (track[index] & (1<<lane))
		{
			
			// if so, colour the two pixels green
			ledmatrix_update_pixel(col, 2*lane, COLOUR_GREEN);
			ledmatrix_update_pixel(col, 2*lane+1, COLOUR_GREEN);
			
			//Ensure the player has not already played the note
			if (note_played[index] == 1) {
				score = score - 1;
				combo = 0;
				update_combo();
				OCR1B = 0;
				beat_when_sounded = 0;
				break;
			}
			note_played[index] = 1;
			//scoring area when future is 0-4
			if (future == 4){
				score = score + 1;
				duty_cycle = 2;
				if (combo){
					combo= 0;
					update_combo();
				}
			}
			else if (future == 3){
				score = score + 2;
				duty_cycle = 10;
				if (combo){
					combo= 0;
					update_combo();
				} 
			}
			else if (future == 2) {
				score = score + 3;
				combo = combo + 1;
				update_combo();
				duty_cycle = 50;
				if (combo >= 3) {
					score = score + 1;
					
				}
			}
			else if (future == 1) {
				score = score + 2;
				duty_cycle = 90;
				if (combo){
					combo= 0;
					update_combo();
				}
			}
			else if (future == 0){
				score = score + 1;
				duty_cycle = 98;
				if (combo){
					combo= 0;
					update_combo();
				}
			}
			
			if (lane==3) {
				freq = b3_note;
			}
			else if (lane == 2) {
				freq = b2_note;
			}
			else if (lane == 1) {
				freq = b1_note;
			}
			else if (lane == 0){
				freq = b0_note;
			}
			uint16_t clockperiod = freq_to_clock_period(freq);
			uint16_t pulsewidth = duty_cycle_to_pulse_width(duty_cycle, clockperiod);
		
			
			if(pulsewidth > 0) {
				OCR1B = pulsewidth - 1;
				beat_when_sounded = beat + 5;
				} 
				else {
					OCR1B = 0;
			}	
			OCR1A = clockperiod - 1;
		}
		else {	
			score = score - 1;
			combo = 0;
			OCR1B = 0; //disable timer
			beat_when_sounded = 0;
			update_combo();
			}
		}
	//display_digit(score, digit); //for SSD display
	update_terminal_score(score);
	}


// Advance the notes one row down the display
void advance_note(void)
{

	// remove all the current notes; reverse of below
	for (uint8_t col=0; col<MATRIX_NUM_COLUMNS; col++)
	{
		uint8_t future = MATRIX_NUM_COLUMNS - 1 - col;
		uint8_t index = (future + beat) / 5;
		if (index >= TRACK_LENGTH)
		{
			game_over_flag = 1;
			continue;
		}
		if ((future+beat) % 5)
		{
			continue;
		}
		
		for (uint8_t lane = 0; lane < 4; lane++)
		{	
			if (track[index] & (1<<lane))
			{
				PixelColour colour;
				// yellows in the scoring area
				if (col==11 || col == 15)
				{
					colour = COLOUR_QUART_YELLOW;
				}
				else if (col==12 || col == 14)
				{
					colour = COLOUR_HALF_YELLOW;
				}
				else if (col==13)
				{
					colour = COLOUR_YELLOW;
				}
				else
				{
					colour = COLOUR_BLACK;
				}
				ledmatrix_update_pixel(col, 2*lane, colour);
				ledmatrix_update_pixel(col, 2*lane+1, colour);
				
				//if the note slides of the screen
				if ((future == 0) & (note_played[index] == 0)) {
					score = score -1;
					combo = 0;
					update_terminal_score(score);
					update_combo();
					
				}
			}
		}
	}
	
			
			
	// increment the beat
	beat++;
	
	//turn off the sound after 5 beats
	if ((beat_when_sounded == beat) && OCR1B){
		OCR1B = 0;
	}
	//GHOST NOTES
	ledmatrix_update_column(-1, COLOUR_BLACK);
	uint8_t index = (MATRIX_NUM_COLUMNS+beat)/5;
	
	//Ensure index is less than track length
	// if there isn't a next note
	// set the game_over flag to true and break out of the loop

	
	//ensure the track has not ended
	uint8_t n_note  = find_next_note(index);

	for (uint8_t lane=0; lane<4; lane++)
	{
		
		if (n_note) //there is a next note
		{
			// check if there's a note in the specific path
			if (track[n_note] & (1<<lane))
			{
				uint8_t colour;
				if (combo >= 3){
					colour = COLOUR_COMBO_GHOST;
				}
				else{
					colour = COLOUR_GHOST;
				}
				
				
				ledmatrix_update_pixel(0, 2*lane, colour);
				ledmatrix_update_pixel(0, 2*lane+1, colour);
						
			}
		}
	}


	
	
	// draw the new notes
	for (uint8_t col=0; col<MATRIX_NUM_COLUMNS; col++)
	{
		// col counts from one end, future from the other
		uint8_t future = MATRIX_NUM_COLUMNS-1-col;


		// notes are only drawn every five columns
		if ((future+beat)%5)
		{
			continue;
			
		}
		
		
		// index of which note in the track to play
		uint8_t index = (future+beat)/5;
		// if the index is beyond the end of the track,
		// no note can be drawn

		if (index >= TRACK_LENGTH)
		{
			game_over_flag = 1;
			continue;
		
		}
		
		// iterate over the four paths
		for (uint8_t lane=0; lane<4; lane++)
		{
			
			// check if there's a note in the specific path
			if (track[index] & (1<<lane))
			{	
				uint8_t colour;
				if (note_played[index] == 1){
					colour = COLOUR_GREEN;
				}
				
				else if (combo >=3) {
					colour = COLOUR_ORANGE;
				}
				
				else {
					colour = COLOUR_RED;
				}
				
				ledmatrix_update_pixel(col, 2*lane, colour);
				ledmatrix_update_pixel(col, 2*lane+1, colour);
				
				}
			}


			
		}


	}


// Returns 1 if the game is over, 0 otherwise.
uint8_t is_game_over()
{
	// YOUR CODE HERE
	// Detect if the game is over i.e. if a player has won.
	if (game_over_flag == 1) {
		OCR1B = 0; // turn off buzzer
		//remove the combo (if it exists)
		move_terminal_cursor(10, 15);
		clear_to_end_of_line();
		move_terminal_cursor(10, 16);
		clear_to_end_of_line();
		move_terminal_cursor(10, 17);
		clear_to_end_of_line();
		move_terminal_cursor(10, 18);
		clear_to_end_of_line();
		move_terminal_cursor(10, 19);
		clear_to_end_of_line();
		move_terminal_cursor(10, 20);
		clear_to_end_of_line();
		move_terminal_cursor(10, 21);
		clear_to_end_of_line();
		move_terminal_cursor(10, 22);
		clear_to_end_of_line();
		ledmatrix_clear();
		return 1;
	}
	else{
		return 0;
	}
}

