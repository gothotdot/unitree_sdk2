#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <iostream>
#include <memory>
#include <thread>

#include <unitree/robot/channel/channel_publisher.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/idl/hg/LowCmd_.hpp>
#include <unitree/idl/hg/LowState_.hpp>
#include <unitree/robot/time/sleep.hpp>
#include <unitree/robot/time/stamp.hpp>

using namespace unitree::common;
using namespace unitree::robot;
using namespace unitree::idl::hg;

// DDS topic names for G1 (humanoid)
constexpr const char* HG_CMD_TOPIC = "rt/lowcmd";
constexpr const char* HG_STATE_TOPIC = "rt/lowstate";

static std::atomic<bool> g_running{true};

void signal_handler(int) {
    g_running = false;
}

class G1Example {
public:
    G1Example(std::string networkInterface)
        : time_(0.0),
          control_dt_(0.002),   // 500 Hz
          duration_(3.0) {
        
        // Initialize DDS channel factory
        ChannelFactory::Instance()->Init(0, networkInterface);
        
        // Create publisher for LowCmd (commands TO robot)
        lowcmd_publisher_.reset(new ChannelPublisher<LowCmd_>(HG_CMD_TOPIC));
        lowcmd_publisher_->InitChannel();
        
        // Create subscriber for LowState (feedback FROM robot)
        lowstate_subscriber_.reset(new ChannelSubscriber<LowState_>(HG_STATE_TOPIC));
        lowstate_subscriber_->InitChannel(
            std::bind(&G1Example::LowStateHandler, this, std::placeholders::_1), 1);
        
        // Initialize LowCmd message with default values
        InitLowCmd();
        
        // Create threads: command writer at 500Hz, control logic at 500Hz
        command_writer_ptr_ = CreateRecurrentThreadEx(
            "command_writer", UT_CPU_ID_NONE, 2000, &G1Example::LowCommandWriter, this);
        control_thread_ptr_ = CreateRecurrentThreadEx(
            "control", UT_CPU_ID_NONE, 2000, &G1Example::Control, this);
    }
    
    ~G1Example() {
        // Send zero commands before exiting
        if (lowcmd_publisher_) {
            for (auto& motor : lowcmd_.motor_cmd()) {
                motor.q() = 0.0;
                motor.dq() = 0.0;
                motor.tau() = 0.0;
            }
            lowcmd_publisher_->Write(lowcmd_);
            Sleep(100);  // 100ms
        }
    }
    
    void Join() {
        if (command_writer_ptr_) command_writer_ptr_->Join();
        if (control_thread_ptr_) control_thread_ptr_->Join();
    }
    
    bool IsRunning() const { return g_running.load(); }

private:
    void InitLowCmd() {
        // Initialize all motor commands to safe defaults
        for (auto& motor : lowcmd_.motor_cmd()) {
            motor.mode() = 1;      // Position control mode
            motor.q() = 0.0;       // Target position
            motor.dq() = 0.0;      // Target velocity
            motor.kp() = 0.0;      // Position gain
            motor.kd() = 0.0;      // Velocity gain
            motor.tau() = 0.0;     // Feedforward torque
            motor.reserve() = 0;
        }
    }
    
    void LowStateHandler(const void* message) {
        // Cast to LowState_ and store
        const LowState_* state = static_cast<const LowState_*>(message);
        lowstate_ = *state;
        state_received_ = true;
    }
    
    void LowCommandWriter() {
        // Thread runs at 500Hz, publishes LowCmd
        if (state_received_) {
            lowcmd_publisher_->Write(lowcmd_);
        }
    }
    
    void Control() {
        // Main control logic — runs at 500Hz
        // This is where you implement your motion sequence
        
        time_ += control_dt_;
        
        // Example: simple ankle swing (sinusoidal) as a placeholder
        // Replace this with your actual motion logic
        
        static double phase = 0.0;
        phase += control_dt_ * 2.0 * M_PI * 0.5;  // 0.5 Hz sine wave
        
        // G1 has 29 DOF. Joint indices depend on your G1 variant.
        // Common ankle joints for G1-29DOF:
        // Left ankle pitch ~ index 12, right ankle pitch ~ index 17
        // (Verify against your robot's joint map)
        
        // For demonstration: small ankle oscillation
        // In real use, you'd implement full body walking or use a motion library
        float ankle_amp = 0.1f;   // radians
        float ankle_freq = 0.5f;  // Hz
        
        // Only send commands after we've received state (know robot is alive)
        if (state_received_) {
            // Example: oscillate left ankle
            // lowcmd_.motor_cmd()[12].q() = ankle_amp * sin(phase);
            // lowcmd_.motor_cmd()[12].kp() = 100.0;
            // lowcmd_.motor_cmd()[12].kd() = 5.0;
            
            // For now, just keep zero commands (safe standby)
            // Replace with your motion sequence
        }
    }

    // DDS communication
    std::shared_ptr<ChannelPublisher<LowCmd_>> lowcmd_publisher_;
    std::shared_ptr<ChannelSubscriber<LowState_>> lowstate_subscriber_;
    
    // Threads
    ThreadPtr command_writer_ptr_;
    ThreadPtr control_thread_ptr_;
    
    // Messages
    LowCmd_ lowcmd_;
    LowState_ lowstate_;
    
    // State
    bool state_received_ = false;
    double time_;
    double control_dt_;
    double duration_;
};

int main(int argc, char** argv) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    const char* iface = (argc > 1) ? argv[1] : "eth0";
    
    std::cout << "Starting G1 control on interface: " << iface << std::endl;
    std::cout << "WARNING: G1 has no SportClient. This code only sets up DDS communication." << std::endl;
    std::cout << "You must implement your own locomotion controller or use a motion library." << std::endl;
    
    G1Example example(iface);
    
    // Wait for Ctrl+C
    while (g_running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\nStopping..." << std::endl;
    example.Join();
    
    std::cout << "Done!" << std::endl;
    return 0;
}
