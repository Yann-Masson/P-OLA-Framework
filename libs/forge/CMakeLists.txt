cmake_minimum_required(VERSION 3.20)

project(Forge VERSION 1.0.0 LANGUAGES CXX)

# Create header-only library
add_library(forge INTERFACE)

# Specify include directories (idiomatic include/ layout)
target_include_directories(forge INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Require C++20
target_compile_features(forge INTERFACE cxx_std_20)

# Installation rules
install(TARGETS forge
    EXPORT forgeTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/forge
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT forgeTargets
    FILE forgeTargets.cmake
    NAMESPACE forge::
    DESTINATION lib/cmake/forge
)

# Add example executable
add_executable(forge_example example.cpp)
target_link_libraries(forge_example PRIVATE forge)
target_compile_features(forge_example PRIVATE cxx_std_20)

