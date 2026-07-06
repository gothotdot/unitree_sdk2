#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/g1/arm/g1_arm_action_client.hpp>
#include <unitree/robot/g1/arm/g1_arm_action_error.hpp>
#include <unitree/robot/g1/loco/g1_loco_client.hpp>

static std::atomic<bool> g_running{true};

void signal_handler(int) {
    g_running = false;
}

int main(int argc, char** argv) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    const char* iface = (argc > 1) ? argv[1] : "eth0";

    // === DDS Initialization ===
    unitree::robot::ChannelFactory::Instance()->Init(0, iface);

    // === LocoClient: Controls walking and body state ===
    unitree::robot::g1::LocoClient loco;
    loco.SetTimeout(5.0f);
    if (!loco.Init()) {
        std::cerr << "Failed to initialize LocoClient." << std::endl;
        return -1;
    }

    // === G1ArmActionClient: Controls predefined arm gestures ===
    unitree::robot::g1::G1ArmActionClient arm;
    arm.SetTimeout(10.0f);
    if (!arm.Init()) {
        std::cerr << "Failed to initialize G1ArmActionClient." << std::endl;
        return -1;
    }

    // ============================================================
    // PHASE 1: Initialize robot to walking-ready state
    // ============================================================

    std::cout << "Damping robot..." << std::endl;
    loco.Damp();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "Standing up..." << std::endl;
    loco.StandUp();
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Enter Regular Mode (501) — REQUIRED for both Move() and arm actions
    std::cout << "Entering Regular Mode (501)..." << std::endl;
    loco.SetFsmId(501);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // ============================================================
    // PHASE 2: Walk forward while waving
    // ============================================================

    // --- Start walking forward ---
    // Move(vx, vy, vyaw): vx=0.3 m/s forward, vy=0, no rotation
    std::cout << "Starting to walk forward..." << std::endl;
    int32_t move_ret = loco.Move(0.3f, 0.0f, 0.0f);
    if (move_ret != 0) {
        std::cerr << "Move() failed with error: " << move_ret << std::endl;
        // Common causes: wrong FSM state, robot not standing, balance not ready
        loco.Damp();
        return -1;
    }

    // --- Start face wave while walking ---
    // The robot can execute arm gestures while moving in mode 501
    std::cout << "Waving while walking!" << std::endl;
    int32_t arm_ret = arm.ExecuteAction(25);  // 25 = face wave
    if (arm_ret != 0) {
        std::cerr << "Arm action failed with error: " << arm_ret << std::endl;
        // Continue anyway — walking is more important than waving
    }

    // --- Let the gesture play out while walking ---
    // Face wave takes ~3-5 seconds. We walk for about that long.
    for (int i = 0; i < 40 && g_running.load(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // ============================================================
    // PHASE 3: Stop and wave again while stationary
    // ============================================================

    // Stop walking — robot stands in place
    std::cout << "Stopping movement..." << std::endl;
    loco.StopMove();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Wave again while stationary (cleaner gesture)
    std::cout << "Waving while standing still..." << std::endl;
    arm_ret = arm.ExecuteAction(26);
    if (arm_ret == 0) {
        for (int i = 0; i < 50 && g_running.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    std::cout<<"Clapping..."<<std::endl;
    arm_ret=arm.ExecuteAction(17);
    if(arm_ret==0){
        for(int i=0;i<50&&g_running.load();++i){
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    // ============================================================
    // PHASE 4: Walk backward (optional)
    // ============================================================

    std::cout << "Walking backward..." << std::endl;
    move_ret = loco.Move(-0.3f, 0.0f, 0.0f);  // vx=-0.3 = backward
    if (move_ret == 0) {
        for (int i = 0; i < 30 && g_running.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    loco.StopMove();

    // ============================================================
    // PHASE 5: Cleanup
    // ============================================================

    std::cout << "Releasing arm control..." << std::endl;
    arm.ExecuteAction(99);  // 99 = release arm
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Damping and exiting..." << std::endl;
    loco.Damp();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "Done!" << std::endl;
    return 0;
}
