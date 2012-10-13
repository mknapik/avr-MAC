REM $Id: iar_build_mac_example_star_high_rate.bat 22901 2010-08-12 11:15:06Z sschneid $

REM **********************************************
REM Build MAC Example Application Star High Rate
REM **********************************************

    cd ..\..\..\..\Applications\MAC_Examples\Star_High_Rate\ATMEGA128RFA1_RCB_6_3_SENS_TERM_BOARD

    iarbuild Star_High_Rate.ewp -clean Release
    iarbuild Star_High_Rate.ewp -build Release

    cd ..\..\..\..\Build\Build_MAC\IAR\AVR\
