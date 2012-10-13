REM $Id: arm_iar_build_mac_example_promiscuous_mode_demo.bat 22389 2010-07-01 10:56:11Z sschneid $

REM ************************
REM ARM IAR Build batch file
REM ************************

REM ARM IAR Build path

set arm_iar_build="c:\Programme\IAR Systems\Embedded Workbench ARM 5.50\common\bin\iarbuild"

REM **********************************************
REM Build MAC Example Application Promiscuous_Mode_Demo
REM **********************************************

    cd ..\..\..\..\Applications\MAC_Examples\Promiscuous_Mode_Demo\AT86RF230B_AT91SAM7X256_REB_2_3_REX_ARM_REV_2

    %arm_iar_build% Promiscuous_Mode_Demo.ewp -clean Promiscuous_Mode_Demo
    %arm_iar_build% Promiscuous_Mode_Demo.ewp -build Promiscuous_Mode_Demo

    cd ..\..\..\..\Build\Build_MAC\IAR\ARM\

