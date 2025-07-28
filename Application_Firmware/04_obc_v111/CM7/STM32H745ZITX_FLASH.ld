/*
******************************************************************************
**
**  File        : LinkerScript.ld
**
**  Author      : STM32CubeIDE
**
**  Abstract    : Linker script for STM32H7 series
**                      1024Kbytes FLASH
**                       800Kbytes RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**  Distribution: The file is distributed as is without any warranty
**                of any kind.
**
*****************************************************************************
** @attention
**
** Copyright (c) 2025 STMicroelectronics.
** All rights reserved.
**
** This software is licensed under terms that can be found in the LICENSE file
** in the root directory of this software component.
** If no LICENSE file comes with this software, it is provided AS-IS.
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM_D1) + LENGTH(RAM_D1); /* end of "RAM_D1" Ram type memory */

_Min_Heap_Size = 0x200; /* required amount of heap  */
_Min_Stack_Size = 0x800; /* required amount of stack */

/* Memories definition */
MEMORY
{
  RAM_D1 (xrw)   : ORIGIN = 0x24000000, LENGTH =  512K
  FLASH  (rx)    : ORIGIN = 0x08040000, LENGTH = 384K    /* Memory is divided. Actual start is 0x08000000 and actual length is 2048K */
  DTCMRAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 128K
  RAM_D2 (xrw)   : ORIGIN = 0x30000000, LENGTH = 288K
  RAM_D3 (xrw)   : ORIGIN = 0x38000000, LENGTH = 62K
  SHARED_RAM (xrw): ORIGIN = 0x3800F800, LENGTH = 2K
  ITCMRAM (xrw)  : ORIGIN = 0x00000000, LENGTH = 64K
  BKPRAM (xrw)  : ORIGIN = 0x38800000, LENGTH = 4K
}

/* Sections */
SECTIONS
{
  .dma_buffer(NOLOAD) : /* Space before ':' is critical */
  {
    . = ALIGN(4);     
    *(.dma_buffer)
    . = ALIGN(4);
  } >RAM_D1

  .fs_buffer(NOLOAD) :
  {
    . = ALIGN(4);       
    *(.fs_buffer) 
    . = ALIGN(4);
  } >RAM_D1

  /* The startup code into "FLASH" Rom type memory */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data into "FLASH" Rom type memory */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data into "FLASH" Rom type memory */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } >FLASH
  .ARM (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4);
  } >FLASH

  .preinit_array (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .init_array (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .fini_array (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
  } >FLASH

  /* Used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections into "RAM" Ram type memory */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    *(.RamFunc)        /* .RamFunc sections */
    *(.RamFunc*)       /* .RamFunc* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM_D1 AT> FLASH

  /* Uninitialized data section into "RAM" Ram type memory */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM_D1

  /* User_heap_stack section, used to check that there is enough "RAM" Ram  type memory left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM_D1
  
  .ram_data_transfer(NOLOAD) :
  {
    . = ALIGN(4);
    /* Giữ lại toàn bộ dữ liệu của các input section .ram_data_bridge */
    KEEP(*(.ram_data_transfer))
    /* Nếu bạn có thêm các section con như .ram_data_bridge.* thì: */
    KEEP(*(.ram_data_transfer*))
    . = ALIGN(4);
    _sram_data_transfer = .;
    . = . + 0x1000;
    _eram_data_transfer = .;
    . = ALIGN(4);
  } >RAM_D1  
  
  .dmesg_buffer (NOLOAD) :
  {
	    . = ALIGN(4);
	    _sdmesg_buffer = .;    /* startpoint buffer */
	    . = . + 0x8000;     /* 32KB = 0x8000 bytes */
	    _edmesg_buffer = .;    /* endpoint buffer */
  } >DTCMRAM

  .custom_data (NOLOAD) :
  {
    . = ALIGN(4);
    _scustom_data = .;        /* startpoint custom_data */
    . = . + 0x8000;          /* 32KB = 0x8000 bytes (20 * 1024) */
    _ecustom_data = .;        /* endpoint custom_data */
  } >RAM_D2	

  .log_data (NOLOAD) :
  {
    . = ALIGN(4);
    _slog_data = .;          /* Start of log data */
    *(.log_data)             /* Any variables placed in .log_data */
    . = . + 0x14000;         /* 80KB = 0x14000 bytes */
    _elog_data = .;          /* End of log data */
  } >RAM_D2
  
  .ram_data_bridge (NOLOAD) :
  {
    . = ALIGN(4);
    /* Giữ lại toàn bộ dữ liệu của các input section .ram_data_bridge */
    KEEP(*(.ram_data_bridge))
    /* Nếu bạn có thêm các section con như .ram_data_bridge.* thì: */
    KEEP(*(.ram_data_bridge*))
    . = ALIGN(4);
    _sram_data_bridge = .;
    . = . + 0x1000;
    _eram_data_bridge = .;
    . = ALIGN(4);
  } >RAM_D3

  .shared_mem (NOLOAD) :
  {
    . = ALIGN(4);
    KEEP(*(.shared_mem))
  } >SHARED_RAM
  
  .bkpram (NOLOAD) :
  {
    *(.bkpram .bkpram.*)
    . = ALIGN(4);
  } > BKPRAM
  /* Remove information from the compiler libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
