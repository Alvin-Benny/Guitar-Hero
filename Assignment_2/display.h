/*
 * display.h
 *
 * Authors: Luke Kamols, Jarrod Bennett, Martin Ploschner, Cody Burnett,
 * Renee Nightingale
 */ 

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "pixel_colour.h"

// Initialise the display for the board, this creates the display
// for an empty board.
void default_grid(void);

// Shows a starting display.
void show_start_screen(void);

// Update dynamic start screen based on frame number (0-11)
void update_start_screen(uint8_t frame_number);

// Updates the colour at square (x, y) to be the colour
// of the object 'object'.
void update_square_colour(uint8_t x, uint8_t y, uint8_t object);

#endif /* DISPLAY_H_ */
