cmake_minimum_required(VERSION 3.20)
project(BlueSkyWeather LANGUAGES CXX)

option(BLUESKY_ENABLE_ECCODES "Build the ecCodes GRIB2 decoder" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(bluesky_weather
    wind_interpolator.cpp
    route_wind_adapter.cpp
    weather_trajectory_calculator.cpp
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
    target_sources(bluesky_weather PRIVATE grib2_decoder.cpp)
    target_include_directories(bluesky_weather PRIVATE ${ECCODES_INCLUDE_DIR})
    target_link_libraries(bluesky_weather PUBLIC ${ECCODES_LIBRARY})
    target_compile_definitions(bluesky_weather PUBLIC BLUESKY_HAVE_ECCODES=1)
endif()

enable_testing()
add_executable(weather_engine_test tests/weather_engine_test.cpp)
target_link_libraries(weather_engine_test PRIVATE bluesky_weather)
target_compile_options(weather_engine_test PRIVATE
    "$<$<CXX_COMPILER_ID:GNU,Clang>:-UNDEBUG>"
    "$<$<CXX_COMPILER_ID:MSVC>:/UNDEBUG>"
)
add_test(NAME weather_engine_contract COMMAND weather_engine_test)

if(BLUESKY_ENABLE_ECCODES)
    add_executable(grib2_decoder_contract_test tests/grib2_decoder_contract_test.cpp)
    target_link_libraries(grib2_decoder_contract_test PRIVATE bluesky_weather)
    target_compile_options(grib2_decoder_contract_test PRIVATE
        "$<$<CXX_COMPILER_ID:GNU,Clang>:-UNDEBUG>"
        "$<$<CXX_COMPILER_ID:MSVC>:/UNDEBUG>"
    )
    add_test(NAME grib2_decoder_contract COMMAND grib2_decoder_contract_test)
endif()
