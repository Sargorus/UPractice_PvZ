#include <vector>
#include <cassert>
#include <iostream>
#include <algorithm>

#include "Entities.h"

void SortEntitiesByX(std::vector<Entity*>& entities, bool ascending);

void ProcessMissiles(std::vector<Entity*>& entities, int linelength,
    std::vector<Missile*>& missilesToRemove,
    std::vector<Zombe*>& zombiesToRemove);

void RemoveDeadZombiesFromWaves(Level& level, const std::vector<Zombe*>& zombiesToRemove);

void ProcessZombies(std::vector<Entity*>& entities,
    std::vector<Zombe*>& zombiesToRemove,
    std::vector<Plant*>& plantsToRemove,
    std::vector<Missile*>& missilesToRemove,
    int& gameover);

template<typename T>
void RemoveEntities(std::vector<Entity*>& entities, std::vector<T*>& toRemove);

Missile* SpawnMissile(Plant* plant, int x, int y);

void PlantAct(std::vector<Entity*>& entities, int& summa,
    std::vector<Zombe*>& zombiesToRemove,
    std::vector<Plant*>& plantsToRemove,
    std::vector<Missile*>& missilesToRemove);

bool AreZombiesAlive(const std::vector<Entity*>& entities);
bool IsWaveFinished(const Wave& wave);

// ��������������, ��� � ��� ���� ����������� Entity � ��� �����������
// � ������� SortEntitiesByX ��� ���������

Entity* CreateEntity(int x) {
    Entity* e = new Entity();
    e->X = x;
    return e;
}

void AssertSorted(const std::vector<Entity*>& entities, const std::vector<int>& expected) {
    assert(entities.size() == expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        assert(entities[i]->X == expected[i]);
    }
}

void Test_EmptyVector() {
    std::vector<Entity*> entities;
    SortEntitiesByX(entities, true);
    assert(entities.empty());
    std::cout << "Test_EmptyVector success.\n";
}

void Test_SingleElement() {
    std::vector<Entity*> entities = { CreateEntity(5) };
    SortEntitiesByX(entities, true);
    AssertSorted(entities, { 5 });
    // ���������� ������
    delete entities[0];
    std::cout << "Test_SingleElement success.\n";
}

void Test_AlreadySortedAscending() {
    std::vector<Entity*> entities = { CreateEntity(1), CreateEntity(2), CreateEntity(3) };
    SortEntitiesByX(entities, true);
    AssertSorted(entities, { 1, 2, 3 });
    for (auto e : entities) delete e;
    std::cout << "Test_AlreadySortedAscending success.\n";
}

void Test_AlreadySortedDescending() {
    std::vector<Entity*> entities = { CreateEntity(3), CreateEntity(2), CreateEntity(1) };
    SortEntitiesByX(entities, true);
    AssertSorted(entities, { 1, 2, 3 });
    for (auto e : entities) delete e;
    std::cout << "Test_AlreadySortedDescending success.\n";
}

void Test_RandomOrderWithDuplicates() {
    std::vector<Entity*> entities = {
        CreateEntity(4),
        CreateEntity(2),
        CreateEntity(4),
        CreateEntity(1),
        CreateEntity(3)
    };
    SortEntitiesByX(entities, true);
    AssertSorted(entities, { 1, 2, 3, 4, 4 });
    for (auto e : entities) delete e;
    std::cout << "Test_RandomOrderWithDuplicates success.\n";
}

void Test_SortDescending() {
    std::vector<Entity*> entities = { CreateEntity(5), CreateEntity(1), CreateEntity(3) };
    SortEntitiesByX(entities, false);
    AssertSorted(entities, { 5, 3, 1 });
    for (auto e : entities) delete e;
    std::cout << "Test_SortDescending success.\n";
}

// ���� 7: ��� �������� ����������
void Test_AllEqual() {
    std::vector<Entity*> entities = { CreateEntity(7), CreateEntity(7), CreateEntity(7) };
    SortEntitiesByX(entities, true);
    AssertSorted(entities, { 7,7,7 });
    for (auto e : entities) delete e;
    std::cout << "Test_AllEqual success.\n";
}

// ���� 8: ������������� � ������������� ��������
void Test_NegativeAndPositive() {
    std::vector<Entity*> entities = {
        CreateEntity(-2),
        CreateEntity(0),
        CreateEntity(-5),
        CreateEntity(3)
    };
    // ascending
    SortEntitiesByX(entities, true);
    AssertSorted(entities, { -5,-2,0,3 });

    // descending
    SortEntitiesByX(entities, false);
    AssertSorted(entities, { 3,0,-2,-5 });

    for (auto e : entities) delete e;
    std::cout << "Test_NegativeAndPositive success.\n";
}

