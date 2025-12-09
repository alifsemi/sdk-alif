import pytest
import time


def ble_connection_between_two_duts(dut1,dut2):
    dut1.write("kernel reboot cold")
    time.sleep(1)
    dut1.write("bt init")
    dut1.expect("bt_hci_core")
    dut1.write("bt name 'zas15 ble shell'")
    dut1.write("bt adv-create conn-scan")
    dut1.expect("Created adv id")
    dut1.write("bt adv-param conn-scan name")
    dut1.write("bt adv-start")
    dut1.expect("Advertiser")
    dut2.write("kernel reboot cold")
    time.sleep(1)
    dut2.write("bt init")
    dut2.expect("bt_hci_core")
    dut2.write("bt connect-name 'zas15 ble shell'")
    time.sleep(1)
    dut2.expect("LE conn param updated")
    dut1.expect("LE conn param updated")

def scanner_finder(dut1,dut2):
    dut1.write("kernel reboot cold")
    time.sleep(1)
    dut1.write("bt init")
    dut1.expect("bt_hci_core")
    dut1.write("bt name 'zas15 ble shell'")
    dut1.write("bt adv-create conn-scan")
    dut1.expect("Created adv id")
    dut1.write("bt adv-param conn-scan name")
    dut1.write("bt adv-start")
    dut2.write("kernel reboot cold")
    time.sleep(1)
    dut2.write("bt init")
    dut2.expect("bt_hci_core")

