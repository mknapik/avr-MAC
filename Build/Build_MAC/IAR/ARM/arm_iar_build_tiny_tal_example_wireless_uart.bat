REM $Id: arm_iar_build_tiny_tal_example_wireless_uart.bat 22389 2010-07-01 10:56:11Z sschneid $

REM ************************
REM ARM IAR Build batch file
REM ************************

REM ARM IAR Build path

set arm_iar_build="c:\Programme\IAR Systems\Embedded Workbench ARM 5.50\common\bin\iarbuild"

REM **********************************************
REM Build Tiny-TAL Example Application Wireless_UART
REM **********************************************

    cd ..\..\..\..\Applications\TINY_TAL_Examples\Wireless_UART\AT86RF230B_AT91SAM7X256_REB_2_3_REX_ARM_REV_2

    %arm_iar_build% Wireless_UART.ewp -clean Wireless_UART
    %arm_iar_build% Wireless_UART.ewp -build Wireless_UART

    cd ..\..\..\..\Build\Build_MAC\IAR\ARM\

