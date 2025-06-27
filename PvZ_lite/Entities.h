#pragma once

#include <random> // для рандомной генерации волны
#include <fstream>
#include <memory>
#include <string>

// Объявляем перечисление (enum) для типов сущностей
enum class EntityType {
    ZOMBIE,
    PLANT,
    WALL,
    HOME,
    MISSILE,
    END_GAME_ZONE,
};


// Основная сущность
struct Entity {
    EntityType type;
    // Абсолютные координаты в консоле
    int X;
    int Y;

    virtual void Handle()
    {
    }

    virtual void Serialize(std::ostream& os) { }
    virtual void Deserialize(std::istream& is) { }
};

struct Home : public Entity
{
    int LineNumber;
    Home(int x, int y, int lineNubmer) : LineNumber(lineNubmer) {
        this->type = EntityType::HOME;
        this->X = x;
        this->Y = y;
    }
    Home() { }

    virtual void Serialize(std::ostream& os) override {
        os << "HOME " << this->X << " " << this->Y << " " << LineNumber << "\n";
    }
    virtual void Deserialize(std::istream& is) override {
        this->type = EntityType::HOME;
        is >> this->X >> this->Y >> LineNumber;
    }
};

enum class ZombeType {
    STANDART,
    FASTER,
    UNSTOPABLE,
    BOSS
};

struct Zombe : public Entity {
    int Health = 3;
    int Damage = 1;
    int Speed = 10;        // клеток за 10 кадров
    int SpeedCounter = 0; // накопленный счётчик
    ZombeType zombeType = ZombeType::STANDART;

    Zombe(int x, int y) {
        this->type = EntityType::ZOMBIE;
        this->X = x;
        this->Y = y;
    }
    Zombe() { }


    void GetDamage(int damage) {
        Health -= damage;
    }

    virtual void Serialize(std::ostream& os) override {
        os << "ZOMBE " << this->X << " " << this->Y << " " << Health << " " << Damage << " " << Speed << " " << SpeedCounter << " " << static_cast<int>(zombeType) << "\n";
    }
    void Serialize(std::ostream& os, int counter) {
        os << "ZOMBE " << counter << " " << this->X << " " << this->Y << " " << Health << " " << Damage << " " << Speed << " " << SpeedCounter << " " << static_cast<int>(zombeType) << "\n";
    }
    virtual void Deserialize(std::istream& is) override {
        this->type = EntityType::ZOMBIE;
        int zombT;
        is >> this->X >> this->Y >> Health >> Damage >> Speed >> SpeedCounter >> zombT; // zombeType добавить
        switch (zombT)
        {
        case 0:
            zombeType = ZombeType::STANDART;
            break;
        case 1:
            zombeType = ZombeType::FASTER;
            break;
        case 2:
            zombeType = ZombeType::UNSTOPABLE;
            break;
        case 3:
            zombeType = ZombeType::BOSS;
            break;
        default:
            break;
        }
    }
};

struct ZombeFaster : public Zombe {
    ZombeFaster(int x, int y) : Zombe(x, y) {
        this->X = x;
        this->Y = y;

        this->Speed = 20;
        this->zombeType = ZombeType::FASTER;
    }
};

struct ZombeUnstopable : public Zombe {
    ZombeUnstopable(int x, int y) : Zombe(x, y) {
        this->X = x;
        this->Y = y;

        this->Health = 9;
        this->zombeType = ZombeType::UNSTOPABLE;
    }
};

struct ZombeBoss : public Zombe {
    ZombeBoss(int x, int y) : Zombe(x, y) {
        this->X = x;
        this->Y = y;

        this->Health = 18;
        this->Speed = 15;
        this->zombeType = ZombeType::BOSS;
    }
};

struct Missile : public Entity {
    int Damage;
    int Speed;
    int SpeedCounter;

    Missile(int x, int y, int damage, int speed)
        : Damage(damage), Speed(speed), SpeedCounter(0) {
        this->type = EntityType::MISSILE;
        this->X = x;
        this->Y = y;
    }
    Missile() { }

    virtual void Serialize(std::ostream& os) override {
        os << "MISSILE " << this->X << " " << this->Y << " " << Damage << " " << Speed << " " << SpeedCounter << "\n";
    }
    virtual void Deserialize(std::istream& is) override {
        this->type = EntityType::MISSILE;
        is >> this->X >> this->Y >> Damage >> Speed >> SpeedCounter; // zombeType добавить
    }
};

