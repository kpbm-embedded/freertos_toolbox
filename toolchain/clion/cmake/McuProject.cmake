# mcu/clion/cmake/McuProject.cmake - helpers for CLion projects whose firmware is built by an SDK's make.
#
#   mcu_firmware(
#     OUTPUT_DIR <dir>                 where the SDK build writes its artifacts (inside the CMake build dir)
#     BUILD <cmd...>                   command CLion's Build runs (target "firmware", part of ALL)
#     ARTIFACTS <file...>              produced files (cleaned by "Clean")
#     [INDEX_DRY_RUN <cmd...>]         make dry run whose compiler lines give the index flags
#     [INDEX_PREPARE <cmd...>]         run once at configure before reading INDEX_COMPDB
#     [INDEX_COMPDB <file>]            compile_commands.json to read instead of a dry run
#     [INDEX_ARGS <args...>]           extra args for lib/mcu_index.py (--map, --exclude, --compiler)
#     [CONFIGURE_DEPENDS <file...>])   files whose change makes CLion reload the project
#
# Defines target "firmware" (ALL) and "firmware-index" (OBJECT, EXCLUDE_FROM_ALL: only for CLion's
# code insight; it is never built). See ../README.md.
include_guard(GLOBAL)

if(NOT MCU_ROOT)
  get_filename_component(MCU_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
endif()
set(MCU_HOST_PYTHON "${MCU_ROOT}/host/python/cpython-3.12-linux-x86_64-gnu/bin/python3")
find_program(MCU_MAKE make
  HINTS "${MCU_ROOT}/host/buildtools/sysroots/x86_64-pokysdk-linux/usr/bin" NO_DEFAULT_PATH REQUIRED)
if(NOT MCU_JOBS)
  set(MCU_JOBS 16)
endif()

function(mcu_firmware)
  cmake_parse_arguments(F "" "OUTPUT_DIR;INDEX_COMPDB"
    "BUILD;ARTIFACTS;INDEX_DRY_RUN;INDEX_PREPARE;INDEX_ARGS;CONFIGURE_DEPENDS" ${ARGN})
  file(MAKE_DIRECTORY "${F_OUTPUT_DIR}")
  add_custom_target(firmware ALL
    COMMAND ${F_BUILD}
    WORKING_DIRECTORY "${F_OUTPUT_DIR}"
    BYPRODUCTS ${F_ARTIFACTS}
    USES_TERMINAL VERBATIM
    COMMENT "SDK build (make) -> ${F_OUTPUT_DIR}")

  set(_out "${CMAKE_BINARY_DIR}/mcu-index.cmake")
  if(F_INDEX_PREPARE)
    execute_process(COMMAND ${F_INDEX_PREPARE} WORKING_DIRECTORY "${F_OUTPUT_DIR}"
      RESULT_VARIABLE _rc OUTPUT_VARIABLE _o ERROR_VARIABLE _o)
    if(NOT _rc EQUAL 0)
      message(WARNING "index prepare step failed (${_rc}); code insight may be incomplete:\n${_o}")
    endif()
  endif()
  if(F_INDEX_COMPDB)
    set(_src --compdb "${F_INDEX_COMPDB}")
  else()
    set(_src --cwd "${F_OUTPUT_DIR}" -- ${F_INDEX_DRY_RUN})
  endif()
  execute_process(
    COMMAND "${MCU_HOST_PYTHON}" -I "${MCU_ROOT}/clion/lib/mcu_index.py"
            --out "${_out}" --target firmware-index ${F_INDEX_ARGS} ${_src}
    WORKING_DIRECTORY "${F_OUTPUT_DIR}"
    RESULT_VARIABLE _rc OUTPUT_VARIABLE _o ERROR_VARIABLE _e)
  message(STATUS "${_o}")
  if(NOT _rc EQUAL 0 OR NOT EXISTS "${_out}")
    message(WARNING "mcu_index failed (${_rc}): ${_e}")
  else()
    if(_e)
      message(STATUS "${_e}")
    endif()
    include("${_out}")
  endif()
  if(F_CONFIGURE_DEPENDS)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${F_CONFIGURE_DEPENDS})
  endif()
endfunction()
