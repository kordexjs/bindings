#  @file tests/CMakeLists.txt
#  @author Softadastra
#
#  Copyright 2026, Softadastra. All rights reserved.
#  https://github.com/softadastra/kordex-bindings
#  Use of this source code is governed by a MIT license
#  that can be found in the LICENSE file.
#
# ====================================================================
# Kordex Bindings - Tests
# ====================================================================

include_guard(GLOBAL)

# --------------------------------------------------------------------
# Validation
# --------------------------------------------------------------------
if(NOT TARGET kordex::bindings)
  message(FATAL_ERROR
      "[bindings/tests] Required target 'kordex::bindings' was not found. "
      "Tests must be added from the parent kordex-bindings CMake project.")
endif()

# --------------------------------------------------------------------
# Helper
# --------------------------------------------------------------------
function(kordex_bindings_add_test target_name source_file)
  add_executable(${target_name}
      ${source_file})

  target_compile_features(${target_name}
      PRIVATE
        cxx_std_20)

  target_link_libraries(${target_name}
      PRIVATE
        kordex::bindings)

  if(TARGET vix::tests)
    target_link_libraries(${target_name}
        PRIVATE
          vix::tests)
  endif()

  if(KORDEX_BINDINGS_ENABLE_WARNINGS)
    target_compile_options(${target_name}
        PRIVATE
          $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
          $<$<CXX_COMPILER_ID:MSVC>:/W4>)
  endif()

  add_test(
      NAME ${target_name}
      COMMAND ${target_name})
endfunction()

# --------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------
kordex_bindings_add_test(kordex_bindings_test_binding_config test_binding_config.cpp)
kordex_bindings_add_test(kordex_bindings_test_binding_options test_binding_options.cpp)
kordex_bindings_add_test(kordex_bindings_test_binding_result test_binding_result.cpp)
kordex_bindings_add_test(kordex_bindings_test_engine test_engine.cpp)
kordex_bindings_add_test(kordex_bindings_test_engine_context test_engine_context.cpp)
kordex_bindings_add_test(kordex_bindings_test_error test_error.cpp)
kordex_bindings_add_test(kordex_bindings_test_function test_function.cpp)
kordex_bindings_add_test(kordex_bindings_test_module test_module.cpp)
kordex_bindings_add_test(kordex_bindings_test_module_registry test_module_registry.cpp)
kordex_bindings_add_test(kordex_bindings_test_native_function test_native_function.cpp)
kordex_bindings_add_test(kordex_bindings_test_native_module test_native_module.cpp)
kordex_bindings_add_test(kordex_bindings_test_object test_object.cpp)
kordex_bindings_add_test(kordex_bindings_test_quickjs_backend test_quickjs_backend.cpp)
kordex_bindings_add_test(kordex_bindings_test_quickjs_errors test_quickjs_errors.cpp)
kordex_bindings_add_test(kordex_bindings_test_quickjs_values test_quickjs_values.cpp)
kordex_bindings_add_test(kordex_bindings_test_runtime_bridge test_runtime_bridge.cpp)
kordex_bindings_add_test(kordex_bindings_test_script test_script.cpp)
kordex_bindings_add_test(kordex_bindings_test_script_result test_script_result.cpp)
kordex_bindings_add_test(kordex_bindings_test_value test_value.cpp)
kordex_bindings_add_test(kordex_bindings_test_version test_version.cpp)
