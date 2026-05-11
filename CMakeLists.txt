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
# Purpose:
#   Native bindings layer for Kordex.
#
#   Provides the bridge between the Kordex runtime and script execution.
#   It owns engine abstractions, contexts, values, objects, functions,
#   native modules, module registry, scripts, and the runtime bridge.
#
# Public Targets:
#   - kordex_bindings   : The actual library target
#   - kordex::bindings  : Namespaced alias for consumers
#
# Public Dependencies:
#   - kordex::runtime
#   - vix::error
#   - vix::log
#   - vix::json
#
# Installation/Export:
#   - Umbrella build exports into: KordexTargets
#   - Standalone build exports into: kordex_bindingsTargets
# ====================================================================

cmake_minimum_required(VERSION 3.20)

project(kordex_bindings
  VERSION 0.1.0
  DESCRIPTION "Native bindings layer for Kordex."
  LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/KordexBindingsOptions.cmake")

# --------------------------------------------------------------------
# Global settings
# --------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
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
# Sources discovery
# --------------------------------------------------------------------
file(GLOB_RECURSE KORDEX_BINDINGS_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# --------------------------------------------------------------------
# Dependency helpers
# --------------------------------------------------------------------
function(kordex_bindings_require_target target_name package_name sibling_dir fetch_option repo_url git_tag)
  if(DEFINED KORDEX_UMBRELLA_BUILD AND KORDEX_UMBRELLA_BUILD)
    if(NOT TARGET ${target_name})
      message(FATAL_ERROR "[bindings] Umbrella build: ${target_name} must be provided before bindings.")
    endif()
    return()
  endif()

  if(TARGET ${target_name})
    return()
  endif()

  if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../${sibling_dir}/CMakeLists.txt")
    message(STATUS "[bindings] Adding ${target_name} from sibling: ../${sibling_dir}")
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../${sibling_dir}" "${sibling_dir}")
  else()
    find_package(${package_name} CONFIG QUIET)
  endif()

  if(TARGET ${target_name})
    return()
  endif()

  if(${fetch_option})
    include(FetchContent)
    message(STATUS "[bindings] Fetching ${target_name} via FetchContent")
    FetchContent_Declare(${package_name}
      GIT_REPOSITORY ${repo_url}
      GIT_TAG        ${git_tag}
    )
    FetchContent_MakeAvailable(${package_name})
  endif()

  if(NOT TARGET ${target_name})
    message(FATAL_ERROR
      "${target_name} not found. Enable ${fetch_option}=ON, install ${package_name}, or provide the target before bindings."
    )
  endif()
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
  ${KORDEX_RUNTIME_GIT_TAG}
)

kordex_bindings_require_target(
  vix::error
  vix_error
  error
  KORDEX_BINDINGS_FETCH_ERROR
  https://github.com/vixcpp/error.git
  ${KORDEX_VIX_GIT_TAG}
)

kordex_bindings_require_target(
  vix::log
  vix_log
  log
  KORDEX_BINDINGS_FETCH_LOG
  https://github.com/vixcpp/log.git
  ${KORDEX_VIX_GIT_TAG}
)

kordex_bindings_require_target(
  vix::json
  vix_json
  json
  KORDEX_BINDINGS_FETCH_JSON
  https://github.com/vixcpp/json.git
  ${KORDEX_VIX_GIT_TAG}
)

set(KORDEX_BINDINGS_DEPENDENCIES
  kordex::runtime
  vix::error
  vix::log
  vix::json
)

# --------------------------------------------------------------------
# Library target
# --------------------------------------------------------------------
add_library(kordex_bindings STATIC ${KORDEX_BINDINGS_SOURCES})
add_library(kordex::bindings ALIAS kordex_bindings)

target_compile_features(kordex_bindings PUBLIC cxx_std_20)

target_include_directories(kordex_bindings
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(kordex_bindings
  PUBLIC
    ${KORDEX_BINDINGS_DEPENDENCIES}
)

target_compile_definitions(kordex_bindings
  PUBLIC
    KORDEX_BINDINGS_VERSION="${PROJECT_VERSION}"
)

if(KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE)
  target_compile_definitions(kordex_bindings PUBLIC KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE=1)
else()
  target_compile_definitions(kordex_bindings PUBLIC KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE=0)
endif()

if(KORDEX_BINDINGS_ENABLE_QUICKJS)
  target_compile_definitions(kordex_bindings PUBLIC KORDEX_BINDINGS_ENABLE_QUICKJS=1)
else()
  target_compile_definitions(kordex_bindings PUBLIC KORDEX_BINDINGS_ENABLE_QUICKJS=0)
endif()

if(KORDEX_BINDINGS_ENABLE_V8)
  target_compile_definitions(kordex_bindings PUBLIC KORDEX_BINDINGS_ENABLE_V8=1)
else()
  target_compile_definitions(kordex_bindings PUBLIC KORDEX_BINDINGS_ENABLE_V8=0)
endif()

if(KORDEX_BINDINGS_ENABLE_WARNINGS AND NOT MSVC)
  target_compile_options(kordex_bindings PRIVATE
    -Wall
    -Wextra
    -Wpedantic
  )
endif()

if(KORDEX_BINDINGS_ENABLE_SANITIZERS AND NOT MSVC)
  target_compile_options(kordex_bindings PRIVATE
    -fno-omit-frame-pointer
    -fsanitize=address,undefined
  )

  target_link_options(kordex_bindings PRIVATE
    -fsanitize=address,undefined
  )
endif()

set_target_properties(kordex_bindings PROPERTIES
  OUTPUT_NAME kordex_bindings
  VERSION ${PROJECT_VERSION}
  SOVERSION 0
  EXPORT_NAME bindings
)

install(TARGETS kordex_bindings
  EXPORT ${KORDEX_BINDINGS_EXPORT_SET}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# --------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------
if(NOT (DEFINED KORDEX_UMBRELLA_BUILD AND KORDEX_UMBRELLA_BUILD))
  install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
      PATTERN "*.hpp"
      PATTERN "*.h"
  )
endif()

# --------------------------------------------------------------------
# Standalone package config/export
# --------------------------------------------------------------------
if(NOT (DEFINED KORDEX_UMBRELLA_BUILD AND KORDEX_UMBRELLA_BUILD))

  configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/KordexBindingsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/kordex_bindingsConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/kordex_bindings"
  )

  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/kordex_bindingsConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )

  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/kordex_bindingsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/kordex_bindingsConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/kordex_bindings"
  )

  install(EXPORT kordex_bindingsTargets
    FILE kordex_bindingsTargets.cmake
    NAMESPACE kordex::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/kordex_bindings"
  )

