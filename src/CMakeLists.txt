# P-OLA Simulator - Source directory
# Aggregates all sub-modules and builds the final executable.

add_subdirectory(Common)
add_subdirectory(Interfaces)
add_subdirectory(Models)
add_subdirectory(Services)
add_subdirectory(Simulation)

add_executable(${PROJECT_NAME} Main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        pola_common
        pola_interfaces
        pola_models
        pola_services
        pola_simulation
        forge
        ${TORCH_LIBRARIES}
)

# Required for Windows to find the LibTorch DLLs at runtime
if(MSVC)
    configure_windows_torch_runtime(${PROJECT_NAME})
endif()

