#  @file KordexBindingsOptions.cmake
#  @author Softadastra
#
#  Copyright 2026, Softadastra. All rights reserved.
#  https://github.com/softadastra/kordex-bindings
#  Use of this source code is governed by a MIT license
#  that can be found in the LICENSE file.
#
# ====================================================================
# Kordex Bindings - Build Options
# ====================================================================

# ifndef(KORDEX_BINDINGS_OPTIONS_INCLUDED)
set(KORDEX_BINDINGS_OPTIONS_INCLUDED ON)

# --------------------------------------------------------------------
# Build options
# --------------------------------------------------------------------
option(KORDEX_BINDINGS_BUILD_TESTS "Build Kordex bindings tests" OFF)
option(KORDEX_BINDINGS_BUILD_EXAMPLES "Build Kordex bindings examples" OFF)

# --------------------------------------------------------------------
# Developer options
# --------------------------------------------------------------------
option(KORDEX_BINDINGS_ENABLE_WARNINGS "Enable compiler warnings for Kordex bindings" ON)
option(KORDEX_BINDINGS_ENABLE_SANITIZERS "Enable sanitizers for Kordex bindings" OFF)

# --------------------------------------------------------------------
# Engine options
# --------------------------------------------------------------------
option(KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE "Enable the built-in minimal native engine" ON)
option(KORDEX_BINDINGS_ENABLE_QUICKJS "Enable QuickJS backend support" OFF)
option(KORDEX_BINDINGS_ENABLE_V8 "Enable V8 backend support" OFF)

# --------------------------------------------------------------------
# Dependency fetch options
# --------------------------------------------------------------------
option(KORDEX_BINDINGS_FETCH_RUNTIME "Auto-fetch kordex::runtime if missing" ON)
option(KORDEX_BINDINGS_FETCH_ERROR "Auto-fetch vix::error if missing" ON)
option(KORDEX_BINDINGS_FETCH_LOG "Auto-fetch vix::log if missing" ON)
option(KORDEX_BINDINGS_FETCH_JSON "Auto-fetch vix::json if missing" ON)
option(KORDEX_BINDINGS_FETCH_TESTS "Auto-fetch vix::tests if missing" ON)

# --------------------------------------------------------------------
# Shared Vix dependency version
# --------------------------------------------------------------------
# Kordex modules often depend on Vix modules during standalone builds.
# By default we use main because Kordex and Vix are developed together and
# tags may not always be published immediately.
# --------------------------------------------------------------------
set(KORDEX_VIX_GIT_TAG main CACHE STRING "Git branch/tag used to fetch Vix modules")

# --------------------------------------------------------------------
# Shared Kordex dependency version
# --------------------------------------------------------------------
set(KORDEX_RUNTIME_GIT_TAG main CACHE STRING "Git branch/tag used to fetch Kordex runtime")

# --------------------------------------------------------------------
# Umbrella build policy
# --------------------------------------------------------------------
# When Kordex bindings is built inside the kordex umbrella repository,
# dependencies should be provided by the umbrella build.
#
# In that mode, bindings must not fetch dependencies by itself.
# The root project is responsible for add_subdirectory order.
# --------------------------------------------------------------------
if(DEFINED KORDEX_UMBRELLA_BUILD AND KORDEX_UMBRELLA_BUILD)
  set(KORDEX_BINDINGS_FETCH_RUNTIME OFF CACHE BOOL "Auto-fetch kordex::runtime if missing" FORCE)
  set(KORDEX_BINDINGS_FETCH_ERROR OFF CACHE BOOL "Auto-fetch vix::error if missing" FORCE)
  set(KORDEX_BINDINGS_FETCH_LOG OFF CACHE BOOL "Auto-fetch vix::log if missing" FORCE)
  set(KORDEX_BINDINGS_FETCH_JSON OFF CACHE BOOL "Auto-fetch vix::json if missing" FORCE)
  set(KORDEX_BINDINGS_FETCH_TESTS OFF CACHE BOOL "Auto-fetch vix::tests if missing" FORCE)
endif()

# --------------------------------------------------------------------
# Backend validation
# --------------------------------------------------------------------
if(KORDEX_BINDINGS_ENABLE_QUICKJS AND KORDEX_BINDINGS_ENABLE_V8)
  message(FATAL_ERROR
    "Kordex bindings cannot enable both QuickJS and V8 at the same time. "
    "Choose one JavaScript backend."
  )
endif()

if(NOT KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE AND
   NOT KORDEX_BINDINGS_ENABLE_QUICKJS AND
   NOT KORDEX_BINDINGS_ENABLE_V8)
  message(FATAL_ERROR
    "Kordex bindings needs at least one engine backend. "
    "Enable KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE, "
    "KORDEX_BINDINGS_ENABLE_QUICKJS, or KORDEX_BINDINGS_ENABLE_V8."
  )
endif()

# endif()
