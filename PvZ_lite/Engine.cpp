#include <vector>
#include <windows.h>
#include <string>


#include "Entities.h"


void RefreshMap(std::vector<Entity*> entities, int& summa);
bool GetInput(HANDLE hStdin, DWORD originalMode, std::vector<Entity*>& entities, int house_offset, std::vector<int> linesY, int columns, int& summa, Level level);


// ���������� ��������� �� X (����-����� +)
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

// ��������� �������� � ����� ������� ��������
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

// ����� �������� � ���� ����� � -57658.... ���-�� ���. ��� ��������� �� ������� ������, �� ��������� � ����� �� ���� �������, ������� ���� ��� ��� ��� ������� ������������: (����-����� +)
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
    SortEntitiesByX(zombies, true); // ����� �������

    for (Entity* e : zombies) {
        Zombe* zombie = dynamic_cast<Zombe*>(e);
        if (!zombie) continue;

        zombie->SpeedCounter += zombie->Speed;
        int moveSteps = zombie->SpeedCounter / 10;
        zombie->SpeedCounter %= 10;

        if (moveSteps == 0) continue;

        for (int step = 0; step < moveSteps; ++step) {
            int targetX = zombie->X - 1;

            // ��������� ����� �����
            Zombe* leftZombie = nullptr;
            for (Entity* other : entities) {
                if (other->type == EntityType::ZOMBIE && other->Y == zombie->Y && other->X == targetX) {
                    leftZombie = dynamic_cast<Zombe*>(other);
                    break;
                }
            }

            if (leftZombie && leftZombie->Speed < zombie->Speed) {
                // ������� ������������ ����� �����

                int jumpX = leftZombie->X - 1; // ������ �� ����� �����

                // ���������, ��� ������ �� ����� ����� �������� ��� �������� ��������/���/������
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
                        // ����� ������� � ������� ��������
                        zombie->X = jumpX;
                        // ������ ��������
                        attackedOrMoved = true;
                        break;
                    }
                    case EntityType::HOME:
                        gameover = 1;
                        // ����� ������� � ������� ���
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
                                // ����� ����, ������ �������
                                break;
                            }
                            // ����� ������� �� ������ � �������� � �������� ����
                            zombie->X = jumpX;
                            attackedOrMoved = true;
                        }
                        break;
                    }
                    case EntityType::ZOMBIE:
                        // ������ ������� �� ������� ������ ����� � �������� ������
                        canJump = false;
                        break;
                    default:
                        // ���� ���-�� ������ � ������� ������ �������
                        canJump = false;
                        break;
                    }
                    if (!canJump) {
                        // ������ ���������� � �������� ��������
                        break;
                    }
                    if (attackedOrMoved) {
                        // ������ �������� � ��������� � ���������� ����
                        break;
                    }
                }
                else {
                    // ������ �������� � �������
                    zombie->X = jumpX;
                    attackedOrMoved = true;
                    break;
                }

                if (attackedOrMoved) break;
            }
            else {
                // ��� ����� ����� � ������� ��������� � ������� ��������
                // ��������� ����������� � ����� (���������� ������ ����)
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


// �������� ���������� �������� �� ������� entities, ����-�����+
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
    // ���� ������� ����� ��������
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
    // ��������� ����� ������� ����� ������ ���� ��������!
    for (Missile* m : newMissiles)
        entities.push_back(m);
}

void SpawnZombiesFromWaves(Level& level, std::vector<Entity*>& entities, int currentTick, int linelength) {
    for (Wave& wave : level.Waves) {
        if (currentTick < wave.StartTick) continue; // ����� ��� �� ��������

        size_t numLines = level.LinesY.size();

        // ��� ������ ����� �������� ���������� �� ������ �����
        for (size_t lineIdx = 0; lineIdx < numLines; ++lineIdx) {
            if (wave.SpawnIndices[lineIdx] >= wave.ZombiesPerLine[lineIdx].size())
                continue; // ����� �� ���� ����� �����������

            Zombe* z = wave.ZombiesPerLine[lineIdx][wave.SpawnIndices[lineIdx]];
            wave.SpawnIndices[lineIdx]++;

            if (z) {
                z->X = linelength;          // ����� ������
                z->Y = level.LinesY[lineIdx]; // ���������� �����
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

// �������� ������� ������ � ������ ������ �������� ���������������
void Enginefun(std::vector<Entity*>& entities, int& linelength, int& gameover, int& timegame, Level& level, int& summa)
{

    // ���������, ����������� �� ������� ����� � ��� ����� �����
    if (level.CurrentWaveIndex < (int)level.Waves.size())
    {
        Wave& currentWave = level.Waves[level.CurrentWaveIndex];
        bool waveFinished = IsWaveFinished(currentWave) && !AreZombiesAlive(entities);

        if (waveFinished)
        {
            if (level.LastWaveEndTick < 0)
            {
                level.LastWaveEndTick = timegame; // ���������� ����� ��������� �����
            }
            else if (timegame - level.LastWaveEndTick >= level.NextWaveDelay)
            {
                level.CurrentWaveIndex++;          // ��������� � ��������� �����
                level.LastWaveEndTick = -1;        // ���������� ������
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
            // ���������� ����� ����� � ����������� ����������
            Wave newWave;
            newWave.StartTick = timegame;

            const int maxDifficulty = 4;
            int waveNumber = (int)level.Waves.size();

            // ������� ������� ������� ��������� (Mod)
            int difficultyLevel = 1 + (waveNumber / 4);

            if (difficultyLevel > maxDifficulty)
                difficultyLevel = maxDifficulty;

            newWave.Mod = difficultyLevel;

            // ���������� ����� ����� ����� ���������� ��������� ���������
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
            // ������� ����� � ���� ��� ����� �������� � ����� ���, ������
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
    // ��������� ������������ ��������� �������
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD originalMode;
    GetConsoleMode(hStdin, &originalMode);

    // ������������� ����� ������� ��� ������������ �����
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
    // ��������� ������������ ��������� �������
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD originalMode;
    GetConsoleMode(hStdin, &originalMode);

    // ������������� ����� ������� ��� ������������ �����
    SetConsoleMode(hStdin, ENABLE_PROCESSED_INPUT);


    int gameover;
    
    Level level;
    level.CreateLvl0();
    level.InfiniteMode = true; // �������� ����������� �����
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
        // �������� �����

        // ���� ������ ��������� ����� ���������, �� ��� 1 � ��� 2 ������ �������

        // ������� ��� 1: ������� ������� (������� � ������ �������)
        // ������� ��� 2: ��������� ����� �� ������. ���� ������ ����� ��������� ��
        // ������� ��� 3: ������� ������ ����� �� �������
        // ��� 4: ������� �����, ���� ����� ���������� �����
        // ��� 5: ������� ������ ��������
        // ��� 6: �������� ������� �������


        // ����� �������� �����
    }

}