// ���� 9: ������� �������� X
void Test_LargeValues() {
    std::vector<Entity*> entities = {
        CreateEntity(INT_MAX),
        CreateEntity(INT_MIN),
        CreateEntity(INT_MAX - 1)
    };

    // ascending
    SortEntitiesByX(entities, true);
    AssertSorted(entities, { INT_MIN, INT_MAX - 1 , INT_MAX });

    // descending
    SortEntitiesByX(entities, false);
    AssertSorted(entities, { INT_MAX , INT_MAX - 1 , INT_MIN });

    for (auto e : entities) delete e;
    std::cout << "Test_LargeValues success.\n";
}

// ���� 10: ����� ��������� (�������� �� ���������)
void Test_ManyElements() {
    std::vector<Entity*> entities;
    for (int i = 50; i >= 0; --i) {
        entities.push_back(CreateEntity(i));
    }
    // ��������� �� �����������
    SortEntitiesByX(entities, true);
    for (size_t i = 0; i < entities.size(); ++i) {
        assert(static_cast<int>(i) == static_cast<int>(entities[i]->X));
    }
    for (auto e : entities) delete e;
    std::cout << "Test_ManyElements success.\n";
}

// ��������������� ������� ��� �������� ��������
Entity* CreateEntity(EntityType type, int x, int y) {
    Entity* e = nullptr;
    if (type == EntityType::MISSILE) {
        auto* m = new Missile();
        m->type = type;
        m->X = x;
        m->Y = y;
        m->Speed = 10; // ��������
        m->SpeedCounter = 0;
        m->Damage = 50; // ������ �����
        e = m;
    }
    else if (type == EntityType::ZOMBIE) {
        auto* z = new Zombe();
        z->type = type;
        z->X = x;
        z->Y = y;
        z->Health = 3; // ������ ��������
        e = z;
    }
    return e;
}

// ���� 1: ������ �������� � ����� � ������� ���
void Test_MissileHitsZombie() {
    std::vector<Entity*> entities;
    std::vector<Missile*> missilesToRemove;
    std::vector<Zombe*> zombiesToRemove;

    // ������� ����� �� ������� X=15, Y=5
    Zombe* zombie = static_cast<Zombe*>(CreateEntity(EntityType::ZOMBIE, 12, 5));
    entities.push_back(zombie);

    // ������� ������ �� ������� X=5, Y=5 � ��������� 10 (���� ���)
    Missile* missile = static_cast<Missile*>(CreateEntity(EntityType::MISSILE, 5, 5));
    missile->Speed = 100;
    missile->SpeedCounter = 0;
    missile->Damage = 50;
    entities.push_back(missile);

    int lineLength = 20;

    ProcessMissiles(entities, lineLength, missilesToRemove, zombiesToRemove);

    // ��������� ��� ����� ������� ���� � ����
    assert(zombie->Health <= 0);
    assert(std::find(zombiesToRemove.begin(), zombiesToRemove.end(), zombie) != zombiesToRemove.end());
    // ������ ������ ���� �������
    assert(std::find(missilesToRemove.begin(), missilesToRemove.end(), missile) != missilesToRemove.end());

    // ������������ ������
    delete zombie;
    delete missile;

    std::cout << "Test_MissileHitsZombie success.\n";
}

// ���� 2: ������ �� �������� � ����� � �������� ������
void Test_MissileNoHit() {
    std::vector<Entity*> entities;
    std::vector<Missile*> missilesToRemove;
    std::vector<Zombe*> zombiesToRemove;

    // ����� ��� ��������� X=50
    Zombe* zombie = static_cast<Zombe*>(CreateEntity(EntityType::ZOMBIE, 50, 5));
    entities.push_back(zombie);

    // ������ �� ������� X=5,Y=5 � ��������� 10
    Missile* missile = static_cast<Missile*>(CreateEntity(EntityType::MISSILE, 5, 5));
    missile->Speed = 100;
    missile->SpeedCounter = 0;
    entities.push_back(missile);

    int lineLength = 60;

    ProcessMissiles(entities, lineLength, missilesToRemove, zombiesToRemove);

    // ������ ������ ���������� �� X=15
    assert(missile->X == 15);

    // ����� ������� �����
    assert(zombie->Health == 3);

    // ������ �� ������� (��� ��� � ����)
    assert(std::find(missilesToRemove.begin(), missilesToRemove.end(), missile) == missilesToRemove.end());

    delete zombie;
    delete missile;

    std::cout << "Test_MissileNoHit success.\n";
}

