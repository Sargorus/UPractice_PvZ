#include <vector>
#include <windows.h>
#include <string>


#include "Entities.h"


void RefreshMap(std::vector<Entity*> entities, int& summa);
bool GetInput(HANDLE hStdin, DWORD originalMode, std::vector<Entity*>& entities, int house_offset, std::vector<int> linesY, int columns, int& summa, Level level);


// Сортировка сущностей по X (блок-схема +)
void SortEntitiesByX(std::vector<Entity*>& entities, bool ascending)
{
    size_t n = entities.size();
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 1; j < n - i; ++j)
        {
            bool condition = ascending ?
                (entities[j - 1]->X > entities[j]->X) :
                (entities[j - 1]->X < entities[j]->X);

            if (condition)
            {
                std::swap(entities[j - 1], entities[j]);
            }
        }
    }
}

// Обработка снарядов с новой логикой скорости
void ProcessMissiles(std::vector<Entity*>& entities, int linelength,
    std::vector<Missile*>& missilesToRemove,
    std::vector<Zombe*>& zombiesToRemove) {
    for (Entity* em : entities)
    {
        if (em->type != EntityType::MISSILE)
        {
            continue;
        }
        Missile* missile = dynamic_cast<Missile*>(em);

        missile->SpeedCounter += missile->Speed;
        int moveSteps = missile->SpeedCounter / 10;
        missile->SpeedCounter %= 10;

        if (moveSteps == 0) {
            continue;
        }

        bool hit = false;
        for (int step = 1; step <= moveSteps; ++step) {
            int checkX = missile->X + step;
            for (Entity* e : entities) {
                if (e->type != EntityType::ZOMBIE)
                {
                    continue;
                }
                Zombe* zombie = dynamic_cast<Zombe*>(e);
                if (zombie && zombie->X == checkX && zombie->Y == missile->Y) {
                    zombie->GetDamage(missile->Damage);
                    hit = true;
                    if (zombie->Health <= 0) zombiesToRemove.push_back(zombie);
                    break;
                }
            }
            if (hit) break;
        }

        if (hit) {
            missilesToRemove.push_back(missile);
            continue;
        }

        missile->X += moveSteps;
        if (missile->X > linelength) {
            missilesToRemove.push_back(missile);
            continue;
        }

    }

}

// Нашёл проблему с теми зомби с -57658.... что-то там. Они удаляются из вектора энтити, но указатель в волне на него остаётся, поэтому надо вот ещё эту функцию использовать: (блок-схема +)
void RemoveDeadZombiesFromWaves(Level& level, const std::vector<Zombe*>& zombiesToRemove) {
    for (Wave& wave : level.Waves)
    {
        for (auto& line : wave.ZombiesPerLine)
        {
            for (Zombe* z : zombiesToRemove)
            {
                auto it = std::find(line.begin(), line.end(), z);
                if (it != line.end())
                {
                    line.erase(it);
                }
            }
        }
    }
}




