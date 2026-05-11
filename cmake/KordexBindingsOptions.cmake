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

include_guard(GLOBAL)

set(KORDEX_BINDINGS_OPTIONS_INCLUDED ON)

# --------------------------------------------------------------------
# Build options
# --------------------------------------------------------------------
option(KORDEX_BINDINGS_BUILD_TESTS "Build Kordex Bindings tests" OFF)
option(KORDEX_BINDINGS_BUILD_EXAMPLES "Build Kordex Bindings examples" OFF)

# --------------------------------------------------------------------
# QuickJS options
# --------------------------------------------------------------------
set(KORDEX_QUICKJS_GIT_REPOSITORY
    "https://github.com/bellard/quickjs.git"
    CACHE STRING
    "Git repository used for QuickJS")

set(KORDEX_QUICKJS_GIT_TAG
    "master"
    CACHE STRING
    "Git tag or branch used for QuickJS")

option(KORDEX_BINDINGS_FETCH_QUICKJS
    "Auto-fetch QuickJS when QuickJS backend is enabled"
    ON)

# --------------------------------------------------------------------
# Install options
# --------------------------------------------------------------------
option(KORDEX_BINDINGS_ENABLE_INSTALL
    "Generate install/export rules for Kordex Bindings"
    OFF)

# --------------------------------------------------------------------
# Developer options
# --------------------------------------------------------------------
option(KORDEX_BINDINGS_ENABLE_WARNINGS "Enable compiler warnings for Kordex Bindings" ON)
option(KORDEX_BINDINGS_ENABLE_SANITIZERS "Enable sanitizers for Kordex Bindings" OFF)

# --------------------------------------------------------------------
# Engine options
# --------------------------------------------------------------------
option(KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE "Enable the built-in minimal native engine" ON)
option(KORDEX_BINDINGS_ENABLE_QUICKJS "Enable QuickJS backend support" OFF)
option(KORDEX_BINDINGS_ENABLE_V8 "Enable V8 backend support" OFF)

# --------------------------------------------------------------------
# Dependency version policy
# --------------------------------------------------------------------
set(KORDEX_VIX_GIT_TAG
    "main"
    CACHE STRING
    "Git tag or branch used for Vix dependencies")

set(KORDEX_RUNTIME_GIT_TAG
    "main"
    CACHE STRING
    "Git tag or branch used for Kordex Runtime")

# --------------------------------------------------------------------
# Dependency fetch policy
# --------------------------------------------------------------------
option(KORDEX_BINDINGS_FETCH_KORDEX_DEPS "Auto-fetch missing Kordex dependencies" ON)
option(KORDEX_BINDINGS_FETCH_VIX_DEPS "Auto-fetch missing Vix dependencies" ON)
option(KORDEX_BINDINGS_FETCH_TESTS "Auto-fetch vix::tests if missing" ON)

set(KORDEX_BINDINGS_FETCH_RUNTIME
    ${KORDEX_BINDINGS_FETCH_KORDEX_DEPS}
    CACHE BOOL
    "Auto-fetch kordex::runtime if missing")

set(KORDEX_BINDINGS_FETCH_ERROR
    ${KORDEX_BINDINGS_FETCH_VIX_DEPS}
    CACHE BOOL
    "Auto-fetch vix::error if missing")

set(KORDEX_BINDINGS_FETCH_LOG
    ${KORDEX_BINDINGS_FETCH_VIX_DEPS}
    CACHE BOOL
    "Auto-fetch vix::log if missing")

set(KORDEX_BINDINGS_FETCH_JSON
    ${KORDEX_BINDINGS_FETCH_VIX_DEPS}
    CACHE BOOL
    "Auto-fetch vix::json if missing")

# --------------------------------------------------------------------
# Umbrella build policy
# --------------------------------------------------------------------
if(DEFINED KORDEX_UMBRELLA_BUILD AND KORDEX_UMBRELLA_BUILD)
  set(KORDEX_BINDINGS_FETCH_KORDEX_DEPS OFF CACHE BOOL "Auto-fetch missing Kordex dependencies" FORCE)
  set(KORDEX_BINDINGS_FETCH_VIX_DEPS OFF CACHE BOOL "Auto-fetch missing Vix dependencies" FORCE)
  set(KORDEX_BINDINGS_FETCH_TESTS OFF CACHE BOOL "Auto-fetch vix::tests if missing" FORCE)
  set(KORDEX_BINDINGS_FETCH_QUICKJS OFF CACHE BOOL "Auto-fetch QuickJS when QuickJS backend is enabled" FORCE)

  set(KORDEX_BINDINGS_FETCH_RUNTIME OFF CACHE BOOL "Auto-fetch kordex::runtime if missing" FORCE)
  set(KORDEX_BINDINGS_FETCH_ERROR OFF CACHE BOOL "Auto-fetch vix::error if missing" FORCE)
  set(KORDEX_BINDINGS_FETCH_LOG OFF CACHE BOOL "Auto-fetch vix::log if missing" FORCE)
  set(KORDEX_BINDINGS_FETCH_JSON OFF CACHE BOOL "Auto-fetch vix::json if missing" FORCE)
endif()

# --------------------------------------------------------------------
# Backend validation
# --------------------------------------------------------------------
if(KORDEX_BINDINGS_ENABLE_QUICKJS AND KORDEX_BINDINGS_ENABLE_V8)
  message(FATAL_ERROR
      "Kordex Bindings cannot enable both QuickJS and V8 at the same time. "
      "Choose one JavaScript backend.")
endif()

if(NOT KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE
   AND NOT KORDEX_BINDINGS_ENABLE_QUICKJS
   AND NOT KORDEX_BINDINGS_ENABLE_V8)
  message(FATAL_ERROR
      "Kordex Bindings needs at least one engine backend. "
      "Enable KORDEX_BINDINGS_ENABLE_NATIVE_ENGINE, "
      "KORDEX_BINDINGS_ENABLE_QUICKJS, or KORDEX_BINDINGS_ENABLE_V8.")
endif()
