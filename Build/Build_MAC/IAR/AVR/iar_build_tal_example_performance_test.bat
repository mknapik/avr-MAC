REM $Id: iar_build_tal_example_performance_test.bat 22790 2010-08-09 06:13:34Z sschneid $

REM **********************************************
REM Build TAL Example Application Performance_Test
REM **********************************************

    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\AT86RF212_ATMEGA1281_RCB_5_3_SENS_TERM_BOARD

    iarbuild Performance.ewp -clean Release
    iarbuild Performance.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\AT86RF212_ATMEGA1281_REB_5_0_STK500_STK501

    iarbuild Performance.ewp -clean Release
    iarbuild Performance.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\AT86RF212_ATXMEGA128A1_REB_5_0_STK600

    iarbuild Performance.ewp -clean Release
    iarbuild Performance.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\AT86RF230B_AT90USB1287_USBSTICK_C

    iarbuild Performance.ewp -clean Release
    iarbuild Performance.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\AT86RF230B_ATMEGA1281_RCB_3_2_SENS_TERM_BOARD

    iarbuild Performance.ewp -clean Release
    iarbuild Performance.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\AT86RF230B_ATMEGA1281_REB_2_3_STK500_STK501

    iarbuild Performance.ewp -clean Release
    iarbuild Performance.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\AT86RF230B_ATXMEGA128A1_RZ600_230B_XPLAIN

    iarbuild Performance.ewp -clean Release
    iarbuild Performance.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\AT86RF231_ATMEGA1281_RCB_4_0_SENS_TERM_BOARD

    iarbuild Performance.ewp -clean Release
    iarbuild Performance.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\AT86RF231_ATMEGA1281_RCB_4_1_SENS_TERM_BOARD

    iarbuild Performance.ewp -clean Release
    iarbuild Performance.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\AT86RF231_ATMEGA1281_REB_4_0_STK500_STK501

    iarbuild Performance.ewp -clean Release
    iarbuild Performance.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\AT86RF231_ATMEGA1281_REB_4_1_STK500_STK501

    iarbuild Performance.ewp -clean Release
    iarbuild Performance.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\AT86RF231_ATXMEGA128A1_REB_4_0_STK600

    iarbuild Performance.ewp -clean Release
    iarbuild Performance.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\AT86RF231_ATXMEGA128A1_REB_4_1_STK600

    iarbuild Performance.ewp -clean Release
    iarbuild Performance.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\AT86RF231_ATXMEGA256D3_REB_4_1_STK600

    iarbuild Performance.ewp -clean Release
    iarbuild Performance.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\ATMEGA128RFA1_EK1

    iarbuild Performance.ewp -clean Release
    iarbuild Performance.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\


    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\ATMEGA128RFA1_RCB_6_3_SENS_TERM_BOARD

    iarbuild Performance.ewp -clean Release
    iarbuild Performance.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\

