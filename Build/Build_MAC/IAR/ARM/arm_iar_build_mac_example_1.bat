REM $Id: arm_iar_build_mac_example_1.bat 22389 2010-07-01 10:56:11Z sschneid $

REM ************************
REM ARM IAR Build batch file
REM ************************

REM ARM IAR Build path

set arm_iar_build="c:\Programme\IAR Systems\Embedded Workbench ARM 5.50\common\bin\iarbuild"

REM **********************************************
REM Build MAC Example Application App_1
REM **********************************************

    cd ..\..\..\..\Applications\MAC_Examples\App_1_Nobeacon\Coordinator\AT86RF230B_AT91SAM7X256_REB_2_3_REX_ARM_REV_2

    %arm_iar_build% Coordinator.ewp -clean Coordinator
    %arm_iar_build% Coordinator.ewp -build Coordinator

    cd ..\..\..\..\..\Build\Build_MAC\IAR\ARM\

    cd ..\..\..\..\Applications\MAC_Examples\App_1_Nobeacon\Device\AT86RF230B_AT91SAM7X256_REB_2_3_REX_ARM_REV_2

    %arm_iar_build% Device.ewp -clean Device
    %arm_iar_build% Device.ewp -build Device

    cd ..\..\..\..\..\Build\Build_MAC\IAR\ARM\


