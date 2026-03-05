/**
 * @file ActorCriticPOLA.cpp
 * @brief Implementation of the Actor-Critic neural network for PPO.
 */

#include "ActorCriticPOLA.hpp"

#include <filesystem>

#include "../Utils/LibTorch.hpp"

using namespace POLA::Training::Utils;
using namespace POLA::Training::POLA;

// ============================================================================
// ActorCriticPOLAImpl
// ============================================================================

ActorCriticPOLAImpl::ActorCriticPOLAImpl(int64_t stateDim, int64_t actionDim,
                                         int64_t hiddenDim)
    : _stateDim(stateDim), _actionDim(actionDim)
{
    // ---- Actor network ----
    actor_fc1 =
        register_module("actor_fc1", torch::nn::Linear(stateDim, hiddenDim));
    actor_fc2 =
        register_module("actor_fc2", torch::nn::Linear(hiddenDim, hiddenDim));
    actor_out =
        register_module("actor_out", torch::nn::Linear(hiddenDim, actionDim));

    // ---- Critic network ----
    critic_fc1 =
        register_module("critic_fc1", torch::nn::Linear(stateDim, hiddenDim));
    critic_fc2 =
        register_module("critic_fc2", torch::nn::Linear(hiddenDim, hiddenDim));
    critic_out = register_module("critic_out", torch::nn::Linear(hiddenDim, 1));

    // ---- Learnable log standard deviation ----
    // Initialized to -0.5 → std ≈ 0.6 in logit space for moderate exploration
    log_std = register_parameter("log_std", torch::full({actionDim}, -0.5));

    // ---- Orthogonal initialization ----
    // Hidden layers: gain = sqrt(2) (standard for ReLU activations)
    // Output layers: small gain for stable initial policy
    orthogonalInit(actor_fc1, std::sqrt(2.0));
    orthogonalInit(actor_fc2, std::sqrt(2.0));
    orthogonalInit(actor_out, 0.01);

    orthogonalInit(critic_fc1, std::sqrt(2.0));
    orthogonalInit(critic_fc2, std::sqrt(2.0));
    orthogonalInit(critic_out, 1.0);
}

std::tuple<torch::Tensor, torch::Tensor>
ActorCriticPOLAImpl::forward(torch::Tensor state)
{
    // Actor pathway → action logit (unbounded)
    auto a = torch::relu(actor_fc1->forward(state));
    a = torch::relu(actor_fc2->forward(a));
    auto actionLogit = actor_out->forward(a);

    // Critic pathway → state value V(s)
    auto c = torch::relu(critic_fc1->forward(state));
    c = torch::relu(critic_fc2->forward(c));
    auto value = critic_out->forward(c);

    return {actionLogit, value};
}

std::tuple<torch::Tensor, torch::Tensor, torch::Tensor>
ActorCriticPOLAImpl::act(torch::Tensor state)
{
    torch::NoGradGuard noGrad;

    auto [actionLogit, value] = forward(state);

    // Sample from Gaussian in logit space: a ~ N(mean_logit, exp(log_std))
    auto std = log_std.exp().expand_as(actionLogit);
    auto noise = torch::randn_like(actionLogit);
    auto sampledLogit = actionLogit + noise * std;

    // Log probability of the sampled action under the policy
    auto logProb = normalLogProb(sampledLogit, actionLogit, std).sum(-1);

    return {sampledLogit.squeeze(), logProb.squeeze(), value.squeeze()};
}

std::tuple<torch::Tensor, torch::Tensor, torch::Tensor>
ActorCriticPOLAImpl::evaluate(torch::Tensor states, torch::Tensor actionLogits)
{
    auto [meanLogits, values] = forward(states);

    auto std = log_std.exp().expand_as(meanLogits);
    auto logProbs = normalLogProb(actionLogits, meanLogits, std).sum(-1);
    auto entropy = normalEntropy(std).sum(-1);

    return {logProbs, values.squeeze(-1), entropy};
}

