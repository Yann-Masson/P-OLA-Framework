# P-OLA Simulator - Service implementations (Clock, Consumption, Inputs)

add_library(pola_services
    Clock.cpp
    TrainingClock.cpp
    ConsumptionService.cpp
    UserComfortService.cpp
    Inputs/EnergyPriceService.cpp
    Inputs/GPSService.cpp
    Inputs/UserPreferenceService.cpp
    Inputs/WeatherService.cpp
    Inputs/UserScheduleService.cpp
)

target_include_directories(pola_services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(pola_services
    PUBLIC
        pola_interfaces
        pola_common
        forge
)

target_compile_features(pola_services PUBLIC cxx_std_23)

# Absolute path to project data directory for runtime loading.
target_compile_definitions(pola_services PUBLIC DATA_DIR="${PROJECT_SOURCE_DIR}/data")

