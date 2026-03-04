/**
 * @file ActorCriticNoGPS.cpp
 * @brief Implementation of the GPS-free Actor-Critic network.
 */

#include "ActorCriticNoGPS.hpp"
#include "TrainingConfig.hpp"

#include <cmath>
#include <filesystem>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace POLA::Training;

// ============================================================================
// Gaussian utilities (same as ActorCritic.cpp)
// ============================================================================

static torch::Tensor normalLogProbNoGPS(const torch::Tensor &x,
                                        const torch::Tensor &mean,
                                        const torch::Tensor &std) {
  const auto var = std * std;
  return -0.5 * ((x - mean).pow(2) / (var + 1e-8)) - std.log() -
         0.5 * std::log(2.0 * M_PI);
}

static torch::Tensor normalEntropyNoGPS(const torch::Tensor &std) {
  return 0.5 + 0.5 * std::log(2.0 * M_PI) + std.log();
}

static void orthogonalInitNoGPS(torch::nn::Linear &layer, double gain = 1.0) {
  torch::nn::init::orthogonal_(layer->weight, gain);
  if (layer->bias.defined())
    torch::nn::init::zeros_(layer->bias);
}

// ============================================================================
// ActorCriticNoGPSImpl
// ============================================================================

ActorCriticNoGPSImpl::ActorCriticNoGPSImpl(int64_t stateDim, int64_t actionDim,
                                           int64_t hiddenDim)
    : _stateDim(stateDim), _actionDim(actionDim) {
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

  orthogonalInitNoGPS(actor_fc1, std::sqrt(2.0));
  orthogonalInitNoGPS(actor_fc2, std::sqrt(2.0));
  orthogonalInitNoGPS(actor_out, 0.01);
  orthogonalInitNoGPS(critic_fc1, std::sqrt(2.0));
  orthogonalInitNoGPS(critic_fc2, std::sqrt(2.0));
  orthogonalInitNoGPS(critic_out, 1.0);
}

std::tuple<torch::Tensor, torch::Tensor>
ActorCriticNoGPSImpl::forward(torch::Tensor state) {
  auto a = torch::relu(actor_fc1->forward(state));
  a = torch::relu(actor_fc2->forward(a));
  auto actionLogit = actor_out->forward(a);

  auto c = torch::relu(critic_fc1->forward(state));
  c = torch::relu(critic_fc2->forward(c));
  auto value = critic_out->forward(c);

  return {actionLogit, value};
}

std::tuple<torch::Tensor, torch::Tensor, torch::Tensor>
ActorCriticNoGPSImpl::act(torch::Tensor state) {
  torch::NoGradGuard noGrad;

  auto [actionLogit, value] = forward(state);

  auto std = log_std.exp().expand_as(actionLogit);
  auto noise = torch::randn_like(actionLogit);
  auto sampledLogit = actionLogit + noise * std;

  auto logProb = normalLogProbNoGPS(sampledLogit, actionLogit, std).sum(-1);

  return {sampledLogit.squeeze(), logProb.squeeze(), value.squeeze()};
}

std::tuple<torch::Tensor, torch::Tensor, torch::Tensor>
ActorCriticNoGPSImpl::evaluate(torch::Tensor states,
                               torch::Tensor actionLogits) {
  auto [meanLogits, values] = forward(states);

  auto std = log_std.exp().expand_as(meanLogits);
  auto logProbs = normalLogProbNoGPS(actionLogits, meanLogits, std).sum(-1);
  auto entropy = normalEntropyNoGPS(std).sum(-1);

  return {logProbs, values.squeeze(-1), entropy};
}