void ProcessZombies(std::vector<Entity*>& entities,
    std::vector<Zombe*>& zombiesToRemove,
    std::vector<Plant*>& plantsToRemove,
    std::vector<Missile*>& missilesToRemove,
    int& gameover) {
    std::vector<Entity*> zombies;
    for (Entity* e : entities) {
        if (e->type == EntityType::ZOMBIE) zombies.push_back(e);
    }
    SortEntitiesByX(zombies, true); // Левых первыми

    for (Entity* e : zombies) {
        Zombe* zombie = dynamic_cast<Zombe*>(e);
        if (!zombie) continue;

        zombie->SpeedCounter += zombie->Speed;
        int moveSteps = zombie->SpeedCounter / 10;
        zombie->SpeedCounter %= 10;

        if (moveSteps == 0) continue;

        for (int step = 0; step < moveSteps; ++step) {
            int targetX = zombie->X - 1;

            // Проверяем зомби слева
            Zombe* leftZombie = nullptr;
            for (Entity* other : entities) {
                if (other->type == EntityType::ZOMBIE && other->Y == zombie->Y && other->X == targetX) {
                    leftZombie = dynamic_cast<Zombe*>(other);
                    break;
                }
            }

            if (leftZombie && leftZombie->Speed < zombie->Speed) {
                // Попытка перепрыгнуть зомби слева

                int jumpX = leftZombie->X - 1; // Клетка за зомби слева

                // Проверяем, что клетка за зомби слева свободна или содержит растение/дом/снаряд
                bool canJump = true;
                Entity* blockingEntity = nullptr;
                for (Entity* other : entities) {
                    if (other->X == jumpX && other->Y == zombie->Y) {
                        blockingEntity = other;
                        break;
                    }
                }
                bool attackedOrMoved;
                if (blockingEntity) {
                    switch (blockingEntity->type) {
                    case EntityType::PLANT: {
                        Plant* plant = dynamic_cast<Plant*>(blockingEntity);
                        if (plant) {
                            plant->GetDamage(zombie->Damage);
                            if (plant->Health <= 0) plantsToRemove.push_back(plant);
                        }
                        // Зомби прыгает и атакует растение
                        zombie->X = jumpX;
                        // Прыжок завершён
                        attackedOrMoved = true;
                        break;
                    }
                    case EntityType::HOME:
                        gameover = 1;
                        // Зомби прыгает и атакует дом
                        zombie->X = jumpX;
                        attackedOrMoved = true;
                        break;
                    case EntityType::MISSILE: {
                        Missile* missile = dynamic_cast<Missile*>(blockingEntity);
                        if (missile) {
                            zombie->GetDamage(missile->Damage);
                            missilesToRemove.push_back(missile);
                            if (zombie->Health <= 0) {
                                zombiesToRemove.push_back(zombie);
                                // Зомби умер, прыжок прерван
                                break;
                            }
                            // Зомби прыгает на клетку с снарядом и получает урон
                            zombie->X = jumpX;
                            attackedOrMoved = true;
                        }
                        break;
                    }
                    case EntityType::ZOMBIE:
                        // Нельзя прыгать на занятую клетку зомби — отменяем прыжок
                        canJump = false;
                        break;
                    default:
                        // Если что-то другое — считаем клетку занятой
                        canJump = false;
                        break;
                    }
                    if (!canJump) {
                        // Прыжок невозможен — отменяем движение
                        break;
                    }
                    if (attackedOrMoved) {
                        // Прыжок выполнен — переходим к следующему шагу
                        break;
                    }
                }
                else {
                    // Клетка свободна — прыгаем
                    zombie->X = jumpX;
                    attackedOrMoved = true;
                    break;
                }

                if (attackedOrMoved) break;
            }
            else {
                // Нет зомби слева с меньшей скоростью — обычное движение
                // Проверяем препятствия и атаки (аналогично вашему коду)
                bool blocked = false;
                bool attacked = false;

                for (Entity* other : entities) {
                    if (other == zombie) continue;
                    if (other->X == targetX && other->Y == zombie->Y) {
                        switch (other->type) {
                        case EntityType::PLANT: {
                            Plant* plant = dynamic_cast<Plant*>(other);
                            if (plant) {
                                plant->GetDamage(zombie->Damage);
                                if (plant->Health <= 0) plantsToRemove.push_back(plant);
                            }
                            attacked = true;
                            break;
                        }
                        case EntityType::HOME:
                            gameover = 1;
                            attacked = true;
                            break;
                        case EntityType::ZOMBIE:
                            blocked = true;
                            break;
                        case EntityType::MISSILE: {
                            Missile* missile = dynamic_cast<Missile*>(other);
                            zombie->GetDamage(missile->Damage);
                            if (zombie->Health <= 0) zombiesToRemove.push_back(zombie);
                            missilesToRemove.push_back(missile);
                            break;
                        }
                        default:
                            break;
                        }
                        if (attacked || blocked) break;
                    }
                }

                if (attacked || blocked) break;

                zombie->X = targetX;
            }
        }
    }
}


// Удаление помеченных объектов из вектора entities, блок-схема+
template<typename T>
void RemoveEntities(std::vector<Entity*>& entities, std::vector<T*>& toRemove)
{
    for (T* obj : toRemove)
    {
        auto it = std::find(entities.begin(), entities.end(), obj);
        if (it != entities.end())
        {
            delete* it;
            entities.erase(it);
        }
    }
    toRemove.clear();
}

