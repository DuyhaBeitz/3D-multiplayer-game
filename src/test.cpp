#include "Game.hpp"

void PrintTest(bool test, std::string name) {
    std::cout << name << std::endl;
    if (test) {
        std::cout << "\t" << "PASSED +" << std::endl;
    }
    else {
        std::cout << "\t" << "FAILED -" << std::endl;
    }
}

int main() {
    BodyData body_data;
    CollisionShape sphere(SphereData{13.0});
    body_data.shapes.push_back(sphere);
    ActorData actor_data(body_data);

    WorldData world_data;
    
    ActorKey key = world_data.AddActor(actor_data);
    world_data.GetActor(key).body.position = Vector3{10, 20, 30};
    world_data.Update(10);

    GameState game1;
    game1.world_data = world_data;

    // GameState game2 = DeserializeGame(SerializeGame(game1));
    // std::string s1 = SerializeGame(game1).dump(4);
    // std::string s2 = SerializeGame(game2).dump(4);

    WorldData world_data2 = DeserializeWorld(SerializeWorld(world_data));
    std::string s1 = SerializeWorld(world_data).dump(4);
    std::string s2 = SerializeWorld(world_data2).dump(4);

    int diff = std::strcmp(
        s1.c_str(),
        s2.c_str()
    );

    PrintTest(diff == 0, "Serialization");
}