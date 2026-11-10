cmake_minimum_required(VERSION 3.20)
project(BlueSkyWeather LANGUAGES CXX)

option(BLUESKY_ENABLE_ECCODES "Build the ecCodes GRIB2 decoder" OFF)
option(BLUESKY_ENABLE_QT_NETWORK "Build the Qt asynchronous NOAA availability client" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(bluesky_weather
    wind_interpolator.cpp
    route_wind_adapter.cpp
    weather_trajectory_calculator.cpp
    gfs_forecast_planner.cpp
    weather_applicability_policy.cpp
    weather_coverage_request.cpp
    ../PLANNING/wind_performance_trajectory.cpp
)
target_include_directories(bluesky_weather PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_compile_features(bluesky_weather PUBLIC cxx_std_17)

if(BLUESKY_ENABLE_ECCODES)
    find_path(ECCODES_INCLUDE_DIR eccodes.h REQUIRED)
    find_library(ECCODES_LIBRARY NAMES eccodes REQUIRED)
    target_sources(bluesky_weather PRIVATE grib2_decoder.cpp grib2_weather_pipeline.cpp)
    target_include_directories(bluesky_weather PRIVATE ${ECCODES_INCLUDE_DIR})
    target_link_libraries(bluesky_weather PUBLIC ${ECCODES_LIBRARY})
    target_compile_definitions(bluesky_weather PUBLIC BLUESKY_HAVE_ECCODES=1)
endif()

enable_testing()

if(BLUESKY_ENABLE_QT_NETWORK)
    find_package(Qt6 REQUIRED COMPONENTS Core Network)
    add_library(bluesky_weather_qt
        gfs_forecast_availability_service.cpp
        gfs_forecast_availability_service.hpp
    )
    set_target_properties(bluesky_weather_qt PROPERTIES AUTOMOC ON)
    target_include_directories(bluesky_weather_qt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(bluesky_weather_qt PUBLIC bluesky_weather Qt6::Core Qt6::Network)
    add_executable(gfs_forecast_availability_test tests/gfs_forecast_availability_test.cpp)
    target_link_libraries(gfs_forecast_availability_test PRIVATE bluesky_weather_qt)
    target_compile_options(gfs_forecast_availability_test PRIVATE
        "$<$<CXX_COMPILER_ID:GNU,Clang>:-UNDEBUG>"
        "$<$<CXX_COMPILER_ID:MSVC>:/UNDEBUG>"
    )
    add_test(NAME gfs_forecast_availability_contract COMMAND gfs_forecast_availability_test)
endif()
add_executable(weather_engine_test tests/weather_engine_test.cpp)
target_link_libraries(weather_engine_test PRIVATE bluesky_weather)
target_compile_options(weather_engine_test PRIVATE
    "$<$<CXX_COMPILER_ID:GNU,Clang>:-UNDEBUG>"
    "$<$<CXX_COMPILER_ID:MSVC>:/UNDEBUG>"
)
add_test(NAME weather_engine_contract COMMAND weather_engine_test)

add_executable(gfs_forecast_planner_test tests/gfs_forecast_planner_test.cpp)
target_link_libraries(gfs_forecast_planner_test PRIVATE bluesky_weather)
target_compile_options(gfs_forecast_planner_test PRIVATE
    "$<$<CXX_COMPILER_ID:GNU,Clang>:-UNDEBUG>"
    "$<$<CXX_COMPILER_ID:MSVC>:/UNDEBUG>"
)
add_test(NAME gfs_forecast_planner_contract COMMAND gfs_forecast_planner_test)

add_executable(weather_applicability_policy_test tests/weather_applicability_policy_test.cpp)
target_link_libraries(weather_applicability_policy_test PRIVATE bluesky_weather)
target_compile_options(weather_applicability_policy_test PRIVATE
    "$<$<CXX_COMPILER_ID:GNU,Clang>:-UNDEBUG>"
    "$<$<CXX_COMPILER_ID:MSVC>:/UNDEBUG>"
)
add_test(NAME weather_applicability_policy_contract COMMAND weather_applicability_policy_test)

add_executable(weather_coverage_request_test tests/weather_coverage_request_test.cpp)
target_link_libraries(weather_coverage_request_test PRIVATE bluesky_weather)
target_compile_options(weather_coverage_request_test PRIVATE
    "$<$<CXX_COMPILER_ID:GNU,Clang>:-UNDEBUG>"
    "$<$<CXX_COMPILER_ID:MSVC>:/UNDEBUG>"
)
add_test(NAME weather_coverage_request_contract COMMAND weather_coverage_request_test)

if(BLUESKY_ENABLE_ECCODES)
    add_executable(grib2_decoder_contract_test tests/grib2_decoder_contract_test.cpp)
    target_link_libraries(grib2_decoder_contract_test PRIVATE bluesky_weather)
    target_compile_options(grib2_decoder_contract_test PRIVATE
        "$<$<CXX_COMPILER_ID:GNU,Clang>:-UNDEBUG>"
        "$<$<CXX_COMPILER_ID:MSVC>:/UNDEBUG>"
    )
    add_test(NAME grib2_decoder_contract COMMAND grib2_decoder_contract_test)
endif()
