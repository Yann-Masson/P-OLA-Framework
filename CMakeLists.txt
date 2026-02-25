cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

# Windows-only: include CUDA helpers and configure before project()
if(WIN32)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/windows_cuda.cmake")
    configure_cuda_windows()
endif()

project(P-OLA_Simulator)

# --- LIBS CONFIGURATION ---
set(LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")

# --- FORGE CONFIGURATION ---
set(LIB_FORGE_DIR "${LIBS_DIR}/forge")
add_subdirectory(${LIB_FORGE_DIR})
# ------------------------------

# --- LIBTORCH CONFIGURATION ---
set(LIB_TORCH_DIR "${LIBS_DIR}/libtorch")
list(APPEND CMAKE_PREFIX_PATH "${LIB_TORCH_DIR}")
find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
# ------------------------------

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/Interfaces/IClock.hpp
        src/Services/Clock.cpp
        src/Services/Clock.hpp
        src/Interfaces/IConsumptionService.hpp
        src/Services/ConsumptionService.cpp
        src/Services/ConsumptionService.hpp
        src/Interfaces/ITemperatureFactor.hpp
        src/Simulation/TemperatureFactor/ATemperatureFactor.hpp
        src/Simulation/TemperatureFactor/ATemperatureFactor.cpp
        src/Simulation/TemperatureFactor/Heater.hpp
        src/Simulation/TemperatureFactor/Heater.cpp
        src/Simulation/TemperatureFactor/Wall.hpp
        src/Simulation/TemperatureFactor/Wall.cpp
        src/Simulation/TemperatureFactor/Window.hpp
        src/Simulation/TemperatureFactor/Window.cpp
        src/Services/Inputs/AInputService.hpp
        src/Services/Inputs/EnergyPriceService.cpp
        src/Services/Inputs/EnergyPriceService.hpp
        src/Services/Inputs/GPSService.cpp
        src/Services/Inputs/GPSService.hpp
        src/Interfaces/IInputService.hpp
        src/Services/Inputs/UserPreferenceService.cpp
        src/Services/Inputs/UserPreferenceService.hpp
        src/Services/Inputs/WeatherService.cpp
        src/Services/Inputs/WeatherService.hpp
        src/Common/DataTypes.hpp
        src/Interfaces/IAIModel.hpp
        src/Models/RuleBasedModel.cpp
        src/Models/RuleBasedModel.hpp
        src/Models/AIModel.cpp
        src/Models/AIModel.hpp
        src/Simulation/Room/Room.cpp
        src/Simulation/Room/Room.hpp
)

target_compile_features(P-OLA_Simulator PRIVATE cxx_std_23)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        forge
        ${TORCH_LIBRARIES}
)

# Required for Windows to find the LibTorch DLLs at runtime
if(MSVC)
    configure_windows_torch_runtime(${PROJECT_NAME})
endif()
