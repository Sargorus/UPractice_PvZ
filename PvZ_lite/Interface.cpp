
#include <iostream>
#include <string>
#include <vector>
#include "Utilities.h"
#include "Entities.h"
#include "SaveController.h"

#define home() 			printf(ESC "[H") //Move cursor to the indicated row, column (origin at 1,1)
#define clrscr()		printf(ESC "[2J") //lear the screen, move to (1,1)
#define gotoxy(x,y)		printf(ESC "[%d;%dH", y, x);
#define visible_cursor() printf(ESC "[?251");
//Set Display Attribute Mode	<ESC>[{attr1};...;{attrn}m
#define resetcolor() printf(ESC "[0m")
#define set_display_atrib(color) 	printf(ESC "[%dm",color)

void Start(int rows, int columns, int house_offset, int FLy, int linelength);
void Start(int rows, int columns, int house_offset, int FLy, int linelength, std::vector<Entity*> entities, Level level);

int _house_offset = 6;
int _game_zone_length = 10 * 2;
int _zombie_place_offset = 20;
int _line_length = _house_offset + _game_zone_length + _zombie_place_offset;

int _rows, _columns = 10, _lastLineY;

const char _ascii_uppercase[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const int _ascii_uppercase_length = 27;

void writeColumns(int columns, int y) {
	gotoxy(_house_offset, y);
	for (int i = 0; i < columns; i++)
	{
		char sym = _ascii_uppercase[i];
		printf("|%c", sym);
		if (i + 1 == columns) {
			printf("|");
		}
	}
}

void DrawMap(int rows, std::vector<Entity*> entities, Level level) {
	Start(rows, _columns, _house_offset, 3, _line_length, entities, level);
}

void DrawMap(int rows) {
	_rows = rows;

	writeColumns(_columns, 1);

	int next_line_y = 2;
	std::string line(_line_length, '-');
	for (int i = 0; i < rows; i++)
	{
		if (i == 0) {
			gotoxy(1, next_line_y);
			std::cout << line;
		}

		gotoxy(2, next_line_y + 1);
		printf("%d", i + 1);

		gotoxy(_house_offset, next_line_y + 1);
		set_display_atrib(B_GREEN);
		printf("|");
		resetcolor();

		gotoxy(1, next_line_y + 2);
		std::cout << line;

		next_line_y += 2;
	}

	writeColumns(_columns, next_line_y + 1);
	_lastLineY = next_line_y + 1;

	gotoxy(1, _lastLineY + 2);
	printf("LEVEL: %d | BALANCE: %d", 1, 100);
	printf("\nAVALIABLE PLANTS: S - sunflower P - peacfire N - nutwall C - cherryboom");
	gotoxy(1, _lastLineY + 4);

	_gameconfig.Rows = rows;
	_gameconfig.LastLineY = _lastLineY;
	Start(rows, _columns, _house_offset, 3, _line_length);
}

void RefreshMap(std::vector<Entity*> entities, int& summa) {
	clrscr();

	writeColumns(_columns, 1);
	std::string line(_line_length, '-');
	Home* home;
	Zombe* zombe;
	Plant* plant;

	for (auto& ent : entities)
	{
		gotoxy(ent->X, ent->Y);

		switch (ent->type)
		{
		case EntityType::HOME:
			home = dynamic_cast<Home*>(ent);

			set_display_atrib(B_GREEN);
			printf("|");
			resetcolor();

			gotoxy(1, ent->Y);
			printf("%d", home->LineNumber);

			gotoxy(1, ent->Y - 1);
			std::cout << line;

			gotoxy(1, ent->Y + 1);
			std::cout << line;

			break;

		case EntityType::ZOMBIE:
			zombe = dynamic_cast<Zombe*>(ent);
			switch (zombe->zombeType)
			{
			case ZombeType::STANDART:
				set_display_atrib(B_GREEN);
				break;
			case ZombeType::FASTER:
				set_display_atrib(B_BLUE);
				break;
			case ZombeType::UNSTOPABLE:
				set_display_atrib(B_YELLOW); // Раньше не правильно работал, поменял на цифры жёлтого цвета, теперь работает
				break;
			case ZombeType::BOSS:
				set_display_atrib(B_RED);
				break;
			}
			printf("Z");
			resetcolor();
			break;

		case EntityType::PLANT:
			plant = dynamic_cast<Plant*>(ent);
			switch (plant->plantType)
			{
			case PlantType::SUNFLOWER:
				printf("S");
				break;
			case PlantType::PEASFIRE:
				printf("P");
				break;
			case PlantType::NUTWALL:
				printf("N");
				break;
			case PlantType::CHERRYBOOM:
				printf("C");
				break;
			}
			break;

		case EntityType::MISSILE:
			printf(">");
			break;
		}
	}

	writeColumns(_columns, _lastLineY);
	gotoxy(1, _lastLineY + 2);
	printf("LEVEL: %d | BALANCE: %d", 1, summa);
	printf("\nAVALIABLE PLANTS: S - sunflower P - peacfire N - nutwall C - cherryboom");
	gotoxy(1, _lastLineY + 4);
}

void DrawAndHandleDialogMenu() {
	setlocale(LC_ALL, "RUS");
	int counter = 1;
	bool canContinue = CanLoadGame();
	if (canContinue) {
		std::cout << counter << ". Продолжить игру\n";
		counter++;
	}
	std::cout << counter << ". Новая игра\n";
	counter++;

	int choose;
	std::cin >> choose;

	if (canContinue && choose == 1) {
		std::vector<Entity*> entities;
		Level level;
		LoadGame(entities, level);

		_rows = _gameconfig.Rows;
		_columns = _gameconfig.Columns;
		_lastLineY = _gameconfig.LastLineY;

		DrawMap(level.LinesY.size(), entities, level);
		return;
	}
	DrawMap(3);
}