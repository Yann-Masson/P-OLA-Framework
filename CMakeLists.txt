cmake_minimum_required(VERSION 4.1)

# Include Windows + CUDA helpers and configure before project()
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/windows_cuda.cmake")
configure_cuda_windows()

project(P-OLA_Simulator)

add_subdirectory(CppDic)

# --- LIBTORCH CONFIGURATION ---
set(LIBTORCH_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libtorch")
list(APPEND CMAKE_PREFIX_PATH "${LIBTORCH_DIR}")
find_package(Torch REQUIRED)
# ------------------------------

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/ThermalModel.cpp
        src/OLA_Agent.cpp
        src/OLA_Agent.hpp)

target_compile_features(P-OLA_Simulator PRIVATE cxx_std_23)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        cppdic
        "${TORCH_LIBRARIES}"
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        CppDic/lib/include
)

# Required for Windows to find the LibTorch DLLs at runtime
configure_windows_torch_runtime(${PROJECT_NAME})
