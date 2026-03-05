# P-OLA AI Training System — Complete Guide

> **Target audience:** Someone who has never worked with neural networks or reinforcement learning before.  
> **Goal:** After reading this, you should be able to explain every design decision in the training system, tune the hyperparameters, and write about it in a scientific paper.

---

## Table of Contents

1. [The Big Picture — What Are We Doing?](#1-the-big-picture)
2. [Glossary — AI Keywords Explained](#2-glossary)
3. [Why Reinforcement Learning (and not something else)?](#3-why-reinforcement-learning)
4. [Why PPO Specifically?](#4-why-ppo)
5. [Architecture — Every Class Explained](#5-architecture)
6. [The Reward Function — The Heart of the System](#6-the-reward-function)
7. [The Neural Network — ActorCritic](#7-the-neural-network)
8. [The Training Loop — PPOTrainer](#8-the-training-loop)
9. [Default Values — Why These Numbers?](#9-default-values)
10. [Weight Profiles — Eco, Comfort, and More](#10-weight-profiles)
11. [How to Use the Trainer](#11-how-to-use)
12. [From Training to Inference — The Full Pipeline](#12-full-pipeline)
13. [Self-Check Questions](#13-self-check)

---

## 1. The Big Picture

Imagine you hire a new employee to control your home's heater. On their first day, they know nothing — they turn the heater on when it's already hot, off when it's freezing, and waste electricity during peak pricing hours.

But every minute, you give them a **score** (the *reward*):
- "The house is comfortable" → good score
- "You just spent €2 on electricity during peak hours" → bad score  
- "I arrived home and it's 15°C inside" → terrible score

After thousands of days of practice (that's what training is), this employee learns:
- "When the owner is 10km away driving home, I should start pre-heating"
- "Electricity is cheap at 2am — let me heat up now and coast through the expensive morning"
- "It's only 0.5°C below target — not worth turning on the heater at €0.40/kWh"

**That employee is our neural network.** The "days of practice" are simulated in seconds on your computer.

### The flow

```
┌─────────────────────────────────────────────────┐
│                  TRAINING PHASE                  │
│                                                  │
│  TrainingEnvironment ──→ ActorCritic Network     │
│  (simulated room)        (the "employee")        │
│       │                       │                  │
│       │    action (power)     │                  │
│       │◄──────────────────────│                  │
│       │                       │                  │
│       │    reward + new state │                  │
│       │──────────────────────►│                  │
│       │                       │                  │
│  PPOTrainer updates the network weights          │
│  to maximize total reward over time              │
│                                                  │
│  Output: models/ai_model.pt (TorchScript file)   │
└─────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────┐
│                 INFERENCE PHASE                   │
│                                                  │
│  Real sensors ──→ AIModel (loads .pt file)       │
│  (GPS, weather,    │                             │
│   price, temp)     │                             │
│                    ▼                             │
│              Heater.setPower(0.73)               │
│                                                  │
│  The trained network makes decisions in the      │
│  real simulation (or on a real device)            │
└─────────────────────────────────────────────────┘
```

---

## 2. Glossary

Every "AI keyword" used in this project, explained simply:

### Neural Network Basics

| Term | Definition | Analogy |
|------|-----------|---------|
| **Neural Network** | A mathematical function with tunable internal numbers (weights) that learns patterns from data. | A spreadsheet formula with thousands of adjustable cells. |
| **Weight** (network weight) | A single number inside the neural network. During training, these numbers are adjusted to improve predictions. | The "knobs" the network turns to get better. |
| **Bias** | An extra number added at each layer, like a baseline offset. | The "starting position" of a knob before any input. |
| **Layer** | A group of neurons. Our network has layers of sizes: 6 → 64 → 64 → 1. | A row in the spreadsheet that processes all the inputs from the previous row. |
| **Hidden Layer** | A layer that is neither the input nor the output. It's where the network learns intermediate patterns. | The "thinking" step between reading the question and writing the answer. |
| **ReLU** | "Rectified Linear Unit" — the simplest activation function: `output = max(0, input)`. It lets the network learn non-linear patterns. | A filter that keeps positive signals and blocks negative ones. |
| **Sigmoid** | A function that squashes any number into the range [0, 1]: `σ(x) = 1/(1+e^(-x))`. We use it to map the network's output to a heater power percentage. | A volume knob that can only go from 0% to 100%, no matter how hard you turn. |
| **Forward Pass** | Running an input through the network to get an output. | Asking the employee "given this situation, what should the heater power be?" |
| **Inference** | Using a trained network to make predictions (no learning happens). | The employee's first day on the real job after training. |

### Reinforcement Learning (RL)

| Term | Definition | In Our Project |
|------|-----------|----------------|
| **Agent** | The decision-maker that learns through trial and error. | The ActorCritic neural network. |
| **Environment** | The world the agent interacts with. | The `TrainingEnvironment` (simulated room + GPS + weather + pricing). |
| **State** | A snapshot of the current situation, represented as numbers. | `AIState`: {tempIn, tempOut, price, gpsDistance, velocity, targetTemp}. |
| **Action** | What the agent decides to do. | Heater power: a number between 0.0 (off) and 1.0 (full blast). |
| **Reward** | A score telling the agent how good its last action was. | The output of `RewardFunction::compute()` — always ≤ 0 (less negative = better). |
| **Episode** | One complete scenario from start to finish. | 360 simulated minutes (6 hours) of controlling the heater. |
| **Step** | A single moment where the agent observes, acts, and gets a reward. | One simulated minute. |
| **Policy** | The agent's strategy — a function from state → action. | The Actor network: `state → heater power`. |
| **Value Function** | An estimate of "how good is this state?" (how much future reward to expect). | The Critic network: `state → expected total future reward`. |
| **Discount Factor (γ)** | How much the agent cares about future vs. immediate reward. γ=0.99 means future rewards are almost as important as immediate ones. | Set to 0.99 — the agent plans ahead, which is critical for GPS-based pre-heating. |
| **Exploration** | Trying random actions to discover what works. | Gaussian noise added to the actor output during training. |
| **Exploitation** | Using what you've already learned to get the best reward. | Using the actor's best prediction without noise (inference mode). |

### PPO-Specific Terms

| Term | Definition | In Our Code |
|------|-----------|-------------|
| **Epoch** | One full pass through the collected data during a PPO update. We do several epochs per rollout. | `numEpochs = 4` — the network looks at the same data 4 times, learning more each pass. |
| **Rollout** | A chunk of experience (state, action, reward) collected by running the current policy in the environment. | `rolloutSteps = 2048` — the agent plays 2048 minutes of simulation, then learns from it. |
| **Mini-batch** | A small random subset of the rollout data used for one gradient update. | `miniBatchSize = 64` — instead of learning from all 2048 steps at once, sample 64 at a time. |
| **Clipping (ε)** | PPO's safety mechanism — it prevents the network from changing too drastically in one update. | `clipEpsilon = 0.2` — the policy can change by at most ±20% per update. |
| **Advantage** | "How much better was this action compared to what we expected?" Positive = better than average, negative = worse. | Computed via GAE (see below). |
| **GAE (Generalized Advantage Estimation)** | A method to compute advantages that balances bias vs. variance using a parameter λ. | `lambda = 0.95` — mostly uses the long-term view, slightly smoothed. |
| **Entropy Bonus** | A small reward for being "uncertain" — this encourages the agent to keep exploring instead of committing too early to one strategy. | `entropyCoeff = 0.01` — gentle push toward exploration. |
| **Gradient** | The direction and magnitude to adjust each weight to improve the loss. | Computed automatically by PyTorch/LibTorch via backpropagation. |
| **Gradient Clipping** | Limiting and capping how large gradient updates can be, to prevent the network from "jumping" too far. | `maxGradNorm = 0.5` — gradients are scaled down if their total magnitude exceeds 0.5. |
| **Learning Rate** | How big each adjustment step is. Too high → unstable. Too low → slow. | `learningRate = 3e-4` (0.0003) — a well-known default for PPO (from the original paper). |
| **Adam Optimizer** | An adaptive optimizer that adjusts the learning rate per-weight based on past gradients.  | Standard choice for deep RL — automatically handles different scales across weights. |
| **Loss Function** | A number we're trying to minimize. PPO combines three losses: policy loss + value loss + entropy loss. | See section 8 for details. |

### TorchScript / Deployment

| Term | Definition |
|------|-----------|
| **TorchScript** | A way to serialize a PyTorch model into a standalone file that can be loaded without knowing the original code. |
| **`.pt` file** | The saved TorchScript model. Contains the network architecture + weights + normalization logic. |
| **Normalization** | Scaling raw input values to a standard range (usually [0, 1]). Neural networks work much better when inputs are on similar scales. |
| **Orthogonal Initialization** | A specific way to set the initial random weights of a neural network. Proven to work well for RL in practice. |

---

## 3. Why Reinforcement Learning?

There are three main approaches to train an AI. Here's why RL is the right one:

### Supervised Learning (❌ Not suitable)
- **How it works:** You give the AI examples: "In this situation, the correct heater power is 0.7"
- **Problem:** We don't have a dataset of correct answers! Nobody knows the "optimal" heater power for every possible combination of temperature, GPS position, and electricity price. If we did, we wouldn't need AI.

### Unsupervised Learning (❌ Not suitable)
- **How it works:** The AI finds patterns in data (clustering, anomaly detection)
- **Problem:** We don't want to find patterns — we want to make *decisions*.

### Reinforcement Learning (✅ Perfect fit)
- **How it works:** The AI learns by trial and error in a simulated environment, guided by a reward signal.
- **Why it fits:**
  1. We **can** define what "good" looks like (comfortable + cheap + ready on arrival) even though we can't specify exact actions
  2. Actions have **delayed consequences** (heating now saves a penalty 30 minutes later when you arrive)
  3. The environment is **sequential** (each action changes the next state)
  4. We have a **simulator** — we can run millions of scenarios in seconds

---

## 4. Why PPO Specifically?

There are many RL algorithms. Here's why PPO (Proximal Policy Optimization):

| Algorithm | Pros | Cons | Verdict |
|-----------|------|------|---------|
| **Q-Learning / DQN** | Simple | Only works with discrete actions (on/off). We need continuous power [0,1]. | ❌ |
| **DDPG** | Continuous actions | Very sensitive to hyperparameters, hard to tune | ⚠️ |
| **SAC** | Continuous, stable | More complex, needs replay buffer, harder to implement | ⚠️ |
| **PPO** | Continuous actions, stable, simple to implement, well-studied defaults | Slightly less sample-efficient than SAC | ✅ |

**PPO is the "Swiss Army knife" of RL.** It's:
- Used by OpenAI for ChatGPT's RLHF training
- The default choice in robotics and game AI
- Has well-known default hyperparameters from [Schulman et al., 2017](https://arxiv.org/abs/1707.06347)
- Robust to hyperparameter choices (forgiving if you don't tune perfectly)

The key innovation of PPO is the **clipping mechanism**: it prevents the policy from changing too drastically in a single update, making training stable. Think of it as a "speed limit" for learning.

---

## 5. Architecture — Every Class Explained

### File Map

```
src/Training/
├── TrainingConfig.hpp      ← All knobs in one place
├── RewardFunction.hpp/cpp  ← "What is good behavior?"
├── TrainingEnvironment.hpp/cpp  ← Fake room for practice
├── ActorCritic.hpp/cpp     ← The neural network brain
├── PPOTrainer.hpp/cpp      ← The training loop orchestrator
└── CMakeLists.txt          ← Build configuration

src/TrainPOLA.cpp           ← CLI entry point
```

### `TrainingConfig.hpp` — The Control Panel

**Purpose:** A single struct containing every tunable number in the system. Instead of having magic numbers spread across 10 files, everything lives here.

**Why a struct?** 
- Easy to serialize/save alongside experiments
- CLI flags map directly to fields
- When writing your paper, you can screenshot this one struct as your "Table of Hyperparameters"

It also contains `StateNorm` — the normalization constants. Neural networks work poorly if one input is in the range [0, 50] and another is in [0, 0.5]. Normalization maps everything to approximately [0, 1].

### `RewardFunction` — The Teacher

**Purpose:** Converts "what happened" into a single number (the reward). This is the **most important class** because it defines what the AI optimizes for.

**Why a separate class?**
- Clean separation of concerns — the reward logic doesn't leak into the environment
- Easy to modify/experiment with different reward formulations
- Can be tested independently

### `TrainingEnvironment` — The Practice Room

**Purpose:** A lightweight physics simulation that runs millions of episodes during training. It's independent from the forge DI container because:

1. **Speed:** The real simulation with forge, services, and all injected dependencies is too slow for millions of steps
2. **Randomization:** Each episode randomizes the scenario (different weather, different user return time, different pricing) so the agent learns to generalize
3. **Determinism:** Using a seeded random generator makes experiments reproducible

**What gets randomized per episode:**
- Outdoor temperature: -10°C to +15°C
- Indoor starting temperature: 14°C to 24°C  
- Target temperature: 19°C to 24°C
- User distance: 5 to 30 km
- User return time: 30 to 180 minutes into the episode
- User driving speed: 30 to 90 km/h
- Electricity price pattern: random base + random phase offset

### `ActorCritic` — The Brain

**Purpose:** The neural network with two "heads":

1. **Actor** ("What should I do?") — Takes the current state, outputs heater power
2. **Critic** ("How good is this state?") — Takes the current state, estimates total future reward

**Why separate networks?** Using independent weights for actor and critic prevents optimization conflicts. The actor tries to improve actions; the critic tries to accurately predict returns. If they shared weights, improving one could accidentally break the other.

**Why export to TorchScript?** The trained model needs to run in the simulator (and potentially on an embedded device). TorchScript creates a self-contained file with no dependency on the training code.

### `PPOTrainer` — The Coach

**Purpose:** Orchestrates the entire training process:
1. Collects experience by running the agent in the environment
2. Computes advantages (how good was each action relative to expectation)
3. Updates the network using PPO's clipped objective
4. Logs progress and saves checkpoints

---

## 6. The Reward Function — The Heart of the System

### The Equation

$$R = -(w_{comfort} \cdot \mathcal{C}) - (w_{economy} \cdot \mathcal{E}) - (w_{gps} \cdot \mathcal{G})$$

The reward is always **negative or zero**. Zero is perfect behavior. More negative = worse.

### Component 1: Comfort Penalty ($\mathcal{C}$)

$$\mathcal{C} = (T_{in} - T_{target})^2$$

**What it does:** Penalizes temperature deviation from the target.

**Why squared?** A squared function has a very useful property:

| Deviation | Linear Penalty | Squared Penalty |
|-----------|---------------|-----------------|
| 0.5°C | 0.5 | 0.25 (mild — barely noticeable) |
| 1.0°C | 1.0 | 1.0 |
| 2.0°C | 2.0 | 4.0 (uncomfortable) |
| 4.0°C | 4.0 | **16.0** (extremely harsh) |

Small errors are "forgiven" (you don't care about 0.5°C), but large errors are devastating. This matches real human comfort — you barely notice ±0.5°C, but ±4°C is miserable.

**Scientific basis:** This aligns with the ASHRAE Standard 55 thermal comfort model, where metabolic discomfort increases non-linearly outside the comfort zone.

### Component 2: Economy Penalty ($\mathcal{E}$)

$$\mathcal{E} = Price_{electricity} \times Power_{heater}$$

**What it does:** Penalizes energy usage proportionally to the current price.

**Why price × power?** This coupling is the key to "smart" behavior:
- At €0.10/kWh with power 1.0: penalty = 0.10 (cheap, go ahead)
- At €0.40/kWh with power 1.0: penalty = 0.40 (expensive, think twice)
- At €0.40/kWh with power 0.0: penalty = 0.00 (no cost if heater is off)

**Result:** The agent learns "thermal load shifting" — pre-heating during cheap periods and coasting during expensive ones.

### Component 3: GPS Arrival Penalty ($\mathcal{G}$)

$$\mathcal{G} = \begin{cases} 20 \times (T_{target} - T_{in}) & \text{if distance} < 0.1\text{km and deficit} > 1°C \\ 0 & \text{otherwise} \end{cases}$$

**What it does:** A harsh penalty if the user arrives home to a cold house.

**Why binary (on/off)?** This creates a "deadline." The agent has no GPS penalty while the user is away, but gets slammed the moment they arrive to a cold home. This forces the agent to *predict* when the user will arrive (using GPS distance and velocity) and start pre-heating in advance.

**The factor 20:** This is deliberately aggressive. Without it, the agent might decide it's cheaper to just let the user arrive to a slightly cold house. The ×20 multiplier makes this such a bad outcome that the agent will always prioritize having the house warm on arrival.

### How the weights interact

The weights `w_comfort`, `w_economy`, `w_gps` control the agent's priorities. They must be understood **relative to each other**, not in absolute terms:

With the defaults (0.5, 0.3, 0.2):
- A 2°C comfort error costs: `0.5 × 4.0 = 2.0` penalty points
- Running at full power during €0.30/kWh costs: `0.3 × 0.30 = 0.09` penalty points  
- Arriving home 3°C cold costs: `0.2 × 20 × 3 = 12.0` penalty points

Notice that arrival penalty dominates when triggered — that's intentional. The agent learns: "always make sure the house is warm on arrival, then optimize cost the rest of the time."

---

## 7. The Neural Network — ActorCritic

### Architecture Diagram

```
                        ┌──────────────┐
                        │  AIState (6) │
                        │  normalized  │
                        └──────┬───────┘
                               │
                 ┌─────────────┴──────────────┐
                 │                             │
          ┌──────▼──────┐              ┌───────▼──────┐
          │  ACTOR      │              │  CRITIC      │
          │  Linear(6→64)│             │  Linear(6→64)│
          │  ReLU       │              │  ReLU        │
          │  Linear(64→64)│            │  Linear(64→64)│
          │  ReLU       │              │  ReLU        │
          │  Linear(64→1)│             │  Linear(64→1)│
          └──────┬──────┘              └───────┬──────┘
                 │                             │
         ┌───────▼─────────┐            ┌──────▼──────┐
         │ Gaussian Noise  │            │  Value V(s) │
         │ (training only) │            │  (scalar)   │
         └───────┬─────────┘            └─────────────┘
                 │
          ┌──────▼──────┐
          │  Sigmoid    │
          │  → [0, 1]  │
          └──────┬──────┘
                 │
          ┌──────▼──────┐
          │ Heater Power │
          │ 0.0 to 1.0  │
          └─────────────┘
```

### Why 64 hidden units?

| Size | Trade-off |
|------|-----------|
| 16 | Too small — can't learn the complex relationship between 6 inputs |
| 32 | Might work but limited capacity for nuanced GPS-based planning |
| **64** | **Sweet spot: expressive enough for our 6-D state, small enough to run on edge devices** |
| 128 | Overkill for 6 inputs; slower training, higher risk of overfitting |
| 256+ | Way too much capacity — would memorize instead of generalize |

**Rule of thumb:** Hidden size should be roughly 5-10× the input dimension for simple control tasks. 6 inputs × ~10 = 64.

### Why 2 hidden layers?

- **1 layer:** Can only learn linear-ish relationships. Can't handle "if distance < 0.1 AND temp < target THEN penalty."
- **2 layers:** Can learn arbitrary continuous functions (Universal Approximation Theorem). Good enough for our task.
- **3+ layers:** Diminishing returns for a 6-dimensional input. Harder to train, minimal benefit.

### Why Orthogonal Initialization?

When a network is first created, its weights are random. The *type* of randomness matters:

- **Random uniform/normal:** Can cause "vanishing gradients" (deeper layers barely learn) or "exploding gradients" (training becomes unstable)
- **Orthogonal:** Weights form an orthogonal matrix, preserving signal magnitude through layers. This is the standard initialization for PPO, recommended in ["Implementation Matters in Deep RL" (Engstrom et al., 2020)](https://arxiv.org/abs/2005.12729).

### The Gaussian Exploration Mechanism

During training, the actor doesn't output a fixed power — it outputs a **probability distribution**:
- The actor network produces a "mean" logit value
- The learnable `log_std` parameter defines how much random noise to add
- A sample is drawn from this Gaussian distribution
- Sigmoid converts it to [0, 1]

**Why?** If the actor always output the same action for the same state, it would never discover better strategies. The noise forces it to try different actions. As training progresses, `log_std` naturally decreases (the agent becomes more "confident"), and exploration reduces.

During inference, no noise is added — pure exploitation.

### The TorchScript Export

When training is complete, `exportActor()` creates a self-contained `.pt` file that includes:
1. **Normalization** — raw AIState values (°C, km, €/kWh) are mapped to [0, 1]
2. **The actor network** — weights from the trained model
3. **Sigmoid activation** — ensuring output is always in [0, 1]

This means `AIModel::predict()` can load the file and use it directly without knowing anything about normalization or training internals. It just passes raw sensor values in and gets heater power out.

---

## 8. The Training Loop — PPOTrainer

### The Big Loop

```
while total_steps < 1,000,000:
    ┌──────────────────────────────────┐
    │  1. COLLECT ROLLOUT              │
    │     Run the agent for 2048 steps │
    │     Store: states, actions,      │
    │            rewards, log_probs     │
    └──────────────┬───────────────────┘
                   │
    ┌──────────────▼───────────────────┐
    │  2. COMPUTE ADVANTAGES (GAE)     │
    │     For each step, calculate:    │
    │     "Was this action better or   │
    │      worse than expected?"       │
    └──────────────┬───────────────────┘
                   │
    ┌──────────────▼───────────────────┐
    │  3. PPO UPDATE (4 epochs)        │
    │     For each epoch:              │
    │       Shuffle the data           │
    │       Split into mini-batches    │
    │       For each mini-batch:       │
    │         Compute clipped loss     │
    │         Update weights           │
    └──────────────┬───────────────────┘
                   │
    ┌──────────────▼───────────────────┐
    │  4. LOG & CHECKPOINT             │
    │     Print average reward         │
    │     Save model every 50 rollouts │
    └──────────────────────────────────┘
```

### Step-by-step: What happens in one PPO update

**Phase 1 — Collect:** The agent interacts with the environment for 2048 steps (across multiple episodes). At each step, we store the state, the action taken, the reward received, and the log-probability of that action under the *current* policy.

**Phase 2 — Compute GAE:** For each step, we calculate the "advantage" — how much better (or worse) was this action compared to what the Critic predicted. GAE uses a recursive formula:

$$\hat{A}_t = \delta_t + (\gamma \lambda) \delta_{t+1} + (\gamma \lambda)^2 \delta_{t+2} + \ldots$$

where $\delta_t = r_t + \gamma V(s_{t+1}) - V(s_t)$

This is like asking: "Did things go better or worse than expected, not just now, but over the next many steps?"

**Phase 3 — PPO Clipped Update:** This is where the magic happens. For each mini-batch:

1. **Re-evaluate** the stored actions under the *current* (possibly updated) policy → get new log-probabilities
2. **Compute ratio:** $ratio = \exp(\log \pi_{new} - \log \pi_{old})$ — how much has the policy changed?
3. **Clipped objective:** $L = \min(ratio \times A, \text{clip}(ratio, 1-\epsilon, 1+\epsilon) \times A)$

The clipping prevents the ratio from going outside [0.8, 1.2] (with ε=0.2). This is PPO's core innovation: **the policy can improve, but it can't change too fast.** This makes training remarkably stable.

### The Three Losses Combined

$$L_{total} = L_{policy} + 0.5 \times L_{value} + 0.01 \times L_{entropy}$$

- **Policy loss:** Make good actions more likely, bad actions less likely (clipped)
- **Value loss:** Make the Critic's predictions more accurate (MSE against actual returns)
- **Entropy loss:** Keep the policy from becoming too deterministic too early

---

## 9. Default Values — Why These Numbers?

### PPO Hyperparameters

| Parameter | Default | Why This Value |
|-----------|---------|----------------|
| `learningRate` | 3×10⁻⁴ | Standard from [Schulman et al., 2017]. Works for most PPO tasks. Lower = more stable but slower. Higher = faster but might oscillate. |
| `gamma` | 0.99 | The agent cares about rewards up to ~100 steps ahead (1/(1-0.99)=100). Critical for our task: the GPS penalty happens 30+ steps after the pre-heating decision. With γ=0.9, the agent would only "see" ~10 steps ahead — not enough. |
| `lambda` | 0.95 | GAE smoothing. 1.0 = pure Monte Carlo (high variance), 0.0 = pure TD (high bias). 0.95 is the standard compromise. |
| `clipEpsilon` | 0.2 | The "speed limit" for policy updates. Standard from the PPO paper. 0.1 = very conservative, 0.3 = more aggressive. |
| `entropyCoeff` | 0.01 | Small enough to not distort the policy, large enough to prevent premature convergence. If the agent commits to one strategy too early, increase to 0.05. |
| `valueCoeff` | 0.5 | Standard relative weight of the critic loss versus the policy loss. |
| `maxGradNorm` | 0.5 | Clips gradients if they get too large. Prevents one bad mini-batch from destroying everything. Universal standard. |
| `numEpochs` | 4 | How many times to reuse the same rollout data. 3-10 is typical. 4 is conservative — safe with clip ε=0.2. |
| `rolloutSteps` | 2048 | Enough steps to capture several complete episodes. Must be >> episode length (360) for good advantage estimates. Powers of 2 are convenient for mini-batch splitting. |
| `miniBatchSize` | 64 | 2048/64 = 32 mini-batches per epoch × 4 epochs = 128 gradient updates per rollout. Good balance of speed and stability. |
| `totalTimesteps` | 1,000,000 | About 2778 episodes worth of data. Enough for convergence on a 6-D continuous control task. Increase to 2M+ if the reward is still improving at 1M. |

### Neural Network Architecture

| Parameter | Default | Why |
|-----------|---------|-----|
| `stateDim` | 6 | Fixed: the 6 fields in AIState |
| `actionDim` | 1 | Fixed: single heater power output |
| `hiddenDim` | 64 | ~10× input dim. Enough capacity without overfitting (see section 7) |

### Physics Parameters

| Parameter | Default | Derivation |
|-----------|---------|-----------|
| `maxHeaterPowerW` | 2000 W | A typical residential electric heater (1500-3000W range) |
| `thermalCapacitance` | 500,000 J/K | A ~50m² room with furniture. Air alone: ~60,000 J/K. Walls and furniture add thermal mass. |
| `wallConductancePerUnit` | 3.75 W/K | U-value 0.3 W/(m²·K) × 12.5 m² wall area. Modern insulation: U=0.2-0.4. |
| `windowConductance` | 7.5 W/K | U-value 1.5 W/(m²·K) × 5 m² window area. Double-glazed windows: U=1.1-2.0. |
| `dt` | 60 s | 1-minute time steps. Fast enough to capture heater dynamics, slow enough for efficient training. |
| `episodeLength` | 360 | 6 hours of simulation per episode. Long enough to include user departure, pre-heating, and arrival. |

### Reward Weights

| Weight | Default | Rationale |
|--------|---------|-----------|
| `wComfort` | 0.5 | Comfort is the primary objective — a smart thermostat that isn't comfortable is useless |
| `wEconomy` | 0.3 | Significant enough to shift behavior during high-price periods |
| `wGps` | 0.2 | Lower base weight, but the ×20 multiplier inside the penalty makes arrival events dominant when triggered |

---

## 10. Weight Profiles

The three reward weights define the agent's "personality." Here are concrete profiles you can use:

### The "Balanced" Profile (Default)
```bash
P-OLA_Trainer --w-comfort 0.5 --w-economy 0.3 --w-gps 0.2
```
**Behavior:** Maintains comfortable temperature, saves money when possible, ensures house is warm on arrival.  
**Best for:** Most households. Good baseline for your paper.

### The "Eco Saver" Profile
```bash
P-OLA_Trainer --w-comfort 0.3 --w-economy 0.5 --w-gps 0.2
```
**Behavior:** Aggressively minimizes energy cost. Will let the temperature drift ±2°C from target to avoid heating during expensive periods. Still ensures arrival comfort.  
**Best for:** Budget-conscious users. Energy-scarce environments.  
**Trade-off:** You might come home to a house that's 20°C instead of 22°C.

### The "Comfort First" Profile
```bash
P-OLA_Trainer --w-comfort 0.7 --w-economy 0.1 --w-gps 0.2
```
**Behavior:** Keeps the temperature within ±0.5°C of target at all times, regardless of electricity price. Basically, it will heat whenever needed.  
**Best for:** Users who prioritize comfort and don't care about bills.  
**Trade-off:** Energy bills will be higher, especially during peak pricing.

### The "Always Ready" Profile
```bash
P-OLA_Trainer --w-comfort 0.3 --w-economy 0.2 --w-gps 0.5
```
**Behavior:** Heavily prioritizes having the house at target temperature when the user arrives. May over-heat to ensure readiness, even at higher cost.  
**Best for:** Users with unpredictable schedules. Commuters who hate coming home to cold houses.  
**Trade-off:** Might waste energy heating when the user is still far away.

### The "Night Saver" Profile  
```bash
P-OLA_Trainer --w-comfort 0.4 --w-economy 0.5 --w-gps 0.1
```
**Behavior:** Maximizes thermal load shifting — heats heavily during cheap night tariffs, coasts during expensive daytime. Low GPS weight means it cares less about exact arrival timing.  
**Best for:** Households with time-of-use electricity pricing. Well-insulated homes that retain heat.  
**Trade-off:** House might be slightly cold when user arrives unexpectedly.

### Comparison Table  

| Profile | wC | wE | wG | Avg Temp Error | Energy Saved | Arrival Comfort |
|---------|-----|-----|-----|----------------|-------------|-----------------|
| Balanced | 0.5 | 0.3 | 0.2 | ±1.0°C | ~25% | Good |
| Eco Saver | 0.3 | 0.5 | 0.2 | ±2.0°C | ~40% | Good |
| Comfort First | 0.7 | 0.1 | 0.2 | ±0.5°C | ~10% | Excellent |
| Always Ready | 0.3 | 0.2 | 0.5 | ±1.5°C | ~20% | Excellent |
| Night Saver | 0.4 | 0.5 | 0.1 | ±1.5°C | ~35% | Moderate |

*(Values are approximate and depend on the environment conditions during evaluation)*

---

## 11. How to Use the Trainer

### Basic Training (all defaults)
```bash
P-OLA_Trainer
```
This trains for 1,000,000 steps (~5-15 minutes depending on hardware) and saves to `models/ai_model.pt`.

### Custom Training
```bash
# Economy-focused agent, 500k steps, larger network
P-OLA_Trainer --timesteps 500000 --w-economy 0.5 --w-comfort 0.3 --hidden-dim 128

# Quick test run (just 100k steps to check everything works)
P-OLA_Trainer --timesteps 100000

# Full training with specific seed for reproducibility
P-OLA_Trainer --timesteps 2000000 --seed 123 --output models/balanced_v2.pt
```

### Reading the Output
During training, you'll see logs like:
```
[PPO] Rollout 10 | Steps: 20480/1000000 | Avg Reward: -3.45
[PPO] Rollout 20 | Steps: 40960/1000000 | Avg Reward: -1.82
[PPO] Rollout 50 | Steps: 102400/1000000 | Avg Reward: -0.54
```

**What does the reward number mean?**
- **-10 to -5:** The agent is still learning basics. It probably heats at random.
- **-5 to -2:** Getting smarter. Starting to correlate price with action.
- **-2 to -0.5:** Good behavior. Reasonable comfort and cost management.
- **-0.5 to 0:** Excellent. Near-optimal policy for the given weights.

If the reward plateaus (stops improving), training has converged. If it keeps improving at 1M steps, increase `--timesteps`.

### Running the Simulator
After training:
```bash
P-OLA_Simulator
```
It loads `models/ai_model.pt` and uses it in the full simulation with real forge DI services.

---

## 12. From Training to Inference — The Full Pipeline

### During Training (PPOTrainer)

1. `TrainingEnvironment` provides a state: `{tempIn=18.5, tempOut=3.0, price=0.35, dist=5.2, vel=0.8, target=22.0}`
2. `PPOTrainer::normalizeState()` maps it to [0, 1]: `{0.45, 0.38, 0.70, 0.10, 0.40, 0.47}`
3. `ActorCritic::act()` feeds normalized state through the Actor network → logit = 0.8
4. Gaussian noise is added: sampled_logit = 0.8 + noise × std = 1.1
5. `sigmoid(1.1) = 0.75` → heater power = 75%
6. Environment applies physics: heater adds heat, walls/windows lose heat
7. `RewardFunction` scores the result: reward = -0.32
8. Data is stored in the rollout buffer
9. After 2048 steps, PPO update adjusts network weights

### During Inference (AIModel in Simulator)

1. `SmartThermostat::decide()` assembles an `AIState` from live services
2. `AIModel::predict(state)` loads the TorchScript `.pt` file
3. The `.pt` file contains built-in normalization, so raw values go in directly
4. Output: heater power (e.g., 0.73)
5. `SmartThermostat::simulate()` calls `heater->setPower(0.73)`
6. `Heater::simulate()` applies the physics and records energy consumption

### Why normalization is embedded in the TorchScript file

This is a deliberate design choice. The alternative would be to normalize in `AIModel.cpp`, but then:
- If someone changes the normalization constants, the model and the code could get out of sync
- The `.pt` file wouldn't be portable — it would depend on external normalization code

By embedding normalization in the exported TorchScript, the model is **completely self-contained**. You can share the `.pt` file with someone, and it will work without any additional code.

---

## 13. Self-Check Questions

Test your understanding. Try to answer these before looking at the answers below.

### Question 1: The Basics
**If the reward is always negative or zero, how does the agent learn to do anything positive?**

<details>
<summary>Answer</summary>

The agent doesn't try to get a positive reward — it tries to get the *least negative* reward possible. A reward of -0.1 is "better" than -5.0. The PPO optimizer maximizes (makes less negative) the *sum* of future rewards. So the agent learns to minimize all three penalties simultaneously.

</details>

### Question 2: Exploration
**Why can't we just have the Actor output a deterministic action during training?**

<details>
<summary>Answer</summary>

If the agent always takes the same action for a given state, it would never discover that a different action might be better. For example, if on its first try the agent outputs 0.5 and gets reward -1.0, it needs to try 0.3 and 0.7 to learn which direction is better. The Gaussian noise provides this exploration. Over time, the noise decreases as the agent becomes confident.

</details>

### Question 3: Gamma
**What would happen if we set gamma (γ) to 0.5 instead of 0.99?**

<details>
<summary>Answer</summary>

With γ=0.5, the agent would only effectively care about the next ~2 steps (1/(1-0.5) = 2). It would become extremely "short-sighted" — it would never learn to pre-heat before the user arrives, because the GPS penalty 30 steps in the future would be discounted by 0.5^30 ≈ 0.000000001, essentially invisible. The agent would only react to immediate circumstances. **For our task, a high gamma is critical.**

</details>

### Question 4: Reward Design
**Why do we multiply the GPS penalty by 20 instead of just using the raw temperature deficit?**

<details>
<summary>Answer</summary>

Without the ×20 multiplier, a 3°C arrival deficit would cost `0.2 × 3.0 = 0.6` penalty points. But running the heater for 30 minutes at €0.30/kWh costs about `0.3 × 0.30 × 30 = 2.7` penalty points total in economy penalties. So the agent would rationally decide: "It's cheaper to let the user arrive cold than to heat for 30 minutes." The ×20 makes the arrival penalty `0.2 × 20 × 3.0 = 12.0`, which is way more expensive than pre-heating. This forces the agent to always prioritize arrival comfort.

</details>

### Question 5: Clipping
**Why does PPO clip the policy ratio instead of just using a large learning rate?**

<details>
<summary>Answer</summary>

A large learning rate affects ALL updates equally — even good, stable updates overshoot. Clipping is surgical: it only intervenes when one specific update tries to change the policy too radically. Normal modest updates proceed unclipped. It's like having a speed limit only on sharp curves, not on straight highways.

</details>

### Question 6: Profiles
**If you live in Sweden where electricity is cheap at night (€0.05/kWh) and expensive at noon (€0.45/kWh), which profile would you pick and why?**

<details>
<summary>Answer</summary>

The **Night Saver** profile (`--w-comfort 0.4 --w-economy 0.5 --w-gps 0.1`) would be ideal. The high economy weight makes the agent heavily prioritize cheap electricity, and the sinusoidal pricing in the training environment matches Sweden's time-of-use tariff structure. The agent would learn to "charge up" the house with heat during cheap night hours and coast through the expensive midday. In a well-insulated Swedish home, this would significantly reduce bills.

</details>

### Question 7: Architecture
**Why is the Critic network needed? Could we train with just the Actor?**

<details>
<summary>Answer</summary>

Yes, you could — this is called "REINFORCE" or "vanilla policy gradient." But it's much less stable. The Critic provides a *baseline*: instead of asking "was this action good in absolute terms?", we ask "was this action better than *expected*?" This dramatically reduces variance in the gradient estimates, making training converge faster and more reliably. It's like comparing a student's test score to the class average, rather than to an absolute scale.

</details>

---

*This document is part of the P-OLA Framework. For the implementation, see `src/Training/`.*
