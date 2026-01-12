import pytest
import time
from helpers.connection import ble_connection_between_two_duts
from helpers.connection import scanner_finder

def test_dut_connection_client_disconnect(DUT1, DUT2):
    ret = ble_connection_between_two_duts(DUT1, DUT2)
    DUT2.write("bt disconnect")
    time.sleep(1)
    DUT1.write("bt advertise on")
    time.sleep(1)
    DUT1.expect("Advertising started")
    time.sleep(1)
    DUT2.write("bt connect-name 'test_shell'")
    time.sleep(1)
    DUT2.expect("LE conn param updated")
    DUT1.expect("LE conn param updated")

def test_dut_connection_server_disconnect(DUT1, DUT2):
    ret = ble_connection_between_two_duts(DUT1, DUT2)
    DUT1.write("bt disconnect")
    time.sleep(1)
    DUT1.write("bt advertise on")
    time.sleep(1)
    DUT1.expect("Advertising started")
    DUT2.write("bt connect-name 'test_shell'")
    time.sleep(1)
    DUT2.expect("LE conn param updated:")
    DUT1.expect("LE conn param updated:")


def test_dut_scan(DUT1, DUT2):
    ret = scanner_finder(DUT1, DUT2)

    DUT2.write("bt scan on")
    time.sleep(3)
    DUT2.write("bt scan off")
    DUT2.write("bt connect-name 'test_shell'")
    time.sleep(1)
    DUT2.expect("LE conn param updated:")
    DUT1.expect("LE conn param updated:")