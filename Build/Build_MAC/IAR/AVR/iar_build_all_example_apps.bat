REM $Id: iar_build_all_example_apps.bat 22879 2010-08-11 13:14:42Z sschneid $

REM **********************************************
REM Build all Example Applications
REM **********************************************


rm -f iar_build_all_example_apps.log
rm -f iar_build_all_example_apps.err


REM ***********************************************
REM MAC Examples
REM ***********************************************

CALL iar_build_mac_example_1.bat >> iar_build_all_example_apps.log 2>> iar_build_all_example_apps.err
CALL iar_build_mac_example_2_nobeacon_indirect_traffic.bat >> iar_build_all_example_apps.log 2>> iar_build_all_example_apps.err
CALL iar_build_mac_example_3_beacon_payload.bat >> iar_build_all_example_apps.log 2>> iar_build_all_example_apps.err
CALL iar_build_mac_example_4_beacon_bc_data.bat >> iar_build_all_example_apps.log 2>> iar_build_all_example_apps.err
CALL iar_build_mac_example_basic_sensor_network.bat >> iar_build_all_example_apps.log 2>> iar_build_all_example_apps.err
CALL iar_build_mac_example_promiscuous_mode_demo.bat >> iar_build_all_example_apps.log 2>> iar_build_all_example_apps.err
CALL iar_build_mac_example_star_network.bat >> iar_build_all_example_apps.log 2>> iar_build_all_example_apps.err
CALL iar_build_mac_example_star_network_system_clock_mhz.bat >> iar_build_all_example_apps.log 2>> iar_build_all_example_apps.err
CALL iar_build_mac_example_star_high_rate.bat >> iar_build_all_example_apps.log 2>> iar_build_all_example_apps.err

REM ***********************************************
REM PAL Examples
REM ***********************************************

CALL iar_build_pal_example_led_control.bat >> iar_build_all_example_apps.log 2>> iar_build_all_example_apps.err

REM ***********************************************
REM STB Examples
REM ***********************************************

CALL iar_build_stb_example_secure_remote_control.bat >> iar_build_all_example_apps.log 2>> iar_build_all_example_apps.err
CALL iar_build_stb_example_secure_sensor.bat >> iar_build_all_example_apps.log 2>> iar_build_all_example_apps.err

REM ***********************************************
REM TAL Examples
REM ***********************************************
CALL iar_build_tal_example_performance_test.bat >> iar_build_all_example_apps.log 2>> iar_build_all_example_apps.err

REM ***********************************************
REM Tiny-TAL Examples
REM ***********************************************
CALL iar_build_tiny_tal_example_wireless_uart.bat >> iar_build_all_example_apps.log 2>> iar_build_all_example_apps.err
