#include <vector>

// ��������� ������������ (enum) ��� ����� ���������
enum class EntityType {
    ZOMBIE,
    PLANT,
    WALL,
    HOME,
    MISSILE,
    END_GAME_ZONE,
    // ��������� ��� �������������
};


// �������� ��������
struct Entity {
    EntityType type;
    // ���������� ���������� � �������
    int X;
    int Y;

    virtual void Handle()
    {
    }
};

struct Home : public Entity {
    Home(int x, int y) {
        this->type = EntityType::HOME;
        this->X = x;
        this->Y = y;
    }

};

struct Zombe : public Entity
{
    int Health;
    int Damage;
    void GetDamage(int damage)
    {

    }
};

// ������ ( � ����� ��� �������, ���� ����� �� ��������, �������)
struct Missile : public Entity {

    int Damage; // ���� ��� ����� ������ ������
    Missile(int damage) : Damage(damage)
    {
    }
    void DoDamage(Zombe* zombe)
    {

    }
};

struct Plant : public Entity {
    int Health;


    void GetDamage(int damage)
    {

    };

    Missile SpawnMissile() {
        Missile MissileSpawned(23);
        return MissileSpawned;
    };

};

struct WallEnd : public Entity {
};




void Start(int rows, int columns, int house_offset, int FLy, int linelength)
{
    static std::vector<Entity*> entities;
    for (int i = 0; i < rows; i++)
    {
        Entity* entity = new Home(house_offset, FLy);
        entities.push_back(entity);
        FLy += 2;
    }



}