cmake_minimum_required(VERSION 3.14)
project(fica-motor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(engine_config
    engine/config/UnitConverter.cpp
)
target_include_directories(engine_config PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

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

add_library(app_localization
    app/localization/Translations.cpp
    app/localization/Localizer.cpp
    app/localization/LocaleDetector.cpp
)
target_include_directories(app_localization PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/app
)

add_executable(localization_test app/tests/test_localization.cpp)
target_link_libraries(localization_test PRIVATE app_localization)

add_library(app_ui
    app/ui/FirstBuildIntegration.cpp
)
target_include_directories(app_ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/app
)
target_link_libraries(app_ui PUBLIC engine_core engine_config engine_logging)

add_executable(first_build_test app/tests/test_first_build.cpp)
target_link_libraries(first_build_test PRIVATE app_ui)

add_executable(unit_converter_test engine/tests/test_unit_converter.cpp)
target_link_libraries(unit_converter_test PRIVATE engine_config)

add_executable(ekf_math_test engine/tests/test_ekf_math.cpp)
target_link_libraries(ekf_math_test PRIVATE engine_math)

add_executable(replay_test engine/tests/test_replay.cpp)
target_link_libraries(replay_test PRIVATE engine_core engine_logging)

add_executable(simulator_test engine/tests/test_simulator.cpp)
target_link_libraries(simulator_test PRIVATE engine_core)
enable_testing()
add_test(NAME EKFTest COMMAND engine_test)
add_test(NAME EKFMathTest COMMAND ekf_math_test)
add_test(NAME UnitConverterTest COMMAND unit_converter_test)
add_test(NAME ReplayTest COMMAND replay_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME SimulatorTest COMMAND simulator_test)
add_test(NAME FirstBuildTest COMMAND first_build_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_library(engine_audio
    engine/audio/AudioEngine.cpp
    engine/audio/tts/DeepgramTTS.cpp
    engine/audio/voice_agent/DeepgramVoiceAgent.cpp
)
target_include_directories(engine_audio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/audio
)

add_executable(audio_interface_test engine/audio/tests/test_audio_interfaces.cpp)
target_link_libraries(audio_interface_test PRIVATE engine_audio)
add_test(NAME AudioInterfaceTest COMMAND audio_interface_test)

add_executable(deepgram_test engine/audio/tests/test_deepgram.cpp)
target_link_libraries(deepgram_test PRIVATE engine_audio)
add_test(NAME DeepgramTest COMMAND deepgram_test)