void ActorCriticPOLAImpl::exportActor(const std::string& path)
{
    namespace fs = std::filesystem;

    // Ensure output directory exists
    const fs::path filePath(path);
    if (filePath.has_parent_path())
    {
        fs::create_directories(filePath.parent_path());
    }

    torch::NoGradGuard noGrad;

    // Build a self-contained TorchScript module for inference.
    // This embeds the normalization constants and sigmoid activation,
    // so AIModel doesn't need to know about training internals.
    torch::jit::script::Module jitModule("ThermostatPolicyPOLA");

    // Register actor parameters (moved to CPU for portability)
    jitModule.register_parameter("actor_fc1_w", actor_fc1->weight.clone().cpu(),
                                 false);
    jitModule.register_parameter("actor_fc1_b", actor_fc1->bias.clone().cpu(),
                                 false);
    jitModule.register_parameter("actor_fc2_w", actor_fc2->weight.clone().cpu(),
                                 false);
    jitModule.register_parameter("actor_fc2_b", actor_fc2->bias.clone().cpu(),
                                 false);
    jitModule.register_parameter("actor_out_w", actor_out->weight.clone().cpu(),
                                 false);
    jitModule.register_parameter("actor_out_b", actor_out->bias.clone().cpu(),
                                 false);

    // Define forward with built-in normalization + sigmoid output.
    // Normalization constants must match StateNorm in TrainingConfig.hpp.
    //
    // Input:  x [batch, 42] — raw AIState values
    // Output: [batch, 1]   — heater power in [0, 1]
    //
    // State layout (column order, matching normalizeState() in trainers):
    //   0: tempIn            [5, 35] °C       → (x - 5.0) / 30.0
    //   1: electricityPrice  [0, 0.50] $/kWh  → x / 0.50
    //   2: userDistanceKm    [0, 50] km        → x / 50.0
    //   3: userVelocityKmMin [0, 2] km/min     → x / 2.0
    //   4..15 (pairs): weather.forecast[0..5]
    //       even cols: outdoorTemp  [-20, 40]  → (x + 20.0) / 60.0
    //       odd  cols: sunlight lux [0, 100k]  → x / 100000.0
    //   16: userPreferences.minTemperature [15, 30] → (x - 15.0) / 15.0
    //   17: userPreferences.maxTemperature [15, 30] → (x - 15.0) / 15.0
    //   18..41: userSchedule.userPresent[0..23]  → 0.0 or 1.0
    jitModule.define(R"(
        def forward(self, x):
            # ---- Scalar features ----
            t_in    = (x[:, 0:1]  - 5.0)  / 30.0
            price   =  x[:, 1:2]          / 0.50
            dist    =  x[:, 2:3]          / 50.0
            vel     =  x[:, 3:4]          / 2.0

            # ---- Weather forecast (6 hours × 2 features) ----
            f0_t    = (x[:,  4:5]  + 20.0) / 60.0
            f0_s    =  x[:,  5:6]          / 100000.0
            f1_t    = (x[:,  6:7]  + 20.0) / 60.0
            f1_s    =  x[:,  7:8]          / 100000.0
            f2_t    = (x[:,  8:9]  + 20.0) / 60.0
            f2_s    =  x[:,  9:10] / 100000.0
            f3_t    = (x[:, 10:11] + 20.0) / 60.0
            f3_s    =  x[:, 11:12] / 100000.0
            f4_t    = (x[:, 12:13] + 20.0) / 60.0
            f4_s    =  x[:, 13:14] / 100000.0
            f5_t    = (x[:, 14:15] + 20.0) / 60.0
            f5_s    =  x[:, 15:16] / 100000.0

            # ---- User preferences ----
            pref_min = (x[:, 16:17] - 15.0) / 15.0
            pref_max = (x[:, 17:18] - 15.0) / 15.0

            # ---- Occupancy schedule (24 hours, already 0/1) ----
            schedule = x[:, 18:42]

            x_norm = torch.cat([
                t_in, price, dist, vel,
                f0_t, f0_s, f1_t, f1_s, f2_t, f2_s,
                f3_t, f3_s, f4_t, f4_s, f5_t, f5_s,
                pref_min, pref_max,
                schedule
            ], dim=1)

            # Actor network (2-layer MLP with ReLU)
            h = torch.relu(torch.matmul(x_norm, self.actor_fc1_w.t()) + self.actor_fc1_b)
            h = torch.relu(torch.matmul(h, self.actor_fc2_w.t()) + self.actor_fc2_b)
            logit = torch.matmul(h, self.actor_out_w.t()) + self.actor_out_b

            # Sigmoid maps logit → heater power in [0, 1]
            return torch.sigmoid(logit)
    )");

    jitModule.save(path);
}

void ActorCriticPOLAImpl::saveCheckpoint(const std::string& path)
{
    namespace fs = std::filesystem;
    const fs::path filePath(path);
    if (filePath.has_parent_path())
    {
        fs::create_directories(filePath.parent_path());
    }
    torch::save(std::make_shared<ActorCriticPOLAImpl>(*this), path);
    std::cout << "[ActorCriticPOLA] Checkpoint saved to: " << path << std::endl;
}

void ActorCriticPOLAImpl::loadCheckpoint(const std::string& path)
{
    auto loaded = std::make_shared<ActorCriticPOLAImpl>(_stateDim, _actionDim);
    torch::load(loaded, path);
    // Copy parameters from the loaded module into this one
    auto srcParams = loaded->named_parameters();
    auto dstParams = this->named_parameters();
    torch::NoGradGuard noGrad;
    for (auto& p : srcParams)
    {
        if (dstParams.contains(p.key()))
        {
            dstParams[p.key()].copy_(p.value());
        }
    }
    std::cout << "[ActorCriticPOLA] Checkpoint loaded from: " << path << std::endl;
}
