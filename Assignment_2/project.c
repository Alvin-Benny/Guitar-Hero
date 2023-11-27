/*
 * project.c
 *
 * Main file
 *
 * Authors: Peter Sutton, Luke Kamols, Jarrod Bennett, Cody Burnett
 * Modified by <Alvin Benny>
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define F_CPU 8000000UL
#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>

#include "game.h"
#include "display.h"
#include "ledmatrix.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "timer0.h"
#include "timer1.h"
#include "timer2.h"



// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void start_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);
void game_count_down(void);

uint16_t game_speed;
uint8_t manual_mode = 0;
uint8_t game_paused = 0;
uint8_t countdown_finished;


uint8_t seven_seg[10] = { 63,6,91,79,102,109,125,7,127,111};
uint8_t digit;
uint16_t game_speed_normal = 1000;
uint16_t game_speed_fast = 500;
uint16_t game_speed_extreme = 250;


//For audio
uint8_t notes_pressed;

//For combo ASCII



void display_game_speed() {
	move_terminal_cursor(10,8);
	clear_to_end_of_line();
	if (game_speed == game_speed_normal){
		printf("Game Speed: Normal");
	}
	else if (game_speed == game_speed_fast) {
		printf("Game Speed: Fast");
	}
	else if (game_speed == game_speed_extreme){
		printf("Game Speed: Extreme");
	}
	update_terminal_score(score);
}

void update_ssd(void){
	//LOGIC FOR SSD
	digit = 1 ^ digit;
	//trying to get
	if(digit == 0) {
		if (score > 9) {
			PORTC = seven_seg[ (score) % 10];
		}
		else if (score >=-9 && score < 0) {
			PORTC = seven_seg[abs(score)];
		}
		else if (score < -9) {
			PORTC =  0b01000000;
		}
		else{
			PORTC = seven_seg[score];
		}
	}
	//If the digit is 1
	else {
		if (score > 9) {
			PORTC = seven_seg[ (score / 10) % 10];
		}
		else if (score < 0) {
			PORTC = 0b01000000; //Display negative sign
		}
		else {
			PORTC = 0;
		}
	}
	
	/* Output the digit selection (CC) bit */
	
	//Need to preserve other bits as things like L7 use PORTD
	
	if (digit == 1) {
		PORTD |= (digit << PORTD2);
	}
	else{
		PORTD &= ~(1 << PORTD2);
	}
	_delay_ms(5); //To prevent ghosting
}

void print_track(void){
	if (current_track == 2){
		printf("Selected Track: A Battle For Time");
	}
	else if (current_track == 1) {
		printf("Selected Track: How Far I Could Go");
	}
	else if (current_track == 0){
		printf("Selected Track: Through the Fire and Flames");
	}
}
/////////////////////////////// main //////////////////////////////////
int main(void)
{
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete.
	start_screen();
	
	// Loop forever and continuously play the game.
	while(1)
	{
		new_game();
		play_game();
		handle_game_over();
		
		
	}
}

void initialise_hardware(void)
{
	ledmatrix_setup();
	init_button_interrupts();
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200, 0);
	
	init_timer0();
	init_timer1();
	init_timer2();
	
	// Turn on global interrupts
	sei();
}

