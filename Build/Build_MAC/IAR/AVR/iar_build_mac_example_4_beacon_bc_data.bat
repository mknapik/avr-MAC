REM $Id: iar_build_mac_example_4_beacon_bc_data.bat 18408 2009-10-08 09:27:07Z sschneid $

REM **********************************************
REM Build MAC Example Application App_4
REM **********************************************

    cd ..\..\..\..\Applications\MAC_Examples\App_4_Beacon_Broadcast_Data\Coordinator\ATMEGA128RFA1_RCB_6_3_SENS_TERM_BOARD

    iarbuild Coordinator_BC_Data.ewp -clean Release
    iarbuild Coordinator_BC_Data.ewp -build Release

    cd ..\..\..\..\..\Build\Build_MAC\IAR\AVR\

    cd ..\..\..\..\Applications\MAC_Examples\App_4_Beacon_Broadcast_Data\Device\ATMEGA128RFA1_RCB_6_3_SENS_TERM_BOARD

    iarbuild Device_BC_Data.ewp -clean Release
    iarbuild Device_BC_Data.ewp -build Release

    cd ..\..\..\..\..\Build\Build_MAC\IAR\AVR\
