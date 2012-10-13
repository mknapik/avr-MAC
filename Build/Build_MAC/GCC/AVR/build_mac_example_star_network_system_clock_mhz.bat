REM $Id: build_mac_example_star_network_system_clock_mhz.bat 17932 2009-09-15 08:03:13Z sschneid $

REM ***************************************************************
REM Build MAC Example Star Nobeacon for various system clock values
REM ***************************************************************


REM     AT86RF231_ATXMEGA128A1_REB_4_1_STK600

REM SYSTEM_CLOCK_MHZ: 4

    cd ..\..\..\..\Applications\MAC_Examples\Star_Nobeacon\AT86RF231_ATXMEGA128A1_REB_4_1_STK600\GCC\

    make clean -f Makefile_4_MHZ
    make all -f Makefile_4_MHZ

    cd ..\..\..\..\..\Build\Build_MAC\GCC\AVR\


REM SYSTEM_CLOCK_MHZ: 8

    cd ..\..\..\..\Applications\MAC_Examples\Star_Nobeacon\AT86RF231_ATXMEGA128A1_REB_4_1_STK600\GCC\

    make clean -f Makefile_8_MHZ
    make all -f Makefile_8_MHZ

    cd ..\..\..\..\..\Build\Build_MAC\GCC\AVR\


REM SYSTEM_CLOCK_MHZ: 32

    cd ..\..\..\..\Applications\MAC_Examples\Star_Nobeacon\AT86RF231_ATXMEGA128A1_REB_4_1_STK600\GCC\

    make clean -f Makefile_32_MHZ
    make all -f Makefile_32_MHZ

    cd ..\..\..\..\..\Build\Build_MAC\GCC\AVR\