void start_screen(void)
{
	PORTC = 0; //Turn off SSD
	//Reset manual mode
	manual_mode = 0;
	// Clear terminal screen and output a message
	clear_terminal();
	show_cursor();
	clear_terminal();
	hide_cursor();
	set_display_attribute(FG_WHITE);
	move_terminal_cursor(10,4);
	printf_P(PSTR("  ______   __     __  _______         __    __"));
	move_terminal_cursor(10,5);
	printf_P(PSTR(" /      \\ |  \\   |  \\|       \\       |  \\  |  \\"));
	move_terminal_cursor(10,6);
	printf_P(PSTR("|  $$$$$$\\| $$   | $$| $$$$$$$\\      | $$  | $$  ______    ______    ______"));
	move_terminal_cursor(10,7);
	printf_P(PSTR("| $$__| $$| $$   | $$| $$__| $$      | $$__| $$ /      \\  /      \\  /      \\"));
	move_terminal_cursor(10,8);
	printf_P(PSTR("| $$    $$ \\$$\\ /  $$| $$    $$      | $$    $$|  $$$$$$\\|  $$$$$$\\|  $$$$$$\\"));
	move_terminal_cursor(10,9);
	printf_P(PSTR("| $$$$$$$$  \\$$\\  $$ | $$$$$$$\\      | $$$$$$$$| $$    $$| $$   \\$$| $$  | $$"));
	move_terminal_cursor(10,10);
	printf_P(PSTR("| $$  | $$   \\$$ $$  | $$  | $$      | $$  | $$| $$$$$$$$| $$      | $$__/ $$"));
	move_terminal_cursor(10,11);
	printf_P(PSTR("| $$  | $$    \\$$$   | $$  | $$      | $$  | $$ \\$$     \\| $$       \\$$    $$"));
	move_terminal_cursor(10,12);
	printf_P(PSTR(" \\$$   \\$$     \\$     \\$$   \\$$       \\$$   \\$$  \\$$$$$$$ \\$$        \\$$$$$$"));
	move_terminal_cursor(10,14);
	// change this to your name and student number; remove the chevrons <>
	printf_P(PSTR("CSSE2010/7201 A2 by Alvin Benny - 48012836"));
	
	// Output the static start screen and wait for a push button 
	// to be pushed or a serial input of 's'
	show_start_screen();

	uint32_t last_screen_update, current_time;
	last_screen_update = get_current_time();
	
	uint8_t frame_number = 0;
	move_terminal_cursor(10,18);
	clear_to_end_of_line();
	game_speed = 1000;
	printf("Game Speed: Normal");
	current_track = 0;
	move_terminal_cursor(10,20);
	printf("Selected Track: Through the Fire and Flames");
	


	// Wait until a button is pressed, or 's' is pressed on the terminal
	while(1)
	{
		
		// First check for if a 's' is pressed
		// There are two steps to this
		// 1) collect any serial input (if available)
		// 2) check if the input is equal to the character 's'
		char serial_input = -1;
		if (serial_input_available())
		{
			serial_input = fgetc(stdin);
		}
		
		move_terminal_cursor(10,16);
		//manual mode
		if ((serial_input == 'M' || serial_input == 'm') && !manual_mode){
			printf("Manual Mode");
			manual_mode = 1;
		}
		else if ((serial_input == 'M' || serial_input == 'm') && manual_mode) {
			clear_to_end_of_line();
			manual_mode = 0;
		}
		
		if (serial_input == '1') {
			game_speed = 1000;
			move_terminal_cursor(10,18);
			clear_to_end_of_line();
			printf("Game Speed: Normal");
		}
		else if (serial_input == '2'){
			game_speed = 500;
			move_terminal_cursor(10,18);
			clear_to_end_of_line();
			printf("Game Speed: Fast");
				
		}
		else if (serial_input == '3') {
			game_speed = 250;
			move_terminal_cursor(10,18);
			clear_to_end_of_line();
			printf("Game Speed: Extreme");
			
		}
		
		
		//Currently hardcoded
		if ((serial_input == 'T' || serial_input == 't')) {
			current_track = current_track + 1;
			move_terminal_cursor(10,20);
			clear_to_end_of_line();
			if (current_track > 2) {
				current_track = 0;
				printf("Selected Track: Through the Fire and Flames");
			}
			else if (current_track == 2){
				printf("Selected Track: A Battle For Time");
			}
			else if (current_track == 1) {
				printf("Selected Track: How Far I Could Go");
			}
		}

		
		// If the serial input is 's', then exit the start screen
		if (serial_input == 's' || serial_input == 'S')
		{
			break;
		}
		
		// Next check for any button presses
		int8_t btn = button_pushed();
		if (btn != NO_BUTTON_PUSHED)
		{
			break;
		}

		// every 200 ms, update the animation
		current_time = get_current_time();
		if (current_time - last_screen_update > game_speed/5)
		{
			update_start_screen(frame_number);
			frame_number = (frame_number + 1) % 32;
			last_screen_update = current_time;
		}
	}
}

