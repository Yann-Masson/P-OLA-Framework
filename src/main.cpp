#include <iostream>
#include <torch/torch.h>

#include "OLA_Agent.hpp"
#include "ThermalModel.hpp"
#include <cppdic/ServiceProvider.hpp>

#include <torch/script.h>
#include <iomanip>
#include <chrono>

int libTorchTest() {
    std::cout << "======================================" << std::endl;
    std::cout << "GPU STRESS TEST Program Started" << std::endl;
    std::cout << "======================================" << std::endl;

    // Check if CUDA (GPU support) is available
    std::cout << "[INFO] Checking CUDA availability..." << std::endl;
    if (torch::cuda::is_available()) {
        std::cout << "[SUCCESS] CUDA is available! Training on GPU." << std::endl;
        std::cout << "[DEBUG] CUDA device count raw: " << static_cast<int>(torch::cuda::device_count()) << std::endl;
        std::cout << "[INFO] Number of CUDA devices: " << torch::cuda::device_count() << std::endl;

        torch::Device device(torch::kCUDA);
    } else {
        std::cout << "[WARNING] CUDA is not available. Training on CPU." << std::endl;
        torch::Device device(torch::kCPU);
    }

    // Define a MUCH HEAVIER neural network for GPU stress testing
    std::cout << "\n[INFO] Defining HEAVY neural network architecture..." << std::endl;
    struct HeavyNet : torch::nn::Module {
        HeavyNet() {
            // Much deeper and wider network
            fc1 = register_module("fc1", torch::nn::Linear(2048, 4096));
            fc2 = register_module("fc2", torch::nn::Linear(4096, 2048));
            fc3 = register_module("fc3", torch::nn::Linear(2048, 1024));
            fc4 = register_module("fc4", torch::nn::Linear(1024, 512));
            fc5 = register_module("fc5", torch::nn::Linear(512, 256));
            fc6 = register_module("fc6", torch::nn::Linear(256, 128));
            fc7 = register_module("fc7", torch::nn::Linear(128, 64));
            fc8 = register_module("fc8", torch::nn::Linear(64, 10));

            // Add batch normalization for stability
            bn1 = register_module("bn1", torch::nn::BatchNorm1d(4096));
            bn2 = register_module("bn2", torch::nn::BatchNorm1d(2048));
            bn3 = register_module("bn3", torch::nn::BatchNorm1d(1024));
            bn4 = register_module("bn4", torch::nn::BatchNorm1d(512));
        }

        torch::Tensor forward(torch::Tensor x) {
            x = torch::relu(bn1->forward(fc1->forward(x)));
            x = torch::relu(bn2->forward(fc2->forward(x)));
            x = torch::relu(bn3->forward(fc3->forward(x)));
            x = torch::relu(bn4->forward(fc4->forward(x)));
            x = torch::relu(fc5->forward(x));
            x = torch::relu(fc6->forward(x));
            x = torch::relu(fc7->forward(x));
            x = fc8->forward(x);
            return x;
        }

        torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr}, fc4{nullptr};
        torch::nn::Linear fc5{nullptr}, fc6{nullptr}, fc7{nullptr}, fc8{nullptr};
        torch::nn::BatchNorm1d bn1{nullptr}, bn2{nullptr}, bn3{nullptr}, bn4{nullptr};
    };

    // Create an instance of the network
    std::cout << "[INFO] Creating HEAVY neural network instance..." << std::endl;
    HeavyNet net;
    std::cout << "[SUCCESS] Network created with architecture:" << std::endl;
    std::cout << "           2048 -> 4096 -> 2048 -> 1024 -> 512 -> 256 -> 128 -> 64 -> 10" << std::endl;
    std::cout << "           (with BatchNorm layers for stability)" << std::endl;

    // Generate MUCH LARGER random data for GPU stress testing
    std::cout << "\n[INFO] Generating LARGE random training dataset..." << std::endl;
    auto x = torch::rand({512, 2048});  // 512 samples, much larger batch
    auto y = torch::randint(10, {512});
    std::cout << "[SUCCESS] Generated data - Input shape: [512, 2048], Label shape: [512]" << std::endl;

    // Move data to GPU if available
    if (torch::cuda::is_available()) {
        std::cout << "[INFO] Moving data and model to GPU..." << std::endl;
        x = x.cuda();
        y = y.cuda();
        net.to(torch::kCUDA);
        std::cout << "[DEBUG] x device: " << x.device() << std::endl;
        std::cout << "[DEBUG] y device: " << y.device() << std::endl;
        std::cout << "[SUCCESS] Data and model successfully transferred to GPU" << std::endl;

        // Force CUDA synchronization to ensure transfer is complete
        torch::cuda::synchronize();
    } else {
        std::cout << "[INFO] Using CPU for computation" << std::endl;
    }

    // Define a loss function and optimizer
    std::cout << "\n[INFO] Setting up loss function and optimizer..." << std::endl;
    torch::nn::CrossEntropyLoss criterion;
    torch::optim::Adam optimizer(net.parameters(), torch::optim::AdamOptions(0.001));
    std::cout << "[SUCCESS] Using CrossEntropyLoss and Adam optimizer (learning rate: 0.001)" << std::endl;

    // Training loop - MANY MORE EPOCHS for GPU stress testing
    const int num_epochs = 500;  // Much more epochs
    std::cout << "\n[INFO] Starting HEAVY training loop (" << num_epochs << " epochs)..." << std::endl;
    std::cout << "[INFO] This will take 1-2 minutes - Monitor your GPU now!" << std::endl;
    std::cout << "======================================" << std::endl;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (size_t epoch = 0; epoch < num_epochs; ++epoch) {
        auto epoch_start = std::chrono::high_resolution_clock::now();

        optimizer.zero_grad();
        auto output = net.forward(x);
        auto loss = criterion(output, y);
        loss.backward();
        optimizer.step();

        // Force GPU synchronization for accurate timing
        if (torch::cuda::is_available()) {
            torch::cuda::synchronize();
        }

        auto epoch_end = std::chrono::high_resolution_clock::now();
        auto epoch_duration = std::chrono::duration_cast<std::chrono::milliseconds>(epoch_end - epoch_start).count();

        // Print progress every 10 epochs
        if ((epoch + 1) % 10 == 0) {
            auto current_time = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

            std::cout << "[EPOCH " << std::setw(3) << (epoch + 1) << "/" << num_epochs << "] ";
            std::cout << "Loss: " << std::fixed << std::setprecision(6) << loss.item<float>();
            std::cout << " | Time: " << epoch_duration << "ms";
            std::cout << " | Elapsed: " << elapsed << "s" << std::endl;
        }
    }

    const auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();

    std::cout << "======================================" << std::endl;
    std::cout << "[SUCCESS] Training complete!" << std::endl;
    std::cout << "[INFO] Total training time: " << total_duration << " seconds" << std::endl;
    std::cout << "======================================" << std::endl;

    return 0;
}

