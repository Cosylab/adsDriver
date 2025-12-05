"""Pytest configuration and fixtures for ADS EPICS module tests."""

import struct
import time
import pytest
import p4p.client.thread
from pyads.testserver import AdsTestServer
from handler import ADSTestHandler
from utils.ioc_manager import IOCManager
from utils.pv_helpers import get_pv_with_prefix


# Configuration constants
TEST_PREFIX = "TEST"
MOCK_ADS_IP = "127.0.0.1"
MOCK_ADS_PORT = 48898

# These paths are relative to the tests directory
IOC_EXECUTABLE = "../bin/linux-x86_64/adsTest"
ST_CMD = "../iocBoot/iocadsTest/st.cmd"


@pytest.fixture(scope="session")
def ads_handler():
    """Create and return an ADSTestHandler instance.

    This fixture provides a handler for managing mock PLC variables
    during testing. The handler persists for the entire test session.

    Returns:
        ADSTestHandler instance
    """
    handler = ADSTestHandler()
    yield handler
    handler.clear()


@pytest.fixture(scope="session")
def set_plc_var(ads_handler):
    """Helper fixture to set PLC variable values directly.

    This fixture provides a convenient function for setting PLC variable
    values directly in the mock handler, simulating PLC-side changes.

    Args:
        ads_handler: The ADSTestHandler fixture

    Returns:
        Function that takes variable name and value, and sets it in PLC

    Example:
        def test_read_from_plc(set_plc_var, read_pv):
            set_plc_var('TestPlan.dint_out', 12345)
            assert read_pv('dint_out') == 12345
    """
    # Mapping of data types to struct format codes
    TYPE_FORMATS = {
        'BOOL': '<?',      # 1 byte boolean
        'SINT': '<b',      # 1 byte signed
        'INT': '<h',       # 2 byte signed
        'DINT': '<i',      # 4 byte signed
        'LINT': '<q',      # 8 byte signed
        'UINT': '<H',      # 2 byte unsigned
        'UDINT': '<I',     # 4 byte unsigned
        'REAL': '<f',      # 4 byte float
        'LREAL': '<d',     # 8 byte double
    }

    def _set_plc_var(var_name: str, value):
        """Set a PLC variable value directly.

        Args:
            var_name: Full PLC variable name (e.g., 'TestPlan.dint_out')
            value: Value to set (int, float, bool, string, or list for arrays)
        """
        var = ads_handler.get_variable_by_name(var_name)
        if var is None:
            raise ValueError(f"Variable {var_name} not found in handler")

        # Detect data type from variable's symbol_type
        symbol_type = var.symbol_type

        # Handle strings specially
        if symbol_type.startswith('STRING'):
            # Extract N from STRING(N) if present
            if '(' in symbol_type:
                n = int(symbol_type.split('(')[1].split(')')[0])
                buffer_size = n + 1  # N chars + null terminator
            else:
                buffer_size = 81  # Default STRING is STRING(80)

            # Convert string value to bytes with null termination
            if isinstance(value, str):
                value_bytes = value.encode('utf-8')
            else:
                value_bytes = b''

            # Ensure null termination and pad to buffer size
            var.value = (value_bytes + b'\x00').ljust(buffer_size, b'\x00')
            return

        # Handle arrays
        if symbol_type.startswith('ARRAY'):
            # Extract element type from "ARRAY [0..N] OF TYPE"
            elem_type = symbol_type.split(' OF ')[-1]

            if elem_type not in TYPE_FORMATS:
                raise ValueError(f"Unsupported array element type: {elem_type}")

            fmt = TYPE_FORMATS[elem_type]

            # Pack array elements
            if not isinstance(value, (list, tuple)):
                raise ValueError(f"Array value must be list or tuple, got {type(value)}")

            array_size = len(value)
            packed = struct.pack(f'<{array_size}{fmt[1]}', *value)

            var.value = packed
            return

        # Handle scalar types
        if symbol_type not in TYPE_FORMATS:
            raise ValueError(f"Unsupported data type: {symbol_type}")

        fmt = TYPE_FORMATS[symbol_type]
        var.value = struct.pack(fmt, value)

    return _set_plc_var


