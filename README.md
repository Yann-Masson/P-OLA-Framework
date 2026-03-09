<div align="center">

![P-OLA Framework Logo](./assets/p-ola-framework.png)

# P-OLA Framework

**Predictive Online Learning Algorithm for Smart Thermostat Control**

*A PPO-based deep reinforcement learning framework for edge IoT devices*

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![LibTorch](https://img.shields.io/badge/LibTorch-2.x-orange.svg)](https://pytorch.org/cppdocs/)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

</div>

---

## Overview

P-OLA is a smart thermostat controller that trains a neural network entirely in C++ to balance **user thermal comfort**, **energy cost minimization**, and **GPS-based predictive pre-heating**. The system runs in-process on the device — there is no Python runtime dependency in production.

The framework implements two variants evaluated against a dataset of 10 real smart homes:

| Variant | State dimensions | GPS pre-heating | Description |
|---------|-----------------|-----------------|-------------|
| **P-OLA** | 42 | Yes | Full model: uses GPS distance/velocity to anticipate user arrival and pre-heat accordingly |
| **OLA** | 24 | No | Ablation baseline: comfort + economy optimization without location awareness |

### Key results (30-day simulation, 10 homes)

| Metric | OLA | P-OLA | Improvement |
|--------|-----|-------|-------------|
| Avg. thermal comfort | 88.1% | 92.2% | **+4.7 pp** |
| Avg. energy cost / home | ~5.79 €/month | ~4.57 €/month | **−21%** |

> Results produced by `scripts/benchmark_all_homes.sh` at 43 200 training timesteps. Full output in [`results/OLA.txt`](results/OLA.txt) and [`results/P-OLA.txt`](results/P-OLA.txt).

---

## Table of Contents

1. [Architecture](#architecture)
2. [Prerequisites](#prerequisites)
3. [Build](#build)
4. [Usage](#usage)
   - [Training](#training)
   - [Simulation / Inference](#simulation--inference)
   - [Benchmarking all homes](#benchmarking-all-homes)
5. [Project Structure](#project-structure)
6. [Algorithm](#algorithm)
7. [Reward Function](#reward-function)
8. [Dataset](#dataset)
9. [Citation](#citation)

---

## Architecture

```
SimulationBuilder (factory)
        │
        ├── Room (physics engine)
        │     ├── Wall × N  (conduction losses, U-value)
        │     ├── Window × N (solar gain + conduction)
        │     └── Heater     (0–1 continuous power)
        │
        ├── Services
        │     ├── EnergyPriceService   (spot electricity price)
        │     ├── WeatherService       (outdoor temperature)
        │     ├── GPSService           (user distance & velocity)
        │     ├── UserScheduleService  (occupancy calendar)
        │     └── UserComfortService   (ASHRAE-based comfort metric)
        │
        └── IAIModel interface
              ├── AIModel              (loads TorchScript .pt — inference)
              ├── RuleBasedModel       (deterministic fallback)
              └── POLATrainingAgent    (PPO actor-critic — training)
```

The training agent **implements the same `IAIModel` interface** as the inference model. The simulation physics are therefore identical between training and deployment — there is no separate gym-style environment.

---

## Prerequisites

| Dependency | Version | Notes |
|------------|---------|-------|
| CMake | ≥ 3.18 | |
| C++ compiler | C++23 | GCC 13+, Clang 16+, or MSVC 2022 |
| LibTorch | 2.x | See setup script below |

### 1 — Download LibTorch

**macOS (ARM64):**
```bash
./utils/setup_libtorch.sh
```

**Windows (CUDA):**
```powershell
.\utils\setup_libtorch.ps1
```

This places LibTorch under `libs/libtorch/`. The CMake script finds it automatically.

---

## Build

```bash
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build all targets
cmake --build build --parallel

# Three executables are produced under build/
#   build/Simulator       — inference / simulation
#   build/P-OLA_Trainer   — train with GPS features
#   build/OLA_Trainer     — train without GPS (ablation)
```

> **Windows + CUDA:** CUDA paths are handled automatically by `cmake/windows_cuda.cmake` when building on Windows. No manual configuration needed.

---

## Usage

### Training

**P-OLA** (with GPS pre-heating, recommended):
```bash
./build/P-OLA_Trainer \
  --timesteps 1000000 \
  --data data/data_home_1_scheduled_GPS.csv \
  --w-comfort 0.5 \
  --w-economy 0.3 \
  --w-gps 0.2 \
  --output models/my_model.pt
```

**OLA** (ablation, no GPS):
```bash
./build/OLA_Trainer \
  --timesteps 1000000 \
  --data data/data_home_1_scheduled.csv \
  --w-comfort 0.6 \
  --w-economy 0.4 \
  --output models/ola_model.pt
```

**All training options:**

| Flag | Default | Description |
|------|---------|-------------|
| `--timesteps N` | `1000000` | Total training steps |
| `--lr RATE` | `3e-4` | Adam learning rate |
| `--w-comfort W` | `0.5` | Comfort penalty weight |
| `--w-economy W` | `0.3` | Economy penalty weight |
| `--w-gps W` | `0.2` | GPS arrival penalty weight *(P-OLA only)* |
| `--hidden-dim N` | `64` | Hidden layer width |
| `--rollout-steps N` | `2048` | Steps per PPO rollout |
| `--epochs N` | `4` | PPO update epochs per rollout |
| `--data PATH` | *(home 1)* | Input CSV file |
| `--output PATH` | `models/ai_model.pt` | Where to save the model |
| `--seed N` | `42` | Random seed |

### Simulation / Inference

```bash
./build/Simulator \
  --model models/my_model.pt \
  --data data/data_home_1_scheduled_GPS.csv
```

The simulator prints per-step temperature, heater power, comfort score, and cumulative energy/cost to stdout and writes a full CSV log to the working directory.

### Benchmarking all homes

```bash
# P-OLA on all 10 homes (default)
./scripts/benchmark_all_homes.sh --timesteps 43200 --model p-ola

# OLA ablation
./scripts/benchmark_all_homes.sh --timesteps 43200 --model ola
```

Per-home CSV records are saved to `scripts/output/`.

---

## Project Structure

```
P-OLA-Framework/
├── CMakeLists.txt              Root build file
├── src/
│   ├── Main.cpp                Simulator entry point
│   ├── TrainOLA.cpp            OLA training entry point
│   ├── TrainPOLA.cpp           P-OLA training entry point
│   ├── Common/                 Shared data types (AIState, sensor structs)
│   ├── Interfaces/             Pure virtual contracts (IClock, IAIModel, …)
│   ├── Models/                 AIModel (TorchScript loader), RuleBasedModel
│   ├── Services/               EnergyPrice, Weather, GPS, Comfort services
│   ├── Simulation/             Room physics, SimulationBuilder factory
│   └── Training/
│       ├── TrainingConfig.hpp  Hyperparameters & state normalisation
│       ├── PPOTrainer.hpp/.cpp PPO update loop (actor-critic, GAE)
│       ├── OLA/                OLA training agent
│       └── P-OLA/              P-OLA training agent
├── data/                       10-home smart home CSV datasets
├── models/                     Pre-trained .pt model files
├── results/                    Benchmark outputs (OLA.txt, P-OLA.txt)
├── scripts/
│   ├── benchmark_all_homes.sh  Multi-home evaluation script
│   └── plot_all_results.sh     Results visualisation script
├── docs/
│   └── AI_Training_Guide.md    In-depth guide to the RL system
├── libs/
│   ├── forge/                  Lightweight C++23 DI container
│   └── libtorch/               PyTorch C++ runtime (downloaded by setup script)
└── utils/
    ├── setup_libtorch.sh       macOS LibTorch installer
    └── setup_libtorch.ps1      Windows LibTorch installer
```

---

## Forge — Dependency Injection Container

P-OLA uses [**Forge**](libs/forge/README.md), a lightweight header-only C++20 dependency injection container included in `libs/forge/`. Forge is the glue that holds the simulation together without hard-coupling components.

**What Forge does here:**

- Every service (`Room`, `IClock`, `GPSService`, …) is registered against its interface inside `SimulationBuilder`.
- When `build()` is called, Forge constructs all objects in the right order, resolving dependencies automatically via constructor injection through `forge::ProviderRef`.
- Swapping implementations — e.g. `AIModel` for `POLATrainingAgent` for `RuleBasedModel` — requires changing a single registration line; no other code changes.

```cpp
// Training: inject the PPO agent as the AI model
auto provider = SimulationBuilder()
    .setClock(60.0)
    .setDataSource("data_home_1_scheduled_GPS.csv")
    .addWall(5.0, 2.5, 0.3, 0.6)
    .addHeater(2000.0)
    .trainPOLAModel(config)  // registers POLATrainingAgent as IAIModel
    .build();                // Forge resolves all dependencies here

// Inference: swap in the loaded TorchScript model — no other change
auto provider = SimulationBuilder()
    ...
    .useAIModel("models/my_model.pt")  // registers AIModel as IAIModel
    .build();
```

Forge has **no external dependencies** and requires no compilation step. See its [full documentation](libs/forge/README.md) for the complete API.

---

## Algorithm

The framework uses **Proximal Policy Optimization (PPO)** with an actor-critic architecture.

### State space

**P-OLA** (42 dimensions):

| Feature group | Dimensions | Description |
|---------------|-----------|-------------|
| Indoor temperature | 1 | Current room temperature (normalized) |
| Target temperature | 1 | User comfort setpoint |
| Outdoor temperature | 1 | External weather |
| Time features | 6 | Hour, day-of-week (sin/cos encoding × 2) |
| Occupancy | 1 | Binary user presence |
| Energy price | 1 | Spot electricity price (€/kWh) |
| GPS distance | 12 | Historical window of user-to-home distances |
| GPS velocity | 12 | Historical window of user travel speeds |
| Heater state | 7 | Recent heater power history |

**OLA** removes the GPS features (24 dimensions total).

### Network architecture

```
Input (42d or 24d)
    └── Linear(hidden=64) → Tanh
            └── Linear(64) → Tanh
                    ├── Actor head  → Linear(64, 1) → Tanh → heater power ∈ [0, 1]
                    └── Critic head → Linear(64, 1)         → V(s)
```

### PPO hyperparameters (defaults)

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| γ (discount) | 0.99 | Long planning horizon (30-day episodes) |
| λ (GAE) | 0.95 | Bias-variance balance for advantage estimation |
| ε (clip) | 0.2 | Standard PPO stability bound |
| Learning rate | 3×10⁻⁴ | Adam default, well-established for PPO |
| Rollout steps | 2048 | Captures full daily cycles |
| Mini-batch size | 64 | GPU-friendly batch |
| PPO epochs | 4 | Standard for continuous control |

---

## Reward Function

The reward is a weighted sum of three negative penalties (lower is better):

$$r_t = -\left( w_c \cdot P_{\text{comfort}} + w_e \cdot P_{\text{economy}} + w_g \cdot P_{\text{GPS}} \right)$$

| Penalty | Formula | Description |
|---------|---------|-------------|
| $P_{\text{comfort}}$ | $(T_{\text{in}} - T_{\text{target}})^2$ | Quadratic temperature deviation |
| $P_{\text{economy}}$ | $\text{price} \times \text{power}$ | Price-weighted energy usage |
| $P_{\text{GPS}}$ | $20 \cdot (T_{\text{target}} - T_{\text{in}})$ if distance < 0.1 km and deficit > 1 °C, else 0 | Penalises a cold home on arrival |

The weights $(w_c, w_e, w_g)$ sum to 1 and define the controller "personality". The defaults (0.5 / 0.3 / 0.2) represent a balanced profile. Raising $w_c$ makes the controller comfort-first; raising $w_e$ makes it cost-first.

---

## Dataset

The `data/` directory contains records from **10 simulated smart homes** derived from the publicly available *Smart Home Energy Management* dataset. Each home has three CSV variants:

| Suffix | Contents |
|--------|---------|
| *(none)* | Raw occupancy + temperature + price |
| `_scheduled` | Above + daily schedule annotations |
| `_scheduled_GPS` | Above + GPS distance/velocity columns |

The preprocessing pipeline is in [`data/process_smart_home_data.py`](data/process_smart_home_data.py).

---

## Citation

This framework was developed as part of a research project at **Halmstad University** (Edge Computing and IoT, 2026). If you use this code or results in academic work, please cite the accompanying paper:

```bibtex
@inproceedings{pola2026iot,
  title     = {P-OLA: A Predictive Online Learning Algorithm for GPS-Aware Smart Thermostat Control on Edge Devices},
  booktitle = {Proceedings of IoT 2026},
  year      = {2026},
  note      = {Group 21, Halmstad University}
}
```

---

## License

This project is released under the [MIT License](LICENSE).
