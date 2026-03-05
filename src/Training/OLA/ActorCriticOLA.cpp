/**
 * @file ActorCriticOLA.cpp
 * @brief Implementation of the GPS-free Actor-Critic network.
 */

#include "ActorCriticOLA.hpp"

#include <filesystem>
#include <iostream>

#include "../Utils/LibTorch.hpp"

using namespace POLA::Training::Utils;
using namespace POLA::Training::OLA;

// ============================================================================
// ActorCriticOLAImpl
// ============================================================================

ActorCriticOLAImpl::ActorCriticOLAImpl(int64_t stateDim, int64_t actionDim,
                                       int64_t hiddenDim)
    : _stateDim(stateDim), _actionDim(actionDim)
{
    actor_fc1 =
        register_module("actor_fc1", torch::nn::Linear(stateDim, hiddenDim));
    actor_fc2 =
        register_module("actor_fc2", torch::nn::Linear(hiddenDim, hiddenDim));
    actor_out =
        register_module("actor_out", torch::nn::Linear(hiddenDim, actionDim));

    critic_fc1 =
        register_module("critic_fc1", torch::nn::Linear(stateDim, hiddenDim));
    critic_fc2 =
        register_module("critic_fc2", torch::nn::Linear(hiddenDim, hiddenDim));
    critic_out = register_module("critic_out", torch::nn::Linear(hiddenDim, 1));

    log_std = register_parameter("log_std", torch::full({actionDim}, -0.5));

    orthogonalInit(actor_fc1, std::sqrt(2.0));
    orthogonalInit(actor_fc2, std::sqrt(2.0));
    orthogonalInit(actor_out, 0.01);
    orthogonalInit(critic_fc1, std::sqrt(2.0));
    orthogonalInit(critic_fc2, std::sqrt(2.0));
    orthogonalInit(critic_out, 1.0);
}

std::tuple<torch::Tensor, torch::Tensor>
ActorCriticOLAImpl::forward(const torch::Tensor& state)
{
    auto a = torch::relu(actor_fc1->forward(state));
    a = torch::relu(actor_fc2->forward(a));
    auto actionLogit = actor_out->forward(a);

    auto c = torch::relu(critic_fc1->forward(state));
    c = torch::relu(critic_fc2->forward(c));
    auto value = critic_out->forward(c);

    return {actionLogit, value};
}

std::tuple<torch::Tensor, torch::Tensor, torch::Tensor>
ActorCriticOLAImpl::act(torch::Tensor state)
{
    torch::NoGradGuard noGrad;

    auto [actionLogit, value] = forward(state);

    auto std = log_std.exp().expand_as(actionLogit);
    auto noise = torch::randn_like(actionLogit);
    auto sampledLogit = actionLogit + noise * std;

    auto logProb = normalLogProb(sampledLogit, actionLogit, std).sum(-1);

    return {sampledLogit.squeeze(), logProb.squeeze(), value.squeeze()};
}

std::tuple<torch::Tensor, torch::Tensor, torch::Tensor>
ActorCriticOLAImpl::evaluate(torch::Tensor states,
                             const torch::Tensor& actionLogits)
{
    auto [meanLogits, values] = forward(std::move(states));

    const auto std = log_std.exp().expand_as(meanLogits);
    auto logProbs = normalLogProb(actionLogits, meanLogits, std).sum(-1);
    auto entropy = normalEntropy(std).sum(-1);

    return {logProbs, values.squeeze(-1), entropy};
}

void ActorCriticOLAImpl::exportActor(const std::string& path)
{
    namespace fs = std::filesystem;

    const fs::path filePath(path);
    if (filePath.has_parent_path())
        fs::create_directories(filePath.parent_path());

    torch::NoGradGuard noGrad;

    torch::jit::script::Module jitModule("ThermostatPolicyOLA");

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

    // Schedule-Only forward:
    //   Input:  x [batch, 24]
    //   Layout:
    //     0..23: userSchedule.userPresent[0..23]
    jitModule.define(R"(
        def forward(self, x):
            # ---- Occupancy schedule (24 hours, already 0/1) ----
            schedule = x[:, 0:24]

            h = torch.relu(torch.matmul(schedule, self.actor_fc1_w.t()) + self.actor_fc1_b)
            h = torch.relu(torch.matmul(h, self.actor_fc2_w.t()) + self.actor_fc2_b)
            logit = torch.matmul(h, self.actor_out_w.t()) + self.actor_out_b

            return torch.sigmoid(logit)
    )");

    jitModule.save(path);
    std::cout << "[ActorCriticOLA] Model exported to: " << path << std::endl;
}

void ActorCriticOLAImpl::saveCheckpoint(const std::string& path)
{
    namespace fs = std::filesystem;
    const fs::path filePath(path);
    if (filePath.has_parent_path())
        fs::create_directories(filePath.parent_path());
    torch::save(std::make_shared<ActorCriticOLAImpl>(*this), path);
    std::cout << "[ActorCriticOLA] Checkpoint saved to: " << path << std::endl;
}

void ActorCriticOLAImpl::loadCheckpoint(const std::string& path)
{
    auto loaded = std::make_shared<ActorCriticOLAImpl>(_stateDim, _actionDim);
    torch::load(loaded, path);
    auto srcParams = loaded->named_parameters();
    auto dstParams = this->named_parameters();
    torch::NoGradGuard noGrad;
    for (auto& p : srcParams)
    {
        if (dstParams.contains(p.key()))
            dstParams[p.key()].copy_(p.value());
    }
    std::cout << "[ActorCriticOLA] Checkpoint loaded from: " << path << std::endl;
}
