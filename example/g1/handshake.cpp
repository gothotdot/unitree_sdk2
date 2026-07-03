#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

#include <unitree/robot/go2/sport/sport_client.hpp>

static std::atomic<bool> g_running{true};

void signal_handler(int) {
    g_running = false;
}

int main(int argc, char** argv) {
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

    for (int x = 0; x < 5 && g_running.load(); ++x) {
        int32_t mover = client.Move(1.0f, 0.0f, 0.0f);
        if (mover == 0) {
            std::cout << "Forward move command accepted!" << std::endl;
        } else {
            std::cerr << "Forward move failed. Error: " << mover << std::endl;
        }

        for (int i = 0; i < 30 && g_running.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        int32_t backer = client.Move(-1.0f, 0.0f, 0.0f);
        if (backer == 0) {
            std::cout << "Backward move command accepted!" << std::endl;
        } else {
            std::cerr << "Backward move failed. Error: " << backer << std::endl;
        }

        for (int i = 0; i < 30 && g_running.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    std::cout << "Stopping..." << std::endl;
    client.Move(0.0f, 0.0f, 0.0f);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Done!" << std::endl;
    return 0;
}
