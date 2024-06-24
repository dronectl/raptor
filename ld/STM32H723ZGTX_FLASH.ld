/**
 ******************************************************************************
 * @file      STM32H723ZGTX_FLASH.ld
 * @author    dronectl
 *  Abstract    : Linker script for NUCLEO-H723ZG Board embedding STM32H723ZGTx Device from stm32h7 series
 *
 *            Set heap size, stack size and stack location according
 *            to application requirements.
 *            Set memory bank area and size if external memory is used
 * Copyright (c) 2024 dronectl.
 * All rights reserved.
 */

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM_DTCM) + LENGTH(RAM_DTCM);	/* end of "RAM" Ram type memory */

_Min_Heap_Size = 0x2000;	/* required amount of heap  */
_Min_Stack_Size = 0x2000;	/* required amount of stack */

/* Memories definition : 2.3.2 Memory map and register boundary addresses */
MEMORY
{
  RAM_ITCM            (xrw)    : ORIGIN = 0x00000000,   LENGTH = 64K            /* 0x00000000 - 0x0000FFFF */
  RAM_ITCM_AXI1       (xrw)    : ORIGIN = 0x00010000,   LENGTH = 192K           /* 0x00010000 - 0x0003FFFF */
  FLASH1              (rx)     : ORIGIN = 0x08000000,   LENGTH = 1024K          /* 0x08000000 - 0x080FFFFF */
  RAM_DTCM            (xrw)    : ORIGIN = 0x20000000,   LENGTH = 128K           /* 0x20000000 - 0x2001FFFF */
  RAM_AXI             (xrw)    : ORIGIN = 0x24000000,   LENGTH = 128K           /* 0x24000000 - 0x2401FFFF */
  RAM_ITCM_AXI2       (xrw)    : ORIGIN = 0x24020000,   LENGTH = 192K           /* 0x24020000 - 0x2404FFFF */
  RAM_D1              (xrw)    : ORIGIN = 0x30000000,   LENGTH = 16K            /* 0x30000000 - 0x30003FFF */
  RAM_D2              (xrw)    : ORIGIN = 0x30003FFF,   LENGTH = 16K            /* 0x30004000 - 0x30007FFF */
  RAM_D3              (xrw)    : ORIGIN = 0x38000000,   LENGTH = 16K            /* 0x38000000 - 0x38003FFF */
  RAM_D4              (xrw)    : ORIGIN = 0x38800000,   LENGTH = 4K             /* 0x38800000 - 0x38800FFF */
}

PHDRS 
{
  isr PT_LOAD FLAGS(2);
  rodata PT_LOAD FLAGS(4);
  text PT_LOAD FLAGS(5);
  data PT_LOAD FLAGS(6);
  bss PT_LOAD FLAGS(6);
  user PT_LOAD FLAGS(2);
}

/* Sections */
SECTIONS
{
  /* The startup code into "FLASH1" Rom type memory */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH1 : isr

  /* The program code and other data into "FLASH1" Rom type memory */
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
  } >FLASH1 : text

  /* Constant data into "FLASH1" Rom type memory */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH1 : rodata

  /* ARM exception table */
  .ARM.extab (READONLY):
  { 
    . = ALIGN(4);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } >FLASH1
  
  .ARM (READONLY):
  {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4);
  } >FLASH1

  .preinit_array  (READONLY):
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(4);
  } >FLASH1
  
  .init_array  (READONLY):
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(4);
  } >FLASH1
  
  .fini_array  (READONLY):
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
  } >FLASH1

  /* Used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections into "RAM" Ram type memory */
  .data : 
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
    
  } >RAM_AXI AT> FLASH1 : data

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
  } >RAM_AXI : bss

  /* User_heap_stack section, used to check that there is enough "RAM" Ram  type memory left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } > RAM_DTCM : user
  
  .lwip_sec (NOLOAD) : {
    . = ABSOLUTE(0x30000000);
    *(.RxDecripSection) 
    
    . = ABSOLUTE(0x30000200);
    *(.TxDecripSection)
    
    . = ABSOLUTE(0x30000400);
    *(.Rx_PoolSection) 
  } >RAM_D1 AT> FLASH1

  /* Remove information from the compiler libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
