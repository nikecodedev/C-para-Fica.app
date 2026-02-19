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

add_library(engine_logging
    engine/logging/TrackingLogger.cpp
)
target_include_directories(engine_logging PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(engine_core
    engine/core/TrackingEngine.cpp
    engine/core/EngineAccessManager.cpp
)
target_include_directories(engine_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(engine_core PUBLIC engine_fusion engine_math engine_logging)

add_executable(engine_test engine/tests/test_ekf.cpp)
add_executable(tracking_test engine/tests/test_tracking_engine.cpp)
target_link_libraries(tracking_test PRIVATE engine_core engine_sensors)

add_executable(logging_test engine/tests/test_logging.cpp)
target_link_libraries(logging_test PRIVATE engine_core)

add_executable(access_manager_test engine/tests/test_access_manager.cpp)
target_link_libraries(access_manager_test PRIVATE engine_core)
enable_testing()
add_test(NAME EKFTest COMMAND engine_test)
