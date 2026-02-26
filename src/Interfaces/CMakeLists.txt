# P-OLA Simulator - Interface definitions

add_library(pola_interfaces INTERFACE)

target_include_directories(pola_interfaces INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(pola_interfaces INTERFACE pola_common)