enum class PlantType {
    SUNFLOWER,
    PEASFIRE,
    NUTWALL,
    CHERRYBOOM
    // Добавлено для расширяемости
};

struct Plant : public Entity {
    int Health;
    int Cost;
    PlantType plantType;

    Plant(int x, int y, int health, int cost , PlantType plantType) : Health(health), Cost(cost), plantType(plantType)
    {
        this->type = EntityType::PLANT;
        this->X = x;
        this->Y = y;
    }
    Plant() { }

    void GetDamage(int damage)
    {
        Health -= damage;
    }
};

struct SunflowerPlant : public Plant
{
    int SumSalary = 1;
    SunflowerPlant(int x, int y) : Plant(x, y, 3, 10, PlantType::SUNFLOWER)
    {

    }
    SunflowerPlant() { }

    virtual void Serialize(std::ostream& os) override {
        os << "PLANT " << static_cast<int>(this->plantType) << " " << this->X << " " << this->Y << " " << this->Health << " " << this->Cost << "\n";
    }
    virtual void Deserialize(std::istream& is) override {
        this->type = EntityType::PLANT;
        is >> this->X >> this->Y >> this->Health >> this->Cost;
        this->plantType = PlantType::SUNFLOWER;
    }
};

struct PeasfirePlant : public Plant {
    int ShootRate = 3;     // количество выстрелов за 10 кадров
    int ShootCounter = 0;  // накопленный счётчик

    int MissileDamage = 1; // урон снаряда
    int MissileSpeed = 20;  // скорость снаряда

    PeasfirePlant(int x, int y) : Plant(x, y, 2, 10, PlantType::PEASFIRE)
    {

    }
    PeasfirePlant() { }

    virtual void Serialize(std::ostream& os) override {
        os << "PLANT " << static_cast<int>(this->plantType) << " " << this->X << " " << this->Y << " " << this->Health << " " << this->Cost << "\n";
    }
    virtual void Deserialize(std::istream& is) override {
        this->type = EntityType::PLANT;
        is >> this->X >> this->Y >> this->Health >> this->Cost;
        this->plantType = PlantType::PEASFIRE;
    }
};

struct NutwallPlant : public Plant {
    NutwallPlant(int x, int y) : Plant(x, y, 20, 20, PlantType::NUTWALL)
    {

    }
    NutwallPlant() { }

    virtual void Serialize(std::ostream& os) override {
        os << "PLANT " << static_cast<int>(this->plantType) << " " << this->X << " " << this->Y << " " << this->Health << " " << this->Cost << "\n";
    }
    virtual void Deserialize(std::istream& is) override {
        this->type = EntityType::PLANT;
        is >> this->X >> this->Y >> this->Health >> this->Cost;
        this->plantType = PlantType::NUTWALL;
    }
};

struct CherryboomPlant : public Plant 
{
    int Damage = 9;
    int XradiusDamage = 4; // урон в лево и в право на 2 клетки  --с--
    int YradiusDamage = 1; // урон на линию свеху и линию снизу

    CherryboomPlant(int x, int y) : Plant(x, y, 100, 10, PlantType::CHERRYBOOM)
    {

    }
    CherryboomPlant() { }

    virtual void Serialize(std::ostream& os) override {
        os << "PLANT " << static_cast<int>(this->plantType) << " " << this->X << " " << this->Y << " " << this->Health << " " << this->Cost << "\n";
    }
    virtual void Deserialize(std::istream& is) override {
        this->type = EntityType::PLANT;
        is >> this->X >> this->Y >> this->Health >> this->Cost;
        this->plantType = PlantType::CHERRYBOOM;
    }
};



struct Wave {
    int StartTick;                // Тик, на котором начинается волна
    int Mod;                     // Модификатор сложности

    // Для каждой линии свой вектор зомби
    std::vector<std::vector<Zombe*>> ZombiesPerLine;

    // Индексы спавна для каждой линии
    std::vector<size_t> SpawnIndices;

    // Инициализация для заданного количества линий
    void InitLines(size_t numLines) {
        ZombiesPerLine.resize(numLines);
        SpawnIndices.resize(numLines, 0);
    }