// ���� 3: ������ ������� �� ������� ����� � ���������
void Test_MissileExitsLine() {
    std::vector<Entity*> entities;
    std::vector<Missile*> missilesToRemove;
    std::vector<Zombe*> zombiesToRemove;

    // ������� ������ ������ � ������� �����
    Missile* missile = static_cast<Missile*>(CreateEntity(EntityType::MISSILE, 95, 5));
    missile->Speed = 100;
    missile->SpeedCounter = 0;
    entities.push_back(missile);

    int lineLength = 100;

    ProcessMissiles(entities, lineLength, missilesToRemove, zombiesToRemove);

    // ������ ������ ���� ������� �� ��������� �����
    assert(std::find(missilesToRemove.begin(), missilesToRemove.end(), missile) != missilesToRemove.end());

    delete missile;

    std::cout << "Test_MissileExitsLine success.\n";
}

Zombe* CreateZombe(int id) {
    Zombe* z = new Zombe();
    return z;
}

// ���� 1: �������� ������ ����� �� ����� �����
void Test_RemoveSingleZombie() {
    // ������� �����
    Zombe* zombie1 = CreateZombe(1);
    Zombe* zombie2 = CreateZombe(2);

    // ������� ����� � ����� �������
    Wave wave;
    wave.ZombiesPerLine = {
        {zombie1, zombie2},
        {CreateZombe(3), CreateZombe(4)}
    };

    // ������� ������� � ����� ������
    Level level;
    level.Waves.push_back(wave);

    // ������� zombie1
    std::vector<Zombe*> toRemove = { zombie1 };

    RemoveDeadZombiesFromWaves(level, toRemove);

    // ���������, ��� zombie1 ������ �� ������ �����
    auto& line0 = level.Waves[0].ZombiesPerLine[0];
    assert(std::find(line0.begin(), line0.end(), zombie1) == line0.end());
    // ������ ����� ��������
    assert(std::find(line0.begin(), line0.end(), zombie2) != line0.end());

    // ������������ ������
    delete zombie1;
    delete zombie2;
    for (auto* z : level.Waves[0].ZombiesPerLine[1]) {
        delete z;
    }

    std::cout << "Test_RemoveSingleZombie success.\n";
}

// ���� 2: �������� ���������� ����� �� ������ �����
void Test_RemoveMultipleZombies() {
    Zombe* z1 = CreateZombe(1);
    Zombe* z2 = CreateZombe(2);
    Zombe* z3 = CreateZombe(3);

    Wave wave;
    wave.ZombiesPerLine = {
        {z1, z2},
        {z3}
    };

    Level level;
    level.Waves.push_back(wave);

    std::vector<Zombe*> toRemove = { z2, z3 };

    RemoveDeadZombiesFromWaves(level, toRemove);

    // �������� ��������
    auto& line0 = level.Waves[0].ZombiesPerLine[0];
    auto& line1 = level.Waves[0].ZombiesPerLine[1];

    assert(std::find(line0.begin(), line0.end(), z2) == line0.end());
    assert(std::find(line1.begin(), line1.end(), z3) == line1.end());

    // ��������� ����� ��������
    assert(std::find(line0.begin(), line0.end(), z1) != line0.end());

    // ������������ ������
    delete z1; delete z2; delete z3;

    std::cout << "Test_RemoveMultipleZombies success.\n";
}

// ���� 3: �������� �����, ������� ��� � ������ (������ �� ��������)
void Test_RemoveNonexistentZombie() {
    Zombe* existingZombie = CreateZombe(10);
    Zombe* nonExistentZombie = CreateZombe(99); // �� � �����

    Wave wave;
    wave.ZombiesPerLine = {
        {existingZombie}
    };

    Level level;
    level.Waves.push_back(wave);

    std::vector<Zombe*> toRemove = { nonExistentZombie };

    RemoveDeadZombiesFromWaves(level, toRemove);

    // ����� � ����� ������� ��� ���������
    auto& line = level.Waves[0].ZombiesPerLine[0];
    assert(std::find(line.begin(), line.end(), existingZombie) != line.end());

    // ������������ ������
    delete existingZombie;
    delete nonExistentZombie;

    std::cout << "Test_RemoveNonexistentZombie success.\n";
}

