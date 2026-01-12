import pytest
import time
from helpers.connection import ble_connection_between_two_duts

def test_L2CAP_test(DUT1, DUT2):
    ret = ble_connection_between_two_duts(DUT1, DUT2)

    DUT1.write("l2cap register 29")
    DUT1.expect("L2CAP psm 41 sec_level 1 registered")
    time.sleep(1)
    DUT2.write("l2cap connect 29")
    DUT2.expect("Channel 0x2000a248 connected")
    DUT2.write("l2cap send 3 14")
    DUT2.expect("Outgoing data channel ")
    DUT1.expect("Incoming data channel ")