Missile* SpawnMissile(Plant* plant, int x, int y) {
    if (plant->plantType == PlantType::PEASFIRE) {
        PeasfirePlant* peasfire = dynamic_cast<PeasfirePlant*>(plant);
        if (peasfire)
        {
            return new Missile(x, y, peasfire->MissileDamage, peasfire->MissileSpeed);
        }
    }
    // если вызвало друго растение
    return nullptr;
}

void PlantAct(std::vector<Entity*>& entities, int& summa,
    std::vector<Zombe*>& zombiesToRemove,
    std::vector<Plant*>& plantsToRemove,
    std::vector<Missile*>& missilesToRemove)
{
    std::vector<Missile*> newMissiles;
    for (Entity* e : entities)
    {
        if (e->type != EntityType::PLANT)
        {
            continue;
        }
        Plant* plant = dynamic_cast<Plant*>(e);
        if (plant)
        {
            switch (plant->plantType)
            {
            case PlantType::PEASFIRE:
            {
                PeasfirePlant* Peasfire = dynamic_cast<PeasfirePlant*>(plant);
                Peasfire->ShootCounter += Peasfire->ShootRate;
                int shots = Peasfire->ShootCounter / 10;
                Peasfire->ShootCounter %= 10;
                for (int i = 0; i < shots; ++i)
                {
                    Missile* missile = SpawnMissile(Peasfire, Peasfire->X, Peasfire->Y);
                    newMissiles.push_back(missile);
                }
                break;
            }
            case PlantType::SUNFLOWER:
            {
                SunflowerPlant* SunFlower = dynamic_cast<SunflowerPlant*>(plant);
                summa += SunFlower->SumSalary;
                break;
            }
            case PlantType::CHERRYBOOM:
            {
                CherryboomPlant* cherry = dynamic_cast<CherryboomPlant*>(plant);
                if (!cherry) break;

                int cx = cherry->X;
                int cy = cherry->Y;

                for (Entity* entity : entities)
                {
                    Zombe* zombie = dynamic_cast<Zombe*>(entity);
                    if (!zombie) continue;

                    int zx = zombie->X;
                    int zy = zombie->Y;

                    int xDistance = abs(zx - cx);
                    int yDistance = abs(zy - cy);

                    bool inXRange = (xDistance <= cherry->XradiusDamage);
                    bool inYRange = (yDistance % 2 == 0) && ((yDistance / 2) <= cherry->YradiusDamage);

                    if (inXRange && inYRange)
                    {
                        zombie->GetDamage(cherry->Damage);
                        if (zombie->Health <= 0)
                        {
                            zombiesToRemove.push_back(zombie);
                        }
                    }
                }
                plantsToRemove.push_back(cherry);
                break;
            }
            }
        }
    }
    // Добавляем новые снаряды после обхода всех растений!
    for (Missile* m : newMissiles)
        entities.push_back(m);
}

void SpawnZombiesFromWaves(Level& level, std::vector<Entity*>& entities, int currentTick, int linelength) {
    for (Wave& wave : level.Waves) {
        if (currentTick < wave.StartTick) continue; // Волна ещё не началась

        size_t numLines = level.LinesY.size();

        // Для каждой линии пытаемся заспавнить по одному зомби
        for (size_t lineIdx = 0; lineIdx < numLines; ++lineIdx) {
            if (wave.SpawnIndices[lineIdx] >= wave.ZombiesPerLine[lineIdx].size())
                continue; // Зомби на этой линии закончились

            Zombe* z = wave.ZombiesPerLine[lineIdx][wave.SpawnIndices[lineIdx]];
            wave.SpawnIndices[lineIdx]++;

            if (z) {
                z->X = linelength;          // Спавн справа
                z->Y = level.LinesY[lineIdx]; // Координата линии
                entities.push_back(z);
            }
        }
    }
}



bool AreZombiesAlive(const std::vector<Entity*>& entities) {
    for (Entity* e : entities) {
        if (e->type == EntityType::ZOMBIE) return true;
    }
    return false;
}

