REM $Id: iar_build_mac_example_star_network_system_clock_mhz.bat 17328 2009-08-19 08:19:25Z sschneid $

REM ***************************************************************
REM Build MAC Example Star Nobeacon for various system clock values
REM ***************************************************************


REM     AT86RF231_ATXMEGA128A1_REB_4_1_STK600

REM SYSTEM_CLOCK_MHZ: 4

    cd ..\..\..\..\Applications\MAC_Examples\Star_Nobeacon\AT86RF231_ATXMEGA128A1_REB_4_1_STK600

    iarbuild Star_4_MHZ.ewp -clean Release
    iarbuild Star_4_MHZ.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


REM SYSTEM_CLOCK_MHZ: 8

    cd ..\..\..\..\Applications\MAC_Examples\Star_Nobeacon\AT86RF231_ATXMEGA128A1_REB_4_1_STK600

    iarbuild Star_8_MHZ.ewp -clean Release
    iarbuild Star_8_MHZ.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


REM SYSTEM_CLOCK_MHZ: 32

    cd ..\..\..\..\Applications\MAC_Examples\Star_Nobeacon\AT86RF231_ATXMEGA128A1_REB_4_1_STK600

    iarbuild Star_32_MHZ.ewp -clean Release
    iarbuild Star_32_MHZ.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


