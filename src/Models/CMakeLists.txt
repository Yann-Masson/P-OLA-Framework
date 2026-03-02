# P-OLA Simulator - AI/Rule-based model implementations

add_library(pola_models
    AIModel.cpp
    RuleBasedModel.cpp
)

target_include_directories(pola_models PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(pola_models
    PUBLIC
        pola_interfaces
        forge
        ${TORCH_LIBRARIES}
)

target_compile_features(pola_models PUBLIC cxx_std_23)

