# P-OLA Simulator - Simulation components (Room, SmartThermostat, TemperatureFactor)

add_library(pola_simulation
    Room/Room.cpp
    SmartThermostat/SmartThermostat.cpp
        SmartThermostat/AIRecorder.cpp
    TemperatureFactor/ATemperatureFactor.cpp
    TemperatureFactor/Heater.cpp
    TemperatureFactor/Wall.cpp
    TemperatureFactor/Window.cpp
    DataManager/DataManager.cpp
    ProviderSetup.cpp
)

target_include_directories(pola_simulation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(pola_simulation
    PUBLIC
        pola_interfaces
        pola_services
        pola_common
        forge
        ${TORCH_LIBRARIES}
)

target_compile_features(pola_simulation PUBLIC cxx_std_23)

