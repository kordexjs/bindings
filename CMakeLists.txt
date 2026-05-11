#  @file CMakeLists.txt
#  @author Softadastra
#
#  Copyright 2026, Softadastra. All rights reserved.
#  https://github.com/softadastra/kordex-bindings
#  Use of this source code is governed by a MIT license
#  that can be found in the LICENSE file.
#
# ====================================================================
# Kordex Bindings
# ====================================================================

cmake_minimum_required(VERSION 3.20)

project(kordex_bindings
    VERSION 0.1.0
    DESCRIPTION "Native bindings layer for Kordex."
    LANGUAGES C CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/KordexBindingsOptions.cmake")

# --------------------------------------------------------------------
# Global settings
# --------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# --------------------------------------------------------------------
# Export set selection
# --------------------------------------------------------------------
if(DEFINED KORDEX_UMBRELLA_BUILD AND KORDEX_UMBRELLA_BUILD)
  set(KORDEX_BINDINGS_EXPORT_SET KordexTargets)
else()
  set(KORDEX_BINDINGS_EXPORT_SET kordex_bindingsTargets)
endif()

# --------------------------------------------------------------------
# Helpers
# --------------------------------------------------------------------
function(kordex_bindings_require_target target_name package_name sibling_dir fetch_option repository tag)
  if(TARGET ${target_name})
    return()
  endif()

  if(DEFINED KORDEX_UMBRELLA_BUILD AND KORDEX_UMBRELLA_BUILD)
    message(FATAL_ERROR
        "[bindings] Umbrella build requires target '${target_name}' to be provided by the root project.")
  endif()

  if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../${sibling_dir}/CMakeLists.txt")
    message(STATUS "[bindings] Adding ${target_name} from sibling: ../${sibling_dir}")

    add_subdirectory(
        "${CMAKE_CURRENT_LIST_DIR}/../${sibling_dir}"
        "${CMAKE_CURRENT_BINARY_DIR}/_deps/${sibling_dir}")
  else()
    find_package(${package_name} CONFIG QUIET)
  endif()

  if(TARGET ${target_name})
    return()
  endif()

  if(${fetch_option})
    include(FetchContent)

    message(STATUS "[bindings] Fetching ${target_name}")

    FetchContent_Declare(
        ${package_name}
        GIT_REPOSITORY ${repository}
        GIT_TAG ${tag}
        GIT_SHALLOW TRUE)

    FetchContent_MakeAvailable(${package_name})
  endif()

  if(NOT TARGET ${target_name})
    message(FATAL_ERROR
        "[bindings] Required dependency target '${target_name}' was not found.\n"
        "Install package '${package_name}', provide sibling '../${sibling_dir}', "
        "or enable ${fetch_option}.")
  endif()
endfunction()

function(kordex_bindings_setup_quickjs)
  if(TARGET kordex_quickjs)
    return()
  endif()

  if(NOT KORDEX_BINDINGS_ENABLE_QUICKJS)
    return()
  endif()

  if(DEFINED KORDEX_UMBRELLA_BUILD AND KORDEX_UMBRELLA_BUILD)
    if(NOT TARGET quickjs AND NOT TARGET kordex_quickjs)
      message(FATAL_ERROR
          "[bindings] Umbrella build with QuickJS enabled requires a QuickJS target "
          "to be provided by the root project.")
    endif()

    return()
  endif()

  find_package(quickjs CONFIG QUIET)

  if(TARGET quickjs)
    return()
  endif()

  if(NOT KORDEX_BINDINGS_FETCH_QUICKJS)
    message(FATAL_ERROR
        "[bindings] QuickJS backend is enabled but QuickJS was not found.\n"
        "Install QuickJS, provide a CMake target named 'quickjs', "
        "or enable KORDEX_BINDINGS_FETCH_QUICKJS.")
  endif()

  include(FetchContent)

  message(STATUS "[bindings] Fetching QuickJS")

  FetchContent_Declare(
      quickjs
      GIT_REPOSITORY ${KORDEX_QUICKJS_GIT_REPOSITORY}
      GIT_TAG ${KORDEX_QUICKJS_GIT_TAG}
      GIT_SHALLOW TRUE)

  FetchContent_MakeAvailable(quickjs)

  set(KORDEX_QUICKJS_SOURCE_DIR "${quickjs_SOURCE_DIR}")

  add_library(kordex_quickjs STATIC
      "${KORDEX_QUICKJS_SOURCE_DIR}/quickjs.c"
      "${KORDEX_QUICKJS_SOURCE_DIR}/libregexp.c"
      "${KORDEX_QUICKJS_SOURCE_DIR}/libunicode.c"
      "${KORDEX_QUICKJS_SOURCE_DIR}/cutils.c")

  target_include_directories(kordex_quickjs
      PUBLIC
        "${KORDEX_QUICKJS_SOURCE_DIR}")

  target_compile_definitions(kordex_quickjs
      PRIVATE
        CONFIG_VERSION="kordex")

  target_compile_options(kordex_quickjs
      PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wno-unused-parameter>
        $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wno-missing-field-initializers>
        $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wno-sign-compare>
        $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wno-unused-function>)

  if(UNIX AND NOT APPLE)
    target_link_libraries(kordex_quickjs
        PUBLIC
          m)
  endif()

  add_library(quickjs ALIAS kordex_quickjs)
endfunction()

# --------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------
kordex_bindings_require_target(
    kordex::runtime
    kordex_runtime
    runtime
    KORDEX_BINDINGS_FETCH_RUNTIME
    https://github.com/kordexjs/runtime.git
    ${KORDEX_RUNTIME_GIT_TAG})

kordex_bindings_require_target(
    vix::error
    vix_error
    error
    KORDEX_BINDINGS_FETCH_ERROR
    https://github.com/vixcpp/error.git
    ${KORDEX_VIX_GIT_TAG})

kordex_bindings_require_target(
    vix::log
    vix_log
    log
    KORDEX_BINDINGS_FETCH_LOG
    https://github.com/vixcpp/log.git
    ${KORDEX_VIX_GIT_TAG})

kordex_bindings_require_target(
    vix::json
    vix_json
    json
    KORDEX_BINDINGS_FETCH_JSON
    https://github.com/vixcpp/json.git
    ${KORDEX_VIX_GIT_TAG})

kordex_bindings_setup_quickjs()

set(KORDEX_BINDINGS_DEPENDENCIES
    kordex::runtime
    vix::error
    vix::log
    vix::json)

if(KORDEX_BINDINGS_ENABLE_QUICKJS)
  if(TARGET quickjs)
    list(APPEND KORDEX_BINDINGS_DEPENDENCIES quickjs)
  elseif(TARGET kordex_quickjs)
    list(APPEND KORDEX_BINDINGS_DEPENDENCIES kordex_quickjs)
  else()
    message(FATAL_ERROR
        "[bindings] QuickJS backend is enabled but no QuickJS target is available.")
  endif()
endif()

# --------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------
file(GLOB_RECURSE KORDEX_BINDINGS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if(NOT KORDEX_BINDINGS_SOURCES)
  message(FATAL_ERROR "[bindings] No source files found in ${CMAKE_CURRENT_SOURCE_DIR}/src")
endif()

# --------------------------------------------------------------------
# Library
# --------------------------------------------------------------------
add_library(kordex_bindings STATIC
    ${KORDEX_BINDINGS_SOURCES})

add_library(kordex::bindings ALIAS kordex_bindings)

target_compile_features(kordex_bindings
    PUBLIC
      cxx_std_20)

target_include_directories(kordex_bindings
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(kordex_bindings
    PUBLIC
      ${KORDEX_BINDINGS_DEPENDENCIES})

target_compile_definitions(kordex_bindings
    PUBLIC
      KORDEX_BINDINGS_VERSION_STRING="${PROJECT_VERSION}"
      KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE=$<BOOL:${KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE}>
      KORDEX_BINDINGS_ENABLE_QUICKJS=$<BOOL:${KORDEX_BINDINGS_ENABLE_QUICKJS}>
      KORDEX_BINDINGS_ENABLE_V8=$<BOOL:${KORDEX_BINDINGS_ENABLE_V8}>)

if(KORDEX_BINDINGS_ENABLE_WARNINGS)
  target_compile_options(kordex_bindings
      PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>)
endif()

if(KORDEX_BINDINGS_ENABLE_SANITIZERS)
  if(MSVC)
    message(WARNING "[bindings] Sanitizers are not enabled for MSVC in this CMake configuration.")
  else()
    target_compile_options(kordex_bindings
        PRIVATE
          -fno-omit-frame-pointer
          -fsanitize=address,undefined)

    target_link_options(kordex_bindings
        PRIVATE
          -fsanitize=address,undefined)
  endif()
endif()

set_target_properties(kordex_bindings PROPERTIES
    OUTPUT_NAME kordex_bindings
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME bindings)

# --------------------------------------------------------------------
# Tests dependency
# --------------------------------------------------------------------
if(KORDEX_BINDINGS_BUILD_TESTS)
  kordex_bindings_require_target(
      vix::tests
      vix_tests
      tests
      KORDEX_BINDINGS_FETCH_TESTS
      https://github.com/vixcpp/tests.git
      ${KORDEX_VIX_GIT_TAG})
endif()

# --------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------
if(KORDEX_BINDINGS_BUILD_TESTS)
  include(CTest)
  enable_testing()

  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
  endif()
endif()

# --------------------------------------------------------------------
# Examples
# --------------------------------------------------------------------
if(KORDEX_BINDINGS_BUILD_EXAMPLES)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
  endif()
endif()

# --------------------------------------------------------------------
# Install
# --------------------------------------------------------------------
if(KORDEX_BINDINGS_ENABLE_INSTALL
   AND NOT (DEFINED KORDEX_UMBRELLA_BUILD AND KORDEX_UMBRELLA_BUILD))

  install(
      TARGETS kordex_bindings
      EXPORT kordex_bindingsTargets
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  install(
      DIRECTORY include/
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h")

  # --------------------------------------------------------------------
  # Standalone package config/export
  # --------------------------------------------------------------------
  configure_package_config_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/KordexBindingsConfig.cmake.in"
      "${CMAKE_CURRENT_BINARY_DIR}/kordex_bindingsConfig.cmake"
      INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/kordex_bindings")

  write_basic_package_version_file(
      "${CMAKE_CURRENT_BINARY_DIR}/kordex_bindingsConfigVersion.cmake"
      VERSION ${PROJECT_VERSION}
      COMPATIBILITY SameMajorVersion)

  install(
      FILES
        "${CMAKE_CURRENT_BINARY_DIR}/kordex_bindingsConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/kordex_bindingsConfigVersion.cmake"
      DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/cmake/kordex_bindings")

  install(
      EXPORT kordex_bindingsTargets
      FILE kordex_bindingsTargets.cmake
      NAMESPACE kordex::
      DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/kordex_bindings")

endif()
# --------------------------------------------------------------------
# Summary
# --------------------------------------------------------------------
message(STATUS "")
message(STATUS "Kordex Bindings")
message(STATUS "  version      : ${PROJECT_VERSION}")
message(STATUS "  target       : kordex::bindings")
message(STATUS "  export       : ${KORDEX_BINDINGS_EXPORT_SET}")
message(STATUS "  compiler     : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  build        : ${CMAKE_BUILD_TYPE}")
message(STATUS "  tests        : ${KORDEX_BINDINGS_BUILD_TESTS}")
message(STATUS "  examples     : ${KORDEX_BINDINGS_BUILD_EXAMPLES}")
message(STATUS "  warnings     : ${KORDEX_BINDINGS_ENABLE_WARNINGS}")
message(STATUS "  sanitizers   : ${KORDEX_BINDINGS_ENABLE_SANITIZERS}")
message(STATUS "  install      : ${KORDEX_BINDINGS_ENABLE_INSTALL}")
message(STATUS "  native engine: ${KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE}")
message(STATUS "  quickjs      : ${KORDEX_BINDINGS_ENABLE_QUICKJS}")
message(STATUS "  v8           : ${KORDEX_BINDINGS_ENABLE_V8}")
message(STATUS "")
