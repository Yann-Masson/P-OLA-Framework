cmake_minimum_required(VERSION 3.20)

# Create header-only library
add_library(dicnew INTERFACE)

# Specify include directories
target_include_directories(dicnew INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Require C++20
target_compile_features(dicnew INTERFACE cxx_std_20)

# Installation rules
install(TARGETS dicnew
    EXPORT dicnewTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT dicnewTargets
    FILE dicnewTargets.cmake
    NAMESPACE dicnew::
    DESTINATION lib/cmake/dicnew
)