void new_game(void)
{
	
	
	game_over_flag = 0; //ensure flag is off8
	score = 0; //reset the score for the new game
	combo = 0;
	DDRC = 0xFF; //For SSD score
	DDRD |= (1<<DDD2); //Set pin for cc
	DDRD |= (1<<DDD3); //Set pin for LED7 (pause)
	DDRD |= (1 << DDD7) | (1<<DDD5) | (1<<DDD6); //Set pins for combo LEDs
	DDRD |= (1 << DDD4); // Make pin OC1B be an output (port D, pin 4)	
	
	// Clear the serial terminal
	clear_terminal();
	
	// Initialise the game and display
	
	if (manual_mode) {
		move_terminal_cursor(10,6);
		printf("Manual Mode");
	}
	display_game_speed();
	PORTC = seven_seg[score]; //Display score during count down
	move_terminal_cursor(10,14);
	print_track();
	update_combo();
	game_count_down(); //do the count down
	initialise_game();

	
	
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
}

void play_game(void)
{
	
	uint32_t last_advance_time, current_time, last_off_time;
	uint8_t btn; // The button pushed
	
	last_advance_time = get_current_time();
	last_off_time = get_current_time();
	notes_pressed = 0; //initialise the notes passed to 0
	
	
	// We play the game until it's over
	while (!is_game_over())
	{
		
		update_ssd();
		// We need to check if any button has been pushed, this will be
		// NO_BUTTON_PUSHED if no button has been pushed
		// Checkout the function comment in `buttons.h` and the implementation
		// in `buttons.c`.
		btn = button_pushed();
		char serial_input = -1; //collect serial input
		if (serial_input_available())
		{
		serial_input = fgetc(stdin);
		}
		if (((btn == BUTTON0_PUSHED) || serial_input == 'f' || serial_input == 'F')&& !game_paused)
		{
			// If button 0 play the lowest note (right lane)
			play_note(3);
		}
		
		else if (((btn == BUTTON1_PUSHED) || serial_input == 'd' || serial_input == 'D') && !game_paused)
		{
			play_note(2);
		}
		
		else if (((btn == BUTTON2_PUSHED) || serial_input == 's' || serial_input == 'S' )&& !game_paused){
			play_note(1);
		}
		
		else if (((btn == BUTTON3_PUSHED) || serial_input == 'a' || serial_input == 'A')&& !game_paused) {
			play_note(0);
		}

	
		if ((serial_input == 'M' || serial_input == 'm') && !manual_mode){
			move_terminal_cursor(10,6);
			printf("Manual Mode");
			manual_mode = 1;
			notes_pressed = 0;
			
		}
		else if ((serial_input == 'M' || serial_input == 'm') && manual_mode) {
			move_terminal_cursor(10,6);
			clear_to_end_of_line();
			manual_mode = 0;	
		}
		
		if ((serial_input == 'P' || serial_input == 'p') && !game_paused){
			move_terminal_cursor(10,10);
			printf("Game Paused");
			game_paused = 1;
			PORTD |= (1 << PORTD3);
			OCR1B = 0;
			
		}
		else if ((serial_input == 'P' || serial_input == 'p') && game_paused){
			move_terminal_cursor(10,10);	
			clear_to_end_of_line();
			game_paused = 0;
			PORTD &= ~(1 << PORTD3);
			OCR1B = 1 - pulsewidth;
		}
		
		
		current_time = get_current_time();
		if (manual_mode == 0) {
			
			if (current_time >= last_off_time + game_speed){
				//turn off any notes 
				OCR1B = 0;
				last_off_time = current_time;
			}
			
			if (current_time >= last_advance_time + game_speed/5)
			{
				// 200ms (0.2 second) has passed since the last time we advance the
				// notes here, so update the advance the notes
			
				if (!game_paused){
					advance_note();

				}
				// Update the most recent time the notes were advance
				last_advance_time = current_time;	

			}
		}
		else {
			if ((serial_input == 'N') | (serial_input == 'n')) {
				advance_note();
				
				//if a note is currently being played
				if (OCR1B) {
					notes_pressed = notes_pressed + 1;
				}
				else {
					notes_pressed = 0;
				}

				if (notes_pressed == 5){
					
					OCR1B = 0;
					notes_pressed = 0;
				}
			}
			

		}

	}
	// We get here if the game is over.
}

