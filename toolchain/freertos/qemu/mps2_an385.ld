/* Arm MPS2 AN385 (Cortex-M3) as emulated by qemu-system-arm -M mps2-an385:
 * 4 MB SSRAM1 at 0x00000000 (code, loaded by -kernel), 4 MB SSRAM2/3 at 0x20000000 (data). */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 4M
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 4M
}
ENTRY(Reset_Handler)
_estack = ORIGIN(RAM) + LENGTH(RAM);
_min_stack = 0x1000;   /* main stack (used before the scheduler starts and by interrupts) */

SECTIONS
{
  .isr_vector : { KEEP(*(.isr_vector)) } > FLASH
  .text : {
    *(.text*) *(.rodata*)
    KEEP(*(.init)) KEEP(*(.fini))
    . = ALIGN(4);
  } > FLASH
  .ARM.exidx : { __exidx_start = .; *(.ARM.exidx* .gnu.linkonce.armexidx.*) __exidx_end = .; } > FLASH
  _sidata = LOADADDR(.data);
  .data : {
    . = ALIGN(4); _sdata = .;
    *(.data*)
    . = ALIGN(4); _edata = .;
  } > RAM AT > FLASH
  .bss (NOLOAD) : {
    . = ALIGN(4); _sbss = .;
    *(.bss*) *(COMMON)
    . = ALIGN(4); _ebss = .;
  } > RAM
  .noinit (NOLOAD) : { . = ALIGN(4); *(.noinit*) . = ALIGN(4); } > RAM
  .heap (NOLOAD) : { . = ALIGN(8); end = .; _end = .; } > RAM
  ASSERT(_ebss + _min_stack < _estack, "RAM overflow")
}
