cmake_minimum_required (VERSION 2.6)
project (Development)
include_directories(
        VL53L0X_API/Api/core/inc
        VL53L0X_rasp/platform/inc
)
add_executable(main main.cpp)
add_executable(connection connection.cpp)