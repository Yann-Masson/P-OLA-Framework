/**
 * @file PPOTrainingAgentNoGPS.hpp
 * @brief PPO training agent for the GPS-free thermostat model (40-dim state).
 *
 * Mirrors PPOTrainingAgent but:
 *  - Implements IAIModelNoGPS (accepts AIStateNoGPS)
 *  - Uses ActorCriticNoGPS (default stateDim = 40)
 *  - normalizeState() builds a 40-element tensor (no GPS features)
 */

#pragma once

#include "ActorCriticNoGPS.hpp"
#include "Interfaces/IAIModelNoGPS.hpp"
#include "Interfaces/IEnvironmentControl.hpp"
#include "RewardFunctionNoGPS.hpp"
#include "TrainingConfig.hpp"

#include <forge/provider.hpp>
#include <memory>
#include <optional>

namespace POLA::Training {

class PPOTrainingAgentNoGPS : public Interfaces::IAIModelNoGPS {
public:
  PPOTrainingAgentNoGPS(const forge::ProviderRef &provider,
                        const TrainingConfig &config, uint32_t seed = 42);

  double predict(const Common::AIStateNoGPS &state) override;

private:
  struct RolloutTransition {
    torch::Tensor stateTensor;
    float actionLogit;
    float logProb;
    float value;
    float reward;
    bool done;
  };

  ActorCriticNoGPS _actorCritic = nullptr;
  std::unique_ptr<torch::optim::Adam> _optimizer;
  torch::Device _device = torch::kCPU;

  TrainingConfig _config;
  forge::ProviderRef _provider;
  RewardFunctionNoGPS _rewardFn;

  std::vector<RolloutTransition> _rollout;

  std::optional<Common::AIStateNoGPS> _prevState;
  std::optional<RolloutTransition> _prevTransition;

  int _totalSteps = 0;
  int _numRollouts = 0;
  double _bestAvgReward = -1e9;

  torch::Tensor normalizeState(const Common::AIStateNoGPS &state) const;
  void updatePPO(const torch::Tensor &finalStateTensor);
  void exportModel();
};

} // namespace POLA::Training
