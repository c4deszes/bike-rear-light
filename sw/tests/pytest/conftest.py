import pytest
import pyvisa
from pytest_bdd import scenarios, given, when, then, parsers
from bike_testframework.bdd.common import *
from bike_testframework.bdd.line_steps import generate_line_steps
from bike_testframework.bdd.psu_steps import generate_psu_steps
from bike_testframework.bdd.uds_steps import generate_uds_steps
from bike_testframework.devices.owon_spe31xx import OwonSPE31xx
from line_protocol.protocol.master import LineMaster
from line_protocol.protocol import LineSerialTransport
from line_protocol.network import Network, load_network
from line_uds.uds_tool import UdsTool

####################################################################################################

@pytest.fixture(scope="session")
def network():
    return load_network("../customer/network.json")

@pytest.fixture(scope="session")
def uds_profile():
    return "../customer/uds/rear_light.json"

# TODO: configurable port
@pytest.fixture
def line_master(network):
    with LineSerialTransport('COM20', network.baudrate, one_wire=True) as phy:
        with LineMaster(phy, network) as master:
            yield master

@pytest.fixture
def uds_tool(line_master, uds_profile):
    with UdsTool(line_master) as tool:
        tool.load_profile('RearLight', uds_profile)
        yield tool

@pytest.fixture
def resource_manager():
    return pyvisa.ResourceManager()

# TODO: configurable port
@pytest.fixture
def power_supply(resource_manager):
    with OwonSPE31xx(resource_manager, 'ASRL9::INSTR') as psu:
        yield psu

# TODO: configurable channel number
@pytest.fixture
def dut_psu_channel(power_supply):
    return power_supply.get_channel(0)

####################################################################################################

# TODO: configurable voltage and current
@pytest.fixture
def base_setup(line_master, dut_psu_channel):
    dut_psu_channel.set_voltage(12.0)
    dut_psu_channel.set_current(0.5)
    yield
    dut_psu_channel.disable()
    time.sleep(5)

@given(parsers.parse("on setup"))
def on_setup(base_setup, line_master, dut_psu_channel):
    dut_psu_channel.enable()
    yield
    line_master.disable_schedule()

@given(parsers.parse("off setup"))
def off_setup(base_setup, line_master, dut_psu_channel):
    dut_psu_channel.disable()
    yield
    line_master.disable_schedule()

@when(parsers.parse("I sweep the brightness from '{start:d}' to '{end:d}' with '{step:d}' increments and '{delay:d}ms' delay"))
def sweep_brightness(line_master, start, end, step, delay):
    for brightness in range(start, end, step):
        line_master.set_signal('LightSynchronization', 'TargetBrightness', brightness)
        time.sleep(delay / 1000)

@then(parsers.parse("the brightness should be '{brightness}'"))
def check_brightness(line_master, brightness):
    time.sleep(5)

####################################################################################################

generate_line_steps("line_master", "LineMaster")
generate_psu_steps("dut_psu_channel", "PSU")
generate_uds_steps("uds_tool", "UDS")