void Test_ProcessZombies_SimpleMove() {
    // ������� �����
    Zombe* z1 = new Zombe(5, 5); // X=5, Speed=2, SpeedCounter=0
    z1->Speed = 20;
    Entity* e1 = z1;
    e1->type = EntityType::ZOMBIE;

    std::vector<Entity*> entities = { e1 };
    std::vector<Zombe*> zombiesToRemove;
    std::vector<Plant*> plantsToRemove;
    std::vector<Missile*> missilesToRemove;
    int gameover = 0;

    ProcessZombies(entities, zombiesToRemove, plantsToRemove, missilesToRemove, gameover);

    // ����� ������ ������������� �� 2 ������ ����� (Speed=2)
    assert(z1->X == 3);
    assert(gameover == 0);

    std::cout << "Test_ProcessZombies_SimpleMove success.\n";

    delete z1;
}

void Test_ProcessZombies_AttackPlant() {
    Zombe* z1 = new Zombe(2, 5);
    Entity* zombieEntity = z1;
    zombieEntity->type = EntityType::ZOMBIE;

    Plant* plant = new PeasfirePlant(1, 5); // �� ������ X=4
    plant->type = EntityType::PLANT;

    std::vector<Entity*> entities = { zombieEntity, plant };
    std::vector<Zombe*> zombiesToRemove;
    std::vector<Plant*> plantsToRemove;
    std::vector<Missile*> missilesToRemove;
    int gameover = 0;

    ProcessZombies(entities, zombiesToRemove, plantsToRemove, missilesToRemove, gameover);

    // ����� ������ ��������� �������� (���������� ��������)
    assert(plant->Health == 1);
    // ����� ������������ �� ������ X=4 (��������)
    assert(z1->X == 2);
    assert(gameover == 0);

    delete z1;
    delete plant;

    std::cout << "Test_ProcessZombies_AttackPlant success.\n";
}

void Test_ProcessZombies_GameOver() {
    Zombe* z1 = new Zombe(1, 5);
    Entity* zombieEntity = z1;
    zombieEntity->type = EntityType::ZOMBIE;

    Entity* home = new Home(0, 5, 1);
    home->type = EntityType::HOME;

    std::vector<Entity*> entities = { zombieEntity, home };
    std::vector<Zombe*> zombiesToRemove;
    std::vector<Plant*> plantsToRemove;
    std::vector<Missile*> missilesToRemove;

    int gameover = 0;

    ProcessZombies(entities, zombiesToRemove, plantsToRemove, missilesToRemove, gameover);

    // ��� ������������ � ����� ���� ������ �����������
    assert(gameover == 1);

    delete z1;

    std::cout << "Test_ProcessZombies_GameOver success.\n";
}

void Test_SpawnMissile_PeasfirePlant() {
    // ������� �������� ���� PEASFIRE
    PeasfirePlant* plant = new PeasfirePlant(1, 1); // ��������� ���������
    plant->plantType = PlantType::PEASFIRE;
    plant->MissileDamage = 20;
    plant->MissileSpeed = 5;

    int spawnX = 10;
    int spawnY = 15;

    Missile* missile = SpawnMissile(plant, spawnX, spawnY);

    assert(missile != nullptr);
    assert(missile->X == spawnX);
    assert(missile->Y == spawnY);
    assert(missile->Damage == 20);
    assert(missile->Speed == 5);

    delete missile;
    delete plant;

    std::cout << "Test_SpawnMissile_PeasfirePlant success.\n";
}

void Test_SpawnMissile_NonPeasfirePlant() {
    // ������� ������� �������� ������� ����
    Plant* plant = new SunflowerPlant(1, 1);
    plant->plantType = PlantType::SUNFLOWER; // �������������� ������ ���

    int spawnX = 5;
    int spawnY = 5;

    Missile* missile = SpawnMissile(plant, spawnX, spawnY);

    // ���������, ��� ������ �� ���������
    assert(missile == nullptr);

    delete plant;

    std::cout << "Test_SpawnMissile_NonPeasfirePlant success.\n";
}

void Test_SpawnMissile_InvalidCast() {
    // ������� �������� ���� PEASFIRE, �� �� ������ PeasfirePlant (��������, ������� �����)
    Plant* plant = new NutwallPlant(1, 1);
    plant->plantType = PlantType::PEASFIRE; // ��� ���������

    int spawnX = 8;
    int spawnY = 8;

    Missile* missile = SpawnMissile(plant, spawnX, spawnY);

    // ��� ��� dynamic_cast �� ���������� ������� (�� ������ PeasfirePlant), ������������ nullptr
    assert(missile == nullptr);

    delete plant;

    std::cout << "Test_SpawnMissile_InvalidCast success.\n";
}

