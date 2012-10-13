REM $Id: build_mac_example_star_high_rate.bat 22888 2010-08-11 14:45:33Z sschneid $

REM ***********************************************
REM Build MAC Example Star High Rate
REM ***********************************************

REM     ATMEGA128RFA1_RCB_6_3_SENS_TERM_BOARD

    cd ..\..\..\..\Applications\MAC_Examples\Star_High_Rate\ATMEGA128RFA1_RCB_6_3_SENS_TERM_BOARD\GCC\

    make clean -f Makefile
    make all -f Makefile

    cd ..\..\..\..\..\Build\Build_MAC\GCC\AVR\
