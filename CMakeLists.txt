cmake_minimum_required(VERSION 3.14)
project(fica-motor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(engine_math
    engine/math/CoordinateConverter.cpp
    engine/math/Quaternion.cpp
)
target_include_directories(engine_math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(engine_sensors
    engine/sensors/GPSSensor.cpp
    engine/sensors/IMUSensor.cpp
    engine/sensors/MagnetometerSensor.cpp
)
target_include_directories(engine_sensors PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(engine_sensors PUBLIC engine_math)

add_library(engine_fusion
    engine/fusion/EKFConfig.cpp
    engine/fusion/EKFProcess.cpp
    engine/fusion/EKFMeasurement.cpp
    engine/fusion/ExtendedKalmanFilter.cpp
)
target_include_directories(engine_fusion PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(engine_fusion PUBLIC engine_math)

add_executable(engine_test engine/tests/test_ekf.cpp)
target_link_libraries(engine_test PRIVATE engine_fusion engine_sensors)
enable_testing()
add_test(NAME EKFTest COMMAND engine_test)
