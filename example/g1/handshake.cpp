#include <iostream>
#include <thread>
#include <chrono>
#include <unitree/robot/go2/sport/sport_client.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>

int main(int argc, char** argv) {
    // 1. Initialize the Unitree Channel network layer
    // Make sure your PC's environment variable is set (e.g., export CYCLONEDDS_URI=...)
    unitree::robot::ChannelFactory::Instance()->Init(0, nullptr);

    // 2. Instantiate the Sport Client
    unitree::robot::go2::SportClient client;
    client.SetTimeout(5.0f);
    client.Init();

    std::cout << "Sending High-Level Wave Command via C++..." << std::endl;
    
    // 3. Trigger the pre-built wave macro
    int32_t result = client.WaveHand();
    int32_t resul = client.WaveHand();
    if (result == 0) {
        std::cout << "Wave command accepted successfully!" << std::endl;
    } else {
        std::cerr << "Failed to trigger wave. Error code: " << result << std::endl;
    }
  
    int32_t mover=Move(1.0f, 0.0f, 0.0f)；
  if (mover == 0) {
        std::cout << "Move command accepted successfully!" << std::endl;
    } else {
        std::cerr << "Failed to trigger walking. Error code: " << mover << std::endl;
    }
    int32_t resul = client.WaveHand();
  if (resul == 0) {
        std::cout << "2nd wave command accepted successfully!" << std::endl;
    } else {
        std::cerr << "Failed to trigger the 2nd wave. Error code: " << resul << std::endl;
    }
    // Allow time for the motion to complete before exiting the program
    std::this_thread::sleep_for(std::chrono::seconds(4));
    return 0;
}
