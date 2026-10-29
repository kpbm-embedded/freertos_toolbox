# Rust and Zig tasks for the qemu FreeRTOS layer (Cortex-M3), linked through the C ABI. The compilers are reached in
# their own IDs, not copied: Rust = $BOX_EMBEDDED_RUST or ../32_17_embedded_rust (rust-box, pinned rustc 1.99.1 with
# thumbv7m-none-eabi), Zig = $BOX_MICROZIG or ../32_24_microzig (microzig-box, Zig 0.17.0).
#   freertos_rust_staticlib(<tgt> <file.rs>)   freertos_zig_object(<tgt> <file.zig>)
# Projects using them ship needs.sh (exit 3 = skip when the ID is missing; scripts/app honours it).
include_guard(GLOBAL)
get_filename_component(_BOX_EMBEDDED "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)
if(DEFINED ENV{BOX_EMBEDDED_RUST})
  set(RUST_BOX "$ENV{BOX_EMBEDDED_RUST}/rust-box")
else()
  set(RUST_BOX "${_BOX_EMBEDDED}/32_17_embedded_rust/rust-box")
endif()
if(DEFINED ENV{BOX_MICROZIG})
  set(ZIG_BOX "$ENV{BOX_MICROZIG}/microzig-box")
else()
  set(ZIG_BOX "${_BOX_EMBEDDED}/32_24_microzig/microzig-box")
endif()

function(freertos_rust_staticlib tgt src)
  if(NOT EXISTS ${RUST_BOX})
    message(FATAL_ERROR "SKIP: needs 32_17_embedded_rust (rust-box not at ${RUST_BOX}; set BOX_EMBEDDED_RUST)")
  endif()
  get_filename_component(abs ${src} ABSOLUTE)
  set(lib ${CMAKE_CURRENT_BINARY_DIR}/lib${tgt}_rust.a)
  add_custom_command(OUTPUT ${lib}
    COMMAND ${CMAKE_COMMAND} -E env RUSTUP_TOOLCHAIN=1.99.1 ${RUST_BOX} run rustc --edition 2021 --crate-type staticlib
      --target thumbv7m-none-eabi -C opt-level=s -C panic=abort -C debuginfo=2 ${abs} -o ${lib}
    DEPENDS ${abs} COMMENT "rustc (32_17_embedded_rust) thumbv7m: ${src}" VERBATIM)
  add_custom_target(${tgt}_rust DEPENDS ${lib})
  add_dependencies(${tgt} ${tgt}_rust)
  target_link_libraries(${tgt} PRIVATE ${lib})
endfunction()

function(freertos_zig_object tgt src)
  if(NOT EXISTS ${ZIG_BOX})
    message(FATAL_ERROR "SKIP: needs 32_24_microzig (microzig-box not at ${ZIG_BOX}; set BOX_MICROZIG)")
  endif()
  get_filename_component(abs ${src} ABSOLUTE)
  set(obj ${CMAKE_CURRENT_BINARY_DIR}/${tgt}_zig.o)
  add_custom_command(OUTPUT ${obj}
    COMMAND ${ZIG_BOX} run zig build-obj -target thumb-freestanding-eabi -mcpu=cortex_m3 -O ReleaseSmall -fno-stack-check
      -fno-unwind-tables --cache-dir ${CMAKE_CURRENT_BINARY_DIR}/zig-cache --global-cache-dir ${CMAKE_CURRENT_BINARY_DIR}/zig-cache
      -femit-bin=${obj} ${abs}
    DEPENDS ${abs} COMMENT "zig (32_24_microzig) cortex-m3: ${src}" VERBATIM)
  add_custom_target(${tgt}_zig DEPENDS ${obj})
  add_dependencies(${tgt} ${tgt}_zig)
  target_link_libraries(${tgt} PRIVATE ${obj})
endfunction()
