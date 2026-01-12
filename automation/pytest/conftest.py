from common_utils import TestDut
from pytest import fixture

tDut = None


@fixture(scope="function")
def TestDutFixture(dut, request):
    """A fixture to capture Alif-specific DUT properties"""
    global tDut
    if tDut is None:
        tDut = TestDut(dut)
    tDut.setDut(dut)
    return tDut

@fixture(scope="function")
def DUT1(TestDutFixture):
    """A fixture to communicate with DUT1"""
    return TestDutFixture.get_duts()[0]

@fixture(scope="function")
def DUT2(TestDutFixture):
    """A fixture to communicate with DUT2"""
    return TestDutFixture.get_duts()[1]