void handle_game_over()
{
	move_terminal_cursor(10,4);
	update_terminal_score(score);
	move_terminal_cursor(10,6);
	print_track();
	move_terminal_cursor(10,14);
	clear_to_end_of_line();
	printf_P(PSTR("GAME OVER"));
	move_terminal_cursor(10,15);
	printf_P(PSTR("Press a button or 's'/'S' to start a new game"));
	
	// Do nothing until a button is pushed. Hint: 's'/'S' should also start a
	// new game

	char serial_input = -1; //collect serial input

	while (button_pushed() == NO_BUTTON_PUSHED)
	{
		update_ssd();
		if (serial_input_available()){
				serial_input = fgetc(stdin);
		}
		if (serial_input == 's' || serial_input == 'S'){
			break;
		}
	}
	start_screen();
	
}
void game_count_down(void) {
	
	//Number 3
	ledmatrix_clear();
	ledmatrix_update_pixel(3, 2, COLOUR_RED);
	ledmatrix_update_pixel(3, 3, COLOUR_RED);
	ledmatrix_update_pixel(3, 4, COLOUR_RED);
	ledmatrix_update_pixel(3, 5, COLOUR_RED);
	ledmatrix_update_pixel(4, 5, COLOUR_RED);
	ledmatrix_update_pixel(5, 5, COLOUR_RED);
	ledmatrix_update_pixel(6, 5, COLOUR_RED);
	ledmatrix_update_pixel(6, 4, COLOUR_RED);
	ledmatrix_update_pixel(6, 3, COLOUR_RED);
	ledmatrix_update_pixel(6, 2, COLOUR_RED);
	ledmatrix_update_pixel(7, 5, COLOUR_RED);
	ledmatrix_update_pixel(8, 5, COLOUR_RED);
	ledmatrix_update_pixel(9, 5, COLOUR_RED);
	ledmatrix_update_pixel(9, 4, COLOUR_RED);
	ledmatrix_update_pixel(9, 3, COLOUR_RED);
	ledmatrix_update_pixel(9, 2, COLOUR_RED);
	_delay_ms(game_speed*2); 
	
	//Number 2
	ledmatrix_clear();
	ledmatrix_update_pixel(3, 2, COLOUR_RED);
	ledmatrix_update_pixel(3, 3, COLOUR_RED);
	ledmatrix_update_pixel(3, 4, COLOUR_RED);
	ledmatrix_update_pixel(3, 5, COLOUR_RED);
	ledmatrix_update_pixel(4, 5, COLOUR_RED);
	ledmatrix_update_pixel(5, 5, COLOUR_RED);
	ledmatrix_update_pixel(6, 5, COLOUR_RED);
	ledmatrix_update_pixel(6, 4, COLOUR_RED);
	ledmatrix_update_pixel(6, 3, COLOUR_RED);
	ledmatrix_update_pixel(6, 2, COLOUR_RED);
	ledmatrix_update_pixel(7, 2, COLOUR_RED);
	ledmatrix_update_pixel(8, 2, COLOUR_RED);
	ledmatrix_update_pixel(9, 2, COLOUR_RED);
	ledmatrix_update_pixel(9, 3, COLOUR_RED);
	ledmatrix_update_pixel(9, 4, COLOUR_RED);
	ledmatrix_update_pixel(9, 5, COLOUR_RED);
	_delay_ms(game_speed*2);
	
	//Number 1
	ledmatrix_clear();
	ledmatrix_update_pixel(3, 3, COLOUR_RED);
	ledmatrix_update_pixel(3, 4, COLOUR_RED);
	ledmatrix_update_pixel(4, 3, COLOUR_RED);
	ledmatrix_update_pixel(4, 4, COLOUR_RED);
	ledmatrix_update_pixel(5, 3, COLOUR_RED);
	ledmatrix_update_pixel(5, 4, COLOUR_RED);
	ledmatrix_update_pixel(6, 3, COLOUR_RED);
	ledmatrix_update_pixel(6, 4, COLOUR_RED);
	ledmatrix_update_pixel(7, 3, COLOUR_RED);
	ledmatrix_update_pixel(7, 4, COLOUR_RED);
	ledmatrix_update_pixel(8, 3, COLOUR_RED);
	ledmatrix_update_pixel(8, 4, COLOUR_RED);
	ledmatrix_update_pixel(9, 3, COLOUR_RED);
	ledmatrix_update_pixel(9, 4, COLOUR_RED);
	ledmatrix_update_pixel(10, 3, COLOUR_RED);
	ledmatrix_update_pixel(10, 4, COLOUR_RED);
	_delay_ms(game_speed*2);
	
	//GOOOOOOOO
	ledmatrix_clear();
	ledmatrix_update_pixel(3, 0, COLOUR_GREEN);
	ledmatrix_update_pixel(3, 1, COLOUR_GREEN);
	ledmatrix_update_pixel(3, 2, COLOUR_GREEN);
	ledmatrix_update_pixel(3, 3, COLOUR_GREEN);
	ledmatrix_update_pixel(4, 0, COLOUR_GREEN);
	ledmatrix_update_pixel(5, 0, COLOUR_GREEN);
	ledmatrix_update_pixel(6, 0, COLOUR_GREEN);
	ledmatrix_update_pixel(7, 0, COLOUR_GREEN);
	ledmatrix_update_pixel(8, 0, COLOUR_GREEN);
	ledmatrix_update_pixel(9, 0, COLOUR_GREEN);
	ledmatrix_update_pixel(9, 1, COLOUR_GREEN);
	ledmatrix_update_pixel(9, 2, COLOUR_GREEN);
	ledmatrix_update_pixel(9, 3, COLOUR_GREEN);
	ledmatrix_update_pixel(8, 3, COLOUR_GREEN);
	ledmatrix_update_pixel(7, 3, COLOUR_GREEN);
	ledmatrix_update_pixel(7, 2, COLOUR_GREEN);
	
	//Letter O
	ledmatrix_update_pixel(3, 5, COLOUR_GREEN);
	ledmatrix_update_pixel(3, 6, COLOUR_GREEN);
	ledmatrix_update_pixel(3, 7, COLOUR_GREEN);
	ledmatrix_update_pixel(4, 5, COLOUR_GREEN);
	ledmatrix_update_pixel(5, 5, COLOUR_GREEN);
	ledmatrix_update_pixel(6, 5, COLOUR_GREEN);
	ledmatrix_update_pixel(7, 5, COLOUR_GREEN);
	ledmatrix_update_pixel(8, 5, COLOUR_GREEN);
	ledmatrix_update_pixel(9, 5, COLOUR_GREEN);
	ledmatrix_update_pixel(9, 6, COLOUR_GREEN);
	ledmatrix_update_pixel(9, 7, COLOUR_GREEN);
	ledmatrix_update_pixel(8, 7, COLOUR_GREEN);
	ledmatrix_update_pixel(7, 7, COLOUR_GREEN);
	ledmatrix_update_pixel(6, 7, COLOUR_GREEN);
	ledmatrix_update_pixel(5, 7, COLOUR_GREEN);
	ledmatrix_update_pixel(4, 7, COLOUR_GREEN);
	_delay_ms(game_speed*2);
	
}





