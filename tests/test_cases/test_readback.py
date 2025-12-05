"""Test asyn:READBACK functionality.

This test module verifies that output records with info(asyn:READBACK, "1")
can correctly read values that the PLC sets independently. This tests the
true PLC→IOC reading capability of the asyn:READBACK mechanism, rather than
just verifying round-trip IOC→PLC→IOC communication.

The tests directly set PLC variable values and verify that output records
update correctly via the asyn:READBACK feature.
"""

import time
import pytest


class TestReadbackScalars:
    """Test readback functionality for scalar data types.

    These tests verify that output records with info(asyn:READBACK, "1")
    can read values set by the PLC, testing true PLC→IOC communication.
    """

    def test_bool_readback(self, set_plc_var, read_pv):
        """Verify BOOL output record reads PLC-set values via asyn:READBACK."""
        # Set True on PLC side and verify output record reads it
        set_plc_var('TestPlan.bo', True)
        time.sleep(0.1)
        assert read_pv('bool_out') == 1

        # Set False on PLC side and verify output record reads it
        set_plc_var('TestPlan.bo', False)
        time.sleep(0.1)
        assert read_pv('bool_out') == 0

    def test_sint_readback(self, set_plc_var, read_pv):
        """Verify SINT output record reads PLC-set values via asyn:READBACK."""
        test_values = [0, 42, -50, 99]
        for value in test_values:
            set_plc_var('TestPlan.sint_out', value)
            time.sleep(0.1)
            assert read_pv('sint_out') == value

    def test_int_readback(self, set_plc_var, read_pv):
        """Verify INT output record reads PLC-set values via asyn:READBACK."""
        test_values = [0, 1234, -5678, 9999]
        for value in test_values:
            set_plc_var('TestPlan.int_out', value)
            time.sleep(0.1)
            assert read_pv('int_out') == value

    def test_dint_readback(self, set_plc_var, read_pv):
        """Verify DINT output record reads PLC-set values via asyn:READBACK."""
        test_values = [0, 123456789, -987654321, 2000000000]
        for value in test_values:
            set_plc_var('TestPlan.dint_out', value)
            time.sleep(0.1)
            assert read_pv('dint_out') == value

    def test_uint_readback(self, set_plc_var, read_pv):
        """Verify UINT output record reads PLC-set values via asyn:READBACK."""
        test_values = [0, 12345, 65000]
        for value in test_values:
            set_plc_var('TestPlan.uint_out', value)
            time.sleep(0.1)
            assert read_pv('uint_out') == value

    def test_udint_readback(self, set_plc_var, read_pv):
        """Verify UDINT output record reads PLC-set values via asyn:READBACK."""
        test_values = [0, 3000000000, 4000000000]
        for value in test_values:
            set_plc_var('TestPlan.udint_out', value)
            time.sleep(0.1)
            assert read_pv('udint_out') == value

    def test_lint_readback(self, set_plc_var, read_pv):
        """Verify LINT output record reads PLC-set values via asyn:READBACK."""
        test_values = [0, 9000000000000000, -8000000000000000]
        for value in test_values:
            set_plc_var('TestPlan.lint_out', value)
            time.sleep(0.1)
            assert read_pv('lint_out') == value

    def test_real_readback(self, set_plc_var, read_pv):
        """Verify REAL output record reads PLC-set values via asyn:READBACK."""
        test_values = [0.0, 123.456, -987.654, 42.424242]
        for value in test_values:
            set_plc_var('TestPlan.real_out', value)
            time.sleep(0.1)
            assert read_pv('real_out') == pytest.approx(value, rel=1e-6)

    def test_lreal_readback(self, set_plc_var, read_pv):
        """Verify LREAL output record reads PLC-set values via asyn:READBACK."""
        test_values = [0.0, 123456.789012, -987654.321098, 3.141592653589793]
        for value in test_values:
            set_plc_var('TestPlan.lreal_out', value)
            time.sleep(0.1)
            assert read_pv('lreal_out') == pytest.approx(value, rel=1e-12)


class TestReadbackStrings:
    """Test readback functionality for string data type.

    These tests verify that output records with info(asyn:READBACK, "1")
    can read string values set by the PLC.
    """

    def test_string_readback(self, set_plc_var, read_pv):
        """Verify STRING output record reads PLC-set values via asyn:READBACK."""
        test_strings = [
            'Hello',
            'The quick brown fox jumps',
            '',
            'Test 123 with numbers',
        ]
        for test_string in test_strings:
            set_plc_var('TestPlan.stringout_n40', test_string)
            time.sleep(0.1)
            assert read_pv('string_out_n40') == test_string


