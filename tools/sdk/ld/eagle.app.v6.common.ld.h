/* This linker script generated from xt-genldscripts.tpp for LSP . */
/* Linker Script for ld -N */

/* Set at build-time. See -DMMU_IRAM_SIZE=... in tools/boards.txt.py and tools/platformio-build.py */
MEMORY
{
  iram1_0_seg (rwx) : org = 0x40100000, len = MMU_IRAM_SIZE
}

/* Manually segment the output. Everything is load'able */
PHDRS
{
  dport0_0_phdr PT_LOAD FLAGS(6) ; /* ____ | PF_W | PF_R */
  dram0_0_phdr PT_LOAD FLAGS(6) ; /* ____ | PF_W | PF_R */
  dram0_0_bss_phdr PT_LOAD FLAGS(6) ; /* ____ | PF_W | PF_R */
  iram1_0_phdr PT_LOAD FLAGS(7) ; /* PF_X | PF_W | PF_R */
  irom0_0_phdr PT_LOAD FLAGS(5) ; /* PF_X | PF_W | ____ */
}


/*  Default entry point  */
ENTRY(app_entry)

/*  Exception vectors  */
EXTERN(_DebugExceptionVector)
EXTERN(_DoubleExceptionVector)
EXTERN(_KernelExceptionVector)
EXTERN(_NMIExceptionVector)
EXTERN(_UserExceptionVector)

/* see core_esp8266_main.cpp, usually stored to .ver_number */
EXTERN(core_version)

PROVIDE(_memmap_vecbase_reset = 0x40000000);

/* Various memory-map dependent cache attribute settings: */
_memmap_cacheattr_wb_base = 0x00000110;
_memmap_cacheattr_wt_base = 0x00000110;
_memmap_cacheattr_bp_base = 0x00000220;
_memmap_cacheattr_unused_mask = 0xFFFFF00F;
_memmap_cacheattr_wb_trapnull = 0x2222211F;
_memmap_cacheattr_wba_trapnull = 0x2222211F;
_memmap_cacheattr_wbna_trapnull = 0x2222211F;
_memmap_cacheattr_wt_trapnull = 0x2222211F;
_memmap_cacheattr_bp_trapnull = 0x2222222F;
_memmap_cacheattr_wb_strict = 0xFFFFF11F;
_memmap_cacheattr_wt_strict = 0xFFFFF11F;
_memmap_cacheattr_bp_strict = 0xFFFFF22F;
_memmap_cacheattr_wb_allvalid = 0x22222112;
_memmap_cacheattr_wt_allvalid = 0x22222112;
_memmap_cacheattr_bp_allvalid = 0x22222222;
PROVIDE(_memmap_cacheattr_reset = _memmap_cacheattr_wb_trapnull);

