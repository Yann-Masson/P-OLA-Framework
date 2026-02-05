cmake_minimum_required(VERSION 4.1)

project(P-OLA_Simulator)

add_subdirectory(CppDic)

add_executable(P-OLA_Simulator
        src/main.cpp
        src/ThermalModel.cpp
        src/OLA_Agent.cpp
        src/OLA_Agent.hpp)

target_compile_features(P-OLA_Simulator PRIVATE cxx_std_20)

target_link_libraries(P-OLA_Simulator
    PRIVATE
        cppdic
)

target_include_directories(P-OLA_Simulator
    PRIVATE
        CppDic/lib/include
)