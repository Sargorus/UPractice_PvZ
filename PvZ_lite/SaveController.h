#include <iostream>
#include <fstream>

#include <string>
#include <windows.h>

#include <vector>
#include "Entities.h"

GameConfig _gameconfig;

bool CanLoadGame() {
	std::ifstream file("pvz_save.data");
	return file.good();
}

bool SaveGame(std::vector<Entity*> entities, Level level) {
	try
	{
		std::ofstream file("pvz_save.data");
		if (file.is_open()) {
			_gameconfig.Serialize(file);
			level.Serialize(file);
			for (Entity* e : entities) {
				e->Serialize(file);
			}
		}
		file.close();
	}
	catch (...)
	{
		return false;
	}
	return true;
}

void LoadGame(std::vector<Entity*>& entities, Level& level) {
	try
	{
		std::ifstream file("pvz_save.data");
		std::string type;
		while (file >> type)
		{
			if (type == "CONFIG") {
				_gameconfig.Deserialize(file);
			}
			else if (type == "LEVEL") {
				level.Deserialize(file);
			}
			else if (type == "HOME") {
				Home* home = new Home();
				home->Deserialize(file);
				entities.push_back(home);
			}
			else if (type == "ZOMBE") {
				Zombe* zombe = new Zombe();
				zombe->Deserialize(file);
				entities.push_back(zombe);
			}
			else if (type == "MISSILE") {
				Missile* missile = new Missile();
				missile->Deserialize(file);
				entities.push_back(missile);
			}
			else if (type == "PLANT") {
				std::string plant_type;
				file >> plant_type;
				Plant* plant = new Plant();
				if (plant_type == "0") {
					plant = new SunflowerPlant();
				}
				else if (plant_type == "1") {
					plant = new PeasfirePlant();
				}
				else if (plant_type == "2") {
					plant = new NutwallPlant();
				}
				else if (plant_type == "3") {
					plant = new CherryboomPlant();
				}
				plant->Deserialize(file);
				entities.push_back(plant);
			}
		}
		file.close();
	}
	catch (...)
	{

	}
}