SECTIONS
{

  .dport0.rodata : ALIGN(4)
  {
    _dport0_rodata_start = ABSOLUTE(.);
    *(.dport0.rodata)
    *(.dport.rodata)
    _dport0_rodata_end = ABSOLUTE(.);
  } >dport0_0_seg :dport0_0_phdr

  .dport0.literal : ALIGN(4)
  {
    _dport0_literal_start = ABSOLUTE(.);
    *(.dport0.literal)
    *(.dport.literal)
    _dport0_literal_end = ABSOLUTE(.);
  } >dport0_0_seg :dport0_0_phdr

  .dport0.data : ALIGN(4)
  {
    _dport0_data_start = ABSOLUTE(.);
    *(.dport0.data)
    *(.dport.data)
    _dport0_data_end = ABSOLUTE(.);
  } >dport0_0_seg :dport0_0_phdr

  .data : ALIGN(4)
  {
    _data_start = ABSOLUTE(.);
    *(.data)
    *(.data.*)
    *(.gnu.linkonce.d.*)
    *(.data1)
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
    *(.sdata2)
    *(.sdata2.*)
    *(.gnu.linkonce.s2.*)
    *(.jcr)
    . = ALIGN(4);
    _Pri_3_HandlerAddress = ABSOLUTE(.);
    _data_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .noinit : ALIGN(4)
  {
    *(.noinit)
  } >dram0_0_seg :dram0_0_phdr

#ifdef VTABLES_IN_DRAM
#include "eagle.app.v6.common.ld.vtables.h"
#endif

  /* IRAM is split into .text and .text1 to allow for moving specific */
  /* functions into IRAM that would be matched by the irom0.text matcher */
  .text : ALIGN(4)
  {
    _stext = .;
    _text_start = ABSOLUTE(.);
     *(.UserEnter.text)
    . = ALIGN(16);
    *(.DebugExceptionVector.text)
    . = ALIGN(16);
    *(.NMIExceptionVector.text)
    . = ALIGN(16);
    *(.KernelExceptionVector.text)
    LONG(0)
    LONG(0)
    LONG(0)
    LONG(0)
    . = ALIGN(16);
    *(.UserExceptionVector.text)
    LONG(0)
    LONG(0)
    LONG(0)
    LONG(0)
    . = ALIGN(16);
    *(.DoubleExceptionVector.text)
    LONG(0)
    LONG(0)
    LONG(0)
    LONG(0)
    . = ALIGN (16);
    *(.entry.text)
    *(.init.literal)
    *(.init)

    *(.text.app_entry*)  /* The main startup code */

    *(.text.gdbstub* .text.gdb_init)    /* Any GDB hooks */

    /* all functional callers are placed in IRAM (including SPI/IRQ callbacks/etc) here */
    *(.text._ZNKSt8functionIF*EE*)  /* std::function<any(...)>::operator()() const */

#ifdef FP_IN_IRAM
    *libgcc.a:*f2.o(.literal .text)
    *libgcc.a:*f3.o(.literal .text)
    *libgcc.a:*fsi.o(.literal .text)
    *libgcc.a:*fdi.o(.literal .text)
    *libgcc.a:*ifs.o(.literal .text)
    *libgcc.a:*idf.o(.literal .text)
#endif

  } >iram1_0_seg :iram1_0_phdr

  .irom0.text (READONLY) : ALIGN(4)
  {
    _irom0_text_start = ABSOLUTE(.);

    /* Stuff the CRC in well known symbols at a well known location */
    __crc_len = ABSOLUTE(.);
    LONG(0x00000000);
    __crc_val = ABSOLUTE(.);
    LONG(0x00000000);

    *(.ver_number)
    *.c.o(.literal* .text*)
    *.cpp.o(EXCLUDE_FILE (umm_malloc.cpp.o) .literal* EXCLUDE_FILE (umm_malloc.cpp.o) .text*)
    *.cc.o(.literal* .text*)
#ifdef VTABLES_IN_FLASH
    *(.rodata._ZTV*) /* C++ vtables */
#endif

    . = ALIGN(4);       /* this table MUST be 4-byte aligned */
    /*  C++ constructor and destructor tables, properly ordered:  */
    __init_array_start = ABSOLUTE(.);
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    __init_array_end = ABSOLUTE(.);
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    . = ALIGN(4);       /* this table MUST be 4-byte aligned */
    _bss_table_start = ABSOLUTE(.);
    LONG(_bss_start)
    LONG(_bss_end)
    _bss_table_end = ABSOLUTE(.);

    *libgcc.a:unwind-dw2.o(.literal .text .rodata .literal.* .text.* .rodata.*)
    *libgcc.a:unwind-dw2-fde.o(.literal .text .rodata .literal.* .text.* .rodata.*)

    *libc.a:(.literal .text .literal.* .text.*)
    *libm.a:(.literal .text .literal.* .text.*)
#ifdef FP_IN_IROM
    *libgcc.a:*f2.o(.literal .text)
    *libgcc.a:*f3.o(.literal .text)
    *libgcc.a:*fsi.o(.literal .text)
    *libgcc.a:*fdi.o(.literal .text)
    *libgcc.a:*ifs.o(.literal .text)
    *libgcc.a:*idf.o(.literal .text)
#endif
    *libgcc.a:_umoddi3.o(.literal .text)
    *libgcc.a:_udivdi3.o(.literal .text)
    *libgcc.a:_divsf3.o(.literal .text)
    *libgcc.a:_fixsfsi.o(.literal .text)
    *libgcc.a:_cmpdf2.o(.literal .text)
    *libgcc.a:_cmpsf2.o(.literal .text)
    *libstdc++.a:( .literal .text .literal.* .text.*)
    *libstdc++-exc.a:( .literal .text .literal.* .text.*)
    *libsmartconfig.a:(.literal .text .literal.* .text.*)
    *liblwip_gcc.a:(.literal .text .literal.* .text.*)
    *liblwip_src.a:(.literal .text .literal.* .text.*)
    *liblwip2-536.a:(.literal .text .literal.* .text.*)
    *liblwip2-1460.a:(.literal .text .literal.* .text.*)
    *liblwip2-536-feat.a:(.literal .text .literal.* .text.*)
    *liblwip2-1460-feat.a:(.literal .text .literal.* .text.*)
    *liblwip6-536-feat.a:(.literal .text .literal.* .text.*)
    *liblwip6-1460-feat.a:(.literal .text .literal.* .text.*)
    *libbearssl.a:(.literal .text .literal.* .text.*)
    *libat.a:(.literal.* .text.*)
    *libcrypto.a:(.literal.* .text.*)
    *libespnow.a:(.literal.* .text.*)
    *libjson.a:(.literal.* .text.*)
    *liblwip.a:(.literal.* .text.*)
    *libmesh.a:(.literal.* .text.*)
    *libnet80211.a:(.literal.* .text.*)
    *libsmartconfig.a:(.literal.* .text.*)
    *libssl.a:(.literal.* .text.*)
    *libupgrade.a:(.literal.* .text.*)
    *libwpa.a:(.literal.* .text.*)
    *libwpa2.a:(.literal.* .text.*)
    *libwps.a:(.literal.* .text.*)
    *(.irom0.literal .irom.literal .irom.text.literal .irom0.text .irom0.text.* .irom.text .irom.text.*)

    /* Constant strings in flash (PSTRs) */
    *(.irom0.pstr.*)

    /* Inline flash strings PSTR() within templated code */
    *(.rodata._ZZ*__pstr__*)

    /* __FUNCTION__ locals */
    *(.rodata._ZZ*__FUNCTION__)
    *(.rodata._ZZ*__PRETTY_FUNCTION__)
    *(.rodata._ZZ*__func__)

    /* std::* exception strings, in their own section to allow string coalescing */
    *(.irom.exceptiontext .rodata.exceptiontext)
    *(.rodata.*__exception_what__*) /* G++ seems to throw out templatized section attributes */

    /* c++ typeof IDs, etc. */
    *(.rodata._ZTIN* .rodata._ZTSN10* .rodata._ZTISt* .rodata._ZTSSt*)

    /* Fundamental type info */
    *(.rodata._ZTIPKc .rodata._ZTIc .rodata._ZTIv .rodata._ZTSv .rodata._ZTSc .rodata._ZTSPKc .rodata._ZTSi .rodata._ZTIi)

    . = ALIGN(4);
    *(.gcc_except_table .gcc_except_table.*)
    . = ALIGN(4);
    __eh_frame = ABSOLUTE(.);
    KEEP(*(.eh_frame))
    . = (. + 7) & ~ 3;  /* Add a 0 entry to terminate the list */

    _irom0_text_end = ABSOLUTE(.);
    _flash_code_end = ABSOLUTE(.);
  } >irom0_0_seg :irom0_0_phdr

  .text1 : ALIGN(4)
  {
    *(.literal .text .iram.literal .iram.text .iram.text.* .literal.* .text.* .stub .gnu.warning .gnu.linkonce.literal.* .gnu.linkonce.t.*.literal .gnu.linkonce.t.*)
#ifdef VTABLES_IN_IRAM
    *(.rodata._ZTV*) /* C++ vtables */
#endif
    *(.fini.literal)
    *(.fini)
    *(.gnu.version)
    _text_end = ABSOLUTE(.);
    _etext = .;
  } >iram1_0_seg :iram1_0_phdr

#if defined(VTABLES_IN_IRAM) || defined(VTABLES_IN_FLASH)
#include "eagle.app.v6.common.ld.vtables.h"
#endif

  .lit4 : ALIGN(4)
  {
    _lit4_start = ABSOLUTE(.);
    *(*.lit4)
    *(.lit4.*)
    *(.gnu.linkonce.lit4.*)
    _lit4_end = ABSOLUTE(.);
  } >iram1_0_seg :iram1_0_phdr

  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */

  /* DWARF 1.  */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }

  /* GNU DWARF 1 extensions.  */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }

  /* DWARF 1.1 and DWARF 2.  */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }

  /* DWARF 2.  */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }

  /* SGI/MIPS DWARF 2 extensions.  */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }

  /* DWARF 3.  */
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  .debug_ranges   0 : { *(.debug_ranges) }

  /* DWARF 5.  */
  .debug_addr     0 : { *(.debug_addr) }
  .debug_line_str 0 : { *(.debug_line_str) }
  .debug_loclists 0 : { *(.debug_loclists) }
  .debug_macro    0 : { *(.debug_macro) }
  .debug_names    0 : { *(.debug_names) }
  .debug_rnglists 0 : { *(.debug_rnglists) }
  .debug_str_offsets 0 : { *(.debug_str_offsets) }
  .debug_sup      0 : { *(.debug_sup) }

}

/* get ROM code address */
INCLUDE "../ld/eagle.rom.addr.v6.ld"
