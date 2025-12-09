import pytest
import time
from helpers.connection import ble_connection_between_two_duts
from helpers.connection import scanner_finder

def test_dut_connection_Client_disconnect(DUT1, DUT2):
    ret= ble_connection_between_two_duts(DUT1, DUT2)
    DUT2.write ("bt disconnect")
    time.sleep(1)
    DUT1.write("bt adv-start")
    time.sleep(1)

    time.sleep(1)
    DUT2.write("bt connect-name 'zas15 ble shell'")
    time.sleep(1)
    DUT2.expect("LE conn param updated")
    DUT1.expect("LE conn param updated")

def test_dut_connection_Server_disconnect(DUT1, DUT2):
    ret= ble_connection_between_two_duts(DUT1, DUT2)
    DUT1.write ("bt disconnect")
    time.sleep(1)
    DUT1.write("bt adv-start")
    time.sleep(1)
    DUT2.write("bt connect-name 'zas15 ble shell'")
    time.sleep(1)
    DUT2.expect("LE conn param updated:")
    DUT1.expect("LE conn param updated:")

def test_dut_scan(DUT1, DUT2):
    ret= scanner_finder(DUT1, DUT2)

    DUT2.write("bt scan on")
    time.sleep(3)
    DUT2.write("bt scan off")
    DUT2.write("bt connect-name 'zas15 ble shell'")
    time.sleep(1)
    DUT2.expect("LE conn param updated:")
    DUT1.expect("LE conn param updated:")