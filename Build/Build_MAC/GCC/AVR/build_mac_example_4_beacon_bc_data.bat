REM $Id: build_mac_example_4_beacon_bc_data.bat 18403 2009-10-08 09:03:15Z sschneid $

REM ***********************************************
REM Build MAC Example App 4
REM ***********************************************

REM     ATMEGA128RFA1_RCB_6_3_SENS_TERM_BOARD Coordinator

    cd ..\..\..\..\Applications\MAC_Examples\App_4_Beacon_Broadcast_Data\Coordinator\ATMEGA128RFA1_RCB_6_3_SENS_TERM_BOARD\GCC\

    make clean -f Makefile
    make all -f Makefile

    cd ..\..\..\..\..\..\Build\Build_MAC\GCC\AVR\

REM     ATMEGA128RFA1_RCB_6_3_SENS_TERM_BOARD Device

    cd ..\..\..\..\Applications\MAC_Examples\App_4_Beacon_Broadcast_Data\Device\ATMEGA128RFA1_RCB_6_3_SENS_TERM_BOARD\GCC\

    make clean -f Makefile
    make all -f Makefile

    cd ..\..\..\..\..\..\Build\Build_MAC\GCC\AVR\