@pytest.fixture(scope="session")
def get_plc_var(ads_handler):
    """Helper fixture to get PLC variable values directly.

    This fixture provides a convenient function for reading PLC variable
    values directly from the mock handler, bypassing the IOC. This allows
    tests to verify that values written through EPICS are correctly
    propagated to the PLC.

    Args:
        ads_handler: The ADSTestHandler fixture

    Returns:
        Function that takes variable name and returns its value

    Example:
        def test_write_to_plc(write_pv, read_pv, get_plc_var):
            write_pv('dint_out', 12345)
            time.sleep(0.1)
            assert read_pv('dint_in') == 12345
            assert get_plc_var('TestPlan.dint_out') == 12345
    """
    # Mapping of data types to struct format codes
    TYPE_FORMATS = {
        'BOOL': '<?',      # 1 byte boolean
        'SINT': '<b',      # 1 byte signed
        'INT': '<h',       # 2 byte signed
        'DINT': '<i',      # 4 byte signed
        'LINT': '<q',      # 8 byte signed
        'UINT': '<H',      # 2 byte unsigned
        'UDINT': '<I',     # 4 byte unsigned
        'BYTE': '<B',      # 1 byte unsigned
        'USINT': '<B',     # 1 byte unsigned
        'WORD': '<H',      # 2 byte unsigned
        'DWORD': '<I',     # 4 byte unsigned
        'REAL': '<f',      # 4 byte float
        'LREAL': '<d',     # 8 byte double
    }

    def _get_plc_var(var_name: str):
        """Get a PLC variable value directly.

        Args:
            var_name: Full PLC variable name (e.g., 'TestPlan.dint_out')

        Returns:
            Variable value (int, float, bool, string, or list for arrays)
        """
        var = ads_handler.get_variable_by_name(var_name)
        if var is None:
            raise ValueError(f"Variable {var_name} not found in handler")

        # Detect data type from variable's symbol_type
        symbol_type = var.symbol_type

        # Handle strings specially
        if symbol_type.startswith('STRING'):
            # Extract N from STRING(N) if present
            if '(' in symbol_type:
                n = int(symbol_type.split('(')[1].split(')')[0])
                buffer_size = n + 1  # N chars + null terminator
            else:
                buffer_size = 81  # Default STRING is STRING(80)

            # Extract bytes up to first null terminator
            raw_bytes = var.value[:buffer_size]
            null_pos = raw_bytes.find(b'\x00')
            if null_pos >= 0:
                raw_bytes = raw_bytes[:null_pos]

            return raw_bytes.decode('utf-8')

        # Handle arrays
        if symbol_type.startswith('ARRAY'):
            # Extract element type from "ARRAY [0..N] OF TYPE"
            elem_type = symbol_type.split(' OF ')[-1]

            if elem_type not in TYPE_FORMATS:
                raise ValueError(f"Unsupported array element type: {elem_type}")

            fmt = TYPE_FORMATS[elem_type]
            elem_size = struct.calcsize(fmt)

            # Extract array bounds from "ARRAY [0..N]"
            bounds = symbol_type.split('[')[1].split(']')[0]
            if '..' in bounds:
                start, end = bounds.split('..')
                array_size = int(end) - int(start) + 1
            else:
                raise ValueError(f"Cannot parse array bounds: {bounds}")

            # Unpack array elements
            array_bytes = var.value[:elem_size * array_size]
            unpacked = struct.unpack(f'<{array_size}{fmt[1]}', array_bytes)
            return list(unpacked)

        # Handle scalar types
        if symbol_type in TYPE_FORMATS:
            fmt = TYPE_FORMATS[symbol_type]
            value_size = struct.calcsize(fmt)
            return struct.unpack(fmt, var.value[:value_size])[0]

        raise ValueError(f"Unsupported data type: {symbol_type}")

    return _get_plc_var


@pytest.fixture(scope="module")
def ads_server(ads_handler):
    """Start an AdsTestServer with the handler that can be restarted.

    This fixture provides a restartable wrapper around AdsTestServer
    that handles the Python thread limitation (threads can only start once)
    by creating new server instances on restart.

    Args:
        ads_handler: The ADSTestHandler fixture

    Yields:
        _RestartableServer instance with start(), stop(), restart(), is_alive() methods
    """
    class _RestartableServer:
        """Private wrapper class for managing ADS test server restarts."""

        def __init__(self, *args, **kwargs):
            self.args = args
            self.kwargs = kwargs
            self._current_server = None
            self.start()

        def start(self):
            """Start the server (or create new instance if restarting)."""
            if self._current_server and self._current_server.is_alive():
                return  # Already running
            self._current_server = AdsTestServer(*self.args, **self.kwargs)
            self._current_server.start()

        def stop(self):
            """Stop the server."""
            if self._current_server:
                self._current_server.stop()
            time.sleep(0.5)  # Brief pause to ensure port is released

        def restart(self):
            """Restart the server by stopping and creating a new instance."""
            self.stop()
            self.start()

        def is_alive(self):
            """Check if server thread is alive."""
            return self._current_server and self._current_server.is_alive()

    server = _RestartableServer(ads_handler, ip_address=MOCK_ADS_IP, port=MOCK_ADS_PORT, logging=False)
    yield server
    server.stop()


@pytest.fixture(scope="session")
def ioc_manager():
    """Create an IOCManager instance.

    This fixture provides a manager for the IOC subprocess. It persists
    for the entire test session but doesn't automatically start the IOC.
    Use the running_ioc fixture to actually start the IOC.

    Yields:
        IOCManager instance
    """
    manager = IOCManager(IOC_EXECUTABLE, ST_CMD)
    yield manager
    if manager.is_running():
        manager.stop()


