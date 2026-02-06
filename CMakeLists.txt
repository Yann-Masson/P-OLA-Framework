cmake_minimum_required(VERSION 4.1)

project(P-OLA_Simulator)

add_subdirectory(CppDic)

add_executable(P-OLA_Simulator
        src/main.cpp
        src/ThermalModel.cpp
        src/OLA_Agent.cpp
        src/OLA_Agent.hpp
        src/DependencyInjectionTest/IMessageSenderInterface.hpp
        src/DependencyInjectionTest/SimpleMessageSender.hpp
        src/DependencyInjectionTest/SimpleMessageSender.cpp
        src/DependencyInjectionTest/MockedMessageSender.hpp
        src/DependencyInjectionTest/MockedMessageSender.cpp
        src/DependencyInjectionTest/IMessageReceiverInterface.hpp
        src/DependencyInjectionTest/SimpleMessageReceiver.hpp
        src/DependencyInjectionTest/SimpleMessageReceiver.cpp
        src/DependencyInjectionTest/MockedMessageReceiver.hpp
        src/DependencyInjectionTest/MockedMessageReceiver.cpp
)

target_compile_features(P-OLA_Simulator PRIVATE cxx_std_23)

target_link_libraries(P-OLA_Simulator
    PRIVATE
        cppdic
)

target_include_directories(P-OLA_Simulator
    PRIVATE
        CppDic/lib/include
)