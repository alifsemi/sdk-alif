import pytest
import time


def ble_connection_between_two_duts(dut1,dut2):
    dut1.write("kernel reboot cold")
    time.sleep(1)
    dut1.write("bt init")
    dut1.expect("bt_hci_core")
    dut1.write("bt advertise on")
    dut1.expect("Advertising started")
    dut2.write("kernel reboot cold")
    time.sleep(1)
    dut2.write("bt init")
    dut2.expect("bt_hci_core")
    dut2.write("bt connect-name 'test_shell'")
    time.sleep(1)
    dut2.expect("LE conn param updated")
    dut1.expect("LE conn param updated")

def scanner_finder(dut1,dut2):
    dut1.write("kernel reboot cold")
    time.sleep(1)
    dut1.write("bt init")
    dut1.expect("bt_hci_core:")
    dut1.write("bt advertise on")
    dut1.expect("Advertising started")
    dut1.write("bt adv-param conn-scan name")
    dut1.write("bt adv-start")
    dut1.expect("Advertiser")
    dut1.write("bt adv-info")
    dut2.write("kernel reboot cold")
    time.sleep(1)
    dut2.write("bt init")
    dut2.expect("bt_hci_core")
