# P-OLA Simulator - RL Training Module (PPO Actor-Critic)

add_library(pola_training
    RewardFunction.cpp
    TrainingEnvironment.cpp
    ActorCritic.cpp
    PPOTrainer.cpp
)

target_include_directories(pola_training PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(pola_training
    PUBLIC
        pola_common
        forge
        ${TORCH_LIBRARIES}
)

target_compile_features(pola_training PUBLIC cxx_std_23)
