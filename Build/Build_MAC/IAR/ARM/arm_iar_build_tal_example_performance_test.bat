REM $Id: arm_iar_build_tal_example_performance_test.bat 22389 2010-07-01 10:56:11Z sschneid $

REM ************************
REM ARM IAR Build batch file
REM ************************

REM ARM IAR Build path

set arm_iar_build="c:\Programme\IAR Systems\Embedded Workbench ARM 5.50\common\bin\iarbuild"

REM **********************************************
REM Build TAL Example Application Performance_Test
REM **********************************************

    cd ..\..\..\..\Applications\TAL_Examples\Performance_Test\AT86RF230B_AT91SAM7X256_REB_2_3_REX_ARM_REV_2

    %arm_iar_build% Performance.ewp -clean Performance
    %arm_iar_build% Performance.ewp -build Performance

    cd ..\..\..\..\Build\Build_MAC\IAR\ARM\

