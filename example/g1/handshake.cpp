#include <iostream>
#include <thread>
#include <chrono>
#include <unitree/robot/go2/sport/sport_client.hpp>

int main(int argc, char** argv) {
    const char* iface = (argc > 1) ? argv[1] : "eth0"; // fallback
    unitree::robot::ChannelFactory::Instance()->Init(0, iface);

    unitree::robot::go2::SportClient client;
    client.SetTimeout(5.0f);
    if (!client.Init()) {
        std::cerr << "Failed to initialize SportClient." << std::endl;
        return -1;
    }

    std::cout << "Sending High-Level Wave Command via C++..." << std::endl;

    int32_t result = client.hello();
    if (result == 0) {
        std::cout << "Wave command accepted successfully!" << std::endl;
    } else {
        std::cerr << "Failed to trigger wave. Error code: " << result << std::endl;
    }

    int32_t mover = client.Move(1.0f, 0.0f, 0.0f);
    if (mover == 0) {
        std::cout << "Move command accepted successfully!" << std::endl;
    } else {
        std::cerr << "Failed to trigger walking. Error code: " << mover << std::endl;
    }

    // Wait for motions to complete
    std::this_thread::sleep_for(std::chrono::seconds(4));

    // Second wave
    int32_t result2 = client.hello();
    if (result2 == 0) {
        std::cout << "2nd wave command accepted successfully!" << std::endl;
    } else {
        std::cerr << "Failed to trigger 2nd wave. Error code: " << result2 << std::endl;
    }
        int32_t backer = client.Move(-1.0f, 0.0f, 0.0f);
    if (backer == 0) {
        std::cout << "Move command accepted successfully!" << std::endl;
    } else {
        std::cerr << "Failed to trigger backing. Error code: " << backer << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
}