bool IsWaveFinished(const Wave& wave) {
    for (size_t i = 0; i < wave.SpawnIndices.size(); ++i) {
        if (wave.SpawnIndices[i] < wave.ZombiesPerLine[i].size())
            return false;
    }
    return true;
}

// Основная функция движка — теперь только вызывает вспомогательные
void Enginefun(std::vector<Entity*>& entities, int& linelength, int& gameover, int& timegame, Level& level, int& summa)
{

    // Проверяем, закончилась ли текущая волна и все зомби убиты
    if (level.CurrentWaveIndex < (int)level.Waves.size())
    {
        Wave& currentWave = level.Waves[level.CurrentWaveIndex];
        bool waveFinished = IsWaveFinished(currentWave) && !AreZombiesAlive(entities);

        if (waveFinished)
        {
            if (level.LastWaveEndTick < 0)
            {
                level.LastWaveEndTick = timegame; // Запоминаем время окончания волны
            }
            else if (timegame - level.LastWaveEndTick >= level.NextWaveDelay)
            {
                level.CurrentWaveIndex++;          // Переходим к следующей волне
                level.LastWaveEndTick = -1;        // Сбрасываем таймер
                //currentWave.Dispose();
            }
        }
        else
        {
            SpawnZombiesFromWaves(level, entities, timegame, linelength);
        }
    }
    else
    {
        if (level.InfiniteMode)
        {
            // Генерируем новую волну с нарастающей сложностью
            Wave newWave;
            newWave.StartTick = timegame;

            const int maxDifficulty = 4;
            int waveNumber = (int)level.Waves.size();

            // Считаем текущий уровень сложности (Mod)
            int difficultyLevel = 1 + (waveNumber / 4);

            if (difficultyLevel > maxDifficulty)
                difficultyLevel = maxDifficulty;

            newWave.Mod = difficultyLevel;

            // Количество зомби растёт после достижения максимума сложности
            int baseCount;
            if (difficultyLevel < maxDifficulty)
            {
                baseCount = 4 + (waveNumber % 4);
            }
            else
            {
                baseCount = 8 + (waveNumber - (maxDifficulty - 1) * 4);
            }

            newWave.GenerateWave(baseCount + 2, baseCount, newWave.Mod, 0, level.LinesY);


            level.Waves.push_back(newWave);
        }
        else
        {
            // Обычный режим — если все волны пройдены и зомби нет, победа
            if (!AreZombiesAlive(entities))
            {
                gameover = 2;
                return;
            }
        }
    }



    std::vector<Missile*> missilesToRemove;
    std::vector<Zombe*> zombiesToRemove;
    std::vector<Plant*> plantsToRemove;

    ProcessMissiles(entities, linelength, missilesToRemove, zombiesToRemove);
    RemoveEntities(entities, missilesToRemove);

    RemoveDeadZombiesFromWaves(level, zombiesToRemove);
    RemoveEntities(entities, zombiesToRemove);

    //RefreshMap(entities);

    ProcessZombies(entities, zombiesToRemove, plantsToRemove, missilesToRemove, gameover);
    
    RemoveEntities(entities, missilesToRemove);

    RemoveDeadZombiesFromWaves(level, zombiesToRemove);
    RemoveEntities(entities, zombiesToRemove);

    RemoveEntities(entities, plantsToRemove);
    
    PlantAct(entities, summa, zombiesToRemove, plantsToRemove, missilesToRemove);

   // RemoveEntities(entities, missilesToRemove);

    RemoveDeadZombiesFromWaves(level, zombiesToRemove);
    RemoveEntities(entities, zombiesToRemove);

    RemoveEntities(entities, plantsToRemove);

    RefreshMap(entities, summa);
}
void Start(int rows, int columns, int house_offset, int FLy, int linelength, std::vector<Entity*> entities, Level level)
{
    // Сохраняем оригинальные настройки консоли
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD originalMode;
    GetConsoleMode(hStdin, &originalMode);

    // Устанавливаем режим консоли для немедленного ввода
    SetConsoleMode(hStdin, ENABLE_PROCESSED_INPUT);


    int gameover;
    RefreshMap(entities, level.balance);
    //
    int timegame = 0;
    for (;;)
    {
        bool existRequest = GetInput(hStdin, originalMode, entities, house_offset, level.LinesY, columns, level.balance, level);
        Enginefun(entities, linelength, gameover, timegame, level, level.balance);
        timegame += 1;
        if (gameover == 1 || existRequest)
        {
            exit(0);
        }
        else if (gameover == 2) {
            exit(100);
        }
    }

}

