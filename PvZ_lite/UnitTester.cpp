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

// Предполагается, что у вас есть определение Entity и его наследников
// и функция SortEntitiesByX уже объявлена

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
    // Освободить память
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

// Тест 7: все элементы одинаковые
void Test_AllEqual() {
    std::vector<Entity*> entities = { CreateEntity(7), CreateEntity(7), CreateEntity(7) };
    SortEntitiesByX(entities, true);
    AssertSorted(entities, { 7,7,7 });
    for (auto e : entities) delete e;
    std::cout << "Test_AllEqual success.\n";
}

// Тест 8: отрицательные и положительные значения
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

// Тест 9: большие значения X
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

// Тест 10: много элементов (например из диапазона)
void Test_ManyElements() {
    std::vector<Entity*> entities;
    for (int i = 50; i >= 0; --i) {
        entities.push_back(CreateEntity(i));
    }
    // сортируем по возрастанию
    SortEntitiesByX(entities, true);
    for (size_t i = 0; i < entities.size(); ++i) {
        assert(static_cast<int>(i) == static_cast<int>(entities[i]->X));
    }
    for (auto e : entities) delete e;
    std::cout << "Test_ManyElements success.\n";
}

// Вспомогательные функции для создания объектов
Entity* CreateEntity(EntityType type, int x, int y) {
    Entity* e = nullptr;
    if (type == EntityType::MISSILE) {
        auto* m = new Missile();
        m->type = type;
        m->X = x;
        m->Y = y;
        m->Speed = 10; // например
        m->SpeedCounter = 0;
        m->Damage = 50; // пример урона
        e = m;
    }
    else if (type == EntityType::ZOMBIE) {
        auto* z = new Zombe();
        z->type = type;
        z->X = x;
        z->Y = y;
        z->Health = 3; // пример здоровья
        e = z;
    }
    return e;
}

// Тест 1: ракета попадает в зомби и убивает его
void Test_MissileHitsZombie() {
    std::vector<Entity*> entities;
    std::vector<Missile*> missilesToRemove;
    std::vector<Zombe*> zombiesToRemove;

    // Создаем зомби на позиции X=15, Y=5
    Zombe* zombie = static_cast<Zombe*>(CreateEntity(EntityType::ZOMBIE, 12, 5));
    entities.push_back(zombie);

    // Создаем ракету на позиции X=5, Y=5 с скоростью 10 (один шаг)
    Missile* missile = static_cast<Missile*>(CreateEntity(EntityType::MISSILE, 5, 5));
    missile->Speed = 100;
    missile->SpeedCounter = 0;
    missile->Damage = 50;
    entities.push_back(missile);

    int lineLength = 20;

    ProcessMissiles(entities, lineLength, missilesToRemove, zombiesToRemove);

    // Проверяем что зомби получил урон и умер
    assert(zombie->Health <= 0);
    assert(std::find(zombiesToRemove.begin(), zombiesToRemove.end(), zombie) != zombiesToRemove.end());
    // Ракета должна быть удалена
    assert(std::find(missilesToRemove.begin(), missilesToRemove.end(), missile) != missilesToRemove.end());

    // Освобождение памяти
    delete zombie;
    delete missile;

    std::cout << "Test_MissileHitsZombie success.\n";
}

// Тест 2: ракета не попадает в зомби и движется дальше
void Test_MissileNoHit() {
    std::vector<Entity*> entities;
    std::vector<Missile*> missilesToRemove;
    std::vector<Zombe*> zombiesToRemove;

    // Зомби вне диапазона X=50
    Zombe* zombie = static_cast<Zombe*>(CreateEntity(EntityType::ZOMBIE, 50, 5));
    entities.push_back(zombie);

    // Ракета на позиции X=5,Y=5 с скоростью 10
    Missile* missile = static_cast<Missile*>(CreateEntity(EntityType::MISSILE, 5, 5));
    missile->Speed = 100;
    missile->SpeedCounter = 0;
    entities.push_back(missile);

    int lineLength = 60;

    ProcessMissiles(entities, lineLength, missilesToRemove, zombiesToRemove);

    // Ракета должна сместиться на X=15
    assert(missile->X == 15);

    // Зомби остался живым
    assert(zombie->Health == 3);

    // Ракета не удалена (она еще в игре)
    assert(std::find(missilesToRemove.begin(), missilesToRemove.end(), missile) == missilesToRemove.end());

    delete zombie;
    delete missile;

    std::cout << "Test_MissileNoHit success.\n";
}

