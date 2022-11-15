#include "pch.h"
#include "message/monster_generated.h"

int main()
{
    std::cout << "Hello Client!\n";

    flatbuffers::FlatBufferBuilder builder;

    auto weapon_one_name = builder.CreateString("Sword");
    short weapon_one_damage = 3;

    auto weapon_two_name = builder.CreateString("Axe");
    short weapon_two_damage = 5;

    // Use the `CreateWeapon` shortcut to create Weapons with all fields set.
    auto sword = MyGame::Sample::CreateWeapon(builder, weapon_one_name, weapon_one_damage);
    auto axe = MyGame::Sample::CreateWeapon(builder, weapon_two_name, weapon_two_damage);

    // Create a FlatBuffer's `vector` from the `std::vector`.
    std::vector<flatbuffers::Offset<MyGame::Sample::Weapon>> weapons_vector;
    weapons_vector.push_back(sword);
    weapons_vector.push_back(axe);
    auto weapons = builder.CreateVector(weapons_vector);

    // Second, serialize the rest of the objects needed by the Monster.
    auto position = MyGame::Sample::Vec3(1.0f, 2.0f, 3.0f);

    auto name = builder.CreateString("MyMonster");

    unsigned char inv_data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    auto inventory = builder.CreateVector(inv_data, 10);

    // Shortcut for creating monster with all fields set:
    auto orc = CreateMonster(builder, &position, 150, 80, name, inventory,
        MyGame::Sample::Color_Red, weapons, MyGame::Sample::Equipment_Weapon, axe.Union());

    builder.Finish(orc);  // Serialize the root of the object.

    builder.GetBufferPointer();
    builder.GetSize();
}
