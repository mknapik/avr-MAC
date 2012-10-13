REM $Id: arm_iar_build_stb_example_secure_remote_control.bat 22389 2010-07-01 10:56:11Z sschneid $

REM ************************
REM ARM IAR Build batch file
REM ************************

REM ARM IAR Build path

set arm_iar_build="c:\Programme\IAR Systems\Embedded Workbench ARM 5.50\common\bin\iarbuild"

REM **********************************************
REM Build STB Example Application Secure_Remote_Control
REM **********************************************

    cd ..\..\..\..\Applications\STB_Examples\Secure_Remote_Control\AT86RF230B_AT91SAM7XC256_REB_2_3_REX_ARM_REV_2

    %arm_iar_build% Secure_Remote_Control.ewp -clean Secure_Remote_Control
    %arm_iar_build% Secure_Remote_Control.ewp -build Secure_Remote_Control

    cd ..\..\..\..\Build\Build_MAC\IAR\ARM\

