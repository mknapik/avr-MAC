REM $Id: build_all_example_apps.bat 22877 2010-08-11 13:11:54Z sschneid $

REM **********************************************
REM Build all Example Applications
REM **********************************************


rm -f build_all_example_apps.log
rm -f build_all_example_apps.err


REM ***********************************************
REM MAC Examples
REM ***********************************************

CALL build_mac_example_1.bat >> build_all_example_apps.log 2>> build_all_example_apps.err
CALL build_mac_example_2_nobeacon_indirect_traffic.bat >> build_all_example_apps.log 2>> build_all_example_apps.err
CALL build_mac_example_3_beacon_payload.bat >> build_all_example_apps.log 2>> build_all_example_apps.err
CALL build_mac_example_4_beacon_bc_data.bat >> build_all_example_apps.log 2>> build_all_example_apps.err
CALL build_mac_example_basic_sensor_network.bat >> build_all_example_apps.log 2>> build_all_example_apps.err
CALL build_mac_example_promiscuous_mode_demo.bat >> build_all_example_apps.log 2>> build_all_example_apps.err
CALL build_mac_example_star_network.bat >> build_all_example_apps.log 2>> build_all_example_apps.err
CALL build_mac_example_star_network_system_clock_mhz.bat >> build_all_example_apps.log 2>> build_all_example_apps.err
CALL build_mac_example_star_high_rate.bat >> build_all_example_apps.log 2>> build_all_example_apps.err

REM ***********************************************
REM PAL Examples
REM ***********************************************

CALL build_pal_example_led_control.bat >> build_all_example_apps.log 2>> build_all_example_apps.err

REM ***********************************************
REM STB Examples
REM ***********************************************

CALL build_stb_example_secure_remote_control.bat >> build_all_example_apps.log 2>> build_all_example_apps.err
CALL build_stb_example_secure_sensor.bat >> build_all_example_apps.log 2>> build_all_example_apps.err

REM ***********************************************
REM TAL Examples
REM ***********************************************

CALL build_tal_example_performance_test.bat >> build_all_example_apps.log 2>> build_all_example_apps.err

REM ***********************************************
REM Tiny-TAL Examples
REM ***********************************************
CALL build_tiny_tal_example_wireless_uart.bat >> build_all_example_apps.log 2>> build_all_example_apps.err