endif()

# --------------------------------------------------------------------
# Tests dependency
# --------------------------------------------------------------------
if(KORDEX_BINDINGS_BUILD_TESTS)
  if(DEFINED KORDEX_UMBRELLA_BUILD AND KORDEX_UMBRELLA_BUILD)
    if(NOT TARGET vix::tests)
      message(FATAL_ERROR "[bindings] Umbrella build: vix::tests must be provided before bindings tests.")
    endif()
  else()
    if(NOT TARGET vix::tests)
      if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../tests/CMakeLists.txt")
        message(STATUS "[bindings] Adding tests from sibling: ../tests")
        add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../tests" "tests")
      else()
        find_package(vix_tests CONFIG QUIET)
      endif()

      if(NOT TARGET vix::tests)
        if(KORDEX_BINDINGS_FETCH_TESTS)
          include(FetchContent)
          message(STATUS "[bindings] Fetching vix::tests via FetchContent")
          FetchContent_Declare(vix_tests
            GIT_REPOSITORY https://github.com/vixcpp/tests.git
            GIT_TAG        ${KORDEX_VIX_GIT_TAG}
          )
          FetchContent_MakeAvailable(vix_tests)
        else()
          message(FATAL_ERROR "vix::tests not found.")
        endif()
      endif()
    endif()
  endif()
endif()

# --------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------
if(KORDEX_BINDINGS_BUILD_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()

# --------------------------------------------------------------------
# Examples
# --------------------------------------------------------------------
if(KORDEX_BINDINGS_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# --------------------------------------------------------------------
# Summary
# --------------------------------------------------------------------
message(STATUS "------------------------------------------------------")
message(STATUS "kordex::bindings configured (${PROJECT_VERSION})")
message(STATUS "Export set: ${KORDEX_BINDINGS_EXPORT_SET}")
message(STATUS "Runtime target: kordex::runtime")
message(STATUS "Error target:   vix::error")
message(STATUS "Log target:     vix::log")
message(STATUS "JSON target:    vix::json")
message(STATUS "Native engine:  ${KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE}")
message(STATUS "QuickJS:        ${KORDEX_BINDINGS_ENABLE_QUICKJS}")
message(STATUS "V8:             ${KORDEX_BINDINGS_ENABLE_V8}")
message(STATUS "Vix Git tag:    ${KORDEX_VIX_GIT_TAG}")
message(STATUS "Runtime tag:    ${KORDEX_RUNTIME_GIT_TAG}")
message(STATUS "Include dir: ${CMAKE_CURRENT_SOURCE_DIR}/include")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "------------------------------------------------------")