@pytest.fixture(scope="module")
def running_ioc(ioc_manager, ads_server):
    """Start the IOC and wait for it to be ready.

    This fixture starts the IOC subprocess and waits for initialization
    to complete. It is module-scoped, so the IOC is restarted for each
    test module. The IOC is automatically stopped after the module's
    tests complete.

    Args:
        ioc_manager: The IOCManager fixture
        ads_server: The AdsTestServer fixture (ensures server is running)

    Yields:
        IOCManager instance with running IOC

    Raises:
        RuntimeError: If IOC fails to start
    """
    # Start the IOC
    success = ioc_manager.start(timeout=30.0)
    if not success:
        raise RuntimeError("Failed to start IOC")

    yield ioc_manager

    # Stop the IOC after tests
    ioc_manager.stop()


@pytest.fixture(scope="module")
def pva(running_ioc):
    """Create a p4p PVA context.

    This fixture provides a PVAccess context for reading and writing
    PVs during testing. It is module-scoped and depends on the IOC
    being running.

    Args:
        running_ioc: The running_ioc fixture (ensures IOC is started)

    Yields:
        p4p.client.thread.Context instance
    """
    context = p4p.client.thread.Context('pva')
    yield context
    context.close()


@pytest.fixture
def read_pv(pva):
    """Helper fixture that returns a function to read a PV.

    This fixture provides a convenient function for reading PVs with
    the TEST prefix automatically prepended.

    Args:
        pva: The PVA context fixture

    Returns:
        Function that takes a PV name and returns its value

    Example:
        def test_temperature(read_pv):
            temp = read_pv("temperature")
            assert temp > 0
    """
    def _read_pv(pv_name: str, timeout: float = 5.0):
        """Read a PV value.

        Args:
            pv_name: PV name (without prefix)
            timeout: Timeout in seconds

        Returns:
            PV value
        """
        full_name = get_pv_with_prefix(pv_name, TEST_PREFIX)
        return pva.get(full_name, timeout=timeout)

    return _read_pv


@pytest.fixture
def write_pv(pva):
    """Helper fixture that returns a function to write a PV.

    This fixture provides a convenient function for writing PVs with
    the TEST prefix automatically prepended.

    Args:
        pva: The PVA context fixture

    Returns:
        Function that takes a PV name and value and writes it

    Example:
        def test_setpoint(write_pv, read_pv):
            write_pv("setpoint", 25.0)
            assert read_pv("setpoint") == 25.0
    """
    def _write_pv(pv_name: str, value, timeout: float = 5.0):
        """Write a PV value.

        Args:
            pv_name: PV name (without prefix)
            value: Value to write
            timeout: Timeout in seconds
        """
        full_name = get_pv_with_prefix(pv_name, TEST_PREFIX)
        pva.put(full_name, value, timeout=timeout)

    return _write_pv


@pytest.fixture
def is_connected(pva):
    """Helper fixture that returns a function to check IOC connection status.

    This fixture provides a convenient function for checking if the IOC
    reports it is connected to the ADS server by reading the CNCT field
    of the asynRecord.

    Args:
        pva: The PVA context fixture

    Returns:
        Function that returns True if connected, False otherwise

    Example:
        def test_communication(is_connected, write_pv, read_pv):
            assert is_connected(), "IOC not connected"
            write_pv("dint_out", 42)
            assert read_pv("dint_in") == 42
    """
    def _is_connected(timeout: float = 5.0) -> bool:
        """Check if IOC reports connected to ADS server.

        Reads the CNCT field from the asynRecord monitoring the ADS port.

        Args:
            timeout: Timeout in seconds

        Returns:
            True if connected (CNCT=1), False if disconnected (CNCT=0)
        """
        try:
            cnct = pva.get(f"{TEST_PREFIX}:asyn.CNCT", timeout=timeout)
            return cnct == 1  # 1 = "Connect", 0 = "Disconnect"
        except Exception:
            return False

    return _is_connected


@pytest.fixture(scope="module", autouse=True)
def ensure_connected(running_ioc, pva, set_plc_var):
    """Ensure IOC is connected to ADS server before running tests.

    This fixture automatically runs before each test module to verify
    the IOC has successfully connected to the ADS server. If not connected,
    the test module is failed with a clear error message.

    This is a prerequisite for all tests in the suite.

    Args:
        running_ioc: The running_ioc fixture (ensures IOC is started)
        pva: The PVA context fixture

    Raises:
        RuntimeError: If IOC is not connected to ADS server
    """

    set_plc_var('TestPlan.sint_in', 119)  # Magic value used for connection test
    try_count = 0
    for _ in range(10):
        cnct = pva.get(f"{TEST_PREFIX}:asyn.CNCT", timeout=0.1)
        magic = pva.get(f"{TEST_PREFIX}:sint_in", timeout=0.1)
        if cnct == 1 and magic == 119:
            break
        time.sleep(0.1)
    else:
        raise RuntimeError(
            "IOC is not connected to ADS server after 1 second. "
            "Check that the ADS test server can start."
        )