void Start(int rows, int columns, int house_offset, int FLy, int linelength)
{
    // Сохраняем оригинальные настройки консоли
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD originalMode;
    GetConsoleMode(hStdin, &originalMode);

    // Устанавливаем режим консоли для немедленного ввода
    SetConsoleMode(hStdin, ENABLE_PROCESSED_INPUT);


    int gameover;
    
    Level level;
    level.CreateLvl0();
    level.InfiniteMode = true; // Включаем бесконечный режим
    level.InitLines({ 3, 5, 7 });

    Wave initialWave;
    initialWave.StartTick = 5;
    initialWave.Mod = 2;
    initialWave.GenerateWave(4, 2, initialWave.Mod, 0, level.LinesY);
    level.AddWave(initialWave);
    

    std::vector<Entity*> entities;
    std::vector<int> linesY;
    int timegame;
    for (int i = 0; i < rows; i++)
    {
        Entity* entity = new Home(house_offset, FLy, i+1);
        entities.push_back(entity);
        linesY.push_back(FLy);
        FLy += 2;
    }

    //entities.push_back(new Home(house_offset+5, FLy, 1));
    // 
    entities.push_back(new NutwallPlant(house_offset + 5, FLy - 2));
    entities.push_back(new NutwallPlant(house_offset + 5, FLy - 4));
    entities.push_back(new NutwallPlant(house_offset + 5, FLy - 6));

    entities.push_back(new NutwallPlant(house_offset + 4, FLy - 2));
    entities.push_back(new NutwallPlant(house_offset + 4, FLy - 4));
    entities.push_back(new NutwallPlant(house_offset + 4, FLy - 6));

    entities.push_back(new PeasfirePlant(house_offset + 3, FLy - 2));
    entities.push_back(new PeasfirePlant(house_offset + 3, FLy - 4));
    entities.push_back(new PeasfirePlant(house_offset + 3, FLy - 6));
    // 
    //entities.push_back(new Zombe(linelength - 1, FLy - 2, 30, 30, 12));
    //entities.push_back(new Zombe(linelength - 2, FLy - 2, 1, 1, 10));
    //entities.push_back(new Zombe(linelength - 3, FLy - 2, 1, 1, 10));
    entities.push_back(new ZombeFaster(linelength - 5, FLy - 2));
    entities.push_back(new ZombeFaster(linelength - 4, FLy - 2));
    entities.push_back(new ZombeFaster(linelength - 3, FLy - 2));
    entities.push_back(new ZombeUnstopable(linelength - 3, FLy - 2));
    entities.push_back(new ZombeBoss(linelength - 4, FLy - 2));
    //entities.push_back(new ZombeBoss(linelength - 4, FLy - 2));
    RefreshMap(entities, level.balance);
    //
    timegame = 0;
    for (;;)
    {
        bool existRequest = GetInput(hStdin, originalMode, entities, house_offset, linesY, columns, level.balance, level);
        Enginefun(entities, linelength, gameover, timegame, level, level.balance);
        timegame += 1;
        if (gameover == 1 || existRequest)
        {
            exit(0);
        }
        else if (gameover == 2) {
            exit(100);
        }
        // Основная часть

        // Если снаряд спавнится перед растением, то шаг 1 и шаг 2 меняем местами

        // Сделано Шаг 1: двигаем снаряды (начиная с самого правого)
        // Сделано Шаг 2: Проверяем попал ли снаряд. Если снаряд попал уменьшаем хп
        // Сделано Шаг 3: Убираем мёртвых зомби из вектора
        // Шаг 4: Двигаем зомби, если нужно инициируем атаку
        // Шаг 5: Убираем мёртвые растения
        // Шаг 6: Растения создают снаряды


        // Конец основной части
    }

}