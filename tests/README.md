# ADS EPICS Module Test Suite

Automated integration tests for the ADS EPICS module using a mock ADS server.

## Prerequisites

- Python 3.8 or later
- Built ADS module (run `make` from module root)
- EPICS Base environment configured

## Setup

1. Create a Python virtual environment:

   ```bash
   cd tests
   python3 -m venv venv
   ```

2. Activate the virtual environment:

   ```bash
   source venv/bin/activate
   ```

3. Install dependencies:

   ```bash
   pip install -r requirements.txt
   ```

## Running Tests

With the virtual environment activated:

```bash
# Run all tests
pytest test_cases/ -v

# Run specific test file
pytest test_cases/test_datatype_integers.py -v

# Run with verbose output (shows print statements)
pytest test_cases/ -v -s

# Run with detailed tracebacks on failure
pytest test_cases/ -v --tb=long

# Run a specific test class
pytest test_cases/test_datatype_integers.py::TestDINTType -v

# Run a specific test
pytest test_cases/test_datatype_integers.py::TestDINTType::test_dint_write_read_zero -v
```

## Test Architecture

The test suite consists of:

1. **Mock ADS Server** (`handler/ads_mock_handler.py`)
   - Simulates a Beckhoff PLC using `pyads.testserver`
   - Stores written values and returns them on read
   - Pre-registers all test variables at startup
   - The pyads server has been augmented to support sum reads and name resolution

2. **Test IOC** (`iocBoot/iocadsTest/`)
   - EPICS IOC with test database records
   - Connects to mock server at localhost:48898
   - PV prefix is `TEST:`

3. **pytest Fixtures** (`conftest.py`)
   - Manages server and IOC lifecycle
   - Provides p4p PVAccess client

4. **Test Cases** (`test_cases/`)
   - Data type conversion tests
   - Read/write operation tests
   - Error recovery tests

## Troubleshooting

### IOC fails to start
- Ensure the module is built: `make` from module root
- Check that `bin/linux-x86_64/adsTest` exists
- Check that `iocBoot/iocadsTest/envPaths` exists

### Cannot connect to PVs
- Verify IOC started successfully (check pytest output for "iocInit" message)
- Ensure no other IOC is using the same PV prefix

### Mock server connection refused
- Check that port 48898 is available
- Ensure no other process is using the port