void Test_PlantAct_PeasfireShooting() {
    std::vector<Entity*> entities;

    auto* pea = new PeasfirePlant(0, 0);
    pea->ShootRate = 2; // ������ ����� ��������� 2 � ��������
    entities.push_back(pea);

    std::vector<Zombe*> zombiesToRemove;
    std::vector<Plant*> plantsToRemove;
    std::vector<Missile*> missilesToRemove;

    int summa = 0;

    // ������ �����: ������� ������ 2 -> �� ��������
    PlantAct(entities, summa, zombiesToRemove, plantsToRemove, missilesToRemove);

    // ������ �����: ������� ������ 4 -> �� ��������
    PlantAct(entities, summa, zombiesToRemove, plantsToRemove, missilesToRemove);

    // ������ �����: ������� ������ 6 -> �������� ���� ���
    PlantAct(entities, summa, zombiesToRemove, plantsToRemove, missilesToRemove);

    assert(entities.size() == 2); // �������� + ���� ������
    bool missileFound = false;

    for (auto* e : entities) {
        if (e != pea && e != nullptr) {
            missileFound = true;
            auto* m = dynamic_cast<Missile*>(e);
            assert(m != nullptr);
            assert(m->X == 0 && m->Y == 0);
            assert(m->Damage == 10 && m->Speed == 3);
        }
    }

    assert(missileFound);

    delete pea;

    for (auto* e : entities) {
        delete e; // ������� ������
    }

    std::cout << "Test_PlantAct_PeasfireShooting success.\n";
}

// �������� �������: �������� ��������� ����� ����������
void Test_PlantAct_CherryboomKillsZombies() {
    std::vector<Entity*> entities;

    auto* cherry = new CherryboomPlant(5, 5);
    entities.push_back(cherry);

    auto* zombie1 = new Zombe(4, 4); // � ������� �� X � Y
    auto* zombie2 = new Zombe(7, 5); // ��� ������� �� X
    auto* zombie3 = new Zombe(5, 8); // ��� ������� �� Y

    entities.push_back(zombie1);
    entities.push_back(zombie2);
    entities.push_back(zombie3);

    std::vector<Zombe*> zombiesToRemove;
    std::vector<Plant*> plantsToRemove;
    std::vector<Missile*> missilesToRemove;

    int sum = 0;

    PlantAct(entities, sum, zombiesToRemove, plantsToRemove, missilesToRemove);

    // zombie1 ������ �������� ���� � �������
    assert(zombie1->Health <= 20);

    // zombie2 � zombie3 �� ������ �������� ����
    assert(zombie2->Health == 50 && zombie3->Health == 50);

    // �������� ������ ���� ������� �� ������ ��������
    bool cherryRemoved = false;
    for (auto* p : plantsToRemove) {
        auto* pCherry = dynamic_cast<CherryboomPlant*>(p);
        if (pCherry && pCherry == cherry) { cherryRemoved = true; }
        delete p;
    }

    assert(cherryRemoved);

    delete zombie1; delete zombie2; delete zombie3;

    std::cout << "Test_PlantAct_CherryboomKillsZombies success.\n";
}

// �������� �������: ���������� ����� ��� SUNFLOWER
void Test_PlantAct_SunflowerSum() {
    std::vector<Entity*> entities;

    auto* sunflower = new SunflowerPlant(0, 0);
    entities.push_back(sunflower);

    std::vector<Zombe*> zombiesToRemove;
    std::vector<Plant*> plantsToRemove;
    std::vector<Missile*> missilesToRemove;

    int sum = 0;

    PlantAct(entities, sum, zombiesToRemove, plantsToRemove, missilesToRemove);

    assert(sum == 100); // ����� ����������� �� ��������

    delete sunflower;

    std::cout << "Test_PlantAct_SunflowerSum success.\n";

}

int TEST_START() {
    Test_EmptyVector();
    Test_SingleElement();
    Test_AlreadySortedAscending();
    Test_AlreadySortedDescending();
    Test_RandomOrderWithDuplicates();
    Test_SortDescending();
    Test_AllEqual();
    Test_NegativeAndPositive();
    Test_LargeValues();
    Test_ManyElements();

    Test_MissileHitsZombie();
    Test_MissileNoHit();
    Test_MissileExitsLine();
   
    Test_RemoveSingleZombie();
    Test_RemoveMultipleZombies();
    Test_RemoveNonexistentZombie();

    Test_ProcessZombies_SimpleMove();
    Test_ProcessZombies_AttackPlant();
    Test_ProcessZombies_GameOver();

    Test_SpawnMissile_PeasfirePlant();
    Test_SpawnMissile_NonPeasfirePlant();
    Test_SpawnMissile_InvalidCast();

    std::cout << "��� ����� ������ �������!" << std::endl;
    return 0;
}