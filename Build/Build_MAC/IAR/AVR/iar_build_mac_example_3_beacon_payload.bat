REM $Id: iar_build_mac_example_3_beacon_payload.bat 18418 2009-10-08 10:51:44Z sschneid $

REM **********************************************
REM Build MAC Example Application App_3
REM **********************************************

    cd ..\..\..\..\Applications\MAC_Examples\App_3_Beacon_Payload\Coordinator\ATMEGA128RFA1_RCB_6_3_SENS_TERM_BOARD

    iarbuild Coordinator_Beacon_Payload.ewp -clean Release
    iarbuild Coordinator_Beacon_Payload.ewp -build Release

    cd ..\..\..\..\..\Build\Build_MAC\IAR\AVR\

    cd ..\..\..\..\Applications\MAC_Examples\App_3_Beacon_Payload\Device\ATMEGA128RFA1_RCB_6_3_SENS_TERM_BOARD

    iarbuild Device_Beacon_Payload.ewp -clean Release
    iarbuild Device_Beacon_Payload.ewp -build Release

    cd ..\..\..\..\..\Build\Build_MAC\IAR\AVR\
