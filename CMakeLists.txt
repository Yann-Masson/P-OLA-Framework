cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

# Windows-only: include CUDA helpers and configure before project()
if(WIN32)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/windows_cuda.cmake")
    configure_cuda_windows()
endif()

project(Simulator)

# Place executables at the root of the build directory.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Define DATA_DIR globally for all targets
add_compile_definitions(DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

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

add_subdirectory(src)