class TestReadbackArrays:
    """Test readback functionality for array/waveform data types.

    These tests verify that output array records with info(asyn:READBACK, "1")
    can read array values set by the PLC.
    """

    def test_sint_arr_readback(self, set_plc_var, read_pv):
        """Verify SINT array output record reads PLC-set values via asyn:READBACK."""
        test_array = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
        set_plc_var('TestPlan.sint_out_array', test_array)
        time.sleep(0.1)
        result = read_pv('sint_arr_out')
        assert list(result) == test_array

    def test_int_arr_readback(self, set_plc_var, read_pv):
        """Verify INT array output record reads PLC-set values via asyn:READBACK."""
        test_array = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
        set_plc_var('TestPlan.int_out_array', test_array)
        time.sleep(0.1)
        result = read_pv('int_arr_out')
        assert list(result) == test_array

    def test_dint_arr_readback(self, set_plc_var, read_pv):
        """Verify DINT array output record reads PLC-set values via asyn:READBACK."""
        test_array = [100, 200, 300, 400, 500, 600, 700, 800, 900, 1000]
        set_plc_var('TestPlan.dint_out_array', test_array)
        time.sleep(0.1)
        result = read_pv('dint_arr_out')
        assert list(result) == test_array

    def test_uint_arr_readback(self, set_plc_var, read_pv):
        """Verify UINT array output record reads PLC-set values via asyn:READBACK."""
        test_array = [1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000]
        set_plc_var('TestPlan.uint_out_array', test_array)
        time.sleep(0.1)
        result = read_pv('uint_arr_out')
        assert list(result) == test_array

    def test_udint_arr_readback(self, set_plc_var, read_pv):
        """Verify UDINT array output record reads PLC-set values via asyn:READBACK."""
        test_array = [100000, 200000, 300000, 400000, 500000, 600000, 700000, 800000, 900000, 1000000]
        set_plc_var('TestPlan.udint_out_array', test_array)
        time.sleep(0.1)
        result = read_pv('udint_arr_out')
        assert list(result) == test_array

    def test_real_arr_readback(self, set_plc_var, read_pv):
        """Verify REAL array output record reads PLC-set values via asyn:READBACK."""
        test_array = [1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.0]
        set_plc_var('TestPlan.real_out_array', test_array)
        time.sleep(0.1)
        result = read_pv('real_arr_out')
        assert all(abs(a - b) < 1e-6 for a, b in zip(result, test_array))

    def test_lreal_arr_readback(self, set_plc_var, read_pv):
        """Verify LREAL array output record reads PLC-set values via asyn:READBACK."""
        test_array = [1.111111, 2.222222, 3.333333, 4.444444, 5.555555,
                      6.666666, 7.777777, 8.888888, 9.999999, 10.101010]
        set_plc_var('TestPlan.lreal_out_array', test_array)
        time.sleep(0.1)
        result = read_pv('lreal_arr_out')
        assert all(abs(a - b) < 1e-12 for a, b in zip(result, test_array))


class TestReadbackConsistency:
    """Test consistency between IOC writes and readbacks.

    These tests verify that when the IOC writes to an output record,
    both the output record (via asyn:READBACK) and the input record
    (via mock PLC linking) reflect the written value. This complements
    the main readback tests which verify PLC→IOC reading.
    """

    def test_dint_write_readback_input_consistency(self, write_pv, read_pv):
        """Verify output readback and input consistency after IOC write for DINT."""
        test_value = 123456789
        write_pv('dint_out', test_value)
        time.sleep(0.1)

        # All three should match: written value, output readback, input value
        output_readback = read_pv('dint_out')
        input_value = read_pv('dint_in')

        assert output_readback == test_value
        assert input_value == test_value
        assert output_readback == input_value

    def test_real_write_readback_input_consistency(self, write_pv, read_pv):
        """Verify output readback and input consistency after IOC write for REAL."""
        test_value = 123.456
        write_pv('real_out', test_value)
        time.sleep(0.1)

        # All three should match: written value, output readback, input value
        output_readback = read_pv('real_out')
        input_value = read_pv('real_in')

        assert output_readback == pytest.approx(test_value, rel=1e-6)
        assert input_value == pytest.approx(test_value, rel=1e-6)
        assert output_readback == pytest.approx(input_value, rel=1e-6)

    def test_string_write_readback_input_consistency(self, write_pv, read_pv):
        """Verify output readback and input consistency after IOC write for STRING."""
        test_value = 'Consistency test string'
        write_pv('string_out_n40', test_value)
        time.sleep(0.1)

        # All three should match: written value, output readback, input value
        output_readback = read_pv('string_out_n40')
        input_value = read_pv('string_in_n40')

        assert output_readback == test_value
        assert input_value == test_value
        assert output_readback == input_value