int main() {
    libTorchTest();

    std::cout << "========== STARTING OLA SIMULATION ==========\n\n";
    ThermalModel room(18.0);       // Start room at 18°C
    OLA_Controller smartAgent;

    double simTime = 0;            // Seconds
    double dt = 60.0;              // 1 minute steps
    double totalCost = 0.0;

    // Simulation loop (e.g., simulate 24 hours)
    for (int i = 0; i < 1440; ++i) {
        // 1. Get External Data (This could be your API calls)
        double currentPrice = 0.15; // $/kWh
        double tempOut = 5.0;       // Cold day

        // 2. Prepare the State for the AI
        State currentState = {
            room.getTemp(),
            tempOut,
            currentPrice,
            10.5,  // 10.5 km away
            1.2    // km/minute
        };

        // 3. AI makes a decision
        double action = smartAgent.decide(currentState);

        // 4. Update Physics
        double energyBefore = room.getTotalEnergyKWh();
        room.update(tempOut, action, dt);
        double energyAfter = room.getTotalEnergyKWh();

        // 5. Track Cost
        totalCost += (energyAfter - energyBefore) * currentPrice;

        simTime += dt;

        // Log results for your paper's graphs
        std::cout << "Time: " << i << "m | Temp: " << room.getTemp()
                  << " | Cost: $" << totalCost << std::endl;
    }

    return 0;
}
