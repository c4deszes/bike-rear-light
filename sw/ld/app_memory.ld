OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
SEARCH_DIR(.)

ENTRY(Reset_Handler)

MEMORY
{
  bl_rom (LRX) : ORIGIN = 0x0, LENGTH = 0x02000
  app_rom (WXR) : ORIGIN = 0x02000, LENGTH = 0xA000
  bl_header (WR) : ORIGIN = 0x3FF00, LENGTH = 0x100
  bl_shared_ram (WX!R): ORIGIN = 0x20000000, LENGTH = 0x0008
  ram (WX!R) : ORIGIN = 0x20000008, LENGTH = 0x7FF8
}

/* The stack size used by the application. NOTE: you need to adjust according to your application. */
STACK_SIZE = DEFINED(STACK_SIZE) ? STACK_SIZE : DEFINED(__stack_size__) ? __stack_size__ : 0x04000;

__rom_end = ORIGIN(bl_rom) + LENGTH(bl_rom);
__ram_end = ORIGIN(ram) + LENGTH(ram);