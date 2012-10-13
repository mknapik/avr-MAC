REM $Id: arm_iar_build_mac_example_star_network.bat 22389 2010-07-01 10:56:11Z sschneid $

REM ************************
REM ARM IAR Build batch file
REM ************************

REM ARM IAR Build path

set arm_iar_build="c:\Programme\IAR Systems\Embedded Workbench ARM 5.50\common\bin\iarbuild"

REM **********************************************
REM Build MAC Example Application Star_Nobeacon
REM **********************************************

    cd ..\..\..\..\Applications\MAC_Examples\Star_Nobeacon\AT86RF230B_AT91SAM7X256_REB_2_3_REX_ARM_REV_2

    %arm_iar_build% Star.ewp -clean Star
    %arm_iar_build% Star.ewp -build Star

    cd ..\..\..\..\Build\Build_MAC\IAR\ARM\



    cd ..\..\..\..\Applications\MAC_Examples\Star_Nobeacon\AT86RF231_AT91SAM7X256_REB_4_0_2_REX_ARM_REV_3

    %arm_iar_build% Star.ewp -clean Star
    %arm_iar_build% Star.ewp -build Star

    cd ..\..\..\..\Build\Build_MAC\IAR\ARM\



    cd ..\..\..\..\Applications\MAC_Examples\Star_Nobeacon\AT86RF212_AT91SAM7X256_REB_5_0_REX_ARM_REV_3

    %arm_iar_build% Star.ewp -clean Star
    %arm_iar_build% Star.ewp -build Star

    cd ..\..\..\..\Build\Build_MAC\IAR\ARM\

