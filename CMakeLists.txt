cmake_minimum_required(VERSION 3.24)
project(Egalis VERSION 0.1.0 LANGUAGES CXX)

add_library(egalis_build_options INTERFACE)
target_compile_features(egalis_build_options INTERFACE cxx_std_20)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
  target_compile_options(egalis_build_options INTERFACE /W4 /permissive-)
else()
  target_compile_options(egalis_build_options INTERFACE -Wall -Wextra -Wpedantic)
endif()
option(EGALIS_ENABLE_SANITIZERS "Enable ASan and UBSan on supported Unix toolchains" OFF)
if(EGALIS_ENABLE_SANITIZERS)
  if(WIN32 OR NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(FATAL_ERROR "Sanitizer builds require a supported Unix GCC/Clang toolchain")
  endif()
  target_compile_options(egalis_build_options INTERFACE
    -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
  target_link_options(egalis_build_options INTERFACE -fsanitize=address,undefined)
endif()

add_library(egalis_foundation
  src/protocol/types.cpp src/kv/command.cpp src/protocol/messages.cpp src/protocol/events.cpp)
target_include_directories(egalis_foundation PUBLIC include)
target_link_libraries(egalis_foundation PUBLIC egalis_build_options)
add_library(egalis_sim
  src/sim/scheduler.cpp src/sim/network.cpp src/sim/network_faults.cpp
  src/sim/random.cpp src/sim/trace.cpp)
target_link_libraries(egalis_sim PUBLIC egalis_foundation)

include(CTest)
if(BUILD_TESTING)
  find_package(GTest 1.15 CONFIG QUIET)
  if(NOT GTest_FOUND)
    include(FetchContent)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_Declare(googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG b514bdc898e2951020cbdca1304b75f5950d1f59
      GIT_SHALLOW FALSE)
    FetchContent_MakeAvailable(googletest)
  endif()
  add_executable(egalis_build_contract tests/build_contract_test.cpp)
  target_link_libraries(egalis_build_contract PRIVATE egalis_build_options GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(egalis_build_contract)
  add_executable(egalis_unit_tests
    tests/unit/types_test.cpp tests/unit/command_test.cpp tests/unit/messages_test.cpp
    tests/unit/events_test.cpp)
  target_link_libraries(egalis_unit_tests PRIVATE egalis_foundation GTest::gtest_main)
  gtest_discover_tests(egalis_unit_tests)
  add_executable(egalis_sim_tests tests/simulation/scheduler_test.cpp tests/simulation/network_test.cpp
                                tests/simulation/trace_test.cpp tests/simulation/faults_test.cpp)
  target_link_libraries(egalis_sim_tests PRIVATE egalis_sim GTest::gtest_main)
  gtest_discover_tests(egalis_sim_tests)
endif()