// Тест 3: ракета выходит за границы линии и удаляется
void Test_MissileExitsLine() {
    std::vector<Entity*> entities;
    std::vector<Missile*> missilesToRemove;
    std::vector<Zombe*> zombiesToRemove;

    // Создаем ракету близко к границе линии
    Missile* missile = static_cast<Missile*>(CreateEntity(EntityType::MISSILE, 95, 5));
    missile->Speed = 100;
    missile->SpeedCounter = 0;
    entities.push_back(missile);

    int lineLength = 100;

    ProcessMissiles(entities, lineLength, missilesToRemove, zombiesToRemove);

    // Ракета должна быть удалена за пределами линии
    assert(std::find(missilesToRemove.begin(), missilesToRemove.end(), missile) != missilesToRemove.end());

    delete missile;

    std::cout << "Test_MissileExitsLine success.\n";
}

Zombe* CreateZombe(int id) {
    Zombe* z = new Zombe();
    return z;
}

// Тест 1: Удаление одного зомби из одной линии
void Test_RemoveSingleZombie() {
    // Создаем зомби
    Zombe* zombie1 = CreateZombe(1);
    Zombe* zombie2 = CreateZombe(2);

    // Создаем волну с двумя линиями
    Wave wave;
    wave.ZombiesPerLine = {
        {zombie1, zombie2},
        {CreateZombe(3), CreateZombe(4)}
    };

    // Создаем уровень с одной волной
    Level level;
    level.Waves.push_back(wave);

    // Удаляем zombie1
    std::vector<Zombe*> toRemove = { zombie1 };

    RemoveDeadZombiesFromWaves(level, toRemove);

    // Проверяем, что zombie1 удален из первой линии
    auto& line0 = level.Waves[0].ZombiesPerLine[0];
    assert(std::find(line0.begin(), line0.end(), zombie1) == line0.end());
    // Другие зомби остались
    assert(std::find(line0.begin(), line0.end(), zombie2) != line0.end());

    // Освобождение памяти
    delete zombie1;
    delete zombie2;
    for (auto* z : level.Waves[0].ZombiesPerLine[1]) {
        delete z;
    }

    std::cout << "Test_RemoveSingleZombie success.\n";
}

// Тест 2: Удаление нескольких зомби из разных линий
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

    // Проверка удаления
    auto& line0 = level.Waves[0].ZombiesPerLine[0];
    auto& line1 = level.Waves[0].ZombiesPerLine[1];

    assert(std::find(line0.begin(), line0.end(), z2) == line0.end());
    assert(std::find(line1.begin(), line1.end(), z3) == line1.end());

    // Остальные зомби остались
    assert(std::find(line0.begin(), line0.end(), z1) != line0.end());

    // Освобождение памяти
    delete z1; delete z2; delete z3;

    std::cout << "Test_RemoveMultipleZombies success.\n";
}

// Тест 3: Удаление зомби, которых нет в волнах (ничего не меняется)
void Test_RemoveNonexistentZombie() {
    Zombe* existingZombie = CreateZombe(10);
    Zombe* nonExistentZombie = CreateZombe(99); // не в волне

    Wave wave;
    wave.ZombiesPerLine = {
        {existingZombie}
    };

    Level level;
    level.Waves.push_back(wave);

    std::vector<Zombe*> toRemove = { nonExistentZombie };

    RemoveDeadZombiesFromWaves(level, toRemove);

    // Зомби в волне остался без изменений
    auto& line = level.Waves[0].ZombiesPerLine[0];
    assert(std::find(line.begin(), line.end(), existingZombie) != line.end());

    // Освобождение памяти
    delete existingZombie;
    delete nonExistentZombie;

    std::cout << "Test_RemoveNonexistentZombie success.\n";
}

void Test_ProcessZombies_SimpleMove() {
    // Создаем зомби
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

    // Зомби должен переместиться на 2 клетки влево (Speed=2)
    assert(z1->X == 3);
    assert(gameover == 0);

    std::cout << "Test_ProcessZombies_SimpleMove success.\n";

    delete z1;
}

void Test_ProcessZombies_AttackPlant() {
    Zombe* z1 = new Zombe(2, 5);
    Entity* zombieEntity = z1;
    zombieEntity->type = EntityType::ZOMBIE;

    Plant* plant = new PeasfirePlant(1, 5); // На клетке X=4
    plant->type = EntityType::PLANT;

    std::vector<Entity*> entities = { zombieEntity, plant };
    std::vector<Zombe*> zombiesToRemove;
    std::vector<Plant*> plantsToRemove;
    std::vector<Missile*> missilesToRemove;
    int gameover = 0;

    ProcessZombies(entities, zombiesToRemove, plantsToRemove, missilesToRemove, gameover);

    // Зомби должен атаковать растение (уменьшение здоровья)
    assert(plant->Health == 1);
    // Зомби переместился на клетку X=4 (атаковал)
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

    // При столкновении с домом игра должна завершиться
    assert(gameover == 1);

    delete z1;

    std::cout << "Test_ProcessZombies_GameOver success.\n";
}

