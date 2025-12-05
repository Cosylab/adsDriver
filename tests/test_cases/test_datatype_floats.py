"""Test floating-point data types (REAL, LREAL).

This test module verifies reading and writing of floating-point data types
supported by the ADS EPICS module.
"""

import time
import pytest


class TestREALType:
    """Test REAL (32-bit floating-point) data type."""

    def test_real_write_read_zero(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading zero."""
        write_pv('real_out', 0.0)
        time.sleep(0.1)
        assert read_pv('real_in') == 0.0
        assert get_plc_var('TestPlan.real_out') == 0.0

    def test_real_write_read_positive(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading positive value."""
        write_pv('real_out', 123.5)
        time.sleep(0.1)
        assert read_pv('real_in') == 123.5
        assert get_plc_var('TestPlan.real_out') == 123.5

    def test_real_write_read_negative(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading negative value."""
        write_pv('real_out', -456.25)
        time.sleep(0.1)
        assert read_pv('real_in') == -456.25
        assert get_plc_var('TestPlan.real_out') == -456.25

    def test_real_write_read_small(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading small value."""
        write_pv('real_out', 0.0009765625)
        time.sleep(0.1)
        assert read_pv('real_in') == 0.0009765625
        assert get_plc_var('TestPlan.real_out') == 0.0009765625

    def test_real_write_read_large(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading large value."""
        write_pv('real_out', 65536.0)
        time.sleep(0.1)
        assert read_pv('real_in') == 65536.0
        assert get_plc_var('TestPlan.real_out') == 65536.0

    def test_real_boundary_max(self, write_pv, read_pv, get_plc_var):
        """Test maximum representable value."""
        write_pv('real_out', 3.4028234663852886e38)
        time.sleep(0.1)
        assert read_pv('real_in') == 3.4028234663852886e38
        assert get_plc_var('TestPlan.real_out') == 3.4028234663852886e38

    def test_real_boundary_min_positive(self, write_pv, read_pv, get_plc_var):
        """Test minimum positive representable value."""
        write_pv('real_out', 1.1754943508222875e-38)
        time.sleep(0.1)
        assert read_pv('real_in') == 1.1754943508222875e-38
        assert get_plc_var('TestPlan.real_out') == 1.1754943508222875e-38

    def test_real_boundary_max_negative(self, write_pv, read_pv, get_plc_var):
        """Test maximum negative representable value."""
        write_pv('real_out', -3.4028234663852886e38)
        time.sleep(0.1)
        assert read_pv('real_in') == -3.4028234663852886e38
        assert get_plc_var('TestPlan.real_out') == -3.4028234663852886e38

    def test_real_special_infinity_positive(self, write_pv, read_pv, get_plc_var):
        """Test positive infinity special value."""
        import math
        write_pv('real_out', float('inf'))
        time.sleep(0.1)
        result = read_pv('real_in')
        assert math.isinf(result) and result > 0, f"Expected +inf, got {result}"
        plc_result = get_plc_var('TestPlan.real_out')
        assert math.isinf(plc_result) and plc_result > 0, f"PLC: Expected +inf, got {plc_result}"

    def test_real_special_infinity_negative(self, write_pv, read_pv, get_plc_var):
        """Test negative infinity special value."""
        import math
        write_pv('real_out', float('-inf'))
        time.sleep(0.1)
        result = read_pv('real_in')
        assert math.isinf(result) and result < 0, f"Expected -inf, got {result}"
        plc_result = get_plc_var('TestPlan.real_out')
        assert math.isinf(plc_result) and plc_result < 0, f"PLC: Expected -inf, got {plc_result}"

    def test_real_special_nan(self, write_pv, read_pv, get_plc_var):
        """Test NaN (not-a-number) special value."""
        import math
        write_pv('real_out', float('nan'))
        time.sleep(0.1)
        result = read_pv('real_in')
        assert math.isnan(result), f"Expected NaN, got {result}"
        plc_result = get_plc_var('TestPlan.real_out')
        assert math.isnan(plc_result), f"PLC: Expected NaN, got {plc_result}"


class TestLREALType:
    """Test LREAL (64-bit floating-point) data type."""

    def test_lreal_write_read_zero(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading zero."""
        write_pv('lreal_out', 0.0)
        time.sleep(0.1)
        assert read_pv('lreal_in') == 0.0
        assert get_plc_var('TestPlan.lreal_out') == 0.0

    def test_lreal_write_read_positive(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading positive value."""
        write_pv('lreal_out', 123456.789012)
        time.sleep(0.1)
        assert read_pv('lreal_in') == 123456.789012
        assert get_plc_var('TestPlan.lreal_out') == 123456.789012

    def test_lreal_write_read_negative(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading negative value."""
        write_pv('lreal_out', -987654.321098)
        time.sleep(0.1)
        assert read_pv('lreal_in') == -987654.321098
        assert get_plc_var('TestPlan.lreal_out') == -987654.321098

    def test_lreal_write_read_small(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading small value."""
        write_pv('lreal_out', 0.000000123456)
        time.sleep(0.1)
        assert read_pv('lreal_in') == 0.000000123456
        assert get_plc_var('TestPlan.lreal_out') == 0.000000123456

    def test_lreal_write_read_large(self, write_pv, read_pv, get_plc_var):
        """Test writing and reading large value."""
        write_pv('lreal_out', 123456789.987654321)
        time.sleep(0.1)
        assert read_pv('lreal_in') == 123456789.987654321
        assert get_plc_var('TestPlan.lreal_out') == 123456789.987654321

    def test_lreal_boundary_max(self, write_pv, read_pv, get_plc_var):
        """Test maximum representable value."""
        write_pv('lreal_out', 1.797693e308)
        time.sleep(0.1)
        assert read_pv('lreal_in') == 1.797693e308
        assert get_plc_var('TestPlan.lreal_out') == 1.797693e308

    def test_lreal_boundary_min_positive(self, write_pv, read_pv, get_plc_var):
        """Test minimum positive representable value."""
        write_pv('lreal_out', 2.225074e-308)
        time.sleep(0.1)
        assert read_pv('lreal_in') == 2.225074e-308
        assert get_plc_var('TestPlan.lreal_out') == 2.225074e-308

    def test_lreal_boundary_max_negative(self, write_pv, read_pv, get_plc_var):
        """Test maximum negative representable value."""
        write_pv('lreal_out', -1.797693e308)
        time.sleep(0.1)
        assert read_pv('lreal_in') == -1.797693e308
        assert get_plc_var('TestPlan.lreal_out') == -1.797693e308

    def test_lreal_special_infinity_positive(self, write_pv, read_pv, get_plc_var):
        """Test positive infinity special value."""
        import math
        write_pv('lreal_out', float('inf'))
        time.sleep(0.1)
        result = read_pv('lreal_in')
        assert math.isinf(result) and result > 0, f"Expected +inf, got {result}"
        plc_result = get_plc_var('TestPlan.lreal_out')
        assert math.isinf(plc_result) and plc_result > 0, f"PLC: Expected +inf, got {plc_result}"

    def test_lreal_special_infinity_negative(self, write_pv, read_pv, get_plc_var):
        """Test negative infinity special value."""
        import math
        write_pv('lreal_out', float('-inf'))
        time.sleep(0.1)
        result = read_pv('lreal_in')
        assert math.isinf(result) and result < 0, f"Expected -inf, got {result}"
        plc_result = get_plc_var('TestPlan.lreal_out')
        assert math.isinf(plc_result) and plc_result < 0, f"PLC: Expected -inf, got {plc_result}"

    def test_lreal_special_nan(self, write_pv, read_pv, get_plc_var):
        """Test NaN (not-a-number) special value."""
        import math
        write_pv('lreal_out', float('nan'))
        time.sleep(0.1)
        result = read_pv('lreal_in')
        assert math.isnan(result), f"Expected NaN, got {result}"
        plc_result = get_plc_var('TestPlan.lreal_out')
        assert math.isnan(plc_result), f"PLC: Expected NaN, got {plc_result}"