    // Генерация волны: для каждой линии генерируем зомби отдельно
    void GenerateWave(int max, int min, int mod, int X, const std::vector<int>& linesY)
    {
        srand(time(NULL)); // Инициализация генератора

        size_t numLines = linesY.size();
        InitLines(numLines);

        if (mod == 3)
        {
            mod += 0;
        }

        for (size_t line = 0; line < numLines; ++line)
        {
            int maxZombiesInLine = rand() % max + min;
            for (int i = 0; i < maxZombiesInLine; ++i)
            {
                int choice = rand() % mod + 1;
                int y = linesY[line];
                switch (choice) {
                case 1:
                    ZombiesPerLine[line].push_back(new Zombe(X, y)); // Простой
                    break;
                case 2:
                    ZombiesPerLine[line].push_back(new ZombeFaster(X, y)); // Быстрый
                    break;
                case 3:
                    ZombiesPerLine[line].push_back(new ZombeUnstopable(X, y)); // Бронированный
                    break;
                case 4:
                    ZombiesPerLine[line].push_back(new ZombeBoss(X, y)); // Босс
                    break;
                }
            }
        }
    }

    // Проверка, есть ли ещё зомби для спавна на всех линиях
    bool HasNext() const
    {
        for (size_t i = 0; i < ZombiesPerLine.size(); ++i)
        {
            if (SpawnIndices[i] < ZombiesPerLine[i].size())
                return true;
        }
        return false;
    }

    // Получить следующего зомби из всех линий (например, по очереди или по приоритету линий)
    // Здесь пример — по очереди по линиям
    Zombe* GetNextZombie()
    {
        for (size_t i = 0; i < ZombiesPerLine.size(); ++i)
        {
            if (SpawnIndices[i] < ZombiesPerLine[i].size())
                return ZombiesPerLine[i][SpawnIndices[i]++];
        }
        return nullptr;
    }


    void Serialize(std::ostream& os) {
        int counter = 0;
        os << "WAVE " << StartTick << " " << Mod << " " << ZombiesPerLine.size() << "\n";
        for (size_t size : SpawnIndices)
            os << "SIX " << size << '\n';
        for (std::vector<Zombe*> vec : ZombiesPerLine) {
            for (Zombe* zom : vec) {
                zom->Serialize(os, counter);
            }
            counter++;
        }
        os << "====" << "\n";
    }
    void Deserialize(std::istream& is) {
        is >> StartTick >> Mod;
        std::string param;
        is >> param;
        ZombiesPerLine.resize(std::stoi(param));
        while (is >> param)
        {
            if (param == "SIX") {
                is >> param;
                SpawnIndices.push_back(std::stoi(param));
            }
            else if (param == "ZOMBE") {
                is >> param;
                int indexLine = std::stoi(param);
                Zombe* zombe = new Zombe();
                zombe->Deserialize(is);
                ZombiesPerLine[indexLine].push_back(zombe);
            }
            else if (param == "====") {
                break;
            }
        }
    }

    //void Dispose()
    //{
    //    for (auto& lineZombies : ZombiesPerLine)
    //    {
    //        for (auto z : lineZombies) {
    //            delete z;
    //        }
    //        lineZombies.clear();
    //    }
    //    ZombiesPerLine.clear();
    //}
};

struct Level {
    int LevelNumber;

    std::vector<Wave> Waves;
    std::vector<int> LinesY; // Координаты линий по Y
    int balance = 20;


    bool InfiniteMode = false;      // Режим бесконечных волн
    int CurrentWaveIndex = 0;       // Текущая волна
    int LastWaveEndTick = -1;       // Время окончания последней волны
    int NextWaveDelay = 10;         // Задержка между волнами в тиках

    // Инициализация линий (например, 3 линии с координатами Y)
    void InitLines(const std::vector<int>& lines)
    {
        LinesY = lines;
    }

    // Добавление волны
    void AddWave(const Wave& wave)
    {
        Waves.push_back(wave);
    }


    // Статический метод создания бесконечного уровня с произвольным количеством линий
    Level CreateLvl0(const std::vector<int>& linesY)
    {
        Level level0;
        level0.LevelNumber = 0;
        level0.InfiniteMode = true; // Включаем бесконечный режим
        level0.InitLines(linesY);

        Wave initialWave;
        initialWave.StartTick = 5;
        initialWave.Mod = 2;
        initialWave.GenerateWave(4, 2, initialWave.Mod, 0, level0.LinesY);
        level0.AddWave(initialWave);

        return level0;
    }

    // Статические методы для создания уровней
    static Level CreateLvl0() {
        Level level0;
        level0.LevelNumber = 0;
        level0.InfiniteMode = true; // Включаем бесконечный режим
        level0.InitLines({ 3, 5, 7 });

        Wave initialWave;
        initialWave.StartTick = 5;
        initialWave.Mod = 2;
        initialWave.GenerateWave(4, 2, initialWave.Mod, 0, level0.LinesY);
        level0.AddWave(initialWave);

        return level0;
    }

