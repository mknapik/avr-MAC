REM $Id: build_pal_example_led_control.bat 19934 2010-01-14 11:51:21Z sschneid $

REM *******************************************************
REM Build PAL Example Application Simple Remote LED Control
REM *******************************************************

REM     ATMEGA128RFA1_RCB_6_3_PLAIN

    cd ..\..\..\..\Applications\PAL_Examples\Simple_Remote_LED_Control\ATMEGA128RFA1_RCB_6_3_PLAIN\GCC\

    make clean -f Makefile
    make all -f Makefile

    cd ..\..\..\..\..\Build\Build_MAC\GCC\AVR\



