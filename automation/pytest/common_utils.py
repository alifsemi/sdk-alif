import pytest

class TestDut:
    def __init__(self, dut):
        self.dut = dut

    def setDut(self, dut):
        self.dut = dut

    def get_duts(self):
        # NOTE! Never ever change the order or tests will start to fail as they rely on the order.
        return self.dut[0], self.dut[1]
