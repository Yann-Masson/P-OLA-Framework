cmake_minimum_required(VERSION 4.1)

project(P-OLA_Simulator)

add_executable(P-OLA_Simulator
        src/ThermalModel.cpp
        src/OLA_Agent.cpp
        src/OLA_Agent.hpp)

target_sources(P-OLA_Simulator
    PRIVATE
        src/main.cpp
)