void ActorCriticNoGPSImpl::exportActor(const std::string &path) {
  namespace fs = std::filesystem;

  const fs::path filePath(path);
  if (filePath.has_parent_path())
    fs::create_directories(filePath.parent_path());

  torch::NoGradGuard noGrad;

  torch::jit::script::Module jitModule("ThermostatPolicyNoGPS");

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

  // No-GPS forward:
  //   Input:  x [batch, 40]
  //   Layout:
  //     0:  tempIn           → (x - 5.0) / 30.0
  //     1:  electricityPrice → x / 0.50
  //     2..13 (pairs): weather.forecast[0..5]
  //       even = outdoorTemp  → (x + 20.0) / 60.0
  //       odd  = sunlight lux → x / 100000.0
  //     14: userPreferences.minTemperature → (x - 15.0) / 15.0
  //     15: userPreferences.maxTemperature → (x - 15.0) / 15.0
  //     16..39: userSchedule.userPresent[0..23] (0.0 or 1.0, no normalization)
  jitModule.define(R"(
        def forward(self, x):
            # ---- Scalar features ----
            t_in  = (x[:, 0:1]  - 5.0)  / 30.0
            price =  x[:, 1:2]          / 0.50

            # ---- Weather forecast (6 hours × 2 features) ----
            f0_t  = (x[:,  2:3]  + 20.0) / 60.0
            f0_s  =  x[:,  3:4]          / 100000.0
            f1_t  = (x[:,  4:5]  + 20.0) / 60.0
            f1_s  =  x[:,  5:6]          / 100000.0
            f2_t  = (x[:,  6:7]  + 20.0) / 60.0
            f2_s  =  x[:,  7:8]          / 100000.0
            f3_t  = (x[:,  8:9]  + 20.0) / 60.0
            f3_s  =  x[:,  9:10] / 100000.0
            f4_t  = (x[:, 10:11] + 20.0) / 60.0
            f4_s  =  x[:, 11:12] / 100000.0
            f5_t  = (x[:, 12:13] + 20.0) / 60.0
            f5_s  =  x[:, 13:14] / 100000.0

            # ---- User preferences ----
            pref_min = (x[:, 14:15] - 15.0) / 15.0
            pref_max = (x[:, 15:16] - 15.0) / 15.0

            # ---- Occupancy schedule (24 hours, already 0/1) ----
            schedule = x[:, 16:40]

            x_norm = torch.cat([
                t_in, price,
                f0_t, f0_s, f1_t, f1_s, f2_t, f2_s,
                f3_t, f3_s, f4_t, f4_s, f5_t, f5_s,
                pref_min, pref_max,
                schedule
            ], dim=1)

            h = torch.relu(torch.matmul(x_norm, self.actor_fc1_w.t()) + self.actor_fc1_b)
            h = torch.relu(torch.matmul(h, self.actor_fc2_w.t()) + self.actor_fc2_b)
            logit = torch.matmul(h, self.actor_out_w.t()) + self.actor_out_b

            return torch.sigmoid(logit)
    )");

  jitModule.save(path);
  std::cout << "[ActorCriticNoGPS] Model exported to: " << path << std::endl;
}

void ActorCriticNoGPSImpl::saveCheckpoint(const std::string &path) {
  namespace fs = std::filesystem;
  const fs::path filePath(path);
  if (filePath.has_parent_path())
    fs::create_directories(filePath.parent_path());
  torch::save(std::make_shared<ActorCriticNoGPSImpl>(*this), path);
  std::cout << "[ActorCriticNoGPS] Checkpoint saved to: " << path << std::endl;
}

void ActorCriticNoGPSImpl::loadCheckpoint(const std::string &path) {
  auto loaded = std::make_shared<ActorCriticNoGPSImpl>(_stateDim, _actionDim);
  torch::load(loaded, path);
  auto srcParams = loaded->named_parameters();
  auto dstParams = this->named_parameters();
  torch::NoGradGuard noGrad;
  for (auto &p : srcParams) {
    if (dstParams.contains(p.key()))
      dstParams[p.key()].copy_(p.value());
  }
  std::cout << "[ActorCriticNoGPS] Checkpoint loaded from: " << path
            << std::endl;
}
