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
    // Register signal handlers for graceful shutdown
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    const char* iface = (argc > 1) ? argv[1] : "eth0";
    unitree::robot::ChannelFactory::Instance()->Init(0, iface);

    unitree::robot::go2::SportClient client;
    client.SetTimeout(5.0f);
    if (!client.Init()) {
        std::cerr << "Failed to initialize SportClient." << std::endl;
        return -1;
    }

    std::cout << "Sending High-Level Wave Command via C++..." << std::endl;

    // dance2() - likely a greeting/wave gesture
    int32_t result = client.dance2();
    if (result == 0) {
        std::cout << "Wave command (dance2) accepted successfully!" << std::endl;
    } else {
        std::cerr << "Failed to trigger dance2. Error code: " << result << std::endl;
    }

    // Move forward 1.0m in X direction, 0 rotation, 0 Y
    int32_t mover = client.Move(1.0f, 0.0f, 0.0f);
    if (mover == 0) {
        std::cout << "Forward move command accepted successfully!" << std::endl;
    } else {
        std::cerr << "Failed to trigger forward walking. Error code: " << mover << std::endl;
    }

    // Wait for motions to complete (4 seconds)
    std::cout << "Waiting for forward motion..." << std::endl;
    for (int i = 0; i < 40 && g_running.load(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (!g_running.load()) {
        std::cout << "Interrupted, exiting..." << std::endl;
        return 0;
    }

    // Second gesture - dance1()
    std::cout << "Sending second wave..." << std::endl;
    int32_t result2 = client.dance1();
    if (result2 == 0) {
        std::cout << "2nd wave command (dance1) accepted successfully!" << std::endl;
    } else {
        std::cerr << "Failed to trigger dance1. Error code: " << result2 << std::endl;
    }

    // Move backward 1.0m
    int32_t backer = client.Move(-1.0f, 0.0f, 0.0f);
    if (backer == 0) {
        std::cout << "Backward move command accepted successfully!" << std::endl;
    } else {
        std::cerr << "Failed to trigger backward walking. Error code: " << backer << std::endl;
    }

    // Wait for backward motion
    std::cout << "Waiting for backward motion..." << std::endl;
    for (int i = 0; i < 40 && g_running.load(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (!g_running.load()) {
        std::cout << "Interrupted, exiting..." << std::endl;
        return 0;
    }

    // Final stand/stop command to ensure robot is stable
    std::cout << "Sequence complete. Returning to stand..." << std::endl;
    client.StandUp();  // or client.StopMove() if available
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "Done!" << std::endl;
    return 0;
}
