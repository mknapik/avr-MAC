REM $Id: arm_iar_build_stb_example_secure_sensor.bat 22744 2010-08-04 09:37:13Z sschneid $

REM ************************
REM ARM IAR Build batch file
REM ************************

REM ARM IAR Build path

set arm_iar_build="c:\Programme\IAR Systems\Embedded Workbench ARM 5.50\common\bin\iarbuild"

REM **********************************************
REM Build STB Example Application Secure_Sensor
REM **********************************************

    cd ..\..\..\..\Applications\STB_Examples\Secure_Sensor\Data_Sink\AT86RF230B_AT91SAM7XC256_REB_2_3_REX_ARM_REV_2

    %arm_iar_build% Data_Sink.ewp -clean Data_Sink
    %arm_iar_build% Data_Sink.ewp -build Data_Sink

    cd ..\..\..\..\..\Build\Build_MAC\IAR\ARM\

    cd ..\..\..\..\Applications\STB_Examples\Secure_Sensor\Sensor\AT86RF230B_AT91SAM7XC256_REB_2_3_REX_ARM_REV_2

    %arm_iar_build% Sensor.ewp -clean Sensor
    %arm_iar_build% Sensor.ewp -build Sensor

    cd ..\..\..\..\..\Build\Build_MAC\IAR\ARM\



    cd ..\..\..\..\Applications\STB_Examples\Secure_Sensor\Data_Sink\AT86RF231_AT91SAM7X256_REB_4_0_2_REX_ARM_REV_3

    %arm_iar_build% Data_Sink.ewp -clean Data_Sink
    %arm_iar_build% Data_Sink.ewp -build Data_Sink

    cd ..\..\..\..\..\Build\Build_MAC\IAR\ARM\

    cd ..\..\..\..\Applications\STB_Examples\Secure_Sensor\Sensor\AT86RF231_AT91SAM7X256_REB_4_0_2_REX_ARM_REV_3

    %arm_iar_build% Sensor.ewp -clean Sensor
    %arm_iar_build% Sensor.ewp -build Sensor

    cd ..\..\..\..\..\Build\Build_MAC\IAR\ARM\


