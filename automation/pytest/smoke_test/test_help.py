import pytest

def test_help_command(DUT1, DUT2):
    DUT1.write("help")
    DUT1.expect("Please press the <Tab> button to see all available commands.")
    DUT2.write("help")
    DUT2.expect("Please press the <Tab> button to see all available commands.")