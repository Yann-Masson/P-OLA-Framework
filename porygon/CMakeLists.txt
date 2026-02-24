add_library(porygon SHARED)

add_subdirectory(src)

# Tests are optional - only build if GTest is available
find_package(GTest QUIET)
if(GTEST_FOUND)
    add_subdirectory(tests)
endif()

target_include_directories(porygon PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_include_directories(porygon PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../..)

if (COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    target_link_libraries(porygon PRIVATE gcov)
endif ()
