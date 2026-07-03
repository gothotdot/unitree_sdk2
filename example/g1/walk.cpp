#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

#include <unitree/robot/go2/sport/sport_client.hpp>

// Emergency stop flag
static std::atomic<bool> g_running{true};

void signal_handler(int) {
    g_running = false;
}

int main(int argc, char** argv) {

    // Move forward 1.0m in X direction, 0 rotation, 0 Y
  for(int x=0;x<5;x++){
    int32_t mover = client.Move(1.0f, 0.0f, 0.0f);
    if (mover == 0) {
        std::cout << "Forward move command accepted successfully!" << std::endl;
    } else {
        std::cerr << "Failed to trigger forward walking. Error code: " << mover << std::endl;
      
    }
    sleep(3);
    int32_t backer = client.Move(-1.0f, 0.0f, 0.0f);
    if (backer == 0) {
        std::cout << "Forward move command accepted successfully!" << std::endl;
    } else {
        std::cerr << "Failed to trigger forward walking. Error code: " << backer << std::endl;
      
    }
    sleep(3);
  }
   // Final stand/stop command to ensure robot is stable
    std::cout << "Sequence complete. Returning to stand..." << std::endl;
    client.StandUp();  // or client.StopMove() if available
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "Done!" << std::endl;
    return 0;
}
