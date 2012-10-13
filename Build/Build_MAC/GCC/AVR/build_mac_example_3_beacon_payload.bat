REM $Id: build_mac_example_3_beacon_payload.bat 18416 2009-10-08 10:44:05Z sschneid $

REM ***********************************************
REM Build MAC Example App 3
REM ***********************************************

REM     ATMEGA128RFA1_RCB_6_3_SENS_TERM_BOARD Coordinator

    cd ..\..\..\..\Applications\MAC_Examples\App_3_Beacon_Payload\Coordinator\ATMEGA128RFA1_RCB_6_3_SENS_TERM_BOARD\GCC\

    make clean -f Makefile
    make all -f Makefile

    cd ..\..\..\..\..\..\Build\Build_MAC\GCC\AVR\

REM     ATMEGA128RFA1_RCB_6_3_SENS_TERM_BOARD Device

    cd ..\..\..\..\Applications\MAC_Examples\App_3_Beacon_Payload\Device\ATMEGA128RFA1_RCB_6_3_SENS_TERM_BOARD\GCC\

    make clean -f Makefile
    make all -f Makefile

    cd ..\..\..\..\..\..\Build\Build_MAC\GCC\AVR\