void Test_SpawnMissile_PeasfirePlant() {
    // Создаем растение типа PEASFIRE
    PeasfirePlant* plant = new PeasfirePlant(1, 1); // примерные параметры
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
    // Создаем обычное растение другого типа
    Plant* plant = new SunflowerPlant(1, 1);
    plant->plantType = PlantType::SUNFLOWER; // предполагается другой тип

    int spawnX = 5;
    int spawnY = 5;

    Missile* missile = SpawnMissile(plant, spawnX, spawnY);

    // Ожидается, что снаряд не создастся
    assert(missile == nullptr);

    delete plant;

    std::cout << "Test_SpawnMissile_NonPeasfirePlant success.\n";
}

void Test_SpawnMissile_InvalidCast() {
    // Создаем растение типа PEASFIRE, но не объект PeasfirePlant (например, базовый класс)
    Plant* plant = new NutwallPlant(1, 1);
    plant->plantType = PlantType::PEASFIRE; // тип совпадает

    int spawnX = 8;
    int spawnY = 8;

    Missile* missile = SpawnMissile(plant, spawnX, spawnY);

    // Так как dynamic_cast не выполнится успешно (не объект PeasfirePlant), возвращается nullptr
    assert(missile == nullptr);

    delete plant;

    std::cout << "Test_SpawnMissile_InvalidCast success.\n";
}

void Test_PlantAct_PeasfireShooting() {
    std::vector<Entity*> entities;

    auto* pea = new PeasfirePlant(0, 0);
    pea->ShootRate = 2; // каждый вызов добавляет 2 к счетчику
    entities.push_back(pea);

    std::vector<Zombe*> zombiesToRemove;
    std::vector<Plant*> plantsToRemove;
    std::vector<Missile*> missilesToRemove;

    int summa = 0;

    // Первый вызов: счетчик станет 2 -> не стреляет
    PlantAct(entities, summa, zombiesToRemove, plantsToRemove, missilesToRemove);

    // Второй вызов: счетчик станет 4 -> не стреляет
    PlantAct(entities, summa, zombiesToRemove, plantsToRemove, missilesToRemove);

    // Третий вызов: счетчик станет 6 -> стреляет один раз
    PlantAct(entities, summa, zombiesToRemove, plantsToRemove, missilesToRemove);

    assert(entities.size() == 2); // растение + один снаряд
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
        delete e; // очистка памяти
    }

    std::cout << "Test_PlantAct_PeasfireShooting success.\n";
}

// Тестовая функция: проверка поражения зомби ЧЕРРИБУМом
void Test_PlantAct_CherryboomKillsZombies() {
    std::vector<Entity*> entities;

    auto* cherry = new CherryboomPlant(5, 5);
    entities.push_back(cherry);

    auto* zombie1 = new Zombe(4, 4); // в радиусе по X и Y
    auto* zombie2 = new Zombe(7, 5); // вне радиуса по X
    auto* zombie3 = new Zombe(5, 8); // вне радиуса по Y

    entities.push_back(zombie1);
    entities.push_back(zombie2);
    entities.push_back(zombie3);

    std::vector<Zombe*> zombiesToRemove;
    std::vector<Plant*> plantsToRemove;
    std::vector<Missile*> missilesToRemove;

    int sum = 0;

    PlantAct(entities, sum, zombiesToRemove, plantsToRemove, missilesToRemove);

    // zombie1 должен получить урон и умереть
    assert(zombie1->Health <= 20);

    // zombie2 и zombie3 не должны получить урон
    assert(zombie2->Health == 50 && zombie3->Health == 50);

    // растение должно быть удалено из списка растений
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

// Тестовая функция: увеличение суммы для SUNFLOWER
void Test_PlantAct_SunflowerSum() {
    std::vector<Entity*> entities;

    auto* sunflower = new SunflowerPlant(0, 0);
    entities.push_back(sunflower);

    std::vector<Zombe*> zombiesToRemove;
    std::vector<Plant*> plantsToRemove;
    std::vector<Missile*> missilesToRemove;

    int sum = 0;

    PlantAct(entities, sum, zombiesToRemove, plantsToRemove, missilesToRemove);

    assert(sum == 100); // сумма увеличилась на зарплату

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

    std::cout << "Все тесты прошли успешно!" << std::endl;
    return 0;
}