    // Пример создания других уровней с фиксированным набором линий
    Level CreateLvl1()
    {
        Level level1;
        level1.LevelNumber = 1;
        level1.InfiniteMode = false;
        level1.InitLines({ 3, 5, 7 });

        Wave wave1;
        wave1.StartTick = 5;
        wave1.Mod = 1;
        wave1.GenerateWave(3, 1, wave1.Mod, 0, level1.LinesY);
        level1.AddWave(wave1);

        Wave wave2;
        wave2.StartTick = 20;
        wave2.Mod = 1;
        wave2.GenerateWave(5, 2, wave2.Mod, 0, level1.LinesY);
        level1.AddWave(wave2);

        Wave wave3;
        wave3.StartTick = 30;
        wave3.Mod = 2;
        wave3.GenerateWave(7, 5, wave3.Mod, 0, level1.LinesY);
        level1.AddWave(wave3);

        return level1;
    }

    Level CreateLvl2()
    {
        Level level2;
        level2.LevelNumber = 2;
        level2.InfiniteMode = false;
        level2.InitLines({ 3, 5, 7 });

        Wave wave1;
        wave1.StartTick = 5;
        wave1.Mod = 1;
        wave1.GenerateWave(3, 1, wave1.Mod, 0, level2.LinesY);
        level2.AddWave(wave1);

        Wave wave2;
        wave2.StartTick = 20;
        wave2.Mod = 2;
        wave2.GenerateWave(5, 2, wave2.Mod, 0, level2.LinesY);
        level2.AddWave(wave2);

        Wave wave3;
        wave3.StartTick = 30;
        wave3.Mod = 3;
        wave3.GenerateWave(14, 5, wave3.Mod, 0, level2.LinesY);
        level2.AddWave(wave3);

        return level2;
    }

    Level CreateLvl3()
    {
        Level level3;
        level3.LevelNumber = 2;
        level3.InfiniteMode = false;
        level3.InitLines({ 3, 5, 7 });

        Wave wave1;
        wave1.StartTick = 5;
        wave1.Mod = 2;
        wave1.GenerateWave(3, 2, wave1.Mod, 0, level3.LinesY);
        level3.AddWave(wave1);

        Wave wave2;
        wave2.StartTick = 20;
        wave2.Mod = 3;
        wave2.GenerateWave(5, 2, wave2.Mod, 0, level3.LinesY);
        level3.AddWave(wave2);

        Wave wave3;
        wave3.StartTick = 30;
        wave3.Mod = 3;
        wave3.GenerateWave(8, 5, wave3.Mod, 0, level3.LinesY);
        level3.AddWave(wave3);

        Wave wave4;
        wave4.StartTick = 40;
        wave4.Mod = 3;
        wave4.GenerateWave(14, 5, wave4.Mod, 0, level3.LinesY);
        level3.AddWave(wave3);

        return level3;
    }


    void Serialize(std::ostream& os) {
        os << "LEVEL " << LevelNumber << " " << InfiniteMode << " " << CurrentWaveIndex << " " << LastWaveEndTick << " " << NextWaveDelay << " " << balance << "\n";
        for (int y : LinesY) {
            os << "LINE " << y << "\n";
        }
        for (Wave w : Waves) {
            w.Serialize(os);
        }
        os << "======" << "\n";
    }

    void Deserialize(std::istream& is) {
        is >> LevelNumber >> InfiniteMode >> CurrentWaveIndex >> LastWaveEndTick >> NextWaveDelay >> balance;
        std::string param;
        while (is >> param)
        {
            if (param == "LINE") {
                is >> param;
                LinesY.push_back(std::stoi(param));
            }
            else if (param == "WAVE") {
                Wave wave;
                wave.Deserialize(is);
                Waves.push_back(wave);
            }
            else if (param == "======") {
                break;
            }
        }
    }
};

struct GameConfig {
    int House_offset = 6;
    int Game_zone_length = 10 * 2;
    int Zombie_place_offset = 20;
    int Line_length = House_offset + Game_zone_length + Zombie_place_offset;

    int Rows, Columns = 10, LastLineY;

    void Serialize(std::ostream& os) {
        os << "CONFIG " << Rows << " " << LastLineY << "\n";
    }
    void Deserialize(std::istream& is) {
        is >> Rows >> LastLineY;
    }
};
