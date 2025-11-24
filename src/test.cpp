
int main() {}
// #include "Game.hpp"
// #include "Serialization.hpp"

// void PrintTest(bool test, std::string name) {
//     std::cout << name << std::endl;
//     if (test) {
//         std::cout << "\t" << "PASSED +" << std::endl;
//     }
//     else {
//         std::cout << "\t" << "FAILED -" << std::endl;
//     }
// }

// int main() {
//     InitWindow(10, 10, "Test");

//     BodyData body_data;
//     CollisionShape sphere(SphereData{13.0});
//     body_data.shapes.push_back(sphere);
//     ActorData actor_data(body_data);

//     WorldData world_data;
    
//     ActorKey key = world_data.AddActor(actor_data);
//     world_data.GetActor(key).body.position = Vector3{10, 20, 30};
//     world_data.Update(10);

//     GameState game1;
//     game1.world_data = world_data;

//     Game game_manager = {};
//     GameState game2 = game1;
//     auto s1 = game_manager.Serialize(game1);
//     auto s2 = game_manager.Serialize(game1);

//     bool eq = (s1.tick == s2.tick) && (s1.size = s2.size);
//     if (eq) {
//         for (int i = 0; i < s1.size; i++) {
//             if (s1.bytes[i] != s2.bytes[i]) {
//                 eq = false;
//                 break;
//             }
//         }
//     }

//     PrintTest(eq, "Serialization");
// }