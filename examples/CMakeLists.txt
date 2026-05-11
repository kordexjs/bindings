#  @file examples/CMakeLists.txt
#  @author Softadastra
#
#  Copyright 2026, Softadastra. All rights reserved.
#  https://github.com/softadastra/kordex-bindings
#  Use of this source code is governed by a MIT license
#  that can be found in the LICENSE file.
#
# ====================================================================
# Kordex Bindings - Examples
# ====================================================================

include_guard(GLOBAL)

# --------------------------------------------------------------------
# Validation
# --------------------------------------------------------------------
if(NOT TARGET kordex::bindings)
  message(FATAL_ERROR
      "[bindings/examples] Required target 'kordex::bindings' was not found. "
      "Examples must be added from the parent kordex-bindings CMake project.")
endif()

# --------------------------------------------------------------------
# Helper
# --------------------------------------------------------------------
function(kordex_bindings_add_example target_name source_file)
  add_executable(${target_name}
      ${source_file})

  target_compile_features(${target_name}
      PRIVATE
        cxx_std_20)

  target_link_libraries(${target_name}
      PRIVATE
        kordex::bindings)

  if(KORDEX_BINDINGS_ENABLE_WARNINGS)
    target_compile_options(${target_name}
        PRIVATE
          $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
          $<$<CXX_COMPILER_ID:MSVC>:/W4>)
  endif()
endfunction()

# --------------------------------------------------------------------
# Examples
# --------------------------------------------------------------------
kordex_bindings_add_example(kordex_bindings_bridge_runtime_example bridge_runtime.cpp)
kordex_bindings_add_example(kordex_bindings_create_engine_example create_engine.cpp)
kordex_bindings_add_example(kordex_bindings_expose_function_example expose_function.cpp)
kordex_bindings_add_example(kordex_bindings_expose_module_example expose_module.cpp)
kordex_bindings_add_example(kordex_bindings_quickjs_eval_example quickjs_eval.cpp)
kordex_bindings_add_example(kordex_bindings_run_script_example run_script.cpp)
