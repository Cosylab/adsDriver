"""Test array/waveform data types.

This test module verifies reading and writing of array data types
supported by the ADS EPICS module. Arrays are mapped to EPICS waveform
records with NELM=10.
"""

import time
import pytest


class TestSINTArray:
    """Test SINT array (8-bit signed integer array)."""

    def test_sint_arr_write_read_zeros(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array of zeros."""
        test_array = [0] * 10
        write_pv('sint_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('sint_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.sint_out_array') == test_array

    def test_sint_arr_write_read_sequential(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array with sequential values."""
        test_array = list(range(10))  # [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
        write_pv('sint_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('sint_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.sint_out_array') == test_array

    def test_sint_arr_write_read_same_value(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array with same value."""
        test_array = [42] * 10
        write_pv('sint_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('sint_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.sint_out_array') == test_array

    def test_sint_arr_edge_values(self, write_pv, read_pv, get_plc_var):
        """Test array with edge values: min, max, zero, and boundary cases."""
        test_array = [-128, 127, 0, -1, 1, -127, 126, -128, 127, 0]
        write_pv('sint_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('sint_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.sint_out_array') == test_array


class TestINTArray:
    """Test INT array (16-bit signed integer array)."""

    def test_int_arr_write_read_zeros(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array of zeros."""
        test_array = [0] * 10
        write_pv('int_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('int_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.int_out_array') == test_array

    def test_int_arr_write_read_sequential(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array with sequential values."""
        test_array = list(range(10))  # [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
        write_pv('int_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('int_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.int_out_array') == test_array

    def test_int_arr_write_read_same_value(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array with same value."""
        test_array = [1234] * 10
        write_pv('int_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('int_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.int_out_array') == test_array

    def test_int_arr_edge_values(self, write_pv, read_pv, get_plc_var):
        """Test array with edge values: min, max, zero, and boundary cases."""
        test_array = [-32768, 32767, 0, -1, 1, -32767, 32766, -32768, 32767, 0]
        write_pv('int_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('int_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.int_out_array') == test_array


class TestDINTArray:
    """Test DINT array (32-bit signed integer array)."""

    def test_dint_arr_write_read_zeros(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array of zeros."""
        test_array = [0] * 10
        write_pv('dint_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('dint_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.dint_out_array') == test_array

    def test_dint_arr_write_read_sequential(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array with sequential values."""
        test_array = list(range(10))  # [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
        write_pv('dint_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('dint_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.dint_out_array') == test_array

    def test_dint_arr_write_read_same_value(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array with same value."""
        test_array = [123456] * 10
        write_pv('dint_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('dint_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.dint_out_array') == test_array

    def test_dint_arr_edge_values(self, write_pv, read_pv, get_plc_var):
        """Test array with edge values: min, max, zero, and boundary cases."""
        test_array = [-2147483648, 2147483647, 0, -1, 1, -2147483647, 2147483646, -2147483648, 2147483647, 0]
        write_pv('dint_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('dint_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.dint_out_array') == test_array


class TestUINTArray:
    """Test UINT array (16-bit unsigned integer array)."""

    def test_uint_arr_write_read_zeros(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array of zeros."""
        test_array = [0] * 10
        write_pv('uint_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('uint_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.uint_out_array') == test_array

    def test_uint_arr_write_read_sequential(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array with sequential values."""
        test_array = list(range(10))  # [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
        write_pv('uint_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('uint_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.uint_out_array') == test_array

    def test_uint_arr_write_read_same_value(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array with same value."""
        test_array = [5000] * 10
        write_pv('uint_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('uint_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.uint_out_array') == test_array

    def test_uint_arr_edge_values(self, write_pv, read_pv, get_plc_var):
        """Test array with edge values: max, zero, and boundary cases."""
        test_array = [65535, 0, 1, 2, 65534, 65533, 32768, 32767, 65535, 0]
        write_pv('uint_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('uint_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.uint_out_array') == test_array


class TestUDINTArray:
    """Test UDINT array (32-bit unsigned integer array)."""

    def test_udint_arr_write_read_zeros(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array of zeros."""
        test_array = [0] * 10
        write_pv('udint_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('udint_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.udint_out_array') == test_array

    def test_udint_arr_write_read_sequential(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array with sequential values."""
        test_array = list(range(10))  # [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
        write_pv('udint_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('udint_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.udint_out_array') == test_array

    def test_udint_arr_write_read_same_value(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array with same value."""
        test_array = [1000000] * 10
        write_pv('udint_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('udint_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.udint_out_array') == test_array

    def test_udint_arr_edge_values(self, write_pv, read_pv, get_plc_var):
        """Test array with edge values: max, zero, and boundary cases."""
        test_array = [4294967295, 0, 1, 2, 4294967294, 4294967293, 2147483648, 2147483647, 4294967295, 0]
        write_pv('udint_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('udint_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.udint_out_array') == test_array


class TestREALArray:
    """Test REAL array (32-bit float array)."""

    def test_real_arr_write_read_zeros(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array of zeros."""
        test_array = [0.0] * 10
        write_pv('real_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('real_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.real_out_array') == test_array

    def test_real_arr_write_read_sequential(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array with sequential values."""
        test_array = [float(i) for i in range(10)]  # [0.0, 1.0, 2.0, ..., 9.0]
        write_pv('real_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('real_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.real_out_array') == test_array

    def test_real_arr_write_read_same_value(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array with same value."""
        test_array = [3.125] * 10
        write_pv('real_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('real_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.real_out_array') == test_array

    def test_real_arr_edge_values(self, write_pv, read_pv, get_plc_var):
        """Test array with edge values: boundaries, special values (inf, nan), and normal values."""
        import math
        test_array = [3.4028234663852886e38, -3.4028234663852886e38, float('inf'), float('-inf'),
                      float('nan'), 0.0, 1.1754943508222875e-38, -1.1754943508222875e-38, 1.0, -1.0]
        write_pv('real_arr_out', test_array)
        time.sleep(0.1)
        ioc_result = list(read_pv('real_arr_in'))
        plc_result = get_plc_var('TestPlan.real_out_array')

        # Exact equality check for each element (IOC)
        assert ioc_result[0] == 3.4028234663852886e38
        assert ioc_result[1] == -3.4028234663852886e38
        assert math.isinf(ioc_result[2]) and ioc_result[2] > 0
        assert math.isinf(ioc_result[3]) and ioc_result[3] < 0
        assert math.isnan(ioc_result[4])
        assert ioc_result[5] == 0.0
        assert ioc_result[6] == 1.1754943508222875e-38
        assert ioc_result[7] == -1.1754943508222875e-38
        assert ioc_result[8] == 1.0
        assert ioc_result[9] == -1.0

        # PLC verification
        assert plc_result[0] == 3.4028234663852886e38
        assert plc_result[1] == -3.4028234663852886e38
        assert math.isinf(plc_result[2]) and plc_result[2] > 0
        assert math.isinf(plc_result[3]) and plc_result[3] < 0
        assert math.isnan(plc_result[4])
        assert plc_result[5] == 0.0
        assert plc_result[6] == 1.1754943508222875e-38
        assert plc_result[7] == -1.1754943508222875e-38
        assert plc_result[8] == 1.0
        assert plc_result[9] == -1.0


class TestLREALArray:
    """Test LREAL array (64-bit float array)."""

    def test_lreal_arr_write_read_zeros(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array of zeros."""
        test_array = [0.0] * 10
        write_pv('lreal_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('lreal_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.lreal_out_array') == test_array

    def test_lreal_arr_write_read_sequential(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array with sequential values."""
        test_array = [float(i) for i in range(10)]  # [0.0, 1.0, 2.0, ..., 9.0]
        write_pv('lreal_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('lreal_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.lreal_out_array') == test_array

    def test_lreal_arr_write_read_same_value(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array with same value."""
        test_array = [3.141592653589793] * 10
        write_pv('lreal_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('lreal_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.lreal_out_array') == test_array

    def test_lreal_arr_edge_values(self, write_pv, read_pv, get_plc_var):
        """Test array with edge values: boundaries, special values (inf, nan), and normal values."""
        import math
        test_array = [1.797693e308, -1.797693e308, float('inf'), float('-inf'),
                      float('nan'), 0.0, 2.225074e-308, -2.225074e-308, 1.0, -1.0]
        write_pv('lreal_arr_out', test_array)
        time.sleep(0.1)
        ioc_result = list(read_pv('lreal_arr_in'))
        plc_result = get_plc_var('TestPlan.lreal_out_array')

        # Exact equality check for each element (IOC)
        assert ioc_result[0] == 1.797693e308
        assert ioc_result[1] == -1.797693e308
        assert math.isinf(ioc_result[2]) and ioc_result[2] > 0
        assert math.isinf(ioc_result[3]) and ioc_result[3] < 0
        assert math.isnan(ioc_result[4])
        assert ioc_result[5] == 0.0
        assert ioc_result[6] == 2.225074e-308
        assert ioc_result[7] == -2.225074e-308
        assert ioc_result[8] == 1.0
        assert ioc_result[9] == -1.0

        # PLC verification
        assert plc_result[0] == 1.797693e308
        assert plc_result[1] == -1.797693e308
        assert math.isinf(plc_result[2]) and plc_result[2] > 0
        assert math.isinf(plc_result[3]) and plc_result[3] < 0
        assert math.isnan(plc_result[4])
        assert plc_result[5] == 0.0
        assert plc_result[6] == 2.225074e-308
        assert plc_result[7] == -2.225074e-308
        assert plc_result[8] == 1.0
        assert plc_result[9] == -1.0


class TestBYTEArray:
    """Test BYTE array (8-bit unsigned integer array)."""

    def test_byte_arr_write_read_zeros(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array of zeros."""
        test_array = [0] * 10
        write_pv('byte_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('byte_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.byte_out_array') == test_array

    def test_byte_arr_write_read_sequential(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array with sequential values."""
        test_array = list(range(10))  # [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
        write_pv('byte_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('byte_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.byte_out_array') == test_array

    def test_byte_arr_write_read_same_value(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading array with same value."""
        test_array = [42] * 10
        write_pv('byte_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('byte_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.byte_out_array') == test_array

    def test_byte_arr_edge_values(self, write_pv, read_pv, get_plc_var):
        """Test array with edge values: max, zero, and boundary cases."""
        test_array = [255, 0, 1, 2, 254, 253, 128, 127, 255, 0]
        write_pv('byte_arr_out', test_array)
        time.sleep(0.1)
        result = read_pv('byte_arr_in')
        assert list(result) == test_array
        assert get_plc_var('TestPlan.byte_out_array') == test_array
