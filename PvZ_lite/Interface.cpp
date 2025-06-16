#include <iostream>
#include <string>
#include "Utilities.h"

#define home() 			printf(ESC "[H") //Move cursor to the indicated row, column (origin at 1,1)
#define clrscr()		printf(ESC "[2J") //lear the screen, move to (1,1)
#define gotoxy(x,y)		printf(ESC "[%d;%dH", y, x);
#define visible_cursor() printf(ESC "[?251");
//Set Display Attribute Mode	<ESC>[{attr1};...;{attrn}m
#define resetcolor() printf(ESC "[0m")
#define set_display_atrib(color) 	printf(ESC "[%dm",color)

/*

--------------------------------------------------------------
	|P|P|									|			Z
--------------------------------------------------------------


*/

void drawMap(int rows, int columns) {
	const char ascii_uppercase[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const int ascii_uppercase_length = 27;
	if (columns > ascii_uppercase_length) {
		columns = ascii_uppercase_length;
	}

	const int house_offset = 6;
	const int game_zone_length = columns * 2;
	const int zombie_place_offset = 20;
	int line_length = house_offset + game_zone_length + zombie_place_offset;
	

	gotoxy(house_offset, 1);
	for (int i = 0; i < columns; i++)
	{
		char sym = ascii_uppercase[i];
		printf("|%c", sym);
		if (i + 1 == columns) {
			printf("|");
		}
	}

	int next_line_y = 2;
	for (int i = 0; i < rows; i++)
	{
		std::string line(line_length, '-');

		if (i == 0) {
			gotoxy(1, next_line_y);
			std::cout << line;
		}

		gotoxy(house_offset, next_line_y + 1);
		set_display_atrib(B_GREEN);
		printf("|");
		resetcolor();

		gotoxy(house_offset + game_zone_length, next_line_y + 1);
		set_display_atrib(B_RED);
		printf("|");
		resetcolor();

		gotoxy(1, next_line_y + 2);
		std::cout << line;

		next_line_y += 2;
	}

	gotoxy(house_offset, next_line_y + 1);
	for (int i = 0; i < columns; i++)
	{
		char sym = ascii_uppercase[i];
		printf("|%c", sym);
		if (i + 1 == columns) {
			printf("|");
		}
	}
}

int start() {
	/*gotoxy(4, 1);
	printf("|A|B|C|D|E\n");
	printf("------------------------------------------------------");

	gotoxy(1, 3);
	printf("1");

	gotoxy(4, 3);
	set_display_atrib(B_RED);
	printf("|");
	resetcolor();

	gotoxy(1, 4);
	printf("------------------------------------------------------");*/

	drawMap(5, 10);

	char k;
	std::cin >> k;
	return 1;
}


void test() {
	printf("1\n");
	printf("2\n");

	home();
	clrscr();
	printf("Home + clrscr\n");
	gotoxy(20, 7);
	printf("gotoxy(20,7)");

	gotoxy(1, 10);
	printf("gotoxy(1,10)  \n\n");

	set_display_atrib(BRIGHT);
	printf("Formatting text:\n");
	resetcolor();

	set_display_atrib(BRIGHT);
	printf("Bold\n");
	resetcolor();

	set_display_atrib(DIM);
	printf("Dim\n");
	resetcolor();

	set_display_atrib(BLINK);
	printf("Blink\n");
	resetcolor();

	set_display_atrib(REVERSE);
	printf("Reverse\n");
	printf("\n");


	set_display_atrib(BRIGHT);
	printf("Text color example:\n");
	resetcolor();

	set_display_atrib(F_RED);
	printf("Red\n");
	resetcolor();

	set_display_atrib(F_GREEN);
	printf("Green\n");
	resetcolor();

	set_display_atrib(F_BLUE);
	printf("Blue\n");
	resetcolor();

	set_display_atrib(F_CYAN);
	printf("Cyan\n");
	resetcolor();

	set_display_atrib(BRIGHT);
	printf("\nBottom color example:\n");
	resetcolor();

	set_display_atrib(B_RED);
	printf("Red\n");
	resetcolor();

	set_display_atrib(B_GREEN);
	printf("Green\n");
	resetcolor();

	set_display_atrib(B_BLUE);
	printf("Blue\n");
	resetcolor();

	set_display_atrib(B_CYAN);
	printf("Cyan\n");
	printf("\n");
	resetcolor();
}