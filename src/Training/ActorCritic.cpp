/**
 * @file ActorCritic.cpp
 * @brief Implementation of the Actor-Critic neural network for PPO.
 */

#include "ActorCritic.hpp"
#include "TrainingConfig.hpp"

#include <cmath>
#include <filesystem>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace POLA::Training;

// ============================================================================
// Gaussian distribution utilities (not provided by LibTorch C++ API)
// ============================================================================

/**
 * @brief Log probability of x under a Gaussian N(mean, std^2).
 *        log p(x) = -0.5 * ((x - mean)/std)^2 - log(std) - 0.5 * log(2π)
 */
torch::Tensor normalLogProb(
    const torch::Tensor& x,
    const torch::Tensor& mean,
    const torch::Tensor& std)
{
    const auto var = std * std;
    return -0.5 * ((x - mean).pow(2) / (var + 1e-8))
           - std.log()
           - 0.5 * std::log(2.0 * M_PI);
}

/**
 * @brief Entropy of a Gaussian N(mean, std^2).
 *        H = 0.5 * (1 + log(2π)) + log(std)
 */
torch::Tensor normalEntropy(const torch::Tensor& std) {
    return 0.5 + 0.5 * std::log(2.0 * M_PI) + std.log();
}

/**
 * @brief Orthogonal weight initialization (standard for PPO).
 */
void orthogonalInit(torch::nn::Linear& layer, double gain = 1.0) {
    torch::nn::init::orthogonal_(layer->weight, gain);
    if (layer->bias.defined()) {
        torch::nn::init::zeros_(layer->bias);
    }
}

// ============================================================================
// ActorCriticImpl
// ============================================================================

ActorCriticImpl::ActorCriticImpl(int64_t stateDim, int64_t actionDim, int64_t hiddenDim)
    : _stateDim(stateDim)
    , _actionDim(actionDim)
{
    // ---- Actor network ----
    actor_fc1 = register_module("actor_fc1", torch::nn::Linear(stateDim, hiddenDim));
    actor_fc2 = register_module("actor_fc2", torch::nn::Linear(hiddenDim, hiddenDim));
    actor_out = register_module("actor_out", torch::nn::Linear(hiddenDim, actionDim));

    // ---- Critic network ----
    critic_fc1 = register_module("critic_fc1", torch::nn::Linear(stateDim, hiddenDim));
    critic_fc2 = register_module("critic_fc2", torch::nn::Linear(hiddenDim, hiddenDim));
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

std::tuple<torch::Tensor, torch::Tensor> ActorCriticImpl::forward(torch::Tensor state)
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

std::tuple<torch::Tensor, torch::Tensor, torch::Tensor> ActorCriticImpl::act(torch::Tensor state)
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

std::tuple<torch::Tensor, torch::Tensor, torch::Tensor> ActorCriticImpl::evaluate(
    torch::Tensor states, torch::Tensor actionLogits)
{
    auto [meanLogits, values] = forward(states);

    auto std = log_std.exp().expand_as(meanLogits);
    auto logProbs = normalLogProb(actionLogits, meanLogits, std).sum(-1);
    auto entropy = normalEntropy(std).sum(-1);

    return {logProbs, values.squeeze(-1), entropy};
}

void ActorCriticImpl::exportActor(const std::string& path)
{
    namespace fs = std::filesystem;

    // Ensure output directory exists
    const fs::path filePath(path);
    if (filePath.has_parent_path()) {
        fs::create_directories(filePath.parent_path());
    }

    torch::NoGradGuard noGrad;

    // Build a self-contained TorchScript module for inference.
    // This embeds the normalization constants and sigmoid activation,
    // so AIModel doesn't need to know about training internals.
    torch::jit::script::Module jitModule("ThermostatPolicy");

    // Register actor parameters (moved to CPU for portability)
    jitModule.register_parameter("actor_fc1_w", actor_fc1->weight.clone().cpu(), false);
    jitModule.register_parameter("actor_fc1_b", actor_fc1->bias.clone().cpu(), false);
    jitModule.register_parameter("actor_fc2_w", actor_fc2->weight.clone().cpu(), false);
    jitModule.register_parameter("actor_fc2_b", actor_fc2->bias.clone().cpu(), false);
    jitModule.register_parameter("actor_out_w", actor_out->weight.clone().cpu(), false);
    jitModule.register_parameter("actor_out_b", actor_out->bias.clone().cpu(), false);

    // Define forward with built-in normalization + sigmoid output.
    // Normalization constants must match StateNorm in TrainingConfig.hpp.
    //
    // Input:  x [batch, 6] — raw AIState values
    // Output: [batch, 1]   — heater power in [0, 1]
    jitModule.define(R"(
        def forward(self, x):
            # Normalize each feature to approximately [0, 1]
            t_in    = (x[:, 0:1] - 5.0)  / 30.0
            t_out   = (x[:, 1:2] + 20.0) / 60.0
            price   = x[:, 2:3] / 0.50
            dist    = x[:, 3:4] / 50.0
            vel     = x[:, 4:5] / 2.0
            target  = (x[:, 5:6] - 15.0) / 15.0
            x_norm  = torch.cat([t_in, t_out, price, dist, vel, target], dim=1)

            # Actor network (2-layer MLP with ReLU)
            h = torch.relu(torch.matmul(x_norm, self.actor_fc1_w.t()) + self.actor_fc1_b)
            h = torch.relu(torch.matmul(h, self.actor_fc2_w.t()) + self.actor_fc2_b)
            logit = torch.matmul(h, self.actor_out_w.t()) + self.actor_out_b

            # Sigmoid maps logit → heater power in [0, 1]
            return torch.sigmoid(logit)
    )");

    jitModule.save(path);
}
